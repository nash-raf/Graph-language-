#!/usr/bin/env python3
"""Quick legacy vs aware comparison on the 3 ranking datasets.

Pure-Python predictions via cost_model (parity with the compiled pass is
verified: worst rel ~1e-9), so no compiles/runs are needed.  Measured costs
come from the CSVs.  Compares full representation rankings per (graph, op):
old = AUTOTUNER_CACHE_MODEL=legacy, new = AUTOTUNER_CACHE_MODEL=aware."""
import csv
import sys
import os
from collections import Counter

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cost_model as cm
import bench_folder as bf

DATASETS = {
    "real_world": ("real_world_runs10_merged.csv", ["real graphs", "new real"]),
    "barabasi": ("barabasi_albert_runs10.csv", ["barabasi_albert"]),
    "erdos": ("erdos_renyi_runs10.csv", ["erdos_renyi"]),
}
DATASET = "/home/kazisahib/sgpl_main/dataset"
LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]


def edge_file_for(g, ds):
    for folder in DATASETS[ds][1]:
        p = os.path.join(DATASET, folder, f"{g}.txt")
        if os.path.exists(p):
            return p
    return None


def rank_of(costs):
    return {l: i + 1 for i, (_, l) in enumerate(sorted((c, l) for l, c in costs.items()))}


def main():
    cm.set_cache_model("aware")
    summary = {}
    for ds, (csv_name, _) in DATASETS.items():
        rows = list(csv.DictReader(open(csv_name)))
        graphs = sorted({r["graph"] for r in rows})
        ops = sorted({r["operation"] for r in rows})
        meas = {(r["graph"], r["operation"], r["layout"]):
                float(r["measured_kernel_ns"])
                for r in rows if r.get("measured_kernel_ns")}
        results = []
        fixed = introduced = 0
        full_old = full_new = 0
        top1_old = top1_new = 0
        pairs = 0
        rho_l, tau_l = [], []
        for g in graphs:
            ef = edge_file_for(g, ds)
            if ef is None:
                continue
            n, m = bf.count_graph_file(ef)
            cm._CUR_GRAPH = g
            for op in ops:
                if not all((g, op, l) in meas for l in LAYOUTS):
                    continue
                costs = {"legacy": {}, "aware": {}}
                for layout in LAYOUTS:
                    cm.set_cache_model("legacy")
                    per_l = cm.predicted_ns(layout, op, n, m)
                    cm.set_cache_model("aware")
                    per_a = cm.predicted_ns(layout, op, n, m)
                    mult = 2.0 if op == "Traverse" else 50.0
                    costs["legacy"][layout] = per_l * mult
                    costs["aware"][layout] = per_a * mult
                mcosts = {l: meas[(g, op, l)] for l in LAYOUTS}
                mr = rank_of(mcosts)
                lr = rank_of(costs["legacy"])
                ar = rank_of(costs["aware"])
                pairs += 1
                if min(costs["legacy"], key=costs["legacy"].get) == min(mcosts, key=mcosts.get):
                    top1_old += 1
                if min(costs["aware"], key=costs["aware"].get) == min(mcosts, key=mcosts.get):
                    top1_new += 1
                fm_old = lr != mr
                fm_new = ar != mr
                full_old += fm_old
                full_new += fm_new
                if fm_old and not fm_new:
                    fixed += 1
                if not fm_old and fm_new:
                    introduced += 1
                mseq = [mcosts[l] for l in LAYOUTS]
                lseq = [costs["legacy"][l] for l in LAYOUTS]
                aseq = [costs["aware"][l] for l in LAYOUTS]
                rho_l.append(np.corrcoef(np.argsort(np.argsort(mseq)).astype(float),
                                         np.argsort(np.argsort(lseq)).astype(float))[0, 1])
                tau_l.append(np.corrcoef(np.argsort(np.argsort(mseq)).astype(float),
                                         np.argsort(np.argsort(aseq)).astype(float))[0, 1])
                if fm_old or fm_new:
                    results.append((g, op, [l for l in sorted(mr, key=mr.get)],
                                    [l for l in sorted(lr, key=lr.get)],
                                    [l for l in sorted(ar, key=ar.get)]))
        summary[ds] = (pairs, top1_old, top1_new, full_old, full_new,
                       fixed, introduced, np.mean(rho_l), np.mean(tau_l))
        print(f"=== {ds} ===")
        for g, op, mm, ll, aa in results:
            tag = "FIXED" if (ll != mm and aa == mm) else \
                  ("REGRESSED" if (ll == mm and aa != mm) else "")
            print(f"  {g:26s} {op:8s} meas={mm} legacy={ll} aware={aa} {tag}")
    print("\n=== SUMMARY ===")
    for ds, (p, t1o, t1n, fo, fn, fx, iu, rho, tau) in summary.items():
        print(f"{ds:12s} pairs={p} top1 {t1o}->{t1n} | full-rank mismatch "
              f"{fo}->{fn} fixed={fx} introduced={iu} | rho={rho:.3f} tau={tau:.3f}")
    print("\nNote: parity of Python vs compiled pass verified at worst rel ~1e-9 "
          "(both modes); these predictions equal the pass's.")


if __name__ == "__main__":
    main()