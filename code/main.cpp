#include "graph.h"
#include "sequential.h"
#include "jp.h"
#include <iostream>
#include <vector>
#include <cstdlib>

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
    Graph graph = graph_generate(10, 0.8);
    // greedy_color(graph);
    jones_plassmann(graph);
    graph.print_graph();
    bool correct = graph.check_coloring();
    printf("valid coloring: %s\n", correct ? "yes" : "no");
    return 0;
}