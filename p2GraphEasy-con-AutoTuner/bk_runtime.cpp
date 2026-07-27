// bk_pivot_single.cpp
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>

extern "C"
{

    struct Graph
    {
        int64_t n;
        int64_t m;
        int64_t *row_ptr; // CSR row pointer (size n+1)
        int32_t *col_idx; // CSR column indices
    };

    // Helper: intersect two sorted vectors
    std::vector<int32_t> intersect_sorted(const std::vector<int32_t>& a, const std::vector<int32_t>& b)
    {
        std::vector<int32_t> result;
        size_t i = 0, j = 0;
        while (i < a.size() && j < b.size())
        {
            if (a[i] < b[j]) i++;
            else if (a[i] > b[j]) j++;
            else { result.push_back(a[i]); i++; j++; }
        }
        return result;
    }

    void bron_kerbosch_with_pivot(Graph* g,
                                  std::vector<int32_t>& R,
                                  std::vector<int32_t>& P,
                                  std::vector<int32_t>& X)
    {
        if (P.empty() && X.empty())
        {
            // R is a maximal clique
            return;
        }

        // Choose pivot u from P ∪ X (heuristic: max degree)
        int32_t pivot = -1;
        int64_t max_deg = -1;
        std::vector<int32_t> union_PX = P;
        union_PX.insert(union_PX.end(), X.begin(), X.end());
        for (int32_t u : union_PX)
        {
            int64_t deg = g->row_ptr[u + 1] - g->row_ptr[u];
            if (deg > max_deg)
            {
                max_deg = deg;
                pivot = u;
            }
        }

        // N(pivot)
        int64_t rp  = g->row_ptr[pivot];
        int64_t rp1 = g->row_ptr[pivot + 1];
        std::vector<int32_t> neighbors_pivot(g->col_idx + rp, g->col_idx + rp1);

        // P \ N(pivot)
        std::vector<int32_t> candidates;
        std::set_difference(P.begin(), P.end(),
                            neighbors_pivot.begin(), neighbors_pivot.end(),
                            std::back_inserter(candidates));

        for (int32_t v : candidates)
        {
            // N(v)
            int64_t rpv  = g->row_ptr[v];
            int64_t rpv1 = g->row_ptr[v + 1];
            std::vector<int32_t> Nv(g->col_idx + rpv, g->col_idx + rpv1);

            // R ∪ {v}
            std::vector<int32_t> R_new = R;
            R_new.push_back(v);

            // P ∩ N(v)
            std::vector<int32_t> P_new = intersect_sorted(P, Nv);
            // X ∩ N(v)
            std::vector<int32_t> X_new = intersect_sorted(X, Nv);

            bron_kerbosch_with_pivot(g, R_new, P_new, X_new);

            // P = P \ {v}, X = X ∪ {v}
            P.erase(std::remove(P.begin(), P.end(), v), P.end());
            X.push_back(v);
        }
    }

    void bk_runtime(Graph* g)
    {
        if (!g) return;

        // Initialize P = {0..n-1}, R = {}, X = {}
        std::vector<int32_t> R;
        std::vector<int32_t> P(g->n);
        std::vector<int32_t> X;

        for (int32_t i = 0; i < g->n; ++i) P[i] = i;

        bron_kerbosch_with_pivot(g, R, P, X);
    }

} // extern "C"
