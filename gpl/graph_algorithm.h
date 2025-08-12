#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H

#include "graph.h"
#include <vector>
#include <queue>
#include <iostream>
#include <functional>
#include <string> // Add string for returning string values
#include <unordered_map>
#include <unordered_set>

// BFS (Breadth-First Search)
std::string BFS(const graph& graph);

// DFS (Depth-First Search)
std::string DFS(const graph& graph);

// Find Connected Components
std::string findConnectedComponents(const graph& graph);

// Detect Cycle in Graph
//std::string detectCycle(graph& graph);

// Find Articulation Points (Cut Vertices)
//std::string findArticulationPoints(graph& graph);

// Find Bridges (Cut Edges)
//std::string findBridges(graph& graph);
#endif // GRAPH_ALGORITHMS_H
