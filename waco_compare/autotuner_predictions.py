#!/usr/bin/env python3
"""Dump AutoTuner per-region layout predictions (costs + local/DP choice)."""
from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

from common import RESULTS, ROOT, compile_graph, load_json

WORK = ROOT / "data" / "workloads"
BUILD = ROOT / "data" / "builds_pred"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--split", choices=["train", "test", "all"], default="test")
    ap.add_argument("--timeout", type=int, default=180)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--out", type=Path, default=RESULTS / "autotuner_pred.csv")
    args = ap.parse_args()

    if args.split == "all":
        cases = []
        for sp in ("train", "test"):
            p = WORK / sp / "index.json"
            if p.is_file():
                cases.extend(load_json(p))
    else:
        cases = load_json(WORK / args.split / "index.json")
    if args.limit:
        cases = cases[: args.limit]

    rows = []
    for i, case in enumerate(cases):
        tag = f"{case['graph']}__{case['template']}"
        print(f"[{i+1}/{len(cases)}] predict {tag}", flush=True)
        wdir = BUILD / tag
        dump = wdir / "pred.jsonl"
        try:
            compile_graph(
                Path(case["path"]),
                wdir,
                prediction_dump=dump,
                timeout=args.timeout,
            )
        except Exception as ex:
            rows.append(
                {
                    "graph": case["graph"],
                    "template": case["template"],
                    "family": case["family"],
                    "error": str(ex),
                }
            )
            continue
        if not dump.is_file():
            rows.append(
                {
                    "graph": case["graph"],
                    "template": case["template"],
                    "family": case["family"],
                    "error": "empty_prediction_dump",
                }
            )
            continue
        for line in dump.read_text().splitlines():
            if not line.strip():
                continue
            obj = json.loads(line)
            obj.update(
                {
                    "graph": case["graph"],
                    "template": case["template"],
                    "family": case["family"],
                    "n_graph": case["n"],
                    "m_graph": case["m"],
                    "error": "",
                }
            )
            rows.append(obj)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    if rows:
        keys = sorted({k for r in rows for k in r.keys()})
        with args.out.open("w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=keys)
            w.writeheader()
            w.writerows(rows)
    print(f"wrote {args.out} ({len(rows)} rows)")


if __name__ == "__main__":
    main()
