"""analytic_rd.py — closed-form expected RD/MRC estimator for BCSR insertion.

Under the stated insertion-process assumptions (uniform vertex selection,
iid block draws, within-block insert position per the degree vector), the
expected RD of an insertion into block k at workload position t is the
per-op constant

    RD_k(t) = (b - a_t(k,t)) + (b_br - a_br[k]) + 1        (cache lines)

where b is the array end line, a_t the block-row start line (including the
deterministic insertion drift: +2 int32 per prior op in a lower block) and
the second term is the brow-window suffix.  The covering process is
governed by block-row START lines (a_s <= ell iff blk_s is a covering
block), not by the insertion position.

First-reference lines (INF: lines below the running minimum of block
starts seen so far) are charged to DRAM; their expected count is computed
exactly from the order statistics of the block-start distribution over
the workload's iid block draws.

    h_C(class) = (1/N) sum_t sum_k P(blk=k) * (w_k - INF_t(k)) * 1[RD_k(t) < C]

No replay: everything is a finite sum over the block structure.
The exact replay (rd_hist.iter_op_line_rd) is the validation oracle.
"""
import json
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rd_hist as rh

L2_LINES = rh.L2_LINES
L3_LINES = rh.L3_LINES


def block_structure_from_edges(edge_file, o=0, o_br=0):
    """Degree-vector block structure: brow int32 prefix, block line counts,
    expected insert-position lines, block vertex weights.

    bcol layout: T0 = 4*m int32 (2 int32 per directed entry; the undirected
    graph is stored as both directions).  Block k's row holds
    entries_k = sum of degrees of its vertices, int32 count 2*entries_k.
    """
    deg = {}
    with open(edge_file) as f:
        for line in f:
            p = line.split()
            if len(p) < 2:
                continue
            try:
                u, v = int(p[0]), int(p[1])
            except ValueError:
                continue
            deg[u] = deg.get(u, 0) + 1
            deg[v] = deg.get(v, 0) + 1
    n = max(deg) + 1 if deg else 0
    m = sum(deg.values()) // 2
    nb = (n + 63) // 64
    brow = np.zeros(nb + 1, dtype=np.int64)
    verts = np.zeros(nb, dtype=np.int64)
    for k in range(nb):
        v0 = k * 64
        v1 = min(n, v0 + 64)
        verts[k] = v1 - v0
        e = sum(deg.get(v, 0) for v in range(v0, v1))
        brow[k + 1] = brow[k] + 2 * e          # int32 row offsets
    return {
        "n": n, "m": m, "nb": nb, "deg": deg,
        "brow": brow, "verts": verts,
        "brow_line": (brow[:-1] + o) // 16,          # block-row start line
        "row_lines": ((brow[1:] - brow[:-1]) + 15) // 16,   # row in lines
        "o": o, "o_br": o_br,
        "b": (4 * m + 1 + o) // 16,            # array end line
        "b_br": (nb - 1 + o_br) // 16,         # brow max line
    }


def expected_ip(st):
    """E[insert position line | blk=k] = (brow[k] + 2*E[ip_pair] + o)//16.

    ip_pair = the start of the new row's run = cumulative pairs of rows
    below row u, averaged over u uniform within the block.
    """
    brow, nb, deg = st["brow"], st["nb"], st["deg"]
    n = st["n"]
    ip = np.empty(nb, dtype=np.float64)
    for k in range(nb):
        v0 = k * 64
        v1 = min(n, v0 + 64)
        cum = 0.0
        total = 0.0
        nv = v1 - v0
        for u in range(v0, v1):
            total += cum
            cum += deg.get(u, 0)
        ip[k] = brow[k] + 2.0 * (total / max(nv, 1))
    return (ip + st["o"]) // 16


