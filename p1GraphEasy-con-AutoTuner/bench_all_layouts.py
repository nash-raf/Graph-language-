#!/usr/bin/env python3
"""
bench_all_layouts.py — Compile and run a graph program under all four
forced layouts (CSR, PCSR, BCSR, SET) and produce a table comparing
predicted vs measured times per operation type (Traverse, Insert, Query).

Usage:
  ./bench_all_layouts.py <input.graph>

Environment:
  LLVM_PREFIX         path to LLVM install (default /usr)
  CLANGXX_BIN         clang++ binary (default clang++-20)
  CLANG_BIN           clang binary (default clang-20)
  LLVM_CONFIG_BIN     llvm-config binary (default llvm-config-20)
  CXX_BIN             C++ compiler for linking (default g++)
  SKIP_BUILD          if 1, reuse existing GraphProgram
  KEEP_BINARIES       if 1, do not delete intermediate binaries
  BENCH_TIMEOUT_SEC   per-layout runtime timeout in seconds (default 60)
"""

import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
SCRIPT_DIR = Path(__file__).resolve().parent

CLANGXX_BIN = os.environ.get("CLANGXX_BIN", "clang++-20")
CLANG_BIN = os.environ.get("CLANG_BIN", "clang-20")
LLVM_CONFIG_BIN = os.environ.get("LLVM_CONFIG_BIN", "llvm-config-20")
CXX_BIN = os.environ.get("CXX_BIN", "g++")
SKIP_BUILD = os.environ.get("SKIP_BUILD", "0") == "1"
KEEP = os.environ.get("KEEP_BINARIES", "0") == "1"
BENCH_TIMEOUT_SEC = int(os.environ.get("BENCH_TIMEOUT_SEC", "60"))

LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]

