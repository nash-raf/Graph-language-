#!/usr/bin/env python3
"""Performance ablation: serial vs Polly vs DOALL/DOACROSS vs combinations.

Modes (env):
  serial       GRAPH_DISABLE_POLLY=1 GRAPH_DISABLE_PDG=1
  pdg          GRAPH_DISABLE_POLLY=1          (DOALL/DOACROSS outlining)
  polly        GRAPH_DISABLE_PDG=1            (Polly O3 + -polly-parallel)
  both         (default: Polly + PDG)

Kernels:
  doall_saxpy, doacross_prefix, polly_matmul, combo_all3

Plots speedup vs serial for each kernel × mode.
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

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parent
LLVM = Path(os.environ.get("LLVM_PREFIX", "/usr/local/llvm-20-polly-rtti"))
OUT = ROOT / "benchmark_logs" / f"parallel_boost_{time.strftime('%Y%m%d_%H%M%S')}"
RUNS = int(os.environ.get("RUNS", "5"))
WARMUP = int(os.environ.get("WARMUP", "1"))

# Problem sizes — large enough for parallel overhead to pay off.
# Arrays are IR alloca VLAs (stack); raise RLIMIT_STACK before runs.
SAXPY_N = int(os.environ.get("SAXPY_N", "6000000"))
PREFIX_N = int(os.environ.get("PREFIX_N", "2000000"))
MATMUL_N = int(os.environ.get("MATMUL_N", "448"))
COMBO_N = int(os.environ.get("COMBO_N", "288"))


def bump_stack(mb: int = 1024) -> None:
    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    want = mb * 1024 * 1024
    try:
        resource.setrlimit(resource.RLIMIT_STACK, (want, hard if hard > 0 else want))
    except (ValueError, OSError):
        try:
            resource.setrlimit(resource.RLIMIT_STACK, (hard, hard))
        except (ValueError, OSError):
            pass
    soft2, _ = resource.getrlimit(resource.RLIMIT_STACK)
    print(f"RLIMIT_STACK soft={soft2 // (1024 * 1024)} MB (was {soft // (1024 * 1024)} MB)")

MODES = ("serial", "pdg", "polly", "both")


def header(edges: str) -> str:
    return f'graph G {{\n  edges: file "{edges}";\n  TRUE\n}};\n\n'


def write_verts(path: Path, n: int) -> None:
    # Single edge spanning 0 .. n-1 so numVertices == n without a huge file.
    path.write_text(f"0 {n - 1} 1\n" if n > 1 else "0 0 1\n")


def kernel_doall(edges: str, n: int) -> str:
    # Keep a single top-level DOALL nest (no outer t-loop): Autotuner TDG tasking
    # around an outer loop prevents the loop-outliner from seeing DOALL candidates.
    return header(edges) + f"""
fn void kernel() {{
  int n = numVertices(G);
  int x[n];
  int y[n];
  int i = 0;
  while (i < n) {{
    x[i] = i + 1;
    y[i] = 1;
    i = i + 1;
  }}
  i = 0;
  while (i < n) {{
    int v = 3 * x[i] + y[i];
    v = v + 5 * x[i];
    v = v + 7 * y[i];
    v = v + 11 * x[i];
    y[i] = v;
    i = i + 1;
  }}
  i = 0;
  while (i < n) {{
    int v = y[i] + x[i];
    v = v * 3 + x[i];
    y[i] = v + y[i];
    i = i + 1;
  }}
  print y[n - 1];
}}
kernel();
"""


def kernel_doacross(edges: str, n: int) -> str:
    return header(edges) + f"""
fn void kernel() {{
  int n = numVertices(G);
  int a[n];
  int x[n];
  int i = 0;
  while (i < n) {{
    a[i] = 1;
    x[i] = (i % 7) + 1;
    i = i + 1;
  }}
  i = 1;
  while (i < n) {{
    a[i] = a[i - 1] + x[i];
    i = i + 1;
  }}
  i = 1;
  while (i < n) {{
    a[i] = a[i - 1] + x[i] + 1;
    i = i + 1;
  }}
  print a[n - 1];
}}
kernel();
"""


def kernel_matmul(edges: str, n: int) -> str:
    return header(edges) + f"""
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


