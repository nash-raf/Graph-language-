import sys, math, os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cost_model as c

# ── Data (read-only — never rewrites hw_calib.json) ─────────────────────────
curve = [tuple(p) for p in c._CURVE]          # measured memmove_curve anchors
curve.sort(key=lambda p: p[0])
Tm = c.Tm
lo, hi = c._ramp_band()
P = c.P_dram
print(f"anchors={len(curve)}  lo={lo:,.0f} B  hi={hi:,.0f} B  P={P:.4f}  Tm={Tm:.4f}",
      file=sys.stderr)

def curve_penalty(ws):                        # NEW model: mem_penalty(ws)
    return c.mem_penalty(ws)

def prev_penalty(ws):                         # OLD model: log-linear ramp 1.0 → P
    return c._ramp_penalty(ws, lo, hi, P)

def lin_penalty(ws):                          # linear ramp over same band
    if ws <= lo:
        return 1.0
    if ws >= hi:
        return P
    return 1.0 + (P - 1.0) * (ws - lo) / (hi - lo)

# Measured anchor penalties, on the same axis as the models (mul / Tm) —
# this is exactly the "working set" axis the pass evaluates memPenalty at.
ws_pts = [p[0] for p in curve]
pen_pts = [p[1] / Tm for p in curve]

grid = np.logspace(np.log10(ws_pts[0] * 0.8), np.log10(ws_pts[-1] * 1.2), 600)

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))

# ── left: linear x ──────────────────────────────────────────────────────
ax1.scatter(ws_pts, pen_pts, color="tab:blue", s=42, zorder=6,
            label=f"Measured memmove anchors ({len(ws_pts)} pts)")
ax1.plot(grid, [curve_penalty(w) for w in grid], "g-", linewidth=2.2,
         label="NEW: curve-interpolated mem_penalty (log-log linear)")
ax1.plot(grid, [prev_penalty(w) for w in grid], "r--", linewidth=2,
         label=f"PREVIOUS: log-linear ramp lo={lo/2**20:.2f}MiB, hi={hi/2**20:.2f}MiB, P={P:.2f}")
ax1.plot(grid, [lin_penalty(w) for w in grid], ":", color="gray", linewidth=1.5,
         label="LINEAR ramp (same band)")
ax1.axhline(1.0, color="green", linestyle="--", linewidth=0.8, alpha=0.5)
ax1.axhline(P, color="red", linestyle="--", linewidth=0.8, alpha=0.5)
for x, lbl in ((lo, "ramp_lo"), (hi, "ramp_hi")):
    ax1.axvline(x, color="gray", linestyle=":", linewidth=0.8, alpha=0.6)
ax1.text(lo, ax1.get_ylim()[0] + 0.06, "ramp_lo", fontsize=8, color="gray")
ax1.text(hi, ax1.get_ylim()[0] + 0.06, "ramp_hi", fontsize=8, color="gray")

ax1.set_xlabel("Working set size (bytes)")
ax1.set_ylabel("Memory penalty multiplier (per-line cost / Tm)")
ax1.set_title("Linear X-axis")
ax1.set_ylim(0, P * 1.15)
ax1.legend(fontsize=8)
ax1.grid(True, alpha=0.3)

# ── left: log X ─────────────────────────────────────────────────────────
ax2.scatter(ws_pts, pen_pts, color="tab:blue", s=42, zorder=6,
            label="Measured anchors")
ax2.plot(grid, [curve_penalty(w) for w in grid], "g-", linewidth=2.2,
         label="NEW: curve-interpolated mem_penalty")
ax2.plot(grid, [prev_penalty(w) for w in grid], "r--", linewidth=2,
         label="PREVIOUS: log-linear ramp")
ax2.plot(grid, [lin_penalty(w) for w in grid], ":", color="gray", linewidth=1.5,
         label="LINEAR ramp (same band)")
ax2.axhline(1.0, color="green", linestyle="--", linewidth=0.8, alpha=0.5)
ax2.axhline(P, color="red", linestyle="--", linewidth=0.8, alpha=0.5)
ax2.set_xscale("log")
ax2.set_xlabel("Working set size (bytes, log scale)")
ax2.set_ylabel("Memory penalty multiplier")
ax2.set_title("Log X-axis")
ax2.set_ylim(0, P * 1.15)
ax2.legend(fontsize=8)
ax2.grid(True, alpha=0.3, which="both")

info = (f"Tm={Tm:.4f} ns/line\n"
        f"P={P:.4f}\n"
        f"anchors={len(ws_pts)}\n"
        f"lo={lo/2**20:.2f} MiB\nhi={hi/2**20:.2f} MiB")
ax2.text(0.97, 0.03, info, transform=ax2.transAxes, fontsize=9,
         verticalalignment="bottom", horizontalalignment="right",
         bbox=dict(boxstyle="round", facecolor="wheat", alpha=0.8))

fig.suptitle("Memory penalty: measured curve data vs NEW curve fit vs PREVIOUS log-ramp vs LINEAR",
             fontsize=13)
plt.tight_layout()

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "mem_penalty_plot.png")
fig.savefig(out, dpi=150, bbox_inches="tight")
print(f"Wrote {out}", file=sys.stderr)