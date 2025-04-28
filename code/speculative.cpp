#include "graph.h"
#include <omp.h>
#include <set>

// color the graph speculatively, then use conflict resolution in order to 
// ensure that neighbors don't have the same color
void speculative_coloring(Graph& graph) {
    int N = graph.size();
    std::vector<int>& colors = graph.get_colors();
    std::vector<std::vector<int>> nbors(N);
    #pragma omp parallel for
    for (int u = 0; u < N; u++) {
        nbors[u] = graph.get_neighbors(u);
    }
    std::vector<int> vertices_to_recolor(N,1);
    bool still_conflicting = true;
    while (still_conflicting) {
        still_conflicting = false;
        // speculative coloring
        #pragma omp parallel for schedule(static)
        for (int u = 0; u < N; u++) {
            if (vertices_to_recolor[u]) {
                std::vector<bool> nbor_colors(N,false);
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

        // reset vertices to color
        std::fill(vertices_to_recolor.begin(), vertices_to_recolor.end(), 0);
        #pragma omp parallel for schedule(static)
        for (int u = 0; u < N; u++) {
            for (int i = 0; i < (int)nbors[u].size(); i++) {
                int v = nbors[u][i];
                // recoloring + conflict detection
                if (u < v && colors[u] == colors[v]) {
                    colors[v] = -1;
                    vertices_to_recolor[v] = 1;
                    #pragma omp atomic write
                    still_conflicting = true;
                }
            }
        }
    }
}
