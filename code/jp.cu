
#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h> 
#include <driver_functions.h>

#include "CycleTimer.h"
#include "graph.h"
#include "sequential.h"       
#include "jp.h"           

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <chrono>
#include <algorithm> 

static std::mt19937 rng(std::random_device{}());
using namespace std;

// generate an erdos-renyi random undirected graph using a compressed‐sparse‐row representation

void random_graph_generator(int N, double p, vector<int> &adjacency_list, vector<int>& vertex_offsets){
    vector<vector<int>> neighbors(N);
    uniform_real_distribution<> dist(0.0, 1.0);
    // mt19937 rng(random_device{}());

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

__global__ void initial_jones_plassmann(int N, int* colors_device, bool* uncolored){
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        colors_device[i] = 0;
        //vertex i is uncolored
        uncolored[i] = true; 
    }
}

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
    bool conflict;

    for (color = 1;; color++){
        conflict = false;
        for (int edge = vertex_offsets[i]; edge < vertex_offsets[i+1];edge++){
            if (colors[adjacency_list[edge]] == color){
                conflict = true;
                break;
            }
        }
        if (!conflict){
            break;
        }
    }
    colors[i] = color;
    uncolored[i] = false;
    *complete = 1; 

}
    
void cuda_jones_plassmann(int N, int* adjacency_list, int* vertex_offsets, int* priorities, int* colors, bool* uncolored){
    int blockWidth = 128;
    dim3 blockDim(blockWidth,1,1);
    int gridWidth = (N + blockWidth -1)/(blockWidth);
    dim3 gridDim(gridWidth,1,1); 
    

    int* changed;
    cudaMalloc(&changed, sizeof(int));

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

void sequential_jones_plassmann(Graph& graph){
    int N = graph.size();
    graph.assign_priorities();
    std::vector<int>& colors = graph.get_colors();
    std::vector<int>& priorities = graph.get_priorities();

    bool colored_vertex = true;
    while (colored_vertex) {
        colored_vertex = false;
        // get all vertices to color
        std::vector<bool> uncolored;
        uncolored.resize(N, false);
        for (int u = 0; u < N; u++) {
            if (colors[u] == -1) {
                bool flag = true; // flag for if the vertex is local max
                std::vector<int> neighbors = graph.get_neighbors(u);
                for (int i = 0; i < neighbors.size(); i++) {
                    int v = neighbors[i];
                    if (colors[v] == -1 && priorities[v] > priorities[u]) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    uncolored[u] = true;
                }
            }
        }

        for (int u = 0; u < graph.size(); u++) {
            if (uncolored[u]) {
                std::set<int> nbor_colors;
                std::vector<int> neighbors = graph.get_neighbors(u);
                for (int i = 0; i < neighbors.size(); i++) {
                    int v = neighbors[i];
                    if (colors[v] != -1) {
                        nbor_colors.insert(colors[v]);
                    }
                }

                int color = 0;
                
                while (nbor_colors.count(color) > 0) { 
                    color++;
                }
                colors[u] = color;
                colored_vertex = true;
            }
        }
    }   
}


int main(){
    srand(time(nullptr)); 
    int N = 1000;
    double p = 0.01;

    vector<int> adjacency_list, vertex_offsets;
    random_graph_generator(N,p, adjacency_list, vertex_offsets);
    printf("Number of nodes: %d \nNumber of edges: %zu\n", N, adjacency_list.size()/2);

    int *adj_list_device, *vertex_offsets_device, *priorities_device, *colors_device; 
    bool* uncolored; 
    cudaMalloc(&adj_list_device, adjacency_list.size()  * sizeof(int));
    cudaMalloc(&vertex_offsets_device, vertex_offsets.size() * sizeof(int));
    cudaMalloc(&priorities_device, N  * sizeof(int));
    cudaMalloc(&colors_device, N * sizeof(int));
    cudaMalloc(&uncolored, N * sizeof(bool));

    cudaMemcpy(adj_list_device, adjacency_list.data(), adjacency_list.size() * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(vertex_offsets_device, vertex_offsets.data(), vertex_offsets.size()  * sizeof(int), cudaMemcpyHostToDevice);

    Graph graph(N); 
    for (int i=0; i<N; i++){
        for (int edge = vertex_offsets[i]; edge<vertex_offsets[i+1]; edge++){
            graph.add_edge(i, adjacency_list[edge]);
        }
    }
    graph.assign_priorities(); 
    vector<int> priorities = graph.get_priorities(); 
    cudaMemcpy(priorities_device,priorities.data(), N * sizeof(int), cudaMemcpyHostToDevice);

    int blockWidth = 128;
    dim3 blockDim(blockWidth,1,1);
    int gridWidth = (N + blockWidth -1)/(blockWidth);
    dim3 gridDim(gridWidth,1,1); 

    initial_jones_plassmann<<<gridDim, blockDim>>>(N,colors_device,uncolored);
    cudaDeviceSynchronize();
    cudaEvent_t start,end;
    cudaEventCreate(&start);
    cudaEventCreate(&end);
    cudaEventRecord(start);
    cuda_jones_plassmann(N,adj_list_device, vertex_offsets_device,priorities_device,colors_device, uncolored);
    cudaEventRecord(end);
    cudaEventSynchronize(end);
    float GPU_elapsed_time = 0;
    cudaEventElapsedTime(&GPU_elapsed_time,start,end);


    vector<int> GPU_colors(N); 
    cudaMemcpy(GPU_colors.data(),colors_device, N*sizeof(int), cudaMemcpyDeviceToHost); 
    auto &colors = graph.get_colors();
    colors = GPU_colors; 

    bool valid_coloring = (graph.check_coloring() == 1);
    int num_colors_used = count_colors(graph); 

    printf("Jones-Plassman on GPU: valid:%d \ncolors:%d \n time elapsed: %.5f seconds\n", valid_coloring,num_colors_used, GPU_elapsed_time); 

    graph.reset_colors(); 
    double initial_time= CycleTimer::currentSeconds();
    sequential_jones_plassmann(graph); 
    double final_time = CycleTimer::currentSeconds(); 
    double elapsed_time = (final_time-initial_time);

    printf("Jones-Plassman on CPU: valid:%d \ncolors:%d \n time elapsed: %.5f seconds\n", valid_coloring,num_colors_used, elapsed_time); 

    graph.reset_colors(); 
    initial_time = CycleTimer::currentSeconds();
    greedy_color(graph); 
    final_time = CycleTimer::currentSeconds();  
    elapsed_time = (final_time-initial_time);

    printf("Greedy on CPU: valid:%d \ncolors:%d \n time elapsed: %.5f seconds\n", valid_coloring,num_colors_used, elapsed_time); 

    cudaFree(adj_list_device);
    cudaFree(vertex_offsets_device); 
    cudaFree(priorities_device);
    cudaFree(colors_device);
    cudaFree(uncolored);
    return 0; 
}