class AnalyticBCSR:
    """Expected per-class (N, L2, L3, DRAM) line counts and cost."""

    def __init__(self, st, n_ops=100):
        self.st = st
        self.n_ops = n_ops
        nb = st["nb"]
        P = st["verts"] / st["n"]
        self.P = P
        bl = st["brow_line"]
        b = st["b"]
        self.a_br = (np.arange(nb, dtype=np.int64) + 1 + st["o_br"]) // 16
        b_br = st["b_br"]
        # expected insert drift: +2 int32 per prior op in a lower block
        drift = np.cumsum(P) / 8.0                 # 2*P(blk < k)/16, lines
        self.drift = np.concatenate([[0.0], drift[:-1]])
        # per-op RD at t=1 (no drift yet): a_t = brow_line[k]
        #   RD_k = (b - a_t) + (b_br - a_br[p] + 2),  p = last covering op.
        # E[a_br[p]] = sum_j row_lines[j]*P(blk<=j)*E[a_br|blk<=j] / norm
        #             = sum_j row_lines[j]*sum_{i<=j} P(i)*a_br[i] / norm
        P = self.P
        rl = st["row_lines"].astype(float)
        cumP = np.cumsum(P)
        E_a_br_p = (rl * cumP * (np.cumsum(P * self.a_br) / np.maximum(cumP, 1e-300))).sum() / max((rl * cumP).sum(), 1e-300)
        bw_const = b_br + 2 - E_a_br_p
        self.bw_const = bw_const
        self.rd0 = (b - bl) + bw_const
        # per-op class line weights (given blk=k)
        ip = expected_ip(st)
        self.w = {
            "scan": st["row_lines"].astype(float),
            "move": (b - ip + 1).astype(float),
            "brow": (b_br - self.a_br + 1).astype(float),
            "struct": np.ones(nb),
        }
        # INF machinery survival functions over iid block draws
        self._surv = self._survival(bl)
        self._surv_blk = self._survival(self.a_br)

    def _survival(self, x):
        """S(v) = P(start > v) over distinct values, per-draw."""
        order = np.argsort(x)
        xs = x[order]
        w = self.P[order]
        cum = np.cumsum(w)
        return xs, 1.0 - cum

    @staticmethod
    def _min_excess_matrix(xs, S, y, n_ops):
        """E[(M_{t-1} - y[k])^+] for t = 2..n_ops (row t-1); row 0 = +inf
        (zero prior draws -> every line first-reference)."""
        order = np.argsort(xs)
        xs_s = xs[order]
        S_s = S[order]
        S2 = np.concatenate([S_s[1:], [0.0]])
        idx = np.searchsorted(xs_s, y, side="right")
        n = len(y)
        M = np.empty((n_ops, n))
        M[0] = np.inf
        for t in range(2, n_ops + 1):
            pm = S_s ** (t - 1) - S2 ** (t - 1)      # P(M = xs_s[j])
            cA = np.cumsum(pm * xs_s)
            cB = np.cumsum(pm)
            prefA = np.concatenate([[0.0], cA[:-1]])
            prefB = np.concatenate([[0.0], cB[:-1]])
            sufA = np.concatenate([cA[-1] - prefA, [0.0]])
            sufB = np.concatenate([cB[-1] - prefB, [0.0]])
            e = sufA[idx] - y * sufB[idx]
            M[t - 1] = np.maximum(e, 0.0)
        return M

    def expected_tiers(self):
        st = self.st
        nb = st["nb"]
        n_ops = self.n_ops
        P = self.P
        xs, S = self._surv
        xs_br, S_br = self._surv_blk
        bl = st["brow_line"]
        bl_end = np.concatenate([bl[1:], [st["b"] + 1]])
        ip = expected_ip(st)
        inf_move = self._min_excess_matrix(xs, S, ip, n_ops)
        inf_scan = (self._min_excess_matrix(xs, S, bl, n_ops)
                    - self._min_excess_matrix(xs, S, bl_end, n_ops))
        inf_brow = self._min_excess_matrix(xs_br, S_br, self.a_br, n_ops)
        inf_move[0] = self.w["move"]       # first op: all lines first-ref
        inf_brow[0] = self.w["brow"]
        inf_scan[0] = self.w["scan"]

        res = {}
        for cls, w in self.w.items():
            c = np.zeros(3)
            e_inf = {"move": inf_move, "scan": inf_scan,
                     "brow": inf_brow}.get(cls)
            for t in range(1, n_ops + 1):
                rd = self.rd0 - (t - 1) * self.drift
                if e_inf is not None:
                    ei = np.where(np.isinf(e_inf[t - 1]), 0.0,
                                  e_inf[t - 1])
                    ei = np.maximum(ei, 0.0)
                    ei = np.minimum(ei, w)
                else:
                    ei = np.zeros(nb)
                    if t == 1:                       # first op: tier 2
                        ei = w.copy()
                in2 = rd < L2_LINES
                in3 = rd < L3_LINES
                l2 = w * in2
                l3 = w * (~in2) * in3
                dram = w * (~in3)
                # INF lines are the lowest-j (highest-RD) lines: take them
                # from L2 first, then L3; they are always charged DRAM.
                t2 = np.maximum(l2 - ei, 0.0)
                rem = np.maximum(ei - l2, 0.0)
                t3 = np.maximum(l3 - rem, 0.0)
                td = dram + ei
                c[0] += float(np.dot(P, t2))
                c[1] += float(np.dot(P, t3))
                c[2] += float(np.dot(P, td))
            res[cls] = c
        return res

    def cost(self, rates):
        tiers = self.expected_tiers()
        total = 0.0
        detail = {}
        for p in tiers:
            c = tiers[p]
            n_s = c.sum()
            h2 = c[0] / n_s if n_s else 0.0
            h3 = (c[0] + c[1]) / n_s if n_s else 0.0
            r = rates[p]
            ns = n_s * (h2 * r[0] + (h3 - h2) * r[1] + (1.0 - h3) * r[2])
            detail[p] = {"lines": n_s, "h2": h2, "h3": h3,
                         "tier_lines": list(c), "ns": ns}
            total += ns
        return total, detail

    def per_class_per_dir(self):
        """Per-directed-insert expected (N_s, h2_s, h3_s) for each access
        class, from expected_tiers() over the n_ops-op workload.  This is
        the compile-time metadata payload (IRGen/AutoTunerPass mirror it).
        """
        tiers = self.expected_tiers()
        out = {}
        for p, c in tiers.items():
            tot = float(c.sum())
            per_op = tot / self.n_ops
            h2 = c[0] / tot if tot else 0.0
            h3 = (c[0] + c[1]) / tot if tot else 0.0
            out[p] = (per_op, float(h2), float(h3))
        return out

    @staticmethod
    def class_tiers_from_edges(edge_file, o=0, o_br=0, n_ops=100):
        """One-call compile-time entry point: per-class (N, h2, h3) per
        directed insert, straight from the edge file (no replay, no o
        knowledge beyond the //16 convention).  Mirrored by
        IRGenVisitor::estimateClassTiers().
        """
        st = block_structure_from_edges(edge_file, o=o, o_br=o_br)
        return AnalyticBCSR(st, n_ops=n_ops).per_class_per_dir()


