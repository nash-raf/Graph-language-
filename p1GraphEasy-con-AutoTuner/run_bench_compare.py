#!/usr/bin/env python3
import csv
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path


ROOT = Path("/home/osamanadeem/Downloads/GPL-2026/Graph-language-")
AUTO_DIR = ROOT / "p1GraphEasy-con-AutoTuner"
BASE_DIR = ROOT / "p1GraphEasy"
SYNTH_DIR = ROOT / "synth_graphs"
TEMPLATE_DIR = AUTO_DIR / "bench_templates"
GEN_DIR = AUTO_DIR / "bench_generated"
LOG_DIR = GEN_DIR / "logs"
OUT_CSV = Path(os.environ.get("OUT_CSV", str(AUTO_DIR / "benchmark_autotuner_vs_baseline.csv")))

LLVM_CONFIG = os.environ.get("LLVM_CONFIG_BIN", "llvm-config-20")
CLANG = os.environ.get("CLANG_BIN", "clang-20")
CLANGXX = os.environ.get("CLANGXX_BIN", "clang++-20")
CXX = os.environ.get("CXX_BIN", "g++")

TIME_RE = re.compile(r"TIME_SEC=([0-9.]+)\s+RSS_KB=([0-9]+)")
SW_RE = re.compile(r"Injected\s+([0-9]+)\s+layout conversions total")


def run(cmd, cwd, log_path=None, check=True):
    if log_path is None:
        return subprocess.run(cmd, cwd=cwd, check=check, text=True, capture_output=True)
    with open(log_path, "w", encoding="utf-8") as logf:
        p = subprocess.run(cmd, cwd=cwd, check=check, text=True, stdout=logf, stderr=subprocess.STDOUT)
    return p


def llvm_flags():
    cxxflags = shlex.split(subprocess.check_output([LLVM_CONFIG, "--cxxflags"], text=True).strip())
    # keep explicit -fexceptions in command line; avoid inherited disabling
    cxxflags = [f for f in cxxflags if f != "-fno-exceptions"]
    ldflags = shlex.split(subprocess.check_output([LLVM_CONFIG, "--ldflags"], text=True).strip())
    libs = shlex.split(
        subprocess.check_output(
            [LLVM_CONFIG, "--libs", "core", "irreader", "analysis", "passes", "executionengine", "mcjit", "native", "support"],
            text=True,
        ).strip()
    )
    system_libs = shlex.split(subprocess.check_output([LLVM_CONFIG, "--system-libs"], text=True).strip())
    return cxxflags, ldflags, libs, system_libs


