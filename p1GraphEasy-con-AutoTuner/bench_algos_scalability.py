#!/usr/bin/env python3
"""
bench_algos_scalability.py — scalability benchmark for the CleanCut effect
algebra + composable runtime over the DSL algorithm corpus.

For every algorithm x graph size the harness:

  1. generates a deterministic synthetic edge list (unweighted + weighted),
  2. prebuilds the .sgplbin sidecar so every launch hits the warm mmap path,
  3. compiles the algorithm twice — rewrite ON (effect-algebra lowering) and
     rewrite OFF (sequential fallback) — and links both against the current
     runtime objects,
  4. validates that the ON (parallel) and OFF (sequential) outputs are
     bit-identical,
  5. measures end-to-end wall time for a size sweep and a thread sweep, and
     attributes graph-load time with a load-only program.

Artifacts land in bench_results/algos_scalability_<stamp>/: CSV tables, SVG
plots, per-build GRAPH_FRONTIER_STATS logs and a markdown summary.

Usage (inside WSL, from the repo root):
  python3 bench_algos_scalability.py                 # full matrix
  python3 bench_algos_scalability.py --quick         # small smoke matrix
  python3 bench_algos_scalability.py --algos cc,pagerank --sizes 100000:100000
"""

import argparse
import csv
import os
import random
import re
import resource
import shutil
import statistics
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent

ALGOS = {
    "cc": {
        "file": "algos/cc.graph",
        "weighted": False,
        # Hand-rolled serial accumulation: the `for each vertex` scalar
        # reduction can be outlined by the PDG and is not reliable as a
        # checksum (observed ~1/300 wrong values at 10 threads).
        "checksum": (
            "int checksum = 0;\n"
            "int ci = 0;\n"
            "while (ci < n) {\n"
            "  checksum = checksum + id[ci];\n"
            "  ci = ci + 1;\n"
            "}\n"
            "print checksum;\n"
        ),
    },
    "kcore": {
        "file": "algos/kcore.graph",
        "weighted": False,
        # k=5 empties the core on the sparse benchmark graphs, making the
        # output checksum trivially zero; k=2 leaves a non-trivial 2-core.
        "patches": [(r"int k = 5;", "int k = 2;")],
        # alive[] is the k-core result; residual deg[] of removed vertices is
        # schedule-dependent by design.
        "checksum": (
            "int checksum = 0;\n"
            "int ci = 0;\n"
            "while (ci < n) {\n"
            "  checksum = checksum + alive[ci];\n"
            "  ci = ci + 1;\n"
            "}\n"
            "print checksum;\n"
        ),
    },
    "bfs_parent": {
        "file": "algos/bfs_parent.graph",
        "weighted": False,
        # The legacy (non-CleanCut) path discovers vertices with a CAS race,
        # so the parent tree is schedule-dependent; reachability is not.
        "checksum": (
            "int checksum = 0;\n"
            "int ci = 0;\n"
            "while (ci < n) {\n"
            "  if (visited[ci] == 1) {\n"
            "    checksum = checksum + 1;\n"
            "  }\n"
            "  ci = ci + 1;\n"
            "}\n"
            "print checksum;\n"
        ),
    },
    "pagerank": {
        "file": "algos/pagerank.graph",
        "weighted": False,
        "checksum": (
            "print cur_rank[0];\n"
            "print cur_rank[1];\n"
            "print cur_rank[n - 1];\n"
        ),
    },
    "sssp": {
        "file": "algos/sssp.graph",
        "weighted": True,
        "checksum": (
            "int checksum = 0;\n"
            "int ci = 0;\n"
            "while (ci < n) {\n"
            "  if (dist[ci] != inf) {\n"
            "    checksum = checksum + dist[ci];\n"
            "  }\n"
            "  ci = ci + 1;\n"
            "}\n"
            "print checksum;\n"
        ),
    },
    "pagerank_dataidx": {
        "file": "algos/pagerank_dataidx.graph",
        "weighted": False,
        "checksum": (
            "print cur_rank[0];\n"
            "print cur_rank[1];\n"
            "print cur_rank[n - 1];\n"
        ),
    },
    # Set-frontier BFS: same algorithm as bfs_parent but expressed with the
    # `for each element` / set-append shape CleanCut recognizes (bfs_parent's
    # array-frontier `while` driver is not detected by the pass).
    "bfs_set": {
        "weighted": False,
        "source": (
            'graph G { edges: file "__EDGE__"; };\n'
            "int n = numVertices(G);\n"
            "int visited[n];\n"
            "int parent[n];\n"
            "set frontier;\n"
            "set next;\n"
            "\n"
            "for each vertex v in G {\n"
            "  visited[v] = 0;\n"
            "  parent[v] = -1;\n"
            "}\n"
            "\n"
            "frontier = {0};\n"
            "visited[0] = 1;\n"
            "parent[0] = 0;\n"
            "\n"
            "while (setSize(frontier) > 0) {\n"
            "  next = {};\n"
            "  for each element v in frontier {\n"
            "    for each neighbor u of v in G {\n"
            "      if (visited[u] == 0) {\n"
            "        visited[u] = 1;\n"
            "        parent[u] = v;\n"
            "        next.add(u);\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  frontier = next;\n"
            "}\n"
            "\n"
            "int checksum = 0;\n"
            "int ci = 0;\n"
            "while (ci < n) {\n"
            "  checksum = checksum + parent[ci];\n"
            "  ci = ci + 1;\n"
            "}\n"
            "print checksum;\n"
        ),
    },
}