def class_tiers_from_edges(edge_file, o=0, o_br=0, n_ops=100):
    """Module-level convenience: AnalyticBCSR.class_tiers_from_edges()."""
    return AnalyticBCSR.class_tiers_from_edges(edge_file, o=o, o_br=o_br,
                                               n_ops=n_ops)


# ══════════════════════════════════════════════════════════════════════
# CSR analytic estimator (mirror of AnalyticBCSR)
#
# Canonical CSR classes (per directed csr_add_directed, trace_gen.py):
#   move   -> col_idx memmove tail      (rmw rates)
#   brow   -> row_ptr[from+1..n] prefix R-M-W  (rmw rates)
#   struct -> graph header line          (rand rates)
#   scan   -> unused (no dup-scan); locate folded into brow.
#
# CSR geometry: col_idx is int32 with T0 = 2*m directed cols; row_ptr is
# int64 (n+1).  Line conventions mirror BCSR with the element-size split:
# col_idx line(i) = (i + o)//16 (o = col_base_mod64//16); row_ptr line(i) =
# (i + o_rp)//8 (o_rp = rp_base_mod64//8).  Each op grows col_idx by 1 int32
# -> drift cumsum(P)/16 (vs BCSR's 2 int32 -> /8).
# ══════════════════════════════════════════════════════════════════════


