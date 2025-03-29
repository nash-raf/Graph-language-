#include <iostream>
#include <vector>
#include <algorithm>
#include "node.h"


node::node() {
}

node::node(NodeType id) {
    this->id = id;
}

node::node(NodeType id, const std::vector<NodeType>& initialData) {
    this->id = id;
    this->data = initialData;
    
}

void node::setID(NodeType id) {
    this->id = id;
}

NodeType node::getId() {
    return id;
}

void node::addData(NodeType d) {
    data.push_back(d);
}

void node::removeData(NodeType d) {
    data.erase(std::remove(data.begin(), data.end(), d), data.end());
}

bool node::hasData(NodeType d) {
    return std::find(data.begin(), data.end(), d) != data.end();
}


void node::printData() {
    for (const auto& d : data) {
        std::visit([](auto&& value) { std::cout << value << " "; }, d);
    }
    std::cout << std::endl;
}

