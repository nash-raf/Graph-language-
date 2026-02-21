#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <chrono>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <edgelist.txt> [source=2]\n", argv[0]);
        return 1;
    }

    int source = (argc >= 3) ? atoi(argv[2]) : 2;

    // Read edge list
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("fopen"); return 1; }

    int u, v, max_node = -1;
    std::vector<std::pair<int,int>> edges;
    while (fscanf(f, "%d %d", &u, &v) == 2) {
        edges.push_back({u, v});
        if (u > max_node) max_node = u;
        if (v > max_node) max_node = v;
    }
    fclose(f);

    int n = max_node + 1;

    // Build adjacency list (undirected)
    std::vector<std::vector<int>> adj(n);
    for (auto &[a, b] : edges) {
        adj[a].push_back(b);
        adj[b].push_back(a);
    }

    // BFS -- matches DSL logic exactly (set-based frontier)
    std::vector<int> dist(n, 0);
    auto t0 = std::chrono::high_resolution_clock::now();

    std::unordered_set<int> frontier = {source};
    std::unordered_set<int> visited;
    int level = 0;

    while (frontier.size() > 0) {
        std::unordered_set<int> next_frontier;
        for (int v : frontier) {
            dist[v] = level;
            visited.insert(v);
            for (int u : adj[v]) {
                if (visited.find(u) == visited.end()) {
                    next_frontier.insert(u);
                }
            }
        }
        frontier = std::move(next_frontier);
        level++;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(t1 - t0).count();

    printf("dist[1] = %d\n", dist[1]);
    printf("BFS time: %.6f seconds\n", elapsed);
    printf("Vertices: %d, Edges: %zu\n", n, edges.size());

    return 0;
}
