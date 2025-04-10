#include <iostream>
#include <vector>
#include <set>

class Graph {
private:
    int N;
    std::vector<std::vector<int> > neighbors;
    std::vector<int> colors;

public:
    Graph(int vertices) {
        N = vertices;
        neighbors.resize(vertices);
        colors.resize(vertices, -1);
    }

    void add_edge(int u, int v) {
        neighbors[u].push_back(v);
        neighbors[v].push_back(u);
    }

    void color_graph() {
        for (int u = 0; u < N; u++) {
            std::set<int> nbor_colors;

            for (int i = 0; i < neighbors[u].size(); i++) {
                int v = neighbors[u][i];
                if (colors[v] != -1)
                    nbor_colors.insert(colors[v]);
            }

            int color = 0;
            while (nbor_colors.count(color) > 0) {
                color++;
            }

            colors[u] = color;
        }
    }

    void print_graph_colors() const {
        for (int i = 0; i < N; i++) {
            std::cout << "vertex " << i << " is colored " << colors[i] << "\n";
        }
    }

    void print_graph() const {
        for (int u = 0; u < N; u++) {
            for (int i = 0; i < neighbors[u].size(); i++) {
                int v = neighbors[u][i];
                std::cout << "edge " << u << "-" << v << "\n";
            }
        }
    }
};

int main() {
    Graph graph(7);
    graph.add_edge(0,1);
    graph.add_edge(0,2);
    graph.add_edge(0,3);
    graph.add_edge(1,2);
    graph.add_edge(1,3);
    graph.add_edge(3,4);
    graph.add_edge(5,6);
    graph.color_graph();
    graph.print_graph();
    graph.print_graph_colors();

    return 0;
}
