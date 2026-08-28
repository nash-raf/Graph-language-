#!/usr/bin/env python3
"""Polly vs p1 PDG/outliner trigger matrix.

Compiles custom dense / non-affine / CSR-ish loop kernels through GraphProgram
under three Polly configs and classifies what fired:

  * Polly SCoP detect (opt -polly-print-detect on PRE IR)
  * Polly transform markers (polly.* on POST IR)
  * OpenMP parallel from Polly (-polly-parallel → GOMP_parallel)
  * PDG loop class metadata (my.loop.parallel parallel.type=DOALL|DOACROSS)
  * Outliner runtime (parallel_for_runtime / parallel_for_runtime_ex)

Pipeline order in main.cpp (important):
  IRGen → [optional PRE dump] → Polly O3 → [POST dump] → Autotuner →
  PDG → reconstructParallelIR → [PDG dump] → loop outliner → …

So Polly and PDG almost never both parallelize the *same* nest: Polly runs first
and consumes affine loops.  Both can still fire in one *program* if some nests
are SCoPs and others are not.
"""

from __future__ import annotations

import csv
import os
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LLVM = Path(os.environ.get("LLVM_PREFIX", "/usr/local/llvm-20-polly-rtti"))
OPT = LLVM / "bin" / "opt"
LLVM_DIS = LLVM / "bin" / "llvm-dis"
OUT = ROOT / "benchmark_logs" / f"polly_vs_pdg_{time.strftime('%Y%m%d_%H%M%S')}"
N = int(os.environ.get("TRIGGER_N", "64"))  # small: trigger detection, not timing


def header(edge_file: str) -> str:
    return f'graph G {{\n  edges: file "{edge_file}";\n  TRUE\n}};\n\n'


# ---------------------------------------------------------------------------
# Kernels
# ---------------------------------------------------------------------------

def k_affine_matmul(edge: str) -> str:
    """Classic affine triple nest — Polly's sweet spot; DOALL when Polly off."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n][n];
  int B[n][n];
  int C[n][n];
  int i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      A[i][j] = i + 1;
      B[i][j] = j + 1;
      C[i][j] = 0;
      j = j + 1;
    }
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      int s = 0;
      int k = 0;
      while (k < n) {
        s = s + A[i][k] * B[k][j];
        k = k + 1;
      }
      C[i][j] = s;
      j = j + 1;
    }
    i = i + 1;
  }
  print C[1][1];
}
kernel();
"""


def k_affine_saxpy(edge: str) -> str:
    """Simple affine DOALL (y[i] = a*x[i] + y[i])."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int x[n];
  int y[n];
  int a = 3;
  int i = 0;
  while (i < n) {
    x[i] = i + 1;
    y[i] = i;
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    y[i] = a * x[i] + y[i];
    i = i + 1;
  }
  print y[n - 1];
}
kernel();
"""


def k_affine_prefix(edge: str) -> str:
    """Affine but loop-carried: prefix sum — DOACROSS / sequential for PDG."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int i = 0;
  while (i < n) {
    a[i] = i + 1;
    i = i + 1;
  }
  i = 1;
  while (i < n) {
    a[i] = a[i] + a[i - 1];
    i = i + 1;
  }
  print a[n - 1];
}
kernel();
"""


def k_nonaffine_indirect(edge: str) -> str:
    """A[B[i]] — non-affine subscript → Polly should form 0 SCoPs."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n];
  int B[n];
  int i = 0;
  while (i < n) {
    A[i] = 0;
    B[i] = n - 1 - i;
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    int idx = B[i];
    A[idx] = A[idx] + i;
    i = i + 1;
  }
  print A[0];
}
kernel();
"""


def k_nonaffine_data_dep_branch(edge: str) -> str:
    """Data-dependent branch inside nest — typically not a SCoP."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n];
  int i = 0;
  while (i < n) {
    A[i] = i;
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    int v = A[i];
    if (v < 0) {
      A[i] = 0 - v;
    } else {
      A[i] = v + 1;
    }
    i = i + 1;
  }
  print A[n / 2];
}
kernel();
"""


