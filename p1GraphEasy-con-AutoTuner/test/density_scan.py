#!/usr/bin/env python3
"""
density_scan.py — Sweep graph density for a fixed n, measuring insert cost
under PCSR and SET.

Usage:
  ./density_scan.py -n 2000 --n-inserts 200 --n-points 20 -o crossover_n2000.png
  ./density_scan.py -n 2240 --n-inserts 200 --n-points 20 -o crossover_n2240.png
"""

import argparse
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import math
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent.parent
SKIP_BUILD = os.environ.get("SKIP_BUILD", "0") == "1"
KEEP_BINARIES = os.environ.get("KEEP_BINARIES", "0") == "1"
DEFAULT_LAYOUTS = ["PCSR", "SET"]

# Profile parsing regexes
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
# Build GraphProgram (once)
# ---------------------------------------------------------------------------
def build_graphprogram():
    cxxflags = shlex.split(
        subprocess.check_output(["llvm-config-20", "--cxxflags"], text=True).strip()
    )
    cxxflags = [f for f in cxxflags if f != "-fno-exceptions"]
    ldflags = shlex.split(
        subprocess.check_output(["llvm-config-20", "--ldflags"], text=True).strip()
    )
    libs = shlex.split(
        subprocess.check_output(
            ["llvm-config-20", "--libs", "core", "irreader", "analysis",
             "passes", "executionengine", "mcjit", "native", "support"],
            text=True,
        ).strip()
    )
    system_libs = shlex.split(
        subprocess.check_output(["llvm-config-20", "--system-libs"], text=True).strip()
    )

    # Emit runtime LLVM IR
    subprocess.run(["clang-20", "-S", "-emit-llvm", "-O2",
                     "autotuner_runtime.c", "-o", "autotuner_runtime.ll"], check=True)
    subprocess.run(["clang-20", "-S", "-emit-llvm", "-O2",
                     "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"], check=True)
    for src in ["bfs_runtime.cpp", "bfs_runtime_src.cpp", "dfs_runtime.cpp", "dfs_runtime_src.cpp"]:
        subprocess.run(["clang-20", "-x", "c++", "-S", "-emit-llvm", "-O2",
                         src, "-o", Path(src).stem + ".ll"], check=True)

    sources = [
        "main.cpp", "IRGenVisitor.cpp", "ASTBuilder.cpp", "pdg.cpp",
        "parallel_loop_outline.cpp", "SemanticAnalyzer.cpp",
        "roaring_bitmap.cpp", "AutoTunerPass.cpp",
        "generated/BaseBaseVisitor.cpp", "generated/BaseLexer.cpp",
        "generated/BaseListener.cpp", "generated/BaseParser.cpp",
        "generated/BaseVisitor.cpp", "generated/BaseBaseListener.cpp",
    ]
    cmd = (
        ["g++", "-O3", "-mavx2", "-march=native", "-g",
         "-std=c++17", "-fexceptions",
         "-I/usr/include/antlr4-runtime", "-Igenerated", "-I.",
         "-pthread"]
        + cxxflags + sources + ldflags
        + ["-lantlr4-runtime"] + libs + system_libs
        + ["-o", "GraphProgram"]
    )
    print("[build] GraphProgram ...")
    subprocess.run(cmd, check=True)
    print("[build] done.")


def ensure_hw_calib():
    calib_dir = Path.home() / ".config" / "sgpl"
    calib_dir.mkdir(parents=True, exist_ok=True)
    calib_file = calib_dir / "hw_calib.json"
    if not calib_file.exists():
        subprocess.run(["./hw_calib_bench"], stdout=open(calib_file, "w"), check=True)


# ---------------------------------------------------------------------------
# Helper: run a command, return (returncode, stdout, stderr)
# ---------------------------------------------------------------------------
def run_cmd(cmd, cwd=None, env=None, timeout=60):
    merged = {**os.environ, **(env or {})}
    try:
        cp = subprocess.run(
            cmd, cwd=cwd or SCRIPT_DIR, env=merged,
            capture_output=True, text=True, timeout=timeout
        )
        return cp.returncode, cp.stdout, cp.stderr
    except subprocess.TimeoutExpired:
        return -1, "", "TIMEOUT"


