#!/usr/bin/env python3
# read_edgelist.py
import sys
import time

def main(path):
    edges = []
    nodes = set()
    t0 = time.perf_counter()

    with open(path, "r", encoding="utf-8") as f:
        for lineno, line in enumerate(f, start=1):
            line = line.strip()
            if not line:
                continue
            if line.startswith("#") or line.startswith("//"):
                continue
            parts = line.split()
            if len(parts) < 2:
                # skip malformed lines
                # print(f"Warning: malformed line {lineno}", file=sys.stderr)
                continue
            try:
                u = int(parts[0])
                v = int(parts[1])
            except ValueError:
                # skip lines that don't start with integers
                continue
            edges.append((u, v))
            nodes.add(u)
            nodes.add(v)

    t1 = time.perf_counter()
    ms = (t1 - t0) * 1000.0

    print(f"File: {path}")
    print(f"Edges read: {len(edges)}")
    print(f"Unique nodes: {len(nodes)}")
    print(f"Read time: {ms:.3f} ms")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: read_edgelist.py <edge-list-file>", file=sys.stderr)
        sys.exit(1)
    main(sys.argv[1])