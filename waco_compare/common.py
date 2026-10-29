#!/usr/bin/env python3
"""Shared helpers for the WACO vs AutoTuner comparison harness."""
from __future__ import annotations

import json
import os
import shutil
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parent
P1 = REPO / "p1GraphEasy-con-AutoTuner"
RESULTS = ROOT / "results"
LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]


def nlopt_env(env: dict | None = None) -> dict:
    e = dict(os.environ if env is None else env)
    prefix = P1 / ".deps" / "nlopt"
    lib = prefix / "lib64"
    if not lib.is_dir():
        lib = prefix / "lib"
    e["LD_LIBRARY_PATH"] = f"{lib}:{e.get('LD_LIBRARY_PATH', '')}"
    e.setdefault("GRAPH_DISABLE_POLLY", "1")
    e.setdefault("FORCE_CPU", "1")  # never emit GPU/PTX IR during comparison builds
    e.setdefault("OMP_NUM_THREADS", "8")
    e.setdefault("SGPL_NUM_THREADS", "8")
    e.setdefault("SGPL_GPU_BACKEND", "0")
    return e


def ensure_hw_calib() -> None:
    dest = Path.home() / ".config" / "sgpl" / "hw_calib.json"
    dest.parent.mkdir(parents=True, exist_ok=True)
    if dest.is_file():
        return
    bench = P1 / "hw_calib_bench"
    if not bench.is_file():
        subprocess.check_call(
            ["gcc", "-O2", "-o", str(bench), str(P1 / "hw_calib_bench.c"), "-lm"],
            cwd=P1,
        )
    out = subprocess.check_output([str(bench)], cwd=P1)
    dest.write_bytes(out)


