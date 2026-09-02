#!/usr/bin/env python3

from pathlib import Path
import random


ROOT = Path(__file__).resolve().parent
EDGE_FILE = ROOT / "random_milo_edges.txt"

N = 80
RANDOM_EDGES = 150
SEED = 20260706


def add_edge(edges, u, v):
    if u != v:
        edges.add((u, v))


def main():
    rng = random.Random(SEED)
    edges = set()

    # Plant separated examples of the motifs so the extracted graphs are visible
    # even if the random background does not contain many exact induced matches.
    planted = [
        # feed-forward loops
        [(0, 1), (0, 2), (1, 2)],
        [(3, 4), (3, 5), (4, 5)],
        [(6, 7), (6, 8), (7, 8)],
        # three-chains
        [(10, 11), (11, 12)],
        [(13, 14), (14, 15)],
        [(16, 17), (17, 18)],
        # bi-fans
        [(20, 22), (20, 23), (21, 22), (21, 23)],
        [(24, 26), (24, 27), (25, 26), (25, 27)],
        # bi-parallels
        [(30, 31), (30, 32), (31, 33), (32, 33)],
        [(34, 35), (34, 36), (35, 37), (36, 37)],
    ]

    protected = set()
    for motif in planted:
        vertices = set()
        for u, v in motif:
            add_edge(edges, u, v)
            vertices.add(u)
            vertices.add(v)
        protected.update(vertices)

    while len(edges) < RANDOM_EDGES + sum(len(m) for m in planted):
        u = rng.randrange(N)
        v = rng.randrange(N)
        if u == v:
            continue

        # Keep planted motif regions readable by avoiding random extra edges
        # inside protected vertices. Cross-region/background edges still make the
        # overall graph look random.
        if u in protected and v in protected:
            continue
        add_edge(edges, u, v)

    EDGE_FILE.write_text(
        "\n".join(f"{u} {v}" for u, v in sorted(edges)) + "\n"
    )
    print(f"wrote {EDGE_FILE}")
    print(f"nodes={N} edges={len(edges)} seed={SEED}")


if __name__ == "__main__":
    main()
