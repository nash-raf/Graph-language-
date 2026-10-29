#!/usr/bin/env python3
"""Score all 4 layouts per test region with WACO-G; optionally emit DP plans."""
from __future__ import annotations

import argparse
import csv
import json
import sys
from collections import defaultdict
from pathlib import Path

import numpy as np
import torch

sys.path.insert(0, str(Path(__file__).resolve().parent))
from model import (
    KIND_TO_ID,
    LAYOUT_TO_ID,
    LAYOUTS,
    WACOGCostModel,
    graph_stats,
    load_edges_txt,
    rasterize_occupancy,
    region_cont_features,
)

ROOT = Path(__file__).resolve().parents[1]


def conversion_cost_proxy(n: float, m: float) -> float:
    # Rough stand-in matching AutoTuner order of magnitude when feeding DP:
    # proportional to reading/writing adjacency.
    return 50.0 * (n + m)


def dp_plan(region_scores: list[dict], n: float, m: float) -> list[str]:
    """Minimize sum(score[i][L]) + conversion between consecutive regions."""
    R = len(region_scores)
    if R == 0:
        return []
    L = len(LAYOUTS)
    conv = conversion_cost_proxy(n, m)
    # scores[i][l]
    scores = np.zeros((R, L), dtype=np.float64)
    for i, row in enumerate(region_scores):
        for l, name in enumerate(LAYOUTS):
            scores[i, l] = float(row.get(f"score_{name}", 1e100))
    dp = np.full((R, L), 1e100)
    parent = np.full((R, L), -1, dtype=np.int32)
    dp[0] = scores[0]
    for i in range(1, R):
        for l in range(L):
            for prev in range(L):
                c = dp[i - 1, prev] + (0.0 if prev == l else conv) + scores[i, l]
                if c < dp[i, l]:
                    dp[i, l] = c
                    parent[i, l] = prev
    # backtrack
    cur = int(np.argmin(dp[R - 1]))
    plan = [cur]
    for i in range(R - 1, 0, -1):
        cur = int(parent[i, cur])
        plan.append(cur)
    plan.reverse()
    return [LAYOUTS[i] for i in plan]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ckpt", type=Path, default=ROOT / "results" / "wacog.pt")
    ap.add_argument("--gt", type=Path, default=ROOT / "results" / "ground_truth.csv")
    ap.add_argument("--out", type=Path, default=ROOT / "results" / "wacog_pred.csv")
    ap.add_argument("--plans", type=Path, default=ROOT / "results" / "wacog_plans.json")
    ap.add_argument("--device", default="cpu")
    args = ap.parse_args()

    corpus = json.loads((ROOT / "data" / "corpus.json").read_text())
    edge_index = {g["name"]: g["edge_list"] for g in corpus["graphs"]}
    test_names = {g["name"] for g in json.loads((ROOT / "data" / "splits" / "test.json").read_text())}

    device = torch.device(args.device)
    model = WACOGCostModel().to(device)
    ckpt = torch.load(args.ckpt, map_location=device)
    model.load_state_dict(ckpt["model"])
    model.eval()

    # Collect decision regions from GT (to know which regions exist)
    rows = list(csv.DictReader(args.gt.open()))
    regions = defaultdict(list)
    for r in rows:
        if r.get("error") or r["graph"] not in test_names:
            continue
        if r.get("kind") not in ("Traverse", "Insert"):
            continue
        regions[(r["graph"], r["template"], int(float(r["region_id"])))].append(r)

    graph_cache = {}
    out_rows = []
    plans = {}

    # Group by (graph, template)
    by_prog = defaultdict(list)
    for key in regions:
        by_prog[(key[0], key[1])].append(key[2])

    with torch.no_grad():
        for (graph, template), rids in by_prog.items():
            if graph not in edge_index:
                continue
            if graph not in graph_cache:
                n, edges = load_edges_txt(edge_index[graph])
                graph_cache[graph] = (
                    rasterize_occupancy(n, edges),
                    graph_stats(n, edges),
                    n,
                    len(edges),
                )
            grid_np, stats_np, n, m = graph_cache[graph]
            grid = torch.from_numpy(grid_np).unsqueeze(0).unsqueeze(0).to(device)
            stats = torch.from_numpy(stats_np).unsqueeze(0).to(device)
            # Use one exemplar row per region for descriptor features
            region_scores = []
            for rid in sorted(set(rids)):
                exemplars = regions[(graph, template, rid)]
                ex = exemplars[0]
                kind = ex.get("kind", "Traverse")
                cont = torch.from_numpy(region_cont_features(ex)).unsqueeze(0).to(device)
                kind_id = torch.tensor([KIND_TO_ID.get(kind, 0)], device=device)
                scores = {}
                for name, lid in LAYOUT_TO_ID.items():
                    lid_t = torch.tensor([lid], device=device)
                    s = float(model(grid, stats, lid_t, kind_id, cont).item())
                    scores[f"score_{name}"] = s
                local = min(LAYOUTS, key=lambda L: scores[f"score_{L}"])
                row = {
                    "graph": graph,
                    "template": template,
                    "region_id": rid,
                    "kind": kind,
                    "local_argmin": local,
                    **scores,
                }
                out_rows.append(row)
                region_scores.append(row)
            plan = dp_plan(region_scores, n, m)
            plan_str = ",".join(f"r{i}:{L}" for i, L in enumerate(plan))
            plans[f"{graph}__{template}"] = {"plan": plan, "env": plan_str}

    args.out.parent.mkdir(parents=True, exist_ok=True)
    if out_rows:
        keys = sorted({k for r in out_rows for k in r.keys()})
        with args.out.open("w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=keys)
            w.writeheader()
            w.writerows(out_rows)
    args.plans.write_text(json.dumps(plans, indent=2))
    print(f"wrote {args.out} ({len(out_rows)} rows)")
    print(f"wrote {args.plans}")


if __name__ == "__main__":
    main()
