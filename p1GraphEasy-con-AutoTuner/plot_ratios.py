#!/usr/bin/env python3
"""Ratio plots: predicted_ns / measured_kernel_ns per graph, one plot per
(layout, operation), from a ranking CSV (arg, default real-world merged)."""
import csv
import os
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

CSV = sys.argv[1] if len(sys.argv) > 1 else "real_world_runs10_merged.csv"
OUTDIR = sys.argv[2] if len(sys.argv) > 2 else "ratio_plots"
EXCLUDE = set(sys.argv[3].split(",")) if len(sys.argv) > 3 else set()
os.makedirs(OUTDIR, exist_ok=True)

rows = list(csv.DictReader(open(CSV)))
graphs = sorted({r["graph"] for r in rows})
layouts = ["CSR", "PCSR", "BCSR", "SET"]
ops = ["Insert", "Traverse"]

for lay in layouts:
    for op in ops:
        sub = [r for r in rows if r["layout"] == lay and r["operation"] == op
               and r["measured_kernel_ns"]
               and r["graph"] not in EXCLUDE]
        if not sub:
            continue
        gs = [r["graph"] for r in sub]
        ratios = [float(r["predicted_ns"]) / float(r["measured_kernel_ns"])
                  for r in sub]
        iqrs = [float(r["measured_iqr_ns"]) / float(r["measured_kernel_ns"])
                for r in sub]
        n = len(ratios)
        med = float(np.median(np.abs(np.array(ratios) - 1.0)))
        fig, ax = plt.subplots(figsize=(11, 5.5))
        x = np.arange(n)
        ax.bar(x, ratios, color="#4c72b0", alpha=0.85, width=0.62)
        ax.errorbar(x, ratios, yerr=iqrs, fmt="none", ecolor="#555555",
                    capsize=3, elinewidth=1)
        ax.axhline(1.0, color="#c44e52", lw=1.6, ls="--",
                   label="ratio = 1 (exact)")
        for i, (g, r) in enumerate(zip(gs, ratios)):
            ax.annotate(f"{r:.2f}", (i, r), textcoords="offset points",
                        xytext=(0, 4), ha="center", fontsize=8)
        ax.set_xticks(x)
        ax.set_xticklabels(gs, rotation=40, ha="right", fontsize=9)
        ax.set_yscale("log")
        ax.set_ylabel("predicted / measured  (log)")
        ax.set_title(f"{lay} — {op}: predicted/measured ratio per graph "
                     f"(median |ratio−1| = {med:.3f})")
        ax.grid(axis="y", alpha=0.3)
        ax.legend(loc="lower right")
        fig.tight_layout()
        out = os.path.join(OUTDIR, f"ratio_{lay}_{op}.png")
        fig.savefig(out, dpi=150)
        plt.close(fig)
        print(f"{out}: {n} graphs, ratios "
              f"{['%.2f' % r for r in ratios]}, median|r-1|={med:.3f}")
print("done")