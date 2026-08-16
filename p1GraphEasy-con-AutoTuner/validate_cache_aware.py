#!/usr/bin/env python3
"""validate_cache_aware.py — legacy vs legacy+cache-aware penalty vs measured.

Per validation graph (real_world), per layout (CSR, BCSR), the predicted
insert kernel cost (K_INS=50 adds) under each AUTOTUNER_CACHE_MODEL mode,
compared against measured_kernel_ns from real_world_runs10_merged.csv.
The only change between 'legacy' and 'aware' is memPenalty -> F(W,h2,h3)
on the existing memory terms; 'class_tier' is shown for diagnostics.
"""
import csv
import sys
import os

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cost_model as cm

MEASURED = "real_world_runs10_merged.csv"
K_INS = 50.0


def measured_map():
    out = {}
    with open(MEASURED) as f:
        for r in csv.DictReader(f):
            if (r.get("operation") == "Insert"
                    and r.get("measured_kernel_ns")):
                out[(r["graph"], r["layout"])] = float(r["measured_kernel_ns"])
    return out


def main():
    meas = measured_map()
    graphs = sorted({g for (g, _) in meas})
    print(f"{'graph':26s} {'lay':4s} {'legacy':>10s} {'aware':>10s} "
          f"{'cls_tier':>10s} {'meas':>12s} {'r_leg':>6s} {'r_aware':>7s}")
    r_leg, r_aware = {l: [] for l in ("CSR", "BCSR")}, {l: [] for l in ("CSR", "BCSR")}
    for g in graphs:
        ef = cm._resolve_edge_file(g)
        if ef is None:
            continue
        n, m = cm.count_graph_file(ef) if hasattr(cm, "count_graph_file") else (None, None)
        # n, m via bench_folder
        import bench_folder as bf
        n, m = bf.count_graph_file(ef)
        cm._CUR_GRAPH = g
        for layout in ("CSR", "BCSR"):
            m_ns = meas.get((g, layout))
            if not m_ns:
                continue
            cm.set_cache_model("legacy")
            p_leg = (K_INS * cm.insert_cost_csr(n, m) if layout == "CSR"
                     else K_INS * cm.insert_cost_bcsr(n, m))
            cm.set_cache_model("aware")
            p_awa = (K_INS * cm.insert_cost_csr(n, m) if layout == "CSR"
                     else K_INS * cm.insert_cost_bcsr(n, m))
            cm.set_cache_model("class_tier")
            p_cls = (K_INS * cm.insert_cost_csr(n, m) if layout == "CSR"
                     else K_INS * cm.insert_cost_bcsr(n, m))
            r_leg[layout].append(p_leg / m_ns)
            r_aware[layout].append(p_awa / m_ns)
            print(f"{g:26s} {layout:4s} {p_leg:10.0f} {p_awa:10.0f} "
                  f"{p_cls:10.0f} {m_ns:12.0f} {p_leg/m_ns:6.3f} "
                  f"{p_awa/m_ns:7.3f}")
    cm.set_cache_model("aware")
    print()
    for layout in ("CSR", "BCSR"):
        print(f"{layout}: median |ratio-1| legacy={np.median([abs(x-1) for x in r_leg[layout]]):.3f}  "
              f"aware={np.median([abs(x-1) for x in r_aware[layout]]):.3f}")


if __name__ == "__main__":
    main()