#!/usr/bin/env python3
"""Regression checks for native iterator layout planning; run in Linux/WSL.

Compiles the real AutoTuner pass. --frontend also executes five distinct SGPL
applications in AUTO/CSR/PCSR/BCSR/SET, using the default compiler backend.
"""
import argparse
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
PROOF = ROOT / "proof/autotuner_iterator_phases_20261015"
DECLS = """
@G = global i8 0
@frontier = global i8 0
@nodes = global i8 0
@edges = global i8 0
@bitmap.slot = global ptr null
declare void @autograph_init(ptr, i64, i64, ptr, ptr, ptr)
declare void @graph_add_edge(ptr, ptr, i32, i32, i32)
declare void @autograph_neighbor_iter_init(ptr, i64, ptr)
declare i32 @autograph_neighbor_iter_next(ptr, ptr)
declare i64 @roaring_bitmap_get_cardinality(ptr)
declare i1 @roaring_bitmap_contains(ptr, i32)
declare void @roaring_bitmap_add(ptr, i32)
declare void @roaring_bitmap_remove(ptr, i32)
declare i32 @graph_get_edge_weight(ptr, i32, i32)
declare ptr @graph_get_node_bitmap(ptr)
declare i32 @autograph_frontier_execute(ptr, ptr)
"""


def native(body="", before="", nested=False, rotated=False):
    """Update burst followed by an iterator scan with an externally used count."""
    middle = """
  br label %outer.header
outer.header:
  %u = phi i64 [0, %update.exit], [%u.next, %outer.latch]
  %more.u = icmp ult i64 %u, 544
  br i1 %more.u, label %scan.pre, label %exit, !autotuner.traverse !1
""" if nested else "  br label %scan.pre\n"
    tail = """
outer.latch:
  %u.next = add i64 %u, 1
  br label %outer.header
""" if nested else ""
    scan_exit = "outer.latch" if nested else "exit"
    # Model loop rotation: a next() can already exist in the preheader.
    first = "  %first = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %v)\n" if rotated else ""
    return DECLS + f"""
define i32 @main() {{
entry:
  %iter = alloca [64 x i8]
  %v = alloca i32
  call void @autograph_init(ptr @G, i64 544, i64 128032, ptr @nodes, ptr @edges, ptr null)
  br label %update.header
update.header:
  %i = phi i32 [0, %entry], [%inc, %update.body]
  %more.i = icmp slt i32 %i, 1000000
  br i1 %more.i, label %update.body, label %update.exit, !autotuner.trip_count !0
update.body:
  call void @graph_add_edge(ptr @G, ptr @edges, i32 0, i32 1, i32 0)
  %inc = add i32 %i, 1
  br label %update.header
update.exit:
  {before}
{middle}
scan.pre:
  call void @autograph_neighbor_iter_init(ptr @G, i64 0, ptr %iter)
{first}  br label %scan.header
scan.header:
  %has = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %v)
  %more = icmp ne i32 %has, 0
  br i1 %more, label %scan.body, label %{scan_exit}, !autotuner.traverse !2
scan.body:
  {body}
  br label %scan.header
{tail}
exit:
  ret i32 0
}}
!0 = !{{i64 1000000}}
!1 = !{{!"vertex"}}
!2 = !{{!"neighbor"}}
"""


def clean_env(**settings):
    env = {k: v for k, v in os.environ.items()
           if not k.startswith(("AUTOTUNER_", "SGPL_", "GRAPH_"))}
    return dict(env, **settings)


