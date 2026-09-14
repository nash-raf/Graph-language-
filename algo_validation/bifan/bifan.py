#!/usr/bin/env python3
"""Bi-fan census for a layered MLP, per epoch, adjacent layers only.

Bi-fan = s1,s2 (layer L) both feeding t1,t2 (layer L+1), all four edges present.
For a bipartite layer pair with binary adjacency A (sources x targets):
    M = A @ A.T          # M[i][j] = number of targets shared by sources i and j
    bifans = sum_{i<j} C(M[i][j], 2)
Verified against brute-force enumeration.  One matmul per layer pair.
"""
import numpy as np

def binarize(W, rule="abs", value=0.01):
    """W: (n_src, n_dst) raw weights -> boolean adjacency.
    rule='abs'      : |w| >= value
    rule='pos'      : w  >= value
    rule='density'  : keep the top `value` fraction by |w|
    rule='topk'     : keep the top `value` outgoing edges per source neuron
    """
    if rule == "abs":     return np.abs(W) >= value
    if rule == "pos":     return W >= value
    if rule == "density":
        k = max(1, int(round(value * W.size)))
        thr = np.partition(np.abs(W).ravel(), -k)[-k]
        return np.abs(W) >= thr
    if rule == "topk":
        k = int(value)
        idx = np.argpartition(-np.abs(W), k-1, axis=1)[:, :k]
        A = np.zeros(W.shape, bool)
        np.put_along_axis(A, idx, True, axis=1)
        return A
    raise ValueError(rule)

def bifan_count(A):
    """Number of bi-fans in one bipartite layer pair."""
    M = (A.astype(np.int32) @ A.astype(np.int32).T)   # shared-target counts
    iu = np.triu_indices(M.shape[0], k=1)             # unordered source pairs
    m = M[iu].astype(np.int64)
    return int((m * (m - 1) // 2).sum())

def census(weight_list, rule="abs", value=0.01):
    """weight_list: [W_0, ..., W_14] for a 16-layer net. Returns per-pair counts."""
    out = []
    for W in weight_list:
        A = binarize(W, rule, value)
        out.append((int(A.sum()), bifan_count(A)))     # (edges, bifans)
    return out
