
#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h> 
#include <driver_functions.h>

#include "CycleTimer.h"
#include "graph.h"
#include "sequential.h"       
#include "jp.h"           
#include "jp_cuda.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <set>
#include <random>
#include <unistd.h>


static std::mt19937 rng(std::random_device{}());
using namespace std;

// generate an erdos-renyi random undirected graph using a compressed‐sparse‐row representation (utilizes less memory than normal adjacency list)

void random_graph_generator(int N, double p, vector<int> &adjacency_list, vector<int>& vertex_offsets){
    vector<vector<int>> neighbors(N);
    uniform_real_distribution<> dist(0.0, 1.0);

    // add each edge (i,j) with probability ~p
    for (int i=0; i< N; i++){
        for (int j= i+1; j <N;j++){
            if (dist(rng) < p){
                neighbors[i].push_back(j);
                neighbors[j].push_back(i);

            }
        }
    }

    // find the offset for each vertex to populate the vertex offsets vextors
    // e.g vertex_offsets[i] = index in adjacency_list vector where all of vertex i's neighbors are listed
    // If adjacency_list = [3,7,2,0,5,6], and vertex_offsets = [0,2,3,6]
    // We have vertex 0's neighbors as 3 and 7
    // vertex 1 neighbors: 2
    // vertex 2 neighbors: 0,5,6

    vertex_offsets.resize(N+1);
    int idx = 0;
    for (int i=0; i <N;i++){
        vertex_offsets[i] = idx;
        for (int nei: neighbors[i]){
            adjacency_list.push_back(nei);
            idx++; 
        }
    }
    // the last value of vertex_offsets is the len of adjacency_list
    vertex_offsets[N] = idx;
}


// GPU Kernel function that initalizes the data structures Jones-plassman uses to color the graph 
__global__ void initial_jones_plassmann(int N, int* colors_device, bool* uncolored){
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        colors_device[i] = 0;
        uncolored[i] = true; 
    }
}


// GPU kernel function that does an iteration of jones-plassman, coloring an maximal independent set of vertices
__global__ void color_independent_set(int N, int* adjacency_list, int* vertex_offsets, int* priorities, int* colors, bool* uncolored, int* complete) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N || !uncolored[i]){
        return;
    }
    int curr_priority = priorities[i];
    for (int edge = vertex_offsets[i]; edge < vertex_offsets[i+1];edge++){
        int nei = adjacency_list[edge];
        if (uncolored[nei] && priorities[nei] > curr_priority){
            return;
        }
    }
    int color =1;
    bool flag;

    for (color = 1;; color++){
        flag = false;
        for (int edge = vertex_offsets[i]; edge < vertex_offsets[i+1];edge++){
            if (colors[adjacency_list[edge]] == color){
                flag = true;
                break;
            }
        }
        if (!flag){
            break;
        }
    }
    colors[i] = color;
    uncolored[i] = false;
    *complete = 1; 

}

// host code that calls the kernel functions
void cuda_jones_plassmann(int N, int num_threads, int* adjacency_list, int* vertex_offsets, int* priorities, int* colors, bool* uncolored){
    int blockWidth = num_threads;
    dim3 blockDim(blockWidth,1,1);
    int gridWidth = (N + blockWidth -1)/(blockWidth);
    dim3 gridDim(gridWidth,1,1); 
    

    int* changed;
    cudaMalloc(&changed, sizeof(int));
    //calls kernel function that colors MIS for each JP iteration until entire graph has been colored
    while(true) {
        int has_changed = 0;
        cudaMemcpy(changed, &has_changed, sizeof(int), cudaMemcpyHostToDevice);
        color_independent_set<<<gridDim,blockDim>>>(N,adjacency_list,vertex_offsets,priorities, colors, uncolored, changed);
        cudaDeviceSynchronize();
        cudaMemcpy(&has_changed, changed, sizeof(int), cudaMemcpyDeviceToHost);
        if (!has_changed){
            break;
        }
    }
    cudaFree(changed);
}