def pass_checks(tmp):
    driver_src = (ROOT / "test/pdg_tdg_verification/autotuner_frequency_driver.cpp").read_text()
    # This regression tests native loops, so it does not need the effect pass.
    driver_src = driver_src.replace('#include "graph_frontier_lowering.h"', '')
    start = driver_src.index('    if (argc > 2')
    end = driver_src.index('    passes.addPass(AutoTunerModulePass());', start)
    driver_src = driver_src[:start] + driver_src[end:]
    source = tmp / "driver.cpp"
    source.write_text(driver_src)
    flags = shlex.split(subprocess.check_output([
        os.environ.get("LLVM_CONFIG", "llvm-config-20"), "--cxxflags", "--ldflags",
        "--libs", "core", "irreader", "analysis", "passes", "--system-libs"], text=True))
    driver = tmp / "driver"
    subprocess.run([os.environ.get("CXX", "g++"), "-O1", "-iquote", str(ROOT),
                    str(source), str(ROOT / "AutoTunerPass.cpp"), *flags,
                    "-o", str(driver)], check=True)

    def run(name, ir, forced=None):
        path = tmp / f"{name}.input.ll"
        path.write_text(ir)
        env = clean_env(AUTOTUNER_PROFILE_STRICT_REGIONS="1")
        if forced:
            env["AUTOTUNER_FORCE_LAYOUT"] = forced
        result = subprocess.run([str(driver), str(path)], env=env, text=True,
                                capture_output=True, check=True)
        out = result.stdout
        (PROOF / f"{name}.ll").write_text(out)
        return out

    def layouts(ir):
        return list(map(int, re.findall(r'call void @autograph_ensure_layout\([^\n]*i32 (\d+)\)', ir)))

    def boundary(ir, nested=False):
        blocks = re.split(r'(?m)^([\w.]+):[^\n]*\n', ir)
        for i in range(1, len(blocks), 2):
            if '@autograph_ensure_layout(' in blocks[i + 1]:
                assert blocks[i] not in ("scan.header", "scan.body", "outer.header", "outer.latch"), blocks[i]
        if '@autograph_ensure_layout(' in ir[ir.index('scan.pre:'):ir.index('scan.header:')]:
            pre = ir[ir.index('scan.pre:'):ir.index('scan.header:')]
            assert pre.index('@autograph_ensure_layout(') < pre.index('@autograph_neighbor_iter_init(')

    plain = run("native_auto", native())
    assert 3 in layouts(plain), layouts(plain)  # No module-wide iterator CSR pin.
    boundary(plain)
    bitmap_body = """
  %size = call i64 @roaring_bitmap_get_cardinality(ptr @frontier)
  %seen = call i1 @roaring_bitmap_contains(ptr @frontier, i32 1)
  call void @roaring_bitmap_add(ptr @frontier, i32 2)
  call void @roaring_bitmap_remove(ptr @frontier, i32 3)
"""
    independent = run("independent_frontier", native(bitmap_body))
    assert layouts(independent) == layouts(plain)
    assert independent.count('call void @autograph_profile_region_enter(') == plain.count('call void @autograph_profile_region_enter(')
    boundary(independent)
    weighted = run("weighted_phase", native('%w = call i32 @graph_get_edge_weight(ptr @G, i32 0, i32 1)'))
    assert layouts(weighted) == [3, 0], layouts(weighted)
    boundary(weighted)
    for nested in (False, True):
        for rotated in (False, True):
            ir = run(f"boundary_{nested}_{rotated}", native(nested=nested, rotated=rotated), "PCSR")
            assert layouts(ir) == [1], layouts(ir)
            boundary(ir, nested)
    for name, before in (
        ("direct_bitmap", "%q = call i64 @roaring_bitmap_get_cardinality(ptr @nodes)"),
        ("bitmap_alias", "store ptr @nodes, ptr @bitmap.slot\n  %alias = load ptr, ptr @bitmap.slot\n  %q = call i64 @roaring_bitmap_get_cardinality(ptr %alias)"),
        ("bitmap_getter", "%bm = call ptr @graph_get_node_bitmap(ptr @G)\n  %q = call i64 @roaring_bitmap_get_cardinality(ptr %bm)"),
    ):
        ir = run(name, native(before=before), "CSR")
        assert layouts(ir) == [3, 0], (name, layouts(ir))
        boundary(ir)
    inside = run("bitmap_inside_scan", native('%q = call i64 @roaring_bitmap_get_cardinality(ptr @nodes)'), "CSR")
    assert layouts(inside) == [3], layouts(inside)
    boundary(inside)
    for mode, layout in (("CSR", 0), ("PCSR", 1), ("BCSR", 2), ("SET", 3)):
        ir = run(f"fixed_{mode}", native(bitmap_body), mode)
        assert all(l == layout for l in layouts(ir)), (mode, layouts(ir))
        boundary(ir)
    two_graphs = native().replace('ptr @G', 'ptr @H')
    two_graphs = '@H = global i8 0\n' + two_graphs.replace('entry:\n', '''entry:
  call void @autograph_init(ptr @G, i64 8, i64 16, ptr null, ptr null, ptr null)
  %row.field = getelementptr i8, ptr @G, i64 16
  %raw.row = load ptr, ptr %row.field
''', 1)
    ir = run("raw_csr_other_graph", two_graphs)
    assert 3 in layouts(ir), layouts(ir)
    boundary(ir)
    raw = native().replace('entry:\n', '''entry:
  %row.field = getelementptr i8, ptr @G, i64 16
  %raw.row = load ptr, ptr %row.field
''', 1)
    assert layouts(run("raw_csr_requirement", raw, "PCSR")) == []
    lowered = DECLS + '''define i32 @main() {
entry:
  call void @autograph_init(ptr @G, i64 544, i64 128032, ptr @nodes, ptr @edges, ptr null)
  %step = call i32 @autograph_frontier_execute(ptr @G, ptr null)
  br label %empty.exit
empty.exit:
  br label %exit, !autotuner.traverse !0
exit:
  ret i32 0
}
!0 = !{!"vertex"}
'''
    ir = run("lowered_metadata", lowered, "CSR")
    assert ir.count('call void @autograph_profile_region_enter(') == 1
    assert "!autotuner.traverse" not in ir
    print("PASS native AutoTuner: switching, phase boundaries, bitmap provenance, weighted constraints, four fixed layouts", flush=True)