def csr_structure_from_edges(edge_file, o=0, o_rp=0):
    """CSR degree structure over 64-vertex blocks: row_ptr int64 prefix,
    per-block col start/insert-position lines and row_ptr brow lines."""
    deg = {}
    with open(edge_file) as f:
        for line in f:
            p = line.split()
            if len(p) < 2:
                continue
            try:
                u, v = int(p[0]), int(p[1])
            except ValueError:
                continue
            deg[u] = deg.get(u, 0) + 1
            deg[v] = deg.get(v, 0) + 1
    n = max(deg) + 1 if deg else 0
    m = sum(deg.values()) // 2
    nb = (n + 63) // 64

    # row_ptr[v] = sum_{i<v} deg[i] (directed col offset of row v's start).
    row_ptr = np.zeros(n + 1, dtype=np.int64)
    for v in range(1, n + 1):
        row_ptr[v] = row_ptr[v - 1] + deg.get(v - 1, 0)

    verts = np.zeros(nb, dtype=np.int64)
    bl = np.empty(nb, dtype=np.float64)       # block col start line
    ip = np.empty(nb, dtype=np.float64)       # E[insert-position line]
    a_rp = np.empty(nb, dtype=np.float64)     # E[row_ptr prefix start line]
    rl = np.empty(nb, dtype=np.float64)       # block col line span
    for k in range(nb):
        v0 = k * 64
        v1 = min(n, v0 + 64)
        nv = v1 - v0
        verts[k] = nv
        base = int(row_ptr[v0])
        bl[k] = (base + o) // 16
        cum = 0
        ip_sum = 0.0
        a_rp_sum = 0.0
        for u in range(v0, v1):
            cum += deg.get(u, 0)
            ip_sum += (base + cum + o) // 16
            a_rp_sum += (u + 1 + o_rp) // 8
        ip[k] = ip_sum / max(nv, 1)
        a_rp[k] = a_rp_sum / max(nv, 1)
        rl[k] = (int(row_ptr[v1]) - base + 15) // 16

    return {
        "n": n, "m": m, "nb": nb, "deg": deg,
        "verts": verts, "bl": bl, "ip": ip, "a_rp": a_rp, "rl": rl,
        "o": o, "o_rp": o_rp,
        "b": (2 * m + o) // 16,            # col array end line (inclusive)
        "b_rp": (n + o_rp) // 8,           # row_ptr max line (inclusive)
    }


