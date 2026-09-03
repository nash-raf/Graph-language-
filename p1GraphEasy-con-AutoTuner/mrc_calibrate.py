"""mrc_calibrate.py — Step 3: MRC + memory-tier rate decomposition.

From the RD histogram p(d) (step 2, /tmp/opencode/rd_histograms.json):

    MR(C) = p(INF) + sum_{d >= C} p(d)        # miss rate at C cache lines
    h(C)  = 1 - MR(C)                         # hit rate

Per-access cost with three-tier hierarchy (L2 -> L3 -> DRAM):

    r = h_L2 * r_L2  +  (h_L3 - h_L2) * r_L3  +  (1 - h_L3) * r_DRAM

where h_L2 = h(L2 lines), h_L3 = h(L3 lines), and r_* are the per-access
costs of an L2 hit, an L3 hit, and a DRAM access.

Calibration (i5-1235U, Alder Lake, max turbo 4.4 GHz = 0.2273 ns/cyc):
    r_L2   = 14  cycles (documented L2 hit latency)
    r_L3   = 45  cycles (documented L3 hit latency)
    r_DRAM = 110 cycles (random-access DRAM latency)
These are scalars in ns; override via R_L2_NS / R_L3_NS / R_DRAM_NS env.
The decomposition (share of r paid to each tier) is the key output: it
separates "fits in L2" behavior from "L3-resident but DRAM-missing".
"""
import json
import math
import os
import sys

import numpy as np

HIST = "/tmp/opencode/rd_histograms.json"
OUT = "/tmp/opencode/mrc_results.json"

L2_LINES = 1310720 // 64      # 20480
L3_LINES = 12582912 // 64     # 196608

R_L2 = float(os.environ.get("R_L2_NS", 14 / 4.4e3 * 1e3 * 1e-3 * 1e9 / 1e9))
# 14 cycles at 4.4 GHz -> ns: cycles / (cycles/ns) = 14 / 4.4 = 3.1818 ns
R_L2 = float(os.environ.get("R_L2_NS", 14 / 4.4))
R_L3 = float(os.environ.get("R_L3_NS", 45 / 4.4))
R_DRAM = float(os.environ.get("R_DRAM_NS", 110 / 4.4))


def load():
    data = json.load(open(HIST))
    for g, s in data.items():
        s["hist"] = {int(k): v for k, v in s["hist"].items()}
    return data


def mrc_from_hist(hist, total, inf_cnt):
    """Returns (sorted finite-bins array, suffix-mass p(d>=C) fn)."""
    bins = sorted((d, c) for d, c in hist.items())
    p_inf = inf_cnt / total
    return bins, p_inf


def miss_rate(hist, total, inf_cnt, C):
    """MR(C) = p(INF) + sum_{d >= C} p(d)."""
    mr = inf_cnt / total
    for d, c in hist.items():
        if d >= C:
            mr += c / total
    return mr


def hit_rate(hist, total, inf_cnt, C):
    return 1.0 - miss_rate(hist, total, inf_cnt, C)


def decompose(hist, total, inf_cnt):
    h_l2 = hit_rate(hist, total, inf_cnt, L2_LINES)
    h_l3 = hit_rate(hist, total, inf_cnt, L3_LINES)
    f_l2 = h_l2 * R_L2
    f_l3 = (h_l3 - h_l2) * R_L3
    f_dram = (1 - h_l3) * R_DRAM
    r = f_l2 + f_l3 + f_dram
    return {
        "h_l2": h_l2,
        "h_l3": h_l3,
        "mr_l2": 1 - h_l2,
        "mr_l3": 1 - h_l3,
        "tier_fraction_l2": f_l2 / r if r else 0,
        "tier_fraction_l3": f_l3 / r if r else 0,
        "tier_fraction_dram": f_dram / r if r else 0,
        "cost_l2_ns": f_l2,
        "cost_l3_ns": f_l3,
        "cost_dram_ns": f_dram,
        "r_ns_per_access": r,
    }


def mrc_curve(hist, total, inf_cnt):
    """Sweep C over 0.5KB .. 256MB (in lines)."""
    lines = np.geomspace(64, 4 * 1024 * 1024, 96).astype(np.int64)
    return [(int(c), miss_rate(hist, total, inf_cnt, int(c))) for c in lines]


def main():
    data = load()
    rows = []
    for g, s in sorted(data.items()):
        d = decompose(s["hist"], s["total_refs"], s["inf_refs"])
        d["graph"] = g
        d["total_refs"] = s["total_refs"]
        rows.append(d)
        print(f"{g:24s} h2={d['h_l2']*100:6.2f}% h3={d['h_l3']*100:6.2f}% "
              f"L3miss={d['mr_l3']*100:6.2f}% | "
              f"r={d['r_ns_per_access']:6.2f} ns/acc "
              f"(L2 {d['tier_fraction_l2']*100:5.1f}% | "
              f"L3 {d['tier_fraction_l3']*100:5.1f}% | "
              f"DRAM {d['tier_fraction_dram']*100:5.1f}%)")

    curves = {g: mrc_curve(s["hist"], s["total_refs"], s["inf_refs"])
              for g, s in sorted(data.items())}
    out = {
        "rates_ns": {"l2": R_L2, "l3": R_L3, "dram": R_DRAM,
                     "source": "i5-1235U Alder Lake documented cycles at 4.4GHz"},
        "rows": rows,
        "mrc": curves,
    }
    json.dump(out, open(OUT, "w"), indent=1)

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(9, 6))
        for g, cv in curves.items():
            xs = [c for c, _ in cv]
            ys = [v for _, v in cv]
            ax.semilogx(xs, [v * 100 for v in ys], label=g, lw=1.6)
        ax.axvline(L2_LINES, color="k", ls=":", lw=1)
        ax.axvline(L3_LINES, color="k", ls="--", lw=1)
        ax.text(L2_LINES * 1.1, 80, "L2", fontsize=9)
        ax.text(L3_LINES * 1.1, 80, "L3", fontsize=9)
        ax.set_xlabel("cache capacity C (lines)")
        ax.set_ylabel("miss rate MR(C) (%)")
        ax.set_title("BCSR add workload — miss rate curves from RD histograms")
        ax.legend(fontsize=8, ncol=2)
        ax.grid(True, which="both", alpha=0.3)
        fig.tight_layout()
        fig.savefig("/tmp/opencode/mrc_curves.png", dpi=150)
        print("\nwrote /tmp/opencode/mrc_curves.png")
    except Exception as e:
        print(f"\n(plot skipped: {e})")
    print(f"wrote {OUT}")


if __name__ == "__main__":
    main()
