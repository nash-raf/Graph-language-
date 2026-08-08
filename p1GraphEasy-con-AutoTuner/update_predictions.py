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
import os
import sys
from pathlib import Path

import cost_model as cm

SCRIPT_DIR = Path(__file__).resolve().parent
N_INSERTS = 50  # must match the benchmark kernel (same as cost_model.K_INS)

LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]


def unit_cost(op, layout, n, m):
    op = "Traverse" if op and op.lower().startswith("traverse") else "Insert"
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


def process(csv_path):
    with open(csv_path, newline="") as f:
        rows = list(csv.DictReader(f))
    fieldnames = list(rows[0].keys()) if rows else []

    op_col = "operation" if "operation" in fieldnames else "op"
    n_col = "n_vertices" if "n_vertices" in fieldnames else "n"
    m_col = ("m_undirected" if "m_undirected" in fieldnames
             else "m_directed" if "m_directed" in fieldnames else "m")
    meas_col = ("measured_kernel_ns" if "measured_kernel_ns" in fieldnames
                else "measured_ns")
    m_is_directed = m_col == "m_directed"

    groups = {}
    order = []
    for r in rows:
        key = (r["graph"], r[op_col])
        if key not in groups:
            groups[key] = []
            order.append(key)
        groups[key].append(r)

    for key in order:
        grp = groups[key]
        g, op = key
        n = int(grp[0][n_col])
        m = int(grp[0][m_col])
        if m_is_directed:
            m //= 2  # model uses undirected count
        mult = N_INSERTS if op == "Insert" else 1

        # Tell the model which graph is being predicted so exact shift
        # fractions are loaded from its edge file (never the bf=1.0 fallback).
        cm._CUR_GRAPH = g
        cm._SHIFT_FRACS = {}

        preds = {}
        for r in grp:
            lay = r["layout"]
            pred = mult * unit_cost(op, lay, n, m)
            preds[lay] = pred
            r["predicted_ns"] = f"{pred:.1f}"

        best = min(LAYOUTS, key=lambda l: preds[l])
        for r in grp:
            r["predicted_best"] = best

        # Verdict = FULL-ranking match: predicted order across ALL layouts vs
        # measured order (never just best-layout equality).
        meas_vals = {r["layout"]: float(r[meas_col])
                     for r in grp if str(r.get(meas_col, "")).strip()}
        verdict = ""
        if len(meas_vals) == len(LAYOUTS):
            pred_order = tuple(sorted(LAYOUTS, key=lambda l: preds[l]))
            meas_order = tuple(sorted(LAYOUTS, key=lambda l: meas_vals[l]))
            verdict = "MATCH" if pred_order == meas_order else "MISMATCH"
        for r in grp:
            r["verdict"] = ""
        grp[0]["verdict"] = verdict

    # make sure new columns exist even if the CSV didn't have them before
    for newcol in ("predicted_best", "verdict"):
        if newcol not in fieldnames:
            fieldnames.append(newcol)

    tmp_path = csv_path.with_suffix(csv_path.suffix + ".tmp")
    with open(tmp_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames, extrasaction="ignore")
        w.writeheader()
        for r in rows:
            w.writerow(r)
        f.flush()
        os.fsync(f.fileno())
    tmp_path.replace(csv_path)
    print(f"  updated {csv_path.name}  ({len(rows)} rows)  "
          f"(measured col: {meas_col})")


if __name__ == "__main__":
    print(f"calib: L={cm.L} t={cm.t} T={cm.T} Tm={cm.Tm} P={cm.P_dram} "
          f"ramp=[{cm.ramp_lo},{cm.ramp_hi}] h_cache={cm.h_cache} h_dram={cm.h_dram}")
    # CLI: update_predictions.py [csv ...]
    # Default: synthetic CSVs + real-graph MEH benchmark.
    if len(sys.argv) > 1:
        targets = [Path(a) if a.startswith("/") else SCRIPT_DIR / a
                   for a in sys.argv[1:]]
    else:
        targets = [
            SCRIPT_DIR / "erdos_renyi.csv",
            SCRIPT_DIR / "erdos_renyi_runs10.csv",
            SCRIPT_DIR / "barabasi_albert.csv",
            SCRIPT_DIR / "barabasi_albert_runs10.csv",
            SCRIPT_DIR / "meh_validate_updated.csv",
        ]
    for p in targets:
        if not p.exists():
            print(f"  SKIP (not found): {p}")
            continue
        process(p)
    sys.exit(0)
