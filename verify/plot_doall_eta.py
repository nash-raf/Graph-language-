#!/usr/bin/env python3
"""DOALL loop: predicted vs measured per thread count (the eta table + figure).

Reads bin/doall/eta.csv (doall_eta_measure.sh).  The model's prediction for one
dispatch, from its own numbers:
    pred_serial   = c_ns * N
    pred_P        = L_total(P) + c_ns * N / P          (its break-even inequality,
                                                        rearranged: N > L/(c(1-1/P)))
    eta_P         = pred_P / measured_P,  1.00x = perfect
measured_P is the program's wall time divided by the round count (the round body
of the DOALL subject is the loop).  c_ns for the serial prediction is taken from
the P=2 row of the same configuration (c is width-independent; P=1 never reaches
the cost model).
"""
import csv, math, os, subprocess

R = os.path.dirname(os.path.abspath(__file__))
rows = [r for r in csv.DictReader(open(os.path.join(R, "bin/doall/eta.csv")))]
doall = {}
for r in rows:
    if r["fixture"] != "doall_scaling":
        continue
    try:
        N, P, rounds = int(r["N"]), int(r["P"]), int(r["rounds"])
        t = float(r["t_s"])
    except ValueError:
        continue
    if N <= 0:
        continue
    key = (r["graph"], N, rounds)
    doall.setdefault(key, {})[P] = (t, r)

print(f"{'graph':34} {'N':>8} {'P':>2} {'meas/disp ms':>12} {'pred ms':>9} {'eta':>6}  notes")
table = []
for (graph, N, rounds) in sorted(doall, key=lambda k: k[1]):
    per = doall[(graph, N, rounds)]
    c_ns = None
    for P in (2, 3, 4):
        if P in per and per[P][1]["c_ns"] not in ("", "-"):
            c_ns = float(per[P][1]["c_ns"])
            break
    if c_ns is None:
        continue
    for P in (1, 2, 3, 4):
        if P not in per:
            continue
        t_s, r = per[P]
        meas_ms = t_s / rounds * 1000.0
        if P == 1:
            pred_ms = c_ns * N / 1e6
            note = "serial anchor (c from P=2)"
        else:
            L = float(r["L_total_ns"])
            pred_ms = (L + c_ns * N / P) / 1e6
            note = f"L={L:.0f}ns c={c_ns:.0f}ns {r['c_state']}"
        eta = pred_ms / meas_ms
        table.append((graph, N, P, meas_ms, pred_ms, eta))
        print(f"{graph:34} {N:>8} {P:>2} {meas_ms:>12.3f} {pred_ms:>9.3f} {eta:>6.2f}  {note}")

# ---- markdown table
md = ["| graph | N | P | measured/dispatch (ms) | predicted (ms) | eta = pred/meas |",
      "|---|---|---|---|---|---|"]
for g, N, P, m, p, e in table:
    md.append(f"| {g} | {N} | {P} | {m:.3f} | {p:.3f} | {e:.2f} |")
open(os.path.join(R, "bin/doall/eta_table.md"), "w").write("\n".join(md) + "\n")

# ---- figure: eta vs N per P
W, H = 1240, 620
MX, MY, PW, PH, GAP = 45, 70, 560, 430, 40
esc = lambda s: str(s).replace("&", "&amp;").replace("<", "&lt;")
svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
       f'<rect width="{W}" height="{H}" fill="#fff"/>']
def text(x, y, s, size=11, anchor="start", color="#222", weight="normal"):
    svg.append(f'<text x="{x:.1f}" y="{y:.1f}" font-size="{size}" text-anchor="{anchor}" fill="{color}" '
               f'font-weight="{weight}" font-family="monospace">{esc(s)}</text>')
text(MX, 28, "DOALL loop: eta = predicted / measured, per thread count (1.00x = perfect)", 15, weight="bold")
text(MX, 46, "prediction = the online model's own algebra: L_total(P) + c_ns*N/P; measured = wall time per dispatch",
     10, color="#666")
import math
lo, hi = math.log10(5000), math.log10(1.5e6)
xs = lambda N: MX + 90 + (math.log10(N) - lo) / (hi - lo) * (PW - 120)
ys = lambda e: MY + 40 + (1.6 - min(1.6, max(0.0, e))) / 1.6 * (PH - 110)
for e in (0.0, 0.4, 0.8, 1.2, 1.6):
    svg.append(f'<line x1="{MX+90}" y1="{ys(e):.1f}" x2="{MX+PW-30}" y2="{ys(e):.1f}" stroke="#eee"/>')
    text(MX + 84, ys(e) + 3, f"{e:.1f}", 9, "end", "#666")
svg.append(f'<line x1="{MX+90}" y1="{ys(1.0):.1f}" x2="{MX+PW-30}" y2="{ys(1.0):.1f}" stroke="#c00" stroke-width="1.5" stroke-dasharray="5 4"/>')
text(MX + PW - 40, ys(1.0) - 5, "perfect (1.00x)", 10, "end", "#c00")
for N in (6008, 20000, 365493, 1000000):
    svg.append(f'<line x1="{xs(N):.1f}" y1="{MY+20}" x2="{xs(N):.1f}" y2="{MY+PH-70}" stroke="#f6f6f6"/>')
    text(xs(N), MY + PH - 54, f"{N}", 9, "middle", "#666")
col = {2: "#08c", 3: "#0a0", 4: "#c60"}
for P in (2, 3, 4):
    pts = sorted((N, e) for (g, N, p, m, pr, e) in table if p == P)
    d = " ".join(f"{xs(N):.1f},{ys(e):.1f}" for N, e in pts)
    svg.append(f'<polyline points="{d}" fill="none" stroke="{col[P]}" stroke-width="1.6"/>')
    for N, e in pts:
        svg.append(f'<circle cx="{xs(N):.1f}" cy="{ys(e):.1f}" r="5" fill="{col[P]}" fill-opacity="0.85"/>')
        text(xs(N) + 7, ys(e) + 4, f"{e:.2f}", 9, "start", "#333")
for k, P in enumerate((2, 3, 4)):
    svg.append(f'<circle cx="{MX+100+k*120}" cy="{MY+8}" r="4" fill="{col[P]}"/>')
    text(MX + 110 + k * 120, MY + 12, f"eta at P={P}", 10)
text(MX + 90, MY + PH - 30, "N (= |V| per dispatch, log scale)", 10)
open(os.path.join(R, "validation_doall_eta.svg"), "w").write("\n".join(svg) + "\n</svg>\n")
out = os.path.join(R, "validation_doall_eta.svg")
for cmd in (["inkscape", "--export-type=png", f"--export-filename={out[:-4]}.png", out],
            ["convert", out, out[:-4] + ".png"]):
    try:
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=120)
        break
    except Exception:
        continue
print("\nwrote bin/doall/eta_table.md and validation_doall_eta.svg/.png")
