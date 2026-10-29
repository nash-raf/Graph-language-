#!/usr/bin/env python3
"""Ground-truth measurement: forced-layout builds with per-region timing.

For each (graph, program), builds four times (CSR/PCSR/BCSR/SET) and records
per-region median times with conversion excluded. Answers must match across
layouts or the row is discarded.
"""
from __future__ import annotations

import argparse
import csv
import json
import traceback
from pathlib import Path

from common import LAYOUTS, RESULTS, ROOT, compile_graph, load_json, run_program

WORK = ROOT / "data" / "workloads"
BUILD = ROOT / "data" / "builds"


def measure_one(case: dict, runs: int, warmup: int, timeout: int) -> list[dict]:
    rows = []
    answers = {}
    graph_path = Path(case["path"])
    tag = f"{case['graph']}__{case['template']}"
    for layout in LAYOUTS:
        wdir = BUILD / tag / layout
        prof = wdir / "profile.jsonl"
        try:
            binary = compile_graph(
                graph_path, wdir, force_layout=layout, timeout=timeout
            )
            result = run_program(
                binary, profile_dump=prof, runs=runs, warmup=warmup, timeout=timeout
            )
        except Exception as ex:
            rows.append(
                {
                    "graph": case["graph"],
                    "template": case["template"],
                    "family": case["family"],
                    "n": case["n"],
                    "m": case["m"],
                    "layout": layout,
                    "error": str(ex),
                }
            )
            continue
        answers[layout] = result["answer"]
        for rid, st in result["regions"].items():
            rows.append(
                {
                    "graph": case["graph"],
                    "template": case["template"],
                    "family": case["family"],
                    "n": case["n"],
                    "m": case["m"],
                    "layout": layout,
                    "region_id": rid,
                    "kind": st["kind"],
                    "measured_ns_median": st["measured_ns_median"],
                    "measured_ns_lo": st["measured_ns_lo"],
                    "measured_ns_hi": st["measured_ns_hi"],
                    "predicted_ns": st["predicted_ns"],
                    "conversion_ns": st["conversion_ns"],
                    "program_median_ms": result["median_ms"],
                    "answer": result["answer"],
                    "error": "",
                }
            )
        if not result["regions"]:
            rows.append(
                {
                    "graph": case["graph"],
                    "template": case["template"],
                    "family": case["family"],
                    "n": case["n"],
                    "m": case["m"],
                    "layout": layout,
                    "region_id": -1,
                    "kind": "NONE",
                    "measured_ns_median": 0,
                    "measured_ns_lo": 0,
                    "measured_ns_hi": 0,
                    "predicted_ns": 0,
                    "conversion_ns": 0,
                    "program_median_ms": result["median_ms"],
                    "answer": result["answer"],
                    "error": "no_regions",
                }
            )

    # Mark answer equality across successful layouts
    ok_answers = {a for L, a in answers.items() if a is not None}
    answer_ok = len(ok_answers) <= 1
    for r in rows:
        r["answer_equal_across_layouts"] = answer_ok
        if not answer_ok and not r.get("error"):
            r["error"] = "answer_mismatch"
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--split", choices=["train", "test", "all"], default="train")
    ap.add_argument("--runs", type=int, default=5)
    ap.add_argument("--warmup", type=int, default=1)
    ap.add_argument("--timeout", type=int, default=180)
    ap.add_argument("--limit", type=int, default=0, help="max cases (0=all)")
    ap.add_argument("--out", type=Path, default=RESULTS / "ground_truth.csv")
    args = ap.parse_args()

    index_path = WORK / args.split / "index.json"
    if args.split == "all":
        # merge
        cases = []
        for sp in ("train", "test"):
            p = WORK / sp / "index.json"
            if p.is_file():
                cases.extend(load_json(p))
    else:
        cases = load_json(index_path)

    if args.limit:
        cases = cases[: args.limit]

    RESULTS.mkdir(parents=True, exist_ok=True)
    all_rows = []
    for i, case in enumerate(cases):
        print(f"[{i+1}/{len(cases)}] {case['graph']}__{case['template']}", flush=True)
        try:
            rows = measure_one(case, args.runs, args.warmup, args.timeout)
            all_rows.extend(rows)
        except Exception:
            traceback.print_exc()

    args.out.parent.mkdir(parents=True, exist_ok=True)
    if all_rows:
        keys = sorted({k for r in all_rows for k in r.keys()})
        with args.out.open("w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=keys)
            w.writeheader()
            w.writerows(all_rows)
    print(f"wrote {args.out} ({len(all_rows)} rows)")


if __name__ == "__main__":
    main()
