#pragma once

#include <iostream>
#include <vector>
#include <variant>

using NodeType = std::variant<int, double, char, bool, std::string>;

class node{
    public:
        NodeType id;
        std::vector<NodeType> data;
        node();
        node(NodeType id);
        node(NodeType id, const std::vector<NodeType>& initialData);
        void setID(NodeType id);
        NodeType getId();
        void addData(NodeType d);
        void removeData(NodeType d);
        bool hasData(NodeType d);
        void printData();
        bool operator==(const node &other) const {
            return this->id == other.id; 
        }
        bool operator!=(const node &other) const {
            return !(*this == other);
        }


};

namespace std {
    template <>
    struct hash<node> {
        size_t operator()(const node &n) const {
            // Use std::visit to hash the underlying value in the variant
            return std::visit([](auto &&arg) -> size_t {
                return std::hash<std::decay_t<decltype(arg)>>()(arg);
            }, n.id);
        }
    };
}
