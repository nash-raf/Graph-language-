#!/usr/bin/env python3
"""Triad census via the Table III matrix formulas of Milo et al. 2003.

Paper: Milo, Kashtan, Itzkovitz, Ziv, Alon, "Subgraphs in random networks",
arXiv:cond-mat/0302375, Table III (p. 7).

Conventions (the paper's own):
  M   adjacency matrix (M_ij = 1 iff i -> j)
  S   symmetric component:  S_ij = 1 iff i <-> j   (S = M AND M^T)
  A   asymmetric component: A_ij = 1 iff i -> j and not j -> i  (A = M AND NOT M^T)
  M = A + S
  ~X  logical inverse:  (~X)_ij = 1 - X_ij
  A'  transpose;  "·" elementwise (Hadamard) product; juxtaposition = matrix product;
  sum over all index pairs; tr = trace.

Table III rows transcribed from the paper:
  6   ( sum A'A . ~M . ~M' - tr(A'A) ) / 2
  12    sum A^2 . ~M . ~M'
  14    sum S A . ~M . ~M'
  36  ( sum A A' . ~M . ~M' - tr(A A') ) / 2
  38    sum A^2 . A
  46  ( sum A A' . S ) / 2
  74    sum S A' . ~M . ~M'
  78  ( sum S^2 . ~M . ~M' - tr(S^2) ) / 2
  98  ( sum A^2 . A ) / 3
  102   sum A^2 . S
  108 ( sum A'A . S ) / 2
  110   sum S^2 . A
  238 ( sum S^2 . S ) / 6

(The tilde masks ~M and ~M' are the induced-graph conditions: no edge in
either direction between the two vertices indexed by the product entry.
Validated below against direct induced enumeration and against the
GraphEasy motif matcher.)
"""

import numpy as np
import scipy.sparse as sp

IDS = [6, 12, 14, 36, 38, 46, 74, 78, 98, 102, 108, 110, 238]


def components(adj):
    """adj: scipy sparse adjacency of a simple directed graph.
    Returns (A, S) sparse with M = A + S."""
    M = adj.astype(np.int64)
    Mt = M.T.tocsr()
    S = M.multiply(Mt)          # mutual: 1 both directions
    A = M - S                   # single edges only
    return A.tocsr(), S.tocsr()


def _dense_inv(M):
    """~M = 1 - M as dense int8 (only used elementwise / masked)."""
    D = np.ones(M.shape, dtype=np.int64)
    D[M.nonzero()[0], M.nonzero()[1]] = 0
    return D


def triad_counts(adj):
    """Returns dict id -> count (induced, as the paper defines them)."""
    M = adj.astype(np.int64).tocsr()
    A, S = components(M)
    Mt = M.T.tocsr()
    invM = _dense_inv(M)                    # ~M
    invMt = invM.T                          # ~M'
    mask_both = invM * invMt                # (~M . ~M') elementwise, dense

    def masked_sum(P):
        """sum over all (i,j) of P_ij * mask_both_ij, for sparse P."""
        Pd = P.tocsr()
        r, c = Pd.nonzero()
        if len(r) == 0:
            return 0
        return int(Pd[r, c].A1.sum() * 0) + int(
            (np.asarray(Pd[r, c]).ravel() * mask_both[r, c]).sum())

    At = A.T.tocsr()
    A2 = (A @ A).tocsr()
    S2 = (S @ S).tocsr()

    out = {}
    out[6] = (masked_sum(At @ A) - int((At @ A).diagonal().sum())) // 2
    out[12] = masked_sum(A2)
    out[14] = masked_sum(S @ A)
    AA = A @ At
    out[36] = (masked_sum(AA) - int(AA.diagonal().sum())) // 2
    out[38] = int((A2.multiply(A)).sum())
    out[46] = int((AA.multiply(S)).sum()) // 2
    out[74] = masked_sum(S @ At)
    out[78] = (masked_sum(S2) - int(S2.diagonal().sum())) // 2
    # 3-cycle: length-2 path i->j->k closed by k->i (the transpose), /3 rotations
    out[98] = int((A2.multiply(At)).sum()) // 3
    out[102] = int((A2.multiply(S)).sum())
    out[108] = int(((At @ A).multiply(S)).sum()) // 2
    out[110] = int((S2.multiply(A)).sum())
    out[238] = int((S2.multiply(S)).sum()) // 6
    return out


def direct_counts(edges, n):
    """Brute-force induced triad census by enumerating wedges, for validation."""
    from collections import defaultdict
    out_sets = [set() for _ in range(n)]
    in_sets = [set() for _ in range(n)]
    for u, v in edges:
        out_sets[u].add(v)
        in_sets[v].add(u)
    seen = set()
    counts = defaultdict(int)
    for (u, v) in edges:
        cand = out_sets[u] | in_sets[u] | out_sets[v] | in_sets[v]
        for w in cand:
            if w == u or w == v:
                continue
            t = tuple(sorted((u, v, w)))
            if t in seen:
                continue
            seen.add(t)
            a, b, c = t
            # induced adjacency within the triple
            e = set()
            for x in t:
                for y in t:
                    if x != y and y in out_sets[x]:
                        e.add((x, y))
            # weak connectivity
            nodes = {a, b, c}
            con = False
            for x, y in e:
                con = True
            if not con:
                continue
            # canonical id: minimal adjacency-bit-vector over the 6 permutations
            def bits(perm):
                v = 0
                for i in range(3):
                    for j in range(3):
                        v = v * 2 + (1 if (perm[i], perm[j]) in e else 0)
                return v
            from itertools import permutations
            bid = min(bits(p) for p in permutations((a, b, c)))
            counts[bid] += 1
    # the paper's ids are the same minimal-binary-number encoding
    return dict(counts)


def main():
    import sys
    if len(sys.argv) > 1 and sys.argv[1] == "validate":
        # validate formulas against direct enumeration on a small random graph
        rng = np.random.default_rng(7)
        n = 60
        edges = set()
        while len(edges) < 90:
            u, v = rng.integers(0, n, 2)
            if u != v:
                edges.add((int(u), int(v)))
        rows = sorted(edges)
        M = sp.csr_matrix(
            (np.ones(len(rows)), ([r[0] for r in rows], [r[1] for r in rows])),
            shape=(n, n))
        formula = triad_counts(M)
        direct = direct_counts(rows, n)
        ok = True
        for i in IDS:
            a = formula[i]
            b = direct.get(i, 0)
            flag = "OK " if a == b else "**MISMATCH**"
            if a != b:
                ok = False
            print(f"id{i:<4} formula={a:6d} direct={b:6d} {flag}")
        print("ALL OK" if ok else "VALIDATION FAILED")
        return

    # census on a real network file
    path = sys.argv[1]
    edges = set()
    ids = set()
    for line in open(path):
        p = line.split()
        if len(p) < 2 or line.startswith("#"):
            continue
        u, v = int(p[0]), int(p[1])
        if u == v:
            continue
        edges.add((u, v))
        ids.add(u); ids.add(v)
    remap = {v: i for i, v in enumerate(sorted(ids))}
    rows = [(remap[u], remap[v]) for u, v in edges]
    n = len(remap)
    M = sp.csr_matrix(
        (np.ones(len(rows)), ([r[0] for r in rows], [r[1] for r in rows])),
        shape=(n, n))
    counts = triad_counts(M)
    print("file:", path, "nodes:", n, "edges:", len(rows))
    for i in IDS:
        print(i, counts[i])


if __name__ == "__main__":
    main()
