"""audit_insert_cost.py — Ground-truth replay of the CSR / BCSR add paths.

Mirrors the runtime semantics EXACTLY (no fitting, no calibration):
  CSR  : graph_mutation_runtime.c csr_add_directed     (pos = row_ptr[from+1],
         realloc(+1 int32), memmove tail, row_ptr[i]++ beyond from)
  BCSR : autotuner_runtime.c      autograph_bcsr_add_edge (insert_pos by
             local_row order, realloc(+2 int32), memmove tail, brow[i] += 2)

Each kernel runs the deterministic workload from
bench_folder.make_insert_workload(seed=42, K_INS=50) — the same pairs the
measured kernels run.  The audit replays both directed adds of each pair on
the same initial state and counts, per kernel:

  - shift_elems  : 4-byte elements touched by memmove
  - sweep_elems  : row_ptr (CSR) / brow (BCSR) suffix increments (int64)
  - reloc_elems  : 4-byte elements copied when realloc relocates (glibc
                   usable-size rounding; real usable sizes probed)

Compared against measured_ns / K_INS from the bench CSVs.
"""
import ctypes
import csv
import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bench_folder
import cost_model as cm

K = 50.0


def usable_size(nbytes):
    """Return the real glibc usable size for a malloc of nbytes."""
    libc = ctypes.CDLL("libc.so.6", use_errno=True)
    libc.malloc.restype = ctypes.c_void_p
    libc.malloc.argtypes = [ctypes.c_size_t]
    libc.malloc_usable_size.restype = ctypes.c_size_t
    libc.malloc_usable_size.argtypes = [ctypes.c_void_p]
    libc.free.argtypes = [ctypes.c_void_p]
    p = libc.malloc(nbytes + 16)   # extra so usable is never < request
    if not p:
        return nbytes + 16
    us = int(libc.malloc_usable_size(p))
    libc.free(p)
    if us < nbytes:
        us = nbytes
    return us


def gen_workload(edge_file, n):
    """Same 50 (u,v) pairs make_insert_workload(seed 42) produces."""
    existing = bench_folder.load_undirected_edge_set(edge_file)
    rng = random.Random(42)
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


def load_edges(edge_file, n):
    rows = [[] for _ in range(n)]
    with open(edge_file) as f:
        for line in f:
            p = line.split()
            if len(p) < 2:
                continue
            try:
                u, v = int(p[0]), int(p[1])
            except ValueError:
                continue
            if 0 <= u < n and 0 <= v < n:
                rows[u].append(v)
                rows[v].append(u)
    return rows


def replay_csr(rows, workload):
    """Replay csr_add_directed: pos=row_ptr[from+1]; shift=m-pos; realloc(+1);
    row_ptr[i]++ for i>from.  Returns totals (shift_elems, sweep_elems,
    reloc_elems, relocation_count)."""
    n = len(rows)
    row_len = [len(r) for r in rows]
    m = sum(row_len)
    cap = m
    shift_elems = 0
    sweep_elems = 0
    reloc_elems = 0
    relocs = 0
    for u, v in workload:
        for fr, to in ((u, v), (v, u)):
            pos = sum(row_len[i] for i in range(fr))     # row_ptr[fr]
            end = pos + row_len[fr]                      # row_ptr[fr+1]
            shift = m - end                              # memmove length
            shift_elems += shift
            m += 1
            if m > cap:
                reloc_elems += m - 1                     # copies old payload
                relocs += 1
                cap = usable_size((m - 1) * 4) // 4
            row_len[fr] += 1
            rows[fr].append(to)
            sweep_elems += n - fr
    return shift_elems, sweep_elems, reloc_elems, relocs


def replay_bcsr(blocks, workload, b):
    """Replay autograph_bcsr_add_edge: dup scan, sorted insert, realloc(+2),
    memmove tail, brow[i]+=2 for i>blk.  Returns same tuple."""
    nb = len(blocks)
    elems = [len(x) for x in blocks]
    total = sum(elems)
    cap = total
    shift_elems = 0
    sweep_elems = 0
    reloc_elems = 0
    relocs = 0
    for u, v in workload:
        for fr, to in ((u, v), (v, u)):
            blk = fr // b
            row = fr % b
            sl = blocks[blk]
            ip = len(sl)
            for i, (lr, _) in enumerate(sl):
                if lr > row:
                    ip = i
                    break
            shift = total - ip
            shift_elems += shift
            elems[blk] += 1
            total += 1
            if total > cap:
                reloc_elems += total - 1
                relocs += 1
                cap = usable_size((total - 1) * 4) // 4
            sl.insert(ip, (row, to))
            sweep_elems += nb - blk - 1
    return shift_elems, sweep_elems, reloc_elems, relocs


def main():
    files = [
        ("erdos_renyi_runs10_updated.csv", "m"),
        ("barabasi_albert_runs10.csv", "m_undirected"),
        ("meh_validate_updated.csv", "m"),
    ]
    for path, mcol in files:
        with open(path, newline="") as f:
            for r in csv.DictReader(f):
                op = r.get("op") or r.get("operation")
                if op != "Insert":
                    continue
                lay = r.get("layout")
                if lay not in ("CSR", "BCSR"):
                    continue
                try:
                    ms = float(
                        (r.get("measured_ns") or r.get("measured_kernel_ns") or "").strip()
                    )
                except ValueError:
                    continue
                if ms <= 0:
                    continue
                g = r.get("graph") or r.get("graph_label") or ""
                n = int(r.get("n") or r.get("n_vertices"))
                m = int(r.get("m") or r.get("m_undirected"))
                ef = cm._resolve_edge_file(g)
                if not ef:
                    continue
                work = gen_workload(ef, n)
                if lay == "CSR":
                    rows = load_edges(ef, n)
                    sh, sw, rl, nr = replay_csr(rows, work)
                else:
                    b = 64
                    rows = load_edges(ef, n)
                    nb = (n + b - 1) // b
                    blocks = [[] for _ in range(nb)]
                    for u in range(n):
                        for v in rows[u]:
                            if v > u:
                                blocks[u // b].append((u % b, v))
                                blocks[v // b].append((v % b, u))
                    sh, sw, rl, nr = replay_bcsr(blocks, work, b)
                perop = ms / K
                tot_bytes = (sh + rl) * 4 + sw * 8
                lm = cm.L
                model_lines = math.ceil(tot_bytes / lm)
                meas_bytes_per_line = perop / max(model_lines, 1)
                print(
                    f"{g:28s} {lay:4s} n={n:6d} m={m:8d} "
                    f"meas/op={perop:9.0f}  "
                    f"shiftB={sh*4:10.0f} sweepB={sw*8:9.0f} relocB={rl*4:10.0f} "
                    f"nreloc={nr:3d}  allB={tot_bytes:11.0f}"
                )


if __name__ == "__main__":
    main()