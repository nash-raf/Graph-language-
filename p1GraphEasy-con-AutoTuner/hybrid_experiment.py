#!/usr/bin/env python3
"""Hybrid experiment (SEPARATE from production — no changes to
cost_model.py / AutoTunerPass.cpp).

Merge legacy + class-tier: the legacy structural equations (locate/write/
move/realloc) keep their volumes, but every memory-penalty term
(Tm·memPenalty(W)) gets its per-line cost replaced by the class-tier
weighted per-line rate for that stream:

  CSR  C_write <- ceil(8n/L)   * wrate(brow)      (was 2T·p(8n))
       C_move  <- ceil(4m·csrFrac/L) * wrate(move) (was Tm·p(...))
  BCSR C_move  <- ceil(16m·bcsrFrac/L) * wrate(move)
       C_realloc <- min(ceil(16m/L)·wrate(struct), R)·2

wrate(cls) = h2·r_L2 + (h3−h2)·r_L3 + (1−h3)·r_DRAM from the class-tier
payload (rates as in the class-tier model: CSR move→csr_move, brow→dep;
BCSR move/brow→rmw, struct→rand).

Compares full representation rankings (legacy vs aware vs hybrid vs
measured) and reports CSR/BCSR absolute accuracy."""
import csv
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cost_model as cm
import analytic_rd as ard
import bench_folder as bf

DATASET = "/home/kazisahib/sgpl_main/dataset"
CSVS = [("real_world", "real_world_runs10_merged.csv", ["real graphs", "new real"]),
        ("barabasi", "barabasi_albert_runs10.csv", ["barabasi_albert"]),
        ("erdos", "erdos_renyi_runs10.csv", ["erdos_renyi"])]
FOLDERS = {d: f for d, _, f in CSVS}
LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]
N_OPS = 100


def edge_file(g, ds):
    for folder in FOLDERS[ds]:
        p = os.path.join(DATASET, folder, g + ".txt")
        if os.path.exists(p):
            return p
    return None


def wrate(tiers, cls, rates):
    _, h2, h3 = tiers[cls]
    r = rates[cls]
    return h2 * r[0] + (h3 - h2) * r[1] + (1.0 - h3) * r[2]


def hybrid_insert_csr(n, m, csr_frac, tiers, rates):
    d = 2.0 * m / n if n > 0 else 1.0
    cLocate = 2 * cm.t + np.ceil(d * 4 / cm.L) * cm.T
    cWrite = np.ceil(8.0 * n / cm.L) * wrate(tiers, "brow", rates)
    cMove = np.ceil(4.0 * m * csr_frac / cm.L) * wrate(tiers, "move", rates)
    cRealloc = cm.R + np.ceil(4.0 * m / cm.L) * cm.T
    return cLocate + cWrite + cMove + cRealloc


def hybrid_insert_bcsr(n, m, bcsr_frac, tiers, rates):
    d = 2.0 * m / n if n > 0 else 1.0
    b = cm.kBcsr
    nb = np.ceil(n / b)
    cLocate = 4.0 * cm.t + 2.0 * np.ceil(8.0 * b * d / cm.L) * cm.T
    cWrite = np.ceil(nb * 4.0 / cm.L) * 2.0 * cm.T
    cMove = np.ceil(16.0 * m * bcsr_frac / cm.L) * wrate(tiers, "move", rates)
    cRealloc = min(np.ceil(16.0 * m / cm.L) * wrate(tiers, "struct", rates),
                   cm.R) * 2.0
    return cLocate + cWrite + cMove + cRealloc


def rank_of(costs):
    return {l: i + 1 for i, (_, l) in enumerate(sorted((c, l) for l, c in costs.items()))}


