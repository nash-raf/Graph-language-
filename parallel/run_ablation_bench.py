#!/usr/bin/env python3
"""
Run ablation benchmark for multi_switch_complex profile and produce ablation_results.csv.
Reproduces the exact format: Graph,Profile,Seeds,RunsPerSeed,Time_NoAuto_ms,Time_Auto_ms,...
"""
import csv
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SYNTH = ROOT.parent / "synth_graphs"
WORKLOAD_DIR = ROOT / "ablation_workloads"
OUT_CSV = ROOT / "ablation_results.csv"

LLVM_CONFIG = os.environ.get("LLVM_CONFIG", "llvm-config-20")
CLANG = os.environ.get("CLANG", "clang-20")
CLANGXX = os.environ.get("CLANGXX", "clang++-20")

TIME_RE = re.compile(r"TIME_SEC=([0-9.]+)\s+RSS_KB=([0-9]+)")
SW_RE = re.compile(r"\[AutoTuner\] Injected\s+([0-9]+)\s+layout conversions total")

GRAPH_LABELS = [
    ("50K_edges", "synth_v_50000_e_50000.txt"),
    ("200K_edges", "synth_v_50000_e_200000.txt"),
    ("500K_edges", "synth_v_50000_e_500000.txt"),
    ("1M_edges", "synth_v_50000_e_1000000.txt"),
]
SEEDS = [11, 22, 33]
RUNS_PER_SEED = 3
PROFILE = "multi_switch_complex"


def run(cmd, cwd, capture=True, check=True):
    r = subprocess.run(cmd, cwd=cwd, text=True, capture_output=capture, check=check)
    return r


