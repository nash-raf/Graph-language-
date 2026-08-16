"""analyze_shift_state.py — Decompose the BCSR cMove slowdown from
shift_state_bench.json:  t_A (clean memmove) -> t_B (+realloc)
-> t_C (+dup-scan) -> t_D (full add path), all at matched size/offset/move.

Reports per (ws, frac, scan):
  t_A t_B t_C t_D, deltas, realloc outcome split, the cost-model curve's
  prediction at the same move size, and a Case 1..4 classification:
    Case 1: A ≈ B ≈ C ≈ D          -> footprint/usage bug, not cache state
    Case 2: A << B ≈ C ≈ D         -> realloc leaves the shift cold
    Case 3: A ≈ B << C ≈ D         -> dup-scan perturbs the cache
    Case 4: A << B << C << D       -> multiple contextual effects
Also overlays the six measured BCSR insert cells (kernel-implied shift rate)
against t_D at the matching bcol footprint.
"""
import csv
import json
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import cost_model as cm

SCAN_EDGES = (0.9, 1.0)  # ws band for cache-resident classification


def load_points(path):
    with open(path) as f:
        data = json.load(f)
    pts = {}
    for p in data["points"]:
        key = (p["v"], p["ws"], p["frac"], p["scan"])
        pts[key] = p
    return pts


def curve_per_line(move_bytes):
    return cm._curve_per_line(move_bytes)


def classify(tA, tB, tC, tD):
    eps = 0.35  # ns/line noise floor
    d_re = tB - tA          # realloc-state effect
    d_sc = tC - tB          # dup-scan effect
    d_re2 = tD - tC         # insert-pos scan + prefix + residual
    if abs(d_re) <= eps and abs(d_sc) <= eps and abs(d_re2) <= eps:
        return "Case 1: A=B=C=D (footprint/usage bug)"
    if d_re > eps and abs(d_sc) <= eps and abs(d_re2) <= eps:
        return "Case 2: realloc-state dominates"
    if abs(d_re) <= eps and d_sc > eps and abs(d_re2) <= eps:
        return "Case 3: dup-scan dominates"
    return "Case 4: multiple contextual effects"


def main():
    res_path = sys.argv[1] if len(sys.argv) > 1 else "/tmp/opencode/shift_state.json"
    insitu_path = sys.argv[2] if len(sys.argv) > 2 else "/tmp/insitu_profiles"
    pts = load_points(res_path)
    ws_list = sorted({k[1] for k in pts})
    frac_list = sorted({k[2] for k in pts})
    scan_list = sorted({k[3] for k in pts})

    print(f"{'ws':>8} {'frac':>4} {'scan':>5} "
          f"{'tA':>7} {'tB':>7} {'tC':>7} {'tD':>7} | "
          f"{'dB':>7} {'dC':>7} {'dD':>7} | {'curve':>7} | "
          f"{'in/mig(B)':>9}")
    for ws in ws_list:
        for frac in frac_list:
            for scan in scan_list:
                t = {}
                for v in "ABCD":
                    t[v] = pts[(v, ws, frac, scan)]["per_line"]
                d_re = t["B"] - t["A"]
                d_sc = t["C"] - t["B"]
                d_re2 = t["D"] - t["C"]
                move = (1.0 - frac) * ws
                curve = curve_per_line(move)
                n_in = pts[("B", ws, frac, scan)]["n_inplace"]
                n_mi = pts[("B", ws, frac, scan)]["n_migrated"]
                print(
                    f"{ws/2**20:7.0f}M {frac:5.2f} {scan:5d} "
                    f"{t['A']:7.2f} {t['B']:7.2f} {t['C']:7.2f} {t['D']:7.2f} | "
                    f"{d_re:7.2f} {d_sc:7.2f} {d_re2:7.2f} | "
                    f"{curve:7.2f} | {n_in:3d}/{n_mi:3d}"
                )

    print("\n--- Case classification (median frac=0.5) ---")
    for ws in ws_list:
        for scan in scan_list:
            t = {v: pts[(v, ws, 0.5, scan)]["per_line"] for v in "ABCD"}
            print(f"ws={ws/2**20:5.0f}M scan={scan:5d}: "
                  f"A={t['A']:.2f} B={t['B']:.2f} C={t['C']:.2f} D={t['D']:.2f}  "
                  f"{classify(t['A'], t['B'], t['C'], t['D'])}")

    print("\n--- In-place vs migrated (B variant) ---")
    for ws in ws_list:
        p = pts[("B", ws, 0.5, 256)]
        print(f"ws={ws/2**20:5.0f}M: t_inplace={p['t_inplace']:.2f} "
              f"t_migrated={p['t_migrated']:.2f} "
              f"({p['n_inplace']} in-place, {p['n_migrated']} migrated)")

    print("\n--- In-situ BCSR shift rates (real kernel, matched avg move) ---")
    try:
        rows = []
        import glob
        for pf in sorted(glob.glob(os.path.join(insitu_path, "*.jsonl"))):
            with open(pf) as f:
                for l in f:
                    l = l.strip()
                    if l:
                        rows.append((os.path.basename(pf)[:-6], json.loads(l)))
    except (FileNotFoundError, TypeError):
        rows = []
    if rows:
        import statistics
        groups = {}
        for g, p in rows:
            groups.setdefault(g, []).append(p)
        print(f"{'graph':22s} {'avgMoveMB':>9s} {'med/line':>8s} "
              f"{'min/max':>13s} {'relocs':>7s} {'curve':>7s} {'ratio':>6s}")
        for g, pl in sorted(groups.items()):
            med = statistics.median(p["shift_per_line"] for p in pl)
            avgmove = sum(p["shift_bytes"] for p in pl) / max(sum(p["shift_cnt"] for p in pl), 1)
            relocs = max(p["reloc_cnt"] for p in pl)
            curv = curve_per_line(avgmove)
            print(f"{g:22s} {avgmove/2**20:9.2f} {med:8.3f} "
                  f"{min(p['shift_per_line'] for p in pl):6.3f}/"
                  f"{max(p['shift_per_line'] for p in pl):.3f} {relocs:7d} "
                  f"{curv:7.2f} {med/curv:6.2f}")
    else:
        print("(no in-situ profiles; rerun insitu_probe.py and pass its dir)")


if __name__ == "__main__":
    main()