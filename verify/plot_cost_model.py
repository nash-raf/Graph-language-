#!/usr/bin/env python3
"""Render cost_model_evidence.csv -> cost_model_evidence.svg + markdown tables.
No third-party deps.  Run after gpu_cost_sweep.sh to update the curve."""
import csv, math, sys, os
R = os.path.dirname(os.path.abspath(__file__))
rows = [r for r in csv.DictReader(l for l in open(os.path.join(R, "cost_model_evidence.csv")) if not l.startswith("#"))]
if not rows:
    sys.exit("no rows")
W, H, ML, MR, MT, MB = 900, 520, 90, 200, 60, 70
lo, hi = math.log10(500), math.log10(3_000_000)
def xs(w): return ML + (math.log10(max(500, w)) - lo) / (hi - lo) * (W - ML - MR)
def ys(r):
    r = max(0.0, min(1.8, r)); return MT + (1.8 - r) / 1.8 * (H - MT - MB)
s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" font-family="monospace" font-size="12">',
     f'<rect width="{W}" height="{H}" fill="white"/>']
for r in (0.0, 0.5, 1.0, 1.5):
    y = ys(r); s.append(f'<line x1="{ML}" y1="{y:.1f}" x2="{W-MR}" y2="{y:.1f}" stroke="#ddd"/>')
    s.append(f'<text x="{ML-8}" y="{y+4:.1f}" text-anchor="end">{r:.1f}</text>')
y1 = ys(1.0); s.append(f'<line x1="{ML}" y1="{y1:.1f}" x2="{W-MR}" y2="{y1:.1f}" stroke="#888" stroke-width="1.5" stroke-dasharray="4 3"/>')
s.append(f'<text x="{W-MR+8}" y="{y1+4:.1f}" fill="#555">break-even</text>')
for w, lbl in ((4096, "4k"), (200000, "200k"), (2000000, "2M")):
    x = xs(w); s.append(f'<line x1="{x:.1f}" y1="{MT}" x2="{x:.1f}" y2="{H-MB}" stroke="#eee"/>')
    s.append(f'<text x="{x:.1f}" y="{H-MB+18}" text-anchor="middle">{lbl}</text>')
for w, lbl, col in ((4096, "min_trips 4096", "#08c"), (2000000, "engine floor 2M arcs", "#c00")):
    x = xs(w); s.append(f'<line x1="{x:.1f}" y1="{MT}" x2="{x:.1f}" y2="{H-MB}" stroke="{col}" stroke-width="1.5" stroke-dasharray="6 4"/>')
    s.append(f'<text x="{x+5:.1f}" y="{MT+14}" fill="{col}">{lbl}</text>')
for r in rows:
    x, y = xs(float(r["work"])), ys(float(r["ratio"]))
    if r["controlled"] == "1":
        col = "#c00" if float(r["ratio"]) < 1 else "#080"
        s.append(f'<polygon points="{x:.1f},{y-7:.1f} {x+7:.1f},{y+5:.1f} {x-7:.1f},{y+5:.1f}" fill="{col}" fill-opacity="0.75"/>')
    elif "backend only" in r["ran_on_device"]:
        s.append(f'<rect x="{x-5:.1f}" y="{y-5:.1f}" width="10" height="10" fill="#777" fill-opacity="0.6"/>')
    else:
        s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="6" fill="#08c" fill-opacity="0.75"/>')
lx, ly = W - MR + 14, MT + 70
for i, (sym, lab, col) in enumerate((("triangle", "controlled median (warm off)", "#c00"),
                                     ("circle", "single pod run", "#08c"),
                                     ("square", "engine step declined (backend only)", "#777"))):
    s.append(f'<text x="{lx}" y="{ly+i*20}" fill="{col}">{sym} {lab}</text>')
s.append(f'<text x="{ML}" y="{H-24}">work items (trips for kernels, arcs for engine steps), log scale</text>')
s.append(f'<text x="{ML}" y="{MT-24}">Device cost-model evidence: speedup cpu/gpu (&gt;1 = device faster) vs gate thresholds</text>')
s.append("</svg>")
open(os.path.join(R, "cost_model_evidence.svg"), "w").write("\n".join(s))
print(f"rendered {len(rows)} points -> cost_model_evidence.svg")
