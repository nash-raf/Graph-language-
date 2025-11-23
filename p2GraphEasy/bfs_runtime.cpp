// bfs_runtime_seq.cpp
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>
#include <omp.h> // kept only for timing (you can replace with chrono if desired)

extern "C"
{

    struct Graph
    {
        int64_t n;
        int64_t m;
        int64_t *row_ptr; // CSR row pointer (size n+1)
        int32_t *col_idx; // CSR column indices
    };

    void bfs_runtime(Graph *g)
    {
        if (!g) return;

        int64_t n   = g->n;
        int64_t *row = g->row_ptr;
        int32_t *col = g->col_idx;

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
                    int64_t rp  = row[u];
                    int64_t rp1 = row[u + 1];

                    for (int64_t i = rp; i < rp1; ++i)
                    {
                        int32_t v = col[i];
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

        printf("[BFS] Completed in %.6f seconds (sequential)\n",
               end_time - start_time);
    }

} // extern "C"