def kernel_combo(edges: str, n: int) -> str:
    return header(edges) + f"""
fn void kernel() {{
  int n = numVertices(G);
  int A[n][n];
  int Bmat[n][n];
  int C[n][n];
  int x[n];
  int y[n];
  int pref[n];
  int Q[n];
  int i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      A[i][j] = i + 1;
      Bmat[i][j] = j + 1;
      C[i][j] = 0;
      j = j + 1;
    }}
    x[i] = i + 1;
    y[i] = 1;
    pref[i] = 1;
    Q[i] = n - 1 - i;
    i = i + 1;
  }}
  i = 0;
  while (i < n) {{
    int j = 0;
    while (j < n) {{
      int s = 0;
      int k = 0;
      while (k < n) {{
        s = s + A[i][k] * Bmat[k][j];
        k = k + 1;
      }}
      C[i][j] = s;
      j = j + 1;
    }}
    i = i + 1;
  }}
  i = 0;
  while (i < n) {{
    y[i] = 3 * x[i] + y[i] + C[i][i];
    i = i + 1;
  }}
  i = 1;
  while (i < n) {{
    int idx = Q[i];
    pref[i] = pref[i - 1] + x[idx];
    i = i + 1;
  }}
  print y[n - 1] + pref[n - 1] + C[1][1];
}}
kernel();
"""


KERNELS = {
    "doall_saxpy": (SAXPY_N, kernel_doall),
    "doacross_prefix": (PREFIX_N, kernel_doacross),
    "polly_matmul": (MATMUL_N, kernel_matmul),
    "combo_all3": (COMBO_N, kernel_combo),
}


def mode_env(mode: str) -> dict:
    env = os.environ.copy()
    env["PATH"] = f"{LLVM}/bin:" + env.get("PATH", "")
    # Scrub sticky debug/flags from the parent shell so modes are orthogonal.
    for k in list(env):
        if k.startswith("DUMP_LLVM_") or k.startswith("GRAPH_"):
            env.pop(k, None)
    # Ensure OpenMP sees cores for Polly GOMP / PDG runtime
    env.setdefault("OMP_NUM_THREADS", str(os.cpu_count() or 4))
    env.setdefault("OMP_PROC_BIND", "close")
    if mode == "serial":
        env["GRAPH_DISABLE_POLLY"] = "1"
        env["GRAPH_DISABLE_PDG"] = "1"
        env["OMP_NUM_THREADS"] = "1"
    elif mode == "pdg":
        env["GRAPH_DISABLE_POLLY"] = "1"
        # Force DOALL past cost-model warmup. Do NOT force DOACROSS: wavefront
        # sync on multi-million trip counts is pathological and times out.
        env["SGPL_FORCE_DOALL_PARALLEL"] = "1"
    elif mode == "polly":
        env["GRAPH_DISABLE_PDG"] = "1"
    elif mode == "both":
        env["SGPL_FORCE_DOALL_PARALLEL"] = "1"
    else:
        raise ValueError(mode)
    return env


