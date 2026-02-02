#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <climits>

using namespace std;

// Union-Find structure
class UnionFind {
public:
    vector<int> parent, size;

    UnionFind(int n) {
        parent.resize(n);
        size.resize(n, 1);
        for (int i = 0; i < n; i++) {
            parent[i] = i;
        }
    }

    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);  // Path compression
        }
        return parent[x];
    }

    void union_sets(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);

        if (rootX != rootY) {
            // Union by size (attach smaller tree to larger one)
            if (size[rootX] < size[rootY]) {
                swap(rootX, rootY);
            }
            parent[rootY] = rootX;
            size[rootX] += size[rootY];
        }
    }
};

// Function to perform Karger's Min-Cut Algorithm
int minCut(vector<pair<int, int>>& edges, int V) {
    // Initialize Union-Find
    UnionFind uf(V);

    // Randomly contract edges until only 2 vertices remain
    while (V > 2) {
        // Randomly pick an edge
        int idx = rand() % edges.size();
        int u = edges[idx].first;
        int v = edges[idx].second;

        int set_u = uf.find(u);
        int set_v = uf.find(v);

        // If u and v are in different sets, contract them
        if (set_u != set_v) {
            uf.union_sets(set_u, set_v);
            V--;  // Reduce the number of vertices
        }

        // Remove the contracted edge from the list of edges
        edges.erase(edges.begin() + idx);
    }

    // Count the number of edges between the two sets left
    int cutSize = 0;
    for (auto& edge : edges) {
        int u = edge.first;
        int v = edge.second;
        if (uf.find(u) != uf.find(v)) {
            cutSize++;
        }
    }

    return cutSize;
}

int main() {
    srand(time(0));  // Seed for random number generation

    // Read the edge list from the file
    ifstream file("edgelist.txt");
    if (!file) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    vector<pair<int, int>> edges;
    int V = 0;  // Number of vertices
    int u, v;

    // Read the edges from the file
    while (file >> u >> v) {
        edges.push_back({u, v});
        V = max(V, max(u, v) + 1);  // Update number of vertices
    }

    file.close();

    // Run Karger's Min-Cut Algorithm multiple times and take the minimum result
    int min_cut = INT_MAX;
    for (int i = 0; i < 100; ++i) {  // Run 100 iterations for a better result
        vector<pair<int, int>> edges_copy = edges;  // Copy of the edge list
        min_cut = min(min_cut, minCut(edges_copy, V));
    }

    cout << "Estimated Minimum Cut: " << min_cut << endl;

    return 0;
}
