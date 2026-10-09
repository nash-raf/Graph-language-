#!/usr/bin/env python3
"""Re-measure real-world graph kernel times (Insert + Traverse, all 4
layouts) with graph_ensure_owned_storage EXCLUDED from the timed kernel.

The one-time ownership transfer (the mmap -> malloc edge-pair copy inside
graph_add_edge / graph_add_node / graph_remove_edge / graph_remove_node)
is moved BEFORE the kernel timer, so it never enters measured kernel time.

Graphs: every *.txt / *.edges file under
    /home/kazisahib/sgpl_main/dataset/real graphs/
    /home/kazisahib/sgpl_main/dataset/new real/
(.mtx Matrix-Market files are skipped: the loader's `u v` parser cannot
read them.)

Output CSV (same schema as real_world_runs10_merged.csv):
    graph,n_vertices,m_undirected,operation,layout,predicted_ns,
    measured_kernel_ns,measured_iqr_ns,kernel_cv,
    predicted_best,measured_best,verdict
predicted_ns comes from cost_model.py (hybrid model; Insert = 50 x per-add,
Traverse = 2 x per-vertex equation), so ratio plots / ranking checks work
immediately.

Run:
    python3 measure_real_world.py --runs 10 --n-inserts 50
"""
import argparse
import csv
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

import numpy as np

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))
import bench_folder as bf
import cost_model as cm

REAL_GRAPHS_DIR = Path("/home/kazisahib/sgpl_main/dataset/real graphs")
NEW_REAL_DIR = Path("/home/kazisahib/sgpl_main/dataset/new real")
PASS_BIN = "./GraphProgram_new"           # current pass binary (hybrid)
LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]

GMR = SCRIPT_DIR / "graph_mutation_runtime.c"
GMR_BAK = SCRIPT_DIR / "graph_mutation_runtime.c.bak_untimed"

# ── Runtime patch: move graph_ensure_owned_storage above the kernel timer ──
# The ownership transfer is a one-time load-time setup cost (copies the
# mmap'd canonical edge-pair backing, 8m bytes); it must not enter the
# measured kernel time of any mutation region.
PATCHES = [
    # graph_add_node
    ("""void graph_add_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                    int32_t node_id) {
  uint64_t start_ns = mutation_now_ns();
  (void)nodes_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }
  graph_ensure_owned_storage(g);""",
     """void graph_add_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                    int32_t node_id) {
  (void)nodes_bmp;
  Graph *g = (Graph *)graph_ptr;
  /* One-time ownership transfer runs BEFORE the kernel timer (load-time
   * setup, not kernel physics — must not enter measured kernel time). */
  if (g)
    graph_ensure_owned_storage(g);
  uint64_t start_ns = mutation_now_ns();
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }"""),
    # graph_remove_node
    ("""void graph_remove_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                       int32_t node_id) {
  uint64_t start_ns = mutation_now_ns();
  (void)nodes_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }
  graph_ensure_owned_storage(g);""",
     """void graph_remove_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                       int32_t node_id) {
  (void)nodes_bmp;
  Graph *g = (Graph *)graph_ptr;
  /* One-time ownership transfer runs BEFORE the kernel timer (load-time
   * setup, not kernel physics — must not enter measured kernel time). */
  if (g)
    graph_ensure_owned_storage(g);
  uint64_t start_ns = mutation_now_ns();
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }"""),
    # graph_add_edge
    ("""void graph_add_edge(void *graph_ptr, void *edges_bmp, int32_t from, int32_t to,
                    int32_t edge_id) {
  uint64_t start_ns = mutation_now_ns();
  (void)edge_id;
  (void)edges_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }

  /* Layout must be read before ensure_owned: after a convert-to-SET the Graph
   * CSR pointers can be stale, and memcpy in ensure_owned would SIGSEGV. */
  int32_t layout = autograph_get_layout(graph_ptr);
  if (layout == LAYOUT_SET) {
    autograph_canonical_add_edge(graph_ptr, from, to);
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }

  graph_ensure_owned_storage(g);""",
     """void graph_add_edge(void *graph_ptr, void *edges_bmp, int32_t from, int32_t to,
                    int32_t edge_id) {
  (void)edge_id;
  (void)edges_bmp;
  Graph *g = (Graph *)graph_ptr;

  /* Layout must be read before ensure_owned: after a convert-to-SET the Graph
   * CSR pointers can be stale, and memcpy in ensure_owned would SIGSEGV. */
  int32_t layout = g ? autograph_get_layout(graph_ptr) : LAYOUT_SET;
  /* One-time ownership transfer (graph_ensure_owned_storage copies the
   * mmap'd canonical edge-pair backing) runs BEFORE the kernel timer —
   * load-time setup, not insert physics; it must not enter measured
   * kernel time. */
  if (g)
    graph_ensure_owned_storage(g);
  uint64_t start_ns = mutation_now_ns();
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }

  if (layout == LAYOUT_SET) {
    autograph_canonical_add_edge(graph_ptr, from, to);
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }"""),
    # graph_remove_edge
    ("""void graph_remove_edge(void *graph_ptr, void *edges_bmp, int32_t from,
                       int32_t to, int32_t edge_id) {
  uint64_t start_ns = mutation_now_ns();
  (void)edge_id;
  (void)edges_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }
  graph_ensure_owned_storage(g);""",
     """void graph_remove_edge(void *graph_ptr, void *edges_bmp, int32_t from,
                       int32_t to, int32_t edge_id) {
  (void)edge_id;
  (void)edges_bmp;
  Graph *g = (Graph *)graph_ptr;
  /* One-time ownership transfer runs BEFORE the kernel timer (load-time
   * setup, not kernel physics — must not enter measured kernel time). */
  if (g)
    graph_ensure_owned_storage(g);
  uint64_t start_ns = mutation_now_ns();
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }"""),
]


