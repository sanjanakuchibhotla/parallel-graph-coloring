#ifndef JP_CUDA_H
#define JP_CUDA_H

#include <vector>

void random_graph_generator(int N, double p, vector<int> &adjacency_list, vector<int>& vertex_offsets);
__global__ void initial_jones_plassmann(int N, int* colors_device, bool* uncolored);
__global__ void color_independent_set(int N, int* adjacency_list, int* vertex_offsets, int* priorities, int* colors, bool* uncolored, int* complete);
void cuda_jones_plassmann(int N, int num_threads, int* adjacency_list, int* vertex_offsets, int* priorities, int* colors, bool* uncolored);

#endif