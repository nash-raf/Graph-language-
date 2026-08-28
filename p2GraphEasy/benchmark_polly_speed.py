#!/usr/bin/env python3
"""Measure end-to-end runtime of dense Polly-compatible DSL kernels.

The corpus intentionally excludes CSR traversals (BFS, motif matching, etc.).
Those contain data-dependent, indirect accesses and are not SCoPs, so a
"Polly on/off speedup" for them would measure only compiler overhead.
"""

from __future__ import annotations

import csv
import os
import statistics
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parent
OUT = ROOT / "benchmark_logs" / f"polly_speed_{time.strftime('%Y%m%d_%H%M%S')}"
GRAPH_PROGRAM = ROOT / "GraphProgram"
RUNS = int(os.environ.get("RUNS", "7"))
THREADS = int(os.environ.get("OMP_NUM_THREADS", "4"))


@dataclass(frozen=True)
class Case:
    name: str
    algorithm: str
    vertices: int


CASES = [
    Case("matmul_n384", "matrix multiplication", 384),
    Case("matmul_n768", "matrix multiplication", 768),
    Case("floyd_n256", "Floyd-Warshall APSP", 256),
    Case("floyd_n384", "Floyd-Warshall APSP", 384),
    Case("floyd_n512", "Floyd-Warshall APSP", 512),
]


def run(command: list[str], *, env: dict[str, str] | None = None) -> None:
    subprocess.run(command, cwd=ROOT, env=env, check=True, text=True)


def write_chain_edges(path: Path, vertices: int) -> None:
    # The loader derives vertex count from the largest vertex id, so the final
    # edge is deliberately incident to vertices - 1.
    path.write_text(
        "".join(f"{vertex} {vertex + 1} 1\n" for vertex in range(1, vertices - 1))
    )


def matmul_source(edge_file: str) -> str:
    return f"""\
// Dense affine kernel; all arrays are n*n int32 values.
graph G {{
  edges: file "{edge_file}";
  TRUE
}};

fn void kernel() {{
  int n = numVertices(G);
  int A[n][n];
  int B[n][n];
  int C[n][n];

  int i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      A[i][j] = i + 1;
      B[i][j] = j + 1;
      C[i][j] = 0;
      j = j + 1;
    }}
    i = i + 1;
  }}

  i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      int s = 0;
      int k = 0;
      while (k < n) {{
        s = s + A[i][k] * B[k][j];
        k = k + 1;
      }}
      C[i][j] = s;
      j = j + 1;
    }}
    i = i + 1;
  }}

  print C[1][1];
}}

kernel();
"""


def floyd_source(edge_file: str) -> str:
    return f"""\
// Dense Floyd-Warshall phase. The chain input keeps the graph loader work tiny.
graph G {{
  edges: file "{edge_file}";
  TRUE
}};

fn void floydWarshall() {{
  int n = numVertices(G);
  int dist[n][n];
  int inf = 1000000000;

  int i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      if (i == j) {{
        dist[i][j] = 0;
      }} else {{
        dist[i][j] = inf;
      }}
      j = j + 1;
    }}
    i = i + 1;
  }}

  for each edge u, v in G {{
    int w = weight(G, u, v);
    if (w < dist[u][v]) {{
      dist[u][v] = w;
      dist[v][u] = w;
    }}
  }}

  int k = 0;
  while (k < n) {{
    i = 0;
    while (i < n) {{
      int j = 0;
      while (j < n) {{
        if (dist[i][k] != inf && dist[k][j] != inf) {{
          int nd = dist[i][k] + dist[k][j];
          if (nd < dist[i][j]) {{
            dist[i][j] = nd;
          }}
        }}
        j = j + 1;
      }}
      i = i + 1;
    }}
    k = k + 1;
  }}

  print dist[1][n - 1];
}}

floydWarshall();
"""


def compile_case(case: Case, polly: bool) -> tuple[Path, str]:
    case_dir = OUT / case.name
    case_dir.mkdir(parents=True, exist_ok=True)
    edges = case_dir / "edges.txt"
    source = case_dir / f"{case.name}.graph"
    write_chain_edges(edges, case.vertices)
    source.write_text(
        matmul_source(str(edges)) if case.algorithm == "matrix multiplication"
        else floyd_source(str(edges))
    )

    env = os.environ.copy()
    if not polly:
        env["GRAPH_DISABLE_POLLY"] = "1"
    suffix = "on" if polly else "off"
    subprocess.run(
        [str(GRAPH_PROGRAM), str(source)],
        cwd=ROOT,
        env=env,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=True,
    )
    binary = case_dir / f"final_polly_{suffix}"
    run([
        "g++", "program.o", "runtime.o", "roaring_bitmap.o",
        "graph_loader_runtime_mmap.o", "graph_runtime.o",
        "-fopenmp", "-no-pie", "-o", str(binary),
    ])
    output = subprocess.check_output([str(binary)], text=True, stderr=subprocess.DEVNULL)
    return binary, output.strip()


def timed_runs(binary: Path) -> list[float]:
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(THREADS)
    samples = []
    for _ in range(RUNS):
        start = time.perf_counter()
        subprocess.run(
            [str(binary)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            env=env,
            check=True,
        )
        samples.append(time.perf_counter() - start)
    return samples


def main() -> int:
    if not GRAPH_PROGRAM.is_file():
        print("GraphProgram is missing; run ./build_lowmem.sh first.", file=sys.stderr)
        return 1
    OUT.mkdir(parents=True)
    run(["g++", "-c", "-O2", "-std=c++17", "-fopenmp",
         "graph_loader_runtime_mmap.cpp", "-o", "graph_loader_runtime_mmap.o"])
    run(["g++", "-c", "-O2", "-std=c++17", "graph_runtime.cpp", "-o", "graph_runtime.o"])

    rows = []
    for case in CASES:
        print(f"== {case.name} ==")
        on_binary, on_output = compile_case(case, polly=True)
        off_binary, off_output = compile_case(case, polly=False)
        if on_output != off_output:
            raise RuntimeError(
                f"{case.name}: output differs (on={on_output!r}, off={off_output!r})"
            )
        on_times = timed_runs(on_binary)
        off_times = timed_runs(off_binary)
        on_median = statistics.median(on_times)
        off_median = statistics.median(off_times)
        row = {
            "case": case.name,
            "algorithm": case.algorithm,
            "vertices": case.vertices,
            "dense_array_mib": f"{(4 * case.vertices * case.vertices * (3 if case.algorithm == 'matrix multiplication' else 1)) / 2**20:.2f}",
            "runs": RUNS,
            "polly_on_median_s": f"{on_median:.6f}",
            "polly_off_median_s": f"{off_median:.6f}",
            "speedup_off_over_on": f"{off_median / on_median:.3f}",
            "output": on_output,
        }
        rows.append(row)
        print(
            f"  on={on_median:.3f}s off={off_median:.3f}s "
            f"speedup={off_median / on_median:.3f}x output={on_output}"
        )

    with (OUT / "summary.csv").open("w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(f"\nResults: {OUT / 'summary.csv'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
