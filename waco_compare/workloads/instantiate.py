#!/usr/bin/env python3
"""Instantiate .graph workloads for each corpus graph."""
from __future__ import annotations

import argparse
import json
import random
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TMPL = Path(__file__).resolve().parent / "templates"
OUT = ROOT / "data" / "workloads"


def make_insert_block(n: int, m_existing: int, n_inserts: int, seed: int) -> str:
    rng = random.Random(seed)
    edges = []
    seen = set()
    # Prefer fresh endpoints beyond original n when possible; else random pairs.
    base = max(n, 1)
    attempts = 0
    while len(edges) < n_inserts and attempts < n_inserts * 50:
        attempts += 1
        u = rng.randrange(0, base + n_inserts)
        v = rng.randrange(0, base + n_inserts)
        if u == v:
            continue
        key = (min(u, v), max(u, v))
        if key in seen:
            continue
        seen.add(key)
        edges.append((u, v))
    if not edges:
        return "// no inserts\n"
    # Batch into addEdges calls of up to 32 edges
    lines = []
    for i in range(0, len(edges), 32):
        chunk = edges[i : i + 32]
        body = ", ".join(f"{u}->{v}" for u, v in chunk)
        lines.append(f"G.addEdges({body});")
    return "\n".join(lines) + "\n"


def instantiate(graphs, templates, max_rounds: int, n_inserts: int, out_dir: Path):
    out_dir.mkdir(parents=True, exist_ok=True)
    index = []
    for g in graphs:
        edge = g["edge_list"]
        for tmpl_name in templates:
            tmpl_path = TMPL / f"{tmpl_name}.graph.tmpl"
            text = tmpl_path.read_text()
            text = text.replace("__EDGE_FILE__", edge)
            text = text.replace("__MAX_ROUNDS__", str(max_rounds))
            if "__INSERT_BLOCK__" in text:
                text = text.replace(
                    "__INSERT_BLOCK__",
                    make_insert_block(g["n"], g["m"], n_inserts, hash(g["name"]) & 0xFFFFFFFF),
                )
            out_name = f"{g['name']}__{tmpl_name}.graph"
            out_path = out_dir / out_name
            out_path.write_text(text)
            index.append(
                {
                    "graph": g["name"],
                    "family": g["family"],
                    "n": g["n"],
                    "m": g["m"],
                    "template": tmpl_name,
                    "path": str(out_path),
                    "edge_list": edge,
                    "csr": g.get("csr"),
                }
            )
    (out_dir / "index.json").write_text(json.dumps(index, indent=2))
    print(f"instantiated {len(index)} workloads -> {out_dir}")
    return index


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--split", choices=["train", "test", "all"], default="all")
    ap.add_argument("--max-rounds", type=int, default=5)
    ap.add_argument("--n-inserts", type=int, default=32)
    ap.add_argument(
        "--templates",
        nargs="+",
        default=["traverse_cc", "traverse_pagerank", "insert_then_traverse"],
    )
    args = ap.parse_args()

    if args.split == "all":
        graphs = json.loads((ROOT / "data" / "corpus.json").read_text())["graphs"]
    else:
        graphs = json.loads((ROOT / "data" / "splits" / f"{args.split}.json").read_text())

    instantiate(graphs, args.templates, args.max_rounds, args.n_inserts, OUT / args.split)


if __name__ == "__main__":
    main()
