#include <bits/stdc++.h>
using namespace std;

// Karger Randomized Min Cut for UNWEIGHTED graph.
// Input file "edgelist.txt": u v

struct Edge { int u, v; };

int random_contraction(vector<Edge> edges, int n) {
    vector<int> parent(n);
    iota(parent.begin(), parent.end(), 0);

    function<int(int)> findp = [&](int x) {
        return parent[x] == x ? x : parent[x] = findp(parent[x]);
    };

    auto unite = [&](int a, int b) {
        a = findp(a); b = findp(b);
        if (a != b) parent[b] = a;
    };

    int components = n;

    while (components > 2) {
        int id = rand() % edges.size();
        int u = edges[id].u;
        int v = edges[id].v;

        int pu = findp(u);
        int pv = findp(v);

        if (pu == pv) continue;

        unite(pu, pv);
        components--;
    }

    // Count crossing edges
    int cut = 0;
    for (auto &e : edges) {
        if (findp(e.u) != findp(e.v)) cut++;
    }
    return cut;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<Edge> edges;
    {
        ifstream fin("edgelist.txt");
        if (!fin.is_open()) {
            cerr << "Could not open edgelist.txt\n";
            return 1;
        }
        int u, v;
        while (fin >> u >> v) {
            if (u != v) edges.push_back({u, v});
        }
    }

    if (edges.empty()) {
        cout << "Graph empty\n";
        return 0;
    }

    int mx = 0;
    for (auto &e : edges) mx = max(mx, max(e.u, e.v));
    int n = mx + 1;

    srand(time(nullptr));

    int trials = 20;   // more trials = better success probability
    int best = INT_MAX;

    for (int i = 0; i < trials; i++) {
        best = min(best, random_contraction(edges, n));
    }

    cout << "Randomized Karger Min Cut ≈ " << best << "\n";
    return 0;
}
