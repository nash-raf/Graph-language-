#include <iostream>
#include <vector>
#include <fstream>
#include <climits>
#include <cstdlib>
#include <ctime>

using namespace std;

// Function to perform Stoer-Wagner's Min-Cut algorithm
int stoerWagnerMinCut(vector<vector<int>>& adj, int V) {
    int min_cut = INT_MAX;

    // We perform the contraction process V-1 times
    for (int phase = 0; phase < V - 1; phase++) {
        // Create a vector to store the weights of the vertices as we grow the "s" side of the cut
        vector<int> A(V, 0);  // A represents the set of vertices on the s side of the cut
        vector<int> W(V, 0);  // W will store the weights of the vertices as we move through the process
        
        int s = 0;  // The starting vertex of the phase (this is s side in the cut)
        int t = -1; // t will be the vertex on the other side of the cut (t side in the cut)

        // Greedily add vertices to the A set
        for (int i = 0; i < V; i++) {
            int max_weight = -1;
            int next_vertex = -1;

            // Find the vertex that has the maximum weight to be added to set A
            for (int j = 0; j < V; j++) {
                if (!A[j] && (next_vertex == -1 || W[j] > max_weight)) {
                    next_vertex = j;
                    max_weight = W[j];
                }
            }

            // Mark the vertex as added to A
            A[next_vertex] = 1;

            // If this is the second-to-last vertex, we found the cut
            if (t != -1) {
                min_cut = min(min_cut, W[next_vertex]);
            }

            // Update the weight of the adjacent vertices
            for (int j = 0; j < V; j++) {
                if (!A[j] && adj[next_vertex][j] > 0) {
                    W[j] += adj[next_vertex][j];
                }
            }
            t = next_vertex;
        }

        // Now we contract the edge between s and t
        for (int i = 0; i < V; i++) {
            if (i != s && i != t) {
                adj[s][i] += adj[t][i];  // Add t's edges to s
                adj[i][s] += adj[i][t];  // Add t's edges to s (undirected graph)
            }
        }

        // Remove vertex t by setting its row and column to -1 (or 0, indicating it no longer exists)
        for (int i = 0; i < V; i++) {
            adj[i][t] = adj[t][i] = -1;
        }

        // Decrease the number of vertices
        V--;
    }

    return min_cut;
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

    // First, we read the edge list and determine the number of vertices
    while (file >> u >> v) {
        edges.push_back({u, v});
        V = max(V, max(u, v) + 1);  // Update number of vertices
    }

    file.close();

    // Convert the edge list to an adjacency matrix
    vector<vector<int>> adj(V, vector<int>(V, 0));
    for (auto& edge : edges) {
        adj[edge.first][edge.second]++;
        adj[edge.second][edge.first]++;  // Since the graph is undirected
    }

    // Call Stoer-Wagner's Min-Cut algorithm
    int min_cut = stoerWagnerMinCut(adj, V);

    // Output the result
    cout << "Deterministic Minimum Cut: " << min_cut << endl;

    return 0;
}
