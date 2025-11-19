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

    // New: mirror bfs_runtime_src signature/semantics
    // int dfs_runtime_src(Graph* g, int32_t src, int32_t** out_array, int32_t* out_size)
    // - if src >= 0: DFS from src
    // - if src < 0: DFS over all components
    int dfs_runtime_src(Graph *g, int32_t src, int32_t **out_array, int32_t *out_size)
    {
        if (!g || !out_array || !out_size)
            return 1;

        int64_t n = g->n;
        int64_t *row = g->row_ptr;
        int32_t *col = g->col_idx;

        if (n < 0)
            return 2;

        if (src >= 0 && (src < 0 || src >= (int32_t)n))
        {
            std::fprintf(stderr, "[dfs_runtime_src] invalid src %d (n=%lld)\n",
                         src, (long long)n);
            return 3;
        }

        std::vector<char> visited((size_t)n, 0);
        std::vector<int32_t> order;
        order.reserve((size_t)n);

        auto run_dfs = [&](int32_t start) {
            std::vector<int32_t> stack;
            stack.reserve(64);
            stack.push_back(start);

            while (!stack.empty())
            {
                int32_t u = stack.back();
                stack.pop_back();
                if (visited[(size_t)u])
                    continue;
                visited[(size_t)u] = 1;
                order.push_back(u);

                int64_t rp = row[u];
                int64_t rp1 = row[u + 1];

                // Push neighbors in reverse to get a stable order
                for (int64_t i = rp1 - 1; i >= rp; --i)
                {
                    int32_t v = col[i];
                    if (!visited[(size_t)v])
                        stack.push_back(v);
                }
            }
        };

        if (src >= 0)
        {
            run_dfs(src);
        }
        else
        {
            for (int32_t start = 0; start < (int32_t)n; ++start)
            {
                if (!visited[(size_t)start])
                    run_dfs(start);
            }
        }

        if (order.empty())
        {
            *out_array = nullptr;
            *out_size = 0;
        }
        else
        {
            int32_t *buf = (int32_t *)std::malloc(sizeof(int32_t) * order.size());
            if (!buf)
                return 4;
            for (size_t i = 0; i < order.size(); ++i)
                buf[i] = order[i];
            *out_array = buf;
            *out_size = static_cast<int32_t>(order.size());
        }

        return 0;
    }

} // extern "C"