def build_graphprogram(project_dir: Path, autotuner: bool):
    cxxflags, ldflags, libs, system_libs = llvm_flags()

    if autotuner:
        run([CLANG, "-S", "-emit-llvm", "-O2", "autotuner_runtime.c", "-o", "autotuner_runtime.ll"], cwd=project_dir)
        run([CLANG, "-S", "-emit-llvm", "-O2", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"], cwd=project_dir)

    sources = [
        "main.cpp",
        "IRGenVisitor.cpp",
        "ASTBuilder.cpp",
        "SemanticAnalyzer.cpp",
        "pdg.cpp",
        "parallel_loop_outline.cpp",
        "roaring_bitmap.cpp",
        "generated/BaseBaseVisitor.cpp",
        "generated/BaseLexer.cpp",
        "generated/BaseListener.cpp",
        "generated/BaseParser.cpp",
        "generated/BaseVisitor.cpp",
        "generated/BaseBaseListener.cpp",
    ]
    if autotuner:
        sources.insert(4, "AutoTunerPass.cpp")

    cmd = [
        CLANGXX,
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
        "-pthread",
    ] + cxxflags + sources + ldflags + ["-lantlr4-runtime"] + libs + system_libs + ["-o", "GraphProgram"]

    print(f"[build] {'auto' if autotuner else 'base'} GraphProgram")
    run(cmd, cwd=project_dir)


def render_template(template_path: Path, edge_a: Path, edge_b: Path) -> str:
    txt = template_path.read_text(encoding="utf-8")
    txt = txt.replace("__EDGE_FILE_A__", str(edge_a))
    txt = txt.replace("__EDGE_FILE_B__", str(edge_b))
    return txt


def run_one(project_dir: Path, graph_file: Path, case_name: str, autotuner: bool):
    tag = f"{'auto' if autotuner else 'base'}_{case_name}"
    compile_log = LOG_DIR / f"{tag}_compile.log"
    run_log = LOG_DIR / f"{tag}_run.log"
    result = {
        "time_ms": None,
        "rss_kb": None,
        "switches": 0,
        "status": "ok",
        "error": "",
    }

    try:
        cp = subprocess.run(
            ["./GraphProgram", str(graph_file)],
            cwd=project_dir,
            text=True,
            capture_output=True,
            check=True,
        )
        compile_text = (cp.stdout or "") + "\n" + (cp.stderr or "")
        compile_log.write_text(compile_text, encoding="utf-8")
        if autotuner:
            m = SW_RE.search(compile_text)
            result["switches"] = int(m.group(1)) if m else 0
    except subprocess.CalledProcessError as e:
        compile_text = (e.stdout or "") + "\n" + (e.stderr or "")
        compile_log.write_text(compile_text, encoding="utf-8")
        result["status"] = "compile_fail"
        result["error"] = "GraphProgram compile failed"
        return result

    try:
        run([CLANG, "-O3", "-c", "parallel_runtime.c", "-o", "parallel_runtime.o"], cwd=project_dir)
        bin_name = f"final_program_bench_{case_name}"
        link_cmd = [
            CXX,
            "-O3",
            "-mavx2",
            "-march=native",
            "-fopenmp",
            "program.o",
            "graph_loader_runtime.cpp",
            "roaring_bitmap.cpp",
            "parallel_runtime.o",
            "-o",
            bin_name,
        ]
        run(link_cmd, cwd=project_dir)

        with open(run_log, "w", encoding="utf-8") as outf:
            rp = subprocess.run(
                ["/usr/bin/time", "-f", "TIME_SEC=%e RSS_KB=%M", f"./{bin_name}"],
                cwd=project_dir,
                text=True,
                stdout=outf,
                stderr=subprocess.PIPE,
                check=True,
            )
        m = TIME_RE.search(rp.stderr or "")
        if not m:
            result["status"] = "run_fail"
            result["error"] = "Could not parse time/rss"
            return result
        result["time_ms"] = round(float(m.group(1)) * 1000.0, 3)
        result["rss_kb"] = int(m.group(2))
    except subprocess.CalledProcessError as e:
        with open(run_log, "a", encoding="utf-8") as outf:
            outf.write("\n=== STDERR ===\n")
            outf.write(e.stderr or "")
        result["status"] = "run_fail"
        result["error"] = "Executable run failed"
    return result


def dataset_cases():
    edge_levels_raw = os.environ.get("EDGE_LEVELS", "50000,200000,500000,1000000")
    edge_levels = [int(x.strip()) for x in edge_levels_raw.split(",") if x.strip()]
    only_edge = os.environ.get("ONLY_EDGE", "").strip()
    if only_edge:
        edge_levels = [int(only_edge)]
    vertices_raw = os.environ.get("BENCH_VERTICES", "50000,1000000")
    vertices = [int(x.strip()) for x in vertices_raw.split(",") if x.strip()]
    out = []
    for v in vertices:
        for i, e in enumerate(edge_levels):
            a = SYNTH_DIR / f"synth_v_{v}_e_{e}.txt"
            b_e = edge_levels[(i + 1) % len(edge_levels)]
            b = SYNTH_DIR / f"synth_v_{v}_e_{b_e}.txt"
            out.append((v, e, a, b))
    return out


def main():
    GEN_DIR.mkdir(parents=True, exist_ok=True)
    LOG_DIR.mkdir(parents=True, exist_ok=True)

    templates = [
        ("simple", TEMPLATE_DIR / "simple_trigger.graph.tmpl"),
        ("complex_multi", TEMPLATE_DIR / "complex_multi_graph.graph.tmpl"),
        ("complex_loop", TEMPLATE_DIR / "complex_loop_graph.graph.tmpl"),
        ("mut_heavy", TEMPLATE_DIR / "mutation_heavy.graph.tmpl"),
        ("mut_heavy_inbounds", TEMPLATE_DIR / "mutation_heavy_inbounds.graph.tmpl"),
        ("pcsr_heavy", TEMPLATE_DIR / "pcsr_heavy.graph.tmpl"),
        ("mut_pcsr_set", TEMPLATE_DIR / "mutation_pcsr_set.graph.tmpl"),
        ("mut_loop_heavy", TEMPLATE_DIR / "mutation_loop_heavy.graph.tmpl"),
        ("minimal_nogain", TEMPLATE_DIR / "minimal_no_gain.graph.tmpl"),
    ]
    template_filter = os.environ.get("TEMPLATE_FILTER", "").strip()
    if template_filter:
        allowed = {x.strip() for x in template_filter.split(",") if x.strip()}
        templates = [t for t in templates if t[0] in allowed]
        if not templates:
            print(f"No templates selected by TEMPLATE_FILTER={template_filter}")
            return 1

    for _, tpath in templates:
        if not tpath.exists():
            print(f"Missing template: {tpath}")
            return 1

    skip_build = os.environ.get("SKIP_BUILD", "0").strip() == "1"
    if skip_build:
        print("[setup] SKIP_BUILD=1, reusing existing GraphProgram binaries")
    else:
        print("[setup] Building both compilers once")
        build_graphprogram(AUTO_DIR, autotuner=True)
        build_graphprogram(BASE_DIR, autotuner=False)

    rows = []
    for v, e, edge_a, edge_b in dataset_cases():
        if not edge_a.exists() or not edge_b.exists():
            print(f"[skip] missing graph file: {edge_a} or {edge_b}")
            continue
        for tname, tpath in templates:
            case_name = f"{tname}_v{v}_e{e}"
            graph_path = GEN_DIR / f"{case_name}.graph"
            graph_path.write_text(render_template(tpath, edge_a, edge_b), encoding="utf-8")

            print(f"[run] {case_name}")
            base_res = run_one(BASE_DIR, graph_path, case_name, autotuner=False)
            auto_res = run_one(AUTO_DIR, graph_path, case_name, autotuner=True)

            gain_pct = None
            rss_diff = None
            if base_res["time_ms"] is not None and auto_res["time_ms"] is not None and base_res["time_ms"] > 0:
                gain_pct = round(((base_res["time_ms"] - auto_res["time_ms"]) / base_res["time_ms"]) * 100.0, 3)
            if base_res["rss_kb"] is not None and auto_res["rss_kb"] is not None:
                rss_diff = int(auto_res["rss_kb"] - base_res["rss_kb"])

            status = "ok"
            if base_res["status"] != "ok" or auto_res["status"] != "ok":
                status = f"base:{base_res['status']} auto:{auto_res['status']}"

            rows.append(
                {
                    "Case": case_name,
                    "Template": tname,
                    "Vertices": v,
                    "Edges": e,
                    "GraphA": str(edge_a),
                    "GraphB": str(edge_b),
                    "Time_NoAuto_ms": base_res["time_ms"],
                    "Time_Auto_ms": auto_res["time_ms"],
                    "Gain_pct": gain_pct,
                    "RSS_NoAuto_KB": base_res["rss_kb"],
                    "RSS_Auto_KB": auto_res["rss_kb"],
                    "RSS_Diff_KB": rss_diff,
                    "Switches_Auto": auto_res["switches"],
                    "Status": status,
                }
            )

    fieldnames = [
        "Case",
        "Template",
        "Vertices",
        "Edges",
        "GraphA",
        "GraphB",
        "Time_NoAuto_ms",
        "Time_Auto_ms",
        "Gain_pct",
        "RSS_NoAuto_KB",
        "RSS_Auto_KB",
        "RSS_Diff_KB",
        "Switches_Auto",
        "Status",
    ]
    with open(OUT_CSV, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        w.writerows(rows)

    print(f"[done] wrote {OUT_CSV}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