def patch_runtime_untimed():
    """Move graph_ensure_owned_storage above the kernel timer in all four
    mutation entry points.  Idempotent; original kept in GMR_BAK."""
    if GMR_BAK.exists() and "BEFORE the kernel timer" in GMR.read_text():
        print("[patch] graph_mutation_runtime.c already untimed (skipped)")
        return True
    if not GMR_BAK.exists():
        shutil.copy2(GMR, GMR_BAK)
    s = GMR.read_text()
    for old, new in PATCHES:
        cnt = s.count(old)
        if cnt != 1:
            print(f"[patch] FAILED: expected exactly 1 occurrence of a "
                  f"patch site, found {cnt} — aborting (original in {GMR_BAK})",
                  file=sys.stderr)
            shutil.copy2(GMR_BAK, GMR)
            return False
        s = s.replace(old, new)
    GMR.write_text(s)
    print("[patch] graph_mutation_runtime.c: ensure_owned moved above the "
          "kernel timer in all 4 mutation entry points "
          f"(original kept at {GMR_BAK.name})")
    return True


def compile_and_run(graph_path, layout, workdir, timeout, n_runs=10):
    """Compile the .graph with the hybrid pass (forced layout), link against
    the PATCHED runtime objects, run 1 warmup + n_runs measured, return
    per-op kernel_ns lists.  Mirrors bench_folder.compile_and_run."""
    env = {"AUTOTUNER_FORCE_LAYOUT": layout}
    cp = subprocess.run([PASS_BIN, str(graph_path)], env=env,
                        capture_output=True, text=True, cwd=str(SCRIPT_DIR))
    if cp.returncode != 0:
        return {"error": f"pass failed: {(cp.stderr or '')[-300:]}"}
    prog_o = SCRIPT_DIR / "program.o"
    if not prog_o.exists():
        return {"error": "no program.o"}

    obj_dir = workdir / f"objs_{layout}"
    obj_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(prog_o, obj_dir / "program.o")

    def _compile(cmd):
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0:
            return r.stderr[-500:] if r.stderr else f"rc={r.returncode}"
        return None

    comps = [
        ["gcc", "-O3", "-c", str(SCRIPT_DIR / "autotuner_runtime.c"),
         "-o", str(obj_dir / "autotuner_runtime.o")],
        ["gcc", "-O3", "-c", str(SCRIPT_DIR / "graph_mutation_runtime.c"),
         "-o", str(obj_dir / "graph_mutation_runtime.o")],
        ["gcc", "-O3", "-c", str(SCRIPT_DIR / "parallel_runtime.c"),
         "-o", str(obj_dir / "parallel_runtime.o")],
        ["gcc", "-O3", "-c", str(SCRIPT_DIR / "runtime.c"),
         "-o", str(obj_dir / "runtime.o")],
        ["g++", "-O3", "-mavx2", "-march=native", "-fopenmp",
         "-c", str(SCRIPT_DIR / "roaring_bitmap.cpp"),
         "-o", str(obj_dir / "roaring_bitmap.o")],
        ["g++", "-O2", "-std=c++17", "-fopenmp",
         "-c", str(SCRIPT_DIR / "graph_loader_runtime.cpp"),
         "-o", str(obj_dir / "graph_loader_runtime.o")],
        ["g++", "-O2", "-std=c++17",
         "-c", str(SCRIPT_DIR / "graph_runtime.cpp"),
         "-o", str(obj_dir / "graph_runtime.o")],
    ]
    for cmd in comps:
        err = _compile(cmd)
        if err:
            return {"error": f"compile failed ({cmd[-3]}): {err}"}

    bin_path = workdir / f"final_{layout}"
    objs = [obj_dir / "program.o", obj_dir / "runtime.o",
            obj_dir / "parallel_runtime.o", obj_dir / "autotuner_runtime.o",
            obj_dir / "graph_mutation_runtime.o", obj_dir / "roaring_bitmap.o",
            obj_dir / "graph_loader_runtime.o", obj_dir / "graph_runtime.o"]
    link = subprocess.run(
        ["g++", "-O3", "-mavx2", "-march=native", "-fopenmp", "-no-pie",
         *[str(o) for o in objs], "-lnlopt", "-o", str(bin_path)],
        capture_output=True, text=True)
    if link.returncode != 0:
        return {"error": f"link failed: {(link.stderr or '')[-500:]}"}

    all_runs = []
    for i in range(n_runs + 1):
        try:
            cp = subprocess.run([str(bin_path)], capture_output=True,
                                text=True, timeout=timeout)
            if cp.returncode != 0:
                if i == 0:
                    return {"error": f"binary crashed (rc={cp.returncode})"}
                continue
            kernels = bf.parse_kernels(cp.stderr or "")
            if i > 0:
                all_runs.append(kernels)
        except subprocess.TimeoutExpired:
            if i == 0:
                return {"timeout": True}
            continue

    if not all_runs:
        return {"timeout": True}

    result = {}
    for op_kind in ["Insert", "Traverse", "Query"]:
        values = [r[op_kind]["kernel_ns"] for r in all_runs
                  if r.get(op_kind, {}).get("kernel_ns") is not None]
        if values:
            result[op_kind] = values
    return result


