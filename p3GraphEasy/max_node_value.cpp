#include <iostream>
#include <fstream>
#include <algorithm>

int main() {
    std::ifstream infile("g.txt");
    int u, v;
    int maxNode = -1;

    while (infile >> u >> v) {
        maxNode = std::max({maxNode, u, v});
    }

    std::cout << "Max node: " << maxNode << std::endl;
    return 0;
}
