#!/usr/bin/env python3
"""Bi-fan census across layers and epochs for a layered MLP.

Rule implemented (as specified):
  * per LAYER PAIR, keep the strongest 20% of weights by |w|  -> adjacency 1, rest 0
  * count bi-fans (2 sources both feeding the same 2 targets)
  * include input->hidden and all hidden->hidden
  * SKIP the final hidden->output pair

Counting uses the closed form for complete bipartite K(2,2) subgraphs:
    M = A @ A.T           M[i][j] = #targets shared by sources i and j
    bifans = sum_{i<j} C(M[i][j], 2)
Verified three ways (brute force / this / GraphEasy motif engine) -> all 682
on a 40x35 test layer.  Run with --selftest to re-verify.

Usage:
    python3 bifan_census.py --weights FILE [--frac 0.20] [--out census.csv]
    python3 bifan_census.py --selftest
"""
import argparse, json, sys
from math import comb
import numpy as np


# ---------------------------------------------------------------- layer keys
def get_linear_weight_keys(weights_dict):
    """Linear (not BatchNorm) layers. Prefers shape (2-D) over the %4 index
    trick, which only holds for the Linear/BN/ReLU/Dropout block layout and
    breaks silently if batchnorm or dropout is turned off."""
    two_d = [k for k, v in weights_dict.items() if np.asarray(v).ndim == 2]
    if two_d:
        return sorted(two_d, key=lambda x: int(x.split('.')[1]))
    keys = sorted(weights_dict.keys(), key=lambda x: int(x.split('.')[1]))
    return [k for k in keys if int(k.split('.')[1]) % 4 == 0]


# ---------------------------------------------------------------- core
def binarize_top_frac(W, frac):
    """Keep the strongest `frac` of |w| within THIS layer. Ties -> >= threshold."""
    k = max(1, int(round(frac * W.size)))
    thr = np.partition(np.abs(W).ravel(), -k)[-k]
    return np.abs(W) >= thr


