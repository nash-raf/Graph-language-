#include <bits/stdc++.h>
using namespace std;

// Deterministic Stoer-Wagner Global Min Cut for UNWEIGHTED graph.
// Input: edgelist.txt with lines "u v"

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<pair<int,int>> edges;
    {
        ifstream fin("edgelist.txt");
        if (!fin.is_open()) {
            cerr << "Could not open edgelist.txt\n";
            return 1;
        }
        int u, v;
        while (fin >> u >> v) {
            edges.emplace_back(u, v);
        }
    }

    if (edges.empty()) {
        cout << "Graph empty\n";
        return 0;
    }

    int mx = 0;
    for (auto &p : edges) mx = max(mx, max(p.first, p.second));
    int n = mx + 1;

    vector<vector<int>> G(n, vector<int>(n, 0));
    for (auto &e : edges) {
        int u = e.first, v = e.second;
        if (u == v) continue;
        G[u][v] += 1;
        G[v][u] += 1;
    }

    vector<int> nodes(n);
    iota(nodes.begin(), nodes.end(), 0);

    int best = INT_MAX;

    while (nodes.size() > 1) {
        int m = nodes.size();
        vector<int> added(m, 0), weight(m, 0);

        int prev = -1, last = -1;

        for (int i = 0; i < m; i++) {
            int sel = -1;
            for (int j = 0; j < m; j++)
                if (!added[j] && (sel == -1 || weight[j] > weight[sel]))
                    sel = j;

            added[sel] = 1;
            if (i == m - 2) prev = sel;
            if (i == m - 1) last = sel;

            for (int j = 0; j < m; j++)
                if (!added[j])
                    weight[j] += G[nodes[sel]][nodes[j]];
        }

        best = min(best, weight[last]);

        int a = nodes[prev], b = nodes[last];
        for (int i = 0; i < n; i++) {
            G[a][i] += G[b][i];
            G[i][a] += G[i][b];
        }
        nodes.erase(nodes.begin() + last);
    }

    cout << "Deterministic Stoer-Wagner Min Cut = " << best << "\n";
    return 0;
}
