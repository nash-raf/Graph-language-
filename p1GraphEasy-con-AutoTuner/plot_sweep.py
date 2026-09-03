import sys, csv, math, os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

rows = []
for r in csv.DictReader(sys.stdin):
    rows.append({
        "ws": int(r["ws_bytes"]),
        "ratio": float(r["ws_llc_ratio"]),
        "ns": float(r["ns_per_line"]),
        "penalty": float(r["penalty"]),
    })

if not rows:
    print("no data", file=sys.stderr)
    sys.exit(1)

llc = rows[0]["ws"] / rows[0]["ratio"] if rows[0]["ratio"] > 0 else 8*1024*1024

ratios = [r["ratio"] for r in rows]
penalties = [r["penalty"] for r in rows]

best_P = max(penalties)

# detect ramp lo/hi from measured data
found_lo = None
found_hi = None
for r in rows:
    if r["penalty"] <= 1.05:
        found_lo = r["ratio"]
for r in rows:
    if r["penalty"] >= 0.95 * best_P and found_hi is None:
        found_hi = r["ratio"]
if found_lo is None:
    found_lo = min(ratios)
if found_hi is None:
    found_hi = max(ratios)

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))

# -- left: linear x --
ax1.scatter(ratios, penalties, color="tab:blue", s=50, zorder=5,
            label=f"Empirical ({len(rows)} points, min-of-7 trials)")

xs = np.linspace(min(ratios), max(ratios), 400)
log_pen = np.where(
    xs <= found_lo, 1.0,
    np.where(xs >= found_hi, best_P,
             1.0 + (best_P - 1.0) * np.log(xs / found_lo) / np.log(found_hi / found_lo))
)
ax1.plot(xs, log_pen, "r--", linewidth=2,
         label=f"Log-linear ramp (lo={found_lo:.2f}×LLC, hi={found_hi:.2f}×LLC, P={best_P:.2f})")

lin_pen = np.where(
    xs <= found_lo, 1.0,
    np.where(xs >= found_hi, best_P,
             1.0 + (best_P - 1.0) * (xs - found_lo) / (found_hi - found_lo))
)
ax1.plot(xs, lin_pen, ":", color="gray", linewidth=1.5, label="Linear ramp (same bounds)")

ax1.axhline(1.0, color="green", linestyle="--", linewidth=0.8, alpha=0.5)
ax1.axhline(best_P, color="red", linestyle="--", linewidth=0.8, alpha=0.5)
ax1.axvline(1.0, color="gray", linestyle=":", linewidth=0.8, alpha=0.5)
ax1.text(1.02, ax1.get_ylim()[0] + 0.05, "LLC", fontsize=9, color="gray")

info = f"LLC = {llc/1024/1024:.1f} MB\nP = {best_P:.2f}\nlo = {found_lo:.2f}×LLC\nhi = {found_hi:.2f}×LLC"
ax1.text(0.97, 0.03, info, transform=ax1.transAxes, fontsize=9,
         verticalalignment="bottom", horizontalalignment="right",
         bbox=dict(boxstyle="round", facecolor="wheat", alpha=0.8))

ax1.set_xlabel("Working set size (× LLC)")
ax1.set_ylabel("Penalty factor (per-line cost / Tm)")
ax1.set_title("Linear X-axis")
ax1.legend(fontsize=8)
ax1.grid(True, alpha=0.3)

# -- right: log x --
ax2.scatter(ratios, penalties, color="tab:blue", s=50, zorder=5,
            label="Empirical (min-of-7)")

ax2.plot(xs, log_pen, "r--", linewidth=2,
         label=f"Log-linear ramp")
ax2.plot(xs, lin_pen, ":", color="gray", linewidth=1.5, label="Linear ramp")

ax2.axhline(1.0, color="green", linestyle="--", linewidth=0.8, alpha=0.5)
ax2.axhline(best_P, color="red", linestyle="--", linewidth=0.8, alpha=0.5)
ax2.axvline(1.0, color="gray", linestyle=":", linewidth=0.8, alpha=0.5)

ax2.set_xlabel("Working set size (× LLC, log scale)")
ax2.set_ylabel("Penalty factor (per-line cost / Tm)")
ax2.set_title("Log X-axis")
ax2.set_xscale("log")
ax2.legend(fontsize=8)
ax2.grid(True, alpha=0.3, which="both")

fig.suptitle("Memmove penalty: empirical sweep vs ramp models", fontsize=13)
plt.tight_layout()

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "sweep_plot.png")
fig.savefig(out, dpi=150, bbox_inches="tight")
print(f"Wrote {out}", file=sys.stderr)
