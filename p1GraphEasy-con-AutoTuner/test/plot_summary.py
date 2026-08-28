#!/usr/bin/env python3
"""Summary plot: predicted vs measured crossover for n=20 and n=50, with discrepancy annotation."""
import csv
import json
from pathlib import Path

HOME = Path.home()
LAYOUTS = ["CSR", "PCSR", "SET"]
COLORS = {"CSR": "#E24A33", "PCSR": "#348ABD", "SET": "#988ED5"}
MARKERS = {"CSR": "o", "PCSR": "s", "SET": "D"}


def read_csv(path):
    rows = []
    with open(path) as f:
        reader = csv.DictReader(f)
        for r in reader:
            r["m"] = int(r["m"])
            r["n"] = int(r["n"])
            r["density"] = float(r["density"])
            for l in LAYOUTS:
                r[f"pred_{l.lower()}"] = float(r[f"pred_{l.lower()}"])
                r[f"meas_{l.lower()}"] = float(r[f"meas_{l.lower()}"])
            rows.append(r)
    return rows


def main():
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    calib_path = HOME / ".config" / "sgpl" / "hw_calib.json"
    calib_text = "no calibration"
    try:
        calib = json.loads(calib_path.read_text())
        calib_text = f"t={calib['t']:.1f}ns  T={calib['T']:.1f}ns  L={calib['L']}"
    except Exception:
        pass

    fig, axes = plt.subplots(2, 2, figsize=(16, 10), sharex="col", sharey="row")

    for col, (csv_path, n_label) in enumerate([
        ("/tmp/density_scan_n20_llv2lmmy/data_n20.csv", "n=20"),
        ("/tmp/density_scan_n50_rvjziozl/data_n50.csv", "n=50"),
    ]):
        results = read_csv(csv_path)
        m_vals = [r["m"] for r in results]

        for row, (ax, src, style_label) in enumerate([
            (axes[0][col], "pred", "Predicted"),
            (axes[1][col], "meas", "Measured"),
        ]):
            for layout in LAYOUTS:
                key = f"{src}_{layout.lower()}"
                y_vals = [r[key] for r in results]
                ax.plot(m_vals, y_vals,
                        color=COLORS[layout],
                        linestyle="--" if src == "pred" else "-",
                        marker=MARKERS[layout], markersize=3,
                        linewidth=1.5, alpha=0.85,
                        label=layout)

            # Find where predicted/measured best changes
            best_key = "best_pred" if src == "pred" else "best_meas"
            prev = None
            for r in results:
                cur = r[best_key]
                if prev is not None and cur != prev:
                    ax.axvline(x=r["m"], color="red", linestyle=":", alpha=0.5, linewidth=1)
                    ax.annotate(f"m={r['m']}\nd={r['density']:.1f}",
                                xy=(r["m"], ax.get_ylim()[1] * 0.5),
                                fontsize=8, color="red", ha="center",
                                bbox=dict(boxstyle="round,pad=0.2", fc="white", ec="red", alpha=0.6))
                prev = cur

            # Star markers for best layout at each point
            for r in results:
                best = r[best_key]
                best_key2 = f"{src}_{best.lower()}"
                ax.scatter(r["m"], r[best_key2],
                           color=COLORS[best], marker="*", s=80, zorder=5,
                           edgecolors="black", linewidths=0.5)

            if row == 1:
                ax.set_xlabel("Edges (m)", fontsize=11)
            ax.set_ylabel("Cost per insert (ns)", fontsize=11)
            if row == 0:
                ax.set_title(f"{n_label} — {style_label}", fontsize=12, fontweight="bold")
            ax.legend(fontsize=7, loc="upper left")
            ax.set_xscale("log")
            ax.set_yscale("log")
            ax.grid(True, alpha=0.3, which="both")
            ax.set_xlim(left=min(m_vals) * 0.85, right=max(m_vals) * 1.15)

    # Add discrepancy annotation
    ax = axes[1][1]  # bottom-right panel (n=50 measured)
    bbox = dict(boxstyle="round,pad=0.5", fc="lightyellow", ec="gold", alpha=0.85)
    ax.annotate(
        "DISCREPANCY:\nModel predicts SET wins\nat m>90 (d>3.6),\nbut PCSR remains\nbest in reality",
        xy=(0.98, 0.05), xycoords="axes fraction", fontsize=9,
        ha="right", va="bottom", bbox=bbox)

    ax = axes[1][0]  # bottom-left panel (n=20 measured)
    ax.annotate(
        "DISCREPANCY:\nModel predicts SET wins\nat m>20 (d>2.0),\nbut PCSR remains\nbest in reality",
        xy=(0.98, 0.05), xycoords="axes fraction", fontsize=9,
        ha="right", va="bottom", bbox=bbox)

    ax = axes[0][1]  # top-right panel (n=50 predicted)
    ax.annotate(
        "Agreement:\nPCSR predicted\nbest at low d",
        xy=(0.98, 0.95), xycoords="axes fraction", fontsize=8,
        ha="right", va="top",
        bbox=dict(boxstyle="round,pad=0.3", fc="lightgreen", ec="green", alpha=0.6))

    fig.suptitle(f"Insert cost crossover: Predicted vs Measured  |  {calib_text}",
                 fontsize=12, y=1.01)
    fig.tight_layout()
    out = "/home/kazisahib/sgpl_main/Graph-language-/p1GraphEasy-con-AutoTuner/test/crossover_summary.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    print(f"[plot] saved to {out}")


if __name__ == "__main__":
    main()
