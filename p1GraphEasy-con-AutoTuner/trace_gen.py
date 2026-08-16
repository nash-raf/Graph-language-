"""trace_gen.py — ordered symbolic memory-event generators.

A *representation registry* (REPS) lets any graph representation plug in its
own build/replay/geometry so the generic machinery (edge load, workload
generation, trace record) is rep-agnostic.  Adding a new representation only
requires implementing its build/replay/geometry — the RD/MRC pipeline
(rd_hist, access_class_cost, analytic_rd) consumes the emitted op records.

Representations
---------------
bcsr (default, unchanged): mirrors autograph_bcsr_add_edge
    (autotuner_runtime.c:1455) EXACTLY in int32 units, per directed add:
      1. dup scan      reads  bcol[start..end)   (bcol[k], k += 2)
      2. insert scan   reads  bcol[start..ip)    (first pair with local_row > row)
      3. realloc(+2 int32)     (no memory traffic; relocs == 0 proven)
      4. memmove       reads  bcol[ip..T), writes bcol[ip+2..T+2)   (T += 2)
      5. pair write    writes bcol[ip..ip+2)
      6. brow update   reads+writes brow[blk+1..nb]   (each += 2)
      7. struct write  writes 1 line (Graph.m, 8 bytes at graph base + 8)
    Classes: scan (block-row dup scan), move (memmove tail), brow (prefix
    sums), struct (header write).

csr: mirrors csr_add_directed (graph_mutation_runtime.c:42), per directed add:
      1. locate       reads  row_ptr[from+1] (int64) — FOLDED into brow (same
                      line as the prefix-update range; latency t only)
      2. realloc col_idx (+1 int32)  (no relocation copy; same proven
                      no-reloc convention as BCSR)
      3. header write writes Graph.m (base+8) / col_idx ptr (base+24) -> 1 line
      4. memmove       reads/writes col_idx[ip..T)   (T += 1)
      5. element write writes col_idx[ip] = to  (within the move line set)
      6. row_ptr       reads+writes row_ptr[from+1..n]   (each += 1)
    Classes: move (col_idx memmove tail), brow (row_ptr prefix R-M-W),
    struct (graph header write); scan unused (no dup-scan); locate folded
    into brow.  Excluded: AutoGraphMeta pointer updates
    (autograph_update_csr_pointers / autograph_record_adjacency_state /
    autograph_mark_canonical_dirty) — runtime-autotuner overhead, same
    convention as BCSR.

Line conventions (both reps, documented here and applied identically by
rd_hist / access_class_cost):
  - each cache line touched by an op is referenced exactly once (intra-op
    read+write collapse; second reference would be RD=0 = hit for C >= 1).
  - reference order within an op: bcol/col lines ascending, then brow/row_ptr
    lines ascending, then the struct line.
  - line(i) = floor((base + elem_size*i)/64); base mod 64 probed from glibc.
    bcol/col_idx are int32 (o = base_mod64 // 16), row_ptr/brow-style int64
    arrays use o = base_mod64 // 8.
"""
import ctypes
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import audit_insert_cost as audit
import cost_model as cm

B = 64          # BCSR block size (kernel: meta->bcsr_block_size)
LS = 64         # cache line bytes


def probe_base_mod64(nbytes):
    """Real glibc base alignment (mod 64) for a malloc of nbytes."""
    libc = ctypes.CDLL("libc.so.6", use_errno=True)
    libc.malloc.restype = ctypes.c_void_p
    libc.malloc.argtypes = [ctypes.c_size_t]
    libc.free.argtypes = [ctypes.c_void_p]
    bases = []
    for _ in range(32):
        p = libc.malloc(nbytes)
        if p:
            bases.append(p % 64)
        libc.free(p)
    return sorted(set(bases))


def load_undirected_graph(edge_file):
    """rows[u] = sorted list of neighbors; m = undirected edges; n = max id."""
    rows = []
    with open(edge_file) as f:
        for line in f:
            p = line.split()
            if len(p) < 2:
                continue
            try:
                u, v = int(p[0]), int(p[1])
            except ValueError:
                continue
            while len(rows) <= max(u, v):
                rows.append([])
            rows[u].append(v)
            rows[v].append(u)
    for r in rows:
        r.sort()
    m = sum(len(r) for r in rows) // 2
    return rows, m


