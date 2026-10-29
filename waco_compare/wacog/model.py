#!/usr/bin/env python3
"""WACO-G: WACO-method cost model adapted to AutoTuner region/layout space.

Feature extractor approximates WACONet without MinkowskiEngine:
  - 128x128 occupancy raster (local pattern)
  - multi-scale degree / block-density histogram (global pattern)
Region embedder encodes layout + region descriptor (type, freqs, ops).
"""
from __future__ import annotations

import math
from typing import Optional

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F

LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]
LAYOUT_TO_ID = {n: i for i, n in enumerate(LAYOUTS)}
KIND_TO_ID = {"Traverse": 0, "Insert": 1, "Query": 2, "SetQuery": 2, "CSRQuery": 3}


def load_edges_txt(path: str) -> tuple[int, np.ndarray]:
    edges = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            a, b = line.split()[:2]
            edges.append((int(a), int(b)))
    if not edges:
        return 0, np.zeros((0, 2), dtype=np.int32)
    arr = np.asarray(edges, dtype=np.int32)
    n = int(arr.max()) + 1
    return n, arr


def rasterize_occupancy(n: int, edges: np.ndarray, size: int = 128) -> np.ndarray:
    grid = np.zeros((size, size), dtype=np.float32)
    if n <= 0 or len(edges) == 0:
        return grid
    scale = (size - 1) / max(n - 1, 1)
    ur = np.clip((edges[:, 0] * scale).astype(np.int32), 0, size - 1)
    vr = np.clip((edges[:, 1] * scale).astype(np.int32), 0, size - 1)
    np.add.at(grid, (ur, vr), 1.0)
    # log1p normalize
    grid = np.log1p(grid)
    mx = grid.max()
    if mx > 0:
        grid /= mx
    return grid


def graph_stats(n: int, edges: np.ndarray) -> np.ndarray:
    m = len(edges)
    if n <= 0:
        return np.zeros(16, dtype=np.float32)
    deg = np.bincount(edges[:, 0], minlength=n).astype(np.float64)
    dens = m / max(n * n, 1)
    feats = [
        math.log1p(n),
        math.log1p(m),
        dens,
        deg.mean() / max(n, 1),
        deg.std() / max(n, 1),
        (deg.max() if len(deg) else 0) / max(n, 1),
        np.percentile(deg, 50) / max(n, 1) if len(deg) else 0,
        np.percentile(deg, 90) / max(n, 1) if len(deg) else 0,
        np.percentile(deg, 99) / max(n, 1) if len(deg) else 0,
        float((deg == 0).mean()),
        float((deg == 1).mean()),
        float((deg >= 10).mean()),
        float((deg >= 100).mean()),
        math.log1p(deg.sum()),
        float(np.median(np.diff(np.sort(edges[:, 0]))) if m > 1 else 0),
        float(len(np.unique(edges[:, 0])) / max(n, 1)),
    ]
    return np.asarray(feats, dtype=np.float32)


class WACONetLite(nn.Module):
    """14-layer-inspired CNN over occupancy + MLP over graph stats."""

    def __init__(self, out_dim: int = 128):
        super().__init__()
        ch = 32
        layers = []
        in_ch = 1
        for i in range(8):
            stride = 1 if i == 0 else 2
            k = 5 if i == 0 else 3
            layers += [
                nn.Conv2d(in_ch, ch, kernel_size=k, stride=stride, padding=k // 2),
                nn.ReLU(inplace=True),
            ]
            in_ch = ch
        self.cnn = nn.Sequential(*layers)
        self.pool = nn.AdaptiveAvgPool2d(1)
        self.stats_mlp = nn.Sequential(
            nn.Linear(16, 64),
            nn.ReLU(),
            nn.Linear(64, 32),
            nn.ReLU(),
        )
        self.out = nn.Sequential(
            nn.Linear(ch + 32, 256),
            nn.ReLU(),
            nn.Linear(256, out_dim),
        )

    def forward(self, grid: torch.Tensor, stats: torch.Tensor) -> torch.Tensor:
        # grid: [B,1,H,W]
        x = self.cnn(grid)
        x = self.pool(x).flatten(1)
        s = self.stats_mlp(stats)
        return self.out(torch.cat([x, s], dim=1))


class RegionEmbedder(nn.Module):
    def __init__(self, out_dim: int = 128):
        super().__init__()
        self.layout = nn.Embedding(4, 32)
        self.kind = nn.Embedding(4, 32)
        self.mlp = nn.Sequential(
            nn.Linear(32 + 32 + 4, 128),
            nn.ReLU(),
            nn.Linear(128, out_dim),
        )

    def forward(self, layout_id, kind_id, cont: torch.Tensor) -> torch.Tensor:
        x = torch.cat([self.layout(layout_id), self.kind(kind_id), cont], dim=1)
        return self.mlp(x)


class WACOGCostModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.graph = WACONetLite(128)
        self.region = RegionEmbedder(128)
        self.final = nn.Sequential(
            nn.Linear(256, 128),
            nn.ReLU(),
            nn.Linear(128, 64),
            nn.ReLU(),
            nn.Linear(64, 1),
        )
        self._graph_cache = {}

    def encode_graph(self, grid, stats):
        return self.graph(grid, stats)

    def forward(self, grid, stats, layout_id, kind_id, cont):
        g = self.encode_graph(grid, stats)
        r = self.region(layout_id, kind_id, cont)
        return self.final(torch.cat([g, r], dim=1)).squeeze(-1)


def pairwise_hinge_loss(scores_j, scores_k, y_j, y_k):
    """L = sign(y_j - y_k) * max(0, 1 - (s_j - s_k)) with lower time better.

    We want score to RANK like cost (lower better), so if y_j < y_k (j faster),
    we want scores_j < scores_k.
    """
    # Prefer lower measured time => lower predicted score
    target = torch.sign(y_k - y_j)  # +1 if j faster than k
    # hinge on (s_k - s_j) when j is faster
    return F.relu(1.0 - target * (scores_k - scores_j)).mean()


def region_cont_features(row: dict) -> np.ndarray:
    # f_T, f_I, log totalOps, log execCount
    def lg(x):
        return math.log1p(max(float(x), 0.0))

    return np.asarray(
        [
            float(row.get("freq_T", 1 if row.get("kind") == "Traverse" else 0)),
            float(row.get("freq_I", 1 if row.get("kind") == "Insert" else 0)),
            lg(row.get("totalOps", 1)),
            lg(row.get("execCount", 1)),
        ],
        dtype=np.float32,
    )
