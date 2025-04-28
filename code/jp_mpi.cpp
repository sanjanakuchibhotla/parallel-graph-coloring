/* Parallel Graph Coloring Implementation using Jones-Plassmann with MPI model.*/

#include <stdio.h>
#include <mpi.h>
#include "jp_mpi.h"
#include "CycleTimer.h"
#include "graph.h"
#include "sequential.h"       
#include "jp.h"           
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace std;

// Generates a random graph with N vertices and edge probability p

Graph graph_generate(int N, double p) {
    Graph graph(N);
    for (int u = 0; u < N; u++) {
        for (int v = u+1; v < N; v++) {
            double rand_n = (double)rand()/(double)(RAND_MAX);
            if (rand_n < p) {
                graph.add_edge(u,v);
            }
        }
    }
    return graph;
}

// Jones-Plassman graph coloring using MPI
void mpi_jones_plassmann(Graph& graph, int pid, int nproc) {
    int N = graph.size();

    // Process 0 assigns random priorities to all vertices
    if (pid == 0) {
        graph.assign_priorities();
    }
    
    // Broadcast priorities once at the beginning
    std::vector<int> global_pri = graph.get_priorities();
    MPI_Bcast(global_pri.data(), N, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Update local priority vector with broadcasted values
    auto &priorities = graph.get_priorities();
    for (int i = 0; i < N; i++) {
        priorities[i] = global_pri[i];
    }

    auto &colors = graph.get_colors();
    
    // Compute vertices per process with better load balancing
    int num_vertices_per_process = (N + nproc - 1) / nproc; 
    int start = pid * num_vertices_per_process;
    int end;
    if (nproc == pid + 1) {
        end = N;
    }
    else {
        end = (pid + 1) * num_vertices_per_process;
    }
    
    // created a smaller array for tracking used colors
    const int MAX_COLORS = 1000; 
    std::vector<int> used_colors(MAX_COLORS, -1);
    
    while (true) {

        for (int i = start; i < end; i++) {
            if (colors[i] != -1) continue;
            
            bool can_color = true;
            for (int neighbor : graph.get_neighbors(i)) {
                if (priorities[neighbor] > priorities[i] && colors[neighbor] == -1) {
                    can_color = false;
                    break;
                }
            }
            
            if (can_color) {
                for (int neighbor : graph.get_neighbors(i)) {
                    if (colors[neighbor] >= 0 && colors[neighbor] < MAX_COLORS) {
                        used_colors[colors[neighbor]] = i;
                    }
                }
                // Find lowest unused color
                int c = 0;
                while (c < MAX_COLORS && used_colors[c] == i) {
                    c++;
                }
                
                colors[i] = c;
                
                // Reset used colors for next vertex
                for (int neighbor : graph.get_neighbors(i)) {
                    if (colors[neighbor] >= 0 && colors[neighbor] < MAX_COLORS) {
                        used_colors[colors[neighbor]] = -1;
                    }
                }
            }
        }
        
        MPI_Allreduce(MPI_IN_PLACE, colors.data(), N, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        
        // Check if all vertices in this process are colored
        int local_done = 1;
        for (int i = start; i < end; i++) {
            if (colors[i] == -1) {
                local_done = 0;
                break;
            }
        }
        
        // Check if all processes are done
        int global_done = 0;
        MPI_Allreduce(&local_done, &global_done, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        
        // If all processes are done, break out of the loop
        if (global_done == nproc) break;
    }
}

int main(int argc, char** argv){
    //initalize MPI 
    MPI_Init(&argc, &argv);
    int pid,nproc;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    // initalize random seed
    unsigned random_seed;

    if (pid == 0) {
        random_seed = time(nullptr);
    }
    // broadcast random seed to make sure all processes generate the same graph
    MPI_Bcast(&random_seed, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    srand(random_seed);

    int num_vertices = 0;
    double edge_prob = 0.01;
    int num_threads = 1;

    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "v:p:n:")) != -1) {
        switch (opt) {
        case 'v':
            num_vertices = atoi(optarg);
            break;
        case 'p':
            edge_prob = atof(optarg);
            break;
        case 'n':
            num_threads = atoi(optarg);
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " -v num_vertices [-p edge_prob] -n num_threads\n";
            exit(EXIT_FAILURE);
        }
    }

    if (num_vertices <= 0 || edge_prob < 0 || num_threads <= 0) {
        std::cerr << "Usage: " << argv[0] << " -v num_vertices [-p edge_prob] -n num_threads\n";
        exit(EXIT_FAILURE);
    }

    Graph graph = graph_generate(num_vertices, edge_prob);

    // sync all processes before starting graph coloring
    MPI_Barrier(MPI_COMM_WORLD);
    double initial_time= CycleTimer::currentSeconds();
    mpi_jones_plassmann(graph,pid,nproc);
    // sync to make sure all processes finished
    MPI_Barrier(MPI_COMM_WORLD);
    double final_time = CycleTimer::currentSeconds(); 
    double gpu_elapsed_time = (final_time - initial_time);


    if (pid==0){
        printf("Jones-Plassman with MPI:\nvalid coloring: %s\ncolors: %d \ntime elapsed: %.5f seconds\n",(graph.check_coloring() == 1) ? "true": "false",graph.count_colors(), gpu_elapsed_time); 
        printf("_____________________________________\n");
    }

    graph.reset_colors(); 
    initial_time = CycleTimer::currentSeconds();
    greedy_color(graph); 
    final_time = CycleTimer::currentSeconds();  
    if (pid == 0) {
        double elapsed_time = (final_time-initial_time);
        bool valid_coloring = (graph.check_coloring() == 1);
        int num_colors_used = graph.count_colors(); 
        printf("Greedy on CPU:\nvalid coloring: %s\ncolors: %d \ntime elapsed: %.5f seconds\n", valid_coloring ? "true": "false",num_colors_used, elapsed_time); 
        printf("speedup(Greedy): %.5f\n", elapsed_time/gpu_elapsed_time);

    }

    MPI_Finalize();
    return 0;
}







