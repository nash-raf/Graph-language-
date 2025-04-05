#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <optional>
#include "edge.h"

class graph : public edge{
    public:
        bool directed = false;
        bool weighted = false;
        bool stochastic = false;
        bool adjacencyMatrixMode = false;
        std::vector<node> nodes;
        std::vector<edge> edges;
        std::unordered_map<node, NodeType> nodeId;
        std::unordered_map<node, int> nodeIndex;
        std::unordered_map<node, std::vector<node>> adjacencyList; //adj list with nodes
        std::vector<std::vector<NodeType>> adjacencyMatrix; //adj matrix with nodes
        graph();
        void makeAdjacencyMatrix();
        void setDirected(bool directed);
        void setWeighted(bool weighted);
        void setStochastic(bool stochastic);
        bool isDirected();
        bool isWeighted();
        bool isStochastic();
        void addNode(node n);
        void removeNode(node n);
        void addEdge(edge e);
        void removeEdge(edge e);
        void printNodes();
        void printEdges();
        void printGraph();
        void removeAdjacencyMatrix();
        bool ensureStochastic();
        EdgeType weightLookup(node from, node to);
    
    private:
        int lastNodeIndex = 0;
};
