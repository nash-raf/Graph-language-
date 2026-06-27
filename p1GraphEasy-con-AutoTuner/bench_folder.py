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
  ./bench_folder.py <edge_folder> [--out results.csv] [--n-inserts 50]
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
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

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
        return d.get("L", 64.0), d.get("t", 1.0), d.get("T", 1.0)
    return 64.0, 1.0, 1.0


def count_graph_file(path):
    """Return (n_vertices, m_directed) by scanning the edge file."""
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
    return nv, edges * 2  # directed edges = 2 * undirected


# ─── Cost model (replicates AutoTunerPass.cpp) ───
def traversal_cost(layout, n, m, L, t, T):
    d = (2.0 * m / n) if n > 0 else 1.0
    if   layout == "CSR":   cNav = 2*t;         bU = 16.0 + 4.0 * d
    elif layout == "PCSR":  cNav = 2*t;         bU = 16.0 + 4.0 * d * kPcsr
    elif layout == "BCSR":  cNav = 2*t;         bU = 8.0  + 8.0 * kBcsr * d
    elif layout == "SET":   cNav = 1*t;         bU = 16.0 * m
    else: return float("inf")
    return n * (cNav + math.ceil(bU / L) * T)


def insert_cost(layout, n, m, L, t, T):
    d = (2.0 * m / n) if n > 0 else 1.0
    if layout == "CSR":
        return ((2 + d)*t + 1 + math.ceil((n/2)*8/L)*t + math.ceil(4*(m/2)/L)*t)
    elif layout == "PCSR":
        gU = kPcsr * d
        return ((2+gU)*t + t)  # gap-fill insert: no memmove
    elif layout == "BCSR":
        b = kBcsr
        nb = math.ceil(n / b)
        d_BR = b * d
        blkIdx = n / (2.0 * b)
        p_B = m
        cLocate = (2.0 + d_BR) * t
        cWrite = 1.0 + math.ceil((nb - blkIdx) * 4.0 / L) * t
        cMove = math.ceil(4.0 * (2.0 * m - p_B) / L) * t
        return cLocate + cWrite + cMove
    elif layout == "SET":
        # Empirical model: autograph_canonical_add_edge cost is dominated by
        # random hash-table probes (~40 bytes per edge in static hash table)
        # plus two roaring_bitmap_add FFI calls.
        cHash = math.ceil(m * 40.0 / L) * T
        cBase = 5.0 * t
        # Array-container regime (n <= 4096): roaring bitmap add does
        # binary-search + memmove of ~n/2 uint16 entries.
        cArray = 0.0
        if n <= 4096.0:
            cShift = math.ceil((n / 2.0) * 2.0 / L) * T
            cArray = 2.0 * (math.ceil(math.log2(n) * 2.0 / L) * T + cShift)
        return cHash + cBase + cArray
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


# ─── Compile + run one layout ───
def make_workload_graph(edge_file, n_inserts, n_vertices, out_path):
    rng = random.Random(42)
    traversal = [
        "for each vertex v in g1 {",
        "    for each neighbor u of v in g1 {",
        "    };",
        "};",
    ]
    lines = [
        "graph g1 {",
        f'    edges: file "{edge_file}";',
        "};",
        "",
    ]
    lines.extend(traversal)
    lines.append("")
    for _ in range(n_inserts):
        u = rng.randint(0, n_vertices - 1)
        v = rng.randint(0, n_vertices - 1)
        while v == u:
            v = rng.randint(0, n_vertices - 1)
        lines.append(f"add {u}->{v} to g1;")
    lines.append("")
    lines.extend(traversal)
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def compile_and_run(graph_path, layout, workdir, timeout):
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

    try:
        cp = subprocess.run([str(bin_path)], text=True, capture_output=True,
                            timeout=timeout, env={**os.environ})
        return parse_kernels(cp.stderr or "")
    except subprocess.TimeoutExpired:
        return {"timeout": True}


