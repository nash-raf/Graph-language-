// bfs_runtime_src.cpp — layout-aware source-based BFS traversal
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>
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

    int bfs_runtime_src(Graph *g, int32_t src, int32_t **out_array, int32_t *out_size)
    {
        if (!g || !out_array || !out_size)
            return 1;

        int64_t n = g->n;
        if (n < 0)
            return 2;

        if (src >= 0 && (src < 0 || src >= (int32_t)n))
        {
            std::fprintf(stderr, "[bfs_runtime_src] invalid src %d (n=%lld)\n",
                         src, (long long)n);
            return 3;
        }

        std::vector<char> visited((size_t)n, 0);
        std::vector<int32_t> order;
        order.reserve((size_t)std::min<int64_t>(1024, n));

        double t0 = omp_get_wtime();

        if (src >= 0)
        {
            std::vector<int32_t> frontier;
            frontier.push_back(src);
            visited[(size_t)src] = 1;
            order.push_back(src);

            while (!frontier.empty())
            {
                std::vector<int32_t> next;
                next.reserve(frontier.size() * 2);
                for (size_t fi = 0; fi < frontier.size(); ++fi)
                {
                    int32_t u = frontier[fi];
                    AutoNeighborIter iter;
                    autograph_neighbor_iter_init(g, (int64_t)u, &iter);
                    int32_t v;
                    while (autograph_neighbor_iter_next(&iter, &v))
                    {
                        if (!visited[(size_t)v])
                        {
                            visited[(size_t)v] = 1;
                            next.push_back(v);
                            order.push_back(v);
                        }
                    }
                }
                frontier.swap(next);
            }
        }
        else
        {
            for (int32_t start = 0; start < (int32_t)n; ++start)
            {
                if (visited[(size_t)start])
                    continue;
                std::vector<int32_t> frontier;
                frontier.push_back(start);
                visited[(size_t)start] = 1;
                order.push_back(start);

                while (!frontier.empty())
                {
                    std::vector<int32_t> next;
                    next.reserve(frontier.size() * 2);
                    for (size_t fi = 0; fi < frontier.size(); ++fi)
                    {
                        int32_t u = frontier[fi];
                        AutoNeighborIter iter;
                        autograph_neighbor_iter_init(g, (int64_t)u, &iter);
                        int32_t v;
                        while (autograph_neighbor_iter_next(&iter, &v))
                        {
                            if (!visited[(size_t)v])
                            {
                                visited[(size_t)v] = 1;
                                next.push_back(v);
                                order.push_back(v);
                            }
                        }
                    }
                    frontier.swap(next);
                }
            }
        }

        double t1 = omp_get_wtime();

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

        // std::printf("[bfs_runtime_src] src=%d visited %zu / %lld nodes in %.6f seconds\n",
        //             src, order.size(), (long long)n, t1 - t0);

        return 0;
    }

} // extern "C"
