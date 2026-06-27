#!/usr/bin/env python3
"""
bench_varying_size.py — Compare predicted vs measured per-operation costs
for Insert and Traverse across varying graph sizes and all four layouts.

Computes model predictions analytically from the cache-line-aware equations
(replicating AutoTunerPass.cpp) and compares against pure kernel times
measured at runtime.

Usage:
  ./bench_varying_size.py

Environment:
  SKIP_BUILD=1    reuse existing GraphProgram
  KEEP_FILES=1    do not delete generated files
  TIMEOUT_SEC=    per-run timeout (default 120)
"""

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
CLANG_BIN = os.environ.get("CLANG_BIN", "clang-20")
LLVM_CONFIG_BIN = os.environ.get("LLVM_CONFIG_BIN", "llvm-config-20")
CXX_BIN = os.environ.get("CXX_BIN", "g++")
SKIP_BUILD = os.environ.get("SKIP_BUILD", "0") == "1"
KEEP = os.environ.get("KEEP_FILES", "0") == "1"
TIMEOUT_SEC = int(os.environ.get("TIMEOUT_SEC", "180"))

LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]

TOTAL_RE = re.compile(
    r"total kind=(\S+)\s+predicted_ns=([0-9.]+)\s+measured_ns=(\d+)\s+"
    r"predicted_ms=([0-9.]+)\s+measured_ms=([0-9.]+)\s+"
    r"pure_kernel_ns=(\d+)\s+pure_kernel_ms=([0-9.]+)"
)

# ─── Cost-model constants (matching AutoTunerPass.cpp) ───
kPcsrExpansionFactor = 2.0
kBcsrBlockSize = 64.0
kRoaringPerCheck = 24.0  # effective bytes per roaring_bitmap_contains

def load_hw_calib():
    """Load hardware calibration, matching loadHwCalib()."""
    path = os.environ.get("AUTOTUNER_HW_CALIB", "")
    if not path:
        home = os.environ.get("HOME", "")
        if home:
            path = os.path.join(home, ".config", "sgpl", "hw_calib.json")
    if path and os.path.isfile(path):
        import json
        try:
            with open(path) as f:
                d = json.load(f)
            return d.get("L", 64.0), d.get("t", 1.0), d.get("T", 1.0)
        except Exception:
            pass
    return 64.0, 1.0, 1.0  # defaults

L, t_ns, T_ns = load_hw_calib()


def traversal_cost(layout, n, m):
    """Replicate traversalCost() from AutoTunerPass.cpp (per-node unit cost)."""
    d = (2.0 * m / n) if n > 0 else 1.0
    if layout == "CSR":
        cNav = 2.0 * t_ns
        bUseful = 16.0 + 4.0 * d
    elif layout == "PCSR":
        cNav = 2.0 * t_ns
        bUseful = 16.0 + 4.0 * d * kPcsrExpansionFactor
    elif layout == "BCSR":
        cNav = 2.0 * t_ns
        bUseful = 8.0 + 8.0 * kBcsrBlockSize * d
    elif layout == "SET":
        cNav = 1.0 * t_ns
        bUseful = 16.0 * m
    else:
        return float("inf")
    cTraffic = math.ceil(bUseful / L) * T_ns
    return n * (cNav + cTraffic)


def insert_cost(layout, n, m):
    """Replicate insertCost() from AutoTunerPass.cpp (per-op unit cost)."""
    d = (2.0 * m / n) if n > 0 else 1.0
    u = n / 2.0
    p = m / 2.0
    gU = kPcsrExpansionFactor * d

    if layout == "CSR":
        return ((2.0 + d) * t_ns
                + 1.0 + math.ceil((n - u) * 8.0 / L) * t_ns
                + math.ceil(4.0 * (m - p) / L) * t_ns)
    elif layout == "PCSR":
        return ((2.0 + gU) * t_ns
                + 1.0 * t_ns)  # gap-fill insert: no memmove
    elif layout == "BCSR":
        b = kBcsrBlockSize
        nb = math.ceil(n / b)
        d_BR = b * d
        blkIdx = n / (2.0 * b)
        p_B = m
        cLocate = (2.0 + d_BR) * t_ns
        cWrite = 1.0 + math.ceil((nb - blkIdx) * 4.0 / L) * t_ns
        cMove = math.ceil(4.0 * (2.0 * m - p_B) / L) * t_ns
        return cLocate + cWrite + cMove
    elif layout == "SET":
        # Hash-table probes dominate; see AutoTunerPass.cpp for rationale.
        cHash = math.ceil(m * 40.0 / L) * T_ns
        cBase = 5.0 * t_ns
        cArray = 0.0
        if n <= 4096.0:
            cShift = math.ceil((n / 2.0) * 2.0 / L) * T_ns
            cArray = 2.0 * (math.ceil(math.log2(n) * 2.0 / L) * T_ns + cShift)
        return cHash + cBase + cArray
    return float("inf")


