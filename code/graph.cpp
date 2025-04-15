#include "graph.h"
#include <set>

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
        for (int i = 0; i < neighbors.size(); i++) {
            int v = neighbors[i];
            if (colors[u] == colors[v]) {
                return false;
            }
        }
    }
    return true;
}

void Graph::assign_priorities() {
    std::set<int> used_priorities;
    for (int u = 0; u < V; u++) {
        int rand_n = rand();
        while (used_priorities.count(rand_n) > 0) {
            rand_n = rand();
        }
        used_priorities.insert(rand_n);
        priorities[u] = rand_n;
        printf("rand number generated: %d\n", rand_n);
    }
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