#!/usr/bin/env python3
"""Independent scipy/numpy references for the verification fixtures.
Regenerates expected/golden.json.  Run this only when a fixture changes."""
import json, numpy as np, pandas as pd, scipy.sparse as sp
from scipy.sparse.csgraph import connected_components, dijkstra
import os
HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
n = 20000
d = pd.read_csv(f"{HERE}/fixtures/g20k_w.txt", sep=' ', header=None, dtype=np.int64).values
u, v, w = d[:, 0], d[:, 1], d[:, 2]
U, V = np.concatenate([u, v]), np.concatenate([v, u])
A = sp.csr_matrix((np.ones(len(U), np.int64), (U, V)), shape=(n, n))
Aw = sp.csr_matrix((np.concatenate([w, w]).astype(float), (U, V)), shape=(n, n))
deg = np.diff(A.indptr)
g = {}

d0 = dijkstra(A, directed=False, indices=0, unweighted=True); fin = np.isfinite(d0)
g["bfs_reached"] = int(fin.sum())
g["bfs_level_checksum"] = int(d0[fin].astype(np.int64).sum() % 1000000007)

nc, lab = connected_components(A, directed=False)
ml = np.full(nc, 1 << 62, np.int64); np.minimum.at(ml, lab, np.arange(n)); idc = ml[lab]
g["cc_components"] = int((idc == np.arange(n)).sum())
g["cc_label_checksum"] = int(idc.sum() % 1000000007)

k = 5; dg = deg.astype(np.int64).copy(); alive = np.ones(n, bool)
src = np.repeat(np.arange(n, dtype=np.int32), deg)
while True:
    peel = alive & (dg < k)
    if not peel.any(): break
    alive[peel] = False
    nb = A.indices[peel[src]]
    dg -= np.bincount(nb[alive[nb]], minlength=n)
g["kcore5_size"] = int(alive.sum())

Af = A.astype(np.float64); degf = deg.astype(np.float64)
damp = .85; beta = (1 - damp) / n; cur = np.full(n, 1.0 / n)
for _ in range(20): cur = beta + Af @ (damp * cur / degf)
g["pagerank_total"] = float(cur.sum())
g["pagerank_r0"] = float(cur[0]); g["pagerank_r1"] = float(cur[1])

d1 = dijkstra(Aw, directed=False, indices=1); f1 = np.isfinite(d1)
g["sssp_reachable"] = int(f1.sum())
g["sssp_dist_checksum"] = int(d1[f1].astype(np.int64).sum() % 1000000007)

json.dump(g, open(f"{HERE}/expected/golden.json", "w"), indent=2, sort_keys=True)
for kk, vv in sorted(g.items()): print(f"  {kk:24s} {vv}")
