#!/usr/bin/env python3
"""Compile the real passes and check execution counts in their emitted IR.

Run in Linux/WSL with LLVM 20:
python3 test/pdg_tdg_verification/test_autotuner_frequency.py
Add --frontend after rebuilding GraphProgram to also compile and execute DSL
programs with one/twenty rounds at one/four threads.
"""
import math
import os
from pathlib import Path
import re
import shlex
import struct
import subprocess
import sys
import tempfile

SUITE = Path(__file__).resolve().parent
ROOT = SUITE.parent.parent
DECLS = """
@G = global i8 0
@out = global [8 x i32] zeroinitializer
declare void @autograph_init(ptr, i64, i64, ptr, ptr, ptr, i32)
declare i32 @autograph_build_clean_cut(ptr, i32)
declare i32 @autograph_frontier_execute(ptr, ptr)
declare i32 @autograph_frontier_fork_join(ptr, ptr, ptr, ptr)
declare i32 @autograph_edgemap(ptr)
declare i32 @bfs_runtime_src(ptr, i32)
declare void @graph_add_edge(ptr, ptr, i32, i32, i32)
declare void @autograph_neighbor_iter_init(ptr, i32, ptr)
declare i32 @autograph_neighbor_iter_next(ptr, ptr)
"""
INIT = "call void @autograph_init(ptr @G, i64 8, i64 16, ptr null, ptr null, ptr null, i32 0)"
STEP = "%result = call i32 @autograph_frontier_execute(ptr @G, ptr null)"


def module(body, metadata=""):
    return DECLS + "\ndefine i32 @main() {\n" + body + "\n}\n" + metadata


def counted_loop(work, rounds=20, start=0, stride=1, metadata=True, tail=""):
    annotation = ", !autotuner.trip_count !0" if metadata else ""
    return module(f"""
entry:
  {INIT}
  br label %round.header
round.header:
  %round = phi i32 [{start}, %entry], [%next, %round.body]
  %active = icmp slt i32 %round, {rounds}
  br i1 %active, label %round.body, label %exit{annotation}
round.body:
  {work}
  %next = add i32 %round, {stride}
  br label %round.header
exit:
  {tail}
  ret i32 0
""", f"!0 = !{{i64 {rounds}}}\n" if metadata else "")


def lowered_loop(rounds):
    # This is an actual canonical source-owned graph traversal. The test runs
    # effect lowering first, rather than faking an executor-only replacement.
    return module(f"""
entry:
  {INIT}
  %iter = alloca [64 x i8]
  %v.slot = alloca i32
  br label %round.header
round.header:
  %round = phi i32 [0, %entry], [%round.next, %round.latch]
  %active = icmp slt i32 %round, {rounds}
  br i1 %active, label %driver.pre, label %exit, !autotuner.trip_count !0
driver.pre:
  br label %driver.header
driver.header:
  %u = phi i64 [0, %driver.pre], [%u.next, %driver.latch]
  %more.u = icmp ult i64 %u, 8
  br i1 %more.u, label %driver.body, label %round.latch, !autotuner.traverse !1
driver.body:
  %u32 = trunc i64 %u to i32
  call void @autograph_neighbor_iter_init(ptr @G, i32 %u32, ptr %iter)
  br label %neighbor.header
neighbor.header:
  %has.next = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %v.slot)
  %more.v = icmp ne i32 %has.next, 0
  br i1 %more.v, label %neighbor.body, label %driver.latch, !autotuner.traverse !2
neighbor.body:
  %cell = getelementptr [8 x i32], ptr @out, i64 0, i64 %u
  %old = load i32, ptr %cell
  %inc = add i32 %old, 1
  store i32 %inc, ptr %cell
  br label %neighbor.header
driver.latch:
  %u.next = add i64 %u, 1
  br label %driver.header
round.latch:
  %round.next = add i32 %round, 1
  br label %round.header
exit:
  ret i32 0
""", f'!0 = !{{i64 {rounds}}}\n!1 = !{{!"vertex"}}\n!2 = !{{!"neighbor"}}\n')


def profile_costs(ir):
    return [struct.unpack(">d", bytes.fromhex(x[2:]))[0] if x.startswith("0x") else float(x)
            for x in re.findall(r"call void @autograph_profile_region_enter\(i32 \d+, i32 0, i32 \d+, double ([^)]+)\)", ir)]