def main():
    meas = {}
    for ds, csvn, _ in CSVS:
        for r in csv.DictReader(open(csvn)):
            if r.get("measured_kernel_ns"):
                meas[(ds, r["graph"], r["operation"], r["layout"])] = float(
                    r["measured_kernel_ns"])

    rates = cm.class_rates_model()
    # class-tier rate mapping used by the hybrid (same as the class-tier model)
    rates_hyb = dict(rates)
    rates_hyb["brow"] = rates["dep"] if False else rates["brow"]
    # per-graph cached tiers
    tiers_cache = {}

    def tiers_for(g, ds, layout):
        key = (g, ds, layout)
        if key not in tiers_cache:
            ef = edge_file(g, ds)
            if layout == "CSR":
                t = ard.class_tiers_csr_from_edges(ef, o=0, o_rp=0, n_ops=N_OPS)
            else:
                t = ard.class_tiers_from_edges(ef, o=0, o_br=0, n_ops=N_OPS)
            tiers_cache[key] = t
        return tiers_cache[key]

    results = []
    for (ds, g, op, lay) in sorted(meas):
        if op != "Insert" or lay not in ("CSR", "BCSR"):
            continue
        path = edge_file(g, ds)
        if path is None:
            continue
        n, m = bf.count_graph_file(path)
        cm._CUR_GRAPH = g
        cf, bf_ = cm._shift_fracs_for_n_m(n, m)
        tiers = tiers_for(g, ds, lay)
        if lay == "CSR":
            hyb = hybrid_insert_csr(n, m, cf, tiers, rates_hyb)
        else:
            hyb = hybrid_insert_bcsr(n, m, bf_, tiers, rates_hyb)
        vals = {}
        for mode in ("legacy", "aware", "class_tier"):
            cm.set_cache_model(mode)
            per = (cm.insert_cost_csr(n, m) if lay == "CSR"
                   else cm.insert_cost_bcsr(n, m))
            vals[mode] = 50.0 * per
        m_ns = meas[(ds, g, op, lay)]
        results.append((g, lay, vals["legacy"], vals["aware"], 50.0 * hyb,
                        vals["class_tier"], m_ns))

    print(f"{'graph':26s} {'lay':4s} {'legacy':>12s} {'aware':>12s} "
          f"{'hybrid':>12s} {'meas':>12s} | {'r_leg':>5s} {'r_awa':>5s} "
          f"{'r_hyb':>5s}")
    acc = {"legacy": {"CSR": [], "BCSR": []}, "aware": {"CSR": [], "BCSR": []},
           "hybrid": {"CSR": [], "BCSR": []}}
    for g, lay, l, a, h, c, m in results:
        for key, v in (("legacy", l), ("aware", a), ("hybrid", h)):
            acc[key][lay].append(v / m)
        print(f"{g:26s} {lay:4s} {l:12.0f} {a:12.0f} {h:12.0f} {m:12.0f} | "
              f"{l/m:5.2f} {a/m:5.2f} {h/m:5.2f}")

    print("\n=== Absolute accuracy (median |ratio-1|) ===")
    for lay in ("CSR", "BCSR"):
        for key in ("legacy", "aware", "hybrid"):
            r = acc[key][lay]
            print(f"  {lay}: {key:8s} = {np.median([abs(x-1) for x in r]):.3f}  (n={len(r)})")

    # ── Full-order rankings (Insert + Traverse) ──────────────────────────
    print("\n=== Full-order rankings (legacy / aware / hybrid vs measured) ===")
    cm.set_cache_model("legacy")
    for ds, csvn, _ in CSVS:
        rows = list(csv.DictReader(open(csvn)))
        graphs = sorted({r["graph"] for r in rows})
        ops = sorted({r["operation"] for r in rows})
        meas_ds = {(r["graph"], r["operation"], r["layout"]):
                   float(r["measured_kernel_ns"]) for r in rows
                   if r.get("measured_kernel_ns")}
        for g in graphs:
            path = edge_file(g, ds)
            if path is None:
                continue
            n, m = bf.count_graph_file(path)
            cm._CUR_GRAPH = g
            for op in ops:
                if not all((g, op, l) in meas_ds for l in LAYOUTS):
                    continue
                mcosts = {l: meas_ds[(g, op, l)] for l in LAYOUTS}
                preds = {}
                for mode in ("legacy", "aware"):
                    cm.set_cache_model(mode)
                    preds[mode] = {}
                    for l in LAYOUTS:
                        per = cm.predicted_ns(l, op, n, m)
                        preds[mode][l] = per * (2.0 if op == "Traverse" else 50.0)
                hyb = {}
                for l in LAYOUTS:
                    if op == "Insert" and l in ("CSR", "BCSR"):
                        tiers = tiers_for(g, ds, l)
                        if l == "CSR":
                            cf, _ = cm._shift_fracs_for_n_m(n, m)
                            hyb[l] = 50.0 * hybrid_insert_csr(n, m, cf, tiers,
                                                              rates_hyb)
                        else:
                            _, bf_ = cm._shift_fracs_for_n_m(n, m)
                            hyb[l] = 50.0 * hybrid_insert_bcsr(n, m, bf_, tiers,
                                                               rates_hyb)
                    else:
                        hyb[l] = preds["legacy"][l]
                mr = rank_of(mcosts)
                lr = rank_of(preds["legacy"])
                ar = rank_of(preds["aware"])
                hr = rank_of(hyb)
                lseq = [l for l in sorted(lr, key=lr.get)]
                hseq = [l for l in sorted(hr, key=hr.get)]
                mseq = [l for l in sorted(mr, key=mr.get)]
                l_ok, a_ok, h_ok = lr == mr, ar == mr, hr == mr
                if not (l_ok and a_ok and h_ok):
                    tag = ""
                    if not l_ok and h_ok:
                        tag = " <-- HYBRID FIXED"
                    elif l_ok and not h_ok:
                        tag = " <-- HYBRID REGRESSED"
                    elif l_ok and a_ok and not h_ok:
                        tag = " <-- HYBRID REGRESSED"
                    print(f"{g:26s} {op:8s} meas={mseq} legacy={lseq} "
                          f"hybrid={hseq}{tag}")

    cm.set_cache_model("aware")
    print("\nNote: PCSR/SET/traverse costs are mode-independent; the hybrid "
          "changes only CSR/BCSR insert costs.")


if __name__ == "__main__":
    main()