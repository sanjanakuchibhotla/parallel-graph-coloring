#include "graph.h"
#include "sequential.h"
#include "jp.h"
#include "gm.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unistd.h>  // getopt
#include <omp.h>
#include <chrono>

Graph graph_generate(int N, double p) {
    Graph graph(N);
    for (int u = 0; u < N; ++u) {
        for (int v = u + 1; v < N; ++v) {
            double r = static_cast<double>(rand()) / RAND_MAX;
            if (r < p) {
                graph.add_edge(u, v);
            }
        }
    }
    return graph;
}

// Write graph edges to file (one edge u v per line)
void write_graph(const Graph &graph, const std::string &filename) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Error: cannot open " << filename << " for writing\n";
        return;
    }
    int N = graph.size();
    for (int u = 0; u < N; ++u) {
        // get_neighbors returns non-const reference, but graph is const,
        // so use const_cast to access neighbors safely
        auto &nbrs = const_cast<Graph&>(graph).get_neighbors(u);
        for (int v : nbrs) {
            if (u < v) ofs << u << ' ' << v << '\n';
        }
    }
}

// Load graph from edge list file
Graph load_graph(int N, const std::string &filename) {
    Graph graph(N);
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cerr << "Error: cannot open " << filename << " for reading\n";
        return graph;
    }
    int u, v;
    while (ifs >> u >> v) {
        if (u >= 0 && u < N && v >= 0 && v < N) {
            graph.add_edge(u, v);
        }
    }
    return graph;
}

int main(int argc, char *argv[]) {
    int    num_vertices = 0;
    double edge_prob    = -1.0;
    int    num_threads  = 1;
    std::string algo;
    std::string infile;
    std::string outfile;
    bool   load_flag    = false;

    int opt;
    while ((opt = getopt(argc, argv, "i:v:p:n:a:o:")) != -1) {
        switch (opt) {
            case 'i': infile       = optarg; load_flag = true;  break;
            case 'v': num_vertices = atoi(optarg);             break;
            case 'p': edge_prob    = atof(optarg);             break;
            case 'n': num_threads  = atoi(optarg);             break;
            case 'a': algo         = optarg;                   break;
            case 'o': outfile      = optarg;                   break;
            default:
                std::cerr << "Usage: " << argv[0]
                          << " [-i infile] [-v num_vertices -p edge_prob] -n num_threads -a algorithm [-o outfile]\n";
                return EXIT_FAILURE;
        }
    }

    if (algo.empty() || num_threads <= 0) {
        std::cerr << "Error: must specify -a algorithm and -n num_threads\n";
        return EXIT_FAILURE;
    }
    if (load_flag) {
        if (infile.empty() || num_vertices <= 0) {
            std::cerr << "Error: loading requires -i infile and -v num_vertices\n";
            return EXIT_FAILURE;
        }
    } else {
        if (num_vertices <= 0 || edge_prob < 0) {
            std::cerr << "Error: generation requires -v num_vertices and -p edge_prob\n";
            return EXIT_FAILURE;
        }
        if (outfile.empty()) outfile = "graph.txt";
    }

    std::srand(unsigned(std::time(nullptr)));
    omp_set_num_threads(num_threads);

    Graph graph = load_flag
                  ? load_graph(num_vertices, infile)
                  : graph_generate(num_vertices, edge_prob);

    if (!load_flag) {
        write_graph(graph, outfile);
        std::cout << "Generated graph saved to " << outfile << "\n";
    }

    // Reset any existing colors before running
    graph.reset_colors();

    auto t0 = std::chrono::steady_clock::now();
    if      (algo == "jp")     jones_plassmann(graph);
    else if (algo == "greedy") greedy_color(graph);
    else if (algo == "gm")     gebremedhin_manne(graph);
    else {
        std::cerr << "Error: unknown algorithm '" << algo << "'\n";
        return EXIT_FAILURE;
    }
    auto t1 = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "==========================\n"
              << "Algorithm: "       << algo         << '\n'
              << "Vertices: "        << num_vertices  << '\n'
              << "Threads: "         << num_threads   << '\n'
              << (load_flag
                  ? "Loaded from file: " + infile + '\n'
                  : "Edge prob: " + std::to_string(edge_prob) + '\n')
              << (load_flag ? "" : "Saved graph to: " + outfile + '\n')
              << "Valid Coloring: "  << (graph.check_coloring() ? "Yes" : "No") << '\n'
              << "Colors used: "     << graph.count_colors()  << '\n'
              << "Time (s): "        << elapsed       << '\n'
              << "==========================\n";

    return 0;
}
