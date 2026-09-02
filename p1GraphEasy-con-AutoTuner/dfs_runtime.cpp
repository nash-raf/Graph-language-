// dfs_runtime.cpp — layout-aware DFS traversal
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>

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

    void dfs_runtime(Graph *g)
    {
        if (!g) return;

        int64_t n = g->n;
        if (n <= 0) return;

        std::vector<char> visited(n, 0);

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

                AutoNeighborIter iter;
                autograph_neighbor_iter_init(g, (int64_t)u, &iter);
                int32_t v;
                // push neighbors in reverse order to maintain similar DFS order
                std::vector<int32_t> nbrs;
                while (autograph_neighbor_iter_next(&iter, &v))
                    nbrs.push_back(v);
                for (int64_t i = (int64_t)nbrs.size() - 1; i >= 0; --i)
                {
                    v = nbrs[(size_t)i];
                    if (!visited[v])
                        stack.push_back(v);
                }
            }
        }
    }

} // extern "C"
