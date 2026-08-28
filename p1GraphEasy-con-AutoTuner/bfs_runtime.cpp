// bfs_runtime.cpp — layout-aware BFS traversal
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <omp.h>

#include "autotuner_runtime.h"

extern "C"
{

    struct Graph
    {
        int64_t n;
        int64_t m;
        int64_t *row_ptr;
        int32_t *col_idx;
    };

    void bfs_runtime(Graph *g)
    {
        if (!g) return;

        int64_t n = g->n;
        if (n <= 0) return;

        std::vector<char> visited(n, 0);

        double start_time = omp_get_wtime();

        for (int32_t start = 0; start < (int32_t)n; ++start)
        {
            if (visited[start]) continue;

            std::vector<int32_t> frontier;
            frontier.push_back(start);
            visited[start] = 1;

            while (!frontier.empty())
            {
                std::vector<int32_t> next_frontier;

                for (size_t idx = 0; idx < frontier.size(); ++idx)
                {
                    int32_t u = frontier[idx];
                    AutoNeighborIter iter;
                    autograph_neighbor_iter_init(g, (int64_t)u, &iter);
                    int32_t v;
                    while (autograph_neighbor_iter_next(&iter, &v))
                    {
                        if (v < 0 || v >= n) continue;
                        if (!visited[v])
                        {
                            visited[v] = 1;
                            next_frontier.push_back(v);
                        }
                    }
                }

                frontier.swap(next_frontier);
            }
        }

        double end_time = omp_get_wtime();

        // printf("[BFS] Completed in %.6f seconds (sequential)\n",
        //        end_time - start_time);
    }

} // extern "C"
