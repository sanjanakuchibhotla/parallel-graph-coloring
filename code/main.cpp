#include "graph.h"
#include "sequential.h"
#include "jp.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>

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

int main() {
    // Graph graph = graph_generate(10, 0.8);
    Graph graph = graph_generate(10000, 0.01);

    // greedy_color(graph);
    printf("Starting Jones-Plassmann coloring...\n"); 
    auto start_time = std::chrono::steady_clock::now();
    jones_plassmann(graph);
    auto end_time = std::chrono::steady_clock::now();
    double elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
    printf("Jones-Plassman coloring complete!");
    printf("Execution time (sec): %.6f\n", elapsed_time);
    graph.print_graph();
    bool correct = graph.check_coloring();
    printf("valid coloring: %s\n", correct ? "yes" : "no");
    return 0;
}