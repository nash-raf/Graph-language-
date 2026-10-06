#!/usr/bin/env python3
"""Separate figure: DOALL loop, predicted vs actual.

Left  : 4-thread time, predicted (the budget model's scaling law t1/min(4,trips))
        against actual, log-log, with the identity and the 5x band.
Right : achieved speedup vs work, with the model's 4x line and the suite's 1.2x
        contract; the trivial array loop is shown as a contrast series.
Reads bin/doall/doall_pred.csv (written by doall_pred_measure.sh).
"""
import csv, os, subprocess

R = os.path.dirname(os.path.abspath(__file__))
W, H = 1240, 560
MX, MY, PW, PH, GAP = 45, 70, 560, 400, 40
esc = lambda s: str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


class SVG:
    def __init__(self):
        self.p = []

    def text(self, x, y, s, size=11, anchor="start", color="#222", weight="normal"):
        self.p.append(f'<text x="{x:.1f}" y="{y:.1f}" font-size="{size}" text-anchor="{anchor}" '
                      f'fill="{color}" font-weight="{weight}" font-family="monospace">{esc(s)}</text>')

    def rect(self, x, y, w, h, fill, stroke="#999", sw=0.8, op=1.0):
        self.p.append(f'<rect x="{x:.1f}" y="{y:.1f}" width="{max(0.5,w):.1f}" height="{max(0.5,h):.1f}" '
                      f'fill="{fill}" fill-opacity="{op}" stroke="{stroke}" stroke-width="{sw}"/>')

    def line(self, x1, y1, x2, y2, color="#888", sw=1.0, dash=None):
        d = f' stroke-dasharray="{dash}"' if dash else ""
        self.p.append(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" '
                      f'stroke="{color}" stroke-width="{sw}"{d}/>')

    def circle(self, x, y, r, fill, op=0.85, stroke=None, sw=0.0):
        s = f' stroke="{stroke}" stroke-width="{sw}"' if stroke else ""
        self.p.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="{fill}" fill-opacity="{op}"{s}/>')

    def out(self, path):
        open(path, "w").write(
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">\n'
            f'<rect width="{W}" height="{H}" fill="#fff"/>\n' + "\n".join(self.p) + "\n</svg>\n")


rows = []
p = os.path.join(R, "bin/doall/doall_pred.csv")
if os.path.exists(p):
    rows = [r for r in csv.DictReader(open(p))]

import math
svg = SVG()
svg.text(MX, 26, "DOALL loop: predicted vs actual (4 threads)", 16, weight="bold")
svg.text(MX, 44, "prediction = the budget model's scaling law t1/min(4,trips) — the compiler emits no time model for array loops",
         10, color="#666")

# ---- left: predicted vs actual, log-log
ox, oy = MX, MY
svg.text(ox, oy - 12, "4-thread time: predicted vs actual", 12, weight="bold")
x0, y0, w, h = ox + 55, oy + 15, PW - 90, PH - 90
lo, hi = 1e-3, 0.3
def sx(v): return x0 + (math.log10(v) - math.log10(lo)) / (math.log10(hi) - math.log10(lo)) * w
def sy(v): return y0 + h - (math.log10(v) - math.log10(lo)) / (math.log10(hi) - math.log10(lo)) * h
for v in (1e-3, 1e-2, 1e-1):
    svg.line(sx(v), y0, sx(v), y0 + h, "#eee"); svg.line(x0, sy(v), x0 + w, sy(v), "#eee")
    svg.text(sx(v), y0 + h + 14, f"{v*1000:g}ms", 9, "middle", "#666")
    svg.text(x0 - 6, sy(v) + 3, f"{v*1000:g}ms", 9, "end", "#666")
