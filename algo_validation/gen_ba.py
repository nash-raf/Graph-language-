#!/usr/bin/env python3
"""Barabasi-Albert (preferential attachment, scale-free) edge list, ~50k edges."""
import networkx as nx, numpy as np, sys
n, m, seed = 25000, 2, 42
G = nx.barabasi_albert_graph(n, m, seed=seed)
rng = np.random.default_rng(seed)
E = sorted(map(lambda e: (min(e), max(e)), G.edges()))
W = rng.integers(1, 101, size=len(E))
base = "/home/user/Course/msk1/2nd/Graph-language-/algo_validation/ba/ba_25000_%d" % len(E)
with open(base + ".txt", "w") as fu, open(base + "_w.txt", "w") as fw:
    for (a, b), w in zip(E, W):
        fu.write("%d %d\n" % (a, b)); fw.write("%d %d %d\n" % (a, b, w))
deg = np.array([d for _, d in G.degree()])
core = nx.core_number(G)
ecc = nx.eccentricity(G, v=0)
print("BA n=%d m_param=%d edges=%d" % (n, m, len(E)))
print("  avg_deg=%.2f  max_deg=%d  isolated=%d" % (deg.mean(), deg.max(), (deg == 0).sum()))
print("  degeneracy=%d" % max(core.values()))
print("  eccentricity(from 0)=%d  connected=%s" % (ecc, nx.is_connected(G)))
print("  5-core size=%d   2-core size=%d" % (sum(1 for c in core.values() if c >= 5),
                                             sum(1 for c in core.values() if c >= 2)))
print("  degree pctiles 50/90/99/max: %d %d %d %d"
      % tuple(np.percentile(deg, [50, 90, 99]).astype(int).tolist() + [deg.max()]))
print("written:", base + ".txt")
