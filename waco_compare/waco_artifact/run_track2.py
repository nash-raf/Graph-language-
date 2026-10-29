#!/usr/bin/env python3
"""Track 2: map WACO artifact format choices onto AutoTuner CSR/BCSR.

Without a full TACO+icc+Minkowski retrain on this laptop, this harness:

1. Uses WACO's SuperSchedule format encoding conventions (UC ≈ CSR,
   UCU/UCUU dense-block ≈ BCSR) when an artifact top-k file is present.
2. Otherwise runs a *format-family heuristic oracle adapter* that scores
   CSR vs BCSR using the same measured GT times (reported separately as
   baseline), and a WACO-like occupancy CNN that predicts CSR vs BCSR only
   on read-only Traverse/PageRank-style regions.

Outputs:
  results/waco_artifact_pred.csv
"""
from __future__ import annotations

import argparse
import csv
import json
import sys
from collections import defaultdict
from pathlib import Path

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "wacog"))
from model import (  # noqa: E402
    graph_stats,
    load_edges_txt,
    rasterize_occupancy,
)

ARTIFACT = ROOT / "third_party" / "Workload-Aware-Co-Optimization"


def map_superschedule_format(fmt: str) -> str | None:
    """Map TACO level-format strings to AutoTuner layouts."""
    f = fmt.upper().replace(" ", "")
    if f in ("UC", "U", "CSR"):
        return "CSR"
    if "UCUU" in f or f in ("UCU", "BCSR"):
        return "BCSR"
    if f.startswith("UC") and "U" in f[2:]:
        return "BCSR"
    return None  # other / unmapped


class CSRBCSRNet(nn.Module):
    def __init__(self):
        super().__init__()
        self.cnn = nn.Sequential(
            nn.Conv2d(1, 16, 5, stride=2, padding=2),
            nn.ReLU(),
            nn.Conv2d(16, 32, 3, stride=2, padding=1),
            nn.ReLU(),
            nn.AdaptiveAvgPool2d(1),
        )
        self.mlp = nn.Sequential(
            nn.Linear(32 + 16, 64),
            nn.ReLU(),
            nn.Linear(64, 2),  # CSR, BCSR scores (lower=better cost)
        )

    def forward(self, grid, stats):
        x = self.cnn(grid).flatten(1)
        return self.mlp(torch.cat([x, stats], dim=1))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gt", type=Path, default=ROOT / "results" / "ground_truth.csv")
    ap.add_argument("--epochs", type=int, default=20)
    ap.add_argument("--out", type=Path, default=ROOT / "results" / "waco_artifact_pred.csv")
    args = ap.parse_args()

    corpus = json.loads((ROOT / "data" / "corpus.json").read_text())
    edge_index = {g["name"]: g["edge_list"] for g in corpus["graphs"]}
    train_names = {g["name"] for g in json.loads((ROOT / "data" / "splits" / "train.json").read_text())}
    test_names = {g["name"] for g in json.loads((ROOT / "data" / "splits" / "test.json").read_text())}

    # Only overlap: traverse_pagerank / traverse_cc (read-only gathers ≈ SpMV)
    rows = list(csv.DictReader(args.gt.open()))
    usable = [
        r
        for r in rows
        if not r.get("error")
        and r.get("kind") == "Traverse"
        and r.get("template", "").startswith("traverse_")
        and r.get("layout") in ("CSR", "BCSR")
    ]

    # Build labels: best of CSR/BCSR per region
    groups = defaultdict(dict)
    for r in usable:
        key = (r["graph"], r["template"], int(float(r["region_id"])))
        groups[key][r["layout"]] = float(r["measured_ns_median"])

    def best_layout(d):
        if "CSR" in d and "BCSR" in d:
            return "CSR" if d["CSR"] <= d["BCSR"] else "BCSR"
        return None

    # Train binary model on train graphs
    train_keys = [k for k in groups if k[0] in train_names and best_layout(groups[k])]
    test_keys = [k for k in groups if k[0] in test_names and best_layout(groups[k])]

    cache = {}
    def feats(name):
        if name not in cache:
            n, e = load_edges_txt(edge_index[name])
            cache[name] = (rasterize_occupancy(n, e), graph_stats(n, e))
        return cache[name]

    model = CSRBCSRNet()
    opt = torch.optim.Adam(model.parameters(), lr=1e-3)
    model.train()
    for epoch in range(args.epochs):
        total = 0.0
        n = 0
        for key in train_keys:
            grid, stats = feats(key[0])
            y = 0 if best_layout(groups[key]) == "CSR" else 1
            # scores: lower better; train so correct class has lower score
            pred = model(
                torch.from_numpy(grid).unsqueeze(0).unsqueeze(0),
                torch.from_numpy(stats).unsqueeze(0),
            )[0]
            # hinge: score_correct + 1 < score_wrong
            sc = pred[y]
            sw = pred[1 - y]
            loss = F.relu(1.0 - (sw - sc))
            opt.zero_grad()
            loss.backward()
            opt.step()
            total += float(loss.item())
            n += 1
        print(f"track2 epoch {epoch+1}/{args.epochs} loss={total/max(n,1):.4f}", flush=True)

    # Also ingest artifact topk if present
    topk_dir = ARTIFACT / "WACO" / "SpMM" / "topk"
    artifact_map = {}
    if topk_dir.is_dir():
        for p in topk_dir.glob("*.txt"):
            # Heuristic parse: look for format tokens
            text = p.read_text()
            mapped = None
            for tok in ("UCUU", "UCU", "UC"):
                if tok in text:
                    mapped = map_superschedule_format(tok)
                    break
            artifact_map[p.stem] = mapped

    model.eval()
    out_rows = []
    with torch.no_grad():
        for key in test_keys:
            grid, stats = feats(key[0])
            pred = model(
                torch.from_numpy(grid).unsqueeze(0).unsqueeze(0),
                torch.from_numpy(stats).unsqueeze(0),
            )[0]
            choice = "CSR" if float(pred[0]) <= float(pred[1]) else "BCSR"
            # unrestricted vs filtered: filtered is already CSR/BCSR-only
            art = artifact_map.get(key[0])
            out_rows.append(
                {
                    "graph": key[0],
                    "template": key[1],
                    "region_id": key[2],
                    "oracle_csr_bcsr": best_layout(groups[key]),
                    "waco_overlap_pred": choice,
                    "waco_artifact_mapped": art or "",
                    "score_CSR": float(pred[0]),
                    "score_BCSR": float(pred[1]),
                    "gt_CSR_ns": groups[key].get("CSR", ""),
                    "gt_BCSR_ns": groups[key].get("BCSR", ""),
                }
            )

    args.out.parent.mkdir(parents=True, exist_ok=True)
    if out_rows:
        keys = sorted({k for r in out_rows for k in r.keys()})
        with args.out.open("w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=keys)
            w.writeheader()
            w.writerows(out_rows)
    print(f"wrote {args.out} ({len(out_rows)} rows)")


if __name__ == "__main__":
    main()
