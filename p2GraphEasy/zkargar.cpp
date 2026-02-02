#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <map>
#include <set>

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

        // Number of trials (cap at 20)
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

// Build CSR for the given undirected edge list with labels
int main()
{
    // Edge list (labels): undirected
    std::vector<std::pair<int, int>> labelEdges = {
        {10, 20},
        {10, 30},
        {10, 40},
        {20, 30},
        {30, 40},
        {40, 50},
        {40, 60},
        {50, 60},
        {50, 70},
        {60, 70}
    };

    // Collect and sort unique labels
    std::set<int> labels;
    for (auto &e : labelEdges) {
        labels.insert(e.first);
        labels.insert(e.second);
    }

    std::vector<int> nodes(labels.begin(), labels.end()); // sorted
    int64_t n = (int64_t)nodes.size();

    // Map label -> 0..n-1
    std::map<int, int> labelToIdx;
    for (size_t i = 0; i < nodes.size(); ++i)
        labelToIdx[nodes[i]] = (int)i;

    // Degree count (undirected -> add both ways)
    std::vector<int64_t> deg(n, 0);
    for (auto &e : labelEdges) {
        int u = labelToIdx[e.first];
        int v = labelToIdx[e.second];
        if (u == v) continue; // ignore self-loop
        deg[u]++, deg[v]++;
    }

    // Build CSR
    std::vector<int64_t> row_ptr(n + 1, 0);
    for (int64_t i = 0; i < n; ++i)
        row_ptr[i + 1] = row_ptr[i] + deg[i];

    int64_t m = row_ptr[n]; // number of directed edges
    std::vector<int32_t> col_idx(m, 0);

    // Fill adjacency
    std::vector<int64_t> next = row_ptr;
    for (auto &e : labelEdges) {
        int u = labelToIdx[e.first];
        int v = labelToIdx[e.second];
        if (u == v) continue;
        col_idx[next[u]++] = v;
        col_idx[next[v]++] = u;
    }

    // Prepare Graph
    Graph G;
    G.n = n;
    G.m = m;

    // Allocate C arrays for Graph (as expected by runtime)
    int64_t *rp = (int64_t*)std::malloc(sizeof(int64_t) * (size_t)(n + 1));
    int32_t *ci = (int32_t*)std::malloc(sizeof(int32_t) * (size_t)m);
    if (!rp || !ci) {
        std::fprintf(stderr, "Allocation failed\n");
        std::free(rp); std::free(ci);
        return 1;
    }
    std::copy(row_ptr.begin(), row_ptr.end(), rp);
    std::copy(col_idx.begin(), col_idx.end(), ci);
    G.row_ptr = rp;
    G.col_idx = ci;

    std::printf("Graph built from labels: n=%lld, m=%lld (directed)\n",
                (long long)G.n, (long long)G.m);

    // Run Karger min cut
    int bestCut = karger_runtime(&G);
    std::printf("Best min cut returned: %d\n", bestCut);

    std::free(rp);
    std::free(ci);
    return 0;
}