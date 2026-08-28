#!/usr/bin/env python3
"""validate_csr_analytic.py — intermediate CSR estimator gate.

Compares, per validation graph and per class, the EXACT replay oracle
(CSR replay -> exact line stream -> exact RD -> exact N/h2/h3) against the
closed-form AnalyticCSR (graph stats -> analytic N/h2/h3).

A single 100-op workload is a NOISY realization of the analytic expectation
(high-variance insert-position distribution), so the oracle is the MEAN over
N_SEEDS independent replay seeds; the gate compares the analytic value to
that mean within a tolerance scaled by the Monte-Carlo standard error.

Pipeline under test:
    CSR replay -> exact RD -> exact (N, h2, h3)          [trace_gen + rd_hist]
    graph stats -> AnalyticCSR -> analytic (N, h2, h3)   [analytic_rd]

Gates on per-class agreement BEFORE any T vs measured comparison (prevents
cancellation of errors).  Only after the class tiers agree is T_analytic
compared against measured CSR insertion time.
"""
import csv
import json
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import trace_gen as tg
import rd_hist as rh
import analytic_rd as ard
import cost_model as cm

MEASURED = "real_world_runs10_merged.csv"
N_SEEDS = int(os.environ.get("CSR_VALIDATE_SEEDS", "8"))
EXACT_CACHE = "/tmp/opencode/csr_exact_cache.json"

# base tolerances (absolute) plus K*SE scale factors
N_TOL = 2.0          # lines per directed op
H_TOL = 0.02         # absolute hit-ratio tolerance
SE_K = 3.0           # allow up to 3*SE beyond the base tolerance


def graph_labels_from_measured():
    labels = []
    with open(MEASURED) as f:
        for r in csv.DictReader(f):
            if r.get("operation") == "Insert" and r.get("layout") == "CSR":
                g = r["graph"]
                if g not in labels:
                    labels.append(g)
    return labels


def per_class_exact(trace):
    """Exact per-class (N, h2, h3) per directed op from CSR replay + exact RD."""
    counts, total, inf = rh.csr_class_counts(trace)
    out = {}
    for cls, c in counts.items():
        n = sum(c)
        h2, h3 = rh.hit_ratios_from_tiers(c[0], c[1], c[2])
        out[cls] = (n / trace["n_ops"], h2, h3)
    return out


def load_exact_cache():
    if os.path.exists(EXACT_CACHE):
        return json.load(open(EXACT_CACHE))
    return {}


def save_exact_cache(cache):
    json.dump(cache, open(EXACT_CACHE, "w"), indent=0)


def exact_oracle(label):
    """Mean +/- SE of exact per-class (N, h2, h3) over N_SEEDS replays."""
    cache = load_exact_cache()
    key = f"{label}|{N_SEEDS}"
    if key in cache:
        return cache[key]
    per_seed = []
    for seed in range(N_SEEDS):
        tr = tg.gen_trace(graph_label=label, rep="csr", seed=seed)
        per_seed.append(per_class_exact(tr))
    out = {}
    for cls in ("move", "brow", "struct"):
        arr = np.array([[s[cls][0], s[cls][1], s[cls][2]] for s in per_seed])
        mean = arr.mean(axis=0)
        se = arr.std(axis=0, ddof=1) / np.sqrt(len(arr))
        out[cls] = {"N": float(mean[0]), "N_se": float(se[0]),
                    "h2": float(mean[1]), "h2_se": float(se[1]),
                    "h3": float(mean[2]), "h3_se": float(se[2])}
    cache[key] = out
    save_exact_cache(cache)
    return out


def validate(traces, rates):
    ok_all = True
    rows = []
    print(f"{'graph':24s} {'class':8s} {'N_ex':>9s} {'N_an':>9s} "
          f"{'h2_ex':>7s} {'h2_an':>7s} {'h3_ex':>7s} {'h3_an':>7s}")
    for g, tr in sorted(traces.items()):
        oracle = exact_oracle(g)
        an = ard.class_tiers_csr_from_edges(tr["edge_file"], o=0, o_rp=0,
                                            n_ops=tr["n_ops"])
        for cls in ("move", "brow", "struct"):
            o = oracle[cls]
            n_an, h2_an, h3_an = an[cls]
            ok_n = abs(n_an - o["N"]) <= max(N_TOL, SE_K * o["N_se"])
            ok_h2 = abs(h2_an - o["h2"]) <= max(H_TOL, SE_K * o["h2_se"])
            ok_h3 = abs(h3_an - o["h3"]) <= max(H_TOL, SE_K * o["h3_se"])
            ok = ok_n and ok_h2 and ok_h3
            ok_all &= ok
            flag = "" if ok else "   <-- MISMATCH"
            print(f"{g:24s} {cls:8s} {o['N']:9.1f} {n_an:9.1f} "
                  f"{o['h2']:7.4f} {h2_an:7.4f} {o['h3']:7.4f} "
                  f"{h3_an:7.4f}{flag}")
            rows.append({"graph": g, "class": cls,
                         "N_exact": o["N"], "N_se": o["N_se"],
                         "N_analytic": n_an,
                         "h2_exact": o["h2"], "h2_se": o["h2_se"],
                         "h2_analytic": h2_an,
                         "h3_exact": o["h3"], "h3_se": o["h3_se"],
                         "h3_analytic": h3_an, "ok": ok})
    print(f"\nCSR class-tier gate: {'PASS' if ok_all else 'FAIL'}"
          f"  ({N_SEEDS} replay seeds)")
    json.dump(rows, open("/tmp/opencode/csr_analytic_validation.json", "w"),
              indent=1)
    return ok_all


def t_vs_measured(traces, rates):
    meas = {}
    with open(MEASURED) as f:
        for r in csv.DictReader(f):
            if (r.get("operation") == "Insert" and r.get("layout") == "CSR"
                    and r.get("measured_kernel_ns")):
                meas[r["graph"]] = float(r["measured_kernel_ns"])
    print(f"\n{'graph':24s} {'T_analytic':>12s} {'T_meas':>12s} "
          f"{'ratio':>7s}")
    ratios = []
    for g, tr in sorted(traces.items()):
        an = ard.AnalyticCSR(
            ard.csr_structure_from_edges(tr["edge_file"], o=0, o_rp=0),
            n_ops=tr["n_ops"])
        t_an, _ = an.cost(rates)
        m_ns = meas.get(g)
        if m_ns:
            ratios.append(t_an / m_ns)
            print(f"{g:24s} {t_an:12.0f} {m_ns:12.0f} {t_an/m_ns:7.3f}")
    if ratios:
        print(f"\nmedian |ratio-1| (CSR insert, analytic): "
              f"{np.median([abs(x-1) for x in ratios]):.3f}")


def main():
    labels = graph_labels_from_measured()
    traces = {}
    for g in labels:
        try:
            traces[g] = tg.gen_trace(graph_label=g, rep="csr")
        except Exception as e:
            print(f"!! {g}: {e}")
    rates = cm.class_rates_model()
    ok = validate(traces, rates)
    if not ok:
        sys.exit("CSR class-tier gate FAILED — do not interpret T or rankings")
    t_vs_measured(traces, rates)


if __name__ == "__main__":
    main()