#!/usr/bin/env python3
"""Generate ablation workloads: random edge lists + .graph files."""

import math
import os
import random
import csv

OUTDIR = "ablation_workloads"

# Parameter grid
N_VALS       = [500, 5000, 50000]
D_VALS       = [2, 16, 64]          # average degree = 2m/n
H_VALS       = [10, 100]            # loop count per phase

random.seed(42)

def write_edgelist(path, n, m_undirected):
    nodes = list(range(n))
    edges = set()
    attempts = 0
    while len(edges) < m_undirected and attempts < m_undirected * 10:
        u = random.choice(nodes)
        v = random.choice(nodes)
        if u >= v:
            continue
        key = (u, v)
        if key not in edges:
            edges.add(key)
        attempts += 1
    with open(path, "w") as f:
        for u, v in sorted(edges):
            f.write(f"{u} {v}\n")
    return len(edges)

def cleanup_workload_dir():
    import shutil
    if os.path.isdir(OUTDIR):
        shutil.rmtree(OUTDIR)
    os.makedirs(OUTDIR, exist_ok=True)

def generate_workloads():
    cleanup_workload_dir()
    rows = []

    for n in N_VALS:
        for d in D_VALS:
            m_dir = n * d         # directed edges = n * d
            m_undir = m_dir // 2  # each undirected edge = 2 directed

            for H in H_VALS:
                tag = f"N{n}_D{d}_H{H}"
                edgelist_name = f"el_{tag}.txt"
                edgelist_path = os.path.join(OUTDIR, edgelist_name)
                actual_m = write_edgelist(edgelist_path, n, m_undir)

                graph_name = f"wl_{tag}.graph"
                graph_path = os.path.join(OUTDIR, graph_name)

                with open(graph_path, "w") as f:
                    f.write(f"graph Ablation_{tag} {{\n")
                    f.write(f'  edges: file "edgelist.txt";\n')
                    f.write("};\n\n")
                    f.write("/* Phase 1: traverse calls */\n")
                    for k in range(H):
                        f.write(f'query bfs_{k}: "bfs" of Ablation_{tag};\n')
                    f.write("\n")
                    f.write("/* Phase 2: insert calls (triggers conversion) */\n")
                    for k in range(H):
                        src = k
                        dst = (k + 1) % n
                        f.write(f"add {src}->{dst} to Ablation_{tag};\n")

                rows.append({
                    "workload": tag,
                    "n": n,
                    "d": d,
                    "m_actual": actual_m * 2,
                    "H": H,
                    "edge_list": edgelist_name,
                    "graph_file": graph_name,
                })

    meta_path = os.path.join(OUTDIR, "workloads.csv")
    with open(meta_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=[
            "workload", "n", "d", "m_actual", "H",
            "edge_list", "graph_file"])
        w.writeheader()
        w.writerows(rows)
    print(f"Generated {len(rows)} workloads in {OUTDIR}/")
    print(f"Metadata: {meta_path}")

if __name__ == "__main__":
    generate_workloads()
