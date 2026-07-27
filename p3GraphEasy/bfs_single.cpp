// bfs_single.cpp
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>
#include <omp.h>   // for omp_get_wtime

extern "C"
{

    struct Graph
    {
        int64_t n;
        int64_t m;
        int64_t *row_ptr; // CSR row pointer (size n+1)
        int32_t *col_idx; // CSR column indices
    };

    // BFS returns traversal order in out_order; returns number of visited nodes
    int bfs_single(Graph *g, int **out_order)
    {
        if (!g) return 0;

        int64_t n   = g->n;
        int64_t *row = g->row_ptr;
        int32_t *col = g->col_idx;

        std::vector<char> visited(n, 0);
        std::vector<int> result;

        double start_time = omp_get_wtime();

        for (int32_t start = 0; start < (int32_t)n; ++start)
        {
            if (visited[start]) continue;

            std::queue<int32_t> q;
            q.push(start);
            visited[start] = 1;

            while (!q.empty())
            {
                int32_t u = q.front();
                q.pop();

                result.push_back(u); // store traversal order

                int64_t rp  = row[u];
                int64_t rp1 = row[u + 1];

                for (int64_t i = rp; i < rp1; ++i)
                {
                    int32_t v = col[i];
                    if (!visited[v])
                    {
                        visited[v] = 1;
                        q.push(v);
                    }
                }
            }
        }

        double end_time = omp_get_wtime();
        printf("[BFS-Single] Completed in %.6f seconds\n", end_time - start_time);

        // allocate and copy to output array
        int *arr = (int*)malloc(result.size() * sizeof(int));
        for (size_t i = 0; i < result.size(); ++i)
            arr[i] = result[i];

        if (out_order)
            *out_order = arr;
        else
            free(arr);

        return result.size();
    }

} // extern "C"