# ---------------------------------------------------------------------------
def run(cmd, cwd=None, env=None, check=True, timeout=None):
    merged = {**os.environ, **(env or {})}
    return subprocess.run(
        cmd, cwd=cwd or SCRIPT_DIR, env=merged,
        check=check, text=True, capture_output=True, timeout=timeout
    )


def parse_kernel_times(stderr_text):
    """Parse pure_kernel_ns from the 'total' profiling lines."""
    kernels = {}
    for line in stderr_text.splitlines():
        m = TOTAL_RE.search(line)
        if m:
            kind = m.group(1)
            kernels[kind] = {
                "pred_ns": float(m.group(2)),
                "meas_ns": int(m.group(3)),
                "kernel_ns": int(m.group(6)),
            }
    return kernels


def llvm_flags():
    cxxflags = shlex.split(
        subprocess.check_output([LLVM_CONFIG_BIN, "--cxxflags"], text=True).strip()
    )
    cxxflags = [f for f in cxxflags if f != "-fno-exceptions"]
    ldflags = shlex.split(
        subprocess.check_output([LLVM_CONFIG_BIN, "--ldflags"], text=True).strip()
    )
    libs = shlex.split(
        subprocess.check_output(
            [LLVM_CONFIG_BIN, "--libs", "core", "irreader", "analysis",
             "passes", "executionengine", "mcjit", "native", "support"],
            text=True,
        ).strip()
    )
    system_libs = shlex.split(
        subprocess.check_output([LLVM_CONFIG_BIN, "--system-libs"], text=True).strip()
    )
    return cxxflags, ldflags, libs, system_libs


def build_graphprogram():
    cxxflags, ldflags, libs, system_libs = llvm_flags()
    run([CLANG_BIN, "-S", "-emit-llvm", "-O2",
         "autotuner_runtime.c", "-o", "autotuner_runtime.ll"])
    run([CLANG_BIN, "-S", "-emit-llvm", "-O2",
         "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2",
         "bfs_runtime.cpp", "-o", "bfs_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2",
         "bfs_runtime_src.cpp", "-o", "bfs_runtime_src.ll"])

    sources = [
        "main.cpp", "IRGenVisitor.cpp", "ASTBuilder.cpp", "pdg.cpp",
        "parallel_loop_outline.cpp", "SemanticAnalyzer.cpp",
        "roaring_bitmap.cpp", "AutoTunerPass.cpp",
        "generated/BaseBaseVisitor.cpp", "generated/BaseLexer.cpp",
        "generated/BaseListener.cpp", "generated/BaseParser.cpp",
        "generated/BaseVisitor.cpp", "generated/BaseBaseListener.cpp",
    ]
    cmd = (
        [CXX_BIN, "-O3", "-mavx2", "-march=native", "-g",
         "-std=c++17", "-fexceptions",
         "-I/usr/include/antlr4-runtime", "-Igenerated", "-I.",
         "-pthread"]
        + cxxflags + sources + ldflags
        + ["-lantlr4-runtime"] + libs + system_libs
        + ["-o", "GraphProgram"]
    )
    print("[build] GraphProgram ...", flush=True)
    run(cmd)
    print("[build] done.\n", flush=True)


def generate_edge_file(path, n, m):
    rng = random.Random(42)
    edges = set()
    while len(edges) < m:
        u = rng.randint(0, n - 1)
        v = rng.randint(0, n - 1)
        if u == v or (v, u) in edges:
            continue
        edges.add((u, v))
    with open(path, "w") as f:
        for u, v in edges:
            f.write(f"{u} {v}\n")


