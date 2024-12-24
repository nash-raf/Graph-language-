#include "algorithms.h"
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>  // For measuring execution time

using namespace std;
using namespace chrono;

// Load graph from .mtx file
unordered_map<int, unordered_set<int>> loadGraphFromMTX(const string& filename) {
    unordered_map<int, unordered_set<int>> graph;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }

    string line;
    bool headerSkipped = false;

    while (getline(file, line)) {
        // Skip comments and metadata
        if (line[0] == '%' || line.empty()) {
            continue;
        }

        // Skip the header (matrix size line)
        if (!headerSkipped) {
            headerSkipped = true;
            continue;
        }

        // Parse edge (Node1 Node2)
        istringstream iss(line);
        int node1, node2;
        if (!(iss >> node1 >> node2)) {
            cerr << "Error: Invalid line format: " << line << endl;
            continue;
        }

        // Build adjacency list (undirected graph)
        graph[node1].insert(node2);
        graph[node2].insert(node1);
    }

    file.close();
    return graph;
}

int main() {
    string filename = "144.mtx"; // Replace with your .mtx file name
    unordered_map<int, unordered_set<int>> graph = loadGraphFromMTX(filename);

    cout << "Graph loaded successfully!" << endl;
    cout << "Number of Nodes: " << graph.size() << endl;

    // Measure time taken for BFS
    auto start = high_resolution_clock::now(); // Start time

    cout << "Starting BFS from the first node..." << endl;
    if (!graph.empty()) {
        // Capture and print the output of BFS
        cout << BFS(graph) << endl;
    } else {
        cout << "Graph is empty. Please check the input file." << endl;
    }

    auto stop = high_resolution_clock::now(); // End time
    auto duration = duration_cast<milliseconds>(stop - start); // Duration in milliseconds

    cout << "BFS completed in " << duration.count() << " milliseconds." << endl;

    return 0;
}