RUNTIME_OBJS = [
    "runtime.o",
    "parallel_runtime.o",
    "gpu_runtime.o",
    "autotuner_runtime.o",
    "graph_mutation_runtime.o",
    "roaring_bitmap.o",
    "graph_loader_runtime.o",
    "graph_runtime.o",
    "semiring_runtime.o",
]

LOAD_PROGRAM = (
    'graph G {{ edges: file "{edge}"; {weighted} }};\n'
    "int n = numVertices(G);\n"
    "print n;\n"
)

EDGE_RE = re.compile(r'edges:\s*file\s*"([^"]+)"')


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

def run(cmd, cwd=ROOT, env=None, check=True, timeout=None, quiet=True):
    merged = os.environ.copy()
    if env:
        merged.update(env)
    proc = subprocess.run(
        [str(c) for c in cmd], cwd=str(cwd), env=merged,
        capture_output=True, text=True, timeout=timeout,
    )
    if check and proc.returncode != 0:
        tail = (proc.stderr or proc.stdout or "")[-3000:]
        raise RuntimeError(f"command failed ({proc.returncode}): {' '.join(map(str, cmd))}\n{tail}")
    if not quiet and proc.stdout:
        sys.stdout.write(proc.stdout)
    return proc


def fmt_ms(ms):
    if ms is None or ms < 0:
        return "-"
    if ms < 1.0:
        return f"{ms:.3f}"
    if ms < 100.0:
        return f"{ms:.1f}"
    return f"{ms:.0f}"


def fmt_size(n):
    if n >= 1_000_000:
        return f"{n / 1_000_000:.3g}M"
    if n >= 1000:
        return f"{n / 1000:.3g}k"
    return str(n)


def median(values):
    return statistics.median(values) if values else None


# ---------------------------------------------------------------------------
# build steps
# ---------------------------------------------------------------------------

def build_runtimes():
    print("[build] runtime objects ...", flush=True)
    run(["gcc", "-O3", "-c", "autotuner_runtime.c", "-o", "autotuner_runtime.o"])
    run(["gcc", "-O3", "-c", "graph_mutation_runtime.c", "-o", "graph_mutation_runtime.o"])
    run(["gcc", "-O3", "-c", "parallel_runtime.c", "-o", "parallel_runtime.o"])
    run(["gcc", "-O3", "-c", "gpu_runtime.c", "-o", "gpu_runtime.o"])
    run(["gcc", "-O3", "-c", "runtime.c", "-o", "runtime.o"])
    run(["gcc", "-O3", "-fopenmp", "-iquote", ".", "-c", "semiring_runtime.c",
         "-o", "semiring_runtime.o"], check=False)
    run(["g++", "-O3", "-mavx2", "-march=native", "-fopenmp",
         "-c", "roaring_bitmap.cpp", "-o", "roaring_bitmap.o"])
    run(["g++", "-O2", "-std=c++17", "-fopenmp",
         "-c", "graph_loader_runtime.cpp", "-o", "graph_loader_runtime.o"])
    run(["g++", "-O2", "-std=c++17", "-c", "graph_runtime.cpp", "-o", "graph_runtime.o"])
    print("[build] runtime objects done", flush=True)


def ensure_hw_calib():
    calib = Path.home() / ".config" / "sgpl" / "hw_calib.json"
    if calib.exists():
        return
    print("[build] hardware calibration ...", flush=True)
    calib.parent.mkdir(parents=True, exist_ok=True)
    if not (ROOT / "hw_calib_bench").exists():
        run(["gcc", "-O2", "-o", "hw_calib_bench", "hw_calib_bench.c"])
    proc = run(["./hw_calib_bench"])
    calib.write_text(proc.stdout)


def ensure_prebuild_tool():
    tool = ROOT / "tools" / "prebuild_graph_cache"
    if tool.exists():
        return tool
    print("[build] prebuild_graph_cache ...", flush=True)
    run(["g++", "-O2", "-std=c++17", "-fopenmp", "-mavx2", "-march=native",
         "tools/prebuild_graph_cache.cpp", "graph_loader_runtime.cpp",
         "roaring_bitmap.cpp", "autotuner_runtime.o", "parallel_runtime.o",
         "-lnlopt", "-o", "tools/prebuild_graph_cache"])
    return tool


