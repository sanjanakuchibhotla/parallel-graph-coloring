#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <iostream>

class Graph {
public:
    Graph(int vertices);
    // function to add an undirected edge u-v to the graph
    void add_edge(int u, int v);
    // function to return the list of neighbors of u
    std::vector<int>& get_neighbors(int u);
    std::vector<int>& get_colors();
    std::vector<int>& get_priorities();
    int count_colors();
    void assign_priorities();

    bool check_coloring();
    int size(); // return size of graph (num of vertices)
    int num_edges();

    // print the graph
    void print_graph();
private:
    int V; // number of vertices
    int E; // number of edges
    std::vector<std::vector<int> > adj_list; // graph rep (adj list)
    std::vector<int> colors;
    std::vector<int> priorities;
};

#endif