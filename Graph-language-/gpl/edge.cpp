#include <iostream>
#include <vector>
#include "edge.h"
#include "node.h"

edge::edge() {
}

edge::edge(node from, node to) {
    this->from = from;
    this->to = to;
}

edge::edge(node from, node to, EdgeType weight) {
    this->from = from;
    this->to = to;
    this->weight = weight;
}
