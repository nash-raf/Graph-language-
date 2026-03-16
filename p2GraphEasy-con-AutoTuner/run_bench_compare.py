#!/usr/bin/env python3
import csv
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path

ROOT = Path("/home/osamanadeem/Downloads/GPL-2026/Graph-language-")
P1 = ROOT / "p1GraphEasy"
P1_CON = ROOT / "p1GraphEasy-con-AutoTuner"
P2_CON = ROOT / "p2GraphEasy-con-AutoTuner"
SYNTH = ROOT / "synth_graphs"
TMPL = P2_CON / "bench_templates"
GEN = P2_CON / "bench_generated"
LOGS = GEN / "logs"

OUT_CSV = Path(os.environ.get("OUT_CSV", str(P2_CON / "benchmark_compare_three.csv")))
OUT_ANALYSIS = Path(os.environ.get("OUT_ANALYSIS", str(P2_CON / "benchmark_compare_three_analysis.md")))
REF_SWITCH_CSV = P2_CON / "benchmark_full_switch_gain_fixed2.csv"

LLVM_CONFIG = os.environ.get("LLVM_CONFIG_BIN", "llvm-config-20")
CLANG = os.environ.get("CLANG_BIN", "clang-20")
CLANGXX = os.environ.get("CLANGXX_BIN", "clang++-20")
CXX = os.environ.get("CXX_BIN", "g++")

TIME_RE = re.compile(r"TIME_SEC=([0-9.]+)\s+RSS_KB=([0-9]+)")
SW_CALL_RE = re.compile(r"call\s+[^\n]*@autograph_ensure_layout\s*\(")

TEMPLATES = [
    ("simple", TMPL / "simple_trigger.graph.tmpl"),
    ("query_burst_after_mut", TMPL / "query_burst_after_mut.graph.tmpl"),
    ("massive_insert_burst", TMPL / "massive_insert_burst.graph.tmpl"),
    ("set_query_mutation", TMPL / "set_query_mutation.graph.tmpl"),
    ("complex_multi", TMPL / "complex_multi_graph.graph.tmpl"),
    ("complex_loop", TMPL / "complex_loop_graph.graph.tmpl"),
    ("mut_heavy", TMPL / "mutation_heavy.graph.tmpl"),
    ("mut_heavy_inbounds", TMPL / "mutation_heavy_inbounds.graph.tmpl"),
    ("pcsr_heavy", TMPL / "pcsr_heavy.graph.tmpl"),
    ("mut_pcsr_set", TMPL / "mutation_pcsr_set.graph.tmpl"),
    ("mut_loop_heavy", TMPL / "mutation_loop_heavy.graph.tmpl"),
    ("minimal_nogain", TMPL / "minimal_no_gain.graph.tmpl"),
]


def run(cmd, cwd, check=True, capture=False):
    return subprocess.run(cmd, cwd=cwd, text=True, check=check, capture_output=capture)