# ---------------------------------------------------------------------------
# Generate edge file for (n, m) — efficient for large graphs
# ---------------------------------------------------------------------------
def generate_edge_file(work_dir, n, m, seed=42):
    import random
    random.seed(seed)
    max_possible = n * (n - 1) // 2
    m = min(m, max_possible)
    path = work_dir / f"graph_n{n}_m{m}.txt"
    if path.exists():
        return path

    two_n_minus_1 = 2 * n - 1

    def idx_to_pair(idx):
        d = two_n_minus_1 * two_n_minus_1 - 8 * idx
        u = (two_n_minus_1 - math.isqrt(d)) // 2
        term = u * (two_n_minus_1 - u) // 2
        while term > idx and u > 0:
            u -= 1
            term = u * (two_n_minus_1 - u) // 2
        while True:
            nxt = (u + 1) * (two_n_minus_1 - u - 1) // 2
            if nxt > idx or u + 1 >= n:
                break
            u += 1
            term = nxt
        v = idx - term + u + 1
        return (u, v)

    if m <= max_possible // 2:
        indices = sorted(random.sample(range(max_possible), m))
        with open(path, "w") as f:
            for idx in indices:
                u, v = idx_to_pair(idx)
                f.write(f"{u} {v}\n")
    else:
        missing = max_possible - m
        excluded = set(random.sample(range(max_possible), missing))
        with open(path, "w") as f:
            for idx in range(max_possible):
                if idx not in excluded:
                    u, v = idx_to_pair(idx)
                    f.write(f"{u} {v}\n")
    return path


# ---------------------------------------------------------------------------
# Generate .graph program: load graph + insert loop
# ---------------------------------------------------------------------------
def generate_graph_program(work_dir, edge_file_rel, n, n_inserts=200):
    lines = [
        "graph G {",
        f'  edges: file "{edge_file_rel}";',
        "};",
        "",
        "int n = numVertices(G);",
        "int i = 0;",
        f"while (i < {n_inserts}) {{",
        "  int src = i % n;",
        "  int dst = (i + 1) % n;",
        "  add src->dst to G;",
        "  i = i + 1;",
        "}",
        "",
        'print numEdges(G);',
    ]
    path = work_dir / f"prog_n{n}.graph"
    with open(path, "w") as f:
        f.write("\n".join(lines) + "\n")
    return path


# ---------------------------------------------------------------------------
# Parse profile data from stderr
# ---------------------------------------------------------------------------
def parse_profile(stderr_text):
    totals = {}
    for line in stderr_text.splitlines():
        m = TOTAL_RE.search(line)
        if m:
            kind = m.group(1)
            totals[kind] = {
                "pred_ns": float(m.group(2)),
                "meas_ns": int(m.group(3)),
                "pure_kernel_ns": int(m.group(6)),
            }
    return totals


