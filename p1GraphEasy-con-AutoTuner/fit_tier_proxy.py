"""fit_tier_proxy.py — fit the compile-time per-class tier-fraction proxy.

Synthetic ER graphs span (n, m) so the bcol array crosses the L2/L3
boundaries; exact move-class RD tier fractions (rd_hist.iter_op_line_rd)
are the training data.  The 9 real-world graphs are held out and validated
at the end.

The pass needs, per access class s: h_L2(s), h_L3(s) = cumulative fraction
of that class's line references whose RD < L2-capacity / L3-capacity.
"""
import json
import os
import random
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rd_hist as rh
import trace_gen as tg

L2 = rh.L2_LINES
L3 = rh.L3_LINES


def make_er(n, m, seed):
    """Edge file for a fresh ER-style undirected graph (u v lines)."""
    path = f"/tmp/opencode/fit_{n}_{m}_{seed}.txt"
    rng = random.Random(seed)
    with open(path, "w") as f:
        pairs = set()
        while len(pairs) < m:
            u = rng.randrange(n)
            v = rng.randrange(n)
            if u == v:
                continue
            lo, hi = (u, v) if u < v else (v, u)
            pairs.add((lo, hi))
            f.write(f"{u} {v}\n")
    return path


def tier_fracs(edge_file):
    tr = tg.gen_trace(edge_file=edge_file, k_ins=50, seed=42)
    o = tr["bcol_base_mod64"][0] // 16
    o_br = tr["brow_base_mod64"][0] // 16
    mv = {"L2": 0, "L3": 0, "DRAM": 0}
    sc = {"L2": 0, "L3": 0, "DRAM": 0}
    for t, ell, rd, inf, mu, rd2, inf2, s_rd in rh.iter_op_line_rd(
            tr, o, o_br):
        op = tr["ops"][t]
        move = (op["ip"] < 16 * ell + 16 - o) & (op["T"] + 2 > 16 * ell - o)
        scan = (op["start"] < 16 * ell + 16 - o) & (op["end"] > 16 * ell - o)
        tier = np.where(inf, 2, 0)
        tier = np.where(~inf & (rd >= L2), 1, tier)
        tier = np.where(~inf & (rd >= L3), 2, tier)
        for name, mask in (("move", move), ("scan", scan)):
            for ti, key in ((0, "L2"), (1, "L3"), (2, "DRAM")):
                (mv if name == "move" else sc)[key] += int(
                    np.count_nonzero(mask & (tier == ti)))
    n_mv = sum(mv.values())
    n_sc = sum(sc.values())
    return {
        "T_lines": (16 * tr["T0"] + 64 - o) / 64,
        "n": tr["n"], "m": tr["m"], "nb": tr["nb"],
        "d": 2.0 * tr["m"] / tr["n"],
        "move_h2": mv["L2"] / n_mv, "move_h3": (mv["L2"] + mv["L3"]) / n_mv,
        "scan_h2": sc["L2"] / n_sc, "scan_h3": (sc["L2"] + sc["L3"]) / n_sc,
    }


def main():
    rng = random.Random(123)
    grids = []
    # n sweeps: bcol lines (16m/64) cross L2 (20480) and L3 (196608)
    for n in (300, 1000, 4000, 16000, 60000, 250000):
        for mult in (0.25, 0.6, 1.4, 3.2, 7.5, 17.0, 40.0, 90.0):
            t_lines = mult * L2
            m = int(t_lines * 64 / 16)      # 16m bytes = t_lines*64
            if m <= 0 or 2 * m / n < 1.0:
                continue
            grids.append((n, m))
    # dedupe + cap
    seen = set()
    points = []
    for n, m in grids:
        key = (n, m)
        if key in seen:
            continue
        seen.add(key)
        points.append((n, m))
    print(f"training on {len(points)} synthetic graphs")
    rows = []
    for i, (n, m) in enumerate(points):
        ef = make_er(n, m, 1000 + i)
        try:
            r = tier_fracs(ef)
        except Exception as e:
            print(f"  skip ({n},{m}): {e}")
            continue
        rows.append(r)
        print(f"  n={r['n']:7d} m={r['m']:8d} d={r['d']:8.2f} "
              f"T_lines={r['T_lines']:10.0f} nb={r['nb']:7d} "
              f"move_h2={r['move_h2']:.3f} move_h3={r['move_h3']:.3f} "
              f"scan_h2={r['scan_h2']:.3f}")
    json.dump(rows, open("/tmp/opencode/tier_fit_synth.json", "w"), indent=1)


if __name__ == "__main__":
    main()
