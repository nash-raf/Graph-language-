// bfs_runtime_single.cpp
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>

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
    }

} // extern "C"
