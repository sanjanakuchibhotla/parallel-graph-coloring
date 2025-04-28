#include "graph.h"
#include <omp.h>
#include <vector>

void speculative_jones_plassmann(Graph& graph) {
    int N = graph.size();
    std::vector<int>& colors = graph.get_colors();
    graph.assign_priorities();
    std::vector<int>& priorities = graph.get_priorities();
    // get all neighbors
    std::vector<std::vector<int>> nbors(N);
    #pragma omp parallel for
    for (int u = 0; u < N; u++) {
        nbors[u] = graph.get_neighbors(u);
    }
    std::vector<int> recoloring_vertices(N,1);
    bool still_conflicting = true;
    while (still_conflicting) {
        still_conflicting = false;
        #pragma omp parallel
        {
            std::vector<bool> nbor_colors(N, false);
            #pragma omp for schedule(dynamic, 64)
            for (int u = 0; u < N; u++) {
                if (recoloring_vertices[u]) {
                    std::fill(nbor_colors.begin(), nbor_colors.end(), false);
                    for (int i = 0; i < (int)nbors[u].size(); i++) {
                        int v = nbors[u][i];
                        if (colors[v] != -1) {
                            nbor_colors[colors[v]] = true;
                        }
                    }
                    int color = 0;
                    while (nbor_colors[color]) {
                        color++;
                    }
                    colors[u] = color;
                }
            }
        }
        std::vector<int> recolor(N,0);
        #pragma omp parallel for schedule(dynamic,64) reduction(|:still_conflicting)
        for (int u = 0; u < N; u++) {
            for (int i = 0; i < (int)nbors[u].size(); i++) {
                int v = nbors[u][i];
                // detect conflicts
                if (u < v and colors[u] == colors[v] && colors[u] != -1) {
                    if (priorities[u] > priorities[v]) {
                        recolor[v] = 1;
                    } else {
                        recolor[u] = 1;
                    }
                    still_conflicting = true;
                }
            }
        }
        #pragma omp parallel for
        for (int u = 0; u < N; u++) {
            if (recolor[u]) {
                colors[u] = -1;
                recoloring_vertices[u] = 1;
            } else {
                recoloring_vertices[u] = 0;
            }
        }
    }
}