def runtime_obj_paths():
    return [ROOT / o for o in RUNTIME_OBJS if (ROOT / o).exists()]


def link_program(obj, bin_path):
    cmd = ["g++", "-O3", "-fopenmp", "-no-pie", str(obj)]
    cmd += [str(o) for o in runtime_obj_paths()]
    cmd += ["-ldl", "-lnlopt", "-o", str(bin_path)]
    run(cmd)


# ---------------------------------------------------------------------------
# graph generation / cache
# ---------------------------------------------------------------------------

def generate_edges(path, n, m, weighted, seed):
    if path.exists():
        return False
    rng = random.Random(seed)
    edges = set()
    while len(edges) < m:
        u = rng.randint(0, n - 1)
        v = rng.randint(0, n - 1)
        if u == v:
            continue
        if u > v:
            u, v = v, u
        edges.add((u, v))
    ordered = sorted(edges)
    with path.open("w") as f:
        for u, v in ordered:
            if weighted:
                f.write(f"{u} {v} {rng.randint(1, 100)}\n")
            else:
                f.write(f"{u} {v}\n")
    return True


def cache_path(edge_path, weighted):
    return Path(str(edge_path) + (".weighted" if weighted else "") + ".sgplbin")


def prebuild_cache(tool, edge_path, weighted):
    if cache_path(edge_path, weighted).exists():
        return
    cmd = [str(tool), "--refresh"]
    if weighted:
        cmd.append("--weighted")
    cmd.append(str(edge_path))
    print(f"[cache] prebuild {edge_path.name}{' (weighted)' if weighted else ''} ...", flush=True)
    run(cmd)


# ---------------------------------------------------------------------------
# workload generation / compilation
# ---------------------------------------------------------------------------

def make_workload(algo, edge_path, out_path):
    spec = ALGOS[algo]
    if "source" in spec:
        out_path.write_text(spec["source"].replace("__EDGE__", str(edge_path)))
        return
    src = (ROOT / spec["file"]).read_text()
    new_src, count = EDGE_RE.subn(f'edges: file "{edge_path}"', src, count=1)
    if count != 1:
        raise RuntimeError(f"could not rewrite edge path in {spec['file']}")
    for pattern, replacement in spec.get("patches", []):
        new_src, n_patch = re.subn(pattern, replacement, new_src)
        if n_patch != 1:
            raise RuntimeError(f"patch '{pattern}' matched {n_patch} times in {algo}")
    out_path.write_text(new_src + "\n" + spec["checksum"])


def make_load_workload(edge_path, out_path, weighted=False):
    out_path.write_text(LOAD_PROGRAM.format(edge=edge_path,
                                            weighted="TRUE" if weighted else ""))


def compile_program(graph_path, mode, stats_path, timeout):
    env = {
        "GRAPH_FRONTIER_STATS": "1",
        "GRAPH_FRONTIER_VERBOSE": "1",
    }
    if mode == "off":
        env["GRAPH_FRONTIER_REWRITE_OFF"] = "1"
    proc = run(["./GraphProgram", "--ir-backend=cpu", str(graph_path)],
               env=env, timeout=timeout)
    if stats_path is not None:
        stats_path.write_text(proc.stderr or "")
    obj = ROOT / "program.o"
    if not obj.exists():
        raise RuntimeError("GraphProgram produced no program.o")
    return obj


# ---------------------------------------------------------------------------
# measurement
# ---------------------------------------------------------------------------

def config_env(config, threads):
    """Environment overrides for a thread-sweep configuration.

    `default` is the as-shipped configuration (the scheduler now clamps the
    block-cyclic chunk adaptively so small trip counts, e.g. the CleanCut
    partition loop, still spread over all workers).  `tuned` additionally
    forces a 4-iteration chunk on large loops and one partition per worker.
    """
    if config == "default":
        return {}
    if config == "tuned":
        return {
            "SGPL_LOOP_CHUNK": "4",
            "SGPL_CLEANCUT_PARTITIONS": str(threads),
        }
    raise ValueError(f"unknown thread config '{config}'")


def run_once(binary, threads, timeout, extra_env=None):
    env = {
        "SGPL_NUM_THREADS": str(threads),
        "OMP_NUM_THREADS": str(threads),
        "SGPL_ROARING_THREADS": "4",
    }
    if extra_env:
        env.update(extra_env)
    start = time.perf_counter()
    proc = run([str(binary)], env=env, check=False, timeout=timeout)
    elapsed_ms = (time.perf_counter() - start) * 1000.0
    if proc.returncode != 0:
        raise RuntimeError(f"{binary} exited {proc.returncode}: {proc.stderr[-1500:]}")
    checksum = "\n".join(line for line in proc.stdout.splitlines() if line.strip())
    return elapsed_ms, checksum