def compile_graph(
    graph_path: Path,
    work_dir: Path,
    *,
    force_layout: str | None = None,
    layout_plan: str | None = None,
    prediction_dump: Path | None = None,
    timeout: int = 300,
) -> Path:
    """Compile graph_path into work_dir/final_program using p1's toolchain."""
    ensure_hw_calib()
    work_dir.mkdir(parents=True, exist_ok=True)
    env = nlopt_env()
    if force_layout:
        env["AUTOTUNER_FORCE_LAYOUT"] = force_layout
    if layout_plan:
        env["AUTOTUNER_LAYOUT_PLAN"] = layout_plan
    if prediction_dump is not None:
        if prediction_dump.exists():
            prediction_dump.unlink()
        env["AUTOTUNER_PREDICTION_DUMP"] = str(prediction_dump)

    # Compile IR
    cmd = [str(P1 / "GraphProgram"), str(graph_path)]
    r = subprocess.run(
        cmd,
        cwd=P1,
        env=env,
        stdin=subprocess.DEVNULL,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    if r.returncode != 0:
        (work_dir / "compile.err").write_text(r.stderr + "\n" + r.stdout)
        raise RuntimeError(f"GraphProgram failed for {graph_path}: {r.stderr[-500:]}")

    # Move program.o into work_dir and link against shared runtime objects in P1.
    prog_o = work_dir / "program.o"
    shutil.copy2(P1 / "program.o", prog_o)

    # Ensure runtime objects exist
    _ensure_runtime_objs()

    prefix = P1 / ".deps" / "nlopt"
    lib = prefix / "lib64" if (prefix / "lib64").is_dir() else prefix / "lib"
    objs = [
        prog_o,
        P1 / "runtime.o",
        P1 / "parallel_runtime.o",
        P1 / "gpu_runtime.o",
        P1 / "autotuner_runtime.o",
        P1 / "graph_mutation_runtime.o",
        P1 / "roaring_bitmap.o",
        P1 / "graph_loader_runtime.o",
        P1 / "graph_runtime.o",
    ]
    final = work_dir / "final_program"
    link = [
        "g++",
        "-O3",
        "-fopenmp",
        "-no-pie",
        *[str(o) for o in objs],
        "-ldl",
        f"-L{lib}",
        f"-Wl,-rpath,{lib}",
        "-lnlopt",
        "-o",
        str(final),
    ]
    subprocess.check_call(link, cwd=work_dir, env=env)
    return final


def _ensure_runtime_objs() -> None:
    prefix = P1 / ".deps" / "nlopt"
    inc = prefix / "include"
    lib = prefix / "lib64" if (prefix / "lib64").is_dir() else prefix / "lib"
    steps = [
        (["gcc", "-O3", "-c", "autotuner_runtime.c", "-o", "autotuner_runtime.o"], True),
        (["gcc", "-O3", "-c", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.o"], True),
        (
            [
                "gcc",
                "-O3",
                f"-I{inc}",
                "-c",
                "parallel_runtime.c",
                "-o",
                "parallel_runtime.o",
            ],
            True,
        ),
        (["gcc", "-O3", "-c", "gpu_runtime.c", "-o", "gpu_runtime.o"], True),
        (["gcc", "-O3", "-c", "runtime.c", "-o", "runtime.o"], True),
        (
            [
                "g++",
                "-O3",
                "-mavx2",
                "-march=native",
                "-fopenmp",
                "-c",
                "roaring_bitmap.cpp",
                "-o",
                "roaring_bitmap.o",
            ],
            True,
        ),
        (
            [
                "g++",
                "-O2",
                "-std=c++17",
                "-fopenmp",
                "-c",
                "graph_loader_runtime.cpp",
                "-o",
                "graph_loader_runtime.o",
            ],
            True,
        ),
        (["g++", "-O2", "-std=c++17", "-c", "graph_runtime.cpp", "-o", "graph_runtime.o"], True),
    ]
    for cmd, _ in steps:
        out = P1 / cmd[-1]
        src = P1 / cmd[-3]
        if out.is_file() and out.stat().st_mtime >= src.stat().st_mtime:
            continue
        subprocess.check_call(cmd, cwd=P1)


def run_program(
    binary: Path,
    *,
    profile_dump: Path | None = None,
    runs: int = 5,
    warmup: int = 1,
    timeout: int = 120,
) -> dict:
    env = nlopt_env()
    times_ms = []
    answers = []
    profiles = []
    for i in range(warmup + runs):
        if profile_dump is not None and profile_dump.exists():
            profile_dump.unlink()
        if profile_dump is not None:
            env["AUTOTUNER_PROFILE_DUMP"] = str(profile_dump)
        else:
            env.pop("AUTOTUNER_PROFILE_DUMP", None)
        t0 = time.perf_counter()
        r = subprocess.run(
            [str(binary)],
            cwd=binary.parent,
            env=env,
            stdin=subprocess.DEVNULL,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
        dt = (time.perf_counter() - t0) * 1e3
        if r.returncode != 0:
            raise RuntimeError(f"run failed: {r.stderr[-500:]}")
        if i < warmup:
            continue
        times_ms.append(dt)
        answers.append(" ".join(r.stdout.strip().split()))
        if profile_dump is not None and profile_dump.exists():
            profiles.append(
                [json.loads(line) for line in profile_dump.read_text().splitlines() if line.strip()]
            )
    times_ms.sort()
    median = times_ms[len(times_ms) // 2]
    # Simple percentile CI
    lo = times_ms[max(0, int(0.025 * len(times_ms)))]
    hi = times_ms[min(len(times_ms) - 1, int(0.975 * len(times_ms)))]
    # Region aggregate across runs (median measured_ns per region)
    region_stats = {}
    if profiles:
        by_rid = {}
        for run in profiles:
            for row in run:
                if row.get("summary"):
                    continue
                rid = row["region_id"]
                by_rid.setdefault(rid, []).append(row)
        for rid, rows in by_rid.items():
            ms = sorted(r["measured_ns"] for r in rows)
            region_stats[rid] = {
                "kind": rows[0]["kind"],
                "layout": rows[0]["layout"],
                "measured_ns_median": ms[len(ms) // 2],
                "measured_ns_lo": ms[0],
                "measured_ns_hi": ms[-1],
                "predicted_ns": rows[0]["predicted_ns"],
                "conversion_ns": rows[0].get("conversion_ns", 0),
            }
    # Answer equality
    answer_ok = len(set(answers)) == 1
    return {
        "median_ms": median,
        "ci_lo_ms": lo,
        "ci_hi_ms": hi,
        "answer": answers[0] if answers else "",
        "answer_ok": answer_ok,
        "regions": region_stats,
        "times_ms": times_ms,
    }


def load_json(path: Path):
    return json.loads(path.read_text())


def write_jsonl(path: Path, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w") as f:
        for row in rows:
            f.write(json.dumps(row) + "\n")