def find_graphs():
    """All *.txt / *.edges files under both dataset folders (skip .mtx)."""
    found = []
    for folder in (REAL_GRAPHS_DIR, NEW_REAL_DIR):
        for p in sorted(folder.glob("*")):
            if p.suffix in (".txt", ".edges"):
                found.append(p)
    return found


def ffloat(s):
    s = str(s).strip()
    if not s or s.lower() == "none":
        return None
    return float(s)


def verdict(preds, meas_mean, meas_iqr):
    valid = [(l, preds[l], meas_mean[l]) for l in LAYOUTS
             if meas_mean.get(l) is not None]
    if not valid:
        return "NODATA", "-", "-"
    bp = min(valid, key=lambda x: x[1])[0]
    bm = min(valid, key=lambda x: x[2])[0]
    ps = sorted(p for _, p, _ in valid)
    pred_tie = (ps[1] - ps[0]) <= 0.05 * max(ps[0], 1.0)
    ms = sorted((m_, l_) for l_, _, m_ in valid)
    a_iqr = meas_iqr.get(ms[0][1], 0) or 0
    b_iqr = meas_iqr.get(ms[1][1], 0) or 0
    meas_tie = abs(ms[0][0] - ms[1][0]) < max(a_iqr, b_iqr)
    if pred_tie and meas_tie:
        return "TIE", bp, bm
    if bp == bm:
        return "MATCH", bp, bm
    if meas_tie:
        return "NOISE", bp, bm
    return "MISMATCH", bp, bm


