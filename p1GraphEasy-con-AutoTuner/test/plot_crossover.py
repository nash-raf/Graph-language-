#!/usr/bin/env python3
"""Plot overlay of predicted (dashed) + measured (solid) from density_scan CSV."""
import argparse
import csv
import json
from pathlib import Path

HOME = Path.home()
COLORS = {"PCSR": "#348ABD", "SET": "#988ED5"}
MARKERS = {"PCSR": "s", "SET": "D"}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("csv", help="CSV from density_scan.py")
    parser.add_argument("-o", "--output", required=True, help="Output PNG")
    parser.add_argument("--n", type=int, help="Override n from CSV (auto-detect by default)")
    args = parser.parse_args()

    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    with open(args.csv) as f:
        reader = csv.DictReader(f)
        rows = list(reader)

    n = args.n or int(rows[0]["n"])
    m_vals = [int(r["m"]) for r in rows]

    fig, ax = plt.subplots(figsize=(10, 7))

    crossover_pred = None
    crossover_meas = None
    prev_pred = None
    prev_meas = None
    for r in rows:
        if prev_pred is not None and r["best_pred"] != prev_pred and crossover_pred is None:
            crossover_pred = int(r["m"])
        if prev_meas is not None and r["best_meas"] != prev_meas and crossover_meas is None:
            crossover_meas = int(r["m"])
        prev_pred = r["best_pred"]
        prev_meas = r["best_meas"]

    layouts = [x for x in ["PCSR", "SET"] if f"pred_{x.lower()}" in rows[0]]

    for layout in layouts:
        pk = f"pred_{layout.lower()}"
        mk = f"meas_{layout.lower()}"
        pred_y = [float(r[pk]) for r in rows]
        meas_y = [float(r[mk]) for r in rows]

        ax.plot(m_vals, pred_y, color=COLORS[layout],
                linestyle="--", marker=MARKERS[layout], markersize=3,
                linewidth=1.8, label=f"{layout} predicted", alpha=0.8)
        ax.plot(m_vals, meas_y, color=COLORS[layout],
                linestyle="-", marker=MARKERS[layout], markersize=4,
                linewidth=2.0, label=f"{layout} measured", alpha=0.9)

    def annotate_crossover(ax, cross_m, label, color):
        if cross_m is not None:
            ax.axvline(x=cross_m, color=color, linestyle=":", linewidth=2, alpha=0.6)
            ax.annotate(f"{label} crossover\nm={cross_m}, d={2*cross_m/n:.1f}",
                        xy=(cross_m, ax.get_ylim()[1] * 0.15),
                        fontsize=10, color=color, ha="center",
                        bbox=dict(boxstyle="round,pad=0.3", fc="white", ec=color, alpha=0.85))

    annotate_crossover(ax, crossover_pred, "Predicted", "#d62728")
    annotate_crossover(ax, crossover_meas, "Measured", "#2ca02c")

    ax.set_xlabel("Edges (m)", fontsize=13)
    ax.set_ylabel("Cost per insert (ns)", fontsize=13)
    ax.set_title(f"Insert cost: predicted vs measured  n={n}", fontsize=14, fontweight="bold")
    ax.legend(fontsize=10, loc="upper left")
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.grid(True, alpha=0.3)

    calib_path = HOME / ".config" / "sgpl" / "hw_calib.json"
    calib_text = "no calibration"
    try:
        calib = json.loads(calib_path.read_text())
        calib_text = f"t={calib['t']:.1f}ns  T={calib['T']:.1f}ns  L={calib['L']}"
    except Exception:
        pass
    fig.text(0.5, 0.01, f"HW calibration: {calib_text}", ha="center", fontsize=9, style="italic")

    fig.tight_layout(rect=[0, 0.03, 1, 0.97])
    fig.savefig(args.output, dpi=150, bbox_inches="tight")
    print(f"[plot] saved to {args.output}")


if __name__ == "__main__":
    main()
