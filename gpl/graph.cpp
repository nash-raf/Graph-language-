#include "graph.h"
#include <algorithm>
#include <optional>
#include <iostream>
#include <variant>

graph::graph() {}

void graph::setDirected(bool directed) {
    this->directed = directed;
}

void graph::setWeighted(bool weighted) {
    this->weighted = weighted;
}

void graph::setStochastic(bool stochastic) {
    this->stochastic = stochastic;
}

bool graph::isDirected() {
    return directed;
}

bool graph::isWeighted() {
    return weighted;
}

bool graph::isStochastic() {
    return stochastic;
}

void graph::addNode(node n) {
    nodes.push_back(n);
    adjacencyList[n] = std::vector<node>();
    nodeId[n] = n.getId();
    nodeIndex[n] = lastNodeIndex;
    lastNodeIndex++;
    if(adjacencyMatrixMode) {
        adjacencyMatrix.resize(nodes.size(), std::vector<NodeType>(nodes.size()));
    }
}

void graph::removeNode(node n) {
    if(nodeIndex.find(n) == nodeIndex.end()) {
        throw std::runtime_error("Node does not exist in the graph.");
        return;
    }
    nodes.erase(std::remove(nodes.begin(), nodes.end(), n), nodes.end());
    adjacencyList.erase(n);
    if(adjacencyMatrixMode) {
        adjacencyMatrix.erase(adjacencyMatrix.begin() + nodeIndex[n]);
        for(auto& row : adjacencyMatrix) {
            row.erase(row.begin() + nodeIndex[n]);
        }
    }
    nodeId.erase(n);
    nodeIndex.erase(n);
}

void graph::makeAdjacencyMatrix() {
    adjacencyMatrix.resize(nodes.size(), std::vector<NodeType>(nodes.size()));
    adjacencyMatrixMode = true;
    if(weighted) {
        for(const auto& e: edges) {
            adjacencyMatrix[nodeIndex[e.from]][nodeIndex[e.to]] = e.weight;
            if(!directed) {
                adjacencyMatrix[nodeIndex[e.to]][nodeIndex[e.from]] = e.weight;
            }
        }
    }
    else {
        for(const auto& e: edges) {
            adjacencyMatrix[nodeIndex[e.from]][nodeIndex[e.to]] = 1;
            if(!directed) {
                adjacencyMatrix[nodeIndex[e.to]][nodeIndex[e.from]] = 1;
            }
        }
    }
    
}

void graph::removeAdjacencyMatrix() {
    adjacencyMatrix.clear();
    adjacencyMatrixMode = false;
}

bool graph::ensureStochastic() {
    if(!stochastic && !weighted) {
        return false;
    }
    bool result = true;
    std::visit([this, &result](auto&& value) -> double
    {
        if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int>
        ||
        std::is_same_v<std::decay_t<decltype(value)>, double>) {
            for(const auto& row : adjacencyMatrix) {
                double sum = 0.0;
                for(const auto& weight : row) {
                    double w = std::visit([](auto&& wvalue) -> double 
                    {
                        if constexpr (std::is_same_v<std::decay_t<decltype(wvalue)>, int>
                        ||
                        std::is_same_v<std::decay_t<decltype(wvalue)>, double>) {
                            return static_cast<double>(wvalue);
                        }
                        else {
                            return 0.0;
                        }
                    }, weight);
                    sum += w;
                }
                if(sum != 1) {
                    stochastic = false;
                    result = false;
                }
            }
            return 0.0;
        }
        else {
            stochastic = false;
            result = false;
            return 0.0;
        }
    }, weight);
    return result;
}

void graph::addEdge(edge e) {
    edges.push_back(e);
    adjacencyList[e.from].push_back(e.to);
    if(!directed) {
        adjacencyList[e.to].push_back(e.from);
    }
    if(adjacencyMatrixMode) {
        if(weighted) {
            adjacencyMatrix[nodeIndex[e.from]][nodeIndex[e.to]] = e.weight;
            if(!directed) {
                adjacencyMatrix[nodeIndex[e.to]][nodeIndex[e.from]] = e.weight;
            }
        } else {
            adjacencyMatrix[nodeIndex[e.from]][nodeIndex[e.to]] = 1;
            if(!directed) {
                adjacencyMatrix[nodeIndex[e.to]][nodeIndex[e.from]] = 1;
            }
        }
    }
}

void graph::removeEdge(edge e) {
    edges.erase(std::remove(edges.begin(), edges.end(), e), edges.end());
    adjacencyList[e.from].erase(std::remove(adjacencyList[e.from].begin(), adjacencyList[e.from].end(), e.to), adjacencyList[e.from].end());
    if(adjacencyMatrixMode) {
        adjacencyMatrix[nodeIndex[e.from]][nodeIndex[e.to]] = 0;
        adjacencyMatrix[nodeIndex[e.to]][nodeIndex[e.from]] = 0;
    }
}

EdgeType graph::weightLookup(node from, node to) {
    if(adjacencyMatrixMode) {
        return adjacencyMatrix[nodeIndex[from]][nodeIndex[to]];

    }
    else {
        for(const auto& e : edges) {
            if(e.from.id == from.id && e.to.id == to.id) {
                return e.weight;
            }
        }
        throw std::runtime_error("Edge does not exist in the graph.");
    }
}

void graph::printNodes() {
    for (const auto& n : nodes) {
        std::visit([](auto&& value) { std::cout << value << " "; }, n.id);
    }
}

void graph::printEdges() {
    for (const auto& e : edges) {
        std::visit([](auto&& value) { std::cout << value << " "; }, e.from.id);
        std::cout << " -> ";
        std::visit([](auto&& value) { std::cout << value << " "; }, e.to.id);
    }
}

void graph::printGraph() {
    std::cout << "Nodes:" << std::endl;
    printNodes();
    std::cout << "Edges:" << std::endl;
    printEdges();
}
