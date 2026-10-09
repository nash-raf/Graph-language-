#!/usr/bin/env python3
"""Reduced-scale recreation of the Fig. 5 / Table II verification.

For each gamma and each network size N:
  - draw out-degrees from the paper's power law P(k) ~ (k+k0)^-gamma,
    tuned so <K> = 1.2 (Eq. 7 with k0 solved numerically);
  - draw in-degrees from a compact (Poisson) distribution with the same total;
  - wire out-stubs to in-stubs at random (Newman-Strogatz-Watts construction),
    repairing self-loops and duplicate directed edges;
  - count the 13 induced 3-node subgraphs with the Table III formulas.
Then fit  alpha_hat = slope of log(mean count) vs log(N)  and compare with
alpha from Eq. (16):

   alpha = n-g+s-1 (gamma<=2);  n-g+s-gamma+1 (2<gamma<s+1);  n-g (gamma>=s+1).

Usage:  python3 table2_simulation.py [--reps 300] [--sizes 100,300,1000,3000]
                                  [--gammas 1.5,2.5,4.0] [--out results/]
"""

import argparse
import csv
import json
import os
import sys

import numpy as np
import scipy.sparse as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from triad_counter import triad_counts, IDS  # noqa: E402

# n, g, s for the 13 three-node subgraphs (paper Table II)
NGC = {
    6: (3, 2, 2), 12: (3, 2, 1), 14: (3, 3, 2), 36: (3, 2, 1), 38: (3, 3, 2),
    46: (3, 4, 2), 74: (3, 3, 1), 78: (3, 4, 2), 98: (3, 3, 1), 102: (3, 4, 2),
    108: (3, 4, 2), 110: (3, 5, 2), 238: (3, 6, 2),
}
NAMES = {
    6: "out-star", 12: "three-chain", 14: "out-star+mutual", 36: "in-star",
    38: "feed-forward-loop", 46: "bi-parallel(3)", 74: "in-star+mutual",
    78: "two-mutual path", 98: "three-cycle", 102: "ffl+mutual",
    108: "mutual+2-in", 110: "two-mutual+single", 238: "three-mutual",
}


def alpha_theory(sid, gamma):
    n, g, s = NGC[sid]
    if gamma <= 2:
        return n - g + s - 1
    if gamma < s + 1:
        return n - g + s - gamma + 1
    return n - g


def sample_out_degrees(N, gamma, mean_k=1.2, kmax=None, rng=None):
    """Discrete P(k) ~ (k+k0)^-gamma on k=0..kmax with mean_k, solved for k0."""
    kmax = min(kmax or N - 1, 4000)
    ks = np.arange(0, kmax + 1)

    def mean_for(k0):
        w = (ks + k0).astype(float) ** (-gamma)
        w /= w.sum()
        return float((w * ks).sum())

    lo, hi = 1e-6, 1e6
    for _ in range(200):
        mid = (lo + hi) / 2
        if mean_for(mid) < mean_k:
            lo = mid
        else:
            hi = mid
    k0 = (lo + hi) / 2
    w = (ks + k0).astype(float) ** (-gamma)
    w /= w.sum()
    return rng.choice(ks, size=N, p=w)