def ensure_link_objs() -> list[str]:
    objs = []
    cmds = []
    if (ROOT / "runtime.c").exists():
        cmds.append(["gcc", "-O3", "-c", "runtime.c", "-o", "runtime.o"])
        objs.append("runtime.o")
    if (ROOT / "roaring_bitmap.cpp").exists():
        cmds.append(["g++", "-O3", "-mavx2", "-c", "roaring_bitmap.cpp", "-o", "roaring_bitmap.o"])
        objs.append("roaring_bitmap.o")
    for name in ("graph_loader_runtime.cpp",):
        if (ROOT / name).exists():
            out = name.replace(".cpp", ".o")
            cmds.append(["g++", "-c", "-O2", "-std=c++17", "-fopenmp", name, "-o", out])
            objs.append(out)
    if (ROOT / "graph_runtime.cpp").exists():
        cmds.append(["g++", "-c", "-O2", "-std=c++17", "graph_runtime.cpp", "-o", "graph_runtime.o"])
        objs.append("graph_runtime.o")
    if (ROOT / "parallel_runtime.c").exists():
        cmds.append(["gcc", "-O3", "-fopenmp", "-c", "parallel_runtime.c", "-o", "parallel_runtime.o"])
        objs.append("parallel_runtime.o")
    if (ROOT / "autotuner_runtime.c").exists():
        cmds.append(["gcc", "-O3", "-c", "autotuner_runtime.c", "-o", "autotuner_runtime.o"])
        objs.append("autotuner_runtime.o")
    if (ROOT / "graph_mutation_runtime.c").exists():
        cmds.append(["gcc", "-O3", "-c", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.o"])
        objs.append("graph_mutation_runtime.o")
    for cmd in cmds:
        out = Path(cmd[cmd.index("-o") + 1])
        src = next(Path(a) for a in cmd if a.endswith((".c", ".cpp")))
        target = ROOT / out
        if not target.exists() or (ROOT / src).stat().st_mtime > target.stat().st_mtime:
            subprocess.run(cmd, cwd=ROOT, check=False, capture_output=True)
    return [o for o in objs if (ROOT / o).exists()]


def compile_bin(kernel: str, mode: str, n: int, src_fn, objs: list[str]):
    case = OUT / "bins" / kernel / mode
    case.mkdir(parents=True, exist_ok=True)
    edges = case / "edges.txt"
    write_verts(edges, n)
    graph = case / f"{kernel}.graph"
    graph.write_text(src_fn(str(edges), n))

    env = mode_env(mode)
    # Capture IR markers for the report
    dump_bc = case / "after.bc"
    env["DUMP_LLVM_BC_AFTER_OUTLINE"] = str(dump_bc)
    gp = ROOT / "GraphProgram"
    log = case / "compile.log"
    proc = subprocess.run(
        [str(gp), str(graph)], cwd=ROOT, env=env, capture_output=True, text=True
    )
    log.write_text(proc.stdout + "\n---\n" + proc.stderr)
    markers = {"GOMP": 0, "pfr": 0, "doacross": 0}
    if dump_bc.exists():
        ll = case / "after.ll"
        subprocess.run(
            [str(LLVM / "bin" / "llvm-dis"), str(dump_bc), "-o", str(ll)],
            capture_output=True,
        )
        if ll.exists():
            text = ll.read_text(errors="ignore")
            markers["GOMP"] = text.count("GOMP_parallel")
            markers["pfr"] = text.count("call void @parallel_for_runtime")
            markers["doacross"] = text.count("doacross.")
            (case / "markers.txt").write_text(
                f"GOMP={markers['GOMP']} pfr={markers['pfr']} doacross={markers['doacross']}\n"
            )
    if proc.returncode != 0 or not (ROOT / "program.o").exists():
        return None, markers

    bin_path = case / "a.out"
    link = [
        "g++",
        "-O3",
        "-fopenmp",
        "-no-pie",
        str(ROOT / "program.o"),
    ] + [str(ROOT / o) for o in objs]
    # nlopt if parallel_runtime needs it
    link += ["-ldl", "-lnlopt", "-lgomp", "-lpthread", "-o", str(bin_path)]
    lr = subprocess.run(link, cwd=ROOT, capture_output=True, text=True)
    (case / "link.log").write_text(lr.stdout + lr.stderr)
    if lr.returncode != 0 or not bin_path.exists():
        return None, markers
    return bin_path, markers


def time_bin(bin_path: Path, cwd: Path, env: dict | None = None) -> list[float]:
    times = []
    run_env = env if env is not None else os.environ.copy()
    for i in range(WARMUP + RUNS):
        t0 = time.perf_counter()
        proc = subprocess.run(
            [str(bin_path)],
            cwd=cwd,
            env=run_env,
            capture_output=True,
            text=True,
            timeout=600,
        )
        dt = time.perf_counter() - t0
        if proc.returncode != 0:
            raise RuntimeError(proc.stderr[-400:] or proc.stdout[-400:])
        if i >= WARMUP:
            times.append(dt)
    return times


def plot_results(rows: list[dict]) -> None:
    kernels = list(KERNELS.keys())
    modes = list(MODES)
    # speedup matrix
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.2), constrained_layout=True)

    # --- grouped speedup bars ---
    ax = axes[0]
    x = np.arange(len(kernels))
    width = 0.18
    colors = {
        "serial": "#9aa0a6",
        "pdg": "#1b7f7a",
        "polly": "#c45c26",
        "both": "#2f4b7c",
    }
    labels = {
        "serial": "serial (no Polly, no PDG)",
        "pdg": "PDG only (DOALL/DOACROSS)",
        "polly": "Polly only",
        "both": "Polly + PDG",
    }
    for i, mode in enumerate(modes):
        ys = []
        yerr = []
        for k in kernels:
            recs = [r for r in rows if r["kernel"] == k and r["mode"] == mode and r["ok"]]
            if not recs:
                ys.append(0.0)
                yerr.append(0.0)
                continue
            ys.append(recs[0]["speedup"])
            yerr.append(recs[0].get("speedup_std", 0.0))
        ax.bar(
            x + (i - 1.5) * width,
            ys,
            width,
            yerr=yerr,
            label=labels[mode],
            color=colors[mode],
            capsize=2,
            edgecolor="white",
            linewidth=0.4,
        )
    ax.axhline(1.0, color="#666", lw=0.8, ls="--")
    ax.set_xticks(x)
    ax.set_xticklabels(
        ["DOALL\nsaxpy", "DOACROSS\nprefix", "Polly\nmatmul", "Combo\nall three"],
        fontsize=9,
    )
    ax.set_ylabel("Speedup vs serial")
    ax.set_title("Parallelization boost by engine")
    ax.legend(fontsize=8, frameon=False, loc="upper left")
    ax.set_ylim(0, max(2.0, max((r["speedup"] for r in rows if r["ok"]), default=2) * 1.15))

    # --- absolute time heatmap-ish table as bars for combo focus ---
    ax2 = axes[1]
    # For each kernel, show median ms
    for i, mode in enumerate(modes):
        ys = []
        for k in kernels:
            recs = [r for r in rows if r["kernel"] == k and r["mode"] == mode and r["ok"]]
            ys.append(recs[0]["median_ms"] if recs else 0.0)
        ax2.bar(
            x + (i - 1.5) * width,
            ys,
            width,
            label=labels[mode],
            color=colors[mode],
            edgecolor="white",
            linewidth=0.4,
        )
    ax2.set_xticks(x)
    ax2.set_xticklabels(
        ["DOALL\nsaxpy", "DOACROSS\nprefix", "Polly\nmatmul", "Combo\nall three"],
        fontsize=9,
    )
    ax2.set_ylabel("Median wall time (ms)")
    ax2.set_title("Absolute runtime")
    ax2.legend(fontsize=8, frameon=False)

    fig.suptitle(
        f"p1 Autotuner parallel ablation  (runs={RUNS}, saxpy/prefix n≈{SAXPY_N}, matmul n={MATMUL_N})",
        fontsize=11,
        y=1.02,
    )
    out_png = OUT / "parallel_boost.png"
    out_pdf = OUT / "parallel_boost.pdf"
    fig.savefig(out_png, dpi=160, bbox_inches="tight")
    fig.savefig(out_pdf, bbox_inches="tight")
    plt.close(fig)

    # Second figure: combo-only breakdown annotation
    fig2, ax = plt.subplots(figsize=(6.5, 3.8), constrained_layout=True)
    combo_rows = [r for r in rows if r["kernel"] == "combo_all3" and r["ok"]]
    if combo_rows:
        order = [m for m in MODES if any(r["mode"] == m for r in combo_rows)]
        vals = []
        for m in order:
            r = next(x for x in combo_rows if x["mode"] == m)
            vals.append(r["speedup"])
        bars = ax.bar(
            [labels[m].replace(" (", "\n(") for m in order],
            vals,
            color=[colors[m] for m in order],
            edgecolor="white",
        )
        ax.axhline(1.0, color="#666", lw=0.8, ls="--")
        ax.set_ylabel("Speedup vs serial")
        ax.set_title(f"Combo kernel (matmul + saxpy + prefix), n={COMBO_N}")
        for b, v in zip(bars, vals):
            ax.text(
                b.get_x() + b.get_width() / 2,
                v + 0.03,
                f"{v:.2f}×",
                ha="center",
                va="bottom",
                fontsize=9,
            )
    fig2.savefig(OUT / "combo_boost.png", dpi=160, bbox_inches="tight")
    fig2.savefig(OUT / "combo_boost.pdf", bbox_inches="tight")
    plt.close(fig2)
    print(f"Wrote {out_png}")
    print(f"Wrote {OUT / 'combo_boost.png'}")


