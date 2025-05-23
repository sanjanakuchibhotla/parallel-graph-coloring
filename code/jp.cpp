// jones plassman parallel graph coloring algorithm 

#include "graph.h"
#include <omp.h>
#include <set>
#include <stdio.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>

//     // every uncolored vertex checks all neighbors and if it has highest priority, color vertex w min color, mark vertex as colored
//     // while ()
// Graph::Graph(int vertices): V(vertices), E(0), adj_list(vertices), colors(vertices, -1) {}
// sequential jp
void jones_plassmann_sequential(Graph& graph) {
    // randomly assign unique priority/weight to each vertex
    int N = graph.size();
    graph.assign_priorities();
    std::vector<int>& colors = graph.get_colors();
    std::vector<int>& priorities = graph.get_priorities();

    bool colored_vertex = true;
    while (colored_vertex) {
        colored_vertex = false;
        // get all vertices to color
        std::vector<bool> uncolored;
        uncolored.resize(N, false);
        for (int u = 0; u < N; u++) {
            if (colors[u] == -1) {
                bool flag = true; // flag for if the vertex is local max
                std::vector<int> neighbors = graph.get_neighbors(u);
                for (int i = 0; i < neighbors.size(); i++) {
                    int v = neighbors[i];
                    if (colors[v] == -1 && priorities[v] > priorities[u]) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    uncolored[u] = true;
                }
            }
        }

        // color the vertices
        for (int u = 0; u < graph.size(); u++) {
            if (uncolored[u]) {
                std::set<int> nbor_colors;
                std::vector<int> neighbors = graph.get_neighbors(u);
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
                colors[u] = color;
                colored_vertex = true;
            }
        }
    }
}


void jones_plassmann(Graph& graph) {
    int N = graph.size();
    auto& colors = graph.get_colors();
    graph.assign_priorities();
    auto& priorities = graph.get_priorities();
    
    // loop for coloring
    while (true) {
        bool flag = false; // flag for if anything was colored on this iteration
        #pragma omp parallel
        {
            std::vector<char> nbor_colors(N, 0);
            #pragma omp for schedule(dynamic,64) reduction(|:flag)
            for (int u = 0; u < N; u++) {
                if (colors[u] != -1) {
                    continue;
                }
                auto& nbors = graph.get_neighbors(u);
                bool max_to_color = true;
                for (int i = 0; i < (int)nbors.size(); i++) {
                    int v = nbors[i];
                    // check nbor not higher priority
                    if (colors[v] == -1 && priorities[v] > priorities[u]) {
                        max_to_color = false;
                        break;
                    }
                }
                // if current vertex is not the one to be colored
                if (!max_to_color) {
                    continue;
                }
                for (int v : nbors) {
                    // int v = nbors[i];
                    int v_color = colors[v];
                    if (v_color > -1) {
                        nbor_colors[v_color] = 1;
                    }
                }
                int color = 0;
                while (nbor_colors[color]) {
                    color++;
                }
                colors[u] = color;
                flag = true;
                for (int v : nbors) {
                    int v_color = colors[v];
                    if (v_color > -1) {
                        nbor_colors[v_color] = 0;
                    }
                }
            }
        }
        
        if (!flag) {
            break;
        }
    }
}