def k_csr_neighbor_walk(edge: str) -> str:
    """Opaque graph traversal — runtime neighbor iter, not affine."""
    return header(edge) + """
fn void kernel() {
  int sum = 0;
  for each vertex u in G {
    for each neighbor v of u in G {
      sum = sum + 1;
    }
  }
  print sum;
}
kernel();
"""


def k_mixed_affine_plus_indirect(edge: str) -> str:
    """One affine nest + one non-affine nest in the same fn — can both fire?"""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n][n];
  int B[n][n];
  int C[n][n];
  int P[n];
  int Q[n];

  int i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      A[i][j] = i + 1;
      B[i][j] = j + 1;
      C[i][j] = 0;
      j = j + 1;
    }
    P[i] = 0;
    Q[i] = n - 1 - i;
    i = i + 1;
  }

  // Affine matmul-ish (Polly target)
  i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      int s = 0;
      int k = 0;
      while (k < n) {
        s = s + A[i][k] * B[k][j];
        k = k + 1;
      }
      C[i][j] = s;
      j = j + 1;
    }
    i = i + 1;
  }

  // Non-affine gather (PDG target when Polly cannot see it)
  i = 0;
  while (i < n) {
    int idx = Q[i];
    P[idx] = P[idx] + C[i][i];
    i = i + 1;
  }

  print P[0] + C[1][1];
}
kernel();
"""


def k_fib_doacross(edge: str) -> str:
    """Two-term recurrence a[i]=a[i-1]+a[i-2] — PDG DOACROSS when Polly off."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int i = 0;
  while (i < n) {
    if (i < 2) { a[i] = i + 1; } else { a[i] = 0; }
    i = i + 1;
  }
  i = 2;
  while (i < n) {
    a[i] = a[i - 1] + a[i - 2];
    i = i + 1;
  }
  print a[n - 1];
}
kernel();
"""


def k_dense_ffl_count(edge: str) -> str:
    """Dense triple nest (Polly-visible FFL count), not CSR motif."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n][n];
  int count = 0;
  int i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      A[i][j] = 0;
      if (i != j) {
        int s = i + j;
        int r = s - (s / 3) * 3;
        if (r != 0) { A[i][j] = 1; }
      }
      j = j + 1;
    }
    i = i + 1;
  }
  int a = 0;
  while (a < n) {
    int b = 0;
    while (b < n) {
      int c = 0;
      while (c < n) {
        int prod = A[a][b] * A[a][c] * A[b][c];
        count = count + prod;
        c = c + 1;
      }
      b = b + 1;
    }
    a = a + 1;
  }
  print count;
}
kernel();
"""


def k_transpose_2d(edge: str) -> str:
    """Pure affine 2D copy/transpose — Polly sweet spot."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n][n];
  int B[n][n];
  int i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      A[i][j] = i + j;
      B[i][j] = 0;
      j = j + 1;
    }
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      B[j][i] = A[i][j];
      j = j + 1;
    }
    i = i + 1;
  }
  print B[1][1];
}
kernel();
"""


def k_bfs_frontier(edge: str) -> str:
    """Array BFS + neighbor walk — opaque CSR, Polly silent."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int visited[n];
  int parent[n];
  int frontier[n];
  int next_frontier[n];
  int frontier_size = 1;
  int next_size = 0;
  int i = 0;
  while (i < n) {
    visited[i] = 0;
    parent[i] = -1;
    i = i + 1;
  }
  frontier[0] = 0;
  visited[0] = 1;
  parent[0] = 0;
  while (frontier_size > 0) {
    next_size = 0;
    i = 0;
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
    i = 0;
    while (i < next_size) {
      frontier[i] = next_frontier[i];
      i = i + 1;
    }
    frontier_size = next_size;
  }
  print parent[n - 1];
}
kernel();
"""


def k_reduction_sum(edge: str) -> str:
    """Affine sum reduction — DOALL when Polly off."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n];
  int sum = 0;
  int i = 0;
  while (i < n) {
    A[i] = i + 1;
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    sum = sum + A[i];
    i = i + 1;
  }
  print sum;
}
kernel();
"""


def k_saxpy_prefix_combo(edge: str) -> str:
    """Saxpy DOALL + prefix DOACROSS, no matmul — tests Polly vs PDG split."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int x[n];
  int y[n];
  int pref[n];
  int i = 0;
  while (i < n) {
    x[i] = i + 1;
    y[i] = 1;
    pref[i] = 1;
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    y[i] = 3 * x[i] + y[i];
    i = i + 1;
  }
  i = 1;
  while (i < n) {
    pref[i] = pref[i - 1] + x[i];
    i = i + 1;
  }
  print y[n - 1] + pref[n - 1];
}
kernel();
"""


def k_combo_triple(edge: str) -> str:
    """Matmul (Polly) + saxpy (DOALL) + prefix/gather (DOACROSS) — same as DOACROSS suite case 10."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n][n];
  int Bmat[n][n];
  int C[n][n];
  int x[n];
  int y[n];
  int pref[n];
  int Q[n];
  int i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      A[i][j] = i + 1;
      Bmat[i][j] = j + 1;
      C[i][j] = 0;
      j = j + 1;
    }
    x[i] = i + 1;
    y[i] = 1;
    pref[i] = 1;
    Q[i] = n - 1 - i;
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      int s = 0;
      int k = 0;
      while (k < n) {
        s = s + A[i][k] * Bmat[k][j];
        k = k + 1;
      }
      C[i][j] = s;
      j = j + 1;
    }
    i = i + 1;
  }
  i = 0;
  while (i < n) { y[i] = 3 * x[i] + y[i]; i = i + 1; }
  i = 1;
  while (i < n) {
    int idx = Q[i];
    pref[i] = pref[i - 1] + x[idx];
    i = i + 1;
  }
  print y[n - 1] + pref[n - 1] + C[1][1];
}
kernel();
"""


def k_stencil_1d(edge: str) -> str:
    """1D Jacobi-style smooth — loop-carried on a[i-1], a[i+1]."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int b[n];
  int i = 0;
  while (i < n) {
    a[i] = i + 1;
    b[i] = 0;
    i = i + 1;
  }
  i = 1;
  while (i < n - 1) {
    b[i] = a[i - 1] + a[i] + a[i + 1];
    i = i + 1;
  }
  print b[n / 2];
}
kernel();
"""


def k_edge_only_walk(edge: str) -> str:
    """for each edge — opaque iteration, not affine."""
    return header(edge) + """
