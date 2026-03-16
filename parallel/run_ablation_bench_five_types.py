#!/usr/bin/env python3
"""Run 5-way ablation benchmark and write a separate CSV + analysis markdown.

Compared implementations:
- p1GraphEasy
- p1GraphEasy-con-AutoTuner
- p2GraphEasy-con-AutoTuner
- parallel (no auto)
- parallel (auto)
"""

import csv
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PAR = ROOT / "parallel"
P1 = ROOT / "p1GraphEasy"
P1_CON = ROOT / "p1GraphEasy-con-AutoTuner"
P2_CON = ROOT / "p2GraphEasy-con-AutoTuner"
SYNTH = ROOT / "synth_graphs"
WORKLOAD_DIR = PAR / "ablation_workloads"

OUT_CSV = Path(os.environ.get("OUT_CSV", str(PAR / "ablation_results_five_types.csv")))
OUT_ANALYSIS = Path(os.environ.get("OUT_ANALYSIS", str(PAR / "ablation_results_five_types_analysis.md")))

TIME_RE = re.compile(r"TIME_SEC=([0-9.]+)\s+RSS_KB=([0-9]+)")
SW_RE = re.compile(r"\[AutoTuner\] Injected\s+([0-9]+)\s+layout conversions total")

GRAPH_LABELS = [
    ("50K_edges", "synth_v_50000_e_50000.txt"),
    ("200K_edges", "synth_v_50000_e_200000.txt"),
    ("500K_edges", "synth_v_50000_e_500000.txt"),
    ("1M_edges", "synth_v_50000_e_1000000.txt"),
]

SEEDS = [int(x.strip()) for x in os.environ.get("SEEDS", "11,22,33").split(",") if x.strip()]
RUNS_PER_SEED = int(os.environ.get("RUNS_PER_SEED", "3"))
PROFILE = os.environ.get("PROFILE", "multi_switch_complex")
SKIP_BUILD = os.environ.get("SKIP_BUILD", "1").strip() == "1"
COMPILE_TIMEOUT_SEC = int(os.environ.get("COMPILE_TIMEOUT_SEC", "120"))
RUN_TIMEOUT_SEC = int(os.environ.get("RUN_TIMEOUT_SEC", "120"))

CLANG = os.environ.get("CLANG", "clang-20")
CXX = os.environ.get("CXX", "g++")


VARIANTS = [
    {
        "label": "p1",
        "dir": P1,
        "bin": "GraphProgram",
        "autotuner": False,
        "build_auto": False,
    },
    {
        "label": "p1_con",
        "dir": P1_CON,
        "bin": "GraphProgram",
        "autotuner": True,
        "build_auto": True,
    },
    {
        "label": "p2_con",
        "dir": P2_CON,
        "bin": "GraphProgram",
        "autotuner": True,
        "build_auto": True,
    },
    {
        "label": "parallel_noauto",
        "dir": PAR,
        "bin": "GraphProgram_noauto",
        "autotuner": False,
        "build_auto": False,
    },
    {
        "label": "parallel_auto",
        "dir": PAR,
        "bin": "GraphProgram",
        "autotuner": True,
        "build_auto": True,
    },
]


def run(cmd, cwd, capture=False, check=True):
    return subprocess.run(cmd, cwd=cwd, text=True, capture_output=capture, check=check)


