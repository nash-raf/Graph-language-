#!/usr/bin/env python3
"""Motif ON vs OFF ablation (~10 cases): EdgeMap + Floyd–Warshall closure.

Modes (compile-time env):
  motif   default (EdgeMap / autograph_closure rewrites enabled)
  normal  GRAPH_DISABLE_MOTIF=1  (naïve loop lowering)

Reports median wall time and speedup = normal/motif.
"""

from __future__ import annotations

import csv
import os
import resource
import statistics
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LLVM = Path(os.environ.get("LLVM_PREFIX", "/usr/local/llvm-20-polly-rtti"))
OUT = ROOT / "benchmark_logs" / f"motif_speedup_{time.strftime('%Y%m%d_%H%M%S')}"
RUNS = int(os.environ.get("RUNS", "3"))
WARMUP = int(os.environ.get("WARMUP", "1"))

G10K = ROOT / "synth_v_10000_e_100000.txt"
G100K = ROOT / "synth_v_100000_e_1000000.txt"
GVAL = Path("/home/user/Course/msk1/grapheasy_algo/graphs/validation_100n_100e.txt")
GVALW = Path("/home/user/Course/msk1/grapheasy_algo/graphs/validation_100n_100e_w.txt")
G50K = Path("/home/user/Course/msk1/2nd/Graph-language-/p1GraphEasy/synth_v_50000_e_500000.txt")


def bump_stack(mb: int = 2048) -> None:
    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    want = mb * 1024 * 1024
    try:
        resource.setrlimit(resource.RLIMIT_STACK, (min(want, hard) if hard > 0 else want, hard if hard > 0 else want))
    except (ValueError, OSError):
        pass


def header(edges: str, weighted: bool = False) -> str:
    w = "  TRUE\n" if weighted else ""
    return f'graph G {{\n  edges: file "{edges}";\n{w}}};\n\n'


def src_bfs(edges: str) -> str:
    return header(edges) + """
fn void kernel() {
  int n = numVertices(G);
  int visited[n];
  int parent[n];
  int frontier[n];
  int next_frontier[n];
  int frontier_size = 1;
  int next_size = 0;
  int init_i = 0;
  while (init_i < n) {
    visited[init_i] = 0;
    parent[init_i] = -1;
    init_i = init_i + 1;
  }
  frontier[0] = 0;
  visited[0] = 1;
  parent[0] = 0;
  while (frontier_size > 0) {
    next_size = 0;
    int i = 0;
    while (i < frontier_size) {
      int v = frontier[i];
      for each neighbor u of v in G {
        if (visited[u] == 0) {
          visited[u] = 1;
          parent[u] = v;
          next_frontier[next_size] = u;
          next_size = next_size + 1;
        }
      }
      i = i + 1;
    }
    swap(frontier, next_frontier);
    frontier_size = next_size;
  }
  print parent[n - 1];
}
kernel();
"""


def src_cc_mincopy(edges: str) -> str:
    # Array-frontier MinCopy (label prop) — set frontiers do NOT match.
    return header(edges) + """
fn void kernel() {
  int n = numVertices(G);
  int id[n];
  int frontier[n];
  int next_frontier[n];
  int frontier_size = n;
  int next_size = 0;
  int i = 0;
  while (i < n) {
    id[i] = i;
    frontier[i] = i;
    i = i + 1;
  }
  int rounds = 0;
  while (frontier_size > 0) {
    if (rounds == n) {
      break;
    }
    next_size = 0;
    i = 0;
    while (i < frontier_size) {
      int u = frontier[i];
      for each neighbor v of u in G {
        if (id[u] < id[v]) {
          id[v] = id[u];
          next_frontier[next_size] = v;
          next_size = next_size + 1;
        }
      }
      i = i + 1;
    }
    swap(frontier, next_frontier);
    frontier_size = next_size;
    rounds = rounds + 1;
  }
  print id[0];
}
kernel();
"""


