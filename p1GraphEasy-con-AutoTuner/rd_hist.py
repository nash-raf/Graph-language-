"""rd_hist.py — reuse-distance histograms for the BCSR add trace (Step 2).

Authoritative definition (sole ground truth):
    RD(line l at reference tau) = # distinct cache lines referenced after
    the previous occurrence of l and before the current occurrence.
    First references get RD = INF (compulsory/cold miss bin).

Two implementations:
  exact()     : materialized ordered line-reference stream + Fenwick-tree
                stack distance (counts lines whose LAST reference lies in
                the interval -> exact distinct count, O(log N) per ref).
  candidate() : closed form exploiting the per-op contiguous touched ranges
                [a_t, b_t] of bcol (and [a_br_t, b_br] of brow).  Window
                unions are contiguous intervals; per-line RD is derived in
                O(1) with prefix/suffix mins and sorted threshold arrays.

Model convention (documented in trace_gen.py, applied IDENTICALLY here):
  - every cache line touched by an op is referenced exactly once (intra-op
    read+write collapse; second reference would be RD=0 = hit for C >= 1);
  - within an op: bcol lines ascending, then brow lines ascending, then
    the struct line.
  - line(i) = floor((base + 4*i)/64), base mod 64 probed from glibc.

Per-line closed form.  Let p = t_prev(l) = the last op before t whose
touched range covers line l (or -1 if none: INF bin).

bcol line l at op t:
  p == t-1  (adjacent): between the two references lie
      op t-1 after l: bcol (l, b[t-1]] + brow [a_br[t-1], b_br] + struct(1)
      op t   before l: bcol [a_t, l)
    RD = (b[t-1] - a_t) + (b_br - a_br[t-1] + 1) + 1
  p <  t-1  (window w = p+1..t-1, non-empty; bcol union of w is the
    contiguous interval [min_a_w, b[t-1]], brow union [min_a_br_w, b_br]):
    window counts + uncovered parts of op p's suffix and op t's prefix:
      window: (b[t-1]-min_a_w+1) + (b_br-min_a_br_w+1) + struct(1)
      prefix in t not in window: [a_t, min(min_a_w, l))  and  [b[t-1]+1, l)
      suffix in p not in window: (l, min(min_a_w-1, b[p])]  (bcol)
                                 [a_br[p], min_a_br_w)        (brow)

brow line mu at op t (coverage of mu by op s  <=>  blk[s] <= 16*mu-o_br+14):
  p == t-1: RD = (b_br - a_br_t) + (b_t - a_t + 1) + 1
      (op t-1 suffix: brow (mu, b_br] + struct(1); op t prefix:
       bcol [a_t, b_t] + brow [a_br_t, mu))
  p <  t-1: window counts + uncovered parts:
      window: (b[t-1]-min_a_w+1) + (b_br-min_a_br_w+1) + 1
      prefix in t not in window: bcol [a_t, min(min_a_w, b_t+1)) and
                                 (b[t-1], b_t]; brow [a_br_t, min(min_a_br_w, mu))
      suffix in p not in window: brow (mu, min(min_a_br_w-1, b_br)]

struct line at op t (t >= 1): RD = bcol_op + brow_op (nothing between the
two struct references except op t's own bcol+brow; struct is op-last).
"""
import json
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import trace_gen as tg

# machine geometry (i5-1235U, from sysfs; ways/sets hidden -> constants)
L2_LINES = 1310720 // 64      # 20480
L3_LINES = 12582912 // 64     # 196608


# ── Generic RD → hit-ratio API (single source of truth) ────────────────────
#
# The final class × residency cost model is
#     T = sum_s N_s [ h_{2,s} r_{s,L2} + (h_{3,s}-h_{2,s}) r_{s,L3}
#                     + (1-h_{3,s}) r_{s,DRAM} ].
# These helpers map any reuse distribution (an RD histogram, or per-tier line
# counts) to per-class (N, h2, h3).  The tier thresholds and the INF-bin
# handling live HERE and only here: consumers (access_class_cost, the trace
# path, the analytic estimator) must not re-implement them.


def rd_to_tier(rd, inf_mask, l2_lines=L2_LINES, l3_lines=L3_LINES):
    """0=L2, 1=L3, 2=DRAM per reuse distance.  First-touch/INF refs are
    compulsory cold misses -> DRAM.  Single source of truth for the tier
    thresholds (equivalent to the final cost model's h(C)=#(d<C)/N bins)."""
    tier = np.zeros_like(np.asarray(rd, dtype=np.int64), dtype=np.int64)
    tier = np.where(rd >= l3_lines, 2, tier)
    tier = np.where((rd >= l2_lines) & (rd < l3_lines), 1, tier)
    tier = np.where(np.asarray(inf_mask, dtype=bool), 2, tier)
    return tier


