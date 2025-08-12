#include <GraphBLAS.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./bfs_graphblas g.txt\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open input file\n";
        return 1;
    }

    // Step 1: Read edges and find max node ID
    std::vector<std::pair<int, int>> edges;
    int maxNodeId = 0, u, v;
    while (infile >> u >> v) {
        edges.emplace_back(u, v);
        maxNodeId = std::max({maxNodeId, u, v});
    }
    int n = maxNodeId + 1;

    // Step 2: Initialize GraphBLAS
    GrB_init(GrB_BLOCKING);

    // Step 3: Create adjacency matrix A
    GrB_Matrix A;
    GrB_Matrix_new(&A, GrB_BOOL, n, n);

    for (const auto& edge : edges) {
        GrB_Matrix_setElement_BOOL(A, true, edge.first, edge.second);
        GrB_Matrix_setElement_BOOL(A, true, edge.second, edge.first); // undirected
    }

    // Step 4: BFS setup
    GrB_Vector frontier, visited;
    GrB_Vector_new(&frontier, GrB_BOOL, n);
    GrB_Vector_new(&visited, GrB_BOOL, n);

    // Start from node 1
    GrB_Vector_setElement_BOOL(frontier, true, 1);
    GrB_Vector_setElement_BOOL(visited, true, 1);

    // Semiring for BFS (Boolean OR-AND)
    GrB_Semiring semiring = GxB_LOR_LAND_BOOL;

    std::cout << "Starting GraphBLAS BFS from node 1...\n";

    auto start = std::chrono::high_resolution_clock::now();

    // Step 5: BFS Loop
    while (true) {
        GrB_Vector new_frontier;
        GrB_Vector_new(&new_frontier, GrB_BOOL, n);

        // frontier = A * frontier (using boolean semiring)
        GrB_vxm(new_frontier, visited, NULL, semiring, frontier, A, GrB_DESC_R);

        GrB_Vector_eWiseAdd_BinaryOp(visited, NULL, NULL, GrB_LOR, visited, new_frontier, NULL);

        bool empty;
        GrB_Vector_reduce_BOOL(&empty, GrB_LAND_MONOID_BOOL, new_frontier, NULL);
        empty = !empty;

        GrB_free(&frontier);
        frontier = new_frontier;

        if (empty) break;
    }

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "GraphBLAS BFS completed in " << elapsed << " ms\n";

    // Cleanup
    GrB_free(&frontier);
    GrB_free(&visited);
    GrB_free(&A);
    GrB_finalize();

    return 0;
}
