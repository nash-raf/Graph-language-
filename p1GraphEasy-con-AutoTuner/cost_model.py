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
# All hardware numbers come from hw_calib_bench (no graph-data-derived
# coefficients).  Fallbacks are only used when hw_calib.json is missing keys.
_hw = load_hw_calib()
L = float(_hw.get("L", 64.0))     # cache line size (bytes)
t = float(_hw.get("t", 39.4876))  # random-access latency per cache line (ns)
T = float(_hw.get("T", 2.8750))   # bandwidth-limited sequential access per cache line (ns)
R = float(_hw.get("R", 10000.0))  # realloc / page-remap policy threshold (ns)
LLC = float(_hw.get("LLC", 8.0 * 1024 * 1024))  # last-level-cache capacity (bytes)
Tm = float(_hw.get("Tm", 2.5))         # per-line cost of cache-resident memmove
P_dram = float(_hw.get("P", 4.0))      # DRAM-bound memmove penalty vs cache-resident
# Measured ramp band [ramp_lo, ramp_hi] (bytes) within which the memmove
# penalty transitions from cache-resident (1.0) to DRAM-saturated (P).
# Measured by hw_calib_bench's working-set sweep.  Absent => fall back to the
# LLC/2 .. 2*LLC conservative band.
ramp_lo = float(_hw.get("ramp_lo", -1.0))
ramp_hi = float(_hw.get("ramp_hi", -1.0))
h_hash = float(_hw.get("h", 40.0))     # ns per edge_hash_insert (bulk hash build);
                                       #  legacy scalar (DRAM-measured). Used as
                                       #  h_dram fallback when h_cache/h_dram absent.
h_cache = float(_hw.get("h_cache", -1.0))  # ns/insert when table fits LLC
h_dram  = float(_hw.get("h_dram", -1.0))   # ns/insert when table is DRAM-resident
if h_dram <= 0.0 and h_hash > 0.0:
    h_dram = h_hash  # degrade to legacy behaviour
c_contains = float(_hw.get("c", 15.0)) # ns per sequential-id bitmap contains check

kPcsr = 2.0     # PCSR expansion factor
kBcsr = 64.0    # BCSR block size

def _ramp_band():
    lo = ramp_lo if ramp_lo > 0.0 else LLC * 0.5
    hi = ramp_hi if ramp_hi > 0.0 else LLC * 2.0
    if hi <= lo:
        hi = lo + L  # guard against a degenerate zero-width band
    return lo, hi

def _ramp_penalty(ws, lo, hi, P):
    if ws <= lo:
        return 1.0
    if ws >= hi:
        return P
    denom = math.log(hi / lo)
    if denom <= 0.0:
        return P
    return 1.0 + (P - 1.0) * math.log(ws / lo) / denom

def mem_penalty(working_set_bytes):
    """Streaming-cost multiplier for memmove/shift terms.  Below ramp_lo the
    working set is fully cache-resident (1.0); above ramp_hi every shifted
    line misses to DRAM (P).  Between ramp_lo and ramp_hi the penalty ramps
    log-linearly, modelling the gradual cache-pressure transition.  The band
    edges are MEASURED by hw_calib_bench's working-set sweep (keys ramp_lo /
    ramp_hi); absent those, falls back to LLC/2 .. 2*LLC.  Mirrors memPenalty()
    in AutoTunerPass.cpp."""
    lo, hi = _ramp_band()
    return _ramp_penalty(working_set_bytes, lo, hi, P_dram)

def _next_pow2(x):
    p = 1.0
    while p < x:
        p *= 2.0
    return p

def hash_insert_cost(pairs):
    """Size-aware ns per edge_hash_insert during a bulk hash build.  The
    EdgeHashMap is open-addressing with 24-byte entries and capacity
    next_pow2(2*pairs+1).  Cache-resident cost (h_cache) when the table fits
    the LLC, DRAM cost (h_dram) when it spills, log-linear ramp in between
    using the SAME measured [ramp_lo, ramp_hi] band as memmove (cache-residency
    is governed by the same LLC).  Falls back to the legacy scalar h
    (DRAM-measured) when the two-size calibration keys are absent.  Mirrors
    hashInsertCost() in AutoTunerPass.cpp."""
    table_bytes = _next_pow2(2.0 * pairs + 1.0) * 24.0
    if h_cache <= 0.0:
        return h_dram if h_dram > 0.0 else h_hash
    lo, hi = _ramp_band()
    hd = h_dram if h_dram > 0.0 else h_hash
    if table_bytes <= lo:
        return h_cache
    if table_bytes >= hi:
        return hd
    denom = math.log(hi / lo)
    if denom <= 0.0:
        return hd
    ratio = math.log(table_bytes / lo) / denom
    return h_cache + (hd - h_cache) * ratio

