#!/usr/bin/env python3
"""
bench_folder.py — Run predicted-vs-measured comparison across graph edge
files, for all four layouts (CSR, PCSR, BCSR, SET).

Outputs a CSV with per-graph, per-layout, per-operation predicted and measured
kernel times, plus a MATCH/MISMATCH verdict.

When the given folder contains .txt files directly, it processes them as a
single group and writes one CSV.  When it contains subdirectories instead,
each subdirectory is processed independently and one CSV per subdirectory is
written to the current working directory.

Usage:
  ./bench_folder.py <edge_folder> [--out results.csv] [--n-inserts 50] [--runs 10]
  ./bench_folder.py dataset/   # processes each subfolder → erdos_renyi.csv etc.

Environment:
  SKIP_BUILD=1     reuse existing GraphProgram
  TIMEOUT_SEC=     per-run timeout (default 180)
  HW_CALIB_JSON=   path to hw_calib.json (for prediction model)
"""

import argparse
import csv
import json
import math
import os
import random
import re
import resource
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

# Set unlimited stack to prevent segfault on large graphs (BFS recursion depth)
resource.setrlimit(resource.RLIMIT_STACK, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))

# ---------------------------------------------------------------------------
SCRIPT_DIR = Path(__file__).resolve().parent

CLANGXX_BIN = os.environ.get("CLANGXX_BIN", "clang++-20")
CLANG_BIN   = os.environ.get("CLANG_BIN",   "clang-20")
LLVM_CONFIG = os.environ.get("LLVM_CONFIG_BIN", "llvm-config-20")
CXX_BIN     = os.environ.get("CXX_BIN", "g++")
SKIP_BUILD  = os.environ.get("SKIP_BUILD", "0") == "1"
TIMEOUT_SEC = int(os.environ.get("TIMEOUT_SEC", "180"))

LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]

TOTAL_RE = re.compile(
    r"total kind=(\S+)\s+predicted_ns=([0-9.]+)\s+measured_ns=(\d+)\s+"
    r"predicted_ms=([0-9.]+)\s+measured_ms=([0-9.]+)\s+"
    r"pure_kernel_ns=(\d+)\s+pure_kernel_ms=([0-9.]+)"
)

# ─── Model constants (matching AutoTunerPass.cpp) ───
kPcsr   = 2.0
kBcsr   = 64.0
kR      = 24.0   # effective bytes per roaring_bitmap_contains


def load_calib(path=None):
    if not path:
        path = os.environ.get("HW_CALIB_JSON", "")
    if not path:
        home = os.environ.get("HOME", "")
        path = os.path.join(home, ".config", "sgpl", "hw_calib.json")
    if os.path.isfile(path):
        with open(path) as f:
            d = json.load(f)
        return d.get("L", 64.0), d.get("t", 32.99), d.get("T", 2.74), d.get("R", 10000.0)
    return 64.0, 32.99, 2.74, 10000.0


def count_graph_file(path):
    """Return (n_vertices, m_undirected) by scanning the edge file."""
    nv, edges = 0, 0
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            if len(parts) >= 2:
                try:
                    u, v = int(parts[0]), int(parts[1])
                    nv = max(nv, u + 1, v + 1)
                    edges += 1
                except ValueError:
                    continue
    return nv, edges  # undirected edge count


# ─── Cost model (replicates AutoTunerPass.cpp) ───
def traversal_cost(layout, n, m, L, t, T):
    d = (2.0 * m / n) if n > 0 else 1.0
    cNav = 0; bU = 0; cCompute = 0.0
    if   layout == "CSR":   cNav = 2*t;         bU = 16.0 + 4.0 * d
    elif layout == "PCSR":  cNav = 2*t;         bU = 16.0 + 4.0 * d * kPcsr
    elif layout == "BCSR":  cNav = 2*t;         bU = 8.0  + 8.0 * kBcsr * d
    elif layout == "SET":   cNav = 2*t;         bU = 16.0 * m;    cCompute = m * t / 6.0
    else: return float("inf")
    return n * (cNav + math.ceil(bU / L) * T + cCompute)


