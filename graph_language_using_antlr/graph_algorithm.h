#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H


#include <vector>
#include <queue>
#include <iostream>
#include <functional>
#include <string> // Add string for returning string values
#include <unordered_map>
#include <unordered_set>

// BFS (Breadth-First Search)
std::string BFS(const std::unordered_map<int, std::unordered_set<int>>& graph);

// DFS (Depth-First Search)
std::string DFS(const std::unordered_map<int, std::unordered_set<int>>& graph);

// Find Connected Components
std::string findConnectedComponents(const std::unordered_map<int, std::unordered_set<int>>& graph);

// Detect Cycle in Graph
std::string detectCycle(const std::unordered_map<int, std::unordered_set<int>>& graph);

// Find Articulation Points (Cut Vertices)
std::string findArticulationPoints(const std::unordered_map<int, std::unordered_set<int>>& graph);

// Find Bridges (Cut Edges)
std::string findBridges(const std::unordered_map<int, std::unordered_set<int>>& graph);
#endif // GRAPH_ALGORITHMS_H
