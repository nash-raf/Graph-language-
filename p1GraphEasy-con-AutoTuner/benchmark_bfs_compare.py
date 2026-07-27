#!/usr/bin/env python3
"""Run paired SGPL/Graptor BFS processes and write CSV + SVG plots."""

import argparse
import csv
import os
from pathlib import Path
import random
import re
import resource
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "benchmark_results"
EXPECTED = os.getenv("EXPECTED_REACHED", "796547")
SGPL = Path(os.getenv("SGPL_BIN", ROOT / "final_program_graptor_compare"))
GROOT = Path(os.getenv("GRAPTOR_ROOT", "/tmp/graptor-benchmark"))
GRAPTOR = Path(os.getenv(
    "GRAPTOR_BIN", GROOT / "build/bench/BFSv_VEBOGraptorPullDataParCached"
))
GGRAPH = Path(os.getenv("GRAPTOR_GRAPH", GROOT / "synth_1m_undir.graptor"))
GLIB = GROOT / "build/external/Build/libconfig_project/lib64"


def require(path, label):
    if not path.is_file():
        raise SystemExit(f"{label} not found: {path}")


def sgpl_run(threads):
    env = os.environ.copy()
    env["SGPL_NUM_THREADS"] = env["OMP_NUM_THREADS"] = str(threads)
    start = time.perf_counter()
    p = subprocess.run([str(SGPL)], env=env, text=True, stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE, check=True)
    elapsed = (time.perf_counter() - start) * 1000
    if p.stdout.strip() != EXPECTED:
        raise RuntimeError(f"SGPL expected {EXPECTED}, got {p.stdout!r}")
    return elapsed


def graptor_run(threads):
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(threads)
    env["LD_LIBRARY_PATH"] = f"{GLIB}:{env.get('LD_LIBRARY_PATH', '')}"
    cmd = [str(GRAPTOR), "-rounds", "1", "-c",
           os.getenv("GRAPTOR_PARTITIONS", "16"), "-l",
           os.getenv("GRAPTOR_SIMD_WIDTH", "8"), "-s", "-b", str(GGRAPH)]
    start = time.perf_counter()
    p = subprocess.run(cmd, env=env, text=True, stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE, check=True)
    elapsed = (time.perf_counter() - start) * 1000
    text = p.stdout + p.stderr
    reached = re.search(r"Number of activated vertices:\s*(\d+)", text)
    if not reached or reached.group(1) != EXPECTED or \
            "Every vertex activated at most once: PASS" not in text:
        raise RuntimeError("Graptor correctness check failed")
    return elapsed


def paired(threads, rounds):
    sgpl_run(threads)
    graptor_run(threads)
    s, g = [], []
    for i in range(rounds):
        if i % 2:
            s.append(sgpl_run(threads))
            g.append(graptor_run(threads))
        else:
            g.append(graptor_run(threads))
            s.append(sgpl_run(threads))
    return s, g


def med(values):
    return statistics.median(values)


def line_chart(title, xs, series, y_label):
    width, height = 920, 390
    left, right, top, bottom = 72, 24, 45, 58
    pw, ph = width-left-right, height-top-bottom
    values = [v for _, ys, _ in series for v in ys]
    lo, hi = min(values), max(values)
    pad = max((hi-lo)*.1, hi*.03, 1)
    lo, hi = max(0, lo-pad), hi+pad
    xp = lambda i: left + pw*i/max(1, len(xs)-1)
    yp = lambda v: top + ph*(hi-v)/max(1e-9, hi-lo)
    out = [f'<svg width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
           f'<text x="{left}" y="25" class="title">{title}</text>']
    for i in range(5):
        v = lo+(hi-lo)*i/4
        y = yp(v)
        out += [f'<line x1="{left}" y1="{y}" x2="{width-right}" y2="{y}" class="grid"/>',
                f'<text x="{left-8}" y="{y+4}" text-anchor="end">{v:.2f}</text>']
    for i, x in enumerate(xs):
        out.append(f'<text x="{xp(i)}" y="{height-30}" text-anchor="middle">{x}</text>')
    out.append(f'<text x="15" y="{top+ph/2}" transform="rotate(-90 15 {top+ph/2})" text-anchor="middle">{y_label}</text>')
    legend_x = left
    for name, ys, color in series:
        points = " ".join(f"{xp(i)},{yp(v)}" for i, v in enumerate(ys))
        out.append(f'<polyline points="{points}" fill="none" stroke="{color}" stroke-width="3"/>')
        out += [f'<circle cx="{xp(i)}" cy="{yp(v)}" r="4" fill="{color}"/>'
                for i, v in enumerate(ys)]
        out.append(f'<text x="{legend_x}" y="{height-8}" fill="{color}">{name}</text>')
        legend_x += 260
    out.append("</svg>")
    return "".join(out)


