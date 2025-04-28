#include "graph.h"
#include "greedy.h"
#include "jp.h"
#include "speculative.h"
#include "spec_jp.h"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <string>
#include <omp.h>

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

void execute_algorithm(Graph &graph, int num_threads, double edge_prob, std::string algo) {
    omp_set_num_threads(num_threads);
    double elapsed_time = 0.0;
    graph.assign_priorities();
    if (algo == "jp") {
        auto start_time = std::chrono::steady_clock::now();
        jones_plassmann(graph);
        auto end_time = std::chrono::steady_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
    } else if (algo == "greedy") {
        auto start_time = std::chrono::steady_clock::now();
        greedy_color(graph);
        auto end_time = std::chrono::steady_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
    } else if (algo == "spec") {
        auto start_time = std::chrono::steady_clock::now();
        speculative_coloring(graph);
        auto end_time = std::chrono::steady_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
    } else if (algo == "specjp") {
        auto start_time = std::chrono::steady_clock::now();
        printf("Running\n");
        speculative_jones_plassmann(graph);
        auto end_time = std::chrono::steady_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
    } else {
        printf("Please provide a valid algorithm\n");
        exit(EXIT_FAILURE);
    }
    printf("==========================\n");
    printf("----------OUTPUT----------\n");
    printf("Edge Probability: %f\n", edge_prob);
    graph.print_graph();
    printf("Num threads: %d\n", num_threads);
    printf("Algorithm: %s\n", algo.c_str());
    bool correct = graph.check_coloring();
    printf("Valid Coloring: %s\n", correct ? "Yes" : "No");
    printf("Execution time (sec): %.6f\n", elapsed_time);
    printf("Number of Colors: %d\n", graph.count_colors());
    printf("--------------------------\n");
    printf("==========================\n");
    graph.reset_colors();
}

int main(int argc, char* argv[]) {
    int num_vertices = 0;
    double edge_prob = 0.01;
    int num_threads = 1;
    std::string algo;

    int opt;
    while ((opt = getopt(argc, argv, "v:p:n:a:")) != -1) {
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
        case 'a':
            algo = optarg;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " -v num_vertices [-p edge_prob] -n num_threads -a algorithm\n";
            exit(EXIT_FAILURE);
        }
    }

    // Check if required options are provided
    if (algo.empty() || num_vertices <= 0 || edge_prob < 0 || num_threads <= 0) {
        std::cerr << "Usage: " << argv[0] << " -v num_vertices [-p edge_prob] -n num_threads -a algorithm\n";
        exit(EXIT_FAILURE);
    }
    
    // initialization time
    auto start_init_time = std::chrono::steady_clock::now();
    Graph graph = graph_generate(num_vertices, edge_prob);
    auto end_init_time = std::chrono::steady_clock::now();
    double total_init_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_init_time - start_init_time).count();
    printf("==========================\n");
    printf("Initialization Time (sec): %.6f\n", total_init_time);

    // greedy
    execute_algorithm(graph, 1, edge_prob, "greedy");

    // jp
    execute_algorithm(graph, 1, edge_prob, "jp");
    // graph.reset_colors();
    execute_algorithm(graph, 2, edge_prob, "jp");
    // graph.reset_colors();
    execute_algorithm(graph, 4, edge_prob, "jp");
    // graph.reset_colors();
    execute_algorithm(graph, 8, edge_prob, "jp");

    // spec jp
    // jp
    execute_algorithm(graph, 1, edge_prob, "specjp");
    // graph.reset_colors();
    execute_algorithm(graph, 2, edge_prob, "specjp");
    // graph.reset_colors();
    execute_algorithm(graph, 4, edge_prob, "specjp");
    // graph.reset_colors();
    execute_algorithm(graph, 8, edge_prob, "specjp");

    // spec
    execute_algorithm(graph, 1, edge_prob, "spec");
    // graph.reset_colors();
    execute_algorithm(graph, 2, edge_prob, "spec");
    // graph.reset_colors();
    execute_algorithm(graph, 4, edge_prob, "spec");
    // graph.reset_colors();
    execute_algorithm(graph, 8, edge_prob, "spec");
    // graph.reset_colors();
    
}
