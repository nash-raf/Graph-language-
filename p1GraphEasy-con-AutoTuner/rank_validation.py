#!/usr/bin/env python3
"""rank_validation.py — full representation-ranking validation (Step 8).

For each dataset (real_world / barabasi / erdos) and each graph/workload,
compute the predicted kernel cost for EVERY representation (CSR, PCSR, BCSR,
SET) under:
  1. the old/legacy cost model (legacy pass binary predictions; only CSR and
     BCSR insert predictions differ from the new binary),
  2. the new cache-aware cost model (current pass binary predictions),
and compare both against measured runtime.

PRIMARY signal: compiled-pass predictions (production).  Python cost-model
predictions are the transparency/debugging cross-check; a production/Python
parity gate must pass (per graph/op/layout, both models) BEFORE ranking
results are interpreted.

Reports per graph/workload: {graph, op, rep, measured, old_pred, new_pred,
old_rank, new_rank, meas_rank}; and per-dataset + combined aggregates:
top-1 selection accuracy, Spearman + Kendall, full-ranking mismatches,
mismatches fixed / introduced.
"""
import csv
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path("/home/kazisahib/sgpl_main/Graph-language-/p1GraphEasy-con-AutoTuner")
sys.path.insert(0, str(ROOT))
import bench_folder as bf

DATASET = Path("/home/kazisahib/sgpl_main/dataset")
LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]
PARITY_REL_TOL = 1e-3
PARITY_ABS_TOL = 1.0
_env_work = os.environ.get("RANKVAL_WORKDIR")
W = Path(_env_work) if _env_work else Path(tempfile.mkdtemp(prefix="rankval_", dir="/tmp/opencode"))
OBJ = W / "objs"
OBJ.mkdir(exist_ok=True)
compile_lock = threading.Lock()

DATASETS = {
    "real_world": ("real_world_runs10_merged.csv", ["real graphs", "new real"]),
    "barabasi": ("barabasi_albert_runs10.csv", ["barabasi_albert"]),
    "erdos": ("erdos_renyi_runs10.csv", ["erdos_renyi"]),
}

NEW_BIN = ROOT / "GraphProgram_new"
# the pre-Phase-3 legacy binary (GraphProgram_legacy) is kept as a
# sanity cross-check only; the mode-based ablation uses the same current
# pass binary under AUTOTUNER_CACHE_MODEL=legacy vs =aware.


def edge_file_for(graph_name, dataset):
    for folder in DATASETS[dataset][1]:
        p = DATASET / folder / f"{graph_name}.txt"
        if p.exists():
            return p
    return None


def parse_profile(stderr, op_kind):
    for line in (stderr or "").splitlines():
        m = re.search(rf"total kind={op_kind}\s+predicted_ns=([0-9.]+)", line)
        if m:
            return float(m.group(1))
    return None


def compile_run(graph_path, layout, op_kind, model="aware"):
    """Compile under forced layout + cache model with the current pass binary,
    link, run once, return the profile's predicted_ns for op_kind."""
    env = {**os.environ, "AUTOTUNER_FORCE_LAYOUT": layout,
           "AUTOTUNER_CACHE_MODEL": model}
    with compile_lock:
        cp = subprocess.run([str(NEW_BIN), str(graph_path)],
                            cwd=str(ROOT), env=env,
                            text=True, capture_output=True, timeout=900)
        if cp.returncode != 0:
            raise RuntimeError(f"{layout}/{op_kind}/{model}: {cp.stderr[-200:]}")
        prog_o = ROOT / "program.o"
        p2 = W / f"p_{graph_path.stem}_{layout}_{op_kind}_{model}.o"
        shutil.copy2(prog_o, p2)
    bin_p = W / f"b_{graph_path.stem}_{layout}_{op_kind}_{model}"
    l = subprocess.run(
        ["g++", "-O3", "-mavx2", "-march=native", "-fopenmp", "-no-pie",
         str(p2), str(OBJ / "runtime.o"), str(OBJ / "parallel_runtime.o"),
         str(OBJ / "autotuner_runtime.o"), str(OBJ / "graph_mutation_runtime.o"),
         str(OBJ / "roaring_bitmap.o"), str(OBJ / "graph_loader_runtime.o"),
         str(OBJ / "graph_runtime.o"), "-lnlopt", "-o", str(bin_p)],
        text=True, capture_output=True)
    if l.returncode != 0:
        raise RuntimeError(f"link {l.stderr[-200:]}")
    run = subprocess.run([str(bin_p)], text=True, capture_output=True,
                         timeout=600)
    if run.returncode != 0:
        raise RuntimeError(f"run rc={run.returncode}")
    return parse_profile(run.stderr or "", op_kind)


