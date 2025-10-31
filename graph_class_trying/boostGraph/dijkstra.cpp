#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <limits>

int main(int argc, char* argv[]) {
    using namespace boost;

    auto start = std::chrono::high_resolution_clock::now();

    // Define the graph type — undirected with weighted edges
    typedef adjacency_list<vecS, vecS, undirectedS,
        no_property, property<edge_weight_t, double>> Graph;
    typedef graph_traits<Graph>::vertex_descriptor Vertex;

    // Read input file (format: u v w)
    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        std::cerr << "Could not open input file\n";
        return 1;
    }

    int u, v;
    double w;
    std::vector<std::tuple<int, int, double>> edges;
    int maxNode = 0;

    while (infile >> u >> v >> w) {
        edges.emplace_back(u, v, w);
        maxNode = std::max({maxNode, u, v});
    }

    // Create graph and add edges
    Graph g(maxNode + 1);
    for (auto& e : edges) {
        add_edge(std::get<0>(e), std::get<1>(e), std::get<2>(e), g);
    }

    // Prepare vectors for distances and predecessors
    std::vector<double> distances(num_vertices(g), (std::numeric_limits<double>::max)());
    std::vector<Vertex> predecessors(num_vertices(g));

    Vertex source = 1; // Start from vertex 1 (you can change this)

    // Run Dijkstra
    dijkstra_shortest_paths(
        g,
        source,
        predecessor_map(make_iterator_property_map(predecessors.begin(), get(vertex_index, g))).
        distance_map(make_iterator_property_map(distances.begin(), get(vertex_index, g)))
    );

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Boost Dijkstra took "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << " ms\n";

    // (Optional) print distances
    
    // for (size_t i = 0; i < distances.size(); ++i) {
    //     if (distances[i] < std::numeric_limits<double>::max())
    //         std::cout << "Distance from " << source << " to " << i << " = " << distances[i] << "\n";
    //     else
    //         std::cout << "Distance from " << source << " to " << i << " = INF\n";
    // }
    

    return 0;
}