def gen_workload_from_rows(rows, seed=42):
    """Same 50 (u,v) pairs make_insert_workload(seed 42) semantics against an
    in-memory graph: no self-loops, not already an edge, no repeated pair."""
    import random
    n = len(rows)
    existing = set()
    for u, r in enumerate(rows):
        for v in r:
            if v > u:
                existing.add((u, v))
    rng = random.Random(seed)
    chosen = []
    chosen_set = set()
    max_attempts = max(50 * 1000, 10000)
    attempts = 0
    while len(chosen) < 50 and attempts < max_attempts:
        attempts += 1
        u = rng.randint(0, n - 1)
        v = rng.randint(0, n - 1)
        if u == v:
            continue
        lo, hi = (u, v) if u < v else (v, u)
        if (lo, hi) in existing or (lo, hi) in chosen_set:
            continue
        chosen_set.add((lo, hi))
        chosen.append((u, v))
    return chosen


def build_blocks(rows, b=B):
    """bcol block slices as pair lists: (local_row, col), grouped by local_row
    ascending, mirroring convert_csr_to_bcsr (autotuner_runtime.c:911)."""
    nb = (len(rows) + b - 1) // b
    blocks = [[] for _ in range(nb)]
    for u, r in enumerate(rows):
        for v in r:
            if v > u:                       # each undirected edge once per end
                blocks[u // b].append((u % b, v))
                blocks[v // b].append((v % b, u))
    return blocks, nb


def replay_adds(rows, blocks, nb, workload, b=B):
    """Replay the kernel add sequence; emit per-op symbolic records.

    Records are (start, end, ip, T, blk) — int32 offsets into bcol.  The
    pair lists are updated exactly as the kernel updates bcol/brow.
    """
    n = len(rows)
    elems = [len(sl) for sl in blocks]      # pairs per block
    brow = [0] * (nb + 1)                   # prefix of pairs -> int32 offsets
    for i in range(nb):
        brow[i + 1] = brow[i] + 2 * elems[i]
    T = brow[nb]                            # total int32s (= 2*m_dir)
    ops = []
    for u, v in workload:
        for fr, to in ((u, v), (v, u)):
            blk = fr // b
            row = fr % b
            start = brow[blk]
            end = brow[blk + 1]
            sl = blocks[blk]
            ip_pair = len(sl)
            for i, (lr, _) in enumerate(sl):
                if lr > row:
                    ip_pair = i
                    break
            ip = start + 2 * ip_pair        # int32 offset of insert position
            ops.append({"t": len(ops), "blk": blk, "row": row,
                        "start": start, "end": end, "ip": ip, "T": T})
            sl.insert(ip_pair, (row, to))
            T += 2
            for i in range(blk + 1, nb + 1):
                brow[i] += 2
    return ops


def gen_trace(graph_label=None, edge_file=None, n=None, m=None, k_ins=50,
              seed=42, rep="bcsr"):
    """Build the full trace record for a graph under `rep`.  Either
    graph_label (resolved via cost_model) or explicit (edge_file, n)."""
    rep = rep or "bcsr"
    if rep not in REPS:
        raise ValueError(f"unknown rep {rep!r}; available: {sorted(REPS)}")
    if edge_file is None:
        edge_file = cm._resolve_edge_file(graph_label)
    rows, m = load_undirected_graph(edge_file)
    n = len(rows)
    work = gen_workload_from_rows(rows, seed=seed)
    work = work[:k_ins]
    mod = REPS[rep]
    state = mod["build"](rows)
    ops = mod["replay"](rows, state, work)
    geom = mod["geometry"](rows, state, n, m)
    return {
        "graph": graph_label,
        "edge_file": edge_file,
        "n": n,
        "m": m,
        "rep": rep,
        "n_ops": len(ops),
        **geom,
        "ops": ops,
    }


# ── BCSR representation ──────────────────────────────────────────────────
def _bcsr_geometry(rows, state, n, m):
    blocks, nb = state
    t0 = 4 * m
    return {
        "nb": nb,
        "bcol_base_mod64": probe_base_mod64(t0),
        "brow_base_mod64": probe_base_mod64((nb + 1) * 4),
        "T0": t0,
    }


def _bcsr_build(rows):
    return build_blocks(rows)


def _bcsr_replay(rows, state, work):
    blocks, nb = state
    return replay_adds(rows, blocks, nb, work)


# ── CSR representation ───────────────────────────────────────────────────
def _csr_build(rows):
    """row_ptr int64 prefix over undirected degrees; row v holds deg[v]
    directed cols (all neighbors).  col_idx is the flat directed adjacency
    (row-major), the growing array mirrored by the kernel."""
    n = len(rows)
    row_ptr = [0] * (n + 1)
    for i, r in enumerate(rows):
        row_ptr[i + 1] = row_ptr[i] + len(r)
    return {"row_ptr": row_ptr, "n": n}


def _csr_replay(rows, state, work):
    """Mirror csr_add_directed (graph_mutation_runtime.c:42) exactly:
    pos = row_ptr[from+1]; insert at pos; row_ptr[from+1..n] += 1; T += 1.
    Per directed add.  Op records use int32 col offsets (start/end/ip/T) and
    the row_ptr index (rp) for the brow line mapping."""
    row_ptr = list(state["row_ptr"])
    n = len(rows)
    T = sum(len(r) for r in rows)          # current directed cols (T0)
    ops = []
    for u, v in work:
        for fr, to in ((u, v), (v, u)):
            pos = row_ptr[fr + 1]
            ops.append({"t": len(ops), "row": fr, "start": row_ptr[fr],
                        "end": pos, "ip": pos, "T": T, "rp": fr + 1})
            for i in range(fr + 1, n + 1):
                row_ptr[i] += 1
            T += 1
    return ops


def _csr_geometry(rows, state, n, m):
    t0 = 2 * m                              # directed cols (int32 each)
    return {
        "col_base_mod64": probe_base_mod64(t0 * 4),
        "rp_base_mod64": probe_base_mod64((n + 1) * 8),
        "T0": t0,
    }


REPS = {
    "bcsr": {"build": _bcsr_build, "replay": _bcsr_replay,
             "geometry": _bcsr_geometry},
    "csr": {"build": _csr_build, "replay": _csr_replay,
            "geometry": _csr_geometry},
}


def gen_tiny_suite():
    """Synthetic tiny graphs for the exact-vs-candidate gate."""
    import random
    rng = random.Random(7)
    suite = []
    for n, m in ((8, 10), (16, 10), (16, 40), (32, 10), (32, 40)):
        edges = set()
        while len(edges) < m:
            a, b = rng.randint(0, n - 1), rng.randint(0, n - 1)
            if a == b:
                continue
            edges.add((min(a, b), max(a, b)))
        rows = [[] for _ in range(n)]
        for a, b in edges:
            rows[a].append(b)
            rows[b].append(a)
        for r in rows:
            r.sort()
        work = gen_workload_from_rows(rows, seed=42)
        blocks, nb = build_blocks(rows)
        ops = replay_adds(rows, blocks, nb, work)
        suite.append({
            "graph": f"synthetic_n{n}_m{m}",
            "edge_file": None, "n": n, "m": m, "nb": nb,
            "rep": "bcsr",
            "n_ops": len(ops),
            "bcol_base_mod64": [0, 16, 32, 48],
            "brow_base_mod64": [0, 16, 32, 48],
            "T0": 4 * m,
            "ops": ops,
        })
    return suite


def main():
    out = {}
    import csv
    for path in ("real_world_runs10_merged.csv", "meh_validate_updated.csv"):
        if not os.path.exists(path):
            continue
        with open(path) as f:
            for r in csv.DictReader(f):
                if r.get("operation") == "Insert" and r.get("layout") == "BCSR":
                    g = r["graph"]
                    if g in out:
                        continue
                    out[g] = gen_trace(graph_label=g)
    with open("/tmp/opencode/traces.json", "w") as f:
        json.dump(out, f)
    for g, tr in sorted(out.items()):
        print(f"{g:24s} n={tr['n']:7d} m={tr['m']:9d} nb={tr['nb']:6d} "
              f"ops={tr['n_ops']:3d} T0={tr['T0']:10d} "
              f"bcol%64={tr['bcol_base_mod64']}")
    print("\nwrote /tmp/opencode/traces.json")


if __name__ == "__main__":
    main()