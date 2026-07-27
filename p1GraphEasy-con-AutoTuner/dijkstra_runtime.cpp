// dijkstra_runtime.cpp


#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>
#include <utility>
#include <limits>
#include <climits>
#include <functional>

extern "C"
{

    struct Graph
    {
        int64_t n;
        int64_t m;
        int64_t *row_ptr; // CSR row pointer (size n+1)
        int32_t *col_idx; // CSR column indices (size m)
        int32_t *weights; // edge weights (size m), may be null => treat as 1
    };

    // New: return distances as an allocated int32_t array and its size.
    // int dijkstra_runtime_src(Graph* g, int32_t src, int32_t** out_array, int32_t* out_size)
    int dijkstra_runtime_src(Graph *g, int32_t src, int32_t **out_array, int32_t *out_size)
    {
        if (!g || !g->row_ptr || !g->col_idx || !out_array || !out_size)
            return 1;

        const int64_t n = g->n;
        if (n <= 0) {
            *out_array = nullptr;
            *out_size = 0;
            return 0;
        }

        if (src < 0 || src >= (int32_t)n) {
            std::fprintf(stderr, "[dijkstra_runtime_src] invalid src %d (n=%lld)\n",
                         src, (long long)n);
            return 2;
        }

        const int64_t *row = g->row_ptr;
        const int32_t *col = g->col_idx;
        const int32_t *w   = g->weights; // may be null => weight 1

        const long long INF = std::numeric_limits<long long>::max() / 4;

        std::vector<long long> dist((size_t)n, INF);
        dist[(size_t)src] = 0;

        using PII = std::pair<long long, int32_t>;
        std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;
        pq.emplace(0LL, src);

        while (!pq.empty())
        {
            auto [du, u] = pq.top();
            pq.pop();
            if (du != dist[(size_t)u]) continue;

            int64_t rp  = row[u];
            int64_t rp1 = row[u + 1];

            for (int64_t ei = rp; ei < rp1; ++ei)
            {
                int32_t v = col[ei];
                long long wt = w ? (long long)w[ei] : 1LL;
                if (wt < 0) continue; // ignore negative edges

                long long nd = du + wt;
                if (nd < dist[(size_t)v])
                {
                    dist[(size_t)v] = nd;
                    pq.emplace(nd, v);
                }
            }
        }

        // Allocate output as int32_t distances; unreachable => -1; clamp to INT32_MAX
        int32_t *buf = (int32_t *)std::malloc(sizeof(int32_t) * (size_t)n);
        if (!buf) return 3;

        for (int64_t i = 0; i < n; ++i)
        {
            long long d = dist[(size_t)i];
            if (d >= INF / 2) buf[(size_t)i] = -1;
            else if (d > INT32_MAX) buf[(size_t)i] = INT32_MAX;
            else buf[(size_t)i] = (int32_t)d;
        }

        *out_array = buf;
        *out_size  = (int32_t)n;
        return 0;
    }

    // Optional: keep old symbol around (no-op) for compatibility
    void dijkstra_runtime(Graph *g)
    {
        (void)g;
        // Intentionally empty; IR will use dijkstra_runtime_src instead.
    }

}