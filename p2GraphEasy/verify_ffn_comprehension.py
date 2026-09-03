#!/usr/bin/env python3

from pathlib import Path


ROOT = Path(__file__).resolve().parent
EDGE_FILE = ROOT / "ffn_edges.txt"
OUTPUT_FILE = ROOT / "ffn_comprehension.out"


def read_edges(path):
    edges = []
    vertices = set()
    for line in path.read_text().splitlines():
        if not line.strip():
            continue
        u, v = map(int, line.split()[:2])
        edges.append((u, v))
        vertices.add(u)
        vertices.add(v)
    return edges, vertices


def main():
    edges, vertices = read_edges(EDGE_FILE)
    edge_set = set(edges)

    motifs = []
    ffn_vertices = set()
    for u, v in edges:
        for _, w in [edge for edge in edges if edge[0] == v]:
            if (u, w) in edge_set:
                motifs.append((u, v, w))
                ffn_vertices.update((u, v, w))

    ffn_edges = [(u, v) for u, v in edges if u in ffn_vertices and v in ffn_vertices]
    expected_header = [
        len(motifs),
        len(ffn_vertices),
        len(ffn_vertices),
        len(ffn_edges),
    ]

    actual = [int(line) for line in OUTPUT_FILE.read_text().splitlines() if line.strip()]
    actual_header = actual[:4]
    actual_vertices = set(actual[4:])
    expected_compact_vertices = set(range(len(ffn_vertices)))

    if actual_header != expected_header:
        raise SystemExit(
            f"FFN FAIL: header {actual_header} != expected {expected_header}; motifs={motifs}"
        )
    if actual_vertices != expected_compact_vertices:
        raise SystemExit(
            "FFN FAIL: compact vertices "
            f"{sorted(actual_vertices)} != expected {sorted(expected_compact_vertices)}"
        )

    print("FFN: PASS")
    print(f"  motifs: {motifs}")
    print(f"  motif vertices: {sorted(ffn_vertices)}")
    print(f"  compact FFN vertices: {sorted(expected_compact_vertices)}")
    print(f"  induced FFN edges: {ffn_edges}")


if __name__ == "__main__":
    main()