def build_parallel_graphprogram(use_auto: bool, out_bin: str):
    llvm_config = os.environ.get("LLVM_CONFIG", "llvm-config-20")
    clangxx = os.environ.get("CLANGXX", "clang++-20")

    cxxflags = subprocess.check_output([llvm_config, "--cxxflags"], text=True).strip().replace("-fno-exceptions", "")
    ldflags = subprocess.check_output([llvm_config, "--ldflags"], text=True).strip()
    libs = subprocess.check_output(
        [llvm_config, "--libs", "core", "irreader", "analysis", "passes", "executionengine", "mcjit", "native", "support"],
        text=True,
    ).strip()
    system_libs = subprocess.check_output([llvm_config, "--system-libs"], text=True).strip()

    run([CLANG, "-S", "-emit-llvm", "-O2", "autotuner_runtime.c", "-o", "autotuner_runtime.ll"], cwd=PAR)
    run([CLANG, "-S", "-emit-llvm", "-O2", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"], cwd=PAR)

    main_src = "main.cpp" if use_auto else "main_noauto.cpp"
    sources = [main_src, "IRGenVisitor.cpp", "ASTBuilder.cpp", "pdg.cpp", "parallel_loop_outline.cpp", "roaring_bitmap.cpp"]
    if use_auto:
        sources.append("AutoTunerPass.cpp")

    cmd = [
        clangxx,
        "-O3",
        "-mavx2",
        "-march=native",
        "-fopenmp=libomp",
        "-L/usr/lib64",
        "-lomp",
        "-g",
        "-std=c++17",
        "-fexceptions",
        "-I/usr/include/antlr4-runtime",
        "-Igenerated",
        "-I.",
        *cxxflags.split(),
        "-pthread",
        *sources,
        "generated/BaseBaseVisitor.cpp",
        "generated/BaseLexer.cpp",
        "generated/BaseListener.cpp",
        "generated/BaseParser.cpp",
        "generated/BaseVisitor.cpp",
        "generated/BaseBaseListener.cpp",
        *ldflags.split(),
        "-lantlr4-runtime",
        *libs.split(),
        *system_libs.split(),
        "-o",
        out_bin,
    ]
    run(cmd, cwd=PAR)


def ensure_binaries():
    missing = []
    for v in VARIANTS:
        path = v["dir"] / v["bin"]
        if not path.exists():
            missing.append(path)

    if not missing:
        return

    if SKIP_BUILD:
        missing_text = "\n".join(f"- {p}" for p in missing)
        raise RuntimeError(f"Missing required binaries while SKIP_BUILD=1:\n{missing_text}")

    for p in missing:
        if p.parent == PAR and p.name in {"GraphProgram", "GraphProgram_noauto"}:
            build_parallel_graphprogram(use_auto=(p.name == "GraphProgram"), out_bin=p.name)
        else:
            raise RuntimeError(
                f"Binary missing and automatic build not implemented for this project: {p}. "
                "Build it manually, or run with SKIP_BUILD=1 after building."
            )


def build_runtime_objects(project_dir: Path):
    runtime_src = "runtime.c" if (project_dir / "runtime.c").exists() else "parallel_runtime.c"
    run([CLANG, "-c", "-O2", "-mavx2", "-march=native", runtime_src, "-o", "runtime.o"], cwd=project_dir)
    if (project_dir / "graph_loader_runtime.cpp").exists():
        run(
            [CXX, "-c", "-O2", "-std=c++17", "-mavx2", "-march=native", "graph_loader_runtime.cpp", "-o", "graph_loader_runtime.o"],
            cwd=project_dir,
        )
    run(
        [CXX, "-c", "-O2", "-std=c++17", "-mavx2", "-march=native", "roaring_bitmap.cpp", "-o", "roaring_bitmap.o"],
        cwd=project_dir,
    )


def run_one(variant, workload: Path):
    project_dir = variant["dir"]
    compile_cmd = [f"./{variant['bin']}", str(workload)]

    try:
        cp = subprocess.run(
            compile_cmd,
            cwd=project_dir,
            text=True,
            capture_output=True,
            check=True,
            timeout=COMPILE_TIMEOUT_SEC,
        )
    except subprocess.CalledProcessError:
        return None, None, 0, "compile_fail"
    except subprocess.TimeoutExpired:
        return None, None, 0, "compile_timeout"

    out = (cp.stdout or "") + "\n" + (cp.stderr or "")
    switches = int(m.group(1)) if variant["autotuner"] and (m := SW_RE.search(out)) else 0

    final_name = f"final_program_{variant['label']}"
    link_inputs = ["program.o", "runtime.o", "roaring_bitmap.o"]
    if (project_dir / "graph_loader_runtime.o").exists():
        link_inputs.append("graph_loader_runtime.o")

    link_cmd = [
        CXX,
        "-O3",
        "-mavx2",
        "-march=native",
        *link_inputs,
        "-fopenmp",
        "-L/usr/lib64",
        "-lomp",
        "-no-pie",
        "-o",
        final_name,
    ]

    try:
        run(link_cmd, cwd=project_dir)
        rp = subprocess.run(
            ["/usr/bin/time", "-f", "TIME_SEC=%e RSS_KB=%M", f"./{final_name}"],
            cwd=project_dir,
            text=True,
            capture_output=True,
            check=True,
            timeout=RUN_TIMEOUT_SEC,
        )
    except subprocess.CalledProcessError:
        return None, None, switches, "run_fail"
    except subprocess.TimeoutExpired:
        return None, None, switches, "run_timeout"

    m = TIME_RE.search(rp.stderr or "")
    if not m:
        return None, None, switches, "run_fail"

    return round(float(m.group(1)) * 1000.0, 3), int(m.group(2)), switches, "ok"


def write_analysis(rows):
    ok_rows = [r for r in rows if r["Status"] == "ok"]
    labels = [v["label"] for v in VARIANTS]

    lines = ["# 5-Type Ablation Analysis", ""]
    lines.append(f"- Successful graph groups: {len(ok_rows)} / {len(rows)}")

    avg_time = {}
    for label in labels:
        vals = [r[f"Time_{label}_ms"] for r in ok_rows if r[f"Time_{label}_ms"] is not None]
        avg_time[label] = (sum(vals) / len(vals)) if vals else None

    lines.append("")
    lines.append("## Average Runtime")
    for label in labels:
        v = avg_time[label]
        lines.append(f"- {label}: {round(v, 3) if v is not None else 'n/a'} ms")

    base = avg_time.get("p1")
    lines.append("")
    lines.append("## Speedup Vs p1")
    for label in labels:
        v = avg_time[label]
        if label == "p1" or base is None or v is None or base <= 0:
            continue
        sp = ((base - v) / base) * 100.0
        lines.append(f"- {label}: {round(sp, 3)}%")

    winner_counts = {label: 0 for label in labels}
    for row in ok_rows:
        times = {label: row[f"Time_{label}_ms"] for label in labels if row[f"Time_{label}_ms"] is not None}
        if not times:
            continue
        best = min(times, key=times.get)
        winner_counts[best] += 1

    lines.append("")
    lines.append("## Per-Graph Winner Counts")
    for label in labels:
        lines.append(f"- {label}: {winner_counts[label]}")

    lines.append("")
    lines.append("## Average Switch Counts (Auto Variants)")
    for label in labels:
        if "auto" not in label and "con" not in label:
            continue
        vals = [r[f"Switches_{label}_avg"] for r in ok_rows if r[f"Switches_{label}_avg"] is not None]
        avg_sw = (sum(vals) / len(vals)) if vals else None
        lines.append(f"- {label}: {round(avg_sw, 3) if avg_sw is not None else 'n/a'}")

    OUT_ANALYSIS.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    WORKLOAD_DIR.mkdir(parents=True, exist_ok=True)

    print("[1] Generating ablation workloads...")
    run(
        [
            sys.executable,
            "generate_ablation_workloads.py",
            "--out-dir",
            str(WORKLOAD_DIR),
            "--seeds",
            ",".join(map(str, SEEDS)),
            "--profile",
            PROFILE,
        ],
        cwd=PAR,
    )

    print("[2] Ensuring required GraphProgram binaries...")
    ensure_binaries()

    print("[3] Building runtime objects per project...")
    runtime_dirs = sorted({v["dir"] for v in VARIANTS}, key=str)
    for d in runtime_dirs:
        build_runtime_objects(d)

    print("[4] Running 5-way benchmark...")
    rows = []
    for graph_label, edge_file in GRAPH_LABELS:
        edge_path = SYNTH / edge_file
        if not edge_path.exists():
            print(f"  [skip] missing {edge_path}")
            continue

        accum = {}
        for v in VARIANTS:
            label = v["label"]
            accum[label] = {
                "times": [],
                "rss": [],
                "switches": [],
                "statuses": [],
            }

        for seed in SEEDS:
            wfile = WORKLOAD_DIR / f"ablation_{graph_label}_seed{seed}.graph"
            if not wfile.exists():
                print(f"  [skip] missing {wfile}")
                continue

            for _ in range(RUNS_PER_SEED):
                for variant in VARIANTS:
                    label = variant["label"]
                    t_ms, rss_kb, sw, st = run_one(variant, wfile)
                    accum[label]["statuses"].append(st)
                    if t_ms is not None:
                        accum[label]["times"].append(t_ms)
                    if rss_kb is not None:
                        accum[label]["rss"].append(rss_kb)
                    if variant["autotuner"]:
                        accum[label]["switches"].append(sw)

        row = {
            "Graph": graph_label,
            "Profile": PROFILE,
            "Seeds": len(SEEDS),
            "RunsPerSeed": RUNS_PER_SEED,
            "Status": "ok",
        }

        for variant in VARIANTS:
            label = variant["label"]
            times = accum[label]["times"]
            rss = accum[label]["rss"]
            sws = accum[label]["switches"]
            sts = accum[label]["statuses"]

            row[f"Time_{label}_ms"] = round(sum(times) / len(times), 3) if times else None
            row[f"RSS_{label}_KB"] = int(sum(rss) / len(rss)) if rss else None
            row[f"Switches_{label}_avg"] = round(sum(sws) / len(sws), 3) if sws else 0
            if any(s != "ok" for s in sts):
                row["Status"] = "partial_fail"

        p1_t = row.get("Time_p1_ms")
        for variant in VARIANTS:
            label = variant["label"]
            t = row.get(f"Time_{label}_ms")
            if label == "p1" or p1_t is None or t is None or p1_t <= 0:
                row[f"Speedup_{label}_vs_p1_pct"] = None
            else:
                row[f"Speedup_{label}_vs_p1_pct"] = round(((p1_t - t) / p1_t) * 100.0, 3)

        rows.append(row)

        msg_parts = [f"{graph_label}"]
        for variant in VARIANTS:
            label = variant["label"]
            t = row.get(f"Time_{label}_ms")
            msg_parts.append(f"{label}={t if t is not None else 'n/a'}ms")
        print("  " + " | ".join(msg_parts))

    fields = ["Graph", "Profile", "Seeds", "RunsPerSeed", "Status"]
    for variant in VARIANTS:
        label = variant["label"]
        fields.extend(
            [
                f"Time_{label}_ms",
                f"RSS_{label}_KB",
                f"Switches_{label}_avg",
                f"Speedup_{label}_vs_p1_pct",
            ]
        )

    with OUT_CSV.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)

    write_analysis(rows)
    print(f"[done] wrote {OUT_CSV}")
    print(f"[done] wrote {OUT_ANALYSIS}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