def frontend_check(tmp, env):
    """Optional end-to-end check using the freshly rebuilt GraphProgram."""
    for path in ROOT.glob("*.ll"):
        (tmp / path.name).symlink_to(path)
    edges = ROOT / "test/shared_edgelist.txt"
    costs = []
    for rounds in (1, 20):
        source = tmp / f"rounds_{rounds}.graph"
        source.write_text(f'''graph G {{ edges: file "{edges}"; }};
int out_degree[6];
int round = 0;
while (round < {rounds}) {{
  for each vertex u in G {{
    for each neighbor v of u in G {{
      out_degree[u] = out_degree[u] + 1;
    }}
  }}
  round = round + 1;
}}
print out_degree[1];
''')
        ir = tmp / f"rounds_{rounds}.ll"
        compiled = subprocess.run([str(ROOT / "GraphProgram"), "--ir-backend=cpu",
                                   f"--emit-ir-to={ir}", str(source)], cwd=tmp,
                                  env=dict(env, GRAPH_DISABLE_POLLY="1"), text=True,
                                  capture_output=True)
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        observed = profile_costs(ir.read_text())
        assert len(observed) == 1, (rounds, observed)
        costs.append(observed[0])
        (tmp / "program.o").rename(tmp / f"rounds_{rounds}.o")
    assert math.isclose(costs[1], 20 * costs[0], rel_tol=1e-9), costs

    objects = []
    for name in ("autotuner_runtime", "graph_mutation_runtime", "parallel_runtime",
                 "gpu_runtime", "semiring_runtime"):
        obj = tmp / f"{name}.o"
        subprocess.run(["gcc", "-O2", "-fopenmp", "-pthread", "-iquote", str(ROOT),
                        "-c", str(ROOT / f"{name}.c"), "-o", str(obj)], check=True)
        objects.append(str(obj))
    for name in ("graph_loader_runtime", "roaring_bitmap"):
        obj = tmp / f"{name}.o"
        subprocess.run(["g++", "-O2", "-mavx2", "-std=c++17", "-fopenmp", "-pthread", "-iquote", str(ROOT),
                        "-c", str(ROOT / f"{name}.cpp"), "-o", str(obj)], check=True)
        objects.append(str(obj))
    for rounds in (1, 20):
        exe = tmp / f"rounds_{rounds}"
        subprocess.run(["g++", "-no-pie", "-fopenmp", "-pthread", str(tmp / f"rounds_{rounds}.o"),
                        *objects, "-lnlopt", "-ldl", "-lm", "-o", str(exe)], check=True)
        for threads in (1, 4):
            result = subprocess.run([str(exe)], cwd=tmp, env=dict(env, SGPL_NUM_THREADS=str(threads)),
                                    text=True, capture_output=True, check=True, timeout=60)
            assert result.stdout.strip() == str(2 * rounds), (rounds, threads, result.stdout)
            assert f"visits={rounds} " in result.stderr, (rounds, result.stderr)
    print("PASS frontend/runtime: 20x predicted work; correct results and visits at 1/4 threads")