class AnalyticCSR:
    """Expected per-class (N, L2, L3, DRAM) line counts for CSR insertion.

    Mirrors AnalyticBCSR's covering/INF machinery with CSR start-lines and
    weights; scan is absent (no dup-scan)."""

    def __init__(self, st, n_ops=100):
        self.st = st
        self.n_ops = n_ops
        nb = st["nb"]
        P = st["verts"] / st["n"]
        self.P = P
        bl = st["bl"]
        ip = st["ip"]
        a_rp = st["a_rp"]
        b = st["b"]
        b_rp = st["b_rp"]
        self.a_rp = a_rp
        # expected insert drift: +1 int32 per prior op in a lower block =
        # 1/16 line per op (BCSR: 2 int32 -> 1/8).
        drift = np.cumsum(P) / 16.0
        self.drift = np.concatenate([[0.0], drift[:-1]])
        # per-op RD at t=1: (b - bl) + brow-window constant (CSR analog of
        # BCSR's bw_const, weighted by the block's col-line span rl).
        P = self.P
        rl = st["rl"]
        cumP = np.cumsum(P)
        E_a_rp_p = (rl * cumP * (np.cumsum(P * a_rp) /
                                 np.maximum(cumP, 1e-300))).sum() / \
            max((rl * cumP).sum(), 1e-300)
        bw_const = b_rp + 2 - E_a_rp_p
        self.bw_const = bw_const
        self.rd0 = (b - bl) + bw_const
        # per-op class line weights (given blk=k)
        self.w = {
            "move": (b - ip + 1).astype(float),
            "brow": (b_rp - a_rp + 1).astype(float),
            "struct": np.ones(nb),
        }
        # Per-class RD at t=1.
        #   move:  (b - bl) + global brow window (bw_const) — the move RD
        #          between two coverages includes the whole brow suffix.
        #   brow:  (b - bl) + the block's OWN brow suffix (b_rp - a_rp + 2):
        #          a brow line's RD window is its own suffix, not the global
        #          window; using bw_const for every block pushes every brow
        #          line past L2 on large graphs, while the exact replay keeps
        #          high (recently-covered) lines in L2.
        #   struct: the op's own move+brow traffic (op-last reference order).
        self.rd0_brow = (b - bl) + (b_rp - a_rp + 2.0)
        self.struct_rd = (b - ip + 1) + (b_rp - a_rp + 1)
        self._surv = self._survival(ip)        # move start distribution
        self._surv_rp = self._survival(a_rp)   # brow start distribution

    def _survival(self, x):
        order = np.argsort(x)
        xs = x[order]
        w = self.P[order]
        cum = np.cumsum(w)
        return xs, 1.0 - cum

    @staticmethod
    def _min_excess_matrix(xs, S, y, n_ops):
        order = np.argsort(xs)
        xs_s = xs[order]
        S_s = S[order]
        S2 = np.concatenate([S_s[1:], [0.0]])
        idx = np.searchsorted(xs_s, y, side="right")
        n = len(y)
        M = np.empty((n_ops, n))
        M[0] = np.inf
        for t in range(2, n_ops + 1):
            pm = S_s ** (t - 1) - S2 ** (t - 1)
            cA = np.cumsum(pm * xs_s)
            cB = np.cumsum(pm)
            prefA = np.concatenate([[0.0], cA[:-1]])
            prefB = np.concatenate([[0.0], cB[:-1]])
            sufA = np.concatenate([cA[-1] - prefA, [0.0]])
            sufB = np.concatenate([cB[-1] - prefB, [0.0]])
            e = sufA[idx] - y * sufB[idx]
            M[t - 1] = np.maximum(e, 0.0)
        return M

    def expected_tiers(self):
        st = self.st
        nb = st["nb"]
        n_ops = self.n_ops
        P = self.P
        ip = st["ip"]
        a_rp = st["a_rp"]
        xs, S = self._surv
        xs_rp, S_rp = self._surv_rp
        inf_move = self._min_excess_matrix(xs, S, ip, n_ops)
        inf_brow = self._min_excess_matrix(xs_rp, S_rp, a_rp, n_ops)
        inf_move[0] = self.w["move"]       # first op: all lines first-ref
        inf_brow[0] = self.w["brow"]

        res = {}
        for cls, w in self.w.items():
            c = np.zeros(3)
            e_inf = {"move": inf_move, "brow": inf_brow}.get(cls)
            for t in range(1, n_ops + 1):
                if cls == "struct":
                    # RD = the op's own move+brow traffic (header line
                    # re-referenced every op with only its own traffic in
                    # between); no drift term needed.
                    rd = self.struct_rd
                elif cls == "brow":
                    rd = self.rd0_brow - (t - 1) * self.drift
                else:
                    rd = self.rd0 - (t - 1) * self.drift
                if e_inf is not None:
                    ei = np.where(np.isinf(e_inf[t - 1]), 0.0,
                                  e_inf[t - 1])
                    ei = np.maximum(ei, 0.0)
                    ei = np.minimum(ei, w)
                else:
                    ei = np.zeros(nb)
                    if t == 1:                       # first op: tier 2
                        ei = w.copy()
                in2 = rd < L2_LINES
                in3 = rd < L3_LINES
                l2 = w * in2
                l3 = w * (~in2) * in3
                dram = w * (~in3)
                t2 = np.maximum(l2 - ei, 0.0)
                rem = np.maximum(ei - l2, 0.0)
                t3 = np.maximum(l3 - rem, 0.0)
                td = dram + ei
                c[0] += float(np.dot(P, t2))
                c[1] += float(np.dot(P, t3))
                c[2] += float(np.dot(P, td))
            res[cls] = c
        return res

    def cost(self, rates):
        tiers = self.expected_tiers()
        total = 0.0
        detail = {}
        for p in tiers:
            c = tiers[p]
            n_s = c.sum()
            h2, h3 = rh.hit_ratios_from_tiers(c[0], c[1], c[2])
            r = rates[p]
            ns = n_s * (h2 * r[0] + (h3 - h2) * r[1] + (1.0 - h3) * r[2])
            detail[p] = {"lines": n_s, "h2": h2, "h3": h3,
                         "tier_lines": list(c), "ns": ns}
            total += ns
        return total, detail

    def per_class_per_dir(self):
        """Per-directed-insert expected (N_s, h2_s, h3_s) for
        {move, brow, struct} from expected_tiers() over the workload.
        scan is absent (unused for CSR); consumers emitting the uniform
        4-class payload insert scan = (0, 0, 0)."""
        tiers = self.expected_tiers()
        out = {}
        for p, c in tiers.items():
            tot = float(c.sum())
            per_op = tot / self.n_ops
            h2, h3 = rh.hit_ratios_from_tiers(c[0], c[1], c[2])
            out[p] = (per_op, float(h2), float(h3))
        return out

    @staticmethod
    def class_tiers_from_edges(edge_file, o=0, o_rp=0, n_ops=100):
        """One-call compile-time entry point: per-class (N, h2, h3) per
        directed insert straight from the edge file.  Mirrored by
        IRGenVisitor::estimateCsrClassTiers()."""
        st = csr_structure_from_edges(edge_file, o=o, o_rp=o_rp)
        return AnalyticCSR(st, n_ops=n_ops).per_class_per_dir()