# ---------------------------------------------------------------------------
# Compile + run for one layout, return (pred_ns_total, meas_ns_total, visits)
# ---------------------------------------------------------------------------
def compile_and_run(layout, graph_file, work_dir, timeout=120, n_inserts=200):
    print(f"    [{layout}] compile ...", end=" ", flush=True)

    # Compile with forced layout
    env = {"AUTOTUNER_FORCE_LAYOUT": layout}
    ret, stdout, stderr = run_cmd(["./GraphProgram", str(graph_file)], env=env, timeout=timeout)
    if ret != 0:
        print(f"FAILED (GraphProgram exit={ret})")
        # Try to show the error
        if stderr:
            sys.stderr.write(f"      stderr: {stderr[-300:]}\n")
        return None

    # Build runtime .o files (only if not already built)
    runtime_o_dir = work_dir / "runtime_o"
    runtime_o_dir.mkdir(exist_ok=True)
    runtime_o_files = [
        "autotuner_runtime.o", "graph_mutation_runtime.o", "parallel_runtime.o",
        "runtime.o", "roaring_bitmap.o", "graph_loader_runtime.o", "graph_runtime.o",
    ]
    if not all((runtime_o_dir / f).exists() for f in runtime_o_files):
        runtime_cmds = [
            ["gcc", "-O3", "-c", "autotuner_runtime.c", "-o", str(runtime_o_dir / "autotuner_runtime.o")],
            ["gcc", "-O3", "-c", "graph_mutation_runtime.c", "-o", str(runtime_o_dir / "graph_mutation_runtime.o")],
            ["gcc", "-O3", "-c", "parallel_runtime.c", "-o", str(runtime_o_dir / "parallel_runtime.o")],
            ["gcc", "-O3", "-c", "runtime.c", "-o", str(runtime_o_dir / "runtime.o")],
            ["g++", "-O3", "-mavx2", "-march=native", "-fopenmp", "-c", "roaring_bitmap.cpp", "-o", str(runtime_o_dir / "roaring_bitmap.o")],
            ["g++", "-O2", "-std=c++17", "-fopenmp", "-c", "graph_loader_runtime.cpp", "-o", str(runtime_o_dir / "graph_loader_runtime.o")],
            ["g++", "-O2", "-std=c++17", "-c", "graph_runtime.cpp", "-o", str(runtime_o_dir / "graph_runtime.o")],
        ]
        for cmd in runtime_cmds:
            subprocess.run(cmd, cwd=SCRIPT_DIR, check=True, capture_output=True)

    # Move program.o to work_dir
    prog_o = SCRIPT_DIR / "program.o"
    if prog_o.exists():
        shutil.move(str(prog_o), str(work_dir / "program.o"))

    # Link
    bin_name = work_dir / f"final_{layout}"
    link_cmd = [
        "g++", "-O3", "-mavx2", "-march=native", "-fopenmp", "-no-pie",
        str(work_dir / "program.o"),
        str(runtime_o_dir / "runtime.o"),
        str(runtime_o_dir / "parallel_runtime.o"),
        str(runtime_o_dir / "autotuner_runtime.o"),
        str(runtime_o_dir / "graph_mutation_runtime.o"),
        str(runtime_o_dir / "roaring_bitmap.o"),
        str(runtime_o_dir / "graph_loader_runtime.o"),
        str(runtime_o_dir / "graph_runtime.o"),
        "-lnlopt", "-o", str(bin_name),
    ]
    subprocess.run(link_cmd, cwd=SCRIPT_DIR, check=True, capture_output=True)

    # Run
    print(f"run ...", end=" ", flush=True)
    ret, stdout, stderr = run_cmd([str(bin_name)], env={}, timeout=timeout)
    if ret != 0:
        print(f"FAILED (exit={ret})")
        if stderr:
            sys.stderr.write(f"      stderr: {stderr[-300:]}\n")
        return None

    # Parse profile
    totals = parse_profile(stderr)
    insert_totals = totals.get("Insert", {})
    if not insert_totals:
        print("NO PROFILE DATA")
        # Debug: show raw stderr
        stderr_lines = stderr.strip().splitlines()
        if stderr_lines:
            for line in stderr_lines[-5:]:
                print(f"      stderr: {line[:150]}")
        return None

    pred_ns = insert_totals.get("pred_ns", 0)
    meas_ns = insert_totals.get("pure_kernel_ns", 0)

    print(f"pred={pred_ns:.0f}ns meas={meas_ns}ns")
    return {"pred_ns": pred_ns, "meas_ns": meas_ns, "visits": n_inserts}