def bar_chart(threads, sm, gm):
    maximum = max(sm, gm)*1.15
    bars = [("SGPL", sm, "#3478d4", 180), ("Graptor", gm, "#238b57", 440)]
    out = [f'<svg width="760" height="360" viewBox="0 0 760 360"><text x="40" y="25" class="title">End-to-end BFS at {threads} threads</text>']
    for name, value, color, x in bars:
        h = 240*value/maximum
        y = 290-h
        out.append(f'<rect x="{x}" y="{y}" width="140" height="{h}" fill="{color}"/>')
        out.append(f'<text x="{x+70}" y="{y-10}" text-anchor="middle" class="value">{value:.1f} ms</text>')
        out.append(f'<text x="{x+70}" y="325" text-anchor="middle">{name}</text>')
    return "".join(out)+"</svg>"


def write_files(mode, rows, charts, note):
    OUT.mkdir(exist_ok=True)
    csv_path, svg_path = OUT/f"bfs_{mode}.csv", OUT/f"bfs_{mode}.svg"
    with csv_path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    svg = f'''<svg xmlns="http://www.w3.org/2000/svg" width="1100" height="{430*len(charts)+110}" viewBox="0 0 1100 {430*len(charts)+110}">
<style>text{{font:12px system-ui;fill:#26303b}}.title{{font-size:17px;font-weight:600}}.value{{font-size:15px;font-weight:600}}.grid{{stroke:#d8dde5;stroke-width:1}}</style>
<text x="25" y="28" class="title">SGPL vs Graptor BFS · complete process time</text>
<text x="25" y="52">{note}</text>
<g transform="translate(70,70)">{"".join(f'<g transform="translate(0,{i*430})">{chart}</g>' for i,chart in enumerate(charts))}</g>
</svg>'''
    svg_path.write_text(svg)
    print(f"CSV:  {csv_path}")
    print(f"Plot: {svg_path}")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("mode", choices=("e2e", "scalability"))
    p.add_argument("--rounds", type=int, default=int(os.getenv("ROUNDS", "7")))
    p.add_argument("--threads", type=int, default=int(os.getenv("THREADS", "8")))
    p.add_argument("--max-threads", type=int, default=int(os.getenv("MAX_THREADS", "10")))
    args = p.parse_args()
    for path, label in ((SGPL, "SGPL binary"), (GRAPTOR, "Graptor binary"),
                        (GGRAPH, "Graptor graph")):
        require(path, label)
    resource.setrlimit(resource.RLIMIT_STACK, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))
    graph_label = os.getenv(
        "BENCHMARK_LABEL", "1M vertices, 1M undirected input edges"
    )
    note = f"{graph_label}, source 0; warm-up + median of {args.rounds} alternating launches"
    if args.mode == "e2e":
        s, g = paired(args.threads, args.rounds)
        sm, gm = med(s), med(g)
        rows = [{"threads": args.threads, "sgpl_median_ms": sm,
                 "sgpl_min_ms": min(s), "sgpl_max_ms": max(s),
                 "graptor_median_ms": gm, "graptor_min_ms": min(g),
                 "graptor_max_ms": max(g)}]
        write_files("e2e", rows, [bar_chart(args.threads, sm, gm)], note)
        print(f"SGPL {sm:.3f} ms; Graptor {gm:.3f} ms; SGPL/Graptor {sm/gm:.3f}x")
    else:
        thread_counts = list(range(1, args.max_threads + 1))
        samples = {t: {"sgpl": [], "graptor": []} for t in thread_counts}
        # Warm every configuration once, then shuffle thread counts each round.
        # This prevents sustained-load or background-system drift from being
        # mistaken for a trend caused by increasing thread count.
        for t in thread_counts:
            sgpl_run(t)
            graptor_run(t)
        rng = random.Random(0x5347504C)
        for round_index in range(args.rounds):
            order = thread_counts.copy()
            rng.shuffle(order)
            for t in order:
                if (round_index + t) % 2:
                    samples[t]["sgpl"].append(sgpl_run(t))
                    samples[t]["graptor"].append(graptor_run(t))
                else:
                    samples[t]["graptor"].append(graptor_run(t))
                    samples[t]["sgpl"].append(sgpl_run(t))
        rows = []
        for t in thread_counts:
            sm, gm = med(samples[t]["sgpl"]), med(samples[t]["graptor"])
            rows.append({"threads": t, "sgpl_median_ms": sm,
                         "graptor_median_ms": gm})
            print(f"{t}: SGPL {sm:.3f} ms; Graptor {gm:.3f} ms")
        xs = [r["threads"] for r in rows]
        sv = [r["sgpl_median_ms"] for r in rows]
        gv = [r["graptor_median_ms"] for r in rows]
        ss, gs = [sv[0]/x for x in sv], [gv[0]/x for x in gv]
        charts = [
            line_chart("End-to-end BFS wall time", xs,
                       [("SGPL", sv, "#3478d4"), ("Graptor", gv, "#238b57")],
                       "Median wall time (ms)"),
            line_chart("End-to-end thread scalability", xs,
                       [("SGPL speedup", ss, "#3478d4"), ("Graptor speedup", gs, "#238b57")],
                       "Speedup relative to 1 thread"),
        ]
        write_files("scalability", rows, charts, note)


if __name__ == "__main__":
    main()
