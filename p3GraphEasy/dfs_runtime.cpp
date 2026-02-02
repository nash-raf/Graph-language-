// dfs_runtime.cpp
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>

extern "C"
{

    struct Graph
    {
        int64_t n;
        int64_t m;
        int64_t *row_ptr; // CSR row pointer (size n+1)
        int32_t *col_idx; // CSR column indices
    };

    void dfs_runtime(Graph *g)
    {
        if (!g) return;

        int64_t n   = g->n;
        int64_t *row = g->row_ptr;
        int32_t *col = g->col_idx;

        std::vector<char> visited(n, 0);

        for (int32_t start = 0; start < (int32_t)n; ++start)
        {
            if (visited[start]) continue;

            // Local stack for DFS
            std::vector<int32_t> stack;
            stack.reserve(n);
            stack.push_back(start);

            while (!stack.empty())
            {
                int32_t u = stack.back();
                stack.pop_back();

                if (visited[u]) continue;
                visited[u] = 1;


                int64_t rp  = row[u];
                int64_t rp1 = row[u + 1];

                // push neighbors in reverse order to maintain similar DFS order
                for (int64_t i = rp1 - 1; i >= rp; --i)
                {
                    int32_t v = col[i];
                    if (!visited[v])
                    {
                        stack.push_back(v);
                    }
                }
            }
        }
    }

} // extern "C"