def llvm_flags():
    cxxflags = shlex.split(subprocess.check_output([LLVM_CONFIG, "--cxxflags"], text=True).strip())
    cxxflags = [f for f in cxxflags if f != "-fno-exceptions"]
    ldflags = shlex.split(subprocess.check_output([LLVM_CONFIG, "--ldflags"], text=True).strip())
    libs = shlex.split(
        subprocess.check_output(
            [
                LLVM_CONFIG,
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
    syslibs = shlex.split(subprocess.check_output([LLVM_CONFIG, "--system-libs"], text=True).strip())
    return cxxflags, ldflags, libs, syslibs


def build_graphprogram(project_dir: Path, autotuner: bool):
    if project_dir == P2_CON:
        if autotuner:
            run([CLANG, "-S", "-emit-llvm", "-O2", "autotuner_runtime.c", "-o", "autotuner_runtime.ll"], project_dir)
            run([CLANG, "-S", "-emit-llvm", "-O2", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"], project_dir)
        run(["gcc", "-c", "runtime.c", "-o", "runtime.o"], project_dir)

        raw_cxxflags = shlex.split(subprocess.check_output([LLVM_CONFIG, "--cxxflags"], text=True).strip())
        cxxflags = [f for f in raw_cxxflags if f != "-fno-exceptions"]
        ldflags = shlex.split(subprocess.check_output([LLVM_CONFIG, "--ldflags"], text=True).strip())
        libs_all = shlex.split(subprocess.check_output([LLVM_CONFIG, "--libs", "all"], text=True).strip())
        syslibs = shlex.split(subprocess.check_output([LLVM_CONFIG, "--system-libs"], text=True).strip())

        gen_cpp = sorted(str(p) for p in (project_dir / "generated").glob("*.cpp"))
        cmd = [
            CXX,
            "-O3",
            "-mavx2",
            "-march=native",
            "-fopenmp",
            "-g",
            "-std=c++17",
            "-I/usr/include/antlr4-runtime",
            "-Igenerated",
            "-I.",
        ] + cxxflags + [
            "-fexceptions",
            "-pthread",
            "main.cpp",
            "IRGenVisitor.cpp",
            "ASTBuilder.cpp",
            "SemanticAnalyzer.cpp",
            "roaring_bitmap.cpp",
        ]
        if autotuner:
            cmd.append("AutoTunerPass.cpp")
        cmd += gen_cpp + [
            "runtime.o",
        ] + ldflags + [
            "-lantlr4-runtime",
            "-lPolly",
            "-lPollyISL",
            "-lisl",
        ] + libs_all + syslibs + ["-o", "GraphProgram"]

        run(cmd, project_dir)
        return

    cxxflags, ldflags, libs, syslibs = llvm_flags()

    if autotuner:
        run([CLANG, "-S", "-emit-llvm", "-O2", "autotuner_runtime.c", "-o", "autotuner_runtime.ll"], project_dir)
        run([CLANG, "-S", "-emit-llvm", "-O2", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.ll"], project_dir)

    base_sources = [
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
    sources = [s for s in base_sources if (project_dir / s).exists()]
    if autotuner:
        insert_at = min(4, len(sources))
        sources.insert(insert_at, "AutoTunerPass.cpp")

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
    ] + cxxflags + sources + ldflags + ["-lantlr4-runtime"] + libs + syslibs + ["-o", "GraphProgram"]

    run(cmd, project_dir)


def render_template(template_path: Path, edge_a: Path, edge_b: Path) -> str:
    txt = template_path.read_text(encoding="utf-8")
    txt = txt.replace("__EDGE_FILE_A__", str(edge_a))
    txt = txt.replace("__EDGE_FILE_B__", str(edge_b))
    return txt


def count_switch_calls(project_dir: Path) -> int:
    ll = project_dir / "program.ll"
    if not ll.exists():
        return 0
    text = ll.read_text(encoding="utf-8", errors="ignore")
    return len(SW_CALL_RE.findall(text))


def build_runtime_objects(project_dir: Path):
    run([CLANG, "-c", "-O2", "-mavx2", "-march=native", "runtime.c", "-o", "runtime.o"], project_dir)
    run([CXX, "-c", "-O2", "-std=c++17", "-mavx2", "-march=native", "graph_loader_runtime.cpp", "-o", "graph_loader_runtime.o"], project_dir)
    run([CXX, "-c", "-O2", "-std=c++17", "-mavx2", "-march=native", "roaring_bitmap.cpp", "-o", "roaring_bitmap.o"], project_dir)


def run_one(project_dir: Path, graph_file: Path, case_name: str, label: str, autotuner: bool):
    compile_log = LOGS / f"{label}_{case_name}_compile.log"
    run_log = LOGS / f"{label}_{case_name}_run.log"

    result = {
        "time_ms": None,
        "rss_kb": None,
        "switches": 0,
        "status": "ok",
        "error": "",
    }

    try:
        cp = run(["./GraphProgram", str(graph_file)], project_dir, capture=True)
        compile_text = (cp.stdout or "") + "\n" + (cp.stderr or "")
        compile_log.write_text(compile_text, encoding="utf-8")
        if autotuner:
            result["switches"] = count_switch_calls(project_dir)
    except subprocess.CalledProcessError as ex:
        compile_text = (ex.stdout or "") + "\n" + (ex.stderr or "")
        compile_log.write_text(compile_text, encoding="utf-8")
        result["status"] = "compile_fail"
        result["error"] = "GraphProgram failed"
        return result

    bin_name = f"final_program_{label}_{case_name}"
    try:
        run([CXX, "-mavx2", "-march=native", "program.o", "runtime.o", "roaring_bitmap.o", "graph_loader_runtime.o", "-fopenmp", "-no-pie", "-o", bin_name], project_dir)
        rp = subprocess.run(
            ["/usr/bin/time", "-f", "TIME_SEC=%e RSS_KB=%M", f"./{bin_name}"],
            cwd=project_dir,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=True,
        )
        run_log.write_text((rp.stdout or "") + "\n=== STDERR ===\n" + (rp.stderr or ""), encoding="utf-8")
        m = TIME_RE.search(rp.stderr or "")
        if not m:
            result["status"] = "run_fail"
            result["error"] = "Missing time metrics"
            return result
        result["time_ms"] = round(float(m.group(1)) * 1000.0, 3)
        result["rss_kb"] = int(m.group(2))
    except subprocess.CalledProcessError as ex:
        run_log.write_text((ex.stdout or "") + "\n=== STDERR ===\n" + (ex.stderr or ""), encoding="utf-8")
        result["status"] = "run_fail"
        result["error"] = "Executable failed"

    return result


def dataset_cases():
    edge_levels = [int(x.strip()) for x in os.environ.get("EDGE_LEVELS", "50000,200000,500000,1000000").split(",") if x.strip()]
    vertices = [int(x.strip()) for x in os.environ.get("BENCH_VERTICES", "50000,1000000").split(",") if x.strip()]
    out = []
    for v in vertices:
        for i, e in enumerate(edge_levels):
            a = SYNTH / f"synth_v_{v}_e_{e}.txt"
            b = SYNTH / f"synth_v_{v}_e_{edge_levels[(i + 1) % len(edge_levels)]}.txt"
            out.append((v, e, a, b))
    return out


def load_reference_switch_counts():
    counts = {}
    if not REF_SWITCH_CSV.exists():
        return counts
    try:
        with REF_SWITCH_CSV.open("r", encoding="utf-8", newline="") as f:
            for row in csv.DictReader(f):
                case = (row.get("Case") or "").strip()
                if not case:
                    continue
                p1c = int(float(row.get("Switches_p1_con") or 0))
                p2c = int(float(row.get("Switches_p2_con") or 0))
                counts[case] = (p1c, p2c)
    except Exception:
        return {}
    return counts


def speedup(base_ms, comp_ms):
    if base_ms is None or comp_ms is None or base_ms <= 0:
        return None
    return round(((base_ms - comp_ms) / base_ms) * 100.0, 3)


def write_analysis(rows):
    ok = [r for r in rows if r["Status"] == "ok"]
    lines = ["# Benchmark Comparison Analysis", ""]
    lines.append(f"- Successful cases: {len(ok)} / {len(rows)}")

    def avg(col):
        vals = [r[col] for r in ok if r[col] is not None]
        return round(sum(vals) / len(vals), 3) if vals else None

    a_p1 = avg("Time_p1_noauto_ms")
    a_p1c = avg("Time_p1_con_ms")
    a_p2c = avg("Time_p2_con_ms")
    lines.append(f"- Average time p1 (no autotuner): {a_p1} ms")
    lines.append(f"- Average time p1-con: {a_p1c} ms")
    lines.append(f"- Average time p2-con: {a_p2c} ms")

    if a_p1 is not None and a_p1c is not None and a_p1 > 0:
        lines.append(f"- p1-con vs p1 speedup: {round((a_p1 - a_p1c) / a_p1 * 100.0, 3)}%")
    if a_p1 is not None and a_p2c is not None and a_p1 > 0:
        lines.append(f"- p2-con vs p1 speedup: {round((a_p1 - a_p2c) / a_p1 * 100.0, 3)}%")
    if a_p1c is not None and a_p2c is not None and a_p1c > 0:
        lines.append(f"- p2-con vs p1-con speedup: {round((a_p1c - a_p2c) / a_p1c * 100.0, 3)}%")

    switched = [r for r in ok if (r["Switches_p1_con"] or 0) > 0 or (r["Switches_p2_con"] or 0) > 0]
    lines.append(f"- Cases with switches in p1-con: {sum(1 for r in ok if (r['Switches_p1_con'] or 0) > 0)}")
    lines.append(f"- Cases with switches in p2-con: {sum(1 for r in ok if (r['Switches_p2_con'] or 0) > 0)}")
    lines.append(f"- Cases with switches in either autotuned implementation: {len(switched)}")

    lines.append("")
    lines.append("## Notes")
    lines.append("- Positive speedup means faster than comparator.")
    lines.append("- Switch counts are sourced from benchmark_full_switch_gain_fixed2.csv for matching case names.")

    OUT_ANALYSIS.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    GEN.mkdir(parents=True, exist_ok=True)
    LOGS.mkdir(parents=True, exist_ok=True)

    selected = list(TEMPLATES)
    filt = os.environ.get("TEMPLATE_FILTER", "").strip()
    if filt:
        allow = {x.strip() for x in filt.split(",") if x.strip()}
        selected = [t for t in selected if t[0] in allow]
        if not selected:
            print(f"No templates selected by TEMPLATE_FILTER={filt}")
            return 1

    for _, tpath in selected:
        if not tpath.exists():
            print(f"Missing template: {tpath}")
            return 1

    skip_build = os.environ.get("SKIP_BUILD", "0").strip() == "1"
    if not skip_build:
        print("[setup] Building GraphProgram for p1, p1-con, p2-con")
        build_graphprogram(P1, autotuner=False)
        build_graphprogram(P1_CON, autotuner=True)
        build_graphprogram(P2_CON, autotuner=True)
    else:
        print("[setup] SKIP_BUILD=1, reusing existing GraphProgram binaries")

    build_runtime_objects(P1)
    build_runtime_objects(P1_CON)
    build_runtime_objects(P2_CON)
    ref_switches = load_reference_switch_counts()

    rows = []
    for v, e, edge_a, edge_b in dataset_cases():
        if not edge_a.exists() or not edge_b.exists():
            print(f"[skip] missing graph file: {edge_a} or {edge_b}")
            continue

        for tname, tpath in selected:
            case_name = f"{tname}_v{v}_e{e}"
            graph_path = GEN / f"{case_name}.graph"
            graph_path.write_text(render_template(tpath, edge_a, edge_b), encoding="utf-8")

            print(f"[run] {case_name}")
            p1 = run_one(P1, graph_path, case_name, "p1", autotuner=False)
            p1c = run_one(P1_CON, graph_path, case_name, "p1_con", autotuner=True)
            p2c = run_one(P2_CON, graph_path, case_name, "p2_con", autotuner=True)
            if case_name in ref_switches:
                p1c["switches"], p2c["switches"] = ref_switches[case_name]

            status = "ok"
            if p1["status"] != "ok" or p1c["status"] != "ok" or p2c["status"] != "ok":
                status = f"p1:{p1['status']} p1-con:{p1c['status']} p2-con:{p2c['status']}"

            rows.append(
                {
                    "Case": case_name,
                    "Template": tname,
                    "Vertices": v,
                    "Edges": e,
                    "GraphA": str(edge_a),
                    "GraphB": str(edge_b),
                    "Time_p1_noauto_ms": p1["time_ms"],
                    "Time_p1_con_ms": p1c["time_ms"],
                    "Time_p2_con_ms": p2c["time_ms"],
                    "RSS_p1_noauto_KB": p1["rss_kb"],
                    "RSS_p1_con_KB": p1c["rss_kb"],
                    "RSS_p2_con_KB": p2c["rss_kb"],
                    "Switches_p1_con": p1c["switches"],
                    "Switches_p2_con": p2c["switches"],
                    "Speedup_p1_con_vs_p1_pct": speedup(p1["time_ms"], p1c["time_ms"]),
                    "Speedup_p2_con_vs_p1_pct": speedup(p1["time_ms"], p2c["time_ms"]),
                    "Speedup_p2_con_vs_p1_con_pct": speedup(p1c["time_ms"], p2c["time_ms"]),
                    "Status": status,
                }
            )

    fields = [
        "Case",
        "Template",
        "Vertices",
        "Edges",
        "GraphA",
        "GraphB",
        "Time_p1_noauto_ms",
        "Time_p1_con_ms",
        "Time_p2_con_ms",
        "RSS_p1_noauto_KB",
        "RSS_p1_con_KB",
        "RSS_p2_con_KB",
        "Switches_p1_con",
        "Switches_p2_con",
        "Speedup_p1_con_vs_p1_pct",
        "Speedup_p2_con_vs_p1_pct",
        "Speedup_p2_con_vs_p1_con_pct",
        "Status",
    ]
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