def src_sssp(edges: str) -> str:
    return header(edges, weighted=True) + """
fn void kernel() {
  int n = numVertices(G);
  int inf = 2147483647;
  int dist[n];
  int frontier[n];
  int next_frontier[n];
  int frontier_size = 1;
  int next_size = 0;
  int i = 0;
  while (i < n) {
    dist[i] = inf;
    i = i + 1;
  }
  dist[0] = 0;
  frontier[0] = 0;
  int rounds = 0;
  while (frontier_size > 0) {
    if (rounds == n) {
      break;
    }
    next_size = 0;
    i = 0;
    while (i < frontier_size) {
      int u = frontier[i];
      for each neighbor v of u in G {
        int nd = dist[u] + weight(G, u, v);
        if (nd < dist[v]) {
          dist[v] = nd;
          next_frontier[next_size] = v;
          next_size = next_size + 1;
        }
      }
      i = i + 1;
    }
    swap(frontier, next_frontier);
    frontier_size = next_size;
    rounds = rounds + 1;
  }
  print dist[n - 1];
}
kernel();
"""


def src_peelk(edges: str, k: int = 3) -> str:
    return header(edges) + f"""
fn void kernel() {{
  int n = numVertices(G);
  int k = {k};
  int deg[n];
  int alive[n];
  int frontier[n];
  int next_frontier[n];
  int frontier_size = 0;
  int next_size = 0;
  int i = 0;
  while (i < n) {{
    deg[i] = 0;
    alive[i] = 1;
    i = i + 1;
  }}
  i = 0;
  while (i < n) {{
    for each neighbor v of i in G {{
      deg[i] = deg[i] + 1;
    }}
    i = i + 1;
  }}
  i = 0;
  while (i < n) {{
    if (deg[i] < k) {{
      frontier[frontier_size] = i;
      frontier_size = frontier_size + 1;
    }}
    i = i + 1;
  }}
  while (frontier_size > 0) {{
    next_size = 0;
    i = 0;
    while (i < frontier_size) {{
      int u = frontier[i];
      if (alive[u] == 1) {{
        alive[u] = 0;
        for each neighbor v of u in G {{
          if (alive[v] == 1) {{
            deg[v] = deg[v] - 1;
            if (deg[v] < k) {{
              next_frontier[next_size] = v;
              next_size = next_size + 1;
            }}
          }}
        }}
      }}
      i = i + 1;
    }}
    swap(frontier, next_frontier);
    frontier_size = next_size;
  }}
  print alive[0];
}}
kernel();
"""


def write_fw_edges(path: Path, n: int) -> None:
    # Sparse-ish directed band so hasEdge is interesting; n vertices for O(n^3) nest.
    lines = []
    for i in range(n):
        if i + 1 < n:
            lines.append(f"{i} {i + 1} 1\n")
        if i + 2 < n:
            lines.append(f"{i} {i + 2} 2\n")
        if i + 3 < n:
            lines.append(f"{i} {i + 3} 3\n")
    path.write_text("".join(lines))


def src_fw_min_plus(edges: str) -> str:
    return f"""graph G {{
  directed: true;
  edges: file "{edges}";
  TRUE
}};

fn void kernel() {{
  int n = numVertices(G);
  int BIG = 1000000;
  int D[n][n];
  int i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      if (hasEdge(G, i, j)) {{
        D[i][j] = weight(G, i, j);
      }} else {{
        D[i][j] = BIG;
      }}
      j = j + 1;
    }}
    D[i][i] = 0;
    i = i + 1;
  }}
  int k = 0;
  while (k < n) {{
    i = 0;
    while (i < n) {{
      int j = 0;
      while (j < n) {{
        int cand = D[i][k] + D[k][j];
        if (cand < D[i][j]) {{
          D[i][j] = cand;
        }}
        j = j + 1;
      }}
      i = i + 1;
    }}
    k = k + 1;
  }}
  print D[0][n - 1];
}}
kernel();
"""


def src_fw_max_times(edges: str) -> str:
    return f"""graph G {{
  directed: true;
  edges: file "{edges}";
  TRUE
}};

fn void kernel() {{
  int n = numVertices(G);
  int D[n][n];
  int i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      if (hasEdge(G, i, j)) {{
        D[i][j] = 1;
      }} else {{
        D[i][j] = 0;
      }}
      j = j + 1;
    }}
    D[i][i] = 1;
    i = i + 1;
  }}
  int k = 0;
  while (k < n) {{
    i = 0;
    while (i < n) {{
      int j = 0;
      while (j < n) {{
        int cand = D[i][k] * D[k][j];
        if (cand > D[i][j]) {{
          D[i][j] = cand;
        }}
        j = j + 1;
      }}
      i = i + 1;
    }}
    k = k + 1;
  }}
  print D[0][n - 1];
}}
kernel();
"""


