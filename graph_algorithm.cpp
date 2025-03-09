#include "graph_algorithm.h"

// BFS (Breadth-First Search)
std::string BFS(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    std::string result;
    std::unordered_set<int> visited;
    for (const auto& node : graph) {
        if (visited.count(node.first)) continue;
        std::queue<int> q;
        q.push(node.first);
        visited.insert(node.first);

        while (!q.empty()) {
            int current = q.front();
            q.pop();
            result += std::to_string(current) + " ";

            for (int neighbor : graph.at(current)) {
                if (!visited.count(neighbor)) {
                    q.push(neighbor);
                    visited.insert(neighbor);
                }
            }
        }
    }
    return result;
}

// DFS (Depth-First Search)
void DFSHelper(int node, const std::unordered_map<int, std::unordered_set<int>>& graph, 
               std::unordered_set<int>& visited, std::string& result) {
    visited.insert(node);
    result += std::to_string(node) + " ";

    for (int neighbor : graph.at(node)) {
        if (!visited.count(neighbor)) {
            DFSHelper(neighbor, graph, visited, result);
        }
    }
}

std::string DFS(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    std::string result;
    std::unordered_set<int> visited;

    for (const auto& node : graph) {
        if (!visited.count(node.first)) {
            DFSHelper(node.first, graph, visited, result);
        }
    }
    return result;
}

// Find Connected Components
void connectedComponentsHelper(int node, const std::unordered_map<int, std::unordered_set<int>>& graph, 
                               std::unordered_set<int>& visited, std::string& component) {
    visited.insert(node);
    component += std::to_string(node) + " ";

    for (int neighbor : graph.at(node)) {
        if (!visited.count(neighbor)) {
            connectedComponentsHelper(neighbor, graph, visited, component);
        }
    }
}

std::string findConnectedComponents(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    std::unordered_set<int> visited;
    std::string result;

    for (const auto& node : graph) {
        if (!visited.count(node.first)) {
            std::string component;
            connectedComponentsHelper(node.first, graph, visited, component);
            result += "{ " + component + "} ";
        }
    }
    return result;
}

// Detect Cycle in Graph
bool detectCycleHelper(int node, int parent, const std::unordered_map<int, std::unordered_set<int>>& graph, 
                       std::unordered_set<int>& visited) {
    visited.insert(node);

    for (int neighbor : graph.at(node)) {
        if (!visited.count(neighbor)) {
            if (detectCycleHelper(neighbor, node, graph, visited)) {
                return true;
            }
        } else if (neighbor != parent) {
            return true;
        }
    }
    return false;
}

std::string detectCycle(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    std::unordered_set<int> visited;
    for (const auto& node : graph) {
        if (!visited.count(node.first)) {
            if (detectCycleHelper(node.first, -1, graph, visited)) {
                return "Cycle Detected";
            }
        }
    }
    return "No Cycle Detected";
}

// Find Articulation Points (Cut Vertices)
void articulationPointsHelper(int node, int parent, int& time, 
                              const std::unordered_map<int, std::unordered_set<int>>& graph, 
                              std::unordered_set<int>& visited, std::unordered_map<int, int>& disc, 
                              std::unordered_map<int, int>& low, std::unordered_set<int>& articulationPoints) {
    visited.insert(node);
    disc[node] = low[node] = ++time;
    int children = 0;

    for (int neighbor : graph.at(node)) {
        if (!visited.count(neighbor)) {
            children++;
            articulationPointsHelper(neighbor, node, time, graph, visited, disc, low, articulationPoints);

            low[node] = std::min(low[node], low[neighbor]);

            if (parent != -1 && low[neighbor] >= disc[node]) {
                articulationPoints.insert(node);
            }

            if (parent == -1 && children > 1) {
                articulationPoints.insert(node);
            }
        } else if (neighbor != parent) {
            low[node] = std::min(low[node], disc[neighbor]);
        }
    }
}

std::string findArticulationPoints(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    std::unordered_set<int> visited;
    std::unordered_map<int, int> disc, low;
    std::unordered_set<int> articulationPoints;
    int time = 0;

    for (const auto& node : graph) {
        if (!visited.count(node.first)) {
            articulationPointsHelper(node.first, -1, time, graph, visited, disc, low, articulationPoints);
        }
    }

    std::string result;
    for (int point : articulationPoints) {
        result += std::to_string(point) + " ";
    }
    return result;
}

// Find Bridges (Cut Edges)
void bridgesHelper(int node, int parent, int& time, 
                   const std::unordered_map<int, std::unordered_set<int>>& graph, 
                   std::unordered_set<int>& visited, std::unordered_map<int, int>& disc, 
                   std::unordered_map<int, int>& low, std::vector<std::pair<int, int>>& bridges) {
    visited.insert(node);
    disc[node] = low[node] = ++time;

    for (int neighbor : graph.at(node)) {
        if (!visited.count(neighbor)) {
            bridgesHelper(neighbor, node, time, graph, visited, disc, low, bridges);

            low[node] = std::min(low[node], low[neighbor]);

            if (low[neighbor] > disc[node]) {
                bridges.emplace_back(node, neighbor);
            }
        } else if (neighbor != parent) {
            low[node] = std::min(low[node], disc[neighbor]);
        }
    }
}

std::string findBridges(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    std::unordered_set<int> visited;
    std::unordered_map<int, int> disc, low;
    std::vector<std::pair<int, int>> bridges;
    int time = 0;

    for (const auto& node : graph) {
        if (!visited.count(node.first)) {
            bridgesHelper(node.first, -1, time, graph, visited, disc, low, bridges);
        }
    }

    std::string result;
    for (const auto& bridge : bridges) {
        result += "(" + std::to_string(bridge.first) + ", " + std::to_string(bridge.second) + ") ";
    }
    return result;
}
