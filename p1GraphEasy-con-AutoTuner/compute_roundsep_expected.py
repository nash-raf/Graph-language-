#!/usr/bin/env python3
"""compute_roundsep_expected.py — ground truth for the round-separation tests.

Simulates the round-separated (frozen-read) semantics of the DSL programs
validate_roundsep.sh compiles:

  mode=min  : one sweep, dist[v] = min(dist[v], dist[u]+1), reads frozen.
  mode=add  : one sweep, dist[v] = dist[v] + dist[u], reads frozen.
  mode=sssp : frontier rounds, dist[v] = min(dist[v], dist[u]+w), reads frozen.

This is exactly what the compiler's emitted shadow snapshot produces, so the
E2E harness compares the program output against it (plus determinism across
partitions/threads).
"""
import re
import sys

INF = 2147483647


def read_edges(path, directed, weighted):
    adj = {}
    n = 0
    for line in open(path, encoding="utf-8"):
        p = line.split()
        if weighted:
            if len(p) < 3:
                continue
            u, v, w = int(p[0]), int(p[1]), int(p[2])
            n = max(n, u + 1, v + 1)
            adj.setdefault(u, []).append((v, w))
            if not directed:
                adj.setdefault(v, []).append((u, w))
        else:
            if len(p) < 2:
                continue
            u, v = int(p[0]), int(p[1])
            n = max(n, u + 1, v + 1)
            adj.setdefault(u, []).append(v)
            if not directed:
                adj.setdefault(v, []).append(u)
    return adj, n


def parse_common(graph_path):
    text = open(graph_path, encoding="utf-8").read()
    directed = bool(re.search(r"\bdirected\s*:\s*true\b", text, re.I))
    weighted = bool(re.search(r"\bTRUE\b", text))
    m = re.search(r"\bint\s+src\s*=\s*(\d+)", text)
    src = int(m.group(1)) if m else 0
    return directed, weighted, src


def sweep_min(adj, n):
    old = list(range(n))
    new = list(old)
    for u in range(n):
        for v in adj.get(u, ()):
            if old[u] + 1 < new[v]:
                new[v] = old[u] + 1
    return new


def sweep_add(adj, n):
    old = [1] * n
    new = list(old)
    for u in range(n):
        for v in adj.get(u, ()):
            new[v] += old[u]
    return new


def frontier_sssp(adj, n, src):
    dist = [INF] * n
    dist[src] = 0
    frontier = {src}
    rounds = 0
    while frontier and rounds < n:
        rounds += 1
        snap = list(dist)
        nxt = set()
        for u in frontier:
            if snap[u] == INF:
                continue
            for v, w in adj.get(u, ()):
                nd = snap[u] + w
                if nd < dist[v]:
                    dist[v] = nd
                    nxt.add(v)
        frontier = nxt
    return dist


def main():
    if len(sys.argv) != 4:
        raise SystemExit("usage: compute_roundsep_expected.py MODE GRAPH EDGES")
    mode, graph_path, edges_path = sys.argv[1], sys.argv[2], sys.argv[3]
    directed, weighted, src = parse_common(graph_path)
    adj, n = read_edges(edges_path, directed, weighted)
    if mode == "min":
        dist = sweep_min(adj, n)
    elif mode == "add":
        dist = sweep_add(adj, n)
    elif mode == "sssp":
        dist = frontier_sssp(adj, n, src)
    else:
        raise SystemExit(f"unknown mode {mode}")
    for d in dist:
        print(d)


if __name__ == "__main__":
    main()