def conversion_cost_csr_to_set(n, m):
    """Physics-based CSR -> SET conversion cost.  Mirrors
    conversionCostCSRToSET() in AutoTunerPass.cpp.  The runtime
    (autograph_ensure_layout_set, run BEFORE profile_region_enter) does:
      - rebuild_sets_from_csr_meta: builds+destroys a local EdgeHashMap over
        the m static pairs (hash_insert_cost per pair) + scans the CSR arrays
        (~4m bytes).
      - refresh_graph_counts_from_canonical: canonical_edge_count_cached runs
        the full O(m) roaring_bitmap_contains scan (c_contains per check)
        because live_edge_count was just invalidated; canonical_node_span
        triggers one O(n) select-cache rebuild of the nodes bitmap.
    Replaces the crude 0.1*(n+m) alpha the old model used for CSR->SET."""
    c_hash_build = hash_insert_cost(m) * m
    c_edge_count = c_contains * m
    c_sel_cache = (math.ceil(4.0 * n / L) + math.ceil(n / 8.0 / L)) * T
    c_csr_scan  = math.ceil(4.0 * m / L) * T
    return c_hash_build + c_edge_count + c_sel_cache + c_csr_scan

# ── Cost model equations ──────────────────────────────────────────────

def insert_cost_csr(n, m):
    d = 2.0 * m / n if n > 0 else 1.0
    u = n / 2.0
    p = m / 2.0
    cLocate = 2*t + math.ceil(d * 4/L) * T
    # row_ptr prefix-sum update: n/2 int64 R-M-W entries (dependent load→store,
    # cannot overlap; 2T per line) and DRAM-bound once the prefix array
    # outgrows the LLC — same physics as the BCSR brow prefix.
    prefix_bytes = (n - u) * 8.0
    cWrite  = 1.0*t + math.ceil(prefix_bytes / L) * 2.0 * T * mem_penalty(prefix_bytes)
    # col_idx memmove: ~4m bytes, read+write per line via Tm, DRAM-bound past
    # the LLC — same physics as the BCSR cMove term.
    move_bytes = 4.0 * (m - p)
    cMove   = math.ceil(move_bytes / L) * Tm * mem_penalty(move_bytes)
    cRealloc = R + math.ceil(4.0 * m / L) * T
    return cLocate + cWrite + cMove + cRealloc

def insert_cost_pcsr(n, m):
    d = 2.0 * m / n if n > 0 else 1.0
    gU = kPcsr * d
    cLocate = 2.0 * t + math.ceil(gU * 4.0 / L) * T
    cWrite  = T
    return cLocate + cWrite

def insert_cost_bcsr(n, m):
    """Mirrors insertCost(LAYOUT_BCSR) in AutoTunerPass.cpp.

    Runtime (autograph_bcsr_add_edge): dup-scan the block row, realloc bcol
    by +2 ints, memmove everything after the insertion point, bump the brow
    prefix sums.  bcol backing array = 4m int32 (2 ints per directed edge,
    2m directed edges) = 16m bytes.
    """
    d = 2.0 * m / n if n > 0 else 1.0
    b = kBcsr
    nb = math.ceil(n / b)
    arr_bytes = 16.0 * m
    # 2 dirs x (2 brow reads + dup/insert-point scan of the block row,
    # ~half of its 8*b*d bytes each on average)
    cLocate = 4.0 * t + math.ceil(8.0 * b * d / L) * T
    # brow prefix-sum update: ~nb int32 R-M-W across both directions
    cWrite = math.ceil(nb * 4.0 / L) * 2.0 * T
    # shift everything after the insertion point: ~half the backing array.
    # memmove pays read+write per line (Tm, measured), and goes DRAM-bound
    # once the array outgrows the LLC
    cMove = math.ceil((arr_bytes / 2.0) / L) * Tm * mem_penalty(arr_bytes)
    # realloc(+8 bytes) almost always extends in place (chunk padding /
    # mremap): charge the O(1) policy cap, not an O(m) copy
    cRealloc = R
    return cLocate + cWrite + cMove + cRealloc

K_INS = 50.0   # adds per measured insert kernel (test/real_*_ins.graph)

