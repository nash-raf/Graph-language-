#!/usr/bin/env python3
"""Train WACO-G on ground_truth.csv (pairwise hinge ranking)."""
from __future__ import annotations

import argparse
import csv
import json
import sys
from collections import defaultdict
from pathlib import Path

import numpy as np
import torch
from torch.utils.data import Dataset

sys.path.insert(0, str(Path(__file__).resolve().parent))
from model import (
    KIND_TO_ID,
    LAYOUT_TO_ID,
    LAYOUTS,
    WACOGCostModel,
    graph_stats,
    load_edges_txt,
    pairwise_hinge_loss,
    rasterize_occupancy,
    region_cont_features,
)

ROOT = Path(__file__).resolve().parents[1]


class PairDataset(Dataset):
    def __init__(self, groups, graph_cache):
        self.samples = []
        for key, rows in groups.items():
            # rows: list of {layout, measured_ns_median, ...}
            by_l = {r["layout"]: r for r in rows if r["layout"] in LAYOUT_TO_ID}
            layouts = list(by_l.keys())
            for i in range(len(layouts)):
                for j in range(i + 1, len(layouts)):
                    a, b = layouts[i], layouts[j]
                    self.samples.append((key, by_l[a], by_l[b]))
        self.graph_cache = graph_cache

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        key, ra, rb = self.samples[idx]
        grid, stats = self.graph_cache[key[0]]
        return {
            "grid": grid,
            "stats": stats,
            "la": LAYOUT_TO_ID[ra["layout"]],
            "lb": LAYOUT_TO_ID[rb["layout"]],
            "ka": KIND_TO_ID.get(ra.get("kind", "Traverse"), 0),
            "kb": KIND_TO_ID.get(rb.get("kind", "Traverse"), 0),
            "ca": region_cont_features(ra),
            "cb": region_cont_features(rb),
            "ya": float(ra["measured_ns_median"]),
            "yb": float(rb["measured_ns_median"]),
        }


def collate(batch):
    return {
        "grid": torch.stack([torch.from_numpy(b["grid"]) for b in batch]).unsqueeze(1),
        "stats": torch.stack([torch.from_numpy(b["stats"]) for b in batch]),
        "la": torch.tensor([b["la"] for b in batch], dtype=torch.long),
        "lb": torch.tensor([b["lb"] for b in batch], dtype=torch.long),
        "ka": torch.tensor([b["ka"] for b in batch], dtype=torch.long),
        "kb": torch.tensor([b["kb"] for b in batch], dtype=torch.long),
        "ca": torch.stack([torch.from_numpy(b["ca"]) for b in batch]),
        "cb": torch.stack([torch.from_numpy(b["cb"]) for b in batch]),
        "ya": torch.tensor([b["ya"] for b in batch], dtype=torch.float32),
        "yb": torch.tensor([b["yb"] for b in batch], dtype=torch.float32),
    }


def load_gt(path: Path, edge_index: dict, split_names: set[str] | None):
    rows = list(csv.DictReader(path.open()))
    groups = defaultdict(list)
    for r in rows:
        if r.get("error"):
            continue
        if r.get("answer_equal_across_layouts") in ("False", "false", "0"):
            continue
        kind = r.get("kind", "")
        if kind not in ("Traverse", "Insert"):
            continue
        if split_names is not None and r["graph"] not in split_names:
            continue
        try:
            r["measured_ns_median"] = float(r["measured_ns_median"])
            r["region_id"] = int(float(r["region_id"]))
        except Exception:
            continue
        key = (r["graph"], r["template"], r["region_id"])
        groups[key].append(r)
    # Keep only groups with >=2 layouts
    groups = {k: v for k, v in groups.items() if len({x["layout"] for x in v}) >= 2}
    return groups


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gt", type=Path, default=ROOT / "results" / "ground_truth.csv")
    ap.add_argument("--epochs", type=int, default=30)
    ap.add_argument("--batch", type=int, default=32)
    ap.add_argument("--lr", type=float, default=1e-4)
    ap.add_argument("--out", type=Path, default=ROOT / "results" / "wacog.pt")
    ap.add_argument("--device", default="cpu")
    args = ap.parse_args()

    corpus = json.loads((ROOT / "data" / "corpus.json").read_text())
    edge_index = {g["name"]: g["edge_list"] for g in corpus["graphs"]}
    train_split = {g["name"] for g in json.loads((ROOT / "data" / "splits" / "train.json").read_text())}

    groups = load_gt(args.gt, edge_index, train_split)
    print(f"train groups: {len(groups)}")

    # Cache graph features
    graph_cache = {}
    needed = {k[0] for k in groups}
    for name in needed:
        n, edges = load_edges_txt(edge_index[name])
        grid = rasterize_occupancy(n, edges, 128)
        stats = graph_stats(n, edges)
        graph_cache[name] = (grid, stats)

    ds = PairDataset(groups, graph_cache)
    if len(ds) == 0:
        raise SystemExit("no training pairs — run measure_regions.py first")
    loader = torch.utils.data.DataLoader(
        ds, batch_size=args.batch, shuffle=True, collate_fn=collate
    )

    device = torch.device(args.device)
    model = WACOGCostModel().to(device)
    opt = torch.optim.Adam(model.parameters(), lr=args.lr)

    model.train()
    for epoch in range(args.epochs):
        total = 0.0
        n = 0
        for batch in loader:
            opt.zero_grad()
            grid = batch["grid"].to(device)
            stats = batch["stats"].to(device)
            sj = model(
                grid,
                stats,
                batch["la"].to(device),
                batch["ka"].to(device),
                batch["ca"].to(device),
            )
            sk = model(
                grid,
                stats,
                batch["lb"].to(device),
                batch["kb"].to(device),
                batch["cb"].to(device),
            )
            loss = pairwise_hinge_loss(sj, sk, batch["ya"].to(device), batch["yb"].to(device))
            loss.backward()
            opt.step()
            total += float(loss.item())
            n += 1
        print(f"epoch {epoch+1}/{args.epochs} loss={total/max(n,1):.4f}", flush=True)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    torch.save({"model": model.state_dict()}, args.out)
    print(f"wrote {args.out}")


if __name__ == "__main__":
    main()
