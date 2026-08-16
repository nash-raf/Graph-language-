"""access_class_cost.py — Step 6: final class × residency model.

    T = sum_s N_s [ h_{2,s} * r_{s,L2}
                  + (h_{3,s} - h_{2,s}) * r_{s,L3}
                  + (1 - h_{3,s}) * r_{s,DRAM} ]

Per-class per-residency rates r_{s,tier} are MEASURED (class_calib.c ->
class_calib.json): seq-read stream, overlapping R-M-W shift, random chase.
Per-class tier fractions h_{2,s}, h_{3,s} come from RD per line (steps 2-3).
"""
import csv
import json
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rd_hist as rh
import cost_model as cm

MEASURED = "real_world_runs10_merged.csv"
OUT = "/tmp/opencode/class_cost_final.json"

L2_LINES = rh.L2_LINES
L3_LINES = rh.L3_LINES


def class_rates():
    """r_{s,tier} (ns per cache line) — delegated to cost_model's measured
    per-class rates (hw_calib.json class_rates -> class_calib.json ->
    measured defaults)."""
    return cm.class_rates_model()


def line_mask(lines, o, r0, r1):
    """Lines (int32-space indices) whose cache line intersects [r0, r1)."""
    return (r0 < 16 * lines + 16 - o) & (r1 > 16 * lines - o)


def per_graph_cost(trace, o, o_br):
    """Per-phase per-tier line counts for one graph's trace.

    Class → line mapping is representation-specific (reads the op records);
    the RD → tier thresholding and the final (N, h2, h3) reduction are the
    generic rd_hist API (single source of truth)."""
    counts = {p: [0, 0, 0] for p in ("scan", "move", "brow", "struct")}
    for t, ell, rd, inf_mask, mu, rd2, inf2, s_rd in rh.iter_op_line_rd(
            trace, o, o_br):
        op = trace["ops"][t]
        start, end = op["start"], op["end"]
        ip, T = op["ip"], op["T"]
        blk = op["blk"]
        nb = trace["nb"]

        t_bcol = rh.rd_to_tier(rd, inf_mask)
        t_brow = rh.rd_to_tier(rd2, inf2)

        scan = line_mask(ell, o, start, end)
        move = line_mask(ell, o, ip, T + 2)

        for p, mask in (("scan", scan), ("move", move)):
            for ti in range(3):
                counts[p][ti] += int(np.count_nonzero(
                    mask & (t_bcol == ti)))
        brow = line_mask(mu, o_br, blk + 1, nb + 1)
        for ti in range(3):
            counts["brow"][ti] += int(np.count_nonzero(
                brow & (t_brow == ti)))
        if s_rd is not None:
            s_tier = rh.rd_to_tier(np.array([s_rd]), np.array([False]))[0]
            counts["struct"][s_tier] += 1
        else:
            counts["struct"][2] += 1
    return counts


def cost_of(counts, rates):
    total = 0.0
    detail = {}
    for p, c in counts.items():
        n_s = sum(c)
        h2, h3 = rh.hit_ratios_from_tiers(c[0], c[1], c[2])
        r = rates[p]
        ns = n_s * (h2 * r[0] + (h3 - h2) * r[1] + (1.0 - h3) * r[2])
        detail[p] = {"lines": n_s, "h2": h2, "h3": h3,
                     "tier_lines": c, "ns": ns}
        total += ns
    return total, detail


def main():
    traces = json.load(open("/tmp/opencode/traces.json"))
    rates = class_rates()
    meas = {}
    with open(MEASURED) as f:
        for r in csv.DictReader(f):
            if r.get("operation") == "Insert" and r.get("layout") == "BCSR":
                meas[r["graph"]] = float(r["measured_kernel_ns"])

    rows = []
    for g, tr in sorted(traces.items()):
        o = tr["bcol_base_mod64"][0] // 16
        o_br = tr["brow_base_mod64"][0] // 16
        counts = per_graph_cost(tr, o, o_br)
        t_pred, detail = cost_of(counts, rates)
        n_ops = tr.get("n_ops", 100) or 100
        m_ns = meas.get(g)
        rows.append({
            "graph": g, "n_ops": n_ops,
            "T_pred_ns": t_pred, "T_pred_per_op_ns": t_pred / n_ops,
            "measured_ns": m_ns,
            "measured_per_op_ns": m_ns / n_ops if m_ns else None,
            "ratio_pred_measured": t_pred / m_ns if m_ns else None,
            "detail": detail,
        })

    print(f"{'graph':24s} {'Tpred':>11s} {'Tmeas':>11s} {'ratio':>6s} | "
          f"{'scan':>7s} {'move':>7s} {'brow':>6s} {'struct':>6s}")
    for r in sorted(rows, key=lambda r: r["graph"]):
        d = r["detail"]
        print(f"{r['graph']:24s} {r['T_pred_ns']:11.0f} "
              f"{r['measured_ns']:11.0f} {r['ratio_pred_measured']:6.2f} | "
              f"{d['scan']['ns']:7.0f} {d['move']['ns']:7.0f} "
              f"{d['brow']['ns']:6.0f} {d['struct']['ns']:6.0f}")

    pred_pop = [r["T_pred_per_op_ns"] for r in rows]
    meas_pop = [r["measured_per_op_ns"] for r in rows]
    rho = np.corrcoef(np.argsort(np.argsort(pred_pop)).astype(float),
                      np.argsort(np.argsort(meas_pop)).astype(float))[0, 1]
    ratios = [r["ratio_pred_measured"] for r in rows
              if r["ratio_pred_measured"]]
    med = np.median([abs(x - 1) for x in ratios])
    print(f"\nSpearman rho: {rho:.3f}   (baseline 0.967, step5 0.967)")
    print(f"median |ratio-1|: {med:.3f}   "
          f"(baseline 2.11, step5 0.185)")

    for r in rows:
        d = r["detail"]
        r["tier_ns"] = {p: d[p]["ns"] for p in d}
        r["tier_lines"] = {p: d[p]["tier_lines"] for p in d}
        r["tier_h"] = {p: (d[p]["h2"], d[p]["h3"]) for p in d}
    json.dump(rows, open(OUT, "w"), indent=1)
    print(f"wrote {OUT}")


if __name__ == "__main__":
    main()
