#include "graph.h"
#include <omp.h>
#include <set>


// SEQUENTIAL ALGORITHM (GREEDY GRAPH COLORING)
// this is our baseline that we compare our parallel algorithms to
/* 
greedy approach colors each vertex with the minimum possible color that no
neighbor vertex has used yet
*/
void greedy_color(Graph& graph) {
    int N = graph.size();
    std::vector<int>& colors = graph.get_colors();
    for (int u = 0; u < N; u++) {
        std::set<int> nbor_colors; // store all colors of neighbors in a set
        std::vector<int>& neighbors = graph.get_neighbors(u);
        // populate neighbors colors:
        for (int i = 0; i < neighbors.size(); i++) {
            int v = neighbors[i];
            if (colors[v] != -1) {
                nbor_colors.insert(colors[v]);
            }
        }

        int color = 0;
        
        while (nbor_colors.count(color) > 0) { // if color is in the set
            color++;
        }
        // printf("coloring vertex %d with color %d\n", u, color);
        // for (int i = 0; i < neighbors.size(); i++) {
        //     int v = neighbors[i];
        //     printf("  edge between %d and %d\n", u, v);
        // }
        colors[u] = color;
    }
}