def frontend_checks(rounds=300, modes=None):
    sys.path.insert(0, str(ROOT / "tools"))
    import layout_application_cases as cases
    from layout_application_benchmark import expected, verify
    from layout_switch_benchmark import build, compile_case, environment, MODES, run
    cases.OUT = PROOF / ("applications" if rounds == 300 else f"applications_rounds{rounds}")
    work, objects = build(cases.OUT)
    dfs = work / "dfs_runtime_src.cpp.o"
    run(["g++", "-O3", "-fopenmp", "-iquote", ROOT, "-c", ROOT / "dfs_runtime_src.cpp", "-o", dfs], work)
    objects.append(dfs)
    wrapper = work / "iterator_audit.o"
    run(["gcc", "-O2", "-iquote", ROOT, "-c", ROOT / "test/pdg_tdg_verification/iterator_phase_audit.c", "-o", wrapper], work)
    wraps = ["autograph_ensure_layout", "autograph_neighbor_iter_init", "autograph_neighbor_iter_next"]
    negative = work / "unsafe_conversion.c"
    negative.write_text('''#include "autotuner_runtime.h"
int main(void) {
  int64_t rows[] = {0, 1, 2}; int32_t cols[] = {1, 0};
  struct { int64_t n, m; int64_t *row; int32_t *col, *weight; int32_t directed; }
    graph = {2, 2, rows, cols, 0, 0};
  AutoNeighborIter iter;
  autograph_init(&graph, 2, 2, 0, 0, 0);
  autograph_neighbor_iter_init(&graph, 0, &iter);
  autograph_ensure_layout(&graph, 2, 2, 0, 0, 0, 0, 0, LAYOUT_SET);
  return 0;
}
''')
    run(["gcc", "-O2", "-iquote", ROOT, "-c", negative, "-o", work / "unsafe_conversion.o"], work)
    run(["g++", "-fopenmp", "-pthread", work / "unsafe_conversion.o", wrapper, *objects,
         *[f"-Wl,--wrap={symbol}" for symbol in wraps], "-ldl", "-lnlopt", "-o", work / "unsafe_conversion"], work)
    bad = subprocess.run([str(work / "unsafe_conversion")], env=environment(), text=True, capture_output=True)
    assert bad.returncode == 71 and "active iterator" in bad.stderr, (bad.returncode, bad.stderr)
    print("PASS audit negative control: conversion during iteration rejected", flush=True)
    summary = []
    for name, src, meta in cases.generate(rounds, 8):
        for mode in modes or MODES:
            # Always exercise the compiler just rebuilt, not an earlier proof's executable.
            (cases.OUT / "binaries" / name / mode).unlink(missing_ok=True)
            exe, schedule = compile_case(cases.OUT, work, objects, name, src, mode)
            if mode == "AUTO":
                assert any(layout != "CSR" for layout in schedule), (name, schedule)
            dest = exe.parent
            audit = dest / (mode + ".audit")
            run(["g++", "-fopenmp", "-pthread", "-no-pie", dest / (mode + ".program.o"), wrapper,
                 *objects, *[f"-Wl,--wrap={symbol}" for symbol in wraps], "-ldl", "-lnlopt", "-o", audit], work)
            ir = (dest / (mode + ".ll")).read_text()
            assert not re.search(r'call .*@autograph_edgemap\(', ir), name
            for threads in (1, 4):
                env = environment()
                env.update(SGPL_NUM_THREADS=str(threads), OMP_NUM_THREADS=str(threads))
                if mode != "AUTO":
                    env["AUDIT_FIXED_LAYOUT"] = str(MODES.index(mode) - 1)
                result = run([audit], cases.OUT / "codes", env)
                verify(result.stdout, expected(meta))
                (dest / f"{mode}.threads{threads}.audit.log").write_text(result.stdout + result.stderr)
                summary.append(dict(case=name, mode=mode, threads=threads, correct=True, schedule=schedule))
            print("PASS default backend", name, mode, schedule, flush=True)
    (PROOF / f"application_validation_rounds{rounds}.json").write_text(json.dumps(summary, indent=2) + "\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--frontend", action="store_true")
    parser.add_argument("--frontend-only", action="store_true")
    parser.add_argument("--rounds", type=int, default=300)
    parser.add_argument("--modes", nargs="+", choices=("AUTO", "CSR", "PCSR", "BCSR", "SET"))
    args = parser.parse_args()
    PROOF.mkdir(parents=True, exist_ok=True)
    if not args.frontend_only:
        with tempfile.TemporaryDirectory(prefix="autotuner-iterator-") as tmp:
            pass_checks(Path(tmp))
    if args.frontend or args.frontend_only:
        frontend_checks(args.rounds, args.modes)


if __name__ == "__main__":
    main()