def hit_ratios_from_tiers(l2, l3, dram):
    """(h2, h3) from per-tier line counts [l2, l3, dram]:
        h2 = l2 / N, h3 = (l2 + l3) / N,  N = l2 + l3 + dram.
    INF/DRAM refs are misses (counted in N but not in the numerator)."""
    n = l2 + l3 + dram
    if n <= 0:
        return 0.0, 0.0
    return l2 / n, (l2 + l3) / n


def hit_ratios(hist, inf_cnt, total, l2_lines=L2_LINES, l3_lines=L3_LINES):
    """(h2, h3) over an RD histogram {rd: count}.
        h(C) = #{d < C} / total
    with first-touch/INF refs excluded from the hit count (compulsory misses
    that only appear in the denominator)."""
    if total <= 0:
        return 0.0, 0.0

    def h_at(c):
        return sum(cnt for d, cnt in hist.items() if d < c) / total

    return h_at(l2_lines), h_at(l3_lines)


def class_tiers(hist_by_class, inf_by_class, total_by_class,
                l2_lines=L2_LINES, l3_lines=L3_LINES):
    """Per-class (N, h2, h3) from per-class RD histograms.  The generic
    RD → hit-ratio entry point: any reuse distribution in, per-class
    (N, h2, h3) out, so it can feed the cost equation for any
    representation (BCSR, CSR, ...)."""
    out = {}
    for cls in hist_by_class:
        n = total_by_class.get(cls, 0)
        h2, h3 = hit_ratios(hist_by_class[cls], inf_by_class.get(cls, 0),
                            n, l2_lines, l3_lines)
        out[cls] = (float(n), h2, h3)
    return out


