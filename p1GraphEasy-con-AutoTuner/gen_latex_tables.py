#!/usr/bin/env python3
"""Generate LaTeX tables from ablation results: measured + predicted."""

import csv
import json

RESULTS = "/home/kazisahib/sgpl_main/Graph-language-/p1GraphEasy-con-AutoTuner/ablation_results.csv"

with open(RESULTS) as f:
    rows = list(csv.DictReader(f))

by_wl = {}
for row in rows:
    tag = row['tag']
    if tag not in by_wl:
        by_wl[tag] = {}
    by_wl[tag][row['layout']] = row

Ns = [500, 5000, 50000]
Ds = [2, 16, 64]
Hs = [10, 100]


def fmt(v, prec=3):
    if v is None:
        return "---"
    return f"{v:.{prec}f}"


# ═══════════════════════════════════════════════════════════════
#  Table 1: Predicted vs measured per-op insert cost
# ═══════════════════════════════════════════════════════════════
print(r"\begin{table}[t]")
print(r"\centering")
print(r"\caption{Per-Operation Insert Cost (\textmu{}s): Predicted vs Measured}")
print(r"\label{tab:perop-insert}")
print(r"\begin{tabular}{crrrrr}")
print(r"\toprule")
print(r"$N$ & $D$ & $H$ & \multicolumn{2}{c}{CSR insert (\textmu s)} & \multicolumn{2}{c}{PCSR insert (\textmu s)} \\")
print(r"\cmidrule(lr){4-5}\cmidrule(lr){6-7}")
print(r" & & & Pred & Meas & Pred & Meas \\")
print(r"\midrule")
for N in Ns:
    for D in Ds:
        for H in Hs:
            tag = f"N{N}_D{D}_H{H}"
            if tag not in by_wl:
                continue
            w = by_wl[tag]
            # CSR per-op insert: predicted and measured
            csr_pred_ns = None
            csr_meas_ns = None
            csr_tops = None
            if "CSR" in w:
                dp_regs = json.loads(w["CSR"].get("dp_regions", "[]"))
                pr_regs = json.loads(w["CSR"].get("profile_regions", "[]"))
                for r in dp_regs:
                    if r["kind"] == "Insert":
                        csr_tops = r["totalOps"]
                        csr_pred_ns = r["predicted_ns"] / r["totalOps"]
                for r in pr_regs:
                    if r["kind"] == "Insert" and csr_tops:
                        csr_meas_ns = r["measured_ns"] / csr_tops
            # PCSR per-op insert
            pcsr_pred_ns = None
            pcsr_meas_ns = None
            pcsr_tops = None
            if "PCSR" in w:
                dp_regs = json.loads(w["PCSR"].get("dp_regions", "[]"))
                pr_regs = json.loads(w["PCSR"].get("profile_regions", "[]"))
                for r in dp_regs:
                    if r["kind"] == "Insert":
                        pcsr_tops = r["totalOps"]
                        pcsr_pred_ns = r["predicted_ns"] / r["totalOps"]
                for r in pr_regs:
                    if r["kind"] == "Insert" and pcsr_tops:
                        pcsr_meas_ns = r["measured_ns"] / pcsr_tops
            # Convert ns to us
            csr_pred = csr_pred_ns / 1e3 if csr_pred_ns else None
            csr_meas = csr_meas_ns / 1e3 if csr_meas_ns else None
            pcsr_pred = pcsr_pred_ns / 1e3 if pcsr_pred_ns else None
            pcsr_meas = pcsr_meas_ns / 1e3 if pcsr_meas_ns else None
            print(f"{N:6d} & {D:3d} & {H:4d} & {fmt(csr_pred,1):>6s} & {fmt(csr_meas,1):>6s} & {fmt(pcsr_pred,3):>6s} & {fmt(pcsr_meas,3):>6s} \\\\")
print(r"\bottomrule")
print(r"\end{tabular}")
print(r"\end{table}")
print()

# ═══════════════════════════════════════════════════════════════
#  Table 2: Conversion cost (predicted vs measured)
# ═══════════════════════════════════════════════════════════════
print(r"\begin{table}[t]")
print(r"\centering")
print(r"\caption{Conversion Cost (ms): Predicted vs Measured}")
print(r"\label{tab:conv-cost}")
print(r"\begin{tabular}{crrrrcr}")
print(r"\toprule")
print(r"$N$ & $D$ & $H$ & DP Layout & Pred Conv (ms) & Meas Conv (ms) & Ratio \\")
print(r"\midrule")
for N in Ns:
    for D in Ds:
        for H in Hs:
            tag = f"N{N}_D{D}_H{H}"
            if tag not in by_wl or "DP" not in by_wl[tag]:
                continue
            w = by_wl[tag]["DP"]
            dp_regions = json.loads(w.get("dp_regions", "[]"))
            layouts = [r['layout'] for r in dp_regions]
            layout_str = " $\\rightarrow$ ".join(layouts)
            pred_conv = float(w.get("predicted_conversion_ns", 0) or 0) / 1.0e6
            meas_conv = float(w.get("total_conversion_ns", 0) or 0) / 1.0e6
            if meas_conv == 0:
                print(f"{N:6d} & {D:3d} & {H:4d} & {layout_str:>10s} & {fmt(0,3):>13s} & {fmt(0,3):>13s} & {'' :>5s} \\\\")
            else:
                ratio = meas_conv / pred_conv if pred_conv > 0 else 0
                print(f"{N:6d} & {D:3d} & {H:4d} & {layout_str:>10s} & {fmt(max(pred_conv,1e-9),3):>13s} & {fmt(meas_conv,3):>13s} & {ratio:>5.0f}$\\times$ \\\\")
