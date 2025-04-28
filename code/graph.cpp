#include "graph.h"
#include <set>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>

Graph::Graph(int vertices): V(vertices), E(0), adj_list(vertices), colors(vertices, -1), priorities(vertices, -1) {}

void Graph::add_edge(int u, int v) {
    adj_list[u].push_back(v);
    adj_list[v].push_back(u);
    E++;
}

std::vector<int>& Graph::get_neighbors(int u) {
    return adj_list[u];
}

std::vector<int>& Graph::get_colors() {
    return colors;
}

std::vector<int>& Graph::get_priorities() {
    return priorities;
}

bool Graph::check_coloring() {
    for (int u = 0; u < V; u++) {
        std::vector<int>& neighbors = adj_list[u];
        for (int i = 0; i < (int)neighbors.size(); i++) {
            int v = neighbors[i];
            if (colors[u] == colors[v]) {
                return false;
            }
        }
    }
    return true;
}

void Graph::reset_colors() {
    std::fill(colors.begin(), colors.end(), -1);
}

void Graph::assign_priorities() {
    std::vector<int> shuffled_priorities(V);
    std::iota(shuffled_priorities.begin(), shuffled_priorities.end(), 0);
    std::mt19937_64 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::shuffle(shuffled_priorities.begin(), shuffled_priorities.end(), rng);
    for (int u = 0; u < V; u++) {
        priorities[u] = shuffled_priorities[u];
    }
}

int Graph::count_colors() {
    std::set<int> all_colors;
    for (size_t i = 0; i < colors.size(); i++) {
        int color = colors[i];
        if (color != -1) {
            all_colors.insert(color);
        }
    }
    return all_colors.size();
}

int Graph::size() {
    return V;
}

int Graph::num_edges() {
    return E;
}

void Graph::print_graph() {
    printf("Number of Vertices: %d\n", V);
    printf("Number of Edges: %d\n", E);
}