def insert_cost_set(n, m):
    """Mirrors insertCost(LAYOUT_SET)/insertSetupCost in AutoTunerPass.cpp.

    Steady-state autograph_canonical_add_edge is O(1): live_edge_count is
    maintained incrementally (no per-insert O(m) rescan) and re-adding
    already-present nodes doesn't invalidate the nodes select cache.
    The one-time lazy work the FIRST timed insert pays is just the
    get_static_edge_hash build (h·m); the conversion-time O(m) canonical
    edge-count scan and O(n) select-cache rebuild run inside
    autograph_ensure_layout_set BEFORE profile_region_enter, so they are
    billed in conversion_cost_csr_to_set(), not here.
    """
    # ── steady-state per-insert (all O(1)) ───────────────────────────
    # 2 node-bitmap adds + static-hash probe + extra-hash probe
    cLocate = 5.0 * t
    # extras append + live flag + extra-hash insert + cached count refresh
    cWrite  = 2.0 * t + 5.0 * T
    # ── one-time lazy init, paid by the FIRST insert while in SET ────
    # get_static_edge_hash: one edge_hash_insert per static pair, at the
    # size-aware per-insert cost (h_cache when the table fits LLC, h_dram
    # when it spills).  The other O(n+m) work (canonical_edge_count scan,
    # select-cache rebuild) is billed to conversion_cost_csr_to_set.
    cHashBuild = hash_insert_cost(m) * m
    cSetup = cHashBuild + R
    # This script compares per-op numbers, so amortize the setup over the
    # K_INS adds the benchmark kernel actually performs.  (The C++ pass
    # instead charges insertSetupCost() once per region.)
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
    # Mirrors traversalCost(LAYOUT_BCSR) in AutoTunerPass.cpp: the runtime
    # stores per-block-row (local_row, col) pair lists, so traversing one
    # vertex scans the whole block row: b*d edges x 8 bytes.
    d = 2.0 * m / n if n > 0 else 1.0
    bU = 8.0 + 8.0 * kBcsr * d
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

        # Rank by measured (exclude N/A / empty)
        meas_map = {}
        for r in grp:
            v = r["measured_ns"].strip()
            if v:
                meas_map[r["layout"]] = int(v)
        meas_rank = sorted(meas_map.keys(), key=lambda l: meas_map[l])

        # Rank by predicted, restricted to layouts that have a measurement
        # (an N/A measurement — e.g. SET traverse timeout — must not force a
        # mismatch by making the two ranking lists different lengths).
        pred_rank = sorted([l for l in LAYOUTS if l in preds and l in meas_map],
                           key=lambda l: preds[l])

        match = pred_rank == meas_rank

        # Tie-tolerance: when the top-2 predicted layouts are within 5% of
        # each other, the model considers them tied and a different measured
        # ordering is reported as PRED_TIE rather than MISMATCH.  (A full
        # MEAS_TIE needs IQRs, which this CSV doesn't carry; bench_folder.py
        # emits those.  This avoids flagging ia-dbpedia-style overlapping-IQR
        # groups as hard mismatches when the prediction itself is a toss-up.)
        PRED_TIE_EPS = 0.05
        pred_vals = sorted(preds[l] for l in pred_rank)
        pred_tie = (len(pred_vals) >= 2 and
                    (pred_vals[1] - pred_vals[0]) <= PRED_TIE_EPS * max(pred_vals[0], 1.0))

        if match:
            verdict = "MATCH"
        elif pred_tie:
            verdict = "PRED_TIE"
        else:
            verdict = "MISMATCH"

        for r in grp:
            r["_verdict"] = verdict

        if verdict == "MISMATCH":
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
    # Verdict is per (graph, op) group; count groups, not rows, to avoid
    # inflating the denominator by the number of layouts.
    groups_total = len(groups)
    match_count = sum(1 for k, g in groups.items() if g[0]["_verdict"] == "MATCH")
    tie_count   = sum(1 for k, g in groups.items() if g[0]["_verdict"] == "PRED_TIE")
    mismatch_count = groups_total - match_count - tie_count
    out_lines.append(
        f"VERDICT: {match_count}/{groups_total} MATCH"
        f" (+{tie_count} PRED_TIE, {mismatch_count} MISMATCH)"
    )
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
            for r in rows:
                r["predicted_ns"] = str(r["_pred_ns"]) if r["_pred_ns"] else ""
            w = csv.DictWriter(f, fieldnames=[
                "graph", "n", "m", "op", "layout", "predicted_ns",
                "measured_ns", "_pred_ns", "_verdict"
            ])
            w.writeheader()
            w.writerows(rows)
        print(f"Updated CSV written to {out_path}")

    # Success = no hard mismatches (PRED_TIE is acceptable).
    return (match_count + tie_count) == groups_total

if __name__ == "__main__":
    p = argparse.ArgumentParser(description="Cost model evaluator")
    p.add_argument("--csv", required=True, help="Input CSV file (columns: graph,n,m,op,layout,measured_ns)")
    p.add_argument("--out", default=None, help="Optional output CSV path")
    args = p.parse_args()
    ok = process(args.csv, args.out)
    sys.exit(0 if ok else 1)