# ─── Process a single folder of .txt files ───
def process_folder(edge_files, args, L, t, T, wdir):
    """Run benchmarks on edge_files and return list of row dicts."""
    rows = []
    for ef in edge_files:
        nv, md = count_graph_file(ef)
        label = ef.stem
        print(f"\n{'─'*55}\n  {label}: n={nv}  m_dir={md}\n{'─'*55}", flush=True)

        graph_path = wdir / f"{label}.graph"
        make_workload_graph(ef, args.n_inserts, nv, graph_path)

        layout_kernels = {}
        for layout in LAYOUTS:
            sys.stdout.write(f"  {layout:>6s} ... "); sys.stdout.flush()
            result = compile_and_run(graph_path, layout, wdir, args.timeout)
            if result.get("timeout"):
                print("TIMEOUT")
                layout_kernels[layout] = {}
            elif result.get("error"):
                print(f"FAIL ({result['error'][:40]})")
                layout_kernels[layout] = {}
            else:
                print("OK")
                layout_kernels[layout] = result

        for op_kind in ["Insert", "Traverse"]:
            op_count = args.n_inserts if op_kind == "Insert" else 1
            unit_fn = insert_cost if op_kind == "Insert" else traversal_cost

            preds = {}
            meas  = {}
            for layout in LAYOUTS:
                k = layout_kernels.get(layout, {})
                kt = k.get(op_kind, {})
                preds[layout] = op_count * unit_fn(layout, nv, md, L, t, T)
                meas[layout]  = kt.get("kernel_ns", None)

            valid = [(l, preds[l], meas[l]) for l in LAYOUTS if meas[l] is not None]
            bp = min(valid, key=lambda x: x[1])[0] if valid else "-"
            bm = min(valid, key=lambda x: x[2])[0] if valid else "-"
            verdict = "MATCH" if bp == bm and bp != "-" else "MISMATCH"

            for layout in LAYOUTS:
                rows.append({
                    "graph":          label,
                    "n_vertices":     nv,
                    "m_directed":     md,
                    "operation":      op_kind,
                    "layout":         layout,
                    "predicted_ns":   f"{preds[layout]:.1f}",
                    "measured_kernel_ns": str(meas[layout]) if meas[layout] is not None else "",
                    "predicted_best": bp,
                    "measured_best":  bm,
                    "verdict":        verdict if layout == LAYOUTS[0] else "",
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
    args = parser.parse_args()

    folder = Path(args.folder).resolve()
    if not folder.is_dir():
        print(f"Error: {folder} is not a directory", file=sys.stderr)
        sys.exit(1)

    L, t, T = load_calib(args.calib)
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

    # Detect mode: single folder with .txt files vs parent of subdirectories
    direct_txt = sorted(folder.glob("*.txt"))
    if direct_txt:
        # Single-folder mode (original behaviour)
        print(f"Single-folder mode — {len(direct_txt)} edge files in {folder}\n", flush=True)
        wdir = Path(tempfile.mkdtemp(prefix="bench_folder_"))
        rows = process_folder(direct_txt, args, L, t, T, wdir)
        out_path = Path(args.out)
        with open(out_path, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=[
                "graph", "n_vertices", "m_directed", "operation", "layout",
                "predicted_ns", "measured_kernel_ns", "predicted_best", "measured_best", "verdict"])
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
                    w = csv.DictWriter(f, fieldnames=[
                        "graph", "n_vertices", "m_directed", "operation", "layout",
                        "predicted_ns", "measured_kernel_ns", "predicted_best", "measured_best", "verdict"])
                    w.writeheader()
                    w.writerows(rows)
                print(f"\n  [done] wrote {csv_name}  ({len(rows)} rows)", flush=True)
        shutil.rmtree(wdir)
        print("\nAll done.", flush=True)


if __name__ == "__main__":
    main()
