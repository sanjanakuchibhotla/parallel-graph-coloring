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


void mpi_jones_plassmann(Graph& graph, int pid, int nproc) {

    int N = graph.size();
    graph.assign_priorities();

    auto& colors = graph.get_colors();
    auto& priorities = graph.get_priorities(); 

    int color=1;
    vector<int> colored(N,0);

    int num_vertices_per_process = N/nproc; 

    int start = pid * num_vertices_per_process;
    int end;
    if (nproc == pid + 1) {
        end = N;
    }
    else {
        end = (pid + 1) * num_vertices_per_process;
    }

    while (true) {
        MPI_Allreduce(MPI_IN_PLACE, colors.data(), N, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        bool complete = false;

        for (int i =start; i < end; i++){
            if (colors[i]!=-1){
                continue;
            }
            bool flag = true;
            for (int nei: graph.get_neighbors(i)){
                if (priorities[i] > priorities[nei] && colors[nei] == -1 ){
                    flag = false;
                    break;
                }
            }
            if (!flag){
                continue;
            }
            color++;
            for (int nei: graph.get_neighbors(i)) {
                int c = colors[nei];
                if (c >= 0){
                    colored[c] = color;
                }
            }
            int c = 0;
            while (colored[c] == color){
                c++;
            }
            colors[i] = c;
            complete = true;
        }

        int local_all_colored = 1;
        for (int i=start;i<end;i++){
            if (colors[i] == -1){
                local_all_colored = 0;
                break;
            }
        }
        int num_complete = 0;
        MPI_Allreduce(&local_all_colored, &num_complete, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if (num_complete == nproc){
            break;
        }

    }

}


int main(int argc, char** argv){
    MPI_Init(&argc, &argv);
    int pid,nproc;

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    unsigned random_seed;

    if (pid == 0) {
        random_seed = time(nullptr);
    }
    MPI_Bcast(&random_seed, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    srand(random_seed);
    int num_vertices = 0;
    double edge_prob = 0.01;
    int num_threads = 1;

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
    MPI_Barrier(MPI_COMM_WORLD);

    double initial_time= CycleTimer::currentSeconds();

    mpi_jones_plassmann(graph,pid,nproc);
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







