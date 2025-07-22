#include "algorithms.h"
using namespace std;

// BFS Algorithm (Unweighted graph)
string BFS(const unordered_map<int, unordered_set<int>>& graph) {
    unordered_map<int, bool> visited;
    queue<int> q;
    string result;

    if (graph.empty()) return "Graph is empty.";

    int start = graph.begin()->first; // Start from the first node
    visited[start] = true;
    q.push(start);

    while (!q.empty()) {
        int node = q.front();
        q.pop();
        result += to_string(node) + " "; // Accumulate visited nodes in the result

        for (const int& neighbor : graph.at(node)) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }

    return result;
}

// DFS Algorithm (Recursive)
string DFS(int node, const unordered_map<int, unordered_set<int>>& graph, unordered_set<int>& visited) {
    visited.insert(node);
    string result = to_string(node) + " ";

    for (const int& neighbor : graph.at(node)) {
        if (visited.find(neighbor) == visited.end()) {
            result += DFS(neighbor, graph, visited); // Accumulate DFS traversal
        }
    }

    return result;
}

// Find Connected Components
string findConnectedComponents(const unordered_map<int, unordered_set<int>>& graph) {
    unordered_set<int> visited;
    string result;

    for (const auto& pair : graph) {
        if (visited.find(pair.first) == visited.end()) {
            result += DFS(pair.first, graph, visited) + "\n"; // Find connected component and add it to result
        }
    }

    return result;
}

// Cycle Detection (DFS-based)
string detectCycle(int node, unordered_map<int, unordered_set<int>>& graph, unordered_set<int>& visited, unordered_set<int>& inRecursionStack) {
    string result;
    
    if (inRecursionStack.find(node) != inRecursionStack.end()) {
        return "Cycle detected!";
    }
    if (visited.find(node) != visited.end()) {
        return "";
    }

    visited.insert(node);
    inRecursionStack.insert(node);

    for (const int& neighbor : graph[node]) {
        result += detectCycle(neighbor, graph, visited, inRecursionStack);
        if (!result.empty()) return result;
    }

    inRecursionStack.erase(node);
    return "";
}

// Articulation Points (Cut Vertices)
string findArticulationPoints(const unordered_map<int, unordered_set<int>>& graph) {
    unordered_map<int, int> discoveryTime;
    unordered_map<int, int> lowTime;
    unordered_set<int> visited;
    unordered_set<int> articulationPoints;
    int time = 0;
    string result;

    function<void(int, int)> dfs = [&](int node, int parent) {
        visited.insert(node);
        discoveryTime[node] = lowTime[node] = time++;
        int children = 0;

        for (const int& neighbor : graph.at(node)) {
            if (visited.find(neighbor) == visited.end()) {
                children++;
                dfs(neighbor, node);
                lowTime[node] = min(lowTime[node], lowTime[neighbor]);

                if (lowTime[neighbor] >= discoveryTime[node] && parent != -1) {
                    articulationPoints.insert(node);
                }
            } else if (neighbor != parent) {
                lowTime[node] = min(lowTime[node], discoveryTime[neighbor]);
            }
        }

        if (parent == -1 && children > 1) {
            articulationPoints.insert(node);
        }
    };

    for (const auto& pair : graph) {
        if (visited.find(pair.first) == visited.end()) {
            dfs(pair.first, -1);
        }
    }

    for (const int& point : articulationPoints) {
        result += to_string(point) + " is an articulation point\n";
    }

    return result;
}

// Bridges (Cut Edges)
struct pair_hash {
    template <typename T1, typename T2>
    size_t operator()(const pair<T1, T2>& p) const {
        auto h1 = hash<T1>{}(p.first);
        auto h2 = hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

string findBridges(const unordered_map<int, unordered_set<int>>& graph) {
    unordered_map<int, int> discoveryTime;
    unordered_map<int, int> lowTime;
    unordered_set<int> visited;
    unordered_set<pair<int, int>, pair_hash> bridges;
    int time = 0;
    string result;

    function<void(int, int)> dfs = [&](int node, int parent) {
        visited.insert(node);
        discoveryTime[node] = lowTime[node] = time++;

        for (const int& neighbor : graph.at(node)) {
            if (visited.find(neighbor) == visited.end()) {
                dfs(neighbor, node);
                lowTime[node] = min(lowTime[node], lowTime[neighbor]);

                if (lowTime[neighbor] > discoveryTime[node]) {
                    bridges.insert({min(node, neighbor), max(node, neighbor)});
                }
            } else if (neighbor != parent) {
                lowTime[node] = min(lowTime[node], discoveryTime[neighbor]);
            }
        }
    };

    for (const auto& pair : graph) {
        if (visited.find(pair.first) == visited.end()) {
            dfs(pair.first, -1);
        }
    }

    for (const auto& bridge : bridges) {
        result += to_string(bridge.first) + " - " + to_string(bridge.second) + " is a bridge\n";
    }

    return result;
}
