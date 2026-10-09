#!/usr/bin/env python3
"""Update ONLY the Traverse predicted_ns columns of the 3 ranking CSVs with
the cache-tier traverse equations (CSR/PCSR: move class at seq rates, BCSR:
scan class at seq rates, SET: unchanged).  Insert rows untouched.  Recompute
predicted_best / verdict (tie-aware)."""
import csv
import shutil
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cost_model as cm

CSVS = ["real_world_runs10_merged.csv",
        "barabasi_albert_runs10.csv",
        "erdos_renyi_runs10.csv"]
LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]
cm.set_cache_model("hybrid")


def ffloat(s):
    s = str(s).strip()
    if not s or s.lower() == "none":
        return None
    return float(s)


def verdict(preds, meas_mean, meas_iqr):
    valid = [(l, preds[l], meas_mean[l]) for l in LAYOUTS
             if meas_mean.get(l) is not None]
    if not valid:
        return "NODATA", "-", "-"
    bp = min(valid, key=lambda x: x[1])[0]
    bm = min(valid, key=lambda x: x[2])[0]
    ps = sorted(p for _, p, _ in valid)
    pred_tie = (ps[1] - ps[0]) <= 0.05 * max(ps[0], 1.0)
    ms = sorted((m_, l_) for l_, _, m_ in valid)
    a_iqr = meas_iqr.get(ms[0][1], 0) or 0
    b_iqr = meas_iqr.get(ms[1][1], 0) or 0
    meas_tie = abs(ms[0][0] - ms[1][0]) < max(a_iqr, b_iqr)
    if pred_tie and meas_tie:
        return "TIE", bp, bm
    if bp == bm:
        return "MATCH", bp, bm
    if meas_tie:
        return "NOISE", bp, bm
    return "MISMATCH", bp, bm


def main():
    for csv_name in CSVS:
        shutil.copy2(csv_name, csv_name + ".travwrate.bak")
        rows = list(csv.DictReader(open(csv_name)))
        fieldnames = rows[0].keys()
        for r in rows:
            if r["operation"] != "Traverse":
                continue
            g = r["graph"]
            lay = r["layout"]
            n = int(float(r["n_vertices"]))
            m = int(float(r["m_undirected"]))
            cm._CUR_GRAPH = g
            per = cm.predicted_ns(lay, "Traverse", n, m)
            r["predicted_ns"] = f"{2.0 * per:.1f}"
        for (g, op) in {(r["graph"], r["operation"]) for r in rows}:
            mm = {l: ffloat(next((x["measured_kernel_ns"] for x in rows
                                  if x["graph"] == g and x["operation"] == op
                                  and x["layout"] == l), None)) for l in LAYOUTS}
            mi = {l: ffloat(next((x["measured_iqr_ns"] for x in rows
                                  if x["graph"] == g and x["operation"] == op
                                  and x["layout"] == l), 0)) for l in LAYOUTS}
            pr = {l: float(next(x["predicted_ns"] for x in rows
                                if x["graph"] == g and x["operation"] == op
                                and x["layout"] == l)) for l in LAYOUTS}
            v, bp, bm = verdict(pr, mm, mi)
            for x in rows:
                if x["graph"] == g and x["operation"] == op:
                    x["predicted_best"] = bp
                    x["measured_best"] = bm
                    x["verdict"] = v if x["layout"] == LAYOUTS[0] else ""
        with open(csv_name, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fieldnames)
            w.writeheader()
            w.writerows(rows)
        print(f"updated {csv_name} ({len(rows)} rows)")


if __name__ == "__main__":
    main()