#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
bench_fraud_scaling.py — Scale streaming_fraud_switch across graph sizes and
compare AutoTuner (dynamic layout) wall time vs forced CSR/PCSR/BCSR/SET.

Usage:
  ./bench_fraud_scaling.py              # full ladder
  ./bench_fraud_scaling.py --sanity     # 50k only
  ./bench_fraud_scaling.py --sizes 50000,100000

Environment:
  SKIP_BUILD=1          reuse existing GraphProgram
  KEEP_FILES=1          keep generated workloads / binaries
  BENCH_REPEATS=3       timed runs per mode (default 3)
  BENCH_WARMUPS=1       warmups before timed runs
  OMP_NUM_THREADS=...   passed through to binaries
"""

from __future__ import annotations

import argparse
import csv
import os
import random
import re
import shlex
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent

CLANGXX_BIN = os.environ.get("CLANGXX_BIN", "clang++-20")
CLANG_BIN = os.environ.get("CLANG_BIN", "clang-20")
LLVM_CONFIG_BIN = os.environ.get("LLVM_CONFIG_BIN", "llvm-config-20")
CXX_BIN = os.environ.get("CXX_BIN", "g++")
SKIP_BUILD = os.environ.get("SKIP_BUILD", "0") == "1"
KEEP = os.environ.get("KEEP_FILES", "0") == "1"
REPEATS = int(os.environ.get("BENCH_REPEATS", "3"))
WARMUPS = int(os.environ.get("BENCH_WARMUPS", "1"))

MODES = ["AUTO", "CSR", "PCSR", "BCSR", "SET"]
STATIC_MODES = ["CSR", "PCSR", "BCSR", "SET"]

# (n, m) with avg degree ≈ 20
DEFAULT_SIZES = [
    (50_000, 500_000),
    (100_000, 1_000_000),
    (250_000, 2_500_000),
    (500_000, 5_000_000),
    (1_000_000, 10_000_000),
]

EXISTING_50K = SCRIPT_DIR / "synth_v_50000_e_500000.txt"
OUT_DIR = SCRIPT_DIR / "benchmark_results"
WORKLOAD_DIR = SCRIPT_DIR / "fraud_scaling_workloads"
CSV_PATH = OUT_DIR / "fraud_scaling.csv"

SCHEDULE_RE = re.compile(
    r"Injected\s+(\d+)\s+layout conversions", re.I
)


def timeout_for_n(n: int, auto_median_ms: float | None = None) -> int:
    """Wall-clock cap per launch. When AUTO time is known, cap static runs at
    ~20x AUTO (min 45s) so pathological layouts don't dominate the study."""
    if n <= 50_000:
        base = 120
    elif n <= 100_000:
        base = 180
    elif n <= 250_000:
        base = 300
    elif n <= 500_000:
        base = 450
    else:
        base = 600
    if auto_median_ms is not None and auto_median_ms == auto_median_ms and auto_median_ms > 0:
        adaptive = max(45, int((auto_median_ms / 1000.0) * 20) + 5)
        return min(base, adaptive)
    return base


def run(cmd, cwd=None, env=None, check=True, timeout=None):
    merged = {**os.environ, **(env or {})}
    try:
        return subprocess.run(
            cmd,
            cwd=cwd or SCRIPT_DIR,
            env=merged,
            check=check,
            text=True,
            capture_output=True,
            timeout=timeout,
        )
    except subprocess.CalledProcessError as e:
        sys.stderr.write(f"  [CMD FAILED] {' '.join(str(c) for c in cmd)}\n")
        if e.stderr:
            sys.stderr.write(f"  [stderr] {e.stderr[:1200].strip()}\n")
        if e.stdout:
            sys.stderr.write(f"  [stdout] {e.stdout[:400].strip()}\n")
        raise


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
            [
                LLVM_CONFIG_BIN,
                "--libs",
                "core",
                "irreader",
                "analysis",
                "passes",
                "executionengine",
                "mcjit",
                "native",
                "support",
            ],
            text=True,
        ).strip()
    )
    system_libs = shlex.split(
        subprocess.check_output(
            [LLVM_CONFIG_BIN, "--system-libs"], text=True
        ).strip()
    )
    return cxxflags, ldflags, libs, system_libs