def main():
    if "--frontend-only" in sys.argv:
        with tempfile.TemporaryDirectory(prefix="ars-frequency-frontend-") as tmp:
            frontend_check(Path(tmp), dict(os.environ, AUTOTUNER_FORCE_LAYOUT="CSR",
                                          SGPL_FRONTIER_STRICT="1"))
        return
    llvm_config = os.environ.get("LLVM_CONFIG", "llvm-config-20")
    flags = shlex.split(subprocess.check_output(
        [llvm_config, "--cxxflags", "--ldflags", "--libs", "core", "irreader", "analysis", "passes", "--system-libs"], text=True))
    with tempfile.TemporaryDirectory(prefix="ars-frequency-") as tmp:
        tmp = Path(tmp)
        driver = tmp / "driver"
        subprocess.run([os.environ.get("CXX", "g++"), "-O1", "-iquote", str(ROOT),
                        str(SUITE / "autotuner_frequency_driver.cpp"),
                        str(ROOT / "AutoTunerPass.cpp"), str(ROOT / "graph_frontier_lowering.cpp"),
                        *flags, "-o", str(driver)], check=True)
        env = dict(os.environ, AUTOTUNER_FORCE_LAYOUT="CSR",
                   AUTOTUNER_PROFILE_STRICT_REGIONS="0", SGPL_FRONTIER_STRICT="1")
        for key in ("GRAPH_FRONTIER_REWRITE_OFF", "SGPL_FRONTIER_BLOCKLIST_GUARD"):
            env.pop(key, None)

        def run(name, source, lower=False):
            path = tmp / f"{name}.ll"
            path.write_text(source)
            result = subprocess.run([str(driver), str(path), *(["--lower"] if lower else [])],
                                    env=env, text=True, capture_output=True, check=True)
            return result.stdout

        baseline = profile_costs(run("one", module(f"entry:\n{INIT}\n{STEP}\nret i32 0")))
        assert len(baseline) == 1 and baseline[0] > 0, baseline
        unit = baseline[0]

        def check(name, source, multiples, lower=False):
            ir = run(name, source, lower)
            costs = profile_costs(ir)
            expected = [unit * n for n in multiples]
            assert len(costs) == len(expected), (name, costs, expected)
            assert all(math.isclose(a, b, rel_tol=1e-9) for a, b in zip(costs, expected)), (name, costs, expected)
            if lower:
                assert "call i32 @autograph_build_clean_cut" in ir, name
                assert "call i32 @autograph_frontier_execute" in ir, name
                assert "!autotuner.traverse" not in ir, name
            print(f"PASS {name}: traversal counts={multiples}")

        check("twenty_rounds", counted_loop(STEP), [20])
        check("offset_stride", counted_loop(STEP, rounds=20, start=4, stride=2, metadata=False), [8])
        check("after_loop", counted_loop("", tail=STEP), [1])
        check("zero_rounds", counted_loop(STEP, rounds=0), [])
        check("dynamic_rounds", counted_loop(STEP, metadata=False).replace(
            "define i32 @main()", "define i32 @main(i32 %limit)").replace(
            "icmp slt i32 %round, 20", "icmp slt i32 %round, %limit"), [1])
        for name, call in [("fork_join", "autograph_frontier_fork_join(ptr @G, ptr null, ptr null, ptr null)"),
                           ("edgemap", "autograph_edgemap(ptr @G)")]:
            check(name, counted_loop(f"%result = call i32 @{call}"), [20])

        # An insertion region must not lend its static-site count to a step.
        adds = "\n".join("call void @graph_add_edge(ptr @G, ptr null, i32 0, i32 1, i32 0)" for _ in range(5))
        check("inserts_then_step", module(f"entry:\n{INIT}\n{adds}\n{STEP}\nret i32 0"), [1])
        check("two_steps", counted_loop(STEP + "\n" + STEP.replace("%result", "%second")), [20, 20])
        check("nested_rounds", module(f"""
entry:
  {INIT}
  br label %outer.header
outer.header:
  %outer = phi i32 [0, %entry], [%outer.next, %outer.latch]
  %outer.active = icmp slt i32 %outer, 3
  br i1 %outer.active, label %inner.pre, label %exit, !autotuner.trip_count !0
inner.pre:
  br label %inner.header
inner.header:
  %inner = phi i32 [0, %inner.pre], [%inner.next, %inner.body]
  %inner.active = icmp slt i32 %inner, 7
  br i1 %inner.active, label %inner.body, label %outer.latch, !autotuner.trip_count !1
inner.body:
  {STEP}
  %inner.next = add i32 %inner, 1
  br label %inner.header
outer.latch:
  %outer.next = add i32 %outer, 1
  br label %outer.header
exit:
  ret i32 0
""", "!0 = !{i64 3}\n!1 = !{i64 7}\n"), [21])
        check("effect_one", lowered_loop(1), [1], lower=True)
        check("effect_twenty", lowered_loop(20), [20], lower=True)
        check("native_twenty", lowered_loop(20).replace(", !autotuner.traverse !2", ""), [20])
        check("different_repetitions", module(f"""
entry:
  {INIT}
  br label %first.header
first.header:
  %i = phi i32 [0, %entry], [%i.next, %first.body]
  %more.i = icmp slt i32 %i, 2
  br i1 %more.i, label %first.body, label %second.pre, !autotuner.trip_count !0
first.body:
  %a = call i32 @bfs_runtime_src(ptr @G, i32 0)
  %i.next = add i32 %i, 1
  br label %first.header
second.pre:
  br label %second.header
second.header:
  %j = phi i32 [0, %second.pre], [%j.next, %second.body]
  %more.j = icmp slt i32 %j, 5
  br i1 %more.j, label %second.body, label %exit, !autotuner.trip_count !1
second.body:
  %b = call i32 @bfs_runtime_src(ptr @G, i32 0)
  %j.next = add i32 %j, 1
  br label %second.header
exit:
  ret i32 0
""", "!0 = !{i64 2}\n!1 = !{i64 5}\n"), [7])
        print("ARS frequency regression tests: PASS")
        if "--frontend" in sys.argv:
            frontend_check(tmp, env)


if __name__ == "__main__":
    main()