def insert_cost(layout, n, m, L, t, T, R=10000.0):
    """Per-undirected-edge insert cost.  m = undirected edge count.

    T  = sequential read-only cost per cache line (and memmove, whose
         independent read+write streams are overlapped by hardware).
    2T = read-modify-write cost per cache line (prefix-sum increments:
         dependent load → store, cannot overlap).
    """
    d = (2.0 * m / n) if n > 0 else 1.0
    if layout == "CSR":
        # One undirected edge = 2 directed inserts (from→to, to→from).
        # Per directed: realloc + memmove + row_ptr prefix-sum update.
        # C_locate  = 2t    (row_ptr[from+1] random read per directed)
        # C_prefix  = 2·⌈n/2·8/L⌉·2T  (2 dirs × n/2 int64 R-M-W)
        # C_move    = 2·⌈4m/L⌉·T      (2 dirs × ~4m bytes memmove)
        # C_realloc = 2·R              (2 dirs × mremap O(1))
        # C_write   = 2                (2 dirs × write 1 int32)
        return (4.0*t
                + 2.0*math.ceil((n/2.0)*8.0/L)*2.0*T
                + 2.0*math.ceil(4.0*m/L)*T
                + 2.0*R
                + 2.0)
    elif layout == "PCSR":
        # Gap-fill insert: no memmove, no realloc.
        # C_locate = 2·(2t + ⌈g·d·4/L⌉·T)  (2 dirs × random reads + read-only gap scan)
        # C_write  = 2·t                    (2 dirs × 1 random gap-slot write)
        gU = kPcsr * d
        return (4.0*t + 2.0*math.ceil(gU*4.0/L)*T
                + 2.0*t)
    elif layout == "BCSR":
        # C_locate  = 4t + 2·⌈d_BR·8/L⌉·T  (2 dirs × brow reads + read-only pair scan)
        # C_write   = 2 + 2·⌈nb/2·4/L⌉·2T   (2 dirs × pair write + brow prefix R-M-W)
        # C_move    = 2·⌈8m/L⌉·T            (2 dirs × ~8m bytes memmove)
        # C_realloc = 2·freshProb·R         (2 dirs × mremap O(1), fresh edges only)
        b = kBcsr
        nb = math.ceil(n / b)
        d_BR = b * d
        # freshProb = P(random (u,v) is a new edge) = 1 − 2m/(n(n−1))
        freshProb = max(0.0, 1.0 - 2.0*m / (n * (n - 1.0))) if n > 1.0 else 1.0
        cLocate = 4.0*t + 2.0*math.ceil(d_BR*8.0/L)*T
        cWrite  = 2.0 + 2.0*math.ceil((nb/2.0)*4.0/L)*2.0*T
        cMove   = 2.0*math.ceil(8.0*m/L)*T
        cRealloc = 2.0*freshProb*R
        return cLocate + freshProb*(cWrite + cMove) + cRealloc
    elif layout == "SET":
        # Per-undirected-edge insert via autograph_canonical_add_edge:
        #   c_ebm   = 2·t·(1 + log2(num_edge_containers))   # contains + add on edges_bitmap
        #   c_nbm   = 2·t·(1 + log2(num_node_containers))   # 2 adds on nodes_bitmap (u, v)
        #   c_refr  = t · num_node_containers               # refresh cardinality walk
        #   c_base  = 5·t                                    # FFI overhead
        #   c_build = 2·t·m / kAmortN                        # lazy static-edge hash build
        #   c_array: tiny-node dense-bitmap cost (unchanged).
        # Container count for a roaring bitmap of N elements = ceil(N / 65536).
        nbE = max(1, math.ceil(m / 65536.0))
        nbV = max(1, math.ceil(n / 65536.0))
        c_ebm  = 2.0 * t * (1.0 + math.log2(nbE))
        c_nbm  = 2.0 * t * (1.0 + math.log2(nbV))
        c_refr = t * nbV
        c_base = 5.0 * t
        kAmortN = 50.0   # must match --n-inserts default
        c_build = 2.0 * t * m / kAmortN
        cArray = 0.0
        if n <= 4096.0:
            cShift = math.ceil((n / 2.0) * 2.0 / L) * T
            cArray = 2.0 * (math.ceil(math.log2(n) * 2.0 / L) * T + cShift)
        return c_ebm + c_nbm + c_refr + c_base + c_build + cArray
    return float("inf")


