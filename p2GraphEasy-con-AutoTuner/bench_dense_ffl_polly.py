#!/usr/bin/env python3
"""Dense FFL count (affine O(n^3)) — Polly on/off for p2 and p2-AT.

Unlike CSR `where motif { ... }`, this builds an n×n adjacency matrix and
counts feed-forward loops with nested while-loops so Polly can see a SCoP.
"""

from __future__ import annotations

import os
import statistics
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
P2 = ROOT
P2_AT = ROOT.parent / "p2GraphEasy-con-AutoTuner"
OUT = ROOT / "benchmark_logs" / f"dense_ffl_polly_{time.strftime('%Y%m%d_%H%M%S')}"
OPT = Path("/usr/local/llvm-20-polly-rtti/bin/opt")
RUNS = int(os.environ.get("RUNS", "5"))
NS = [int(x) for x in os.environ.get("FFL_NS", "128,192,256").split(",") if x.strip()]


def write_chain(path: Path, n: int) -> None:
    # Loader needs |V|=n; chain is only for numVertices.
    path.write_text("".join(f"{i} {i + 1} 1\n" for i in range(1, n - 1)))


def ffl_source(edge_file: str) -> str:
    # Synthetic directed adj: edge i->j when (i+j)%3 != 0.
    # Count FFLs with a multiply accumulate (more SCoP-friendly than deep if-nests).
    return f"""\
// Dense affine FFL count — Polly target (not CSR motif comprehension).
graph G {{
  edges: file "{edge_file}";
  TRUE
}};

fn void countFFL() {{
  int n = numVertices(G);
  int A[n][n];
  int count = 0;

  int i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      A[i][j] = 0;
      if (i != j) {{
        int s = i + j;
        int r = s - (s / 3) * 3;
        if (r != 0) {{
          A[i][j] = 1;
        }}
      }}
      j = j + 1;
    }}
    i = i + 1;
  }}

  int a = 0;
  while (a < n) {{
    int b = 0;
    while (b < n) {{
      int c = 0;
      while (c < n) {{
        int ab = A[a][b];
        int ac = A[a][c];
        int bc = A[b][c];
        int prod = ab * ac * bc;
        count = count + prod;
        c = c + 1;
      }}
      b = b + 1;
    }}
    a = a + 1;
  }}

  print count;
}}

countFFL();
"""


def ensure_objs(project: Path, autotuner: bool) -> list[str]:
    objs = ["runtime.o", "roaring_bitmap.o", "graph_runtime.o"]
    commands = [
        ["gcc", "-O3", "-c", "runtime.c", "-o", "runtime.o"],
        ["g++", "-O3", "-mavx2", "-c", "roaring_bitmap.cpp", "-o", "roaring_bitmap.o"],
        ["g++", "-c", "-O2", "-std=c++17", "graph_runtime.cpp", "-o", "graph_runtime.o"],
    ]
    if (project / "graph_loader_runtime_mmap.cpp").exists():
        commands.append(
            [
                "g++",
                "-c",
                "-O2",
                "-std=c++17",
                "-fopenmp",
                "graph_loader_runtime_mmap.cpp",
                "-o",
                "graph_loader_runtime_mmap.o",
            ]
        )
        objs.append("graph_loader_runtime_mmap.o")
    else:
        commands.append(
            [
                "g++",
                "-c",
                "-O2",
                "-std=c++17",
                "-fopenmp",
                "graph_loader_runtime.cpp",
                "-o",
                "graph_loader_runtime.o",
            ]
        )
        objs.append("graph_loader_runtime.o")

    if autotuner:
        # Match 2final_run.sh: Autotuner IR is linked inside GraphProgram; the
        # user binary only needs the shim (+ optional semiring), not autotuner_runtime.o.
        commands.append(
            ["gcc", "-O3", "-fopenmp", "-iquote", ".", "-c", "parallel_runtime_shim.c", "-o", "parallel_runtime_shim.o"]
        )
        objs.append("parallel_runtime_shim.o")
        if (project / "semiring_runtime.c").exists():
            commands.append(
                ["gcc", "-O3", "-fopenmp", "-iquote", ".", "-c", "semiring_runtime.c", "-o", "semiring_runtime.o"]
            )
            objs.append("semiring_runtime.o")

    for cmd in commands:
        subprocess.run(cmd, cwd=project, check=True, capture_output=True)
    return objs