def main():
    ap = argparse.ArgumentParser(
        description="Re-measure real-world graphs with ensure_owned untimed")
    ap.add_argument("--runs", type=int, default=10,
                    help="measured runs per layout (default 10; first is warmup)")
    ap.add_argument("--n-inserts", type=int, default=50,
                    help="inserts per insert workload")
    ap.add_argument("--timeout", type=int, default=180,
                    help="per-run timeout (seconds)")
    ap.add_argument("--out", default="real_world_remeasured_untimed.csv")
    ap.add_argument("--workdir", default="/tmp/opencode/remeasure")
    args = ap.parse_args()

    os.chdir(SCRIPT_DIR)
    if not Path(PASS_BIN[2:]).exists():
        print(f"Error: {PASS_BIN} not found — build the pass first", file=sys.stderr)
        sys.exit(1)
    if not patch_runtime_untimed():
        sys.exit(1)

    cm.set_cache_model("hybrid")
    graphs = find_graphs()
    if not graphs:
        print("No .txt/.edges graphs found under the dataset folders",
              file=sys.stderr)
        sys.exit(1)
    print(f"Graphs to measure ({len(graphs)}):")
    for p in graphs:
        print(f"  {p.parent.name}/{p.name}")

    workdir = Path(args.workdir)
    workdir.mkdir(parents=True, exist_ok=True)
    rows = []
    for ef in graphs:
        label = ef.stem
        nv, m_und = bf.count_graph_file(str(ef))
        print(f"\n{'─' * 55}\n  {label}: n={nv} m_und={m_und}\n{'─' * 55}",
              flush=True)
        gdir = workdir / label
        gdir.mkdir(parents=True, exist_ok=True)

        traverse_graph = gdir / f"{label}_traverse.graph"
        insert_graph = gdir / f"{label}_insert.graph"
        bf.make_traverse_workload(str(ef), traverse_graph)
        try:
            n_fresh = bf.make_insert_workload(str(ef), args.n_inserts, nv,
                                              insert_graph)
            print(f"  insert workload: {n_fresh} guaranteed-fresh edges",
                  flush=True)
        except RuntimeError as e:
            print(f"  SKIP insert+traverse ({e})", flush=True)
            continue

        raw = {}  # op -> layout -> [kernel_ns...]
        for layout in LAYOUTS:
            for op, graph_path in (("Traverse", traverse_graph),
                                   ("Insert", insert_graph)):
                sys.stdout.write(f"  {layout:>6s} {op:>8s} ... ")
                sys.stdout.flush()
                t0 = time.time()
                result = compile_and_run(graph_path, layout, gdir,
                                         args.timeout, args.runs)
                dt = time.time() - t0
                raw.setdefault(op, {})
                if result.get("timeout"):
                    print(f"TIMEOUT ({dt:.0f}s)")
                elif result.get("error"):
                    print(f"FAIL ({result['error'][:50]})")
                else:
                    vals = result.get(op, [])
                    raw[op][layout] = vals
                    if vals:
                        print(f"OK mean={np.mean(vals):.0f}ns "
                              f"({len(vals)} runs)")
                    else:
                        print("OK (no values)")

        for op_kind in ["Insert", "Traverse"]:
            meas_mean = {l: (np.mean(raw[op_kind][l]) if raw[op_kind].get(l)
                             else None) for l in LAYOUTS}
            meas_iqr = {}
            for l in LAYOUTS:
                v = raw[op_kind].get(l, [])
                meas_iqr[l] = (float(np.percentile(v, 75) -
                                     np.percentile(v, 25)) if v else None)

            preds = {}
            for l in LAYOUTS:
                cm._CUR_GRAPH = label
                per = cm.predicted_ns(l, op_kind, nv, m_und)
                mult = 2.0 if op_kind == "Traverse" else float(args.n_inserts)
                preds[l] = per * mult

            v, bp, bm = verdict(preds, meas_mean, meas_iqr)
            for l in LAYOUTS:
                med = meas_mean[l]
                iq = meas_iqr[l]
                rows.append({
                    "graph": label,
                    "n_vertices": nv,
                    "m_undirected": m_und,
                    "operation": op_kind,
                    "layout": l,
                    "predicted_ns": f"{preds[l]:.1f}",
                    "measured_kernel_ns": (f"{med:.0f}" if med is not None
                                           else ""),
                    "measured_iqr_ns": (f"{iq:.0f}" if iq is not None else ""),
                    "kernel_cv": (f"{(iq / 1.349) / med:.3f}"
                                  if med and med > 0 and iq else ""),
                    "predicted_best": bp,
                    "measured_best": bm,
                    "verdict": v if l == LAYOUTS[0] else "",
                })

    fields = ["graph", "n_vertices", "m_undirected", "operation", "layout",
              "predicted_ns", "measured_kernel_ns", "measured_iqr_ns",
              "kernel_cv", "predicted_best", "measured_best", "verdict"]
    out = Path(args.out)
    with open(out, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)
    print(f"\nwrote {out} ({len(rows)} rows)")
    print("NOTE: graph_ensure_owned_storage is untimed (patch applied to "
          "graph_mutation_runtime.c; original at "
          "graph_mutation_runtime.c.bak_untimed)")


if __name__ == "__main__":
    main()