def generate_workload_graph(edge_file, n_inserts, n_vertices, out_path):
    """Program: Traverse → N inserts → Traverse (3 separate regions)."""
    lines = [
        "graph g1 {",
        f'    edges: file "{edge_file}";',
        "};",
        "",
        'query bfs_pre: "bfs" of g1;',
        "",
    ]
    import random as _random
    r = _random.Random(42)
    for _ in range(n_inserts):
        u = r.randint(0, n_vertices - 1)
        v = r.randint(0, n_vertices - 1)
        while v == u:
            v = r.randint(0, n_vertices - 1)
        lines.append(f"add {u}->{v} to g1;")
    lines.append("")
    lines.append('query bfs_post: "bfs" of g1;')
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def compile_and_link(graph_file, layout, workdir):
    env = {"AUTOTUNER_FORCE_LAYOUT": layout}
    cp = run(["./GraphProgram", str(graph_file)], env=env, check=False)
    if cp.returncode != 0:
        raise RuntimeError(f"GraphProgram failed: {cp.stderr[-300:]}")
    if not Path("program.o").exists():
        raise RuntimeError(f"No program.o for {layout}")

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
    link_cmd = (
        [CXX_BIN, "-O3", "-mavx2", "-march=native", "-fopenmp", "-no-pie",
         "program.o", "runtime.o", "parallel_runtime.o",
         "autotuner_runtime.o", "graph_mutation_runtime.o",
         "roaring_bitmap.o", "graph_loader_runtime.o", "graph_runtime.o",
         "-lnlopt", "-o", str(bin_path)]
    )
    run(link_cmd)
    for f in Path().glob("*.o"):
        shutil.move(str(f), str(workdir / f.name))
    return bin_path


def run_binary(binary, timeout=TIMEOUT_SEC):
    try:
        cp = subprocess.run(
            [str(binary)], text=True, capture_output=True,
            timeout=timeout, env={**os.environ}
        )
        return parse_kernel_times(cp.stderr or "")
    except subprocess.TimeoutExpired:
        return {"timeout": True}


# ---------------------------------------------------------------------------
def fmt_ns(ns):
    if ns <= 0:
        return "       -"
    ms = ns / 1.0e6
    if ms < 1.0:
        return f"{ms:8.3f}"
    elif ms < 100.0:
        return f"{ms:8.1f}"
    elif ms < 10000.0:
        return f"{ms:8.0f}"
    else:
        return f"{ms:8.0f}"


def best_layout(values):
    """Return (best, verdict_string) for a dict {layout: (pred, meas)}."""
    valid = {l: (p, m) for l, (p, m) in values.items() if m > 0}
    if not valid:
        return ("-", "-")
    bp = min(valid, key=lambda l: valid[l][0])
    bm = min(valid, key=lambda l: valid[l][1])
    return (bp, "MATCH" if bp == bm else "MISMATCH")