def build_network(N, gamma, rng):
    k = sample_out_degrees(N, gamma, rng=rng)
    m = int(k.sum())
    if m == 0:
        return np.zeros((0, 2), dtype=np.int64), 0
    # in-degrees: Poisson(mean 1.2), adjusted to the same total
    r = rng.poisson(1.2, size=N)
    diff = m - int(r.sum())
    tries = 0
    while diff != 0 and tries < 10000:
        i = rng.integers(0, N)
        if diff > 0 and r[i] < N - 1:
            r[i] += 1
            diff -= 1
        elif diff < 0 and r[i] > 0:
            r[i] -= 1
            diff += 1
        tries += 1
    # stubs
    outs = np.repeat(np.arange(N), k)
    ins = np.repeat(np.arange(N), r)
    rng.shuffle(outs)
    rng.shuffle(ins)
    used = set()
    edges = []
    dropped = 0
    L = len(outs)
    bad = []
    for idx in range(L):
        u, v = int(outs[idx]), int(ins[idx])
        if u == v or (u, v) in used:
            bad.append(idx)
            continue
        used.add((u, v))
        edges.append((u, v))
    # repair pass: swap in-stubs of bad pairs with random positions until valid
    for idx in bad:
        fixed = False
        u = int(outs[idx])
        for _ in range(256):
            j = int(rng.integers(0, L))
            if j == idx:
                continue
            uj = int(outs[j])
            vi = int(ins[idx])
            vj = int(ins[j])
            if (u, vj) in used or u == vj or (uj, vi) in used or uj == vi:
                continue
            ins[idx], ins[j] = vj, vi
            used.add((u, vj))
            if (outs[j], int(ins[j])) != (u, vj):
                used.add((uj, vi))
            edges.append((u, vj))
            if (outs[j], int(ins[j])) not in used or True:
                edges.append((uj, int(ins[j])))
            fixed = True
            break
        if not fixed:
            dropped += 1
    return np.array(edges, dtype=np.int64), dropped


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--reps", type=int, default=300)
    ap.add_argument("--sizes", default="100,300,1000,3000")
    ap.add_argument("--gammas", default="1.5,2.5,4.0")
    ap.add_argument("--seed", type=int, default=11)
    ap.add_argument("--out", default=os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "results"))
    args = ap.parse_args()

    sizes = [int(s) for s in args.sizes.split(",")]
    gammas = [float(g) for g in args.gammas.split(",")]
    rng = np.random.default_rng(args.seed)
    os.makedirs(args.out, exist_ok=True)

    means = {}          # (gamma, N, sid) -> mean count
    drops = {}
    for gamma in gammas:
        for N in sizes:
            acc = {i: 0.0 for i in IDS}
            drop = 0
            for rep in range(args.reps):
                edges, dropped = build_network(N, gamma, rng)
                drop += dropped
                M = sp.csr_matrix(
                    (np.ones(len(edges), dtype=np.int64), (edges[:, 0], edges[:, 1])),
                    shape=(N, N))
                c = triad_counts(M)
                for i in IDS:
                    acc[i] += c[i]
            for i in IDS:
                means[(gamma, N, i)] = acc[i] / args.reps
            drops[(gamma, N)] = drop
            print(f"gamma={gamma} N={N} reps={args.reps} "
                  f"dropped_pairs={drop}")

    # fits
    rows = []
    print(f"\n{'subgraph':>22} {'gamma':>5} {'alpha_theory':>12} {'alpha_fit':>10} "
          f"{'mean counts (N)':>32}")
    for gamma in gammas:
        for sid in IDS:
            xs, ys = [], []
            cnts = []
            for N in sizes:
                c = means[(gamma, N, sid)]
                cnts.append(c)
                if c > 0:
                    xs.append(np.log(N))
                    ys.append(np.log(c))
            if len(xs) >= 2:
                slope = float(np.polyfit(xs, ys, 1)[0])
            else:
                slope = float("nan")
            th = alpha_theory(sid, gamma)
            rows.append(dict(gamma=gamma, id=sid, name=NAMES[sid],
                             alpha_theory=th, alpha_fit=round(slope, 3),
                             counts=";".join(f"{c:.2f}" for c in cnts)))
            print(f"{NAMES[sid]:>22} {gamma:>5} {th:>12} {slope:>10.3f} "
                  f"{' '.join(f'{c:.2f}' for c in cnts):>32}")

    with open(os.path.join(args.out, "table2_simulation_fits.csv"), "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)
    with open(os.path.join(args.out, "table2_simulation_means.json"), "w") as f:
        json.dump({f"{g}|{N}|{i}": means[(g, N, i)]
                   for g in gammas for N in sizes for i in IDS}, f, indent=0)
    with open(os.path.join(args.out, "table2_simulation_meta.json"), "w") as f:
        json.dump(dict(reps=args.reps, sizes=sizes, gammas=gammas,
                       seed=args.seed, drops={f"{g}|{N}": d for (g, N), d in drops.items()}),
                  f, indent=1)
    print("\nwrote results/table2_simulation_fits.csv, _means.json, _meta.json")


if __name__ == "__main__":
    main()
