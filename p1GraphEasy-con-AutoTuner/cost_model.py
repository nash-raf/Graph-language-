#!/usr/bin/env python3
"""
Standalone cost model evaluator.
Modify the equations in `predicted_ns()` to experiment with different models.
Usage:
    python3 cost_model.py --csv <path_to_results.csv>
"""

import csv, math, sys, argparse, os, json
from pathlib import Path

def load_hw_calib():
    path = os.environ.get("AUTOTUNER_HW_CALIB", "")
    if not path:
        path = str(Path.home() / ".config" / "sgpl" / "hw_calib.json")
    if not path or not os.path.isfile(path):
        return {}
    try:
        with open(path) as f:
            return json.load(f)
    except Exception:
        return {}

# ── Calibration constants ─────────────────────────────────────────────
_hw = load_hw_calib()
L = float(_hw.get("L", 64.0))     # cache line size (bytes)
t = float(_hw.get("t", 39.4876))  # random-access latency per cache line (ns)
T = float(_hw.get("T", 2.8750))   # bandwidth-limited sequential access per cache line (ns)
R = float(_hw.get("R", 10000.0))  # realloc / page-remap policy threshold (ns)

kPcsr = 2.0     # PCSR expansion factor
kBcsr = 64.0    # BCSR block size

# ── Cost model equations ──────────────────────────────────────────────

def insert_cost_csr(n, m):
    d = 2.0 * m / n if n > 0 else 1.0
    u = n / 2.0
    p = m / 2.0
    cLocate = 2*t + math.ceil(d * 4/L) * T
    cWrite  = 1.0*t + math.ceil((n - u) * 8.0 / L) * T
    cMove   = math.ceil(4.0 * (m - p) / L) * T
    cRealloc = R + math.ceil(4.0 * m / L) * T
    # cRealloc = 2.0 * cReallocPerDir
    return cLocate + cWrite + cMove + cRealloc

def insert_cost_pcsr(n, m):
    d = 2.0 * m / n if n > 0 else 1.0
    gU = kPcsr * d
    cLocate = 2.0 * t + math.ceil(gU * 4.0 / L) * T
    cWrite  = T
    return cLocate + cWrite

def insert_cost_bcsr(n, m):
    d = 2.0 * m / n if n > 0 else 1.0
    b = kBcsr
    nb = math.ceil(n / b)
    d_BR = d / b
    blkIdx = n / (2.0 * b)
    B_blk = 4 + b*b/8
    m_u = m - blkIdx
    p_B = m
    cLocate = 4*t + 2*math.ceil(d * b * 4 / L) * T
    cWrite  = 4*t + 4*math.ceil(nb*2/L) * T
    cMove   = math.ceil(4*(2*m - p_B)/L) * T
    cRealloc = R + math.ceil(8.0 * m / L) * T
    # total_no_block_records = math.ceil(m/(b*b))
    # cRealloc = total_no_block_records + B_blk
    # cReallocPerDir = min(math.ceil(8.0 * m / L) * T, R)
    # freshProb = (1.0 - m / (n * (n - 1.0))) if n > 1 else 1.0
    # cRealloc = 2.0 * freshProb * cReallocPerDir
    return cLocate + (cWrite + cMove) + cRealloc

# def insert_cost_set(n, m):
#     #cHash = math.ceil(m * 40.0 / L) * T
#     #cBase = 5.0 * t
#     # cArray = 0.0
#     # if n <= 4096.0:
#     #     cShift = math.ceil((n / 2.0) * 2.0 / L) * T
#     #     cArray = 2.0 * (math.ceil(math.log2(n) * 2.0 / L) * T + cShift)
#     # return cHash + cBase + cArray
#     cLocate = 2*t
#     cWrite = 5*T
#     I_g = 0
#     if n > 4096:
#         I_g = 1
#     cRealloc = I_g * (math.ceil(8*m/L) + math.ceil(8*m/L) + 2* math.ceil(24 * m/L)) * T + I_g * 2 * math.ceil(8192/L) * T + R
#     return cLocate + cWrite + cRealloc

