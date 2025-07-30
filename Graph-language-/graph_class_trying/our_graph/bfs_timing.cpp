#include "graph.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>  // for timing

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./graph_program g.txt\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open input file\n";
        return 1;
    }

    std::vector<Edge_t> edges;
    int maxNodeId = 0;
    int u, v;

    while (infile >> u >> v) {
        edges.emplace_back(u, v, false); // false = undirected
        maxNodeId = std::max({maxNodeId, u, v});
    }

    int v_size = maxNodeId + 1;
    int e_size = edges.size();

    Graph G(v_size, e_size, false, false, false);
    G.buildGraph(edges);

    std::vector<bool> visited(v_size, false);
    std::queue<int> q;

    std::cout << "Starting BFS from node 1...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    visited[1] = true;
    q.push(1);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int i = G.cd[u]; i < G.cd[u + 1]; ++i) {
            int neighbor = G.adj[i];
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    std::cout << "BFS completed in " << elapsed_ms << " ms\n";

    return 0;
}
