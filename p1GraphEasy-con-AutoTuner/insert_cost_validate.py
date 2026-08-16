"""insert_cost_validate.py — Step 4: predicted insertion cost vs measured.

    T_pred  = N * r,  N = cache-line accesses (total_refs from the RD
    histogram, step 2), r = ns per line access (step 3 calibration).

Tier decomposition (kept visible so a bad prediction is diagnosable):
    T_L2    = N * h_L2 * r_L2
    T_L3    = N * (h_L3 - h_L2) * r_L3
    T_DRAM  = N * (1 - h_L3) * r_DRAM
    T_pred  = T_L2 + T_L3 + T_DRAM

Workload alignment: the measured runs (real_world_runs10_merged.csv,
Insert/BCSR) insert 50 fresh undirected pairs (seed 42) -> 100 directed
adds == the 100 ops of each trace (k_ins=50).  So N and measured_kernel_ns
cover the same 100-op workload; per-op values divide both by n_ops=100.

Validation order (per methodology): (1) ordering across graphs (Spearman
rank of per-op predicted vs per-op measured), (2) absolute ratio.
"""
import csv
import json
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mrc_calibrate as mrc

MEASURED = "real_world_runs10_merged.csv"
OUT = "/tmp/opencode/insert_cost_validation.json"


def load_measured():
    rows = []
    with open(MEASURED) as f:
        for r in csv.DictReader(f):
            if r.get("operation") == "Insert" and r.get("layout") == "BCSR":
                rows.append({
                    "graph": r["graph"],
                    "measured_ns": float(r["measured_kernel_ns"]),
                    "iqr_ns": float(r["measured_iqr_ns"]),
                    "cv": float(r["kernel_cv"]),
                    "old_predicted_ns": float(r["predicted_ns"]),
                })
    return {r["graph"]: r for r in rows}


def main():
    data = json.load(open(mrc.HIST))
    for g, s in data.items():
        s["hist"] = {int(k): v for k, v in s["hist"].items()}
    meas = load_measured()

    out_rows = []
    for g, s in sorted(data.items()):
        if g not in meas:
            continue
        d = mrc.decompose(s["hist"], s["total_refs"], s["inf_refs"])
        N = s["total_refs"]
        n_ops = s.get("n_ops", 100) or 100
        t_l2 = N * d["h_l2"] * mrc.R_L2
        t_l3 = N * (d["h_l3"] - d["h_l2"]) * mrc.R_L3
        t_dram = N * (1 - d["h_l3"]) * mrc.R_DRAM
        t_pred = t_l2 + t_l3 + t_dram
        m = meas[g]
        pred_pop = t_pred / n_ops
        meas_pop = m["measured_ns"] / n_ops
        out_rows.append({
            "graph": g,
            "n_line_accesses": N,
            "n_ops": n_ops,
            "h_l2": d["h_l2"],
            "h_l3": d["h_l3"],
            "T_L2_ns": t_l2,
            "T_L3_ns": t_l3,
            "T_DRAM_ns": t_dram,
            "T_pred_ns": t_pred,
            "T_pred_per_op_ns": pred_pop,
            "measured_total_ns": m["measured_ns"],
            "measured_per_op_ns": meas_pop,
            "measured_cv": m["cv"],
            "old_predicted_ns": m["old_predicted_ns"],
            "ratio_pred_measured": t_pred / m["measured_ns"],
        })

    out_rows.sort(key=lambda r: r["graph"])
    print(f"{'graph':24s} {'N':>10s} {'Tpred':>11s} {'Tmeas':>11s} "
          f"{'perop_p':>8s} {'perop_m':>8s} {'ratio':>6s} | "
          f"{'L2':>5s} {'L3':>5s} {'DRAM':>6s}")
    for r in out_rows:
        t = r
        print(f"{t['graph']:24s} {t['n_line_accesses']:10d} "
              f"{t['T_pred_ns']:11.0f} {t['measured_total_ns']:11.0f} "
              f"{t['T_pred_per_op_ns']:8.0f} {t['measured_per_op_ns']:8.0f} "
              f"{t['ratio_pred_measured']:6.2f} | "
              f"{t['T_L2_ns']/t['T_pred_ns']*100:5.1f} "
              f"{t['T_L3_ns']/t['T_pred_ns']*100:5.1f} "
              f"{t['T_DRAM_ns']/t['T_pred_ns']*100:6.1f}")

    pred_order = [r["T_pred_per_op_ns"] for r in out_rows]
    meas_order = [r["measured_per_op_ns"] for r in out_rows]
    rho = np.corrcoef(
        np.argsort(np.argsort(pred_order)).astype(float),
        np.argsort(np.argsort(meas_order)).astype(float))[0, 1]
    pred_rank = {r["graph"]: i + 1 for i, r in
                 enumerate(sorted(out_rows, key=lambda r: -r["T_pred_per_op_ns"]))}
    meas_rank = {r["graph"]: i + 1 for i, r in
                 enumerate(sorted(out_rows, key=lambda r: -r["measured_per_op_ns"]))}
    print(f"\nSpearman rho (per-op predicted vs measured): {rho:.3f}")
    print(f"{'graph':24s} {'rank_pred':>10s} {'rank_meas':>10s}")
    for r in sorted(out_rows, key=lambda r: -r["T_pred_per_op_ns"]):
        print(f"{r['graph']:24s} {pred_rank[r['graph']]:10d} "
              f"{meas_rank[r['graph']]:10d}")

    med = np.median([abs(r["ratio_pred_measured"] - 1) for r in out_rows])
    print(f"\nmedian |ratio-1|: {med:.3f}")
    json.dump(out_rows, open(OUT, "w"), indent=1)
    print(f"\nwrote {OUT}")


if __name__ == "__main__":
    main()
