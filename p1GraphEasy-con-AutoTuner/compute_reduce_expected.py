#!/usr/bin/env python3
"""compute_reduce_expected.py — independent ground-truth for the reduction tests.

Reads a reduce_*.graph program and its edge list, folds the neighbor-iteration
semantics in plain Python (vertex order ascending, neighbors in CSR/file order,
op applied to each visited v), and prints the serial result.  This is the
reference that validate_reduction.sh compares the compiler's emitted red step
against.  The compiler's graph is directed when the .graph body declares
`directed: true;`, in which case each edge (u,v) is visited once from u;
otherwise the edge list is symmetrized and each undirected edge is visited
twice.
"""
import re
import sys


def parse_graph(path):
    text = open(path, encoding="utf-8").read()
    op = None
    init = None
    directed = bool(re.search(r"\bdirected\s*:\s*true\b", text, re.I))
    m = re.search(r"\bint\s+acc\s*=\s*(-?\d+)", text)
    if m:
        init = int(m.group(1))
    m = re.search(r"\bacc\s*=\s*acc\s*([&|^+\-*/])", text)
    if m:
        op = m.group(1)
    else:
        m = re.search(r"\bif\s*\(\s*v\s*([<>])\s*acc\s*\)", text)
        if m:
            op = "min" if m.group(1) == "<" else "max"
    if op is None or init is None:
        raise SystemExit(f"could not parse reduction (op={op!r} init={init!r}) from {path}")
    return init, op, directed


def build_csr(edges_path, directed):
    adj = {}
    for line in open(edges_path, encoding="utf-8"):
        parts = line.split()
        if len(parts) < 2:
            continue
        u, v = int(parts[0]), int(parts[1])
        adj.setdefault(u, []).append(v)
        if not directed:
            adj.setdefault(v, []).append(u)
    return adj


def signed_i32(x):
    x &= 0xFFFFFFFF
    return x - 0x100000000 if x >= 0x80000000 else x


def fold(init, op, adj):
    acc = init
    n = max(adj.keys()) + 1 if adj else 0
    for u in range(n):
        for v in adj.get(u, ()):
            if op == "&":
                acc &= v
            elif op == "|":
                acc |= v
            elif op == "^":
                acc ^= v
            elif op == "+":
                acc += v
            elif op == "-":
                acc -= v
            elif op == "*":
                acc *= v
            elif op == "min":
                if v < acc:
                    acc = v
            elif op == "max":
                if v > acc:
                    acc = v
    return signed_i32(acc)


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: compute_reduce_expected.py GRAPH_FILE EDGES_FILE")
    init, op, directed = parse_graph(sys.argv[1])
    adj = build_csr(sys.argv[2], directed)
    result = fold(init, op, adj)
    print(result)


if __name__ == "__main__":
    main()