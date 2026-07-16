#!/usr/bin/env python3
"""
Analyze ablation results: compute Var(ratio_diff), produce summary tables.
Usage:
    python3 analyze_ablation.py                          # reads ablation_results.csv
    python3 analyze_ablation.py --csv path/to/results.csv
"""

import argparse
import csv
import math
import sys
import os


def load_results(path):
    with open(path) as f:
        return list(csv.DictReader(f))


def mean_std(vals):
    n = len(vals)
    if n == 0:
        return 0.0, 0.0
    m = sum(vals) / n
    var = sum((x - m) ** 2 for x in vals) / n
    return m, math.sqrt(var)


def main():
    parser = argparse.ArgumentParser(
        description="Analyze ablation study results.")
    parser.add_argument("--csv", default="ablation_results.csv",
                        help="Path to results CSV")
    parser.add_argument("--out", default="ablation_analysis.txt",
                        help="Path to summary output file")
    args = parser.parse_args()

    results = load_results(args.csv)
    if not results:
        print("No results found.")
        return 1

    # Parse numeric fields
    for r in results:
        r["ratio_diff"] = float(r["ratio_diff"])
        r["ratio_meas"] = float(r["ratio_meas"])
        r["ratio_pred"] = float(r["ratio_pred"])
        r["n"] = int(r["n"])
        r["d"] = int(r["d"])
        r["H"] = int(r["H"])
        r["conv_mean_ns"] = float(r["conv_mean_ns"])
        r["total_meas_ns"] = float(r["total_meas_ns"])

    # ── Overall variance ──
    diffs = [r["ratio_diff"] for r in results if math.isfinite(r["ratio_diff"])]
    mean_diff, std_diff = mean_std(diffs)

    lines = []
    lines.append("=" * 72)
    lines.append("Ablation Study: Conversion Cost Model Distortion")
    lines.append("=" * 72)
    lines.append(f"Total workloads:       {len(results)}")
    lines.append(f"Valid diffs:           {len(diffs)}")
    lines.append(f"Mean(ratio_diff):      {mean_diff:.6f}")
    lines.append(f"Std(ratio_diff):       {std_diff:.6f}")
    lines.append(f"Var(ratio_diff):       {std_diff**2:.6f}")
    lines.append("")

    # ── Grouped by from_layout ──
    lines.append("-" * 72)
    lines.append("By from_layout (source layout before SET conversion):")
    lines.append("-" * 72)
    groups = {}
    for r in results:
        fl = r.get("from_layout", "UNKNOWN")
        groups.setdefault(fl, []).append(r["ratio_diff"])
    for fl in sorted(groups.keys(), key=lambda k: -len(groups[k])):
        vals = [v for v in groups[fl] if math.isfinite(v)]
        if not vals:
            continue
        m, s = mean_std(vals)
        lines.append(f"  {fl:6s}:  N={len(vals):3d}  mean_diff={m:+.6f}  std={s:.6f}  var={s**2:.6f}")
    lines.append("")
    lines.append(f"Note: 'from_layout' reveals which layout the DP chose for the Traverse")
    lines.append(f"region. A positive mean_diff means conversion is under-predicted")
    lines.append(f"(measured cost > predicted cost).")
    lines.append("")

    # ── Grouped by (n, d) — graph size and density ──
    lines.append("-" * 72)
    lines.append("By graph size (n) and density (d = 2m/n):")
    lines.append("-" * 72)
    groups_nd = {}
    for r in results:
        key = (r["n"], r["d"])
        groups_nd.setdefault(key, []).append(r["ratio_diff"])
    for key in sorted(groups_nd.keys()):
        vals = [v for v in groups_nd[key] if math.isfinite(v)]
        if not vals:
            continue
        m, s = mean_std(vals)
        lines.append(f"  n={key[0]:6d}  d={key[1]:3d}:  N={len(vals):3d}  "
                     f"mean_diff={m:+.6f}  std={s:.6f}  var={s**2:.6f}")
    lines.append("")

    # ── Grouped by H (loop count = op repetition) ──
    lines.append("-" * 72)
    lines.append("By operation count H (bfs loop iterations):")
    lines.append("-" * 72)
    groups_h = {}
    for r in results:
        key = r["H"]
        groups_h.setdefault(key, []).append(r["ratio_diff"])
    for h in sorted(groups_h.keys()):
        vals = [v for v in groups_h[h] if math.isfinite(v)]
        if not vals:
            continue
        m, s = mean_std(vals)
        lines.append(f"  H={h:4d}:  N={len(vals):3d}  mean_diff={m:+.6f}  std={s:.6f}  var={s**2:.6f}")
    lines.append("")

    # ── Per-workload detail ──
    lines.append("-" * 72)
    lines.append("Per-workload detail:")
    lines.append("-" * 72)
    header = f"{'workload':28s} {'fl':6s} {'n':6s} {'d':3s} {'H':4s}  {'ratio_meas':>10s}  {'ratio_pred':>10s}  {'diff':>10s}"
    lines.append(header)
    lines.append("-" * len(header))
    for r in results:
        d = r["ratio_diff"]
        tag = r["workload"]
        lines.append(
            f"{tag:28s} {r['from_layout']:6s} {r['n']:6d} {r['d']:3d} {r['H']:4d}  "
            f"{r['ratio_meas']:>10.6f}  {r['ratio_pred']:>10.6f}  "
            f"{d:>+10.6f}")
    lines.append("")

    # ── Summary ──
    lines.append("=" * 72)
    lines.append("FINAL METRIC: Var(ratio_diff) = {:.6f}".format(std_diff**2))
    lines.append("  (Lower = conversion model tracks the tradeoff more faithfully)")
    lines.append("=" * 72)

    # Print and write
    output = "\n".join(lines)
    print(output)

    if args.out:
        with open(args.out, "w") as f:
            f.write(output + "\n")
        print(f"Written to {args.out}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