svg.line(sx(1e-3), sy(1e-3), sx(0.3), sy(0.3), "#888", 1.2, "4 3")
svg.text(sx(0.15), sy(0.15) + 4, "identity", 9, "start", "#555")
# 5x band
svg.line(sx(1e-3), sy(5e-3), sx(0.06), sy(0.3), "#c00", 1.1, "5 4")
svg.line(sx(5e-3), sy(1e-3), sx(0.3), sy(0.06), "#c00", 1.1, "5 4")
svg.text(sx(0.05), sy(0.28), "5x band", 9, "start", "#c00")
col = {"doall_scaling": "#08c", "array_doall": "#c60"}
for r in rows:
    x, y = sx(float(r["pred4_s"])), sy(float(r["t4_s"]))
    svg.circle(x, y, 5, col[r["shape"]])
    svg.text(x - 8, y + 4, f"{r['work']} ({r['speedup']}x)", 9, "end", "#333")
svg.text(x0, y0 + h + 32, "predicted 4-thread time (log)", 10, "middle", "#444")
svg.text(ox, oy + 4, "actual", 10, "middle", "#444")
svg.circle(x0 + 8, oy + 2, 4, "#08c"); svg.text(x0 + 18, oy + 6, "doall_scaling (compute-bound)", 10)
svg.circle(x0 + 230, oy + 2, 4, "#c60"); svg.text(x0 + 240, oy + 6, "array_doall (trivial)", 10)

# ---- right: speedup vs work
ox2, oy2 = MX + PW + GAP, MY
svg.text(ox2, oy2 - 12, "achieved speedup vs the model", 12, weight="bold")
x0, y0, w, h = ox2 + 55, oy2 + 15, PW - 90, PH - 90
lo, hi = math.log10(50000), math.log10(5e6)
def sxw(v): return x0 + (math.log10(v) - lo) / (hi - lo) * w
lo2, hi2 = 0.0, 4.4
def syv(v): return y0 + h - (min(hi2, max(lo2, v)) - lo2) / (hi2 - lo2) * h
for v in (0, 1, 2, 3, 4):
    svg.line(x0, syv(v), x0 + w, syv(v), "#eee")
    svg.text(x0 - 6, syv(v) + 3, f"{v}x", 9, "end", "#666")
for v, lbl in ((100000, "100k"), (500000, "500k"), (2000000, "2M"), (4000000, "4M")):
    svg.line(sxw(v), y0, sxw(v), y0 + h, "#f4f4f4")
    svg.text(sxw(v), y0 + h + 14, lbl, 9, "middle", "#666")
svg.line(x0, syv(4), x0 + w, syv(4), "#08c", 1.4, "6 4")
svg.text(x0 + w, syv(4) - 5, "model assumption: 4x (width clamp)", 9, "end", "#08c")
svg.line(x0, syv(1.2), x0 + w, syv(1.2), "#c00", 1.4, "5 4")
svg.text(x0 + w, syv(1.2) - 5, "suite contract: 1.2x", 9, "end", "#c00")
pts = {"doall_scaling": [], "array_doall": []}
for r in rows:
    trip = 20000 * int(r["work"].split("x")[0]) if "x" in r["work"] else int(r["work"])
    pts[r["shape"]].append((trip, float(r["speedup"])))
for shape, ps in pts.items():
    ps.sort()
    d = " ".join(f"{sxw(t):.1f},{syv(v):.1f}" for t, v in ps)
    svg.p.append(f'<polyline points="{d}" fill="none" stroke="{col[shape]}" stroke-width="1.6"/>')
    for t, v in ps:
        svg.circle(sxw(t), syv(v), 5, col[shape])
        svg.text(sxw(t), syv(v) + (16 if shape == "doall_scaling" else -8), f"{v:.2f}x", 9, "middle", "#333")
svg.text((x0 + w / 2), y0 + h + 32, "work (trips, log)", 10, "middle", "#444")
svg.text(ox2 + 4, oy2 + 4, "speedup 1thr -> 4thr", 10, "middle", "#444")

out = os.path.join(R, "validation_doall_pred.svg")
svg.out(out)
ok = False
for cmd in (["inkscape", "--export-type=png", f"--export-filename={out[:-4]}.png", out],
            ["convert", out, out[:-4] + ".png"]):
    try:
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=120)
        ok = True
        break
    except Exception:
        continue
print(f"wrote {out}" + (f" and {out[:-4]}.png" if ok else " (no png converter)"))
print("\nshape,work,t1_s,t4_s,pred4_s,speedup,pred_speedup,answers_equal")
for r in rows:
    print(",".join(r.values()))