def build_graphprogram():
    cxxflags, ldflags, libs, system_libs = llvm_flags()
    run([CLANG_BIN, "-S", "-emit-llvm", "-O2", "autotuner_runtime.c", "-o", "autotuner_runtime.ll"])
    run([CLANG_BIN, "-S", "-emit-llvm", "-O2", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2", "bfs_runtime.cpp", "-o", "bfs_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2", "bfs_runtime_src.cpp", "-o", "bfs_runtime_src.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2", "dfs_runtime.cpp", "-o", "dfs_runtime.ll"])
    run([CLANG_BIN, "-x", "c++", "-S", "-emit-llvm", "-O2", "dfs_runtime_src.cpp", "-o", "dfs_runtime_src.ll"])

    sources = [
        "main.cpp",
        "IRGenVisitor.cpp",
        "ASTBuilder.cpp",
        "pdg.cpp",
        "parallel_loop_outline.cpp",
        "SemanticAnalyzer.cpp",
        "roaring_bitmap.cpp",
        "AutoTunerPass.cpp",
        "generated/BaseBaseVisitor.cpp",
        "generated/BaseLexer.cpp",
        "generated/BaseListener.cpp",
        "generated/BaseParser.cpp",
        "generated/BaseVisitor.cpp",
        "generated/BaseBaseListener.cpp",
    ]
    cmd = (
        [
            CXX_BIN,
            "-O3",
            "-mavx2",
            "-march=native",
            "-g",
            "-std=c++17",
            "-fexceptions",
            "-I/usr/include/antlr4-runtime",
            "-Igenerated",
            "-I.",
            "-pthread",
        ]
        + cxxflags
        + sources
        + ldflags
        + ["-lantlr4-runtime"]
        + libs
        + system_libs
        + ["-o", "GraphProgram"]
    )
    print("[build] GraphProgram ...")
    run(cmd)
    print("[build] done.")


def ensure_hw_calib():
    calib_dir = Path.home() / ".config" / "sgpl"
    calib_dir.mkdir(parents=True, exist_ok=True)
    calib_file = calib_dir / "hw_calib.json"
    if calib_file.exists():
        return
    bench = SCRIPT_DIR / "hw_calib_bench"
    if not bench.exists():
        run(["gcc", "-O2", "-o", "hw_calib_bench", "hw_calib_bench.c"])
    out = run(["./hw_calib_bench"])
    calib_file.write_text(out.stdout)


def write_edgelist(path: Path, n: int, m_undirected: int, seed: int = 42) -> int:
    """Write undirected unique edges. Uses a hash-set of packed uint64 keys."""
    rng = random.Random(seed)
    edges: set[int] = set()
    attempts = 0
    limit = max(m_undirected * 40, m_undirected + 1000)
    while len(edges) < m_undirected and attempts < limit:
        u = rng.randrange(n)
        v = rng.randrange(n)
        attempts += 1
        if u == v:
            continue
        if u > v:
            u, v = v, u
        edges.add((u << 32) | v)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w") as f:
        for key in edges:
            u = key >> 32
            v = key & 0xFFFFFFFF
            f.write(f"{u} {v}\n")
    return len(edges)


def render_fraud_graph(edge_path: Path, n: int, out_path: Path) -> None:
    """OOB insert IDs stay beyond n so the switch character is size-invariant."""
    suspicious_base = n + 11000
    mule_base = n + 21000
    merchant_base = n + 31000
    # Prefer a path relative to SCRIPT_DIR (cwd when binaries run). Absolute
    # paths have been observed to segfault some forced-layout binaries.
    try:
        edge_rel = os.path.relpath(edge_path.resolve(), SCRIPT_DIR)
    except ValueError:
        edge_rel = str(edge_path.resolve())
    text = f"""// Parameterized streaming fraud switch (n={n})
// Insert vertices are deliberately out-of-bounds relative to the base graph.
graph TransactionGraph {{
  edges: file "{edge_rel}";
}};

query pre_alert_reachability: "bfs" of TransactionGraph;

int suspicious_base = {suspicious_base};
int mule_base = {mule_base};
int merchant_base = {merchant_base};
int batch = 0;

while (batch < 16) {{
  int suspicious = suspicious_base + batch;
  int mule = mule_base + batch;
  int merchant = merchant_base + batch;
  add suspicious->mule, mule->merchant, merchant->(merchant + 500) to TransactionGraph;
  batch = batch + 1;
}}

set frontier = {{{suspicious_base}, {suspicious_base + 1}, {suspicious_base + 2}, {suspicious_base + 3}}};
set visited = {{{suspicious_base}, {suspicious_base + 1}, {suspicious_base + 2}, {suspicious_base + 3}}};
int rounds = 0;

while (rounds < 3) {{
  set next = {{}};
  for each element u in frontier {{
    for each neighbor v of u in TransactionGraph {{
      if (!visited.contains(v)) {{
        visited.add(v);
        next.add(v);
      }}
    }}
  }}
  frontier = next;
  rounds = rounds + 1;
}}

query post_burst_triage: "bfs" of TransactionGraph;

print setSize(visited);
print numEdges(TransactionGraph);
"""
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(text)


def prepare_workload(n: int, m: int) -> tuple[Path, Path, int]:
    WORKLOAD_DIR.mkdir(parents=True, exist_ok=True)
    if n == 50_000 and m == 500_000 and EXISTING_50K.exists():
        edge_path = EXISTING_50K
        actual_m = m
    else:
        edge_path = WORKLOAD_DIR / f"synth_v_{n}_e_{m}.txt"
        if not edge_path.exists():
            print(f"  [gen] writing {edge_path.name} (n={n}, m={m}) ...")
            actual_m = write_edgelist(edge_path, n, m, seed=42 + n)
            print(f"  [gen] wrote {actual_m} edges")
        else:
            actual_m = m
            print(f"  [gen] reuse {edge_path.name}")
    graph_path = WORKLOAD_DIR / f"streaming_fraud_n{n}.graph"
    render_fraud_graph(edge_path, n, graph_path)
    return graph_path, edge_path, actual_m


def prebuild_cache(edge_path: Path) -> None:
    prebuild = SCRIPT_DIR / "tools" / "prebuild_graph_cache"
    src = SCRIPT_DIR / "tools" / "prebuild_graph_cache.cpp"
    if not prebuild.exists() or (src.exists() and src.stat().st_mtime > prebuild.stat().st_mtime):
        print("  [prebuild] compiling prebuild_graph_cache ...")
        run(
            [
                CXX_BIN,
                "-O2",
                "-std=c++17",
                "-fopenmp",
                "-mavx2",
                "-march=native",
                str(src),
                "graph_loader_runtime.cpp",
                "roaring_bitmap.cpp",
                "-o",
                str(prebuild),
            ]
        )
    print(f"  [prebuild] cache for {edge_path.name} ...")
    run([str(prebuild), "--refresh", str(edge_path)], timeout=max(120, timeout_for_n(10**6)))


def compile_for_mode(graph_file: Path, mode: str, workdir: Path) -> Path:
    print(f"  [compile] mode={mode} ...")
    env = {}
    if mode != "AUTO":
        env["AUTOTUNER_FORCE_LAYOUT"] = mode
    # Clear force layout for AUTO by not setting it; also strip if inherited.
    full_env = {**os.environ, **env}
    if mode == "AUTO":
        full_env.pop("AUTOTUNER_FORCE_LAYOUT", None)

    cp = run(["./GraphProgram", str(graph_file)], env=full_env, check=False)
    if cp.returncode != 0:
        sys.stderr.write(f"  GraphProgram stderr:\n{(cp.stderr or '')[-1500:]}\n")
        raise RuntimeError(f"GraphProgram failed for mode={mode}")

    program_o = Path("program.o")
    if not program_o.exists():
        raise RuntimeError(f"GraphProgram did not produce program.o for mode={mode}")

    run(["gcc", "-O3", "-c", "autotuner_runtime.c", "-o", "autotuner_runtime.o"])
    run(["gcc", "-O3", "-c", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.o"])
    run(["gcc", "-O3", "-c", "parallel_runtime.c", "-o", "parallel_runtime.o"])
    run(["gcc", "-O3", "-c", "runtime.c", "-o", "runtime.o"])
    run(
        [
            CXX_BIN,
            "-O3",
            "-mavx2",
            "-march=native",
            "-fopenmp",
            "-c",
            "roaring_bitmap.cpp",
            "-o",
            "roaring_bitmap.o",
        ]
    )
    run(
        [
            CXX_BIN,
            "-O2",
            "-std=c++17",
            "-fopenmp",
            "-c",
            "graph_loader_runtime.cpp",
            "-o",
            "graph_loader_runtime.o",
        ]
    )
    run(
        [
            CXX_BIN,
            "-O2",
            "-std=c++17",
            "-c",
            "graph_runtime.cpp",
            "-o",
            "graph_runtime.o",
        ]
    )

    bin_name = workdir / f"final_{mode}"
    run(
        [
            CXX_BIN,
            "-O3",
            "-mavx2",
            "-march=native",
            "-fopenmp",
            "-no-pie",
            "program.o",
            "runtime.o",
            "parallel_runtime.o",
            "autotuner_runtime.o",
            "graph_mutation_runtime.o",
            "roaring_bitmap.o",
            "graph_loader_runtime.o",
            "graph_runtime.o",
            "-lnlopt",
            "-o",
            str(bin_name),
        ]
    )
    # Move leftover .o out of cwd so next compile is clean.
    for f in Path().glob("*.o"):
        dest = workdir / f"{mode}_{f.name}"
        shutil.move(str(f), str(dest))
    return bin_name


def extract_schedule(graph_file: Path) -> str:
    helper = SCRIPT_DIR / "test" / "extract_autotuner_schedule.py"
    if not helper.exists():
        return ""
    try:
        cp = run([sys.executable, str(helper), str(graph_file)], check=False, timeout=120)
        out = (cp.stdout or "").strip()
        return out.splitlines()[-1] if out else ""
    except Exception as e:
        return f"(schedule extract failed: {e})"


def run_timed(binary: Path, timeout_sec: int) -> tuple[list[float], str]:
    times_ms: list[float] = []
    last_status = "OK"
    env = {**os.environ}
    env.setdefault("OMP_NUM_THREADS", str(os.cpu_count() or 4))
    env.setdefault("SGPL_ROARING_THREADS", "4")

    # Match 03_run.sh: deep recursion / large frames need a bigger stack.
    try:
        import resource

        soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
        target = 256 * 1024 * 1024  # 256 MiB
        if soft < target:
            resource.setrlimit(resource.RLIMIT_STACK, (min(target, hard), hard))
    except Exception:
        pass

    total_launches = WARMUPS + REPEATS
    for i in range(total_launches):
        try:
            t0 = time.perf_counter()
            cp = subprocess.run(
                [str(binary)],
                cwd=SCRIPT_DIR,
                env=env,
                text=True,
                capture_output=True,
                timeout=timeout_sec,
            )
            elapsed_ms = (time.perf_counter() - t0) * 1000.0
            if cp.returncode != 0:
                detail = (cp.stderr or cp.stdout or "")[-300:].strip()
                last_status = f"ERROR exit={cp.returncode}: {detail}"
                if i >= WARMUPS:
                    times_ms.append(float("nan"))
                continue
            if i >= WARMUPS:
                times_ms.append(elapsed_ms)
                print(f"    launch {i - WARMUPS + 1}/{REPEATS}: {elapsed_ms:.1f} ms", flush=True)
        except subprocess.TimeoutExpired:
            # Record timeout as a lower-bound wall time so pathological
            # static layouts still produce a speedup estimate.
            last_status = f"TIMEOUT (>{timeout_sec}s)"
            bound_ms = timeout_sec * 1000.0
            if i >= WARMUPS:
                times_ms.append(bound_ms)
                print(f"    launch {i - WARMUPS + 1}/{REPEATS}: TIMEOUT (>= {bound_ms:.0f} ms)", flush=True)
            else:
                print(f"    warmup TIMEOUT (>= {timeout_sec}s) — aborting remaining launches", flush=True)
                # Fill remaining timed slots with the timeout bound.
                while len(times_ms) < REPEATS:
                    times_ms.append(bound_ms)
                break
            # One timed timeout is enough; remaining repeats would just burn wall clock.
            while len(times_ms) < REPEATS:
                times_ms.append(bound_ms)
            break
    return times_ms, last_status


def median_or_nan(vals: list[float]) -> float:
    good = [v for v in vals if v == v]  # drop NaN
    if not good:
        return float("nan")
    return statistics.median(good)


def write_csv(rows: list[dict]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    fields = [
        "n",
        "m",
        "mode",
        "median_ms",
        "speedup_vs_auto",
        "status",
        "schedule",
        "raw_ms",
    ]
    with CSV_PATH.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        for r in rows:
            w.writerow(r)
    print(f"\n[csv] wrote {CSV_PATH}")


def bench_size(n: int, m: int, workdir: Path) -> list[dict]:
    print(f"\n{'=' * 64}")
    print(f"  SIZE n={n:,}  m={m:,}  degree≈{2 * m / n:.1f}")
    print(f"{'=' * 64}")

    graph_path, edge_path, actual_m = prepare_workload(n, m)
    prebuild_cache(edge_path)
    schedule = extract_schedule(graph_path)
    print(f"  [schedule] {schedule or '(none)'}")

    mode_medians: dict[str, float] = {}
    mode_status: dict[str, str] = {}
    mode_raw: dict[str, list[float]] = {}
    auto_ms: float | None = None

    for mode in MODES:
        timeout_sec = timeout_for_n(n, auto_ms)
        print(f"  [timeout] mode={mode} cap={timeout_sec}s", flush=True)
        try:
            binary = compile_for_mode(graph_path, mode, workdir)
            times_ms, status = run_timed(binary, timeout_sec)
            med = median_or_nan(times_ms)
            mode_medians[mode] = med
            mode_status[mode] = status
            mode_raw[mode] = times_ms
            if mode == "AUTO" and med == med:
                auto_ms = med
            print(
                f"  [{mode:<4}] median={med:10.2f} ms  status={status}  "
                f"raw={[round(t, 1) if t == t else 'nan' for t in times_ms]}"
            )
        except Exception as e:
            mode_medians[mode] = float("nan")
            mode_status[mode] = f"BUILD FAILED: {e}"
            mode_raw[mode] = []
            print(f"  [{mode:<4}] FAILED: {e}")

    auto_ms = mode_medians.get("AUTO", float("nan"))
    rows = []
    for mode in MODES:
        med = mode_medians.get(mode, float("nan"))
        if mode == "AUTO":
            speedup = 1.0 if med == med else float("nan")
        elif auto_ms == auto_ms and med == med and auto_ms > 0:
            speedup = med / auto_ms
        else:
            speedup = float("nan")
        rows.append(
            {
                "n": n,
                "m": actual_m,
                "mode": mode,
                "median_ms": f"{med:.4f}" if med == med else "",
                "speedup_vs_auto": f"{speedup:.4f}" if speedup == speedup else "",
                "status": mode_status.get(mode, ""),
                "schedule": schedule if mode == "AUTO" else "",
                "raw_ms": ";".join(
                    f"{t:.2f}" if t == t else "nan" for t in mode_raw.get(mode, [])
                ),
            }
        )
    return rows


def parse_sizes(arg: str | None, sanity: bool) -> list[tuple[int, int]]:
    if sanity:
        return [(50_000, 500_000)]
    if not arg:
        return list(DEFAULT_SIZES)
    out = []
    wanted = {int(x.strip()) for x in arg.split(",") if x.strip()}
    lookup = {n: m for n, m in DEFAULT_SIZES}
    for n in sorted(wanted):
        if n not in lookup:
            # assume degree 20
            out.append((n, n * 10))
        else:
            out.append((n, lookup[n]))
    return out


def main():
    # Unbuffered logs so long CSR runs still show progress.
    try:
        sys.stdout.reconfigure(line_buffering=True)
        sys.stderr.reconfigure(line_buffering=True)
    except Exception:
        pass

    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--sanity", action="store_true", help="Run 50k only")
    ap.add_argument("--sizes", type=str, default=None, help="Comma-separated n values")
    args = ap.parse_args()

    os.chdir(SCRIPT_DIR)
    sizes = parse_sizes(args.sizes, args.sanity)

    if not SKIP_BUILD or not Path("./GraphProgram").exists():
        if SKIP_BUILD and not Path("./GraphProgram").exists():
            print("SKIP_BUILD=1 but GraphProgram missing; building.")
        build_graphprogram()

    ensure_hw_calib()

    workdir = Path(tempfile.mkdtemp(prefix="fraud_scale_"))
    print(f"[workdir] {workdir}")
    print(f"[config] warmups={WARMUPS} repeats={REPEATS} sizes={sizes}")

    all_rows: list[dict] = []
    try:
        for n, m in sizes:
            rows = bench_size(n, m, workdir)
            all_rows.extend(rows)
            # Persist incrementally so a late OOM still leaves partial results.
            write_csv(all_rows)
    finally:
        if not KEEP:
            shutil.rmtree(workdir, ignore_errors=True)
            print(f"[cleanup] removed {workdir}")
        else:
            print(f"[keep] {workdir}")

    # Summary: speedups vs AUTO for static modes
    print("\n" + "=" * 70)
    print("  SPEEDUP vs AUTO  (T_static / T_AUTO; >1 means AutoTuner wins)")
    print("=" * 70)
    header = f"  {'n':>10} |" + "".join(f" {mode:>8}" for mode in STATIC_MODES)
    print(header)
    print("  " + "-" * (12 + 9 * len(STATIC_MODES)))
    by_n: dict[int, dict[str, float]] = {}
    for r in all_rows:
        by_n.setdefault(int(r["n"]), {})[r["mode"]] = (
            float(r["speedup_vs_auto"]) if r["speedup_vs_auto"] else float("nan")
        )
    for n in sorted(by_n):
        line = f"  {n:>10,} |"
        for mode in STATIC_MODES:
            sp = by_n[n].get(mode, float("nan"))
            line += f" {sp:8.3f}" if sp == sp else f" {'-':>8}"
        print(line)

    print(f"\nDone. Results: {CSV_PATH}")


if __name__ == "__main__":
    main()