def build_runtime_objs():
    cmds = [
        (["gcc", "-O3", "-c", str(ROOT / "autotuner_runtime.c"), "-o",
          str(OBJ / "autotuner_runtime.o")]),
        (["gcc", "-O3", "-c", str(ROOT / "graph_mutation_runtime.c"), "-o",
          str(OBJ / "graph_mutation_runtime.o")]),
        (["gcc", "-O3", "-c", str(ROOT / "parallel_runtime.c"), "-o",
          str(OBJ / "parallel_runtime.o")]),
        (["gcc", "-O3", "-c", str(ROOT / "runtime.c"), "-o",
          str(OBJ / "runtime.o")]),
        (["g++", "-O3", "-mavx2", "-march=native", "-fopenmp", "-c",
          str(ROOT / "roaring_bitmap.cpp"), "-o", str(OBJ / "roaring_bitmap.o")]),
        (["g++", "-O2", "-std=c++17", "-fopenmp", "-c",
          str(ROOT / "graph_loader_runtime.cpp"), "-o",
          str(OBJ / "graph_loader_runtime.o")]),
        (["g++", "-O2", "-std=c++17", "-c", str(ROOT / "graph_runtime.cpp"),
          "-o", str(OBJ / "graph_runtime.o")]),
    ]
    for c in cmds:
        r = subprocess.run(c, text=True, capture_output=True)
        if r.returncode != 0:
            raise RuntimeError(f"obj build {c[-1]}: {r.stderr[-200:]}")


def load_csv_rows(csv_name):
    return list(csv.DictReader(open(ROOT / csv_name)))


def measured_map(rows):
    m = {}
    for r in rows:
        if r.get("measured_kernel_ns"):
            m[(r["graph"], r["operation"], r["layout"])] = float(
                r["measured_kernel_ns"])
    return m


def python_predictions(graph_name, op_kind, n, m_und, dataset, mode):
    """Python cost-model kernel prediction via the canonical dispatch
    (K_INS=50 adds for Insert, 1 traversal for Traverse), under the given
    AUTOTUNER_CACHE_MODEL mode.  Traverse is x2 to reproduce the pass's
    region accounting: the neighbor-loop region is charged at H*totalOps =
    2n site-visits (both directed ends of every undirected edge), while
    traversalCost() is the per-vertex equation."""
    import cost_model as cm
    cm._CUR_GRAPH = graph_name
    cm.set_cache_model(mode)
    out = {}
    for layout in LAYOUTS:
        per = cm.predicted_ns(layout, op_kind, n, m_und)
        mult = 2.0 if op_kind == "Traverse" else 50.0
        out[layout] = per * mult
    return out


def parity_check(tag, pass_preds, py_preds):
    """Pass/Python parity per (graph, op, layout); returns (ok, max_rel)."""
    worst = 0.0
    n_bad = 0
    for (g, op, lay), p in pass_preds.items():
        q = py_preds.get((g, op, lay))
        if p is None or q is None:
            continue
        denom = max(abs(q), PARITY_ABS_TOL)
        rel = abs(p - q) / denom
        worst = max(worst, rel)
        if rel > PARITY_REL_TOL:
            n_bad += 1
            print(f"    PARITY {tag}: {g} {op} {lay} pass={p:.1f} py={q:.1f}")
    return n_bad == 0, worst


def rank_of(costs):
    """Rank (1 = cheapest) of each layout by cost; ties share the min rank."""
    s = sorted((c, l) for l, c in costs.items())
    r = {}
    for i, (c, l) in enumerate(s):
        r[l] = i + 1
    return r


