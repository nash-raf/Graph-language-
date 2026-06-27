// dfs_runtime.cpp — layout-aware DFS traversal
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>

extern "C" void autograph_get_neighbors(void *graph_ptr, int64_t u,
                                        int32_t *out_buf, int64_t *out_count);

extern "C"
{

    struct Graph
    {
        int64_t n;
        int64_t m;
        int64_t *row_ptr;
        int32_t *col_idx;
    };

    void dfs_runtime(Graph *g)
    {
        if (!g) return;

        int64_t n = g->n;
        if (n <= 0) return;

        std::vector<char> visited(n, 0);
        std::vector<int32_t> nbuf(n);

        for (int32_t start = 0; start < (int32_t)n; ++start)
        {
            if (visited[start]) continue;

            std::vector<int32_t> stack;
            stack.reserve(n);
            stack.push_back(start);

            while (!stack.empty())
            {
                int32_t u = stack.back();
                stack.pop_back();

                if (visited[u]) continue;
                visited[u] = 1;

                int64_t cnt = 0;
                autograph_get_neighbors(g, (int64_t)u, nbuf.data(), &cnt);

                // push neighbors in reverse order to maintain similar DFS order
                for (int64_t i = cnt - 1; i >= 0; --i)
                {
                    int32_t v = nbuf[i];
                    if (!visited[v])
                    {
                        stack.push_back(v);
                    }
                }
            }
        }
    }

} // extern "C"