int main(int argc, char** argv) {
    srand(time(nullptr)); 
    int opt;
    int num_vertices;
    int num_threads;
    double edge_prob;

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
        // case 'a':
        //     algo = optarg;
        //     break;
        default:
            std::cerr << "Usage: " << argv[0] << " -v num_vertices [-p edge_prob] -n num_threads\n";
            exit(EXIT_FAILURE);
        }
    }

    // Check if required options are provided
    if (num_vertices <= 0 || edge_prob < 0 || num_threads <= 0) {
        std::cerr << "Usage: " << argv[0] << " -v num_vertices [-p edge_prob] -n num_threads -a algorithm\n";
        exit(EXIT_FAILURE);
    }
    int N = num_vertices;
    double p = edge_prob;
    double initial_time, final_time, elapsed_time;
    initial_time= CycleTimer::currentSeconds();



    vector<int> adjacency_list, vertex_offsets;
    
    random_graph_generator(N,p, adjacency_list, vertex_offsets);
    final_time = CycleTimer::currentSeconds(); 
    elapsed_time = (final_time-initial_time);
    printf("Initalization time: %.5f \n", elapsed_time);

    printf("Number of nodes: %d \nNumber of edges: %zu\n", N, adjacency_list.size()/2);
    printf("__________________________________________\n");
    int *adj_list_device, *vertex_offsets_device, *priorities_device, *colors_device; 
    bool* uncolored; 

    // Allocate GPU memory for graph data
    cudaMalloc(&adj_list_device, adjacency_list.size()  * sizeof(int));
    cudaMalloc(&vertex_offsets_device, vertex_offsets.size() * sizeof(int));
    cudaMalloc(&priorities_device, N  * sizeof(int));
    cudaMalloc(&colors_device, N * sizeof(int));
    cudaMalloc(&uncolored, N * sizeof(bool));

    cudaMemcpy(adj_list_device, adjacency_list.data(), adjacency_list.size() * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(vertex_offsets_device, vertex_offsets.data(), vertex_offsets.size()  * sizeof(int), cudaMemcpyHostToDevice);
    
    // Create Graph object for CPU algos
    Graph graph(N); 
    for (int i=0; i<N; i++){
        for (int edge = vertex_offsets[i]; edge<vertex_offsets[i+1]; edge++){
            graph.add_edge(i, adjacency_list[edge]);
        }
    }
    graph.assign_priorities(); 
    vector<int> priorities = graph.get_priorities(); 
    cudaMemcpy(priorities_device,priorities.data(), N * sizeof(int), cudaMemcpyHostToDevice);
    bool valid_coloring;
    int num_colors_used; 

    // kernel launch params
    int blockWidth = num_threads;
    dim3 blockDim(blockWidth,1,1);
    int gridWidth = (N + blockWidth -1)/(blockWidth);
    dim3 gridDim(gridWidth,1,1); 

    initial_time= CycleTimer::currentSeconds();
    initial_jones_plassmann<<<gridDim, blockDim>>>(N,colors_device,uncolored);
    cudaDeviceSynchronize();
    cuda_jones_plassmann(N,num_threads, adj_list_device, vertex_offsets_device,priorities_device,colors_device, uncolored);
    final_time = CycleTimer::currentSeconds(); 

    double GPU_elapsed_time = (final_time-initial_time);;
    vector<int> GPU_colors(N); 
    cudaMemcpy(GPU_colors.data(),colors_device, N*sizeof(int), cudaMemcpyDeviceToHost); 
    auto &colors = graph.get_colors();
    colors = GPU_colors; 
    
    valid_coloring = (graph.check_coloring() == 1);
    num_colors_used = graph.count_colors(); 

    printf("Jones-Plassman on GPU:\nvalid coloring: %s\ncolors: %d \ntime elapsed: %.5f seconds\n", valid_coloring ? "true": "false",num_colors_used, GPU_elapsed_time); 
    printf("__________________________________________\n");

    graph.reset_colors(); 
    initial_time= CycleTimer::currentSeconds();
    jones_plassmann(graph); 
    final_time = CycleTimer::currentSeconds(); 
    elapsed_time = (final_time-initial_time);
    valid_coloring = (graph.check_coloring() == 1);
    num_colors_used = graph.count_colors(); 

    printf("Jones-Plassman on CPU:\nvalid coloring: %s\ncolors: %d \ntime elapsed: %.5f seconds\n", valid_coloring ? "true": "false",num_colors_used, elapsed_time); 
    printf("speedup(JP): %.5f\n", elapsed_time/GPU_elapsed_time);
    printf("__________________________________________\n");


    graph.reset_colors(); 
    initial_time = CycleTimer::currentSeconds();
    greedy_color(graph); 
    final_time = CycleTimer::currentSeconds();  
    elapsed_time = (final_time-initial_time);
    valid_coloring = (graph.check_coloring() == 1);
    num_colors_used = graph.count_colors(); 
    printf("Greedy on CPU:\nvalid coloring: %s\ncolors: %d \ntime elapsed: %.5f seconds\n", valid_coloring ? "true": "false",num_colors_used, elapsed_time); 
    printf("speedup(Greedy): %.5f\n", elapsed_time/GPU_elapsed_time);

    cudaFree(adj_list_device);
    cudaFree(vertex_offsets_device); 
    cudaFree(priorities_device);
    cudaFree(colors_device);
    cudaFree(uncolored);
    return 0; 
}
