#!/usr/bin/env python3
"""Generate ablation visualization: CSR vs PCSR vs DP kernel times."""

import csv
import json
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

RESULTS = "/home/kazisahib/sgpl_main/Graph-language-/p1GraphEasy-con-AutoTuner/ablation_results.csv"
OUTPUT = "/home/kazisahib/sgpl_main/Graph-language-/p1GraphEasy-con-AutoTuner/ablation_plot.png"

with open(RESULTS) as f:
    rows = list(csv.DictReader(f))

by_wl = {}
for row in rows:
    tag = row['tag']
    if tag not in by_wl:
        by_wl[tag] = {}
    by_wl[tag][row['layout']] = row

# Sort: N ascending, D ascending, H ascending
def sort_key(tag):
    m = [int(x) for x in tag.replace("N", "").replace("D", "_").replace("H", "_").split("_") if x]
    return tuple(m)

tags_sorted = sorted(by_wl.keys(), key=sort_key)

# N values and D values in the dataset
Ns = [500, 5000, 50000]
Ds = [2, 16, 64]
Hs = [10, 100]

cmap = plt.get_cmap("Set2")
colors = {"CSR": "#66c2a5", "PCSR": "#fc8d62", "DP": "#8da0cb"}

fig, axes = plt.subplots(len(Ns), len(Ds), figsize=(14, 11), sharey=False)
fig.suptitle("Ablation Study: CSR-only vs PCSR-only vs DP-Optimal", fontsize=14, y=0.98)

for ni, N in enumerate(Ns):
    for di, D in enumerate(Ds):
        ax = axes[ni][di]
        xi = [0, 1]  # H=10 at x=0, H=100 at x=1
        width = 0.22
        offsets = [-width, 0, width]

        for li, (layout, color) in enumerate(colors.items()):
            vals = []
            for H in Hs:
                tag = f"N{N}_D{D}_H{H}"
                if tag in by_wl and layout in by_wl[tag]:
                    k = float(by_wl[tag][layout].get("pure_kernel_total_ms", 0) or 0)
                else:
                    k = 0
                vals.append(k)
            ax.bar([x + offsets[li] for x in xi], vals, width,
                   label=layout if ni == 0 and di == 0 else "",
                   color=color, edgecolor='gray', linewidth=0.5)

        # Show conversion cost segment on DP bar
        for hi, H in enumerate(Hs):
            tag = f"N{N}_D{D}_H{H}"
            if tag in by_wl and "DP" in by_wl[tag]:
                dp_k = float(by_wl[tag]["DP"].get("pure_kernel_total_ms", 0) or 0)
                conv_ns = float(by_wl[tag]["DP"].get("total_conversion_ns", 0) or 0)
                conv_ms = conv_ns / 1.0e6
                if conv_ms > 0:
                    ax.bar(xi[hi] + offsets[2], conv_ms, width,
                           bottom=dp_k - conv_ms,
                           color="red", alpha=0.4, hatch="///", edgecolor='darkred',
                           linewidth=0.5)

        ax.set_title(f"N={N}, D={D}", fontsize=11)
        ax.set_xticks(xi)
        ax.set_xticklabels(["H=10", "H=100"], fontsize=9)
        if di == 0:
            ax.set_ylabel("Kernel time (ms)", fontsize=9)
        ax.grid(axis='y', alpha=0.3)

        # Mark which layout is best per H
        for hi, H in enumerate(Hs):
            tag = f"N{N}_D{D}_H{H}"
            if tag not in by_wl:
                continue
            best_layout = None
            best_val = float('inf')
            for lay in ["CSR", "PCSR", "DP"]:
                v = float(by_wl[tag][lay].get("pure_kernel_total_ms", 0) or 0)
                if v < best_val:
                    best_val = v
                    best_layout = lay
            if best_layout:
                ax.annotate("★", (xi[hi] + offsets[["CSR", "PCSR", "DP"].index(best_layout)],
                                 best_val),
                           fontsize=10, ha='center', va='bottom', color='gold',
                           weight='bold', clip_on=True)

fig.legend(loc='lower center', ncol=3, fontsize=10, bbox_to_anchor=(0.5, -0.02))
fig.tight_layout(rect=[0, 0.04, 1, 0.96])
fig.savefig(OUTPUT, bbox_inches='tight')
print(f"Saved to {OUTPUT}")
