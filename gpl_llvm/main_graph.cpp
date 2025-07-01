#include "def.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>

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

    EBBkC_Graph_t G(v_size, e_size, false, false, false);
    G.buildGraph(edges);

    // BFS from node 0
    std::vector<bool> visited(v_size, false);
    std::queue<int> q;

    // for (const auto& e : edges) {
    // std::cout << "Edge: " << e.s << " " << e.t << std::endl;
    // }

    std::cout << "BFS traversal from node 0:\n";

    visited[0] = true;
    q.push(0);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        std::cout << u << " ";

        for (int i = G.cd[u]; i < G.cd[u + 1]; ++i) {
            int neighbor = G.adj[i];
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }

    
    return 0;
}

