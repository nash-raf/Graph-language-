#!/usr/bin/env python3
"""Benchmark cold text construction, cache publication, and warm cache hits."""

import csv
import os
from pathlib import Path
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "benchmark_results"
PROGRAM = Path(os.getenv("SGPL_LOADER_BIN", ROOT / "graph_loader_benchmark"))
GRAPH = Path(os.getenv("SGPL_GRAPH_FILE", "/tmp/graptor-benchmark/synth_2m_8m.txt"))
EXPECTED = os.getenv("EXPECTED_VERTICES", "2000000")
ROUNDS = int(os.getenv("ROUNDS", "3"))
WARM_ROUNDS = int(os.getenv("WARM_ROUNDS", "7"))
MAX_THREADS = int(os.getenv("MAX_THREADS", "10"))
SELECTED_THREADS = int(os.getenv("THREADS", "8"))


def timed_run(threads, cache_mode):
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = env["SGPL_NUM_THREADS"] = str(threads)
    if cache_mode:
        env["SGPL_GRAPH_CACHE"] = cache_mode
    else:
        env.pop("SGPL_GRAPH_CACHE", None)
    start = time.perf_counter()
    process = subprocess.run(
        [str(PROGRAM), str(GRAPH)], env=env, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, check=True
    )
    elapsed = (time.perf_counter() - start) * 1000
    if process.stdout.strip() != EXPECTED:
        raise RuntimeError(f"expected {EXPECTED}, got {process.stdout!r}")
    return elapsed


def samples(threads, mode, rounds):
    return [timed_run(threads, mode) for _ in range(rounds)]


def summary(phase, threads, values):
    return {
        "phase": phase,
        "threads": threads,
        "median_ms": statistics.median(values),
        "min_ms": min(values),
        "max_ms": max(values),
    }


def svg(rows):
    cold = [row for row in rows if row["phase"] == "cold_text"]
    phase_rows = [
        next(row for row in rows if row["phase"] == phase)
        for phase in ("cold_text", "cache_build", "warm_cache")
        if any(row["phase"] == phase and row["threads"] == SELECTED_THREADS
               for row in rows)
    ]
    phase_rows[0] = next(
        row for row in rows
        if row["phase"] == "cold_text" and row["threads"] == SELECTED_THREADS
    )
    width, height = 1000, 760
    chart_left, chart_width = 85, 830

    cold_max = max(row["median_ms"] for row in cold) * 1.08
    points = []
    for index, row in enumerate(cold):
        x = chart_left + chart_width * index / max(1, len(cold) - 1)
        y = 335 - 245 * row["median_ms"] / cold_max
        points.append((x, y, row))

    bar_max = max(row["median_ms"] for row in phase_rows) * 1.12
    bars = []
    for index, row in enumerate(phase_rows):
        x = 180 + index * 250
        bar_height = 230 * row["median_ms"] / bar_max
        bars.append((x, 690 - bar_height, bar_height, row))

    output = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<style>text{font:12px system-ui;fill:#26303b}.title{font-size:18px;font-weight:600}.label{font-size:14px;font-weight:600}.grid{stroke:#d8dde5;stroke-width:1}</style>',
        '<text x="35" y="30" class="title">SGPL binary graph cache and parallel CSR construction</text>',
        '<text x="35" y="53">2M vertices · 8M undirected input edges · complete process wall time</text>',
        '<text x="35" y="82" class="label">Cold text parse + CSR construction scaling</text>',
        '<text x="16" y="220" transform="rotate(-90 16 220)">Median wall time (ms)</text>',
    ]
    for tick in range(5):
        value = cold_max * tick / 4
        y = 335 - 245 * tick / 4
        output += [
            f'<line x1="{chart_left}" y1="{y}" x2="{chart_left + chart_width}" y2="{y}" class="grid"/>',
            f'<text x="{chart_left - 8}" y="{y + 4}" text-anchor="end">{value:.0f}</text>',
        ]
    output.append('<polyline fill="none" stroke="#3478d4" stroke-width="3" points="' +
                  " ".join(f"{x},{y}" for x, y, _ in points) + '"/>')
    for x, y, row in points:
        output += [
            f'<circle cx="{x}" cy="{y}" r="4" fill="#3478d4"/>',
            f'<text x="{x}" y="357" text-anchor="middle">{row["threads"]}</text>',
        ]
    output += [
        '<text x="500" y="382" text-anchor="middle">Configured threads</text>',
        f'<text x="35" y="420" class="label">{SELECTED_THREADS}-thread loader phases</text>',
        '<text x="16" y="580" transform="rotate(-90 16 580)">Median wall time (ms)</text>',
    ]
    labels = {
        "cold_text": "Cold text",
        "cache_build": "Build + publish cache",
        "warm_cache": "Warm binary cache",
    }
    colors = {"cold_text": "#3478d4", "cache_build": "#8b6f47",
              "warm_cache": "#238b57"}
    for x, y, bar_height, row in bars:
        output += [
            f'<rect x="{x}" y="{y}" width="130" height="{bar_height}" fill="{colors[row["phase"]]}"/>',
            f'<text x="{x + 65}" y="{y - 8}" text-anchor="middle">{row["median_ms"]:.1f} ms</text>',
            f'<text x="{x + 65}" y="715" text-anchor="middle">{labels[row["phase"]]}</text>',
        ]
    output.append('<text x="35" y="744">Source: local median timings; cache payload includes CSR, logical edge metadata, and serialized node/edge bitmaps.</text>')
    output.append("</svg>")
    return "".join(output)


def main():
    if not PROGRAM.is_file() or not GRAPH.is_file():
        raise SystemExit(f"missing loader binary or graph: {PROGRAM}, {GRAPH}")
    rows = []
    for threads in range(1, MAX_THREADS + 1):
        values = samples(threads, "off", ROUNDS)
        rows.append(summary("cold_text", threads, values))
        print(f"cold {threads}: {statistics.median(values):.3f} ms")

    build_values = samples(SELECTED_THREADS, "refresh", ROUNDS)
    rows.append(summary("cache_build", SELECTED_THREADS, build_values))
    timed_run(SELECTED_THREADS, "refresh")
    warm_values = samples(SELECTED_THREADS, None, WARM_ROUNDS)
    rows.append(summary("warm_cache", SELECTED_THREADS, warm_values))

    OUT.mkdir(exist_ok=True)
    csv_path = OUT / "graph_loader_2m8m.csv"
    svg_path = OUT / "graph_loader_2m8m.svg"
    with csv_path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    svg_path.write_text(svg(rows))
    print(f"cache build: {statistics.median(build_values):.3f} ms")
    print(f"warm cache: {statistics.median(warm_values):.3f} ms")
    print(f"CSV: {csv_path}")
    print(f"Plot: {svg_path}")


if __name__ == "__main__":
    main()