K_INS = 50.0   # adds per measured insert kernel (test/real_*_ins.graph)
def insert_cost_set(n, m):
    pairs = m / 2.0                      # static undirected pair count
    # ── steady-state per-insert (all O(1)) ───────────────────────────
    # 2 node-bitmap adds + static-hash probe + extra-hash probe
    cLocate = 5.0 * t
    # extras append + live flag + extra-hash insert + cached count refresh
    cWrite  = 2.0 * t + 5.0 * T
    # ── one-time lazy init, paid by the FIRST insert while in SET ────
    # get_static_edge_hash: one edge_hash_insert per static pair
    cHashBuild = pairs * 1.5 * t
    # first canonical_edge_count: one roaring_bitmap_contains per pair id
    # (sequential ids -> mostly container-local, cheaper than a full miss)
    cFirstCount = pairs * 1.0 * t
    # one select-cache rebuild for the nodes bitmap: write 4n bytes,
    # read the bitmap containers (~n/8 bytes)
    cSelCache = (math.ceil(4.0 * n / L) + math.ceil(n / 8.0 / L)) * T
    # extras array doubling is amortized; charge one realloc event
    cSetup = cHashBuild + cFirstCount + cSelCache + R
    # cost_model.py compares per-op numbers, so amortize the setup over
    # the k inserts the kernel actually performs
    return cLocate + cWrite + cSetup / K_INS

def traverse_cost_csr(n, m):
    d = 2.0 * m / n if n > 0 else 1.0
    bU = 16.0 + 4.0 * d
    return n * (2.0 * t + math.ceil(bU / L) * T)

def traverse_cost_pcsr(n, m):
    d = 2.0 * m / n if n > 0 else 1.0
    bU = 16.0 + 4.0 * d * kPcsr
    return n * (2.0 * t + math.ceil(bU / L) * T)

def traverse_cost_bcsr(n, m):
    d = 2.0 * m / n if n > 0 else 1.0
    bU = 8.0 + 256 * d
    return n * (2.0 * t + math.ceil(bU / L) * T)

def traverse_cost_set(n, m):
    bU = 8 * m
    return n * ( math.ceil(bU / L) * T)


# ── Dispatch ─────────────────────────────────────────────────────────

LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]

def predicted_ns(layout, op_kind, n, m):
    if op_kind == "Insert":
        if layout == "CSR":  return insert_cost_csr(n, m)
        if layout == "PCSR": return insert_cost_pcsr(n, m)
        if layout == "BCSR": return insert_cost_bcsr(n, m)
        if layout == "SET":  return insert_cost_set(n, m)
    elif op_kind == "Traverse":
        if layout == "CSR":  return traverse_cost_csr(n, m)
        if layout == "PCSR": return traverse_cost_pcsr(n, m)
        if layout == "BCSR": return traverse_cost_bcsr(n, m)
        if layout == "SET":  return traverse_cost_set(n, m)
    return float("inf")

# ── CSV processing ────────────────────────────────────────────────────

def _col(mapping, *names):
    for n in names:
        if n in mapping:
            return mapping[n]
    return None

def _require(cmap, key, csv_path, *names):
    v = _col(cmap, *names)
    if v is not None:
        return v
    print(f"error: {csv_path} is missing a column for '{key}'.\n"
          f"       Expected one of: {names}\n"
          f"       Found columns: {list(cmap.values())}",
          file=sys.stderr)
    sys.exit(1)