def measure(binary, threads, rounds, timeout):
    """warmup + `rounds` timed runs; returns (median_ms, min_ms, checksum)."""
    checksum = None
    times = []
    for i in range(rounds + 1):
        ms, ck = run_once(binary, threads, timeout)
        if checksum is None:
            checksum = ck
        elif ck != checksum:
            raise RuntimeError(f"nondeterministic output at {threads} threads")
        if i > 0:
            times.append(ms)
    return median(times), min(times), checksum


def measure_thread_sweep(binary, thread_counts, rounds, timeout, rng,
                         expected_checksum=None, config="default"):
    """Warm every config, then sample with a shuffled order per round."""
    samples = {t: [] for t in thread_counts}
    checksums = {}
    for t in thread_counts:
        _, ck = run_once(binary, t, timeout, config_env(config, t))
        checksums[t] = ck
    for _ in range(rounds):
        order = list(thread_counts)
        rng.shuffle(order)
        for t in order:
            ms, ck = run_once(binary, t, timeout, config_env(config, t))
            samples[t].append(ms)
            if ck != checksums[t]:
                raise RuntimeError(f"nondeterministic output at {t} threads")
    if expected_checksum is not None:
        for t, ck in checksums.items():
            if ck != expected_checksum:
                raise RuntimeError(f"checksum differs at {t} threads")
    return samples, checksums


# ---------------------------------------------------------------------------
# SVG plotting (stdlib only)
# ---------------------------------------------------------------------------

