#!/usr/bin/env python3
"""Eq. (5) expectations from a directed edge list — the Table I model.

Milo, Kashtan, Itzkovitz, Ziv, Alon, "Subgraphs in random networks",
arXiv:cond-mat/0302375, Eq. (5) + Appendix B corrections.

Degree split (the paper's convention, validated against their C. elegans
"neurons" column):
  K = single out-degree, R = single in-degree, M = mutual degree
  (an edge whose reverse also exists contributes to M instead of K/R)
Self-loops are excluded.

Outputs the 13 induced ("*"-corrected) expectations.
"""

import sys
import numpy as np


def edges_from_file(path):
    e = set()
    for line in open(path):
        if line.startswith("#"):
            continue
        p = line.split()
        if len(p) < 2:
            continue
        u, v = int(p[0]), int(p[1])
        if u != v:
            e.add((u, v))
    return e


def moments(edges):
    ids = sorted({x for e in edges for x in e})
    remap = {v: i for i, v in enumerate(ids)}
    n = len(ids)
    M = np.zeros((n, n), dtype=np.int64)
    for u, v in edges:
        M[remap[u], remap[v]] = 1
    S = (M.T & M)
    A = (M - S).astype(np.int64)
    K = A.sum(1).astype(float)   # single out-degree
    R = A.sum(0).astype(float)   # single in-degree
    Mdeg = S.sum(1).astype(float)  # mutual degree
    N = float(n)

    mK, mR, mM = K.mean(), R.mean(), Mdeg.mean()
    mKK1 = (K * (K - 1)).mean()
    mRR1 = (R * (R - 1)).mean()
    mMM1 = (Mdeg * (Mdeg - 1)).mean()
    mKR = (K * R).mean()
    mKM = (K * Mdeg).mean()
    mRM = (R * Mdeg).mean()

    dK1, dK2, dK3 = mK, mK ** 2, mK ** 3
    dM1, dM2 = mM, mM ** 2

    e = {}
    e[6] = N * mKK1 / 2
    e[12] = N * mKR
    e[14] = N * mKM
    e[36] = N * mRR1 / 2
    e[38] = mKK1 * mKR * mRR1 / dK3 if dK3 else 0.0
    # id98: the paper prints the c=2 divisor; the symmetry-factor definition and
    # Appendix C both specify c=3 (see the reproduction write-up).
    e[98] = mKR ** 3 / (3 * dK3) if dK3 else 0.0
    e[46] = e[74] = e[78] = e[102] = e[108] = e[110] = e[238] = 0.0
    if dM1 > 0:
        e[46] = mKM * mKM * mRR1 / (2 * dK2 * dM1)
        e[74] = N * mRM
        e[78] = N * mMM1 / 2
        e[102] = mKM * mRM * mKR / (dK2 * dM1)
        e[108] = mRM * mRM * mKK1 / (2 * dK2 * dM1)
        if dM2 > 0:
            e[110] = mKM * mRM * mMM1 / (dK1 * dM2)
            e[238] = mMM1 ** 3 / (6 * dM2 * dM1)

    # Appendix B induced corrections
    i = dict(e)
    i[6] = e[6] - e[38] - e[108]
    i[12] = e[12] - e[38] - e[102]
    i[14] = e[14] - e[46] - e[102] - e[110]
    i[36] = e[36] - e[38] - e[46]
    i[74] = e[74] - e[102] - e[108] - e[110]
    i[78] = e[78] - e[110] - e[238]
    return dict(n=n, edges=len(edges), mK=mK, mR=mR, mM=mM), i


if __name__ == "__main__":
    stats, i = moments(edges_from_file(sys.argv[1]))
    print("file:", sys.argv[1])
    print("nodes:", stats["n"], "edges:", stats["edges"],
          "mean K=%.4f R=%.4f M=%.4f" % (stats["mK"], stats["mR"], stats["mM"]))
    for k in [6, 12, 14, 36, 38, 46, 74, 78, 98, 102, 108, 110, 238]:
        print(f"i{k} {i[k]:.6f}")