# Profiling regex — parse [AutoTunerProfile] lines from stderr
REGION_RE = re.compile(
    r"region=(\d+)\s+kind=(\S+)\s+layout=(\S+)\s+visits=(\d+)\s+"
    r"predicted_ns=([0-9.]+)\s+measured_ns=(\d+)\s+"
    r"predicted_ms=([0-9.]+)\s+measured_ms=([0-9.]+)"
)
TOTAL_RE = re.compile(
    r"total kind=(\S+)\s+predicted_ns=([0-9.]+)\s+measured_ns=(\d+)\s+"
    r"predicted_ms=([0-9.]+)\s+measured_ms=([0-9.]+)\s+"
    r"pure_kernel_ns=(\d+)\s+pure_kernel_ms=([0-9.]+)"
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def run(cmd, cwd=None, env=None, check=True):
    """Run a command, return CompletedProcess.  On failure, print stderr."""
    merged = {**os.environ, **(env or {})}
    try:
        return subprocess.run(
            cmd, cwd=cwd or SCRIPT_DIR, env=merged,
            check=check, text=True, capture_output=True
        )
    except subprocess.CalledProcessError as e:
        sys.stderr.write(f"  [CMD FAILED] {' '.join(cmd)}\n")
        if e.stderr:
            sys.stderr.write(f"  [stderr] {e.stderr[:800].strip()}\n")
        if e.stdout:
            sys.stderr.write(f"  [stdout] {e.stdout[:400].strip()}\n")
        raise


def llvm_flags():
    """Return (cxxflags, ldflags, libs, system_libs) from llvm-config."""
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


def parse_profile(stderr_text):
    """
    Parse AutoTunerProfile lines from stderr.
    Returns dict:
      { "regions": [ {kind, layout, visits, pred_ns, meas_ns, pred_ms, meas_ms}, ... ],
        "totals":  { "Traverse": {pred_ns, meas_ns, pred_ms, meas_ms, kernel_ns, kernel_ms},
                      "Insert":   {...}, "Query": {...} } }
    """
    regions = []
    totals = {}
    for line in stderr_text.splitlines():
        if "pure_kernel" in line or "total kind=" in line:
            m = TOTAL_RE.search(line)
            if m:
                kind = m.group(1)
                totals[kind] = {
                    "pred_ns": float(m.group(2)),
                    "meas_ns": int(m.group(3)),
                    "pred_ms": float(m.group(4)),
                    "meas_ms": float(m.group(5)),
                    "kernel_ns": int(m.group(6)),
                    "kernel_ms": float(m.group(7)),
                }
        elif "region=" in line:
            m = REGION_RE.search(line)
            if m:
                regions.append({
                    "id": int(m.group(1)),
                    "kind": m.group(2),
                    "layout": m.group(3),
                    "visits": int(m.group(4)),
                    "pred_ns": float(m.group(5)),
                    "meas_ns": int(m.group(6)),
                    "pred_ms": float(m.group(7)),
                    "meas_ms": float(m.group(8)),
                })
    return {"regions": regions, "totals": totals}


# ---------------------------------------------------------------------------
# Build GraphProgram (once)
# ---------------------------------------------------------------------------
def build_graphprogram():
    cxxflags, ldflags, libs, system_libs = llvm_flags()

    # Emit runtime LLVM IR
    run([CLANG_BIN, "-S", "-emit-llvm", "-O2",
         "autotuner_runtime.c", "-o", "autotuner_runtime.ll"])
    run([CLANG_BIN, "-S", "-emit-llvm", "-O2",
         "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2",
         "bfs_runtime.cpp", "-o", "bfs_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2",
         "bfs_runtime_src.cpp", "-o", "bfs_runtime_src.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2",
         "dfs_runtime.cpp", "-o", "dfs_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2",
         "dfs_runtime_src.cpp", "-o", "dfs_runtime_src.ll"])

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
    print("[build] GraphProgram ...")
    run(cmd)
    print("[build] done.\n")


# ---------------------------------------------------------------------------
# Compile & link for one layout
# ---------------------------------------------------------------------------
def compile_for_layout(graph_file, layout, workdir):
    """Run GraphProgram with forced layout, produce an executable."""
    print(f"  [compile] GraphProgram {graph_file} layout={layout} ...")
    env = {"AUTOTUNER_FORCE_LAYOUT": layout}
    cp = run(["./GraphProgram", str(graph_file)], env=env, check=False)
    if cp.returncode != 0:
        sys.stderr.write(f"  GraphProgram stderr:\n{cp.stderr[-1000:]}\n")
        raise RuntimeError(f"GraphProgram failed for layout={layout} (exit {cp.returncode})")
    # GraphProgram writes program.o in cwd
    program_o = Path("program.o")
    if not program_o.exists():
        sys.stderr.write(f"  GraphProgram stderr:\n{cp.stderr[:1000]}\n")
        raise RuntimeError(f"GraphProgram did not produce program.o for layout={layout}")

    print(f"  [compile] runtime .o files ...")
    # Compile runtime objects (C files use gcc, C++ files use g++)
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

    print(f"  [link] final_{layout} ...")
    bin_name = workdir / f"final_{layout}"
    link_cmd = (
        [CXX_BIN, "-O3", "-mavx2", "-march=native", "-fopenmp", "-no-pie",
         "program.o", "runtime.o", "parallel_runtime.o",
         "autotuner_runtime.o", "graph_mutation_runtime.o",
         "roaring_bitmap.o", "graph_loader_runtime.o", "graph_runtime.o",
         "-lnlopt", "-o", str(bin_name)]
    )
    run(link_cmd)

    # Move artifacts to workdir
    for f in Path().glob("*.o"):
        shutil.move(str(f), str(workdir / f.name))
    return bin_name


# ---------------------------------------------------------------------------
# Run one layout and parse
# ---------------------------------------------------------------------------
def run_layout(binary, timeout_sec=BENCH_TIMEOUT_SEC):
    """
    Run compiled binary, capture stderr, parse profile data.
    Returns (perf_dict, status_string).
    """
    try:
        cp = subprocess.run(
            [str(binary)], text=True, capture_output=True,
            timeout=timeout_sec, env={**os.environ}
        )
        perf = parse_profile(cp.stderr or "")
        if cp.returncode != 0:
            # Binary crashed or exited with error
            detail = ""
            if cp.stderr:
                detail = cp.stderr.strip()[-200:]
            return perf, f"ERROR (exit={cp.returncode}): {detail}"
        return perf, "OK"
    except subprocess.TimeoutExpired:
        return {"regions": [], "totals": {}}, f"TIMEOUT (>{timeout_sec}s)"


# ---------------------------------------------------------------------------
# Formatting
# ---------------------------------------------------------------------------
NS_TO_MS = 1.0 / 1_000_000.0


def fmt_ms(ns_or_ms, is_ms=False):
    if is_ms:
        v = ns_or_ms
    else:
        v = ns_or_ms * NS_TO_MS
    if abs(v) < 0.001:
        return "         -"
    elif abs(v) < 1.0:
        return "%10.4f" % v
    elif abs(v) < 1000.0:
        return "%10.2f" % v
    else:
        return "%10.0f" % v


def print_table(results, title, key, use_kernel=False):
    """
    Print a table: rows=layouts, columns=[pred_ms, meas_ms, (kernel_ms)].
    results: dict layout -> parsed profile.
    key: 'Traverse', 'Insert', or 'Query'.
    """
    has_data = any(
        r["totals"].get(key, {}).get("meas_ns", 0) > 0
        for r in results.values()
    )
    if not has_data:
        return

    print(f"\n  {title}")
    col_count = 3 if use_kernel else 2
    hdr = "  {:<6s} | {:>10s} | {:>10s}".format("Layout", "Pred (ms)", "Meas (ms)")
    if use_kernel:
        hdr += " | {:>10s}".format("Kernel(ms)")
    print("  " + "-" * (14 + col_count * 13))
    print(hdr)
    print("  " + "-" * (14 + col_count * 13))

    for layout in LAYOUTS:
        perf = results.get(layout, {})
        t = perf.get("totals", {}).get(key, {})
        pred = t.get("pred_ns", 0) * NS_TO_MS
        meas = t.get("meas_ns", 0) * NS_TO_MS
        kern = t.get("kernel_ns", 0) * NS_TO_MS

        if meas == 0 and pred == 0:
            continue

        line = "  {:<6s} | {:>10s} | {:>10s}".format(layout, fmt_ms(pred, True), fmt_ms(meas, True))
        if use_kernel:
            line += " | {:>10s}".format(fmt_ms(kern, True))
        print(line)

    # Best layout annotation
    layouts_with = [l for l in LAYOUTS if l in results]
    if not layouts_with:
        return
    best_pred = min(
        layouts_with,
        key=lambda l: results[l]["totals"].get(key, {}).get("pred_ns", float("inf"))
    )
    best_meas = min(
        layouts_with,
        key=lambda l: results[l]["totals"].get(key, {}).get("meas_ns", float("inf"))
    )
    match = "MATCH" if best_pred == best_meas else "MISMATCH"
    print(f"  Predicted best: {best_pred}   Measured best: {best_meas}   [{match}]")


def print_per_op_table(results, title, key):
    """Print per-operation unit cost (total / visits)."""
    has_data = False
    rows = []
    for layout in LAYOUTS:
        perf = results.get(layout, {})
        for r in perf.get("regions", []):
            if r["kind"] == key:
                visits = r["visits"]
                if visits > 0:
                    rows.append((layout, r["pred_ns"] / visits, r["meas_ns"] / visits))
                    has_data = True

    if not has_data:
        return

    print(f"\n  {title} — per-operation cost (ns)")
    print("  {:<6s} | {:>12s} | {:>12s} | {:>12s}".format("Layout", "Pred/op", "Meas/op", "Pred/Meas"))
    print("  " + "-" * 50)
    for layout, pred_op, meas_op in rows:
        ratio = pred_op / meas_op if meas_op > 0 else float("inf")
        print("  {:<6s} | {:>12.1f} | {:>12.1f} | {:>11.3f}x".format(
            layout, pred_op, meas_op, ratio))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input.graph>")
        sys.exit(1)

    graph_file = Path(sys.argv[1]).resolve()
    if not graph_file.exists():
        print(f"Error: {graph_file} not found")
        sys.exit(1)

    os.chdir(SCRIPT_DIR)

    # --- Pre-flight: check that graph data files are reachable ---
    _check_graph_data_files(graph_file)

    # 1. Build GraphProgram (once)
    if not SKIP_BUILD:
        build_graphprogram()
    elif not Path("./GraphProgram").exists():
        print("SKIP_BUILD=1 but GraphProgram not found. Building anyway.")
        build_graphprogram()

    # 2. Ensure hardware calibration exists
    calib_dir = Path.home() / ".config" / "sgpl"
    calib_dir.mkdir(parents=True, exist_ok=True)
    calib_file = calib_dir / "hw_calib.json"
    if not calib_file.exists():
        run(["./hw_calib_bench"], stdout=open(str(calib_file), "w"))

    # 3. For each layout: compile → link → run
    workdir = Path(tempfile.mkdtemp(prefix="bench_layouts_"))
    print(f"[workdir] {workdir}")

    results = {}
    statuses = {}
    for layout in LAYOUTS:
        print(f"\n{'='*60}")
        print(f"  Layout: {layout}")
        print(f"{'='*60}")
        try:
            binary = compile_for_layout(graph_file, layout, workdir)
            perf, status = run_layout(binary)
            results[layout] = perf
            statuses[layout] = status
            print(f"  Status: {status}")
        except Exception as e:
            print(f"  FAILED: {e}")
            results[layout] = {"regions": [], "totals": {}}
            statuses[layout] = f"BUILD FAILED: {e}"

    # 4. Print comparison tables
    print("\n" + "=" * 70)
    print("  RESULTS: Predicted vs Measured across all layouts")
    print("  Input:", graph_file)
    print("=" * 70)

    # Print per-layout status summary
    print("\n  Per-layout status:")
    for layout in LAYOUTS:
        label = statuses.get(layout, "UNKNOWN")
        has_profile = any(
            v > 0
            for k in ["Traverse", "Insert", "Query"]
            for v in [results.get(layout, {}).get("totals", {}).get(k, {}).get("meas_ns", 0)]
        )
        if has_profile:
            label += " (has profile data)"
        else:
            label += " (no profile data)"
        print(f"    {layout:<6s}  {label}")

    print_table(results, "TRAVERSE", "Traverse", use_kernel=True)
    print_table(results, "INSERT", "Insert", use_kernel=True)
    print_table(results, "QUERY", "Query", use_kernel=False)

    print_per_op_table(results, "INSERT", "Insert")
    print_per_op_table(results, "TRAVERSE", "Traverse")

    # 5. Summary
    print("\n  SUMMARY: Best layout per operation type")
    print("  {:<12s} | {:>12s} | {:>12s} | {:>12s}".format("Op", "Pred Best", "Meas Best", "Verdict"))
    print("  " + "-" * 55)
    for key in ["Traverse", "Insert", "Query"]:
        has = any(
            results.get(l, {}).get("totals", {}).get(key, {}).get("meas_ns", 0) > 0
            for l in LAYOUTS
        )
        if not has:
            continue
        layouts_with_data = [
            l for l in LAYOUTS
            if results.get(l, {}).get("totals", {}).get(key, {}).get("meas_ns", 0) > 0
        ]
        if not layouts_with_data:
            continue
        best_pred = min(layouts_with_data,
                        key=lambda l: results[l]["totals"][key]["pred_ns"])
        best_meas = min(layouts_with_data,
                        key=lambda l: results[l]["totals"][key]["meas_ns"])
        verdict = "OK" if best_pred == best_meas else "MISMATCH"
        print("  {:<12s} | {:>12s} | {:>12s} | {:>12s}".format(key, best_pred, best_meas, verdict))

    # Cleanup
    if not KEEP:
        shutil.rmtree(workdir)
        print(f"\n[cleanup] removed {workdir}")
    else:
        print(f"\n[keep] binaries in {workdir}")


def _check_graph_data_files(graph_file):
    """Warn if relative data files referenced by the graph are not reachable
    from SCRIPT_DIR."""
    import re as _re
    try:
        text = graph_file.read_text()
    except Exception:
        return
    # Look for "file" paths in the graph DSL:  edges: file "path";
    for m in _re.finditer(r'file\s+"([^"]+)"', text):
        data_rel = m.group(1)
        # Try to resolve relative to the graph file's directory first
        candidate = (graph_file.parent / data_rel).resolve()
        if candidate.exists():
            continue
        # Try relative to SCRIPT_DIR
        candidate = (SCRIPT_DIR / data_rel).resolve()
        if candidate.exists():
            continue
        # Try with graph_file's parent as base
        if not data_rel.startswith("/"):
            print(f"  [warn] Data file not found: {data_rel}")
            print(f"         (looked in: {(graph_file.parent / data_rel).resolve()})")
            print(f"         (looked in: {(SCRIPT_DIR / data_rel).resolve()})")
            print(f"         The benchmarked binary may fail at runtime.")


if __name__ == "__main__":
    main()