def line_chart(title, x_labels, series, y_label, y_log=False, width=980, height=420):
    left, right, top, bottom = 78, 26, 52, 62
    pw, ph = width - left - right, height - top - bottom
    all_values = [v for _, ys, _ in series for v in ys if v is not None]
    if not all_values:
        all_values = [0.0, 1.0]
    lo = min(all_values)
    hi = max(all_values)
    if y_log:
        lo = max(lo, 1e-3)
        import math
        llo, lhi = math.log10(lo), math.log10(hi)
        if lhi - llo < 0.5:
            llo, lhi = llo - 0.25, lhi + 0.25
        pad = (lhi - llo) * 0.08
        llo, lhi = llo - pad, lhi + pad
        yp = lambda v: top + ph * (lhi - math.log10(max(v, 1e-3))) / (lhi - llo)
    else:
        pad = max((hi - lo) * 0.1, hi * 0.03, 1e-9)
        lo, hi = max(0.0, lo - pad), hi + pad
        yp = lambda v: top + ph * (hi - v) / (hi - lo)
    n = max(1, len(x_labels) - 1)
    xp = lambda i: left + pw * i / n
    out = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
        f'viewBox="0 0 {width} {height}">',
        '<style>text{font:12px system-ui;fill:#26303b}.title{font-size:16px;font-weight:600}'
        '.grid{stroke:#d8dde5;stroke-width:1}</style>',
        f'<text x="{left}" y="26" class="title">{title}</text>',
    ]
    for i in range(5):
        frac = i / 4.0
        if y_log:
            import math
            v = 10 ** (llo + (lhi - llo) * frac)
            label = f"{v:.3g}"
        else:
            v = lo + (hi - lo) * frac
            label = fmt_ms(v)
        y = yp(v)
        out.append(f'<line x1="{left}" y1="{y:.1f}" x2="{width - right}" y2="{y:.1f}" class="grid"/>')
        out.append(f'<text x="{left - 8}" y="{y + 4:.1f}" text-anchor="end">{label}</text>')
    for i, x in enumerate(x_labels):
        out.append(f'<text x="{xp(i):.1f}" y="{height - 34}" text-anchor="middle">{x}</text>')
    out.append(
        f'<text x="16" y="{top + ph / 2:.1f}" transform="rotate(-90 16 {top + ph / 2:.1f})" '
        f'text-anchor="middle">{y_label}</text>')
    legend_x = left
    for name, ys, color in series:
        pts = [(i, v) for i, v in enumerate(ys) if v is not None]
        if len(pts) >= 2:
            points = " ".join(f"{xp(i):.1f},{yp(v):.1f}" for i, v in pts)
            out.append(f'<polyline points="{points}" fill="none" stroke="{color}" stroke-width="3"/>')
        for i, v in pts:
            out.append(f'<circle cx="{xp(i):.1f}" cy="{yp(v):.1f}" r="4" fill="{color}"/>')
        out.append(f'<text x="{legend_x}" y="{height - 8}" fill="{color}">{name}</text>')
        legend_x += 150
    out.append("</svg>")
    return "\n".join(out)


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def parse_sizes(spec):
    sizes = []
    for part in spec.split(","):
        part = part.strip()
        if not part:
            continue
        n, m = part.split(":")
        sizes.append((int(n), int(m)))
    return sizes


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--algos", default=",".join(ALGOS),
                        help=f"comma list from {','.join(ALGOS)}")
    parser.add_argument("--sizes", default="125000:125000,250000:250000,500000:500000,1000000:1000000",
                        help="comma list of n:m pairs (vertices:undirected edges)")
    parser.add_argument("--threads", default="1,2,4,6,8,10,12",
                        help="thread counts for the thread sweep")
    parser.add_argument("--thread-configs", default="default,tuned",
                        help="thread sweep configurations: default (as shipped), "
                             "tuned (SGPL_LOOP_CHUNK=4, partitions=threads)")
    parser.add_argument("--sweep-threads", type=int, default=12,
                        help="thread count used for the size sweep")
    parser.add_argument("--rounds", type=int, default=3, help="timed rounds (median reported)")
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--timeout", type=float, default=900.0, help="per-run timeout (s)")
    parser.add_argument("--outdir", default=None)
    parser.add_argument("--skip-build", action="store_true", help="reuse existing runtime objects")
    parser.add_argument("--skip-off", action="store_true", help="skip the sequential baseline")
    parser.add_argument("--skip-thread-sweep", action="store_true")
    parser.add_argument("--quick", action="store_true",
                        help="small smoke matrix: 2 algos, 1 size, 3 thread counts, 1 round")
    args = parser.parse_args()

    if args.quick:
        args.algos = "cc,pagerank"
        args.sizes = "20000:20000"
        args.threads = "1,2,4"
        args.sweep_threads = 4
        args.rounds = 1

    algos = [a.strip() for a in args.algos.split(",") if a.strip()]
    for a in algos:
        if a not in ALGOS:
            raise SystemExit(f"unknown algo '{a}' (have: {', '.join(ALGOS)})")
    sizes = parse_sizes(args.sizes)
    thread_counts = [int(t) for t in args.threads.split(",") if t.strip()]

    stamp = time.strftime("%Y%m%d_%H%M%S")
    outdir = Path(args.outdir) if args.outdir else ROOT / "bench_results" / f"algos_scalability_{stamp}"
    outdir = outdir.resolve()
    graphs_dir = outdir / "graphs"
    stats_dir = outdir / "stats"
    bin_dir = outdir / "bin"
    for d in (outdir, graphs_dir, stats_dir, bin_dir):
        d.mkdir(parents=True, exist_ok=True)

    print("=" * 78)
    print("  CleanCut / composable-runtime algorithm scalability benchmark")
    print(f"  algos   : {', '.join(algos)}")
    print(f"  sizes   : {', '.join(f'{n}:{m}' for n, m in sizes)}")
    print(f"  threads : {thread_counts}")
    print(f"  rounds  : {args.rounds} (+1 warmup)")
    print(f"  outdir  : {outdir}")
    print("=" * 78, flush=True)

    # The CleanCut runtime emits per-round state on the driver stack; the
    # project's own scripts all run with an unlimited stack (03_run.sh).
    try:
        resource.setrlimit(resource.RLIMIT_STACK, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))
    except (ValueError, OSError):
        pass

    host_info = []
    for cmd in (["uname", "-a"], ["nproc"], ["lscpu"], ["free", "-h"],
                ["git", "rev-parse", "HEAD"], ["git", "status", "--short"]):
        try:
            proc = subprocess.run(cmd, cwd=str(ROOT), capture_output=True, text=True)
            host_info.append(f"$ {' '.join(cmd)}\n{proc.stdout.strip()}\n")
        except Exception:
            pass
    (outdir / "host.txt").write_text("\n".join(host_info))

    if not args.skip_build:
        build_runtimes()
    ensure_hw_calib()
    tool = ensure_prebuild_tool()

    # ---- generate graphs + caches -----------------------------------------
    edge_files = {}
    for n, m in sizes:
        unweighted = graphs_dir / f"synth_v_{n}_e_{m}_bench.txt"
        weighted = graphs_dir / f"synth_v_{n}_e_{m}_bench_w.txt"
        generate_edges(unweighted, n, m, False, args.seed)
        prebuild_cache(tool, unweighted, False)
        edge_files[(n, m, False)] = unweighted
        if any(ALGOS[a]["weighted"] for a in algos):
            generate_edges(weighted, n, m, True, args.seed)
            prebuild_cache(tool, weighted, True)
            edge_files[(n, m, True)] = weighted

    # ---- load-only baseline ------------------------------------------------
    print("\n[load] graph-load-only baseline", flush=True)
    load_times = {}  # (n, m, weighted, threads) -> median ms
    load_checksums = {}
    for n, m in sizes:
        for weighted in (False, True):
            if (n, m, weighted) not in edge_files:
                continue
            suffix = "_w" if weighted else ""
            graph = outdir / f"bench_load_{n}_{m}{suffix}.graph"
            make_load_workload(edge_files[(n, m, weighted)], graph, weighted)
            obj = compile_program(graph, "on", None, args.timeout)
            obj_path = bin_dir / f"load_{n}_{m}{suffix}.o"
            bin_path = bin_dir / f"final_bench_load_{n}_{m}{suffix}"
            shutil.move(str(obj), str(obj_path))
            link_program(obj_path, bin_path)
            counts = {1, args.sweep_threads}
            if (n, m) == sizes[-1]:
                counts |= set(thread_counts)
            for t in sorted(counts):
                ms, min_ms, ck = measure(bin_path, t, args.rounds, args.timeout)
                load_times[(n, m, weighted, t)] = ms
                load_checksums[(n, m, weighted)] = ck
            print(f"  n={n} m={m}{' w' if weighted else '  '} "
                  f"load@1={fmt_ms(load_times[(n, m, weighted, 1)])}ms "
                  f"load@{args.sweep_threads}={fmt_ms(load_times[(n, m, weighted, args.sweep_threads)])}ms",
                  flush=True)

    # ---- compile / link ----------------------------------------------------
    print("\n[compile] workloads (ON = effect-algebra lowering, OFF = sequential)", flush=True)
    builds = {}  # (algo, n, m, mode) -> dict(bin, stats, checksum)
    modes = ["on"] if args.skip_off else ["on", "off"]
    for algo in algos:
        spec = ALGOS[algo]
        for n, m in sizes:
            edge = edge_files[(n, m, spec["weighted"])]
            tag = f"{algo}_{n}_{m}"
            graph = outdir / f"bench_{tag}.graph"
            make_workload(algo, edge, graph)
            for mode in modes:
                stats = stats_dir / f"{tag}_{mode}.stats"
                obj = compile_program(graph, mode, stats, args.timeout)
                obj_path = bin_dir / f"{tag}_{mode}.o"
                shutil.move(str(obj), str(obj_path))
                bin_path = bin_dir / f"final_bench_{tag}_{mode}"
                link_program(obj_path, bin_path)
                builds[(algo, n, m, mode)] = {"bin": bin_path, "stats": stats}
            print(f"  {algo:16s} n={n:>8d} m={m:>8d} compiled", flush=True)

    # ---- correctness: ON vs OFF checksums ----------------------------------
    # ON at 1 thread exercises the executor with the minimum partition count,
    # ON at sweep threads exercises the full parallel schedule, OFF at 1 thread
    # is the sequential reference.  All three must agree bit-for-bit.
    print("\n[validate] parallel (ON) vs sequential (OFF) checksums", flush=True)
    validation = {}
    ref_checksums = {}
    errors = {}
    for algo in algos:
        for n, m in sizes:
            try:
                on_bin = builds[(algo, n, m, "on")]["bin"]
                _, on1_ck = run_once(on_bin, 1, args.timeout)
                _, onN_ck = run_once(on_bin, args.sweep_threads, args.timeout)
                ok = (on1_ck == onN_ck)
                ref_checksums[(algo, n, m)] = on1_ck
                if not args.skip_off:
                    off_bin = builds[(algo, n, m, "off")]["bin"]
                    _, off_ck = run_once(off_bin, 1, args.timeout)
                    ok = ok and (off_ck == on1_ck)
                validation[(algo, n, m)] = ok
                status = "MATCH" if ok else "MISMATCH"
            except Exception as exc:
                validation[(algo, n, m)] = False
                ref_checksums[(algo, n, m)] = None
                errors[(algo, n, m)] = str(exc)
                status = f"ERROR ({str(exc).splitlines()[0][:80]})"
            print(f"  {algo:16s} n={n:>8d} {status}", flush=True)
    if not all(validation.values()):
        print("!! checksum mismatches / errors detected — results below are suspect !!", flush=True)

    # ---- size sweep ---------------------------------------------------------
    print(f"\n[size sweep] rewrite ON at {args.sweep_threads} threads vs OFF at 1 thread",
          flush=True)
    size_rows = []
    for algo in algos:
        spec = ALGOS[algo]
        for n, m in sizes:
            weighted = spec["weighted"]
            on_ms = on_min = off_ms = off_min = None
            err = errors.get((algo, n, m))
            try:
                on_bin = builds[(algo, n, m, "on")]["bin"]
                on_ms, on_min, _ = measure(on_bin, args.sweep_threads, args.rounds, args.timeout)
                if not args.skip_off:
                    off_bin = builds[(algo, n, m, "off")]["bin"]
                    off_ms, off_min, _ = measure(off_bin, 1, args.rounds, args.timeout)
            except Exception as exc:
                err = str(exc)
            load_on = load_times.get((n, m, weighted, args.sweep_threads))
            load_off = load_times.get((n, m, weighted, 1))
            on_kernel = on_ms - load_on if (on_ms is not None and load_on is not None) else None
            off_kernel = off_ms - load_off if (off_ms is not None and load_off is not None) else None
            speedup = (off_kernel / on_kernel) if (on_kernel and off_kernel and on_kernel > 0) else None
            speedup_e2e = (off_ms / on_ms) if (off_ms and on_ms) else None
            size_rows.append({
                "algo": algo, "n": n, "m": m,
                "threads": args.sweep_threads,
                "on_ms": on_ms, "on_min_ms": on_min,
                "off_ms": off_ms, "off_min_ms": off_min,
                "load_on_ms": load_on, "load_off_ms": load_off,
                "on_kernel_ms": on_kernel, "off_kernel_ms": off_kernel,
                "speedup_kernel": speedup, "speedup_e2e": speedup_e2e,
                "checksum_ok": validation[(algo, n, m)],
                "error": err,
            })
            if speedup is not None:
                print(f"  {algo:16s} n={n:>8d} on={fmt_ms(on_ms):>8s}ms "
                      f"off={fmt_ms(off_ms):>8s}ms speedup={speedup:.2f}x", flush=True)
            elif on_ms is not None:
                print(f"  {algo:16s} n={n:>8d} on={fmt_ms(on_ms)}ms", flush=True)
            else:
                print(f"  {algo:16s} n={n:>8d} ERROR ({str(err).splitlines()[0][:80]})", flush=True)

    # ---- thread sweep -------------------------------------------------------
    thread_rows = []
    thread_configs = [c.strip() for c in args.thread_configs.split(",") if c.strip()]
    if not args.skip_thread_sweep:
        n, m = sizes[-1]
        print(f"\n[thread sweep] largest size n={n} m={m}, configs {thread_configs}",
              flush=True)
        rng = random.Random(args.seed)
        for config in thread_configs:
            for algo in algos:
                spec = ALGOS[algo]
                weighted = spec["weighted"]
                try:
                    on_bin = builds[(algo, n, m, "on")]["bin"]
                    samples, checksums = measure_thread_sweep(
                        on_bin, thread_counts, args.rounds, args.timeout, rng,
                        expected_checksum=ref_checksums[(algo, n, m)], config=config)
                except Exception as exc:
                    thread_rows.append({
                        "config": config, "algo": algo, "n": n, "m": m, "threads": None,
                        "median_ms": None, "min_ms": None,
                        "load_ms": None, "kernel_ms": None,
                        "speedup_vs_1t": None, "efficiency": None,
                        "checksum_ok": validation[(algo, n, m)],
                        "error": str(exc),
                    })
                    print(f"  {config:8s} {algo:16s} ERROR "
                          f"({str(exc).splitlines()[0][:80]})", flush=True)
                    continue
                base = median(samples[thread_counts[0]])
                for t in thread_counts:
                    med = median(samples[t])
                    mn = min(samples[t])
                    load = load_times.get((n, m, weighted, t))
                    kernel = med - load if load is not None else None
                    base_kernel = base - (load_times.get((n, m, weighted, thread_counts[0])) or 0.0)
                    speedup = base_kernel / kernel if (kernel and base_kernel and kernel > 0) else None
                    eff = (speedup / t) if speedup else None
                    thread_rows.append({
                        "config": config, "algo": algo, "n": n, "m": m, "threads": t,
                        "median_ms": med, "min_ms": mn,
                        "load_ms": load, "kernel_ms": kernel,
                        "speedup_vs_1t": speedup, "efficiency": eff,
                        "checksum_ok": validation[(algo, n, m)],
                        "error": None,
                    })
                rowtxt = "  ".join(
                    f"{t}T:{fmt_ms(median(samples[t]))}" for t in thread_counts)
                print(f"  {config:8s} {algo:16s} {rowtxt}", flush=True)

    # ---- write artifacts ----------------------------------------------------
    size_csv = outdir / "size_scaling.csv"
    with size_csv.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(size_rows[0].keys()))
        writer.writeheader()
        writer.writerows(size_rows)

    if thread_rows:
        thread_csv = outdir / "thread_scaling.csv"
        with thread_csv.open("w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=list(thread_rows[0].keys()))
            writer.writeheader()
            writer.writerows(thread_rows)

    # coverage: which effect-expression candidates each build lowered
    coverage_rows = []
    for algo in algos:
        for n, m in sizes:
            stats = builds[(algo, n, m, "on")]["stats"]
            text = stats.read_text(errors="replace") if stats.exists() else ""
            cands = [l.strip() for l in text.splitlines() if "candidate:" in l]
            for i, line in enumerate(cands):
                coverage_rows.append({"algo": algo, "n": n, "m": m, "candidate": i, "detail": line})
    coverage_csv = outdir / "effect_coverage.csv"
    with coverage_csv.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["algo", "n", "m", "candidate", "detail"])
        writer.writeheader()
        writer.writerows(coverage_rows)

    # ---- plots ---------------------------------------------------------------
    colors = ["#3478d4", "#d4553a", "#238b57", "#8a5cd4", "#c79a1f", "#0f8b8d"]
    if size_rows:
        labels = [fmt_size(n) for n, _ in sizes]
        series_on, series_off = [], []
        for idx, algo in enumerate(algos):
            ys_on = [next((r["on_kernel_ms"] for r in size_rows
                           if r["algo"] == algo and r["n"] == n), None) for n, _ in sizes]
            ys_off = [next((r["off_kernel_ms"] for r in size_rows
                            if r["algo"] == algo and r["n"] == n), None) for n, _ in sizes]
            series_on.append((algo, ys_on, colors[idx % len(colors)]))
            series_off.append((algo, ys_off, colors[idx % len(colors)]))
        (outdir / "size_scaling_on.svg").write_text(
            line_chart("Algorithm time vs graph size (rewrite ON, kernel time)",
                       labels, series_on, "ms (log)", y_log=True))
        if not args.skip_off:
            (outdir / "size_scaling_off.svg").write_text(
                line_chart("Algorithm time vs graph size (rewrite OFF, kernel time)",
                           labels, series_off, "ms (log)", y_log=True))
    if thread_rows:
        labels = [str(t) for t in thread_counts]
        for config in thread_configs:
            series = []
            for idx, algo in enumerate(algos):
                ys = [next((r["speedup_vs_1t"] for r in thread_rows
                            if r["config"] == config and r["algo"] == algo
                            and r["threads"] == t), None) for t in thread_counts]
                series.append((algo, ys, colors[idx % len(colors)]))
            series.append(("ideal", [float(t) for t in thread_counts], "#9aa4b2"))
            (outdir / f"thread_scaling_{config}.svg").write_text(
                line_chart(f"Thread scaling at n={sizes[-1][0]} — {config} "
                           f"(kernel speedup vs 1 thread)",
                           labels, series, "speedup", y_log=False))

    # ---- markdown summary ----------------------------------------------------
    lines = []
    lines.append("# Algorithm scalability — effect algebra + composable runtime")
    lines.append("")
    lines.append(f"- build: {stamp}, seed {args.seed}, "
                 f"rounds {args.rounds} (+1 warmup), threads {thread_counts}")
    lines.append(f"- size sweep: rewrite ON at {args.sweep_threads} threads, "
                 f"rewrite OFF at 1 thread")
    lines.append("- `on_kernel/off_kernel` subtract the load-only baseline at the same "
                 "thread count; `speedup_kernel = off_kernel / on_kernel`")
    lines.append("- thread sweep configs: `default` = as shipped "
                 "(`SGPL_CLEANCUT_PARTITIONS` unset → workers*4, `SGPL_LOOP_CHUNK` unset "
                 "→ 256); `tuned` = `SGPL_LOOP_CHUNK=4`, "
                 "`SGPL_CLEANCUT_PARTITIONS=<threads>`")
    lines.append("")
    lines.append("## Size scaling (kernel ms)")
    lines.append("")
    header = "| algo | size | ON ms | OFF ms | speedup | load ON ms | checksum |"
    lines.append(header)
    lines.append("|---|---|---|---|---|---|---|")
    for r in size_rows:
        sp = f"{r['speedup_kernel']:.2f}x" if r["speedup_kernel"] else "-"
        lines.append(f"| {r['algo']} | {fmt_size(r['n'])} | {fmt_ms(r['on_kernel_ms'])} | "
                     f"{fmt_ms(r['off_kernel_ms'])} | {sp} | {fmt_ms(r['load_on_ms'])} | "
                     f"{'OK' if r['checksum_ok'] else 'MISMATCH'} |")
    for config in thread_configs:
        rows_cfg = [r for r in thread_rows if r["config"] == config]
        if not rows_cfg:
            continue
        lines.append("")
        lines.append(f"## Thread scaling at n={sizes[-1][0]} — {config} "
                     "(kernel speedup vs 1 thread)")
        lines.append("")
        lines.append("| algo | " + " | ".join(f"{t}T" for t in thread_counts) + " |")
        lines.append("|" + "---|" * (len(thread_counts) + 1))
        for algo in algos:
            cells = []
            for t in thread_counts:
                row = next((r for r in rows_cfg
                            if r["algo"] == algo and r["threads"] == t), None)
                cells.append(f"{row['speedup_vs_1t']:.2f}x" if row and row["speedup_vs_1t"] else "-")
            lines.append(f"| {algo} | " + " | ".join(cells) + " |")
    lines.append("")
    (outdir / "summary.md").write_text("\n".join(lines))

    print("\n" + "=" * 78)
    print("  artifacts")
    print(f"    summary : {outdir / 'summary.md'}")
    print(f"    size    : {size_csv}")
    if thread_rows:
        print(f"    threads : {outdir / 'thread_scaling.csv'}")
    print(f"    coverage: {coverage_csv}")
    print("=" * 78, flush=True)


if __name__ == "__main__":
    main()
