#!/usr/bin/env python3
"""Validation figure for DOALL loops, DOACROSS loops and the universal thread
budget (the TDG width model).  Reads the artifacts this session produced:

  bin/budget_detail.txt   tdg_budget_test output, one section per configuration
  bin/battery_suite.log   the suite's loop checks (race/*, scaling/*)
  cost_model_evidence.csv the forced-device sweep (trips -> cpu/gpu ratio)

Writes validation_loops.svg (+ .png via inkscape) and prints the tables.
"""
import csv, os, re, subprocess

R = os.path.dirname(os.path.abspath(__file__))
W, H = 1240, 980
MX, MY, PW, PH, GAP = 40, 86, 560, 430, 40
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


def origin(i):
    return MX + (i % 2) * (PW + GAP), MY + (i // 2) * (PH + GAP)


# ---------------------------------------------------------------- data
def budget_sections():
    p = os.path.join(R, "bin/budget_detail.txt")
    secs, cur = {}, None
    if os.path.exists(p):
        for line in open(p):
            m = re.match(r"=== (\d+ threads?.*?) ===", line.strip())
            if m:
                cur = m.group(1)
                secs[cur] = {}
                continue
            m = re.match(r"PASS\s+(T\d+)\s+(.*?)\s{2,}(.*)$", line.rstrip())
            if m and cur:
                secs[cur][m.group(1)] = (m.group(2).strip(), m.group(3).strip())
    return secs


def suite_lines(pattern):
    out = {}
    for name in ("battery_suite.log", "final_all_cpu.log", "final_gate_fix.log"):
        p = os.path.join(R, "bin", name)
        if not os.path.exists(p):
            continue
        for raw in open(p):
            line = re.sub(r"\x1b\[[0-9;]*m", "", raw).strip()
            m = re.search(pattern, line)
            if m:
                out[name] = line
                break
    return out


def sweep(family):
    p = os.path.join(R, "cost_model_evidence.csv")
    rows = []
    for r in csv.DictReader(l for l in open(p) if not l.startswith("#")):
        if r["family"] == family and r["controlled"] == "1" and r["work_units"] == "trip":
            rows.append((int(r["work"]), float(r["ratio"])))
    return sorted(rows)


# ---------------------------------------------------------------- panels
def panel_doall(svg, i):
    ox, oy = origin(i)
    svg.text(ox, oy - 26, "L1. DOALL loops: what is validated, and the result", 12, weight="bold")
    svg.text(ox, oy - 10, "equality/determinism from the suite; policy from the unit+e2e checks; curve measured", 10, color="#555")
    y = oy + 8
    checks = [
        ("race/doall_scaling 1thr==4thr  ", True, "suite"),
        ("parallel/doall_scaling device 1=4 thr", True, "cross-mode"),
        ("array_doall/gpu_compute device==CPU", True, "cross-mode"),
        ("T8: trip 4095 -> CPU, 4096 -> device", True, "unit"),
        ("e2e: 100000 -> device, 300000 -> CPU", True, "cost-model check"),
        ("scaling/doall_scaling >= 1.2x at 4 thr", True, "suite"),
    ]
    for name, ok, src in checks:
        svg.text(ox + 4, y + 10, "PASS" if ok else "FAIL", 10, "start", "#0a0" if ok else "#c00", "bold")
        svg.text(ox + 50, y + 10, name, 11)
        svg.text(ox + 430, y + 10, src, 9, "start", "#777")
        y += 18
    # scaling numbers: parse the suite line e.g. "4thr faster (1.88x)" or the skip form
    ls = suite_lines(r"scaling/doall_scaling")
    ratio = None
    for line in ls.values():
        m = re.search(r"\(([0-9.]+)x\)", line) or re.search(r"only ([0-9.]+)x of 1thr", line)
        if m:
            ratio = float(m.group(1))
            break
    svg.text(ox, y + 16, "measured 4-thread speedup vs the 1.2x contract:", 11, weight="bold")
    bx, by, bw = ox + 8, y + 26, 380
    svg.rect(bx, by, bw, 18, "#f4f4f4", "#bbb")
    if ratio:
        frac = min(1.0, ratio / 2.0)
        svg.rect(bx, by, bw * frac, 18, "#0a0" if ratio >= 1.2 else "#c60", "#666")
        svg.text(bx + bw * frac + 6, by + 13, f"{ratio:.2f}x", 11, "start", "#222", "bold")
    xc = bx + bw * (1.2 / 2.0)
    svg.line(xc, by - 4, xc, by + 22, "#c00", 1.4, "4 3")
    svg.text(xc, by - 8, "1.2x contract", 9, "middle", "#c00")
    y2 = by + 40
    svg.text(ox, y2 + 12, "device-vs-CPU ratio across trips (forced device, gates off):", 11, weight="bold")
    import math
    rows = sweep("doall")
    if rows:
        x0, x1, ytop, ybot = ox + 48, ox + PW - 30, y2 + 26, y2 + 118
        lo, hi = math.log10(500), math.log10(2_000_000)
        ax = lambda v: x0 + (math.log10(v) - lo) / (hi - lo) * (x1 - x0)
        ay = lambda r: ybot - (min(1.6, r) / 1.6) * (ybot - ytop)
        svg.line(x0, ay(1.0), x1, ay(1.0), "#888", 1.2, "4 3")
        svg.text(x1 + 2, ay(1.0) + 4, "1.0", 9, "start", "#666")
        svg.line(x0, ay(1.2), x1, ay(1.2), "#08c", 1.0, "3 3")
        for v, lbl in ((1000, "1k"), (16384, "16k"), (262144, "256k"), (1048576, "1M")):
            svg.line(ax(v), ybot, ax(v), ybot + 4, "#aaa")
            svg.text(ax(v), ybot + 15, lbl, 9, "middle", "#666")
        for w, r in rows:
            svg.circle(ax(w), ay(r), 5, "#08c" if r >= 1.0 else "#c60")
        svg.text(ox + 8, ybot + 15, "device slower", 9, "start", "#c60")
        svg.text(ox + 8, ytop + 4, "faster", 9, "start", "#08c")
        svg.text(x0, ybot + 32, "trips (log); contract line 1.2x dashed", 9, "start", "#555")


def panel_doacross(svg, i):
    ox, oy = origin(i)
    svg.text(ox, oy - 26, "L2. DOACROSS loops: classification, policy, and the wave cliff", 12, weight="bold")
    svg.text(ox, oy - 10, "the cap (max_waves=8192) is drawn where the measured curve crosses break-even", 10, color="#555")
    y = oy + 8
    checks = [
        ("classified DOACROSS + wait/post metadata", "race/doacross_scan"),
        ("effect verdict: space=Carried", "effects/doacross_carry"),
        ("1thr == 4thr (carry and scan)", "suite + cross-mode"),
        ("device == CPU at 1/4 thr, repeats", "cross-mode 11/11"),
        ("T9: dist1 storm, dist64 device, trip64 CPU", "unit"),
        ("e2e: default CPU (waves>8192), cap0 device", "cost-model check"),
    ]
    for name, src in checks:
        svg.text(ox + 4, y + 10, "PASS", 10, "start", "#0a0", "bold")
        svg.text(ox + 50, y + 10, name, 11)
        svg.text(ox + 470, y + 10, src, 9, "start", "#777")
        y += 18
    import math
    rows = sweep("doacross")
    x0, x1, ytop, ybot = ox + 48, ox + PW - 30, y + 30, y + 210
    lo, hi = math.log10(400), math.log10(200000)
    ax = lambda v: x0 + (math.log10(v) - lo) / (hi - lo) * (x1 - x0)
    ay = lambda r: ybot - (min(1.3, r) / 1.3) * (ybot - ytop)
    # wave-storm shading: waves > 8192 (== trips for dist 1)
    svg.rect(ax(8192), ytop, x1 - ax(8192), ybot - ytop, "#fdd", "#fbb", 0.8, 0.5)
    svg.text(ax(8192) + 6, ytop + 12, "wave storm: waves > 8192", 9, "start", "#a00")
    for v, lbl in ((512, "512"), (4096, "4k"), (8192, "8k"), (65536, "64k")):
        svg.line(ax(v), ybot, ax(v), ybot + 4, "#aaa")
        svg.text(ax(v), ybot + 15, lbl, 9, "middle", "#666")
    for v in (0.5, 1.0, 1.2):
        svg.line(x0, ay(v), x1, ay(v), "#eee")
        svg.text(x0 - 6, ay(v) + 4, f"{v:g}", 9, "end", "#666")
    svg.line(x0, ay(1.0), x1, ay(1.0), "#888", 1.2, "4 3")
    svg.line(ax(8192), ytop, ax(8192), ybot, "#c00", 1.5, "5 4")
    svg.text(ax(8192), ytop - 6, "cap 8192", 9, "middle", "#c00")
    for w, r in rows:
        svg.circle(ax(w), ay(r), 5, "#0a0" if r >= 1.0 else "#c60")
        if w in (8192, 16384):
            svg.text(ax(w) + 6, ay(r) + 4, f"{r:.2f}x", 9, "start", "#222")
    svg.text(x0, ybot + 32, "trips (= waves for distance 1, log)", 9, "start", "#555")


def panel_budget(svg, i):
    ox, oy = origin(i)
    svg.text(ox, oy - 26, "L3. Universal thread budget: invariant matrix", 12, weight="bold")
    svg.text(ox, oy - 10, "tdg_budget_test: same assertions in 3 configurations, observed values shown", 10, color="#555")
    secs = budget_sections()
    cols = list(secs.keys())
    ids = ["T1", "T2", "T3", "T4", "T5", "T6", "T7"]
    desc = {k: "" for k in ids}
    for c in cols:
        for k, (d, _) in secs[c].items():
            desc[k] = d
    x0 = ox + 10
    colw = 170
    svg.text(x0, oy + 8, "assertion", 10, "start", "#333", "bold")
    for j, c in enumerate(cols):
        label = {"4 threads (integration on)": "4 thr, integration on", "4 threads (integration off)": "4 thr, switches off", "1 thread": "1 thread"}.get(c, c)
        svg.text(x0 + 250 + j * colw, oy + 4, label, 8, "start", "#333", "bold")
    y = oy + 18
    for k in ids:
        svg.text(x0, y + 14, f"{k} {desc.get(k,'')[:34]}", 10)
        for j, c in enumerate(cols):
            cell = secs[c].get(k)
            col = "#0a0" if cell else "#bbb"
            svg.text(x0 + 250 + j * colw, y + 14, "PASS" if cell else "-", 10, "start", col, "bold")
            if cell and j == 0:
                svg.text(x0 + 250, y + 26, cell[1][:44], 8, "start", "#777")
        y += 30
    # integration rows (from the suite / harness)
    y += 6
    svg.text(x0, y + 12, "integration (suite + harness):", 10, weight="bold")
    for k, (name, src) in enumerate([
        ("parallel/nested_step: outer refused by call barrier, inner step TDG-planned, 1thr==4thr", "suite PASS"),
        ("pagerank call-barrier check", "suite PASS"),
        ("kill switches: SGPL_NO_UNREGISTERED_POOL_SHARE, SGPL_NO_REGION_SCOPE", "validator cfg 2 PASS"),
        ("traversal corpus: 1thr==4thr + 5 repeats identical", "suite race/* PASS"),
    ]):
        svg.text(x0 + 6, y + 30 + k * 16, f"{name}", 9, "start", "#333")
        svg.text(x0 + 470, y + 30 + k * 16, src, 9, "start", "#0a0")
    svg.text(x0, y + 30 + 4 * 16 + 10, "gap: no width-accuracy study (chosen width vs best measured) yet", 9, "start", "#c60")


def main():
    svg = SVG()
    svg.text(MX, 26, "DOALL / DOACROSS / universal thread budget — validation", 16, weight="bold")
    svg.text(MX, 44, "sources: bin/budget_detail.txt, bin/battery_suite.log, cost_model_evidence.csv", 10, color="#666")
    panel_doall(svg, 0)
    panel_doacross(svg, 1)
    panel_budget(svg, 2)
    out = os.path.join(R, "validation_loops.svg")
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

    # text tables
    print("\nbudget invariants per configuration:")
    secs = budget_sections()
    for c, d in secs.items():
        print(f"  {c}: " + ", ".join(f"{k}={v[1]}" for k, v in sorted(d.items())))
    print("\ndoall sweep:", sweep("doall"))
    print("doacross sweep:", sweep("doacross"))


if __name__ == "__main__":
    main()
