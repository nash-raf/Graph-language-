#!/usr/bin/env python3
"""
update_predictions.py — Recompute ONLY the predicted columns (predicted_ns,
predicted_best, verdict) of erdos_renyi.csv / barabasi_albert.csv using the
CURRENT cost model (cost_model.py mirrors AutoTunerPass.cpp + the existing
hw_calib.json calibration; no hw_calib_bench rerun).

Measured columns (measured_kernel_ns, measured_best) are left untouched.
Real-graph / meh CSVs are never touched.
"""
import csv
import sys
from pathlib import Path

import cost_model as cm

SCRIPT_DIR = Path(__file__).resolve().parent
N_INSERTS = 50  # must match the benchmark kernel (same as cost_model.K_INS)

LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]


def unit_cost(op, layout, n, m):
    if op == "Traverse":
        if layout == "CSR":
            return cm.traverse_cost_csr(n, m)
        if layout == "PCSR":
            return cm.traverse_cost_pcsr(n, m)
        if layout == "BCSR":
            return cm.traverse_cost_bcsr(n, m)
        return cm.traverse_cost_set(n, m)
    if layout == "CSR":
        return cm.insert_cost_csr(n, m)
    if layout == "PCSR":
        return cm.insert_cost_pcsr(n, m)
    if layout == "BCSR":
        return cm.insert_cost_bcsr(n, m)
    return cm.insert_cost_set(n, m)


def process(csv_path, m_is_directed):
    with open(csv_path, newline="") as f:
        rows = list(csv.DictReader(f))
    fieldnames = list(rows[0].keys())

    groups = {}
    order = []
    for r in rows:
        key = (r["graph"], r["operation"])
        if key not in groups:
            groups[key] = []
            order.append(key)
        groups[key].append(r)

    for key in order:
        grp = groups[key]
        g, op = key
        n = int(grp[0]["n_vertices"])
        if m_is_directed:
            m = int(grp[0]["m_directed"]) // 2  # model uses undirected count
        else:
            m = int(grp[0]["m_undirected"])
        mult = N_INSERTS if op == "Insert" else 1

        preds = {}
        for r in grp:
            lay = r["layout"]
            pred = mult * unit_cost(op, lay, n, m)
            preds[lay] = pred
            r["predicted_ns"] = f"{pred:.1f}"

        best = min(LAYOUTS, key=lambda l: preds[l])
        for r in grp:
            r["predicted_best"] = best

        mbest = grp[0].get("measured_best", "").strip()
        verdict = ""
        if mbest:
            verdict = "MATCH" if best == mbest else "MISMATCH"
        for r in grp:
            r["verdict"] = ""
        grp[0]["verdict"] = verdict

    with open(csv_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        for r in rows:
            w.writerow(r)
    print(f"  updated {csv_path.name}  ({len(rows)} rows)")


if __name__ == "__main__":
    print(f"calib: L={cm.L} t={cm.t} T={cm.T} Tm={cm.Tm} P={cm.P_dram} "
          f"ramp=[{cm.ramp_lo},{cm.ramp_hi}] h_cache={cm.h_cache} h_dram={cm.h_dram}")
    # CLI: update_predictions.py [csv ...]
    # Default: synthetic CSVs only (never real-graph / meh).
    # m_undirected CSVs pass m_is_directed=False; m_directed → True.
    if len(sys.argv) > 1:
        targets = []
        for a in sys.argv[1:]:
            p = Path(a)
            if not p.is_absolute():
                p = SCRIPT_DIR / p
            # Heuristic: erdos_* historically store m_directed; BA / *_runs10 store m_undirected
            directed = "erdos" in p.name and "undirected" not in p.name
            # Prefer column-aware: process() reads the column names from the CSV header
            targets.append((p, directed))
    else:
        targets = [
            (SCRIPT_DIR / "erdos_renyi.csv", True),
            (SCRIPT_DIR / "erdos_renyi_runs10.csv", False),
            (SCRIPT_DIR / "barabasi_albert.csv", False),
            (SCRIPT_DIR / "barabasi_albert_runs10.csv", False),
        ]
    for p, directed in targets:
        if not p.exists():
            print(f"  SKIP (not found): {p}")
            continue
        # Detect undirected vs directed from header when present
        with open(p, newline="") as f:
            cols = next(csv.reader(f))
        if "m_undirected" in cols:
            directed = False
        elif "m_directed" in cols:
            directed = True
        process(p, directed)
    sys.exit(0)