def main() -> int:
    bump_stack(2048)
    OUT.mkdir(parents=True, exist_ok=True)
    if not (ROOT / "GraphProgram").exists():
        print("GraphProgram missing — rebuild first", file=sys.stderr)
        return 1
    stub = ROOT / "bfs_runtime.ll"
    if not stub.exists():
        stub.write_text(
            '; ModuleID = "stub"\nsource_filename = "stub.c"\n'
            'target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"\n'
            'target triple = "x86_64-unknown-linux-gnu"\n'
        )

    objs = ensure_link_objs()
    print(f"OUT={OUT}")
    print(f"RUNS={RUNS} sizes: saxpy/prefix={SAXPY_N} matmul={MATMUL_N} combo={COMBO_N}")
    print(f"link objs: {objs}")
    print(f"OMP_NUM_THREADS default={os.cpu_count()}")

    rows = []
    serial_med: dict[str, float] = {}

    # Compile & time: serial first per kernel for baseline
    for kernel, (n, fn) in KERNELS.items():
        for mode in MODES:
            print(f"  compile {kernel:18s} {mode:7s} n={n} ...", flush=True)
            bin_path, markers = compile_bin(kernel, mode, n, fn, objs)
            mark_s = f"GOMP={markers['GOMP']} pfr={markers['pfr']} dx={markers['doacross']}"
            if not bin_path:
                print(f"    FAIL compile/link  ({mark_s})")
                rows.append(
                    {
                        "kernel": kernel,
                        "mode": mode,
                        "n": n,
                        "ok": False,
                        "median_s": None,
                        "median_ms": None,
                        "speedup": 0.0,
                        "speedup_std": 0.0,
                        "times": "",
                        "GOMP": markers["GOMP"],
                        "pfr": markers["pfr"],
                        "doacross": markers["doacross"],
                    }
                )
                continue
            try:
                times = time_bin(bin_path, bin_path.parent, env=mode_env(mode))
            except Exception as e:
                print(f"    FAIL run: {e}  ({mark_s})")
                rows.append(
                    {
                        "kernel": kernel,
                        "mode": mode,
                        "n": n,
                        "ok": False,
                        "median_s": None,
                        "median_ms": None,
                        "speedup": 0.0,
                        "speedup_std": 0.0,
                        "times": "",
                        "GOMP": markers["GOMP"],
                        "pfr": markers["pfr"],
                        "doacross": markers["doacross"],
                    }
                )
                continue
            med = statistics.median(times)
            if mode == "serial":
                serial_med[kernel] = med
            spd = (serial_med.get(kernel, med) / med) if med > 0 else 0.0
            # rough std on speedup from time std
            if len(times) > 1 and med > 0 and kernel in serial_med:
                spd_samples = [serial_med[kernel] / t for t in times if t > 0]
                spd_std = statistics.stdev(spd_samples) if len(spd_samples) > 1 else 0.0
            else:
                spd_std = 0.0
            print(f"    {med*1000:.1f} ms  speedup={spd:.2f}×  ({mark_s})")
            rows.append(
                {
                    "kernel": kernel,
                    "mode": mode,
                    "n": n,
                    "ok": True,
                    "median_s": med,
                    "median_ms": med * 1000.0,
                    "speedup": spd,
                    "speedup_std": spd_std,
                    "times": ";".join(f"{t:.6f}" for t in times),
                    "GOMP": markers["GOMP"],
                    "pfr": markers["pfr"],
                    "doacross": markers["doacross"],
                }
            )

    # Recompute speedups now that all serials known (in case order was mixed — we did serial first within each kernel)
    for r in rows:
        if r["ok"] and r["kernel"] in serial_med and r["median_s"]:
            r["speedup"] = serial_med[r["kernel"]] / r["median_s"]

    csv_path = OUT / "summary.csv"
    with csv_path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)

    plot_results(rows)

    md = OUT / "REPORT.md"
    lines = [
        "# Parallel boost ablation",
        "",
        f"Runs={RUNS}, saxpy/prefix n={SAXPY_N}, matmul n={MATMUL_N}, combo n={COMBO_N}",
        "",
        "| kernel | mode | median ms | speedup | GOMP | pfr | doacross |",
        "|---|---|---:|---:|---:|---:|---:|",
    ]
    for r in rows:
        if not r["ok"]:
            lines.append(
                f"| {r['kernel']} | {r['mode']} | FAIL | | {r.get('GOMP',0)} | {r.get('pfr',0)} | {r.get('doacross',0)} |"
            )
        else:
            lines.append(
                f"| {r['kernel']} | {r['mode']} | {r['median_ms']:.1f} | {r['speedup']:.2f}× | {r.get('GOMP',0)} | {r.get('pfr',0)} | {r.get('doacross',0)} |"
            )
    lines += [
        "",
        "## Modes",
        "- **serial**: no Polly, no PDG/outliner",
        "- **pdg**: DOALL/DOACROSS outlining only (`GRAPH_DISABLE_POLLY=1`)",
        "- **polly**: Polly O3 + `-polly-parallel` only (`GRAPH_DISABLE_PDG=1`)",
        "- **both**: Polly + PDG",
        "",
        "## Notes",
        "- `SGPL_FORCE_DOALL_PARALLEL=1` is set for `pdg`/`both` so single-shot DOALL",
        "  kernels skip cost-model warmup (otherwise they stay serial within one process).",
        "- DOACROSS is left to the cost model (forcing it on multi-million prefixes hangs).",
        "- Nested-loop pool miss previously forced `P=1`; fixed in `parallel_runtime.c`.",
        "",
        f"Plots: `parallel_boost.png`, `combo_boost.png`",
    ]
    md.write_text("\n".join(lines))
    print(f"Wrote {csv_path}\nWrote {md}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