print(r"\bottomrule")
print(r"\end{tabular}")
print(r"\end{table}")
print()

# ═══════════════════════════════════════════════════════════════
#  Table 3: Kernel times for all three layouts
# ═══════════════════════════════════════════════════════════════
print(r"\begin{table}[t]")
print(r"\centering")
print(r"\caption{Measured Pure-Kernel Time (ms) for Each Layout Policy}")
print(r"\label{tab:kernel-times}")
print(r"\begin{tabular}{crrrrc}")
print(r"\toprule")
print(r"$N$ & $D$ & $H$ & CSR & PCSR & DP & Best \\")
print(r"\midrule")
for N in Ns:
    for D in Ds:
        for H in Hs:
            tag = f"N{N}_D{D}_H{H}"
            if tag not in by_wl:
                continue
            w = by_wl[tag]
            csr = float(w["CSR"].get("pure_kernel_total_ms", 0) or 0)
            pcsr = float(w["PCSR"].get("pure_kernel_total_ms", 0) or 0)
            dp = float(w["DP"].get("pure_kernel_total_ms", 0) or 0)
            best = min(csr, pcsr, dp)
            best_name = ["CSR", "PCSR", "DP"][[csr, pcsr, dp].index(best)]
            print(f"{N:6d} & {D:3d} & {H:4d} & {csr:8.2f} & {pcsr:8.2f} & {dp:8.2f} & {best_name:3s} \\\\")
print(r"\bottomrule")
print(r"\end{tabular}")
print(r"\end{table}")
print()

# ═══════════════════════════════════════════════════════════════
#  Table 4: Conversion cost vs savings (only for CSR->PCSR cases)
# ═══════════════════════════════════════════════════════════════
print(r"\begin{table}[t]")
print(r"\centering")
print(r"\caption{Conversion Cost vs Insert Savings (cases with conversion)}")
print(r"\label{tab:conv-vs-savings}")
print(r"\begin{tabular}{crrrrr}")
print(r"\toprule")
print(r"$N$ & $D$ & $H$ & Meas Conv (ms) & Pred sav/op (\textmu s) & Pred break-even $H$ \\")
print(r"\midrule")
for N in Ns:
    for D in Ds:
        for H in Hs:
            tag = f"N{N}_D{D}_H{H}"
            if tag not in by_wl or "DP" not in by_wl[tag]:
                continue
            dp = by_wl[tag]["DP"]
            dp_regions = json.loads(dp.get("dp_regions", "[]"))
            layouts = [r['layout'] for r in dp_regions]
            if layouts != ["CSR", "PCSR"]:
                continue
            meas_conv = float(dp.get("total_conversion_ns", 0) or 0) / 1.0e6
            # Get per-op insert cost for CSR and PCSR from predictions
            csr = by_wl[tag]["CSR"]
            pcsr = by_wl[tag]["PCSR"]
            csr_regs = json.loads(csr.get("dp_regions", "[]"))
            pcsr_regs = json.loads(pcsr.get("dp_regions", "[]"))
            csr_ins_pred = None
            pcsr_ins_pred = None
            for r in csr_regs:
                if r["kind"] == "Insert" and r["totalOps"] > 0:
                    csr_ins_pred = r["predicted_ns"] / r["totalOps"]
            for r in pcsr_regs:
                if r["kind"] == "Insert" and r["totalOps"] > 0:
                    pcsr_ins_pred = r["predicted_ns"] / r["totalOps"]
            if csr_ins_pred and pcsr_ins_pred:
                sav_per_op_ns = csr_ins_pred - pcsr_ins_pred
                conv_ns = float(dp.get("total_conversion_ns", 0) or 0)
                be_h = conv_ns / sav_per_op_ns if sav_per_op_ns > 0 else 0
                print(f"{N:6d} & {D:3d} & {H:4d} & {fmt(meas_conv,3):>13s} & {sav_per_op_ns/1e3:14.1f} & {be_h:>17.0f} \\\\")
print(r"\bottomrule")
print(r"\end{tabular}")
print(r"\end{table}")
