// floyd_runtime.cpp
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <limits>
#include <omp.h>

extern "C"
{

    struct Graph
    {
        int64_t n;           // number of vertices
        int64_t m;           // number of edges
        int64_t *row_ptr;    // CSR row pointer (size n+1)
        int32_t *col_idx;    // CSR column indices (size m)
    };

    // Convert CSR to adjacency matrix (unweighted, undirected)
    std::vector<std::vector<int>> csr_to_adj_matrix(Graph *g)
    {
        int64_t n   = g->n;
        int64_t *row = g->row_ptr;
        int32_t *col = g->col_idx;

        int INF = 1e9; // large number to represent infinity

        std::vector<std::vector<int>> adj(n, std::vector<int>(n, INF));

        for (int64_t i = 0; i < n; i++)
            adj[i][i] = 0; // distance to self = 0

        for (int64_t u = 0; u < n; u++)
        {
            for (int64_t j = row[u]; j < row[u + 1]; j++)
            {
                int v = col[j];
                adj[u][v] = 1; // unweighted → weight = 1
                adj[v][u] = 1; // undirected → symmetric
            }
        }

        return adj;
    }

    // Print adjacency matrix
    // void print_matrix(const std::vector<std::vector<int>> &mat)
    // {
    //     int n = mat.size();
    //     for (int i = 0; i < n; i++)
    //     {
    //         for (int j = 0; j < n; j++)
    //         {
    //             if (mat[i][j] >= 1e9)
    //                 printf("INF ");
    //             else
    //                 printf("%3d ", mat[i][j]);
    //         }
    //         printf("\n");
    //     }
    // }

    // Parallel Floyd–Warshall
    void floyd_runtime(Graph *g)
    {
        if (!g) return;

        auto dist = csr_to_adj_matrix(g);

        //printf("Initial adjacency matrix:\n");
        //print_matrix(dist);

        int n = g->n;

        double start_time = omp_get_wtime();

        for (int k = 0; k < n; k++)
        {
            #pragma omp parallel for collapse(2) schedule(dynamic)
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    if (dist[i][k] + dist[k][j] < dist[i][j])
                    {
                        dist[i][j] = dist[i][k] + dist[k][j];
                    }
                }
            }
        }

        double end_time = omp_get_wtime();

        //printf("\nAll-pairs shortest path matrix:\n");
        //print_matrix(dist);

        printf("\n[Floyd-Warshall] Completed in %.6f seconds using %d threads\n",
               end_time - start_time,
               omp_get_max_threads());
    }

} // extern "C"