# ---------------------------------------------------------------------------
# Main scan loop
# ---------------------------------------------------------------------------
def scan_density(n, max_edges, work_dir, layouts, n_points, n_inserts, timeout):
    """Scan density from sparse to dense, return list of results dicts."""
    log_min = math.log10(max(4, n))
    log_max = math.log10(max_edges)
    m_values = set()
    m_values.add(max(3, n // 4))
    for i in range(n_points):
        m = int(10 ** (log_min + (log_max - log_min) * i / (n_points - 1)))
        m = min(m, max_edges)
        m = max(m, 1)
        m_values.add(m)
    m_values = sorted(m_values)

    results = []

    for m in m_values:
        print(f"\n  n={n}, m={m} (d={2*m/n:.2f})")

        edge_path = generate_edge_file(work_dir, n, m)
        graph_path = generate_graph_program(work_dir, str(edge_path.resolve()), n, n_inserts)

        point_data = {"n": n, "m": m, "density": 2 * m / n, "layouts": {}}
        all_ok = True
        for layout in layouts:
            data = compile_and_run(layout, graph_path, work_dir, timeout=timeout, n_inserts=n_inserts)
            if data is None:
                all_ok = False
                break
            point_data["layouts"][layout] = data

        if all_ok:
            best_pred = min(layouts, key=lambda l: point_data["layouts"][l]["pred_ns"])
            best_meas = min(layouts, key=lambda l: point_data["layouts"][l]["meas_ns"])
            point_data["best_pred"] = best_pred
            point_data["best_meas"] = best_meas
            results.append(point_data)
            print(f"    Best: predicted={best_pred}, measured={best_meas}")

    return results


# ---------------------------------------------------------------------------
# Plotting — overlay PCSR + SET predicted (dashed) + measured (solid) on one axis
# ---------------------------------------------------------------------------
def plot_results(results, n, output_path, layouts):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("ERROR: matplotlib not installed. Install with: pip install matplotlib")
        sys.exit(1)

    colors = {"PCSR": "#348ABD", "SET": "#988ED5"}
    markers = {"PCSR": "s", "SET": "D"}

    fig, ax = plt.subplots(figsize=(10, 7))

    m_vals = [r["m"] for r in results]

    # Find crossover: first point where best layout changes
    crossover_pred = None
    crossover_meas = None
    prev_pred = None
    prev_meas = None
    for r in results:
        if prev_pred is not None and r["best_pred"] != prev_pred and crossover_pred is None:
            crossover_pred = r["m"]
        if prev_meas is not None and r["best_meas"] != prev_meas and crossover_meas is None:
            crossover_meas = r["m"]
        prev_pred = r["best_pred"]
        prev_meas = r["best_meas"]

    for layout in layouts:
        pred_y = [r["layouts"][layout]["pred_ns"] / r["layouts"][layout]["visits"] for r in results]
        meas_y = [r["layouts"][layout]["meas_ns"] / r["layouts"][layout]["visits"] for r in results]

        ax.plot(m_vals, pred_y, color=colors[layout],
                linestyle="--", marker=markers[layout], markersize=3,
                linewidth=1.8, label=f"{layout} predicted", alpha=0.8)
        ax.plot(m_vals, meas_y, color=colors[layout],
                linestyle="-", marker=markers[layout], markersize=4,
                linewidth=2.0, label=f"{layout} measured", alpha=0.9)

    # Crossover annotations
    def annotate_crossover(ax, cross_m, label, color):
        if cross_m is not None:
            ax.axvline(x=cross_m, color=color, linestyle=":", linewidth=2, alpha=0.6)
            ax.annotate(f"{label} crossover\nm={cross_m}, d={2*cross_m/n:.1f}",
                        xy=(cross_m, ax.get_ylim()[1] * 0.15),
                        fontsize=10, color=color, ha="center",
                        bbox=dict(boxstyle="round,pad=0.3", fc="white", ec=color, alpha=0.85))

    annotate_crossover(ax, crossover_pred, "Predicted", "#d62728")
    annotate_crossover(ax, crossover_meas, "Measured", "#2ca02c")

    # Legend: PCSR predicted, PCSR measured, SET predicted, SET measured
    ax.set_xlabel("Edges (m)", fontsize=13)
    ax.set_ylabel("Cost per insert (ns)", fontsize=13)
    ax.set_title(f"Insert cost: predicted vs measured  n={n}", fontsize=14, fontweight="bold")
    ax.legend(fontsize=10, loc="upper left")
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.grid(True, alpha=0.3)

    calib_path = Path.home() / ".config" / "sgpl" / "hw_calib.json"
    calib_text = "no calibration"
    try:
        import json
        calib = json.loads(calib_path.read_text())
        calib_text = f"t={calib.get('t', '?'):.1f}ns  T={calib.get('T', '?'):.1f}ns  L={calib.get('L', '?')}"
    except Exception:
        pass
    fig.text(0.5, 0.01, f"HW calibration: {calib_text}", ha="center", fontsize=9, style="italic")

    fig.tight_layout(rect=[0, 0.03, 1, 0.97])
    fig.savefig(output_path, dpi=150, bbox_inches="tight")
    print(f"\n[plot] saved to {output_path}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(description="Density crossover scan")
    parser.add_argument("-n", type=int, default=2240, help="Number of vertices")
    parser.add_argument("-o", "--output", default=None, help="Output plot path")
    parser.add_argument("--build-only", action="store_true", help="Only build GraphProgram, then exit")
    parser.add_argument("--layouts", default="PCSR,SET", help="Comma-separated layouts to test")
    parser.add_argument("--n-inserts", type=int, default=200, help="Insert iterations per program")
    parser.add_argument("--n-points", type=int, default=20, help="Number of density points")
    parser.add_argument("--timeout", type=int, default=180, help="Timeout per compile+run (s)")
    args = parser.parse_args()

    n = args.n
    layouts = [x.strip().upper() for x in args.layouts.split(",")]
    output = args.output or f"crossover_n{n}.png"

    os.chdir(SCRIPT_DIR)

    if args.build_only:
        build_graphprogram()
        return

    if not SKIP_BUILD:
        build_graphprogram()
    elif not (SCRIPT_DIR / "GraphProgram").exists():
        build_graphprogram()

    ensure_hw_calib()

    max_edges = n * (n - 1) // 2

    work_dir = Path(tempfile.mkdtemp(prefix=f"density_scan_n{n}_"))
    print(f"[workdir] {work_dir}")

    try:
        results = scan_density(n, max_edges, work_dir, layouts,
                               args.n_points, args.n_inserts, args.timeout)

        if not results:
            print("ERROR: no results collected")
            sys.exit(1)

        # CSV
        col_headers = ["n", "m", "density"]
        for l in layouts:
            col_headers += [f"pred_{l.lower()}", f"meas_{l.lower()}"]
        col_headers += ["best_pred", "best_meas"]

        csv_path = work_dir / f"data_n{n}.csv"
        with open(csv_path, "w") as f:
            f.write(",".join(col_headers) + "\n")
            for r in results:
                row = [r["n"], r["m"], f"{r['density']:.4f}"]
                for l in layouts:
                    d = r["layouts"][l]
                    row.append(f"{d['pred_ns']/d['visits']:.1f}")
                    row.append(f"{d['meas_ns']/d['visits']:.1f}")
                row += [r["best_pred"], r["best_meas"]]
                f.write(",".join(str(v) for v in row) + "\n")
        print(f"[data] saved to {csv_path}")

        # Summary table
        hdr_fields = "m d pred_best meas_best"
        for l in layouts:
            hdr_fields += f" {l}_pred {l}_meas"
        print(f"  {'m':>6s} {'d':>7s}  {'pred_best':>10s} {'meas_best':>10s}", end="")
        for l in layouts:
            print(f"  {l+'_pred':>10s} {l+'_meas':>10s}", end="")
        print()
        for r in results:
            print(f"  {r['m']:6d} {r['density']:7.2f}  {r['best_pred']:>10s} {r['best_meas']:>10s}", end="")
            for l in layouts:
                d = r["layouts"][l]
                print(f"  {d['pred_ns']/d['visits']:10.1f} {d['meas_ns']/d['visits']:10.1f}", end="")
            print()

        # Plot
        try:
            plot_results(results, n, output, layouts)
        except Exception as e:
            print(f"WARNING: plotting failed: {e}")
            import traceback
            traceback.print_exc()

    finally:
        if not KEEP_BINARIES:
            shutil.rmtree(work_dir)
            print(f"[cleanup] removed {work_dir}")


if __name__ == "__main__":
    main()
