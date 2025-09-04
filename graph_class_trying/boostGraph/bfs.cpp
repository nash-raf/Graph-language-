#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp> 
#include <boost/graph/depth_first_search.hpp>// for default_dfs_visitor
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

int main(int argc, char* argv[]) {
    using namespace boost;

    typedef adjacency_list<vecS, vecS, undirectedS> Graph;

    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        std::cerr << "Could not open input file\n";
        return 1;
    }

    int u, v;
    std::vector<std::pair<int, int>> edges;
    int maxNode = 0;

    while (infile >> u >> v) {
        edges.emplace_back(u, v);
        maxNode = std::max({maxNode, u, v});
    }

    Graph g(maxNode + 1);
    for (auto& e : edges) {
        add_edge(e.first, e.second, g);
    }

    auto start = std::chrono::high_resolution_clock::now();

    breadth_first_search(g, vertex(1,g) ,visitor(default_bfs_visitor()));

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Boost BFS took "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << " ms\n";

    return 0;
}