# ─── Shell helpers ───
def run(cmd, cwd=None, env=None, check=True, timeout=None):
    return subprocess.run(cmd, cwd=cwd or SCRIPT_DIR,
                          env={**os.environ, **(env or {})},
                          check=check, text=True, capture_output=True,
                          timeout=timeout)


def parse_kernels(stderr):
    out = {}
    for line in stderr.splitlines():
        m = TOTAL_RE.search(line)
        if m:
            out[m.group(1)] = {
                "pred_ns":  float(m.group(2)),
                "meas_ns":  int(m.group(3)),
                "kernel_ns": int(m.group(6)),
            }
    return out


def median(values):
    """Return median of a list of ints."""
    if not values:
        return None
    s = sorted(values)
    n = len(s)
    if n % 2 == 1:
        return s[n // 2]
    return (s[n // 2 - 1] + s[n // 2]) // 2


def iqr(values):
    """Return interquartile range of a list of ints (0 if <4 values)."""
    if len(values) < 4:
        return 0
    s = sorted(values)
    n = len(s)
    q1 = s[n // 4]
    q3 = s[(3 * n) // 4]
    return q3 - q1


# ─── Build GraphProgram (once) ───
def build_graphprogram():
    run([CLANG_BIN, "-S", "-emit-llvm", "-O2",
         "autotuner_runtime.c", "-o", "autotuner_runtime.ll"])
    run([CLANG_BIN, "-S", "-emit-llvm", "-O2",
         "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2",
         "bfs_runtime.cpp", "-o", "bfs_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2",
         "bfs_runtime_src.cpp", "-o", "bfs_runtime_src.ll"])

    cxxf = shlex.split(subprocess.check_output([LLVM_CONFIG, "--cxxflags"], text=True).strip())
    cxxf = [x for x in cxxf if x != "-fno-exceptions"]
    ldf  = shlex.split(subprocess.check_output([LLVM_CONFIG, "--ldflags"], text=True).strip())
    libs = shlex.split(subprocess.check_output(
        [LLVM_CONFIG, "--libs", "core", "irreader", "analysis",
         "passes", "executionengine", "mcjit", "native", "support"], text=True).strip())
    slib = shlex.split(subprocess.check_output([LLVM_CONFIG, "--system-libs"], text=True).strip())

    srcs = [
        "main.cpp", "IRGenVisitor.cpp", "ASTBuilder.cpp", "pdg.cpp",
        "parallel_loop_outline.cpp", "SemanticAnalyzer.cpp",
        "roaring_bitmap.cpp", "AutoTunerPass.cpp",
        "generated/BaseBaseVisitor.cpp", "generated/BaseLexer.cpp",
        "generated/BaseListener.cpp", "generated/BaseParser.cpp",
        "generated/BaseVisitor.cpp", "generated/BaseBaseListener.cpp",
    ]
    cmd = ([CXX_BIN, "-O3", "-mavx2", "-march=native", "-g",
            "-std=c++17", "-fexceptions",
            "-I/usr/include/antlr4-runtime", "-Igenerated", "-I.", "-pthread"]
           + cxxf + srcs + ldf + ["-lantlr4-runtime"] + libs + slib
           + ["-o", "GraphProgram"])
    print("[build] GraphProgram ...", flush=True)
    run(cmd)
    print("[build] done.\n", flush=True)


# ─── Workload generation ───
def make_traverse_workload(edge_file, out_path):
    """Generate a traverse-only .graph file (no inserts)."""
    lines = [
        "graph g1 {",
        f'    edges: file "{edge_file}";',
        "};",
        "",
        "for each vertex v in g1 {",
        "    for each neighbor u of v in g1 {",
        "    };",
        "};",
    ]
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def make_insert_workload(edge_file, n_inserts, n_vertices, out_path):
    """Generate an insert-only .graph file (no traversals)."""
    rng = random.Random(42)
    lines = [
        "graph g1 {",
        f'    edges: file "{edge_file}";',
        "};",
        "",
    ]
    for _ in range(n_inserts):
        u = rng.randint(0, n_vertices - 1)
        v = rng.randint(0, n_vertices - 1)
        while v == u:
            v = rng.randint(0, n_vertices - 1)
        lines.append(f"add {u}->{v} to g1;")
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


# ─── Compile + run one layout ───
def compile_and_run(graph_path, layout, workdir, timeout, n_runs=10):
    env = {"AUTOTUNER_FORCE_LAYOUT": layout}
    cp = run(["./GraphProgram", str(graph_path)], env=env, check=False)
    if cp.returncode != 0:
        return {"error": f"GraphProgram failed: {cp.stderr[-200:]}"}

    if not Path("program.o").exists():
        return {"error": "no program.o"}

    run(["gcc", "-O3", "-c", "autotuner_runtime.c", "-o", "autotuner_runtime.o"])
    run(["gcc", "-O3", "-c", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.o"])
    run(["gcc", "-O3", "-c", "parallel_runtime.c", "-o", "parallel_runtime.o"])
    run(["gcc", "-O3", "-c", "runtime.c", "-o", "runtime.o"])
    run([CXX_BIN, "-O3", "-mavx2", "-march=native", "-fopenmp",
         "-c", "roaring_bitmap.cpp", "-o", "roaring_bitmap.o"])
    run([CXX_BIN, "-O2", "-std=c++17", "-fopenmp",
         "-c", "graph_loader_runtime.cpp", "-o", "graph_loader_runtime.o"])
    run([CXX_BIN, "-O2", "-std=c++17",
         "-c", "graph_runtime.cpp", "-o", "graph_runtime.o"])

    bin_path = workdir / f"final_{layout}"
    run([CXX_BIN, "-O3", "-mavx2", "-march=native", "-fopenmp", "-no-pie",
         "program.o", "runtime.o", "parallel_runtime.o",
         "autotuner_runtime.o", "graph_mutation_runtime.o",
         "roaring_bitmap.o", "graph_loader_runtime.o", "graph_runtime.o",
         "-lnlopt", "-o", str(bin_path)])

    # Clean up .o files
    for f in Path().glob("*.o"):
        f.unlink()

    # Run binary: 1 warmup + n_runs measured, return raw per-run kernel_ns
    all_runs = []  # list of parsed kernel dicts
    for i in range(n_runs + 1):
        try:
            cp = subprocess.run([str(bin_path)], text=True, capture_output=True,
                                timeout=timeout, env={**os.environ})
            if cp.returncode != 0:
                if i == 0:
                    return {"error": f"binary crashed (rc={cp.returncode})"}
                continue  # skip crashed measured run
            kernels = parse_kernels(cp.stderr or "")
            if i > 0:  # skip warmup
                all_runs.append(kernels)
        except subprocess.TimeoutExpired:
            if i == 0:
                return {"timeout": True}
            continue  # skip timed-out measured run

    if not all_runs:
        return {"timeout": True}

    # Return raw per-run kernel_ns values per op_kind (lists of ints)
    result = {}
    for op_kind in ["Insert", "Traverse", "Query"]:
        values = []
        for r in all_runs:
            kt = r.get(op_kind, {})
            kns = kt.get("kernel_ns", None)
            if kns is not None:
                values.append(kns)
        if values:
            result[op_kind] = values
    return result


# ─── Process a single folder of .txt files ───
def process_folder(edge_files, args, L, t, T, wdir):
    """Run benchmarks on edge_files and return list of row dicts."""
    rows = []
    for ef in edge_files:
        nv, m_und = count_graph_file(ef)
        label = ef.stem
        print(f"\n{'─'*55}\n  {label}: n={nv}  m_und={m_und}\n{'─'*55}", flush=True)

        # Generate separate workloads for clean measurements
        traverse_graph = wdir / f"{label}_traverse.graph"
        insert_graph = wdir / f"{label}_insert.graph"
        make_traverse_workload(ef, traverse_graph)
        make_insert_workload(ef, args.n_inserts, nv, insert_graph)

        # Run each layout against each workload
        traverse_raw = {}  # layout → {"Insert": [kns,...], "Traverse": [...]}
        insert_raw  = {}
        for layout in LAYOUTS:
            sys.stdout.write(f"  {layout:>6s} traverse ... "); sys.stdout.flush()
            result = compile_and_run(traverse_graph, layout, wdir, args.timeout, args.runs)
            if result.get("timeout"):
                print("TIMEOUT")
                traverse_raw[layout] = {}
            elif result.get("error"):
                print(f"FAIL ({result['error'][:40]})")
                traverse_raw[layout] = {}
            else:
                print("OK")
                traverse_raw[layout] = result

            sys.stdout.write(f"  {layout:>6s} insert ...   "); sys.stdout.flush()
            result = compile_and_run(insert_graph, layout, wdir, args.timeout, args.runs)
            if result.get("timeout"):
                print("TIMEOUT")
                insert_raw[layout] = {}
            elif result.get("error"):
                print(f"FAIL ({result['error'][:40]})")
                insert_raw[layout] = {}
            else:
                print("OK")
                insert_raw[layout] = result

        for op_kind in ["Insert", "Traverse"]:
            op_count = args.n_inserts if op_kind == "Insert" else 1
            raw_src  = insert_raw if op_kind == "Insert" else traverse_raw
            unit_fn  = insert_cost if op_kind == "Insert" else traversal_cost

            preds = {}
            meas_median = {}
            meas_iqr    = {}
            for layout in LAYOUTS:
                raw = raw_src.get(layout, {}).get(op_kind, [])
                preds[layout]      = op_count * unit_fn(layout, nv, m_und, L, t, T)
                meas_median[layout] = median(raw) if raw else None
                meas_iqr[layout]    = iqr(raw) if raw else None

            valid = [(l, preds[l], meas_median[l]) for l in LAYOUTS if meas_median[l] is not None]
            bp = min(valid, key=lambda x: x[1])[0] if valid else "-"
            bm = min(valid, key=lambda x: x[2])[0] if valid else "-"

            # NOISE if mis-match but IQRs overlap
            if bp != "-" and bm != "-" and bp != bm:
                p_iqr = meas_iqr.get(bp, 0)
                m_iqr = meas_iqr.get(bm, 0)
                if p_iqr is None: p_iqr = 0
                if m_iqr is None: m_iqr = 0
                overlap = abs(meas_median[bp] - meas_median[bm]) < max(p_iqr, m_iqr)
                verdict = "NOISE" if overlap else "MISMATCH"
            else:
                verdict = "MATCH"

            for layout in LAYOUTS:
                med = meas_median[layout]
                iq  = meas_iqr[layout]
                rows.append({
                    "graph":             label,
                    "n_vertices":        nv,
                    "m_undirected":      m_und,
                    "operation":         op_kind,
                    "layout":            layout,
                    "predicted_ns":      f"{preds[layout]:.1f}",
                    "measured_kernel_ns": str(med) if med is not None else "",
                    "measured_iqr_ns":   str(iq) if med is not None else "",
                    "kernel_cv":         f"{(iq / 1.349) / med:.3f}" if med and med > 0 and iq > 0 else "",
                    "predicted_best":    bp,
                    "measured_best":     bm,
                    "verdict":           verdict if layout == LAYOUTS[0] else "",
                })
    return rows


# ─── Main ───
def main():
    parser = argparse.ArgumentParser(
        description="Benchmark all layouts across graph files (one CSV per subfolder)")
    parser.add_argument("folder", help="Folder with .txt files, or parent of subfolders")
    parser.add_argument("--out", default="bench_results.csv", help="Output CSV path (single-folder mode)")
    parser.add_argument("--n-inserts", type=int, default=50, help="Inserts per workload")
    parser.add_argument("--timeout", type=int, default=TIMEOUT_SEC, help="Per-run timeout")
    parser.add_argument("--calib", help="Path to hw_calib.json")
    parser.add_argument("--runs", type=int, default=10,
                        help="Measured runs per layout (default 10; first run is warmup)")
    args = parser.parse_args()

    folder = Path(args.folder).resolve()
    if not folder.is_dir():
        print(f"Error: {folder} is not a directory", file=sys.stderr)
        sys.exit(1)

    L, t, T, R = load_calib(args.calib)
    print(f"HW calib: L={L:.0f}  t={t:.2f}ns  T={T:.2f}ns\n", flush=True)

    os.chdir(SCRIPT_DIR)

    # Calibration & build (once)
    calib_dir = Path.home() / ".config" / "sgpl"
    calib_dir.mkdir(parents=True, exist_ok=True)
    if not (calib_dir / "hw_calib.json").exists():
        run(["./hw_calib_bench"], stdout=open(str(calib_dir / "hw_calib.json"), "w"))
    if not SKIP_BUILD:
        build_graphprogram()
    elif not Path("./GraphProgram").exists():
        build_graphprogram()

    csv_fields = [
        "graph", "n_vertices", "m_undirected", "operation", "layout",
        "predicted_ns", "measured_kernel_ns", "measured_iqr_ns", "kernel_cv",
        "predicted_best", "measured_best", "verdict"]

    # Detect mode: single folder with .txt files vs parent of subdirectories
    direct_txt = sorted(folder.glob("*.txt"))
    if direct_txt:
        # Single-folder mode (original behaviour)
        print(f"Single-folder mode — {len(direct_txt)} edge files in {folder}\n", flush=True)
        wdir = Path(tempfile.mkdtemp(prefix="bench_folder_"))
        rows = process_folder(direct_txt, args, L, t, T, wdir)
        out_path = Path(args.out)
        with open(out_path, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=csv_fields)
            w.writeheader()
            w.writerows(rows)
        print(f"\n[done] wrote {out_path}  ({len(rows)} rows)", flush=True)
        shutil.rmtree(wdir)
    else:
        # Multi-folder mode — one CSV per subdirectory
        subdirs = sorted(d for d in folder.iterdir() if d.is_dir())
        if not subdirs:
            print(f"Error: no .txt files or subdirectories found in {folder}", file=sys.stderr)
            sys.exit(1)

        print(f"Multi-folder mode — {len(subdirs)} subdirectories found\n", flush=True)
        wdir = Path(tempfile.mkdtemp(prefix="bench_folder_"))
        for subdir in subdirs:
            edge_files = sorted(subdir.glob("*.txt"))
            if not edge_files:
                print(f"  SKIP (empty): {subdir.name}", flush=True)
                continue

            csv_name = subdir.name.replace(" ", "_") + ".csv"
            print(f"\n{'='*60}\n  >>> {subdir.name} ({len(edge_files)} files) → {csv_name}\n{'='*60}", flush=True)

            rows = process_folder(edge_files, args, L, t, T, wdir)
            if rows:
                with open(csv_name, "w", newline="") as f:
                    w = csv.DictWriter(f, fieldnames=csv_fields)
                    w.writeheader()
                    w.writerows(rows)
                print(f"\n  [done] wrote {csv_name}  ({len(rows)} rows)", flush=True)
        shutil.rmtree(wdir)
        print("\nAll done.", flush=True)


if __name__ == "__main__":
    main()