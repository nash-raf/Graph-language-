#pragma once

#include <iostream>
#include <vector>
#include "node.h"

using EdgeType = std::variant<int, double, char, bool, std::string>;

class edge : public node{
    public:
        EdgeType weight;
        node from;
        node to;
        edge();
        edge(node from, node to);
        edge(node from, node to, EdgeType weight);
        void setWeight(EdgeType weight);
        EdgeType getWeight();

};