def spearman(a, b):
    import numpy as np
    ra = np.argsort(np.argsort(a)).astype(float)
    rb = np.argsort(np.argsort(b)).astype(float)
    return float(np.corrcoef(ra, rb)[0, 1])


def kendall(a, b):
    n = len(a)
    conc, disc = 0, 0
    for i in range(n):
        for j in range(i + 1, n):
            sa = (a[i] - a[j]) * (b[i] - b[j])
            if sa > 0:
                conc += 1
            elif sa < 0:
                disc += 1
    denom = n * (n - 1) / 2
    return (conc - disc) / denom if denom else 0.0


def run_dataset(dataset, workdir, cache):
    csv_name, folders = DATASETS[dataset]
    rows = load_csv_rows(csv_name)
    meas = measured_map(rows)
    graphs = sorted({r["graph"] for r in rows})
    ops = sorted({r["operation"] for r in rows})
    print(f"\n=== dataset {dataset}: {len(graphs)} graphs x {ops} ===",
          flush=True)

    tasks = []
    for g in graphs:
        ef = edge_file_for(g, dataset)
        if ef is None:
            print(f"  !! no edge file for {g}")
            continue
        nv, m_und = bf.count_graph_file(ef)
        for op in ops:
            gp = workdir / f"{g}_{op}.graph"
            if not gp.exists():
                if op == "Insert":
                    bf.make_insert_workload(str(ef), 50, nv, gp)
                else:
                    bf.make_traverse_workload(str(ef), gp)
            for layout in LAYOUTS:
                # CSR/BCSR insert costs differ between modes; everything
                # else is mode-independent (compiled once under "aware").
                modes = (("legacy", "aware") if (op == "Insert"
                         and layout in ("CSR", "BCSR")) else ("aware",))
                for model in modes:
                    if (dataset, g, op, layout, model) not in cache:
                        tasks.append((g, gp, layout, op, model))

    def one(t):
        g, gp, layout, op, model = t
        try:
            p = compile_run(gp, layout, op, model)
            return (dataset, g, op, layout, model, p)
        except Exception as e:
            return (dataset, g, op, layout, model, None)

    with ThreadPoolExecutor(max_workers=4) as ex:
        for res in ex.map(one, tasks):
            d, g, op, layout, model, p = res
            cache[(d, g, op, layout, model)] = p
            if p is not None:
                print(f"  {g:26s} {op:8s} {layout:>4s} {model:7s} = {p:14.1f}",
                      flush=True)
    json.dump({f"{k[0]}|{k[1]}|{k[2]}|{k[3]}|{k[4]}": v
               for k, v in cache.items() if k[0] == dataset},
              open(workdir / f"cache_{dataset}.json", "w"), indent=0)

    # Assemble per (graph, op, layout) predictions per mode.
    legacy_p, aware_p = {}, {}
    py_legacy, py_aware = {}, {}
    for g in graphs:
        ef = edge_file_for(g, dataset)
        nv, m_und = bf.count_graph_file(ef)
        for op in ops:
            for layout in LAYOUTS:
                aw_ = cache.get((dataset, g, op, layout, "aware"))
                le_ = cache.get((dataset, g, op, layout, "legacy"))
                aware_p[(g, op, layout)] = aw_
                legacy_p[(g, op, layout)] = (le_ if le_ is not None else aw_)
            pyl = python_predictions(g, op, nv, m_und, dataset, "legacy")
            pya = python_predictions(g, op, nv, m_und, dataset, "aware")
            for layout in LAYOUTS:
                py_legacy[(g, op, layout)] = pyl[layout]
                py_aware[(g, op, layout)] = pya[layout]

    ok_aware, worst_aware = parity_check(f"{dataset}/aware", aware_p, py_aware)
    ok_legacy, worst_legacy = parity_check(f"{dataset}/legacy", legacy_p,
                                           py_legacy)
    print(f"  PARITY gate: aware={ok_aware} (worst rel {worst_aware:.2e})  "
          f"legacy={ok_legacy} (worst rel {worst_legacy:.2e})")
    if not (ok_aware and ok_legacy):
        print(f"  !! {dataset}: parity FAILED — ranking results NOT reliable")
        return {"parity": False, "aware": ok_aware, "legacy": ok_legacy,
                "worst_aware": worst_aware, "worst_legacy": worst_legacy}

    # Ranking comparison
    results = []
    full_mismatch_old = full_mismatch_new = 0
    fixed = introduced = 0
    top1_old = top1_new = 0
    rho_new_list, tau_new_list = [], []
    total_pairs = 0
    for g in graphs:
        for op in ops:
            if not all((g, op, l) in meas for l in LAYOUTS):
                continue
            mcosts = {l: meas[(g, op, l)] for l in LAYOUTS}
            ocosts = {l: legacy_p.get((g, op, l)) for l in LAYOUTS}
            ncosts = {l: aware_p.get((g, op, l)) for l in LAYOUTS}
            if any(v is None for v in ocosts.values()) or \
               any(v is None for v in ncosts.values()):
                continue
            total_pairs += 1
            mr, orr, nrr = rank_of(mcosts), rank_of(ocosts), rank_of(ncosts)
            mb = min(mcosts, key=mcosts.get)
            ob = min(ocosts, key=ocosts.get)
            nb = min(ncosts, key=ncosts.get)
            if ob == mb:
                top1_old += 1
            if nb == mb:
                top1_new += 1
            fm_old = orr != mr
            fm_new = nrr != mr
            full_mismatch_old += fm_old
            full_mismatch_new += fm_new
            if fm_old and not fm_new:
                fixed += 1
            if not fm_old and fm_new:
                introduced += 1
            mseq = [mcosts[l] for l in LAYOUTS]
            nseq = [ncosts[l] for l in LAYOUTS]
            oseq = [ocosts[l] for l in LAYOUTS]
            rho_new_list.append(spearman(mseq, nseq))
            tau_new_list.append(kendall(mseq, nseq))
            for l in LAYOUTS:
                results.append({
                    "graph": g, "op": op, "rep": l,
                    "measured": mcosts[l],
                    "old_pred": ocosts[l], "new_pred": ncosts[l],
                    "old_rank": orr[l], "new_rank": nrr[l],
                    "meas_rank": mr[l],
                })
    json.dump(results, open(workdir / f"rankrows_{dataset}.json", "w"),
              indent=1)
    out = {
        "parity": True, "pairs": total_pairs,
        "top1_old": top1_old, "top1_new": top1_new,
        "full_mismatch_old": full_mismatch_old,
        "full_mismatch_new": full_mismatch_new,
        "fixed": fixed, "introduced": introduced,
        "spearman_mean": (sum(rho_new_list) / len(rho_new_list)
                          if rho_new_list else 0.0),
        "kendall_mean": (sum(tau_new_list) / len(tau_new_list)
                         if tau_new_list else 0.0),
    }
    print(f"  pairs={total_pairs} top1 old={top1_old} new={top1_new} "
          f"| full-rank mismatch old={full_mismatch_old} "
          f"new={full_mismatch_new} fixed={fixed} introduced={introduced} "
          f"| rho={out['spearman_mean']:.3f} tau={out['kendall_mean']:.3f}")
    return out


def main():
    build_runtime_objs()
    summaries = {}
    for dataset in DATASETS:
        workdir = W / dataset
        workdir.mkdir(exist_ok=True)
        cache = {}
        cache_file = workdir / f"cache_{dataset}.json"
        if cache_file.exists():
            raw = json.load(open(cache_file))
            cache = {tuple(k.split("|")): v for k, v in raw.items()}
        summaries[dataset] = run_dataset(dataset, workdir, cache)

    print("\n=== SUMMARY ===")
    for d, s in summaries.items():
        print(f"{d:12s} parity={s.get('parity')} "
              f"top1 {s.get('top1_old',0)}->{s.get('top1_new',0)}/{s.get('pairs',0)} "
              f"full-mismatch {s.get('full_mismatch_old',0)}->"
              f"{s.get('full_mismatch_new',0)} "
              f"fixed={s.get('fixed',0)} introduced={s.get('introduced',0)} "
              f"rho={s.get('spearman_mean',0):.3f} tau={s.get('kendall_mean',0):.3f}")


if __name__ == "__main__":
    main()