#include "graph_algorithm.h"
#include "utilities.h"

// BFS (Breadth-First Search)
std::string BFS(const graph& graph) {
    std::string result;
    std::unordered_set<node> visited;
    for (const auto& Node : graph.adjacencyList) {
        if (visited.count(Node.first)) continue;
        std::queue<node> q;
        q.push(Node.first);
        visited.insert(Node.first);

        while (!q.empty()) {
            node current = q.front();
            q.pop();
            result += utilities::nodeTypeToString(current.id) + " ";

            for (node neighbor : graph.adjacencyList.at(current)) {
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
void DFSHelper(node Node, const graph& graph, 
               std::unordered_set<node>& visited, std::string& result) {
    visited.insert(Node);
    result += utilities::nodeTypeToString(Node.id) + " ";

    for (node neighbor : graph.adjacencyList.at(Node)) {
        if (!visited.count(neighbor)) {
            DFSHelper(neighbor, graph, visited, result);
        }
    }
}

std::string DFS(const graph& graph) {
    std::string result;
    std::unordered_set<node> visited;

    for (const auto& Node : graph.adjacencyList) {
        if (!visited.count(Node.first)) {
            DFSHelper(Node.first, graph, visited, result);
        }
    }
    return result;
}

// Find Connected Components
void connectedComponentsHelper(node Node, const graph& graph, 
                               std::unordered_set<node>& visited, std::string& component) {
    visited.insert(Node);
    component += utilities::nodeTypeToString(Node.id) + " ";

    for (node neighbor : graph.adjacencyList.at(Node)) {
        if (!visited.count(neighbor)) {
            connectedComponentsHelper(neighbor, graph, visited, component);
        }
    }
}

std::string findConnectedComponents(const graph& graph) {
    std::unordered_set<node> visited;
    std::string result;

    for (const auto& node : graph.adjacencyList) {
        if (!visited.count(node.first)) {
            std::string component;
            connectedComponentsHelper(node.first, graph, visited, component);
            result += "{ " + component + "} ";
        }
    }
    return result;
}

// Detect Cycle in Graph
bool detectCycleHelper(node Node, node parent, const graph& graph, 
                       std::unordered_set<node>& visited) {
    visited.insert(Node);

    for (node neighbor : graph.adjacencyList.at(Node)) {
        if (!visited.count(neighbor)) {
            if (detectCycleHelper(neighbor, Node, graph, visited)) {
                return true;
            }
        } else if (neighbor != parent) {
            return true;
        }
    }
    return false;
}

// std::string detectCycle(const graph& graph) {
//     std::unordered_set<node> visited;
//     for (const auto& node : graph.adjacencyList) {
//         if (!visited.count(node.first)) {
//             if (detectCycleHelper(node.first, -1, graph, visited)) {
//                 return "Cycle Detected";
//             }
//         }
//     }
//     return "No Cycle Detected";
// }

// // Find Articulation Points (Cut Vertices)
// void articulationPointsHelper(int node, int parent, int& time, 
//                               const std::unordered_map<int, std::unordered_set<int>>& graph, 
//                               std::unordered_set<int>& visited, std::unordered_map<int, int>& disc, 
//                               std::unordered_map<int, int>& low, std::unordered_set<int>& articulationPoints) {
//     visited.insert(node);
//     disc[node] = low[node] = ++time;
//     int children = 0;

//     for (int neighbor : graph.at(node)) {
//         if (!visited.count(neighbor)) {
//             children++;
//             articulationPointsHelper(neighbor, node, time, graph, visited, disc, low, articulationPoints);

//             low[node] = std::min(low[node], low[neighbor]);

//             if (parent != -1 && low[neighbor] >= disc[node]) {
//                 articulationPoints.insert(node);
//             }

//             if (parent == -1 && children > 1) {
//                 articulationPoints.insert(node);
//             }
//         } else if (neighbor != parent) {
//             low[node] = std::min(low[node], disc[neighbor]);
//         }
//     }
// }

// std::string findArticulationPoints(const std::unordered_map<int, std::unordered_set<int>>& graph) {
//     std::unordered_set<int> visited;
//     std::unordered_map<int, int> disc, low;
//     std::unordered_set<int> articulationPoints;
//     int time = 0;

//     for (const auto& node : graph) {
//         if (!visited.count(node.first)) {
//             articulationPointsHelper(node.first, -1, time, graph, visited, disc, low, articulationPoints);
//         }
//     }

//     std::string result;
//     for (int point : articulationPoints) {
//         result += std::to_string(point) + " ";
//     }
//     return result;
// }

// // Find Bridges (Cut Edges)
// void bridgesHelper(int node, int parent, int& time, 
//                    const std::unordered_map<int, std::unordered_set<int>>& graph, 
//                    std::unordered_set<int>& visited, std::unordered_map<int, int>& disc, 
//                    std::unordered_map<int, int>& low, std::vector<std::pair<int, int>>& bridges) {
//     visited.insert(node);
//     disc[node] = low[node] = ++time;

//     for (int neighbor : graph.at(node)) {
//         if (!visited.count(neighbor)) {
//             bridgesHelper(neighbor, node, time, graph, visited, disc, low, bridges);

//             low[node] = std::min(low[node], low[neighbor]);

//             if (low[neighbor] > disc[node]) {
//                 bridges.emplace_back(node, neighbor);
//             }
//         } else if (neighbor != parent) {
//             low[node] = std::min(low[node], disc[neighbor]);
//         }
//     }
// }

// std::string findBridges(const std::unordered_map<int, std::unordered_set<int>>& graph) {
//     std::unordered_set<int> visited;
//     std::unordered_map<int, int> disc, low;
//     std::vector<std::pair<int, int>> bridges;
//     int time = 0;

//     for (const auto& node : graph) {
//         if (!visited.count(node.first)) {
//             bridgesHelper(node.first, -1, time, graph, visited, disc, low, bridges);
//         }
//     }

//     std::string result;
//     for (const auto& bridge : bridges) {
//         result += "(" + std::to_string(bridge.first) + ", " + std::to_string(bridge.second) + ") ";
//     }
//     return result;
// }
