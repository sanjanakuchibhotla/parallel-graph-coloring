#include "graph.h"
#include "sequential.h"
#include "jp.h"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <string>


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

int main(int argc, char* argv[]) {
    int num_vertices = 0;
    double edge_prob = 0.1;
    char mode = '\0'; // P or S (parallel or sequential)
    std::string algo;

    int opt;
    while ((opt = getopt(argc, argv, "v:p:m:a:")) != -1) {
        switch (opt) {
        case 'v':
            num_vertices = atoi(optarg);
            break;
        case 'p':
            edge_prob = atof(optarg);
            break;
        case 'm':
            mode = *optarg;
            break;
        case 'a':
            algo = optarg;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " -v num_vertices [-p edge_prob] -m mode (P or S) -a algorithm\n";
            exit(EXIT_FAILURE);
        }
    }

    // Check if required options are provided
    if (algo.empty() || num_vertices <= 0 || edge_prob < 0 || (mode != 'S' && mode != 'P')) {
        std::cerr << "Usage: " << argv[0] << " -v num_vertices [-p edge_prob] -m mode (P or S) -a algorithm\n";
        exit(EXIT_FAILURE);
    }
    Graph graph = graph_generate(num_vertices, edge_prob);
    if (mode == 'S') {
        if (algo == "jp") {
            jones_plassmann(graph);
        }
        else if (algo == "greedy") {
            greedy_color(graph);
        }
    }
    printf("==========================\n");
    printf("----------OUTPUT----------\n");
    printf("Edge Probability: %f\n", edge_prob);
    graph.print_graph();
    printf("Mode (P/S): %c\n", mode);
    printf("Algorithm: %s\n", algo.c_str());
    bool correct = graph.check_coloring();
    printf("Valid Coloring: %s\n", correct ? "Yes" : "No");
    printf("--------------------------\n");
    printf("==========================\n");
    return 0;
}