#!/usr/bin/env python3
"""Build a graph corpus for the WACO vs AutoTuner comparison.

Generates synthetic ER / BA / RMAT graphs, writes SGPL edge lists and WACO
CSR files, checks equivalence, and emits an 80/20 graph-level train/test split.
"""
from __future__ import annotations

import argparse
import json
import os
import random
import struct
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "data"
EDGE = DATA / "edge_lists"
CSR = DATA / "csr"
SPLITS = DATA / "splits"


def write_edge_list(path: Path, edges: np.ndarray, n: int) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w") as f:
        f.write(f"# n={n} m={len(edges)}\n")
        for u, v in edges:
            f.write(f"{int(u)} {int(v)}\n")


def write_waco_csr(path: Path, n: int, edges: np.ndarray) -> None:
    """WACO artifact .csr: binary CSR (row_ptr int64, col_idx int32)."""
    path.parent.mkdir(parents=True, exist_ok=True)
    # Sort edges by (u,v) and build CSR
    order = np.lexsort((edges[:, 1], edges[:, 0]))
    e = edges[order]
    deg = np.bincount(e[:, 0], minlength=n).astype(np.int64)
    row_ptr = np.zeros(n + 1, dtype=np.int64)
    row_ptr[1:] = np.cumsum(deg)
    col_idx = e[:, 1].astype(np.int32)
    with path.open("wb") as f:
        f.write(struct.pack("<qq", n, int(row_ptr[-1])))
        f.write(row_ptr.tobytes())
        f.write(col_idx.tobytes())


def edges_from_csr(path: Path) -> tuple[int, np.ndarray]:
    with path.open("rb") as f:
        n, m = struct.unpack("<qq", f.read(16))
        row_ptr = np.frombuffer(f.read((n + 1) * 8), dtype=np.int64)
        col_idx = np.frombuffer(f.read(m * 4), dtype=np.int32)
    edges = []
    for u in range(n):
        for j in range(row_ptr[u], row_ptr[u + 1]):
            edges.append((u, int(col_idx[j])))
    return n, np.asarray(edges, dtype=np.int32)


def check_equivalence(edge_path: Path, csr_path: Path) -> bool:
    edges = []
    with edge_path.open() as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            a, b = line.split()[:2]
            edges.append((int(a), int(b)))
    edges = np.asarray(edges, dtype=np.int32)
    n_csr, csr_edges = edges_from_csr(csr_path)
    if edges.size == 0 and csr_edges.size == 0:
        return True
    a = set(map(tuple, edges.tolist()))
    b = set(map(tuple, csr_edges.tolist()))
    return a == b and (edges.max() if len(edges) else -1) < n_csr


def gen_er(n: int, avg_deg: float, seed: int, directed: bool = False) -> np.ndarray:
    rng = np.random.default_rng(seed)
    m = int(n * avg_deg if directed else n * avg_deg / 2)
    edges = set()
    while len(edges) < m:
        u = int(rng.integers(0, n))
        v = int(rng.integers(0, n))
        if u == v:
            continue
        if directed:
            edges.add((u, v))
        else:
            a, b = (u, v) if u < v else (v, u)
            edges.add((a, b))
    arr = np.asarray(sorted(edges), dtype=np.int32)
    if not directed:
        # SGPL undirected loaders typically expect one orientation; emit both
        # for directed CSR equivalence used by WACO SpMV/SpMM.
        rev = arr[:, ::-1]
        arr = np.vstack([arr, rev])
        arr = np.unique(arr, axis=0)
    return arr


def gen_ba(n: int, m0: int, seed: int) -> np.ndarray:
    """Preferential-attachment undirected BA; emit bidirectional edges."""
    rng = np.random.default_rng(seed)
    m0 = max(1, min(m0, n - 1))
    edges = []
    deg = np.zeros(n, dtype=np.float64)
    # Start with a star on m0+1 nodes
    for v in range(1, m0 + 1):
        edges.append((0, v))
        deg[0] += 1
        deg[v] += 1
    for v in range(m0 + 1, n):
        # sample m0 distinct targets proportional to degree
        probs = deg[:v].copy()
        s = probs.sum()
        if s <= 0:
            probs[:] = 1.0
            s = float(v)
        probs /= s
        targets = rng.choice(v, size=min(m0, v), replace=False, p=probs)
        for u in targets:
            edges.append((int(u), v))
            deg[u] += 1
            deg[v] += 1
    arr = np.asarray(edges, dtype=np.int32)
    rev = arr[:, ::-1]
    arr = np.unique(np.vstack([arr, rev]), axis=0)
    return arr