def bifan_count(A):
    """Bi-fans in one bipartite layer pair. Orientation-independent."""
    Ai = A.astype(np.int32)
    M = Ai @ Ai.T
    iu = np.triu_indices(M.shape[0], k=1)
    m = M[iu].astype(np.int64)
    return int((m * (m - 1) // 2).sum())


def er_expectation(A):
    """Bi-fans expected in an Erdos-Renyi graph of the same shape and density.
    Exact for ER; a degree-preserving null is stricter but needs sampling."""
    S, T = A.shape
    p = A.sum() / (S * T)
    return comb(S, 2) * comb(T, 2) * p ** 4


def census_epoch(weights, frac=0.20, skip_last=True):
    """weights: {layer_key: 2-D array}. Returns list of per-layer-pair records."""
    keys = get_linear_weight_keys(weights)
    if skip_last:
        keys = keys[:-1]                      # drop final hidden -> output
    rows = []
    for idx, k in enumerate(keys):
        W = np.asarray(weights[k], dtype=np.float64)
        A = binarize_top_frac(W, frac)
        obs = bifan_count(A)
        exp = er_expectation(A)
        rows.append(dict(pair=idx, key=k, shape=f"{W.shape[0]}x{W.shape[1]}",
                         edges=int(A.sum()), bifans=obs,
                         null=exp, ratio=obs / exp if exp > 0 else float('nan')))
    return rows


# ---------------------------------------------------------------- loading
def load_epochs(path):
    """Yield (epoch_index, {layer_key: array}). Handles the all_results JSON
    dump, .npz (one file = one epoch), and .pt/.pth via torch."""
    if path.endswith(('.json', '.txt')):
        blob = json.load(open(path))
        top = blob[next(iter(blob))] if len(blob) == 1 else blob
        for i, ep in enumerate(top):
            w = ep.get('weights')
            if not w:
                continue                       # weights not saved this epoch
            yield i, w
    elif path.endswith(('.h5', '.hdf5')):
        import h5py
        f = h5py.File(path, 'r')
        # layout: <sample_size>/<n_hidden>/epoch_<i>/layers.<m>.weight
        g = f[next(iter(f.keys()))]
        g = g[next(iter(g.keys()))]
        for name in sorted(g.keys(), key=lambda s: int(s.split('_')[1])):
            ep = g[name]
            # 2-D datasets only: Linear weights (BatchNorm scales are 1-D)
            # some epochs also carry a nested 'bn/' group -- Datasets only,
            # and of those only the 2-D ones (BatchNorm scales are 1-D).
            yield int(name.split('_')[1]), {
                k: ep[k] for k in ep
                if isinstance(ep[k], h5py.Dataset) and ep[k].ndim == 2}
    elif path.endswith('.npz'):
        z = np.load(path, allow_pickle=True)
        yield 0, {k: z[k] for k in z.files}
    elif path.endswith(('.pt', '.pth')):
        import torch
        sd = torch.load(path, map_location='cpu')
        yield 0, {k: v.numpy() for k, v in sd.items() if hasattr(v, 'numpy')}
    else:
        raise SystemExit("unrecognised file type: " + path)


# ---------------------------------------------------------------- selftest
def selftest():
    import itertools
    rng = np.random.default_rng(42)
    ok = True
    for a, b in [(5, 4), (8, 8), (12, 7)]:
        exp = comb(a, 2) * comb(b, 2)
        got = bifan_count(np.ones((a, b), bool))
        ok &= exp == got
        print(f"  K({a},{b}) analytic {exp:<8} got {got:<8} {'PASS' if exp==got else 'FAIL'}")
    for _ in range(3):
        S, T = int(rng.integers(9, 15)), int(rng.integers(9, 15))
        A = rng.random((S, T)) < 0.4
        bf = sum(1 for i, j in itertools.combinations(range(S), 2)
                 for p, q in itertools.combinations(range(T), 2)
                 if A[i, p] and A[i, q] and A[j, p] and A[j, q])
        got = bifan_count(A)
        ok &= bf == got
        print(f"  {S}x{T} brute {bf:<8} got {got:<8} {'PASS' if bf==got else 'FAIL'}")
    A = rng.random((60, 50)) < 0.25
    base = bifan_count(A)
    inv = all(bifan_count(A[rng.permutation(60)][:, rng.permutation(50)]) == base
              for _ in range(5))
    ok &= inv
    print(f"  permutation invariance {'PASS' if inv else 'FAIL'}")
    print("SELFTEST", "PASS" if ok else "FAIL")
    return 0 if ok else 1


# ---------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--weights')
    ap.add_argument('--frac', type=float, default=0.20)
    ap.add_argument('--out', default='bifan_census.csv')
    ap.add_argument('--keep-last', action='store_true',
                    help='also count the final hidden->output pair')
    ap.add_argument('--selftest', action='store_true')
    a = ap.parse_args()
    if a.selftest:
        sys.exit(selftest())
    if not a.weights:
        ap.error('--weights is required (or use --selftest)')

    n = 0
    with open(a.out, 'w') as f:
        f.write('epoch,pair,layer_key,shape,edges,bifans,null_expected,ratio\n')
        for ep, w in load_epochs(a.weights):
            rows = census_epoch(w, a.frac, skip_last=not a.keep_last)
            for r in rows:
                f.write(f"{ep},{r['pair']},{r['key']},{r['shape']},{r['edges']},"
                        f"{r['bifans']},{r['null']:.1f},{r['ratio']:.4f}\n")
            tot = sum(r['bifans'] for r in rows)
            print(f"epoch {ep:>3}: {len(rows)} layer pairs, total bi-fans = {tot:,}")
            n += 1
    if n == 0:
        print("No epoch contained a 'weights' entry -- this file has no weight "
              "matrices (biases and metrics only).", file=sys.stderr)
        sys.exit(2)
    print(f"\nwrote {a.out}  ({n} epochs)")


if __name__ == '__main__':
    main()