def process(csv_path, out_path=None):
    with open(csv_path, newline="") as f:
        reader = csv.DictReader(f)
        raw = list(reader)

    cols = raw[0].keys()
    # Map flexible column names to canonical names
    cmap = {}
    for c in cols:
        cl = c.lower().replace(" ", "_")
        if cl in ("graph", "g"):           cmap["graph"] = c
        elif cl in ("n", "n_vertices",
                     "num_vertices", "verts"):  cmap["n"] = c
        elif cl in ("m", "m_directed",
                     "m_undirected",
                     "edges", "nedges",
                     "num_edges", "edge_count"): cmap["m"] = c
        elif cl in ("op", "operation",
                     "op_kind"):                cmap["op"] = c
        elif cl in ("layout", "lay"):          cmap["layout"] = c
        elif cl in ("measured_ns",
                     "measured_kernel_ns",
                     "kernel_ns"):             cmap["measured_ns"] = c
        elif cl in ("predicted_ns",):          cmap["predicted_ns"] = c
        elif cl in ("predicted_best",):        cmap["predicted_best"] = c
        elif cl in ("measured_best",):         cmap["measured_best"] = c
        elif cl in ("verdict",):               cmap["verdict"] = c

    graph_col = _require(cmap, "graph", csv_path, "graph")
    n_col     = _require(cmap, "n",     csv_path, "n")
    m_col     = _require(cmap, "m",     csv_path, "m")
    op_col    = _require(cmap, "op",    csv_path, "op")
    lay_col   = _require(cmap, "layout", csv_path, "layout")
    meas_col  = _col(cmap, "measured_ns")
    pred_col  = _col(cmap, "predicted_ns")

    rows = []
    for r in raw:
        rows.append({
            "graph":        r[graph_col],
            "n":            int(r[n_col]),
            "m":            int(r[m_col]),
            "op":           r[op_col],
            "layout":       r[lay_col],
            "measured_ns":  r[meas_col].strip() if meas_col and r[meas_col].strip() else "",
            "_pred_ns":     0.0,
        })

    groups = {}
    for r in rows:
        key = (r["graph"], r["op"])
        groups.setdefault(key, []).append(r)

    mismatches = []
    for key in sorted(groups):
        grp = groups[key]
        g, op = key
        n = grp[0]["n"]
        m = grp[0]["m"]

        # Compute predicted_ns for each layout in group
        preds = {}
        for r in grp:
            lay = r["layout"]
            preds[lay] = predicted_ns(lay, op, n, m)
            r["_pred_ns"] = preds[lay]

        # Rank by predicted
        pred_rank = sorted([l for l in LAYOUTS if l in preds],
                           key=lambda l: preds[l])

        # Rank by measured (exclude N/A / empty)
        meas_map = {}
        for r in grp:
            v = r["measured_ns"].strip()
            if v:
                meas_map[r["layout"]] = int(v)
        meas_rank = sorted(meas_map.keys(), key=lambda l: meas_map[l])

        match = pred_rank == meas_rank

        for r in grp:
            r["_verdict"] = "MATCH" if match else "MISMATCH"

        if not match:
            mismatches.append((g, op, pred_rank, meas_rank, preds, meas_map))

    # ── Output ────────────────────────────────────────────────────
    out_lines = []
    header = f"{'graph':30s} {'op':10s} {'layout':6s}  {'pred_ns':>12s}  {'meas_ns':>10s}  verdict"
    out_lines.append(header)
    out_lines.append("-" * len(header))
    for r in rows:
        mn = r["measured_ns"].strip() if r["measured_ns"].strip() else "N/A"
        out_lines.append(
            f"{r['graph']:30s} {r['op']:10s} {r['layout']:6s}  "
            f"{r['_pred_ns']:>12.1f}  {mn:>10s}  "
            f"{r['_verdict']}"
        )

    out_lines.append("")
    total = len(rows)
    match_count = sum(1 for r in rows if r['_verdict'] == 'MATCH')
    out_lines.append(f"VERDICT: {match_count}/{total} {'MATCH' if match_count == total else 'MISMATCH'}")
    out_lines.append("")

    if mismatches:
        out_lines.append("MISMATCH DETAILS (full ranking):")
        out_lines.append("-" * 100)
        for g, op, pr, mr, preds_dict, meas_dict in mismatches:
            pred_str = " > ".join(f"{l}({preds_dict[l]:.0f})" for l in pr)
            meas_str = " > ".join(f"{l}({meas_dict[l]})" for l in mr)
            out_lines.append(f"  {g:30s} {op:10s}  pred: {pred_str}")
            out_lines.append(f"  {'':30s} {'':10s}  meas: {meas_str}")
        out_lines.append("")

    text = "\n".join(out_lines)
    print(text)

    if out_path:
        out_path = os.path.abspath(out_path)
        with open(out_path, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=[
                "graph", "n", "m", "op", "layout", "predicted_ns",
                "measured_ns", "_pred_ns", "_verdict"
            ])
            w.writeheader()
            w.writerows(rows)
        print(f"Updated CSV written to {out_path}")

    return match_count == total

if __name__ == "__main__":
    p = argparse.ArgumentParser(description="Cost model evaluator")
    p.add_argument("--csv", required=True, help="Input CSV file (columns: graph,n,m,op,layout,measured_ns)")
    p.add_argument("--out", default=None, help="Optional output CSV path")
    args = p.parse_args()
    ok = process(args.csv, args.out)
    sys.exit(0 if ok else 1)
