#!/usr/bin/env python3
"""Render the whole p1 validation as one dashboard SVG (+ PNG via inkscape).

Panels
  A  schedule cost model (GPU): device-vs-CPU speedup across sizes from
     cost_model_evidence.csv, with the gate thresholds drawn
  B  format model (CPU layouts): runtime per layout per workload from
     bin/fmt/results.csv, conversion cost annotated
  C  region-prediction accuracy: measured/predicted per case per machine from
     region_prediction.csv, with the 5x contract line
  D  contract summary: what passed, with counts, from this session's runs

Stdlib only; inkscape/convert used opportunistically for the PNG.
"""
import csv
import os
import subprocess

R = os.path.dirname(os.path.abspath(__file__))
W, H = 1240, 940
PANEL = 2  # 2x2
PW, PH = 560, 400
MX, MY = 40, 70
GAP = 40


def esc(s):
    return (str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"))


class SVG:
    def __init__(self):
        self.parts = []

    def add(self, s):
        self.parts.append(s)

    def text(self, x, y, s, size=11, anchor="start", color="#222", weight="normal"):
        self.add(f'<text x="{x:.1f}" y="{y:.1f}" font-size="{size}" text-anchor="{anchor}" '
                 f'fill="{color}" font-weight="{weight}" font-family="monospace">{esc(s)}</text>')

    def rect(self, x, y, w, h, fill, stroke="#999", sw=0.8, op=1.0):
        self.add(f'<rect x="{x:.1f}" y="{y:.1f}" width="{max(0.5,w):.1f}" height="{max(0.5,h):.1f}" '
                 f'fill="{fill}" fill-opacity="{op}" stroke="{stroke}" stroke-width="{sw}"/>')

    def line(self, x1, y1, x2, y2, color="#888", sw=1.0, dash=None):
        d = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" '
                 f'stroke="{color}" stroke-width="{sw}"{d}/>')

    def circle(self, x, y, r, fill, op=0.8):
        self.add(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="{fill}" fill-opacity="{op}"/>')

    def out(self, path):
        head = (f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
                f'viewBox="0 0 {W} {H}">\n<rect width="{W}" height="{H}" fill="#ffffff"/>\n')
        open(path, "w").write(head + "\n".join(self.parts) + "\n</svg>\n")


def panel_origin(i):
    return MX + (i % PANEL) * (PW + GAP), MY + (i // PANEL) * (PH + GAP)


# ── data ────────────────────────────────────────────────────────────────────
def load_evidence():
    p = os.path.join(R, "cost_model_evidence.csv")
    if not os.path.exists(p):
        return []
    return [r for r in csv.DictReader(l for l in open(p) if not l.startswith("#"))]


def load_fmt():
    p = os.path.join(R, "bin/fmt/results.csv")
    rows = []
    if os.path.exists(p):
        for line in open(p):
            f = line.strip().split(",")
            if len(f) >= 3:
                rows.append({"tag": f[0], "mode": f[1], "ms": int(f[2]),
                             "ans": f[3] if len(f) > 3 else "", "conv": f[4] if len(f) > 4 else ""})
    return rows


def load_pred():
    p = os.path.join(R, "region_prediction.csv")
    rows = []
    if os.path.exists(p):
        for r in csv.DictReader(l for l in open(p) if not l.startswith("#")):
            rows.append(r)
    return rows


# ── panel A: schedule cost model ────────────────────────────────────────────
def panel_a(svg, i):
    ox, oy = panel_origin(i)
    svg.text(ox, oy - 26, "A. Schedule cost model (GPU): device vs CPU", 12, weight="bold")
    svg.text(ox, oy - 10, "forced device, gates off, median of 3; >1 = device faster",
             10, color="#555")
    rows = [r for r in load_evidence() if r.get("controlled") == "1"
            and r.get("work_units") in ("trip", "arcs")]
    if not rows:
        svg.text(ox + 10, oy + 40, "no sweep data yet", 11, color="#a00")
        return
    import math
    xs = lambda w: ox + 40 + (math.log10(max(400, int(w))) - math.log10(400)) / \
        (math.log10(2_100_000) - math.log10(400)) * (PW - 70)
    lo, hi = 0.0, 1.6
    ys = lambda r: oy + 20 + (hi - max(lo, min(hi, r))) / (hi - lo) * (PH - 70)
    # grid
    for v in (0.0, 0.4, 0.8, 1.2, 1.6):
        svg.line(ox + 40, ys(v), ox + PW - 30, ys(v), "#eee")
        svg.text(ox + 34, ys(v) + 4, f"{v:.1f}", 10, "end", "#666")
    svg.line(ox + 40, ys(1.0), ox + PW - 30, ys(1.0), "#888", 1.4, "5 4")
    svg.text(ox + PW - 28, ys(1.0) + 4, "break-even", 10, "start", "#555")
    for w, lbl, col in ((4096, "min_trips 4096", "#08c"), (2000000, "engine floor 2M", "#c00")):
        if w <= 2_100_000:
            svg.line(xs(w), oy + 20, xs(w), oy + PH - 50, col, 1.2, "6 4")
            svg.text(xs(w) - 4, oy + 30, lbl, 9, "end", col)
    for w, lbl in ((1000, "1k"), (4096, "4k"), (16384, "16k"), (262144, "256k"), (1048576, "1M")):
        svg.line(xs(w), oy + PH - 50, xs(w), oy + PH - 46, "#aaa")
        svg.text(xs(w), oy + PH - 36, lbl, 9, "middle", "#666")
    svg.text(ox + PW / 2, oy + PH - 16, "work items (trips for kernels, arcs for engine steps), log", 10, "middle", "#444")
    colors = {"doall": "#08c", "doacross": "#c60", "engine-step": "#a00"}
    for r in rows:
        v = float(r["ratio"])
        x, y = xs(int(r["work"])), ys(v)
        if r["work_units"] == "arcs":
            svg.add(f'<rect x="{x-5:.1f}" y="{y-5:.1f}" width="10" height="10" '
                    f'fill="{colors["engine-step"]}" fill-opacity="0.8"/>')
        else:
            svg.circle(x, y, 5, colors.get(r["family"], "#333"))
    # legend inside the plot, lower-left, away from the threshold labels
    ly = oy + PH - 96
    for k, (sym, lab, col) in enumerate((("c", "doall (trips)", "#08c"),
                                         ("c", "doacross (trips)", "#c60"),
                                         ("s", "engine step (arcs)", "#a00"))):
        yy = ly + k * 16
        if sym == "c":
            svg.circle(ox + 56, yy, 4, col)
        else:
            svg.add(f'<rect x="{ox+52:.1f}" y="{yy-4:.1f}" width="8" height="8" fill="{col}" fill-opacity="0.8"/>')
        svg.text(ox + 66, yy + 4, lab, 10, "start", "#333")


# ── panel B: format model ───────────────────────────────────────────────────
def panel_b(svg, i):
    ox, oy = panel_origin(i)
    svg.text(ox, oy - 26, "B. Format model: runtime per forced layout", 12, weight="bold")
    svg.text(ox, oy - 10, "compiled with AUTOTUNER_FORCE_LAYOUT; all modes must give the same answer",
             10, color="#555")
    rows = load_fmt()
    tags, modes = [], []
    for r in rows:
        if r["tag"] not in tags:
            tags.append(r["tag"])
        if r["mode"] not in modes:
            modes.append(r["mode"])
    for m in ("cost-model", "CSR", "PCSR", "BCSR", "SET"):
        if m not in modes:
            modes.append(m)
    if not tags:
        svg.text(ox + 10, oy + 40, "no format data yet", 11, color="#a00")
        return
    import math
    def bar_h(ms):
        return (math.log10(max(1, ms)) / math.log10(1_000_000)) * (PH - 110)
    colors = {"cost-model": "#333", "CSR": "#08c", "PCSR": "#0a0", "BCSR": "#c60", "SET": "#a00"}
    slot = (PW - 60) / len(tags)
    for ti, tag in enumerate(tags):
        bx = ox + 50 + ti * slot
        for mi, m in enumerate(modes):
            r = next((x for x in rows if x["tag"] == tag and x["mode"] == m), None)
            if not r:
                continue
            h = bar_h(r["ms"])
            x = bx + mi * (slot - 10) / len(modes)
            svg.rect(x, oy + PH - 70 - h, (slot - 14) / len(modes) - 2, h, colors.get(m, "#999"), "#666")
            if r["ms"] >= 600000:
                svg.text(x + 2, oy + PH - 74 - h, "timeout", 9, "start", "#a00")
        svg.text(bx + (slot - 10) / 2, oy + PH - 52, tag, 10, "middle", "#333")
    for ms, lbl in ((1, "1ms"), (100, "100ms"), (10000, "10s"), (600000, "10m+")):
        y = oy + PH - 70 - bar_h(ms)
        svg.line(ox + 46, y, ox + PW - 30, y, "#eee")
        svg.text(ox + 42, y + 3, lbl, 9, "end", "#666")
    svg.text(ox + 8, oy + 30, "●model ●CSR ●PCSR ●BCSR ●SET", 10, "start", "#333")
    for k, (c, m) in enumerate((("#333", "cost-model"), ("#08c", "CSR"), ("#0a0", "PCSR"),
                                ("#c60", "BCSR"), ("#a00", "SET"))):
        svg.circle(ox + 10 + k * 72, oy + 27, 4, c)
    svg.text(ox + 40, oy + PH - 16, "workload (all modes answer-identical where reported)",
             10, "middle", "#444")


# ── panel C: region prediction ──────────────────────────────────────────────
def panel_c(svg, i):
    ox, oy = panel_origin(i)
    svg.text(ox, oy - 26, "C. CPU region model: predicted vs measured", 12, weight="bold")
    svg.text(ox, oy - 10, "identity = perfect; band = the 5x contract; gpu-backend rows profile device work",
             10, color="#555")
    rows = []
    p = os.path.join(R, "region_prediction_pairs.csv")
    if os.path.exists(p):
        for r in csv.DictReader(l for l in open(p) if not l.startswith("#")):
            rows.append(r)
    if not rows:
        svg.text(ox + 10, oy + 40, "no data", 11, color="#a00")
        return
    import math
    lo, hi = 0.5, 200.0
    def xs(v):
        return ox + 50 + (math.log10(max(lo, v)) - math.log10(lo)) / (math.log10(hi) - math.log10(lo)) * (PW - 90)
    def ys(v):
        return oy + PH - 60 - (math.log10(max(lo, v)) - math.log10(lo)) / (math.log10(hi) - math.log10(lo)) * (PH - 120)
    for v in (0.5, 1, 5, 20, 200):
        svg.line(xs(v), oy + 20, xs(v), oy + PH - 60, "#eee")
        svg.line(ox + 50, ys(v), ox + PW - 40, ys(v), "#eee")
        svg.text(xs(v), oy + PH - 46, f"{v:g}", 9, "middle", "#666")
        svg.text(ox + 44, ys(v) + 3, f"{v:g}", 9, "end", "#666")
    svg.line(ox + 50, ys(1), ox + PW - 40, ys(1), "#888", 1.2, "4 3")
    svg.line(xs(1), oy + 20, xs(1), oy + PH - 60, "#888", 1.2, "4 3")
    # 5x band around the identity (y = 5x and y = x/5)
    pts_hi = " ".join(f"{xs(v):.1f},{ys(min(hi, v * 5)):.1f}" for v in (0.5, 2, 8, 40))
    pts_lo = " ".join(f"{xs(v):.1f},{ys(max(lo, v / 5)):.1f}" for v in (2.5, 10, 50, 200))
    svg.add(f'<polyline points="{pts_hi}" fill="none" stroke="#c00" stroke-width="1.2" stroke-dasharray="5 4"/>')
    svg.add(f'<polyline points="{pts_lo}" fill="none" stroke="#c00" stroke-width="1.2" stroke-dasharray="5 4"/>')
    series = {}
    for r in rows:
        k = (r["machine"], r["mode"])
        series.setdefault(k, []).append(r)
    cols = {"i3-9100F": "#0a0", "EPYC-7352 (fresh calib)": "#c60"}
    for (machine, mode), rs in series.items():
        for r in rs:
            x, y = xs(float(r["predicted_ms"])), ys(float(r["measured_ms"]))
            if mode == "gpu-backend":
                svg.add(f'<rect x="{x-4:.1f}" y="{y-4:.1f}" width="8" height="8" '
                        f'fill="none" stroke="{cols[machine]}" stroke-width="1.6"/>')
            else:
                svg.circle(x, y, 5, cols.get(machine, "#333"))
    ly = oy + 22
    svg.circle(49 + ox, ly, 4, "#0a0"); svg.text(ox + 58, ly + 4, "i3-9100F cpu-only", 10, "start", "#333")
    svg.circle(ox + 190, ly, 4, "#c60"); svg.text(ox + 199, ly + 4, "EPYC cpu-only", 10, "start", "#333")
    svg.add(f'<rect x="{ox+300:.1f}" y="{ly-4:.1f}" width="8" height="8" fill="none" stroke="#c60" stroke-width="1.6"/>')
    svg.text(ox + 313, ly + 4, "EPYC gpu-backend", 10, "start", "#333")
    svg.text(ox + PW / 2, oy + PH - 16, "predicted ms (log)", 10, "middle", "#444")
    svg.text(ox + 10, oy + PH / 2, "measured", 10, "middle", "#444")


# ── panel D: contract summary ───────────────────────────────────────────────
def panel_d(svg, i):
    ox, oy = panel_origin(i)
    svg.text(ox, oy - 26, "D. Contract checks (this session)", 12, weight="bold")
    svg.text(ox, oy - 10, "each row names its artifact; ✓ = reproduced after the last source change",
             10, color="#555")
    rows = [
        ("T1-T7 CPU budget model", "ALL PASS x3 configs", "validate_tdg_budget.sh", "#0a0"),
        ("T8-T10 device policy", "ALL PASS x3 configs", "tdg_budget_test.c", "#0a0"),
        ("GPU cost-model direction", "14/14 (box), 8/0/6 (cpu-only)", "gpu_cost_model_check.sh", "#0a0"),
        ("Cross-mode equality (11 fixtures)", "11/11 CPU=dev, 1=4 thr, repeats", "gpu_cross_mode_check.sh", "#0a0"),
        ("Broad equality (50 fixtures)", "50/50 both boxes", "gpu_broad_cross_check.sh", "#0a0"),
        ("Scale (780k/157k graphs)", "3/3 device==CPU", "gpu_scale_check.sh", "#0a0"),
        ("Device harness (proof + fallback)", "11 pass / 0 fail / 2 policy-skip", "gpu_check.sh", "#0a0"),
        ("Local suite (CPU-only)", "93 pass / 0 fail", "verify/run.sh", "#0a0"),
        ("Box suite (GPU)", "90 pass / 4 fail: 1 load, 3 prediction", "verify/run.sh", "#c60"),
        ("Format answers across layouts", "identical (kcore, big_kcore, bfs16m)", "p1_format_probe.sh", "#0a0"),
        ("Force-layout wiring", "was dead; fixed, now converts", "AutoTunerPass.cpp", "#0a0"),
        ("Region model: local (CPU-only)", "all 5 within 5x (1.0-3.0x)", "region_prediction_pairs.csv", "#0a0"),
        ("Region model: EPYC pod (CPU-only)", "cc 3.0x ok; bfs 6.1x, pagerank 7.8x off", "same file", "#c60"),
        ("Region model: EPYC (GPU backend)", "inflated by device work (cc 118x)", "model predicts CPU only", "#c60"),
    ]
    y = oy + 6
    for name, res, art, col in rows:
        svg.text(ox + 4, y + 12, "✓" if col == "#0a0" else "~", 12, "start", col, "bold")
        svg.text(ox + 22, y + 12, name, 11, "start", "#222")
        svg.text(ox + 250, y + 12, res, 10, "start", col)
        svg.text(ox + 250, y + 25, art, 9, "start", "#777")
        y += 30


def main():
    svg = SVG()
    svg.text(MX, 30, "p1 cost-model validation dashboard", 17, weight="bold")
    panel_a(svg, 0)
    panel_b(svg, 1)
    panel_c(svg, 2)
    panel_d(svg, 3)
    out = os.path.join(R, "validation_dashboard.svg")
    svg.out(out)
    png_ok = False
    for cmd in (["inkscape", "--export-type=png", f"--export-filename={out[:-4]}.png", out],
                ["convert", out, out[:-4] + ".png"]):
        try:
            subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL, timeout=120)
            png_ok = True
            break
        except Exception:
            continue
    print(f"wrote {out}" + (f" and {out[:-4]}.png" if png_ok else " (no PNG converter worked)"))


if __name__ == "__main__":
    main()
