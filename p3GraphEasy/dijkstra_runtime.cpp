// dijkstra_runtime.cpp


#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>
#include <utility>
#include <limits>
#include <functional>

extern "C"
{

    struct Graph
    {
        int64_t n;
        int64_t m;
        int64_t *row_ptr; // CSR row pointer (size n+1)
        int32_t *col_idx; // CSR column indices (size m)
        int32_t *weights; // edge weights (size m), must be non-negative
    };

    void dijkstra_runtime(Graph *g)
    {
        // printf("running00\n");
        int32_t source = 0;
        if (!g)
            return;
        printf("running0\n");

        if (!g->row_ptr || !g->col_idx || !g->weights)
            return;
        printf("running1\n  ");
        const int64_t n = g->n;
        if (n <= 0)
            return;

        printf("running2\n");

        if (source < 0 || source >= (int32_t)n)
            return;
        printf("running3");
        const int64_t *row = g->row_ptr;
        const int32_t *col = g->col_idx;
        const int32_t *w = g->weights;

        const long long INF = std::numeric_limits<long long>::max() / 4;

        std::vector<long long> dist;
        dist.assign((size_t)n, INF);
        dist[source] = 0;

        using PII = std::pair<long long, int32_t>;
        std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;
        pq.emplace(0LL, source);

        while (!pq.empty())
        {
            auto top = pq.top();
            pq.pop();
            long long d = top.first;
            int32_t u = top.second;

            if (d != dist[(size_t)u])
                continue;

            int64_t rp = row[(size_t)u];
            int64_t rp1 = row[(size_t)u + 1];
            for (int64_t ei = rp; ei < rp1; ++ei)
            {
                int32_t v = col[(size_t)ei];
                long long wuv = (long long)w[(size_t)ei];

                if (wuv < 0)
                    continue;

                if (d != INF)
                {
                    long long nd = d + wuv;
                    if (nd < dist[(size_t)v])
                    {
                        dist[(size_t)v] = nd;
                        pq.emplace(nd, v);
                    }
                }
            }
        }

        // if (out_dist)
        // {
        //     for (int64_t i = 0; i < n; ++i)
        //     {
        //         if (dist[(size_t)i] >= INF / 2)
        //             out_dist[(size_t)i] = std::numeric_limits<int64_t>::max();
        //         else
        //             out_dist[(size_t)i] = (int64_t)dist[(size_t)i];
        //     }
        // }

        printf("Shortest distances from source node %d:\n", source);
        for (int64_t i = 0; i < n; ++i)
        {
            if (dist[i] >= INF / 2)
                printf("Node %ld: INF (unreachable)\n", i);
            else
                printf("Node %ld: %lld\n", i, dist[i]);
        }
        printf("\n");
    }

} // extern "C"