fn void kernel() {
  int sum = 0;
  for each edge u, v in G {
    sum = sum + u + v;
  }
  print sum;
}
kernel();
"""


def k_2d_init_only(edge: str) -> str:
    """Double-nested init only — small SCoP, often no GOMP at small N."""
    return header(edge) + """
fn void kernel() {
  int n = numVertices(G);
  int A[n][n];
  int i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      A[i][j] = i * n + j;
      j = j + 1;
    }
    i = i + 1;
  }
  print A[1][1];
}
kernel();
"""


KERNELS = {
    # --- original 7 ---
    "affine_matmul": k_affine_matmul,
    "affine_saxpy": k_affine_saxpy,
    "affine_prefix": k_affine_prefix,
    "nonaffine_indirect": k_nonaffine_indirect,
    "nonaffine_branch": k_nonaffine_data_dep_branch,
    "csr_neighbor": k_csr_neighbor_walk,
    "mixed_affine_indirect": k_mixed_affine_plus_indirect,
    # --- extended trigger patterns ---
    "fib_doacross": k_fib_doacross,
    "dense_ffl_count": k_dense_ffl_count,
    "transpose_2d": k_transpose_2d,
    "bfs_frontier": k_bfs_frontier,
    "reduction_sum": k_reduction_sum,
    "saxpy_prefix_combo": k_saxpy_prefix_combo,
    "combo_triple": k_combo_triple,
    "stencil_1d": k_stencil_1d,
    "edge_only_walk": k_edge_only_walk,
    "2d_init_only": k_2d_init_only,
}

# Polly configs: name → env mutations (on top of clean PATH)
CONFIGS = {
    "polly_off": {"GRAPH_DISABLE_POLLY": "1"},
    "polly_on": {},  # default: Polly + -polly-parallel
    "polly_nopar": {"GRAPH_POLLY_NO_PARALLEL": "1"},
}


def write_chain(path: Path, n: int) -> None:
    path.write_text("".join(f"{i} {i + 1} 1\n" for i in range(0, max(1, n - 1))))


def count_in_ll(text: str, needle: str) -> int:
    return text.count(needle)


def analyze_bc(bc: Path) -> dict:
    out = {
        "exists": bc.exists(),
        "polly_refs": 0,
        "gomp_parallel": 0,
        "parallel_for_runtime": 0,
        "parallel_for_runtime_ex": 0,
        "md_doall": 0,
        "md_doacross": 0,
        "md_sequential": 0,
        "my_loop_parallel": 0,
        "doacross_wait": 0,
        "scop_valid": -1,
    }
    if not bc.exists():
        return out
    ll = bc.with_suffix(".ll")
    subprocess.run(
        [str(LLVM_DIS), str(bc), "-o", str(ll)],
        check=False,
        capture_output=True,
    )
    text = ll.read_text(errors="replace") if ll.exists() else ""
    out["polly_refs"] = len(re.findall(r"\bpolly\.", text))
    out["gomp_parallel"] = count_in_ll(text, "GOMP_parallel")
    out["parallel_for_runtime"] = count_in_ll(text, "parallel_for_runtime")
    out["parallel_for_runtime_ex"] = count_in_ll(text, "parallel_for_runtime_ex")
    out["md_doall"] = count_in_ll(text, "parallel.type=DOALL")
    out["md_doacross"] = count_in_ll(text, "parallel.type=DOACROSS")
    out["md_sequential"] = count_in_ll(text, "parallel.type=SEQUENTIAL")
    out["my_loop_parallel"] = count_in_ll(text, "my.loop.parallel")
    out["doacross_wait"] = count_in_ll(text, "doacross.wait")
    return out


def scop_count(bc: Path) -> int:
    if not bc.exists() or not OPT.exists():
        return -1
    proc = subprocess.run(
        [str(OPT), "-polly-print-detect", "-disable-output", str(bc)],
        capture_output=True,
        text=True,
        check=False,
    )
    return (proc.stdout + proc.stderr).count("Valid Region for Scop")


def trigger_pattern(row: dict) -> str:
    """Coarse Polly vs PDG vs outline quadrant (polly_on row preferred)."""
    if row["cfg"] == "polly_off":
        polly_fires = False
    else:
        polly_fires = row["post_gomp"] > 0 or row["post_polly_refs"] > 0
    pdg_fires = row["pdg_doall"] > 0 or row["pdg_doacross"] > 0
    outlined = row["out_pfr"] > 0 or row["out_pfr_ex"] > 0

    if polly_fires and pdg_fires and outlined:
        return "Polly+PDG+outline (different nests)"
    if polly_fires and pdg_fires:
        return "Polly+PDG metadata (mixed program)"
    if polly_fires and outlined:
        return "Polly GOMP + residual outline"
    if polly_fires and not pdg_fires:
        return "Polly only (PDG idle on consumed nests)"
    if not polly_fires and pdg_fires and outlined:
        return "PDG+outline only (Polly silent)"
    if not polly_fires and pdg_fires and not outlined:
        return "PDG classifies, no outline (opaque/non-profit)"
    if not polly_fires and not pdg_fires:
        return "Neither fires"
    if pdg_fires:
        return "PDG metadata only"
    return "other"


def pattern_summary(rows: list[dict]) -> list[str]:
    """Build trigger-pattern cheat sheet from polly_on rows."""
    on_rows = [r for r in rows if r.get("ok") and r["cfg"] == "polly_on"]
    lines = [
        "## Trigger patterns (polly_on, one row per kernel)",
        "",
        "| kernel | SCoPs | GOMP | PDG DOALL | PDG DOACROSS | outlined | pattern |",
        "|---|---:|---:|---:|---:|---:|---|",
    ]
    for r in on_rows:
        pat = trigger_pattern(r)
        lines.append(
            f"| {r['kernel']} | {r['pre_scops']} | {r['post_gomp']} | "
            f"{r['pdg_doall']} | {r['pdg_doacross']} | {r['out_pfr']} | {pat} |"
        )
    lines += [
        "",
        "## Quadrant guide",
        "",
        "| Polly | PDG | Outline | Example kernels |",
        "|---|---|---|---|",
        "| yes (GOMP) | yes | yes | `combo_triple`, `mixed_affine_indirect` |",
        "| yes (GOMP) | partial | partial | `affine_matmul`, `transpose_2d`, `dense_ffl_count` |",
        "| yes (xform) | yes | no | `affine_prefix`, `fib_doacross` (Polly eats carried loop) |",
        "| scop only | yes | varies | `affine_saxpy` (tiny nest, detect but no GOMP) |",
        "| no | yes | no | `csr_neighbor`, `bfs_frontier`, `edge_only_walk`, `nonaffine_indirect` |",
        "| no | yes | yes | same kernels with `polly_off` |",
        "",
    ]
    return lines


def classify_row(row: dict) -> str:
    """Human bucket for the trigger matrix."""
    polly = row["cfg"] != "polly_off" and (
        row["post_polly_refs"] > 0 or row["post_gomp"] > 0 or row["pre_scops"] > 0
    )
    polly_par = row["post_gomp"] > 0
    pdg_doall = row["pdg_doall"] > 0
    pdg_doacross = row["pdg_doacross"] > 0
    outlined = row["out_pfr"] > 0 or row["out_pfr_ex"] > 0

    bits = []
    if polly_par:
        bits.append("POLLY_PAR")
    elif row["post_polly_refs"] > 0:
        bits.append("POLLY_XFORM")
    elif row["cfg"] != "polly_off" and row["pre_scops"] > 0:
        bits.append("POLLY_SCOP_ONLY")
    elif row["cfg"] != "polly_off":
        bits.append("POLLY_NONE")

    if pdg_doall:
        bits.append("PDG_DOALL")
    if pdg_doacross:
        bits.append("PDG_DOACROSS")
    if row["pdg_seq"] > 0 and not (pdg_doall or pdg_doacross):
        bits.append("PDG_SEQ_ONLY")
    if outlined:
        bits.append("OUTLINED")
    if not bits:
        bits.append("NONE")
    return "+".join(bits)


def ensure_runtime_objs() -> list[str]:
    """Build the usual link objs if missing (best-effort)."""
    objs = []
    cmds = []
    if (ROOT / "runtime.c").exists():
        cmds.append(["gcc", "-O3", "-c", "runtime.c", "-o", "runtime.o"])
        objs.append("runtime.o")
    if (ROOT / "roaring_bitmap.cpp").exists():
        cmds.append(["g++", "-O3", "-mavx2", "-c", "roaring_bitmap.cpp", "-o", "roaring_bitmap.o"])
        objs.append("roaring_bitmap.o")
    for name in ("graph_loader_runtime_mmap.cpp", "graph_loader_runtime.cpp"):
        if (ROOT / name).exists():
            out = name.replace(".cpp", ".o")
            cmds.append(
                ["g++", "-c", "-O2", "-std=c++17", "-fopenmp", name, "-o", out]
            )
            objs.append(out)
            break
    if (ROOT / "parallel_runtime_shim.c").exists():
        cmds.append(
            ["gcc", "-O3", "-fopenmp", "-iquote", ".", "-c", "parallel_runtime_shim.c", "-o", "parallel_runtime_shim.o"]
        )
        objs.append("parallel_runtime_shim.o")
    elif (ROOT / "parallel_runtime.c").exists():
        cmds.append(["gcc", "-O3", "-fopenmp", "-c", "parallel_runtime.c", "-o", "parallel_runtime.o"])
        objs.append("parallel_runtime.o")
    for cmd in cmds:
        target = Path(cmd[-1])
        if not (ROOT / target).exists():
            subprocess.run(cmd, cwd=ROOT, check=False, capture_output=True)
    return [o for o in objs if (ROOT / o).exists()]


def compile_case(kernel: str, cfg: str, objs: list[str]) -> dict:
    case = OUT / kernel / cfg
    case.mkdir(parents=True, exist_ok=True)
    edges = case / "edges.txt"
    graph = case / f"{kernel}.graph"
    write_chain(edges, N)
    graph.write_text(KERNELS[kernel](str(edges)))

    pre = case / "pre.bc"
    post = case / "post.bc"
    pdg = case / "pdg.bc"
    outline = case / "after_outline.bc"

    env = os.environ.copy()
    env["PATH"] = f"{LLVM}/bin:" + env.get("PATH", "")
    # Clear Polly toggles then apply config
    for k in ("GRAPH_DISABLE_POLLY", "GRAPH_POLLY_NO_PARALLEL", "GRAPH_POLLY_MATMUL_OPT", "GRAPH_POLLY_EXTRA_FLAGS"):
        env.pop(k, None)
    env.update(CONFIGS[cfg])
    env["DUMP_LLVM_BC_PRE"] = str(pre)
    env["DUMP_LLVM_BC_POST"] = str(post)
    env["DUMP_LLVM_BC_PDG"] = str(pdg)
    env["DUMP_LLVM_BC_AFTER_OUTLINE"] = str(outline)

    gp = ROOT / "GraphProgram"
    log = case / "compile.log"
    proc = subprocess.run(
        [str(gp), str(graph)],
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
    )
    log.write_text(proc.stdout + "\n--- stderr ---\n" + proc.stderr)
    # Dumps are written before runtime .ll linking; treat dump presence as success
    # for trigger detection even if GraphProgram exits non-zero on a missing .ll.
    dumps_ok = pre.exists() and post.exists() and pdg.exists()
    if proc.returncode != 0 and not dumps_ok:
        return {
            "kernel": kernel,
            "cfg": cfg,
            "ok": False,
            "err": (proc.stderr or proc.stdout)[-800:],
        }

    # Link like the other benches if program.o appeared
    program_o = ROOT / "program.o"
    if program_o.exists() and objs:
        bin_path = case / "a.out"
        link = ["g++", "-O3", "-fopenmp", str(program_o)] + [str(ROOT / o) for o in objs] + [
            "-o",
            str(bin_path),
            "-lgomp",
            "-lpthread",
        ]
        subprocess.run(link, cwd=ROOT, capture_output=True)

    pre_a = analyze_bc(pre)
    post_a = analyze_bc(post)
    pdg_a = analyze_bc(pdg)
    out_a = analyze_bc(outline)
    # SCoP detect on PRE (canonicalized-ish user IR before Polly)
    pre_scops = scop_count(pre)

    row = {
        "kernel": kernel,
        "cfg": cfg,
        "ok": True,
        "compile_rc": proc.returncode,
        "pre_scops": pre_scops,
        "post_polly_refs": post_a["polly_refs"],
        "post_gomp": post_a["gomp_parallel"],
        "pdg_doall": pdg_a["md_doall"],
        "pdg_doacross": pdg_a["md_doacross"],
        "pdg_seq": pdg_a["md_sequential"],
        "pdg_myloop": pdg_a["my_loop_parallel"],
        "pdg_doacross_md": pdg_a["doacross_wait"],
        "out_pfr": out_a["parallel_for_runtime"],
        "out_pfr_ex": out_a["parallel_for_runtime_ex"],
        "out_gomp": out_a["gomp_parallel"],
        "out_polly": out_a["polly_refs"],
    }
    row["bucket"] = classify_row(row)
    return row


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    if not (ROOT / "GraphProgram").exists():
        print("GraphProgram missing; rebuild with 02_run.sh", file=sys.stderr)
        return 1
    if not OPT.exists():
        print(f"opt not found at {OPT}", file=sys.stderr)
        return 1

    objs = ensure_runtime_objs()
    print(f"OUT={OUT}")
    print(f"N={N} kernels={len(KERNELS)} configs={list(CONFIGS)}")

    rows = []
    for kernel in KERNELS:
        for cfg in CONFIGS:
            print(f"  {kernel:28s} {cfg:12s} ...", flush=True)
            row = compile_case(kernel, cfg, objs)
            rows.append(row)
            if not row.get("ok"):
                print(f"    FAIL: {row.get('err', '')[:200]}")
            else:
                print(
                    f"    scops={row['pre_scops']} polly={row['post_polly_refs']} "
                    f"gomp={row['post_gomp']} doall={row['pdg_doall']} "
                    f"doacross={row['pdg_doacross']} out={row['out_pfr']} "
                    f"=> {row['bucket']}"
                )

    csv_path = OUT / "summary.csv"
    fields = [
        "kernel",
        "cfg",
        "ok",
        "pre_scops",
        "post_polly_refs",
        "post_gomp",
        "pdg_doall",
        "pdg_doacross",
        "pdg_seq",
        "pdg_myloop",
        "pdg_doacross_md",
        "out_pfr",
        "out_pfr_ex",
        "out_gomp",
        "out_polly",
        "bucket",
    ]
    with csv_path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        w.writeheader()
        for r in rows:
            w.writerow(r)

    # Markdown report
    md = OUT / "REPORT.md"
    lines = [
        "# Polly vs PDG/TDG trigger matrix (p1 Autotuner)",
        "",
        f"N={N}, date={time.strftime('%Y-%m-%d')}",
        "",
        "## Pipeline fact",
        "",
        "Polly runs **before** PDG + loop outliner. Affine nests Polly parallelizes",
        "become opaque; the outliner then sees **0** remaining loops on those nests.",
        "`GRAPH_DISABLE_POLLY=1` hands the same nests to PDG (DOALL/DOACROSS).",
        "",
        "## Results",
        "",
        "| kernel | cfg | SCoPs | polly.* | GOMP | PDG DOALL | PDG DOACROSS | outlined | bucket |",
        "|---|---|---:|---:|---:|---:|---:|---:|---|",
    ]
    for r in rows:
        if not r.get("ok"):
            lines.append(f"| {r['kernel']} | {r['cfg']} | FAIL | | | | | | |")
            continue
        lines.append(
            f"| {r['kernel']} | {r['cfg']} | {r['pre_scops']} | {r['post_polly_refs']} | "
            f"{r['post_gomp']} | {r['pdg_doall']} | {r['pdg_doacross']} | "
            f"{r['out_pfr']} | `{r['bucket']}` |"
        )

    lines += pattern_summary([r for r in rows if r.get("ok")])

    lines += [
        "",
        "## How to read buckets",
        "",
        "- `POLLY_PAR` — Polly emitted OpenMP (`GOMP_parallel`); PDG usually idle on those nests",
        "- `POLLY_XFORM` / `POLLY_SCOP_ONLY` — Polly saw/transformed without parallel, or detect-only",
        "- `POLLY_NONE` — Polly enabled but no SCoP / no transform",
        "- `PDG_DOALL` / `PDG_DOACROSS` — `my.loop.parallel` metadata after PDG",
        "- `OUTLINED` — `parallel_for_runtime` call sites after outliner",
        "- Combined `POLLY_*+PDG_*` on **mixed** kernels means different nests, not the same loop",
        "",
        f"Raw CSV: `{csv_path}`",
        "",
    ]
    md.write_text("\n".join(lines))
    print(f"\nWrote {csv_path}\nWrote {md}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
