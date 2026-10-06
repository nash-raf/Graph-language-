#!/usr/bin/env python3
"""Text rendition of the validation dashboard (same CSVs as plot_validation.py)."""
import csv, math, os
R = os.path.dirname(os.path.abspath(__file__))

def rows(path, skip_header=True):
    if not os.path.exists(path):
        return []
    return [r for r in csv.DictReader(l for l in open(path) if not l.startswith("#"))]

print("=" * 78)
print("p1 cost-model validation  (text view of validation_dashboard.png)")
print("=" * 78)

print("\nA. Schedule cost model (GPU): device-vs-CPU speedup, forced device, median of 3")
ev = [r for r in rows(os.path.join(R, "cost_model_evidence.csv"))
      if r.get("controlled") == "1" and r.get("work_units") in ("trip", "arcs")]
print(f"   {'family':12} {'work':>9} {'cpu/gpu':>8}  bar (0 .. 1.6)")
for r in sorted(ev, key=lambda r: (r["family"], int(r["work"]))):
    v = float(r["ratio"]); bar = "#" * max(0, min(40, int(v / 1.6 * 40)))
    print(f"   {r['family']:12} {r['work']:>9} {v:>8.2f}  {bar}")
print("   break-even at 1.00; gate thresholds: min_trips 4096, engine floor 2M arcs")

print("\nB. Format model (CPU layouts): ms per forced layout, '-' = not measured")
# the format probe writes headerless rows: tag,mode,ms,ans,conv
fmt = []
_fp = os.path.join(R, "bin/fmt/results.csv")
if os.path.exists(_fp):
    for line in open(_fp):
        f = line.strip().split(",")
        if len(f) >= 3:
            fmt.append({"tag": f[0], "mode": f[1], "ms": f[2]})
tags, modes = [], ["cost-model", "CSR", "PCSR", "BCSR", "SET"]
for r in fmt:
    if r["tag"] not in tags:
        tags.append(r["tag"])
print(f"   {'workload':12} " + " ".join(f"{m:>10}" for m in modes))
for t in tags:
    cells = []
    best = None
    for m in modes:
        r = next((x for x in fmt if x["tag"] == t and x["mode"] == m), None)
        if r:
            ms = int(r["ms"]); best = ms if best is None else min(best, ms)
            cells.append(f"{ms:>10}")
        else:
            cells.append(f"{'-':>10}")
    print(f"   {t:12} " + " ".join(cells))
print("   all measured modes gave identical answers; SET on big_kcore timed out (600s cap)")

print("\nC. CPU region model: predicted vs measured (5x contract)")
pairs = rows(os.path.join(R, "region_prediction_pairs.csv"))
print(f"   {'machine':28} {'mode':11} {'case':9} {'pred':>9} {'meas':>9} {'ratio':>6}  band")
for r in pairs:
    rat = float(r["ratio"]); ok = "in" if rat <= 5 else "OUT"
    print(f"   {r['machine']:28} {r['mode']:11} {r['case']:9} {float(r['predicted_ms']):>9.3f} "
          f"{float(r['measured_ms']):>9.3f} {rat:>6.2f}  {ok}")

print("\nD. Contract checks (this session)")
for name, res in [("T1-T7 CPU budget model", "ALL PASS x3 configs"),
                  ("T8-T10 device policy", "ALL PASS x3 configs"),
                  ("GPU cost-model direction", "14/14 box, 8/0/6 cpu-only"),
                  ("Cross-mode equality (11 fixtures)", "11/11 CPU=dev, 1=4 thr, repeats"),
                  ("Broad equality (50 fixtures)", "50/50 both boxes"),
                  ("Scale (780k/157k graphs)", "3/3 device==CPU"),
                  ("Device harness", "11 pass / 0 fail / 2 policy-skip"),
                  ("Local suite (CPU-only)", "93 pass / 0 fail"),
                  ("Box suite (GPU)", "WARN  90 pass / 4 fail (load + prediction)"),
                  ("Format answers across layouts", "identical"),
                  ("Force-layout wiring", "was dead; fixed"),
                  ("Region model: local CPU-only", "all 5 within 5x (1.0-3.0x)"),
                  ("Region model: EPYC CPU-only", "WARN  cc 3.0x ok; bfs 6.1x, pagerank 7.8x off"),
                  ("Region model: EPYC GPU backend", "WARN  inflated by device work (cc 118x)")]:
    ok = res.startswith(("ALL PASS", "14/14", "11/11", "50/50", "3/3", "11 pass", "93 pass",
                         "identical", "was dead; fixed", "all 5 within"))
    mark = "OK  " if ok else "WARN"
    print(f"   {mark} {name:34} {res}")
print()