def scop_count(bc: Path) -> int:
    if not bc.exists() or not OPT.exists():
        return -1
    proc = subprocess.run(
        [str(OPT), "-polly-print-detect", "-disable-output", str(bc)],
        capture_output=True,
        text=True,
        check=False,
    )
    text = proc.stdout + proc.stderr
    # Count Valid Region lines under main only roughly: total is fine for signal.
    return text.count("Valid Region for Scop")


def compile_and_time(project: Path, label: str, n: int, polly: bool, objs: list[str]) -> dict:
    case_dir = OUT / label / f"n{n}" / ("polly_on" if polly else "polly_off")
    case_dir.mkdir(parents=True, exist_ok=True)
    edges = case_dir / "edges.txt"
    graph = case_dir / "dense_ffl.graph"
    write_chain(edges, n)
    graph.write_text(ffl_source(str(edges)))

    env = os.environ.copy()
    env["PATH"] = "/usr/local/llvm-20-polly-rtti/bin:" + env.get("PATH", "")
    if not polly:
        env["GRAPH_DISABLE_POLLY"] = "1"
    pre_bc = case_dir / "pre.bc"
    env["DUMP_LLVM_BC_PRE"] = str(pre_bc)

    gp = project / "GraphProgram"
    subprocess.run(
        [str(gp), str(graph)],
        cwd=project,
        env=env,
        check=True,
        capture_output=True,
        text=True,
    )

    binary = case_dir / "final"
    link = ["g++", "-O3", "-fopenmp", "-no-pie", "program.o", *objs, "-o", str(binary)]
    subprocess.run(link, cwd=project, check=True, capture_output=True)

    # Warmup
    subprocess.run([str(binary)], cwd=project, check=True, capture_output=True)
    times: list[float] = []
    out = ""
    for _ in range(RUNS):
        t0 = time.perf_counter()
        proc = subprocess.run([str(binary)], cwd=project, capture_output=True, text=True, check=True)
        times.append(time.perf_counter() - t0)
        out = proc.stdout.strip()

    return {
        "project": label,
        "n": n,
        "polly": "on" if polly else "off",
        "median_s": statistics.median(times),
        "mean_s": statistics.mean(times),
        "stdout": out.splitlines()[-1] if out else "",
        "scops_pre": scop_count(pre_bc),
        "raw_s": times,
    }


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    results = []

    for project, label, at in [
        (P2, "p2GraphEasy", False),
        (P2_AT, "p2GraphEasy-con-AutoTuner", True),
    ]:
        if not (project / "GraphProgram").is_file():
            print(f"SKIP {label}: no GraphProgram")
            continue
        print(f"=== preparing objs: {label} ===", flush=True)
        objs = ensure_objs(project, at)
        for n in NS:
            for polly in (True, False):
                print(f"  {label} n={n} polly={'on' if polly else 'off'} ...", flush=True)
                r = compile_and_time(project, label, n, polly, objs)
                results.append(r)
                print(
                    f"    median={r['median_s']*1000:.2f}ms scops={r['scops_pre']} out={r['stdout']}",
                    flush=True,
                )

    # Summary speedups
    print("\n=== SPEEDUP (off/on; >1 means Polly faster) ===")
    by = {}
    for r in results:
        by[(r["project"], r["n"], r["polly"])] = r
    for project, _, _ in [("p2GraphEasy", None, None), ("p2GraphEasy-con-AutoTuner", None, None)]:
        for n in NS:
            on = by.get((project, n, "on"))
            off = by.get((project, n, "off"))
            if not on or not off:
                continue
            speed = off["median_s"] / on["median_s"] if on["median_s"] > 0 else float("nan")
            match = "ok" if on["stdout"] == off["stdout"] else "MISMATCH"
            print(
                f"{project:28} n={n:<4} polly_on={on['median_s']*1000:8.2f}ms  "
                f"polly_off={off['median_s']*1000:8.2f}ms  speedup={speed:.3f}x  "
                f"scops_on/off={on['scops_pre']}/{off['scops_pre']}  {match} out={on['stdout']}"
            )

    csv_path = OUT / "summary.csv"
    with csv_path.open("w", encoding="utf-8") as f:
        f.write("project,n,polly,median_s,mean_s,scops_pre,stdout\n")
        for r in results:
            f.write(
                f"{r['project']},{r['n']},{r['polly']},{r['median_s']:.6f},"
                f"{r['mean_s']:.6f},{r['scops_pre']},{r['stdout']}\n"
            )
    print(f"\nWrote {csv_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
