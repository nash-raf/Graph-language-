#!/usr/bin/env python3
"""Independent Python/scipy references for the /algo programs on the 2M/16M graph.
Run:  python3 reference.py     -- prints the values the DSL must reproduce."""
import numpy as np, load
from scipy.sparse.csgraph import connected_components, dijkstra
n = 2000000
u, v, w = load.load()
A = load.csr(n, u, v)
deg = np.diff(A.indptr)
print("graph: n=%d m=%d avg_deg=%.2f max_deg=%d isolated=%d"
      % (n, len(u), deg.mean(), deg.max(), (deg == 0).sum()))

# --- bfs_parent / bfs_level : BFS from vertex 0 -------------------------------
d0 = dijkstra(A, directed=False, indices=0, unweighted=True)
fin = np.isfinite(d0)
print("bfs   reached        =", int(fin.sum()))
print("bfs   level_checksum =", int(d0[fin].astype(np.int64).sum() % 1000000007))

# --- cc : min-label propagation ----------------------------------------------
ncomp, lab = connected_components(A, directed=False)
minlab = np.full(ncomp, 1 << 62, dtype=np.int64)
np.minimum.at(minlab, lab, np.arange(n))
idarr = minlab[lab]
print("cc    components     =", int((idarr == np.arange(n)).sum()))
print("cc    label_checksum =", int(idarr.sum() % 1000000007))

# --- kcore : 5-core -----------------------------------------------------------
k = 5
dg = deg.astype(np.int64).copy(); alive = np.ones(n, bool)
src = np.repeat(np.arange(n, dtype=np.int32), deg)
while True:
    peel = alive & (dg < k)
    if not peel.any(): break
    alive[peel] = False
    nb = A.indices[peel[src]]
    dg -= np.bincount(nb[alive[nb]], minlength=n)
print("kcore kcore_size     =", int(alive.sum()))

# --- pagerank : 20 iterations, damping 0.85 -----------------------------------
Af = A.astype(np.float64); degf = deg.astype(np.float64)
damp = 0.85; beta = (1.0 - damp) / n; cur = np.full(n, 1.0 / n)
for _ in range(20):
    cur = beta + Af @ (damp * cur / degf)
print("pr    rank_total     = %.17g" % cur.sum())
print("pr    rank_0         = %.17g" % cur[0])
print("pr    rank_1         = %.17g" % cur[1])

# --- sssp : Dijkstra from vertex 1 -------------------------------------------
Aw = load.csr(n, u, v, w.astype(np.float64))
d1 = dijkstra(Aw, directed=False, indices=1)
fin1 = np.isfinite(d1)
print("sssp  reachable      =", int(fin1.sum()))
print("sssp  dist_checksum  =", int(d1[fin1].astype(np.int64).sum() % 1000000007))
