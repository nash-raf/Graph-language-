#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <limits>

int main(int argc, char* argv[]) {
    using namespace boost;

    typedef adjacency_list<vecS, vecS, directedS,
        no_property, property<edge_weight_t, int>> Graph;

    typedef property_map<Graph, edge_weight_t>::type WeightMap;

    if (argc < 2) {
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        return 1;
    }

    int u, v, w = 1; // default weight = 1 if not in file
    std::vector<std::tuple<int, int, int>> edges;
    int maxNode = 0;

    while (infile >> u >> v) {
        // if file has weights: infile >> u >> v >> w;
        edges.emplace_back(u, v, w);
        maxNode = std::max({maxNode, u, v});
    }

    Graph g(maxNode + 1);
    WeightMap weightmap = get(edge_weight, g);

    for (auto& e : edges) {
        auto [src, dst, w] = e;
        auto edge = add_edge(src, dst, g).first;
        weightmap[edge] = w;
    }

    std::vector<std::vector<int>> dist(num_vertices(g),
                                       std::vector<int>(num_vertices(g)));

    auto start = std::chrono::high_resolution_clock::now();

    floyd_warshall_all_pairs_shortest_paths(
        g,
        dist,
        weight_map(weightmap).
        distance_inf((std::numeric_limits<int>::max)()).
        distance_zero(0)
    );

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << std::chrono::duration<double>(end - start).count() << "\n";

    return 0;
}