def class_tiers_csr_from_edges(edge_file, o=0, o_rp=0, n_ops=100):
    """Module-level convenience: AnalyticCSR.class_tiers_from_edges()."""
    return AnalyticCSR.class_tiers_from_edges(edge_file, o=o, o_rp=o_rp,
                                              n_ops=n_ops)

    def bootstrap_h2_sd(self, cls, n_draws=3000, seed=0):
        """sd of h2 over independent n_ops-op workload draws (iid blocks
        from P); characterizes the sampling noise a single realized
        workload is subject to (INF lines ignored)."""
        rng = np.random.default_rng(seed)
        nb = self.st["nb"]
        n_ops = self.n_ops
        blks = rng.choice(nb, size=(n_draws, n_ops), p=self.P)
        w = self.w[cls][blks]
        t = np.arange(1, n_ops + 1)
        rd = self.rd0[blks] - t[None, :] * self.drift[blks]
        m = w * (rd < L2_LINES)
        h2 = m.sum(axis=1) / np.maximum(w.sum(axis=1), 1.0)
        return float(h2.std())


def class_rates():
    d = json.load(open("/tmp/opencode/class_calib.json"))
    lev = d["levels"]
    seq = (lev["seq"]["L2"], lev["seq"]["L3"], lev["seq"]["DRAM"])
    rmw = (lev["rmw"]["L2"], lev["rmw"]["L3"], lev["rmw"]["DRAM"])
    rnd = (lev["rand"]["L2"], lev["rand"]["L3"], lev["rand"]["DRAM"])
    return {
        "scan": seq, "move": rmw, "brow": rmw, "struct": rnd,
    }