def gen_rmat(n: int, m: int, seed: int, a=0.57, b=0.19, c=0.19) -> np.ndarray:
    """Recursive RMAT-like edge generator (undirected, bidirectional)."""
    rng = np.random.default_rng(seed)
    # round n up to power of two for quiet recursion
    bits = max(1, int(np.ceil(np.log2(max(n, 2)))))
    N = 1 << bits
    edges = set()
    d = 1.0 - (a + b + c)
    while len(edges) < m:
        u = 0
        v = 0
        for _ in range(bits):
            r = rng.random()
            if r < a:
                pass
            elif r < a + b:
                v += 1
            elif r < a + b + c:
                u += 1
            else:
                u += 1
                v += 1
            u <<= 1
            v <<= 1
        u >>= 1
        v >>= 1
        if u >= n or v >= n or u == v:
            continue
        x, y = (u, v) if u < v else (v, u)
        edges.add((x, y))
    arr = np.asarray(sorted(edges), dtype=np.int32)
    rev = arr[:, ::-1]
    return np.unique(np.vstack([arr, rev]), axis=0)


def build(args: argparse.Namespace) -> None:
    EDGE.mkdir(parents=True, exist_ok=True)
    CSR.mkdir(parents=True, exist_ok=True)
    SPLITS.mkdir(parents=True, exist_ok=True)

    records = []
    seed0 = args.seed
    # Size ladder capped for RTX 3050 / 16GB RAM (~2M edges max)
    sizes = [
        (200, 4),
        (500, 6),
        (1000, 8),
        (2000, 8),
        (5000, 10),
        (10000, 8),
        (20000, 6),
        (50000, 4),
    ]
    idx = 0
    for family, gen in (("er", "er"), ("ba", "ba"), ("rmat", "rmat")):
        for n, deg in sizes:
            for rep in range(args.reps):
                seed = seed0 + idx * 17 + rep
                if gen == "er":
                    edges = gen_er(n, deg, seed)
                elif gen == "ba":
                    edges = gen_ba(n, max(1, deg // 2), seed)
                else:
                    edges = gen_rmat(n, int(n * deg / 2), seed)
                # Cap edges
                if len(edges) > args.max_edges:
                    edges = edges[: args.max_edges]
                name = f"{family}_n{n}_d{deg}_r{rep}"
                epath = EDGE / f"{name}.txt"
                cpath = CSR / f"{name}.csr"
                write_edge_list(epath, edges, n)
                write_waco_csr(cpath, n, edges)
                ok = check_equivalence(epath, cpath)
                records.append(
                    {
                        "name": name,
                        "family": family,
                        "n": n,
                        "m": int(len(edges)),
                        "avg_deg": float(len(edges) / max(n, 1)),
                        "edge_list": str(epath),
                        "csr": str(cpath),
                        "equiv_ok": bool(ok),
                    }
                )
                idx += 1
                if len(records) >= args.limit:
                    break
            if len(records) >= args.limit:
                break
        if len(records) >= args.limit:
            break

    # Stratified 80/20 by family then size bucket
    rng = random.Random(args.seed)
    by_key: dict[str, list] = {}
    for r in records:
        key = f"{r['family']}:{int(np.log10(max(r['n'], 1)))}"
        by_key.setdefault(key, []).append(r)
    train, test = [], []
    for key, group in by_key.items():
        rng.shuffle(group)
        k = max(1, int(round(0.2 * len(group)))) if len(group) >= 5 else max(1, len(group) // 5 or 1)
        # ensure at least one train when possible
        if len(group) == 1:
            train.extend(group)
        else:
            test.extend(group[:k])
            train.extend(group[k:])

    meta = {
        "n_graphs": len(records),
        "n_train": len(train),
        "n_test": len(test),
        "max_edges": args.max_edges,
        "equiv_failures": sum(1 for r in records if not r["equiv_ok"]),
    }
    (DATA / "corpus.json").write_text(json.dumps({"meta": meta, "graphs": records}, indent=2))
    (SPLITS / "train.json").write_text(json.dumps(train, indent=2))
    (SPLITS / "test.json").write_text(json.dumps(test, indent=2))
    print(json.dumps(meta, indent=2))
    print(f"wrote {DATA / 'corpus.json'}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--reps", type=int, default=2, help="replicates per size/family")
    ap.add_argument("--limit", type=int, default=96, help="max graphs")
    ap.add_argument("--max-edges", type=int, default=2_000_000)
    ap.add_argument("--seed", type=int, default=42)
    args = ap.parse_args()
    build(args)


if __name__ == "__main__":
    main()