def per_op_bcol_range(ops, o):
    """bcol line ranges [a_t, b_t] (inclusive) for each op."""
    a = np.array([(op["start"] + o) // 16 for op in ops], dtype=np.int64)
    b = np.array([(op["T"] + 1 + o) // 16 for op in ops], dtype=np.int64)
    return a, b


def iter_op_line_rd(trace, o, o_br):
    """Yield per op: (t, ell, rd, inf_mask, mu, rd2, inf2, s_rd).

    Per-line reuse distances for the op's bcol lines (ell/rd), brow lines
    (mu/rd2) and the struct line (s_rd), computed with the same closed form
    as candidate() — the single source of truth for RD.
    """
    ops = trace["ops"]
    t_ops = len(ops)
    T0 = trace["T0"]
    a, b = per_op_bcol_range(ops, o)
    nb = trace["nb"]
    b_br = (nb - 1 + o_br) // 16          # brow max line (inclusive)
    blk = np.array([op["blk"] for op in ops], dtype=np.int64)
    a_br = (blk + 1 + o_br) // 16

    for t in range(t_ops):
        at, bt = int(a[t]), int(b[t])
        # ---- bcol lines of this op ----
        ell = np.arange(at, bt + 1, dtype=np.int64)
        n = ell.shape[0]
        # first op with b_s >= ell  (T_s = T0 + 2s monotone in s)
        first_t = np.maximum(
            0, (16 * ell - o - 1 - T0 + 1) // 2)      # ceil((...)/2)
        if t > 0:
            # t_prev_raw = max{s < t : a_s <= ell}
            sa = np.sort(a[:t])
            sidx = np.argsort(a[:t], kind="stable")
            # prefix max over sorted order: max original index among the
            # first j+1 smallest a-values
            suf = np.maximum.accumulate(sidx)
            pos = np.searchsorted(sa, ell, side="right")
            tprev_raw = np.where(pos > 0, suf[pos - 1], -1)
        else:
            tprev_raw = np.full(n, -1, dtype=np.int64)
        tprev = np.where(tprev_raw >= first_t, tprev_raw, -1)
        inf_mask = tprev < 0
        adj = (tprev == t - 1) & ~inf_mask            # empty window
        far = (tprev < t - 1) & ~inf_mask             # non-empty window

        rd = np.empty(n, dtype=np.int64)
        if np.any(adj):
            a_br_prev = int(a_br[t - 1])
            rd[adj] = (int(b[t - 1]) - at) + (b_br - a_br_prev + 1) + 1
        if np.any(far):
            p = tprev[far]
            suf_min_a = np.minimum.accumulate(a[:t][::-1])[::-1]
            suf_min_br = np.minimum.accumulate(a_br[:t][::-1])[::-1]
            min_a_w = suf_min_a[p + 1]        # min a_s over window
            min_a_br_w = suf_min_br[p + 1]    # min a_br_s over window
            b_p = b[p]
            a_br_p = a_br[p]
            rd[far] = ((int(b[t - 1]) - min_a_w + 1)          # window bcol
                       + (b_br - min_a_br_w + 1)              # window brow
                       + 1                                    # window struct
                       + np.maximum(0, ell[far] - int(b[t - 1]) - 1)
                       + np.maximum(0, np.minimum(min_a_w, ell[far]) - at)
                       + np.maximum(0, np.minimum(min_a_w - 1, b_p)
                                    - ell[far])
                       + np.maximum(0, min_a_br_w - a_br_p))

        # ---- brow lines of this op ----
        a_br_t = int(a_br[t])
        mu = np.arange(a_br_t, b_br + 1, dtype=np.int64)
        m = mu.shape[0]
        bcol_op = bt - at + 1
        if t > 0:
            sblk = np.sort(blk[:t])
            sidx2 = np.argsort(blk[:t], kind="stable")
            suf2 = np.maximum.accumulate(sidx2)
            thr = 16 * mu - o_br + 14        # coverage: blk_s <= thr
            pos2 = np.searchsorted(sblk, thr, side="right")
            tp2 = np.where(pos2 > 0, suf2[pos2 - 1], -1)
        else:
            tp2 = np.full(m, -1, dtype=np.int64)
        inf2 = tp2 < 0
        adj2 = (tp2 == t - 1) & ~inf2
        far2 = (tp2 < t - 1) & ~inf2
        rd2 = np.empty(m, dtype=np.int64)
        if np.any(adj2):
            # suffix in t-1: brow after mu + struct; prefix in t: bcol + brow
            rd2[adj2] = (b_br - a_br_t) + bcol_op + 1
        if np.any(far2):
            p2 = tp2[far2]
            suf_min_a2 = np.minimum.accumulate(a[:t][::-1])[::-1]
            suf_min_br2 = np.minimum.accumulate(a_br[:t][::-1])[::-1]
            min_a_w2 = suf_min_a2[p2 + 1]
            min_a_br_w2 = suf_min_br2[p2 + 1]
            rd2[far2] = ((int(b[t - 1]) - min_a_w2 + 1)      # window bcol
                         + (b_br - min_a_br_w2 + 1)          # window brow
                         + 1                                 # window struct
                         + np.maximum(0, bt - int(b[t - 1]))
                         + np.maximum(0, np.minimum(min_a_w2, bt + 1) - at)
                         + np.maximum(0, np.minimum(min_a_br_w2, mu[far2])
                                      - a_br_t)
                         + np.maximum(0, min_a_br_w2 - mu[far2] - 1))

        # ---- struct line ----
        s_rd = None if t == 0 else bcol_op + (b_br - a_br_t + 1)
        yield t, ell, rd, inf_mask, mu, rd2, inf2, s_rd


def candidate(trace, o, o_br):
    """Closed-form global RD histogram (INF bin + finite bins)."""
    hist = {}
    inf_cnt = 0
    for t, ell, rd, inf_mask, mu, rd2, inf2, s_rd in iter_op_line_rd(
            trace, o, o_br):
        inf_cnt += int(np.count_nonzero(inf_mask))
        for rv in rd[~inf_mask]:
            hist[int(rv)] = hist.get(int(rv), 0) + 1
        inf_cnt += int(np.count_nonzero(inf2))
        for rv in rd2[~inf2]:
            hist[int(rv)] = hist.get(int(rv), 0) + 1
        if s_rd is None:
            inf_cnt += 1
        else:
            hist[s_rd] = hist.get(s_rd, 0) + 1
    total = inf_cnt + sum(hist.values())
    return hist, inf_cnt, total


def exact(trace, o, o_br):
    """Exact RD over the materialized ordered reference stream (Fenwick)."""
    ops = trace["ops"]
    nb = trace["nb"]
    a, b = per_op_bcol_range(ops, o)
    b_br = (nb - 1 + o_br) // 16
    blk = [op["blk"] for op in ops]

    bcol_max = int(b.max()) + 1
    struct_id = bcol_max + b_br + 1
    stream = []
    for t in range(len(ops)):
        stream.extend(range(int(a[t]), int(b[t]) + 1))
        a_br_t = (blk[t] + 1 + o_br) // 16
        # brow lines live above the bcol id space (no id collisions)
        stream.extend(range(bcol_max + a_br_t, bcol_max + b_br + 1))
        stream.append(struct_id)

    N = len(stream)
    maxid = struct_id + 1
    last = np.full(maxid, -1, dtype=np.int64)
    bit = np.zeros(N + 1, dtype=np.int64)

    def query(i):
        s = 0
        while i > 0:
            s += bit[i]
            i -= i & -i
        return s

    def update(i, d):
        while i <= N:
            bit[i] += d
            i += i & -i

    hist = {}
    inf_cnt = 0
    for tau, lid in enumerate(stream):
        p = int(last[lid])
        if p < 0:
            inf_cnt += 1
        else:
            # lines whose last reference lies in (p, tau):
            #   distinct lines referenced strictly between the two refs
            rd = query(tau) - query(p) - 1
            hist[rd] = hist.get(rd, 0) + 1
        if p >= 0:
            update(p + 1, -1)
        update(tau + 1, 1)
        last[lid] = tau
    return hist, inf_cnt, N


# ── CSR exact-RD + per-class aggregation ─────────────────────────────────
# CSR has no dup-scan (scan unused) and no closed-form candidate here; the
# exact Fenwick materialization is the CSR oracle.  Class mapping follows the
# canonical semantics in trace_gen.py (move = col_idx tail, brow = row_ptr
# prefix R-M-W, struct = header line; locate folded into brow).


def _csr_op_ranges(trace):
    """Per-op (col/move lines, row_ptr/brow lines) in cache-line id space."""
    ops = trace["ops"]
    o = trace["col_base_mod64"][0] // 16
    o_rp = trace["rp_base_mod64"][0] // 8
    n = trace["n"]
    ranges = []
    for op in ops:
        a = (op["ip"] + o) // 16            # move tail start line
        b = (op["T"] + o) // 16             # inclusive end line (append at T)
        a_br = (op["rp"] + o_rp) // 8       # row_ptr prefix start line
        b_br = (n + o_rp) // 8              # row_ptr max line (inclusive)
        col = np.arange(a, b + 1, dtype=np.int64)
        br = np.arange(a_br, b_br + 1, dtype=np.int64)
        ranges.append((col, br))
    return ranges


def _fenwick_rd(stream):
    """Exact RD per reference over a materialized line-id stream; first
    references (cold) get RD = -1 (INF)."""
    if not stream:
        return np.array([], dtype=np.int64)
    N = len(stream)
    maxid = max(stream) + 1
    last = np.full(maxid, -1, dtype=np.int64)
    bit = np.zeros(N + 1, dtype=np.int64)
    rds = np.full(N, -1, dtype=np.int64)

    def query(i):
        s = 0
        while i > 0:
            s += bit[i]
            i -= i & -i
        return s

    def update(i, d):
        while i <= N:
            bit[i] += d
            i += i & -i

    for tau, lid in enumerate(stream):
        p = int(last[lid])
        if p >= 0:
            rd = query(tau) - query(p) - 1
            rds[tau] = rd
            update(p + 1, -1)
        update(tau + 1, 1)
        last[lid] = tau
    return rds


def csr_class_counts(trace, l2_lines=L2_LINES, l3_lines=L3_LINES):
    """Per-class [l2, l3, dram] line counts for a CSR trace via exact RD.

    Returns (counts, total_refs, inf_refs) where counts maps
    {move, brow, struct} -> [l2, l3, dram]."""
    ranges = _csr_op_ranges(trace)
    n = trace["n"]
    o_rp = trace["rp_base_mod64"][0] // 8
    b_br = (n + o_rp) // 8
    col_max = 0
    for col, _ in ranges:
        if len(col):
            col_max = max(col_max, int(col[-1]) + 1)
    stream = []
    classes = []
    for col, br in ranges:
        for c in col:
            stream.append(int(c))
            classes.append("move")
        for c in br:
            stream.append(int(col_max + c))
            classes.append("brow")
        stream.append(int(col_max + b_br + 1))
        classes.append("struct")
    rds = _fenwick_rd(stream)
    counts = {"move": [0, 0, 0], "brow": [0, 0, 0], "struct": [0, 0, 0]}
    inf_mask = rds < 0
    tiers = rd_to_tier(rds, inf_mask, l2_lines, l3_lines)
    for tier, cls in zip(tiers, classes):
        counts[cls][int(tier)] += 1
    return counts, len(stream), int(np.count_nonzero(inf_mask))


def csr_exact(trace, l2_lines=L2_LINES, l3_lines=L3_LINES):
    """Global RD histogram over a CSR trace (all classes merged)."""
    ranges = _csr_op_ranges(trace)
    n = trace["n"]
    o_rp = trace["rp_base_mod64"][0] // 8
    b_br = (n + o_rp) // 8
    col_max = 0
    for col, _ in ranges:
        if len(col):
            col_max = max(col_max, int(col[-1]) + 1)
    stream = []
    for col, br in ranges:
        stream.extend(int(c) for c in col)
        stream.extend(int(col_max + c) for c in br)
        stream.append(int(col_max + b_br + 1))
    rds = _fenwick_rd(stream)
    hist = {}
    inf_cnt = 0
    for rd in rds:
        if rd < 0:
            inf_cnt += 1
        else:
            hist[int(rd)] = hist.get(int(rd), 0) + 1
    return hist, inf_cnt, len(stream)


def summarize(trace, o, o_br):
    """Candidate histogram + percentiles + cumulative hit rates."""
    hist, inf_cnt, total = candidate(trace, o, o_br)
    finite = sorted((d, c) for d, c in hist.items())
    acc = 0
    pcts = {}
    targets = {0.50: "p50", 0.75: "p75", 0.90: "p90",
               0.95: "p95", 0.99: "p99"}
    tot_fin = total - inf_cnt
    for d, c in finite:
        acc += c
        for f, name in targets.items():
            if name not in pcts and acc >= f * tot_fin:
                pcts[name] = d
    for f, name in targets.items():
        pcts.setdefault(name, None)
    h_l2, h_l3 = hit_ratios(hist, inf_cnt, total)
    return {
        "total_refs": total,
        "inf_refs": inf_cnt,
        "p_inf": inf_cnt / total,
        "unique_lines": len(finite) + 1,
        "h_l2": h_l2,
        "h_l3": h_l3,
        "pcts": pcts,
        "hist": hist,
    }


def validate(traces):
    """Tiny-suite + fb gate: candidate must equal exact (incl. INF bin)."""
    ok_all = True
    for tr in traces:
        for o in tr["bcol_base_mod64"]:
            for o_br in tr["brow_base_mod64"]:
                ch, ci, ct = candidate(tr, o // 16, o_br // 16)
                eh, ei, et = exact(tr, o // 16, o_br // 16)
                match = (ci == ei and ct == et and ch == eh)
                if not match:
                    ok_all = False
                print(f"{tr['graph']:24s} o={o//16} o_br={o_br//16} "
                      f"refs={et} inf={ei}  candidate==exact: {match}")
                if not match:
                    keys = sorted(set(ch) | set(eh))
                    for k in keys[:8]:
                        print(f"    d={k}: cand={ch.get(k,0)} exact={eh.get(k,0)}")
    print(f"\nGATE: {'PASS' if ok_all else 'FAIL'}")
    return ok_all


def main():
    traces = json.load(open("/tmp/opencode/traces.json"))
    # tiny suite + fb through the gate
    gate_traces = tg.gen_tiny_suite()
    fb = traces["fb-pages-company"]
    fb["graph"] = "fb-pages-company"
    gate_traces.append(fb)
    gate = validate(gate_traces)
    if not gate:
        sys.exit("gate FAILED — candidate rejected; no real-graph predictions")

    out = {}
    for g, tr in sorted(traces.items()):
        o = tr["bcol_base_mod64"][0] // 16
        o_br = tr["brow_base_mod64"][0] // 16
        s = summarize(tr, o, o_br)
        s["graph"] = g
        s["bcol_o"] = o
        s["brow_o"] = o_br
        out[g] = s
        print(f"{g:24s} refs={s['total_refs']:10d} inf={s['inf_refs']:7d} "
              f"p_inf={s['p_inf']:.4f} h2={s['h_l2']:.4f} h3={s['h_l3']:.4f} "
              f"p50={s['pcts']['p50']} p75={s['pcts']['p75']} "
              f"p90={s['pcts']['p90']} p95={s['pcts']['p95']} "
              f"p99={s['pcts']['p99']}")
    json.dump(out, open("/tmp/opencode/rd_histograms.json", "w"), indent=1)
    print("\nwrote /tmp/opencode/rd_histograms.json")


if __name__ == "__main__":
    main()
