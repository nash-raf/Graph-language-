// karger_min_cut.cpp
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>

#ifdef _OPENMP
#include <omp.h>   // only for timing; otherwise comment out
#endif

extern "C"
{

    // Simple CSR graph container
    struct Graph
    {
        int64_t n;         // number of nodes
        int64_t m;         // number of edges (in CSR col_idx, counts each directed edge)
        int64_t *row_ptr;  // size n+1
        int32_t *col_idx;  // size m
    };

    // Union-find (disjoint set)
    struct UnionFind
    {
        std::vector<int> parent;
        std::vector<int> size;

        UnionFind(int n = 0) : parent(n), size(n, 1)
        {
            for (int i = 0; i < n; ++i)
                parent[i] = i;
        }

        int find(int x)
        {
            if (parent[x] == x)
                return x;
            return parent[x] = find(parent[x]);
        }

        bool unite(int a, int b)
        {
            a = find(a);
            b = find(b);
            if (a == b)
                return false;
            if (size[a] < size[b])
                std::swap(a, b);
            parent[b] = a;
            size[a] += size[b];
            return true;
        }
    };

    // Main Karger algorithm runtime function
    int karger_runtime(Graph *g)
    {
        if (!g)
            return -1;
        if (g->n <= 0)
            return -1;
        if (g->n == 1)
            return 0;

        const int64_t n = g->n;
        int64_t *row = g->row_ptr;
        int32_t *col = g->col_idx;

        // Build an undirected edge list (u < v) from CSR
        std::vector<std::pair<int, int>> edges;
        edges.reserve((size_t)g->m / 2);

        for (int u = 0; u < (int)n; ++u)
        {
            for (int64_t ei = row[u]; ei < row[u + 1]; ++ei)
            {
                int v = col[ei];
                if (u < v)
                    edges.emplace_back(u, v);
            }
        }

        if (edges.empty())
        {
            std::printf("[Karger] graph has no edges (n=%lld)\n", (long long)n);
            return 0;
        }

        const size_t E = edges.size();

        // Number of trials (you capped at 20 earlier; preserving that logic)
        int64_t trials = n * n;
        const int64_t TRIAL_CAP = 20;
        if (trials > TRIAL_CAP)
            trials = TRIAL_CAP;

        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, E - 1);

        int bestCut = std::numeric_limits<int>::max();

        #ifdef _OPENMP
        double t0 = omp_get_wtime();
        #endif

        // Run Karger contractions
        for (int64_t t = 0; t < trials; ++t)
        {
            UnionFind uf((int)n);
            int remaining = (int)n;

            // Contract until only 2 supernodes left
            while (remaining > 2)
            {
                size_t idx = dist(gen);
                int u = edges[idx].first;
                int v = edges[idx].second;

                int ru = uf.find(u);
                int rv = uf.find(v);

                if (ru == rv)
                    continue;

                uf.unite(ru, rv);
                remaining--;
            }

            // Count cut edges
            int cutSize = 0;
            for (size_t i = 0; i < E; ++i)
            {
                int u = edges[i].first;
                int v = edges[i].second;
                if (uf.find(u) != uf.find(v))
                    ++cutSize;
            }

            if (cutSize < bestCut)
            {
                bestCut = cutSize;
                if (bestCut <= 1)
                    break; // cannot go lower
            }
        }

        #ifdef _OPENMP
        double t1 = omp_get_wtime();
        std::printf("[Karger] n=%lld, m=%lld, trials=%lld -> mincut=%d  (time=%.6f s)\n",
                    (long long)g->n, (long long)g->m, (long long)trials,
                    bestCut, t1 - t0);
        #else
        std::printf("[Karger] n=%lld, m=%lld, trials=%lld -> mincut=%d\n",
                    (long long)g->n, (long long)g->m, (long long)trials,
                    bestCut);
        #endif

        return bestCut;
    }

} // extern "C"