def build_graphprogram(project_dir: Path, use_auto: bool, out_bin: str):
    """Build GraphProgram with or without AutoTuner."""
    cxxflags = subprocess.check_output([LLVM_CONFIG, "--cxxflags"], text=True).strip()
    cxxflags = cxxflags.replace("-fno-exceptions", "")
    ldflags = subprocess.check_output([LLVM_CONFIG, "--ldflags"], text=True).strip()
    libs = subprocess.check_output(
        [LLVM_CONFIG, "--libs", "core", "irreader", "analysis", "passes", "executionengine", "mcjit", "native", "support"],
        text=True,
    ).strip()
    system_libs = subprocess.check_output([LLVM_CONFIG, "--system-libs"], text=True).strip()

    run([CLANG, "-S", "-emit-llvm", "-O2", "autotuner_runtime.c", "-o", "autotuner_runtime.ll"], cwd=project_dir)
    run([CLANG, "-S", "-emit-llvm", "-O2", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"], cwd=project_dir)

    main_src = "main.cpp" if use_auto else "main_noauto.cpp"
    sources = [main_src, "IRGenVisitor.cpp", "ASTBuilder.cpp", "pdg.cpp", "parallel_loop_outline.cpp", "roaring_bitmap.cpp"]
    if use_auto:
        sources.append("AutoTunerPass.cpp")

    cmd = [
        CLANGXX,
        "-O3", "-mavx2", "-march=native",
        "-fopenmp=libomp", "-L/usr/lib64", "-lomp",
        "-g", "-std=c++17", "-fexceptions",
        "-I/usr/include/antlr4-runtime", "-Igenerated", "-I.",
        *cxxflags.split(),
        "-pthread",
        *sources,
        "generated/BaseBaseVisitor.cpp", "generated/BaseLexer.cpp", "generated/BaseListener.cpp",
        "generated/BaseParser.cpp", "generated/BaseVisitor.cpp", "generated/BaseBaseListener.cpp",
        *ldflags.split(),
        "-lantlr4-runtime",
        *libs.split(),
        *system_libs.split(),
        "-o", out_bin,
    ]
    run(cmd, cwd=project_dir)


def run_one_workload(project_dir: Path, workload: Path, graphprogram_bin: str, use_auto: bool, verbose=False):
    """Compile workload, link, run, return (time_ms, rss_kb, switches)."""
    try:
        cp = subprocess.run(
            [f"./{graphprogram_bin}", str(workload)],
            cwd=project_dir,
            text=True,
            capture_output=True,
            check=True,
        )
        out = (cp.stdout or "") + "\n" + (cp.stderr or "")
        switches = int(m.group(1)) if (m := SW_RE.search(out)) else 0
    except subprocess.CalledProcessError as e:
        if verbose:
            print(f"    [fail] {graphprogram_bin} compile: exit {e.returncode}")
            if e.stderr:
                print(f"      stderr: {e.stderr[:500]}")
        return None, None, 0

    # Link program.o -> final_program
    run([CLANG, "-O3", "-c", "parallel_runtime.c", "-o", "parallel_runtime.o"], cwd=project_dir)
    link_cmd = [
        "g++", "-O3", "-mavx2", "-march=native", "-fopenmp",
        "-L/usr/lib64", "-lomp",
        "program.o", "parallel_runtime.o", "roaring_bitmap.o",
        "-o", "final_program",
    ]
    try:
        run(link_cmd, cwd=project_dir)
    except subprocess.CalledProcessError as e:
        if verbose:
            print(f"    [fail] link: {e}")
        return None, None, switches

    # Run and time
    try:
        rp = subprocess.run(
            ["/usr/bin/time", "-f", "TIME_SEC=%e RSS_KB=%M", "./final_program"],
            cwd=project_dir,
            text=True,
            capture_output=True,
            check=True,
        )
        m = TIME_RE.search(rp.stderr or "")
        if m:
            time_ms = round(float(m.group(1)) * 1000.0, 3)
            rss_kb = int(m.group(2))
            return time_ms, rss_kb, switches
    except subprocess.CalledProcessError as e:
        if verbose:
            print(f"    [fail] final_program run: exit {e.returncode} (SIGSEGV=139)")
            if e.stderr:
                for line in (e.stderr or "").strip().split("\n")[-5:]:
                    print(f"      {line}")
        pass
    return None, None, switches


def main():
    WORKLOAD_DIR.mkdir(parents=True, exist_ok=True)

    # Generate workloads
    print("[1] Generating ablation workloads...")
    run([
        sys.executable, "generate_ablation_workloads.py",
        "--out-dir", str(WORKLOAD_DIR),
        "--seeds", ",".join(map(str, SEEDS)),
        "--profile", PROFILE,
    ], cwd=ROOT)

    # Build both compilers
    print("[2] Building GraphProgram (auto) and GraphProgram_noauto...")
    if not (ROOT / "generated").exists():
        run(["antlr4", "-Dlanguage=Cpp", "-visitor", "Base.g4", "-o", "generated"], cwd=ROOT)
    build_graphprogram(ROOT, use_auto=True, out_bin="GraphProgram")
    build_graphprogram(ROOT, use_auto=False, out_bin="GraphProgram_noauto")

    print("[3] Running benchmarks...")
    rows = []
    for graph_label, edge_file in GRAPH_LABELS:
        edge_path = SYNTH / edge_file
        if not edge_path.exists():
            print(f"  [skip] missing {edge_path}")
            continue

        times_noauto = []
        times_auto = []
        rss_noauto = []
        rss_auto = []
        switches_auto = []

        for seed in SEEDS:
            wfile = WORKLOAD_DIR / f"ablation_{graph_label}_seed{seed}.graph"
            if not wfile.exists():
                print(f"  [skip] missing {wfile}")
                continue

            for run_idx in range(RUNS_PER_SEED):
                verbose = (run_idx == 0 and seed == SEEDS[0])
                t_no, r_no, _ = run_one_workload(ROOT, wfile, "GraphProgram_noauto", use_auto=False, verbose=verbose)
                t_auto, r_auto, sw = run_one_workload(ROOT, wfile, "GraphProgram", use_auto=True, verbose=verbose)
                if t_no is not None:
                    times_noauto.append(t_no)
                    rss_noauto.append(r_no or 0)
                if t_auto is not None:
                    times_auto.append(t_auto)
                    rss_auto.append(r_auto or 0)
                    switches_auto.append(sw)

        if not times_noauto or not times_auto:
            print(f"  [skip] {graph_label}: no valid runs")
            continue

        avg_noauto = sum(times_noauto) / len(times_noauto)
        avg_auto = sum(times_auto) / len(times_auto)
        delta = avg_noauto - avg_auto
        speedup = (delta / avg_noauto * 100.0) if avg_noauto > 0 else 0.0
        rss_noauto_avg = sum(rss_noauto) / len(rss_noauto) if rss_noauto else 0
        rss_auto_avg = sum(rss_auto) / len(rss_auto) if rss_auto else 0
        sw_avg = sum(switches_auto) / len(switches_auto) if switches_auto else 0

        rows.append({
            "Graph": graph_label,
            "Profile": PROFILE,
            "Seeds": len(SEEDS),
            "RunsPerSeed": RUNS_PER_SEED,
            "Time_NoAuto_ms": round(avg_noauto, 3),
            "Time_Auto_ms": round(avg_auto, 3),
            "Delta_Time_ms": round(delta, 3),
            "Speedup_pct": round(speedup, 1),
            "RSS_NoAuto_KB": int(rss_noauto_avg),
            "RSS_Auto_KB": int(rss_auto_avg),
            "Delta_RSS_KB": int(rss_auto_avg - rss_noauto_avg),
            "Switches_NoAuto_Avg": 0,
            "Switches_Auto_Avg": round(sw_avg, 1),
        })
        print(f"  {graph_label}: NoAuto={avg_noauto:.1f}ms Auto={avg_auto:.1f}ms Speedup={speedup:.1f}%")

    fieldnames = [
        "Graph", "Profile", "Seeds", "RunsPerSeed",
        "Time_NoAuto_ms", "Time_Auto_ms", "Delta_Time_ms", "Speedup_pct",
        "RSS_NoAuto_KB", "RSS_Auto_KB", "Delta_RSS_KB",
        "Switches_NoAuto_Avg", "Switches_Auto_Avg",
    ]
    with open(OUT_CSV, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        w.writerows(rows)

    print(f"[done] wrote {OUT_CSV}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