def src_fw_max_plus(edges: str) -> str:
    return f"""graph G {{
  directed: true;
  edges: file "{edges}";
  TRUE
}};

fn void kernel() {{
  int n = numVertices(G);
  int NEG = -1000000;
  int D[n][n];
  int i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      if (hasEdge(G, i, j)) {{
        D[i][j] = weight(G, i, j);
      }} else {{
        D[i][j] = NEG;
      }}
      j = j + 1;
    }}
    D[i][i] = 0;
    i = i + 1;
  }}
  int k = 0;
  while (k < n) {{
    i = 0;
    while (i < n) {{
      int j = 0;
      while (j < n) {{
        int cand = D[i][k] + D[k][j];
        if (cand > D[i][j]) {{
          D[i][j] = cand;
        }}
        j = j + 1;
      }}
      i = i + 1;
    }}
    k = k + 1;
  }}
  print D[0][n - 1];
}}
kernel();
"""


def ensure_link_objs() -> list[str]:
    objs = []
    specs = [
        (["gcc", "-O3", "-c", "runtime.c", "-o", "runtime.o"], "runtime.c", "runtime.o"),
        (["g++", "-O3", "-mavx2", "-c", "roaring_bitmap.cpp", "-o", "roaring_bitmap.o"], "roaring_bitmap.cpp", "roaring_bitmap.o"),
        (["g++", "-c", "-O2", "-std=c++17", "-fopenmp", "graph_loader_runtime.cpp", "-o", "graph_loader_runtime.o"], "graph_loader_runtime.cpp", "graph_loader_runtime.o"),
        (["g++", "-c", "-O2", "-std=c++17", "graph_runtime.cpp", "-o", "graph_runtime.o"], "graph_runtime.cpp", "graph_runtime.o"),
        (["gcc", "-O3", "-fopenmp", "-c", "parallel_runtime.c", "-o", "parallel_runtime.o"], "parallel_runtime.c", "parallel_runtime.o"),
        (["gcc", "-O3", "-c", "autotuner_runtime.c", "-o", "autotuner_runtime.o"], "autotuner_runtime.c", "autotuner_runtime.o"),
        (["gcc", "-O3", "-c", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.o"], "graph_mutation_runtime.c", "graph_mutation_runtime.o"),
        (["gcc", "-O3", "-fopenmp", "-c", "semiring_runtime.c", "-o", "semiring_runtime.o"], "semiring_runtime.c", "semiring_runtime.o"),
    ]
    for cmd, src, out in specs:
        if not (ROOT / src).exists():
            continue
        target = ROOT / out
        if not target.exists() or (ROOT / src).stat().st_mtime > target.stat().st_mtime:
            subprocess.run(cmd, cwd=ROOT, check=False, capture_output=True)
        if target.exists():
            objs.append(out)
    return objs


def mode_env(mode: str) -> dict:
    env = os.environ.copy()
    env["PATH"] = f"{LLVM}/bin:" + env.get("PATH", "")
    for k in list(env):
        if k.startswith("DUMP_LLVM_") or k.startswith("GRAPH_"):
            env.pop(k, None)
    env.setdefault("OMP_NUM_THREADS", str(os.cpu_count() or 4))
    env.setdefault("SGPL_NUM_THREADS", env["OMP_NUM_THREADS"])
    env["SGPL_FORCE_DOALL_PARALLEL"] = "1"
    if mode == "normal":
        env["GRAPH_DISABLE_MOTIF"] = "1"
    return env


def compile_case(name: str, mode: str, src: str, objs: list[str]) -> tuple[Path | None, dict]:
    case = OUT / "bins" / name / mode
    case.mkdir(parents=True, exist_ok=True)
    graph = case / f"{name}.graph"
    graph.write_text(src)
    env = mode_env(mode)
    dump = case / "after.bc"
    env["DUMP_LLVM_BC_AFTER_OUTLINE"] = str(dump)
    proc = subprocess.run(
        [str(ROOT / "GraphProgram"), str(graph)],
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
    )
    (case / "compile.log").write_text(proc.stdout + "\n---\n" + proc.stderr)
    markers = {"edgemap": 0, "closure": 0}
    if dump.exists():
        ll = case / "after.ll"
        subprocess.run([str(LLVM / "bin" / "llvm-dis"), str(dump), "-o", str(ll)], capture_output=True)
        if ll.exists():
            text = ll.read_text(errors="ignore")
            markers["edgemap"] = text.count("autograph_edgemap")
            markers["closure"] = text.count("autograph_closure")
            (case / "markers.txt").write_text(
                f"edgemap={markers['edgemap']} closure={markers['closure']}\n"
            )
    if proc.returncode != 0 or not (ROOT / "program.o").exists():
        return None, markers
    bin_path = case / "a.out"
    link = (
        ["g++", "-O3", "-fopenmp", "-no-pie", str(ROOT / "program.o")]
        + [str(ROOT / o) for o in objs]
        + ["-ldl", "-lnlopt", "-lgomp", "-lpthread", "-o", str(bin_path)]
    )
    lr = subprocess.run(link, cwd=ROOT, capture_output=True, text=True)
    (case / "link.log").write_text(lr.stdout + lr.stderr)
    if lr.returncode != 0:
        return None, markers
    return bin_path, markers


def time_bin(bin_path: Path, env: dict) -> list[float]:
    times = []
    for i in range(WARMUP + RUNS):
        t0 = time.perf_counter()
        r = subprocess.run(
            [str(bin_path)],
            cwd=bin_path.parent,
            env=env,
            capture_output=True,
            text=True,
            timeout=600,
        )
        dt = time.perf_counter() - t0
        if r.returncode != 0:
            raise RuntimeError(r.stderr[-400:] or r.stdout[-400:] or "nonzero")
        if i >= WARMUP:
            times.append(dt)
    return times


def build_cases() -> list[tuple[str, str, str]]:
    """Build ablation cases. Set CLOSURE_ONLY=1 for FW/semiring only."""
    cases: list[tuple[str, str, str]] = []
    closure_only = os.environ.get("CLOSURE_ONLY", "0") not in (
        "0",
        "",
        "false",
        "False",
    )

    if not closure_only:
        if GVAL.exists():
            cases.append(("01_bfs_val100", "edgemap", src_bfs(str(GVAL))))
        if G10K.exists():
            cases.append(("02_bfs_10k", "edgemap", src_bfs(str(G10K))))
        if G50K.exists():
            cases.append(("03_bfs_50k", "edgemap", src_bfs(str(G50K))))
        if G100K.exists():
            cases.append(("04_bfs_100k", "edgemap", src_bfs(str(G100K))))
        if G10K.exists():
            cases.append(("05_cc_mincopy_10k", "edgemap", src_cc_mincopy(str(G10K))))
        if G50K.exists():
            cases.append(("06_cc_mincopy_50k", "edgemap", src_cc_mincopy(str(G50K))))
        if GVALW.exists():
            cases.append(("07_sssp_val100w", "edgemap", src_sssp(str(GVALW))))
        w50 = ROOT / "edgelistw.txt"
        if w50.exists() and w50.stat().st_size > 1000:
            cases.append(("08_sssp_edgelistw", "edgemap", src_sssp(str(w50))))
        elif GVALW.exists():
            cases.append(("08_sssp_val100w_b", "edgemap", src_sssp(str(GVALW))))
        if G10K.exists():
            cases.append(("09_peelk_10k", "edgemap", src_peelk(str(G10K), k=3)))

    # FW / dense semiring closure
    # Default IR: MaxTimes/MaxPlus rewrite; MinPlus left for Polly (no <1×).
    # GRAPH_FORCE_CLOSURE_MOTIF=1 forces MinPlus through autograph_closure too.
    fw_dir = OUT / "fw_edges"
    fw_dir.mkdir(parents=True, exist_ok=True)
    fw_specs = [
        (96, "fw_minplus_n96", "minplus"),
        (192, "fw_minplus_n192", "minplus"),
        (400, "fw_minplus_n400", "minplus"),
        (96, "fw_maxtimes_n96", "maxtimes"),
        (192, "fw_maxtimes_n192", "maxtimes"),
        (400, "fw_maxtimes_n400", "maxtimes"),
        (96, "fw_maxplus_n96", "maxplus"),
        (192, "fw_maxplus_n192", "maxplus"),
    ]
    for n, tag, kind in fw_specs:
        ep = fw_dir / f"fw_{n}.txt"
        write_fw_edges(ep, n)
        if kind == "maxtimes":
            cases.append((tag, "closure", src_fw_max_times(str(ep))))
        elif kind == "maxplus":
            cases.append((tag, "closure", src_fw_max_plus(str(ep))))
        else:
            cases.append((tag, "closure", src_fw_min_plus(str(ep))))
    return cases


def main() -> int:
    bump_stack(2048)
    OUT.mkdir(parents=True, exist_ok=True)
    if not (ROOT / "GraphProgram").exists():
        print("GraphProgram missing", file=sys.stderr)
        return 1
    stub = ROOT / "bfs_runtime.ll"
    if not stub.exists():
        stub.write_text("; stub\n")

    objs = ensure_link_objs()
    cases = build_cases()
    print(f"OUT={OUT}")
    print(f"cases={len(cases)} runs={RUNS} objs={objs}")

    rows = []
    for name, kind, src in cases:
        print(f"\n== {name} ({kind}) ==")
        times = {}
        marks = {}
        ok = True
        for mode in ("normal", "motif"):
            print(f"  compile {mode} ...", flush=True)
            bin_path, markers = compile_case(name, mode, src, objs)
            marks[mode] = markers
            if not bin_path:
                print(f"    FAIL compile/link (see bins/{name}/{mode}/)")
                ok = False
                break
            hit = markers["edgemap"] + markers["closure"]
            print(f"    markers edgemap={markers['edgemap']} closure={markers['closure']}")
            if mode == "motif" and hit == 0:
                print("    WARN: expected motif rewrite missing")
            if mode == "normal" and hit != 0:
                print("    WARN: motif still present under GRAPH_DISABLE_MOTIF")
            try:
                ts = time_bin(bin_path, mode_env(mode))
            except Exception as e:
                print(f"    FAIL run: {e}")
                ok = False
                break
            med = statistics.median(ts)
            times[mode] = med
            print(f"    {med * 1000:.1f} ms")
        if not ok or "normal" not in times or "motif" not in times:
            rows.append(
                {
                    "case": name,
                    "kind": kind,
                    "ok": False,
                    "normal_ms": None,
                    "motif_ms": None,
                    "speedup": 0.0,
                    "edgemap": marks.get("motif", {}).get("edgemap", 0),
                    "closure": marks.get("motif", {}).get("closure", 0),
                }
            )
            continue
        spd = times["normal"] / times["motif"] if times["motif"] > 0 else 0.0
        print(f"  speedup motif vs normal = {spd:.2f}×")
        rows.append(
            {
                "case": name,
                "kind": kind,
                "ok": True,
                "normal_ms": times["normal"] * 1000.0,
                "motif_ms": times["motif"] * 1000.0,
                "speedup": spd,
                "edgemap": marks["motif"]["edgemap"],
                "closure": marks["motif"]["closure"],
            }
        )

    csv_path = OUT / "summary.csv"
    with csv_path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)

    md = OUT / "REPORT.md"
    lines = [
        "# Motif ON vs OFF speedup",
        "",
        f"Runs={RUNS}. `normal` = `GRAPH_DISABLE_MOTIF=1`. `motif` = default rewrite.",
        "",
        "| case | kind | normal ms | motif ms | speedup | edgemap | closure |",
        "|---|---|---:|---:|---:|---:|---:|",
    ]
    for r in rows:
        if not r["ok"]:
            lines.append(f"| {r['case']} | {r['kind']} | FAIL | | | | |")
        else:
            lines.append(
                f"| {r['case']} | {r['kind']} | {r['normal_ms']:.1f} | {r['motif_ms']:.1f} | "
                f"{r['speedup']:.2f}× | {r['edgemap']} | {r['closure']} |"
            )
    lines += [
        "",
        "## Notes",
        "- Closure engine: **bit-parallel transitive closure** for MaxTimes on {0,1};",
        "  tiled FW + persistent OpenMP team (n≥512) for general semirings.",
        "- Closure-only bench: `CLOSURE_ONLY=1 python3 bench_motif_speedup.py`.",
        "",
    ]
    md.write_text("\n".join(lines))
    print(f"\nWrote {csv_path}\nWrote {md}")
    # print table
    print("\n".join(lines))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
