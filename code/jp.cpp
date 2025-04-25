// jones plassman parallel graph coloring algorithm 

#include "graph.h"
#include <omp.h>
#include <set>
#include <stdio.h>
#include <chrono>
#include <iostream>

// Graph::Graph(int vertices): V(vertices), E(0), adj_list(vertices), colors(vertices, -1) {}
void jones_plassmann(Graph& graph) {
     // randomly assign unique priority/weight to each vertex
    int N = graph.size();
    graph.assign_priorities();
    std::vector<int>& colors = graph.get_colors();
    std::vector<int>& priorities = graph.get_priorities();

    std::vector<std::vector<int>> neighbors(N);
    #pragma omp parallel for
    for (int i = 0; i < N; ++i) {
        neighbors[i] = graph.get_neighbors(i);
    }

    bool colored_vertex = true;
    while (colored_vertex) {
        colored_vertex = false;

        std::vector<bool> vertices_to_color(N, false);

        #pragma omp parallel for schedule(static)
        for (int u = 0; u < N; ++u) {
            if (colors[u] == -1) {
                bool flag = true; // flag for if the vertex is local max
                for (int v : neighbors[u]) {
                    if (colors[v] == -1 && priorities[v] > priorities[u]) {
                        flag = false;
                        break;
                    }
                }
                vertices_to_color[u] = flag;
            }
        }

        #pragma omp parallel for schedule(static) reduction(|:colored_vertex)
        for (int u = 0; u < N; u++) {
            if (vertices_to_color[u]) {
                std::vector<bool> nbor_colors(N, false);
                for (int v : neighbors[u]) {
                    if (colors[v] != -1) {
                        nbor_colors[colors[v]] = true;
                    }
                }

                int color = 0;
                while (nbor_colors[color]) {
                    color++;
                }

                colors[u] = color;
                colored_vertex = true;
            }
        }
    }
}
    // every uncolored vertex checks all neighbors and if it has highest priority, color vertex w min color, mark vertex as colored
    // while ()
