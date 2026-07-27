#!/usr/bin/env python3
"""
Generate multi-seed ablation workloads over 50k-node synth graphs.

Profiles:
  - two_traversals: traversal -> long mutation -> traversal
  - mutation_then_query: traversal -> long mutation -> query-heavy set ops
  - mutation_only: long mutation only (no traversal)
  - multi_switch_complex: repeated traverse -> mutate -> query epochs
"""

from __future__ import annotations

import argparse
import random
from pathlib import Path


GRAPH_VARIANTS = [
    ("50K_edges", "../synth_graphs/synth_v_50000_e_50000.txt"),
    ("200K_edges", "../synth_graphs/synth_v_50000_e_200000.txt"),
    ("500K_edges", "../synth_graphs/synth_v_50000_e_500000.txt"),
    ("1M_edges", "../synth_graphs/synth_v_50000_e_1000000.txt"),
]


MUTATION_STRESS = {
    "50K_edges": (2400, 1200),
    "200K_edges": (4400, 2200),
    "500K_edges": (7200, 3600),
    "1M_edges": (10400, 5200),
}

QUERY_STRESS = {
    "50K_edges": 400,
    "200K_edges": 700,
    "500K_edges": 1000,
    "1M_edges": 1500,
}

COMPLEX_EPOCHS = {
    "50K_edges": 2,
    "200K_edges": 2,
    "500K_edges": 3,
    "1M_edges": 3,
}


def sample_ids(rng: random.Random, lo: int, hi: int, n: int) -> list[int]:
    return rng.sample(range(lo, hi + 1), n)


def write_workload(path: Path, label: str, edge_file: str, seed: int, profile: str) -> None:
    rng = random.Random(seed)

    def edge_batch(k: int) -> list[tuple[int, int]]:
        out = []
        for _ in range(k):
            u = rng.randint(0, 49999)
            v = rng.randint(0, 49999)
            while v == u:
                v = rng.randint(0, 49999)
            out.append((u, v))
        return out

    add_k, del_k = MUTATION_STRESS[label]
    add_batch = edge_batch(add_k)
    del_batch = add_batch[:del_k]
    query_probe_n = QUERY_STRESS[label]
    query_probe = sample_ids(rng, 1, 130000, query_probe_n)
    set_literal = sample_ids(rng, 1, 130000, 64)

    lines: list[str] = []
    lines.append(f"// Auto-generated workload (seed={seed})")
    lines.append("graph g1 {")
    lines.append(f'    edges: file "{edge_file}";')
    lines.append("};")
    lines.append("")

    if profile == "two_traversals":
        lines.append('query bfs_a: "bfs" of g1;')
        for u, v in add_batch:
            lines.append(f"add {u}->{v} to g1;")
        for u, v in del_batch:
            lines.append(f"remove {u}->{v} from g1;")
        lines.append('query bfs_c: "bfs" of g1;')
    elif profile == "mutation_then_query":
        lines.append('query bfs_a: "bfs" of g1;')
        for u, v in add_batch:
            lines.append(f"add {u}->{v} to g1;")
        for u, v in del_batch:
            lines.append(f"remove {u}->{v} from g1;")
        lines.append("set s_nodes = g1.nodes;")
        lines.append("set s_aux = {" + ", ".join(str(x) for x in set_literal) + "};")
        lines.append("set s_u = s_nodes union s_aux;")
        lines.append("set s_i = s_u intersect s_aux;")
        for q in query_probe:
            lines.append(f"if (s_i.contains({q})) {{ }} else {{ }}")
    elif profile == "mutation_only":
        for u, v in add_batch:
            lines.append(f"add {u}->{v} to g1;")
        for u, v in del_batch:
            lines.append(f"remove {u}->{v} from g1;")
    elif profile == "multi_switch_complex":
        epochs = COMPLEX_EPOCHS[label]
        lines.append('query bfs_boot: "bfs" of g1;')
        for e in range(epochs):
            add_e = edge_batch(max(1, add_k // epochs))
            del_e = add_e[: max(1, del_k // epochs)]
            probe_e = sample_ids(rng, 1, 130000, max(64, query_probe_n // epochs))
            lit_e = sample_ids(rng, 1, 130000, 64)

            # Mutation-heavy phase
            for u, v in add_e:
                lines.append(f"add {u}->{v} to g1;")
            for u, v in del_e:
                lines.append(f"remove {u}->{v} from g1;")

            # Query-heavy set phase
            lines.append(f"set s_nodes_{e} = g1.nodes;")
            lines.append(
                f"set s_aux_{e} = " + "{" + ", ".join(str(x) for x in lit_e) + "};"
            )
            lines.append(f"set s_u_{e} = s_nodes_{e} union s_aux_{e};")
            lines.append(f"set s_i_{e} = s_u_{e} intersect s_aux_{e};")
            for q in probe_e:
                lines.append(f"if (s_i_{e}.contains({q})) {{ }} else {{ }}")

            # Traversal checkpoint forces CSR-compatible phase
            if e < epochs - 1:
                lines.append(f'query bfs_epoch_{e}: "bfs" of g1;')

        lines.append('query bfs_final: "bfs" of g1;')
    else:
        raise ValueError(f"Unsupported profile: {profile}")

    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir", required=True, help="Output directory")
    parser.add_argument(
        "--seeds",
        required=True,
        help="Comma-separated integer seeds, e.g. 11,22,33",
    )
    parser.add_argument(
        "--profile",
        default="two_traversals",
        choices=[
            "two_traversals",
            "mutation_then_query",
            "mutation_only",
            "multi_switch_complex",
        ],
        help="Workload shape profile",
    )
    args = parser.parse_args()

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    seeds = [int(x.strip()) for x in args.seeds.split(",") if x.strip()]

    for label, edge_file in GRAPH_VARIANTS:
        for seed in seeds:
            out_path = out_dir / f"ablation_{label}_seed{seed}.graph"
            write_workload(out_path, label, edge_file, seed, args.profile)
            print(str(out_path))


if __name__ == "__main__":
    main()

