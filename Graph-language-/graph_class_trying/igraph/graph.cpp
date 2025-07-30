#include <igraph.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

void load_graph_from_file(const std::string& filename, igraph_t* g) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
    }

    std::vector<igraph_real_t> edge_list;
    int u, v;
    while (infile >> u >> v) {
        edge_list.push_back(u);
        edge_list.push_back(v);
    }

    igraph_vector_t edges;
    igraph_vector_init(&edges, edge_list.size());
    for (size_t i = 0; i < edge_list.size(); ++i)
        VECTOR(edges)[i] = edge_list[i];

    igraph_create(g, &edges, 0, IGRAPH_UNDIRECTED);
    igraph_vector_destroy(&edges);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./bfs_igraph <graph_file>\n";
        return 1;
    }

    igraph_t graph;
    load_graph_from_file(argv[1], &graph);

    igraph_vector_t order;
    igraph_vector_init(&order, 0);

    auto start = std::chrono::high_resolution_clock::now();

    igraph_bfs(&graph, /*root=*/0, nullptr, &order, nullptr, nullptr, nullptr, nullptr, nullptr, IGRAPH_BFS_DEFAULT);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "BFS visited " << igraph_vector_size(&order) << " nodes.\n";
    std::cout << "Time taken: " << elapsed.count() << " ms\n";

    // Uncomment to print traversal order
    /*
    std::cout << "BFS Order: ";
    for (int i = 0; i < igraph_vector_size(&order); ++i)
        std::cout << VECTOR(order)[i] << " ";
    std::cout << "\n";
    */

    igraph_vector_destroy(&order);
    igraph_destroy(&graph);
    return 0;
}