def main():
    os.chdir(SCRIPT_DIR)

    calib_dir = Path.home() / ".config" / "sgpl"
    calib_dir.mkdir(parents=True, exist_ok=True)
    if not (calib_dir / "hw_calib.json").exists():
        run(["./hw_calib_bench"], stdout=open(str(calib_dir / "hw_calib.json"), "w"))

    if not SKIP_BUILD:
        build_graphprogram()
    elif not Path("./GraphProgram").exists():
        build_graphprogram()

    SIZES = [
        ("5Kv_10Ke",   5000,   10000),
        ("5Kv_25Ke",   5000,   25000),
        ("5Kv_50Ke",   5000,   50000),
    ]
    N_INSERTS = 50

    print(f" HW: L={L:.0f}  t={t_ns:.2f}ns  T={T_ns:.2f}ns  kR={kRoaringPerCheck:.0f}")
    print()

    all_results = {}
    wdir_base = Path(tempfile.mkdtemp(prefix="bench_vsize_"))
    print(f"[workdir] {wdir_base}\n", flush=True)

    for label, nv, me in SIZES:
        # Directed edge count = 2 * undirected edges
        m_directed = me * 2
        print(f"{'─' * 65}")
        print(f"  {label}: n={nv}  m_undir={me}  m_dir={m_directed}")
        print(f"{'─' * 65}")

        size_dir = wdir_base / label
        size_dir.mkdir(exist_ok=True)

        edge_path = size_dir / f"edges_{label}.txt"
        generate_edge_file(edge_path, nv, me)

        graph_path = size_dir / f"workload_{label}.graph"
        generate_workload_graph(edge_path, N_INSERTS, nv, graph_path)

        layout_data = {}
        for layout in LAYOUTS:
            sys.stdout.write(f"  {layout:>6s} ... ")
            sys.stdout.flush()
            try:
                binary = compile_and_link(graph_path, layout, size_dir)
                kernels = run_binary(binary)
                if kernels.get("timeout"):
                    print("TIMEOUT")
                    layout_data[layout] = {"timeout": True}
                else:
                    print("OK")
                    layout_data[layout] = kernels
            except Exception as e:
                print(f"FAIL ({e})")
                layout_data[layout] = {"error": str(e)}

        all_results[label] = (nv, m_directed, layout_data)

    # ── Generate comparison tables ──
    for kind_display, kind_key, unit_cost_fn, op_count in [
        ("INSERT", "Insert", insert_cost, N_INSERTS),
        ("TRAVERSE", "Traverse", traversal_cost, 1),
    ]:
        print(f"\n{'=' * 90}")
        print(f"  {kind_display}: Predicted (model) vs Measured (kernel) across graph sizes")
        print(f"  Predicted = {op_count} op(s) × unit_cost(model, n, m_dir)")
        print(f"{'=' * 90}")

        hdr_cols = " ".join(f"{l:>8s}" for l in LAYOUTS)
        hdr = f"  {'Size':<14s} | {hdr_cols} | {hdr_cols} | {'Pred Best':>10s} | {'Meas Best':>10s}"
        sub = f"  {'':14s} | {'(pred ms)':>34s} | {'(meas ms)':>34s} |"
        sep = "  " + "-" * 108
        print(sep)
        print(hdr)
        print(sep)

        score_ok = 0
        score_total = 0
        for label, (nv, m_dir, layout_data) in all_results.items():
            preds = {}
            meas = {}
            for layout in LAYOUTS:
                kd = layout_data.get(layout, {})
                if kd.get("timeout") or kd.get("error"):
                    preds[layout] = -1
                    meas[layout] = -1
                    continue
                # Predict from model
                preds[layout] = op_count * unit_cost_fn(layout, nv, m_dir)
                # Measured from kernel time
                kt = kd.get(kind_key, {})
                m = kt.get("kernel_ns", 0)
                if m == 0:
                    m = kt.get("meas_ns", 0)  # fallback
                meas[layout] = m

            pred_strs = [fmt_ns(preds[l]) for l in LAYOUTS]
            meas_strs = [fmt_ns(meas[l]) for l in LAYOUTS]

            values = {l: (preds[l], meas[l]) for l in LAYOUTS if meas[l] > 0}
            bp, bm, v = "-", "-", "-"
            if values:
                bp = min(values, key=lambda l: values[l][0])
                bm = min(values, key=lambda l: values[l][1])
                v = "MATCH" if bp == bm else "MISMATCH"
                score_total += 1
                if v == "MATCH":
                    score_ok += 1

            row = f"  {label:<14s} | {' '.join(pred_strs)} | {' '.join(meas_strs)} | {bp:>10s} | {bm:>10s}  {v}"
            print(row)

        print(sep)
        if score_total > 0:
            print(f"  Score: {score_ok}/{score_total} correct  ({100*score_ok/score_total:.0f}%)")

    # ── Per-op unit cost detail ──
    print(f"\n{'=' * 70}")
    print("  PER-OP UNIT COST: Predicted vs Measured (ns)")
    print(f"{'=' * 70}")
    for label, (nv, m_dir, layout_data) in all_results.items():
        print(f"\n  {label}  (n={nv}, m_dir={m_dir})")
        print(f"  {'Layout':<6s} | {'Ins Pred/op':>11s} | {'Ins Meas/op':>11s} | "
              f"{'Trav Pred/op':>12s} | {'Trav Meas/op':>12s}")
        print(f"  {'-' * 65}")
        for layout in LAYOUTS:
            kd = layout_data.get(layout, {})
            if kd.get("timeout") or kd.get("error"):
                print(f"  {layout:<6s} | {'(timeout)':>11s}")
                continue
            ip = insert_cost(layout, nv, m_dir)
            tp = traversal_cost(layout, nv, m_dir)
            im = kd.get("Insert", {}).get("kernel_ns", 0) / N_INSERTS
            tm = kd.get("Traverse", {}).get("kernel_ns", 0) / 1.0
            print(f"  {layout:<6s} | {ip:>11.1f} | {im:>11.1f} | {tp:>12.1f} | {tm:>12.1f}")

    if not KEEP:
        shutil.rmtree(wdir_base)
        print(f"\n[cleanup] removed {wdir_base}")
    else:
        print(f"\n[keep] files in {wdir_base}")


if __name__ == "__main__":
    main()
