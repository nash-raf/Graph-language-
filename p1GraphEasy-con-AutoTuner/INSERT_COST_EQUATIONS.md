# Insert-Cost Equations (All Layouts) + SET Hash Setup + Memory Penalty

This document records the exact insert-cost equations of the current model,
identical in `AutoTunerPass.cpp` (`insertCost`, `insertSetupCost`,
`hashInsertCost`, `memPenalty`/`curvePerLine`, `reallocCost`) and in
`cost_model.py` (`insert_cost_csr/pcsr/bcsr/set`, `insert_setup_cost_set`,
`hash_insert_cost`, `mem_penalty`, `realloc_cost`).  All costs are
**nanoseconds per undirected `graph_add_edge`** unless a section says otherwise.

## 1. Notation

| Symbol | Calibration key / formula | Meaning |
|---|---|---|
| `L` (64) | `L` | cache-line size, bytes |
| `t` | `t` | random-access (redirection) latency per cache line, ns |
| `T` | `T` | bandwidth-limited sequential read cost per line, ns |
| `Tm` | `Tm` | cache-resident per-line memmove cost (read+write), ns |
| `R` | `R` (10000.0) | realloc / page-remap policy cap, ns |
| `h_cache`, `h_dram` | `h_cache`, `h_dram` | ns per hash insert at cache-resident / DRAM table sizes |
| `h` (legacy) | `h` | fallback for `h_dram` when absent |
| `lo`, `hi` | `ramp_lo`, `ramp_hi` | cache-residency ramp band, bytes |
| `P` | `P` | DRAM saturation multiplier (ramp fallback only) |
| `curve` | `memmove_curve` | measured log-spaced `[ws_bytes, per_line_ns]` sweep |
| `n`, `m` | graph | vertex count, undirected edge count |
| `d` | `2m/n` | average undirected degree |
| `g` | `kPcsrExpansionFactor = 2` | PCSR expansion factor |
| `b` | `kBcsrBlockSize = 64` | BCSR block size |
| `n_b` | `ceil(n/b)` | number of BCSR block rows |
| `csrFrac` | compile-time `estimateShiftFractions` | exact CSR memmove tail multiplier (1.0 = blanket half-array) |
| `bcsrFrac` | compile-time | exact BCSR memmove tail multiplier |

## 2. Memory penalty — the curve form

`mem_penalty(w)` is the per-line cost multiplier for shifting/overlap-copying a
working set of `w` bytes, relative to the cache-resident floor `Tm`.

With the measured curve present (≥ 2 anchors):

```
curve_per_line(w):                                   # ns per shifted cache line
    w <= curve[0].w   : return curve[0].per_line
    w >= curve[-1].w  : return curve[-1].per_line
    else:  f  = ln(w / w_i) / ln(w_{i+1} / w_i)      # w_i <= w <= w_{i+1}
           return per_line_i + (per_line_{i+1} - per_line_i) * f

mem_penalty(w) = curve_per_line(w) / Tm
```

Without the curve, fall back to the log-linear ramp over the measured
`[ramp_lo, ramp_hi]` band (itself falling back to `[LLC/2, 2·LLC]`):

```
ramp_penalty(w, lo, hi, P):
    w <= lo : return 1.0            # fully cache-resident
    w >= hi : return P              # fully DRAM-bound
    else    : 1.0 + (P-1)·ln(w/lo)/ln(hi/lo)

mem_penalty(w) = ramp_penalty(w, lo, hi, P)
```

## 3. SET hash-table setup cost

The lazy `EdgeHashMap` the first timed SET insert must bulk-build:

```
static_hash_table_bytes(m) = next_pow2(2·m + 1) · 24        # open-addressing, 24 B/cell

hash_insert_cost(pairs):                    # per-pair build cost, size-aware
    table_bytes = next_pow2(2·pairs + 1) · 24
    h_dram  = h_dram > 0 ? h_dram : h       # legacy single-point fallback
    if h_cache <= 0:          return h_dram
    table_bytes <= lo:        return h_cache                 # fits in LLC
    table_bytes >= hi:        return h_dram                  # DRAM-resident
    else:  h_cache + (h_dram - h_cache) · ln(table_bytes/lo) / ln(hi/lo)

static_hash_build_cost(m) = hash_insert_cost(m) · m             # per pair × pairs

realloc_cost(bytes, objs):
    bytes <= 0            : 0
    copy  = ceil(bytes/L) · Tm · mem_penalty(bytes)             # stream+write
    return min(copy, R) · objs                                  # cap at page-remap

insert_setup_cost(SET) = static_hash_build_cost(m)
                       + realloc_cost(static_hash_table_bytes(m), 1.0)
```

This one-time cost is charged **once per region** (not multiplied by ops): the
built hash survives all later SET inserts.  The other O(n+m) first-insert work
(canonical edge-count rescan `c·m`, node select-cache rebuild `⌈4n/L⌉+⌈n/(8L)⌉`
and the rebuild-sets local hash) runs inside the transition itself and is
billed in the conversion cost, not here.

## 4. Per-layout insert (one undirected edge = 2 directed inserts)

### 4.1 CSR

```
cLocate  = 2·t + ceil(4·d / L)·T           # row_ptr[from+1] read + row scan (rel. d fits)
prefixBytes = 8·n                           # n int64 prefix-sum R-M-W entries
cWrite   = 1·t + ceil(prefixBytes/L)·2T·mem_penalty(prefixBytes)   # dependent L→S, 2T/line
moveBytes= 4·m·csrFrac                       # 2 dirs, 4B/col, exact tail expected cols
cMove    = ceil(moveBytes/L)·Tm·mem_penalty(8·m + moveBytes)
cRealloc = R + ceil(4·m/L)·T                # growth write + cap

insert(CSR) = cLocate + cWrite + cMove + cRealloc
```

### 4.2 PCSR

```
gU    = g·d                                  # expected padded row span per vertex
cLocate = 2·t + ceil(4·gU/L)·T               # random read + gap scan of full row
cWrite  = T                                  # one gap-slot write
insert(PCSR) = cLocate + cWrite              # no shifting, no realloc
```

### 4.3 BCSR

```
dirs = 2;  b = 64;  n_b = ceil(n/b)
bcol bytes = 16·m                             # (local_row, col) i32 pairs, 2m directed edges

cLocate = 4·t + dirs·ceil(8·b·d/L)·T          # dup + insert-point scans of block row
cWrite  = ceil(4·n_b/L)·2T                    # brow prefix R-M-W, E[n_b - blk] = n_b/2, 2T/line
moveBytes = dirs·8·bcsrFrac·m                 # 8 B/edge-pair × exact expected tail
cMove    = ceil(moveBytes/L)·Tm·mem_penalty(moveBytes)
cRealloc = realloc_cost(16·m, dirs)           # two realloc events

insert(BCSR) = cLocate + cWrite + cMove + cRealloc
```

`bcsrFrac` = E[blockSuffix(blk+1) + same-block suffix] / m, computed from the
real degree vector. Deliberately no degree piecewise heuristic — the exact
expected tail replaces any append shortcut / second-shift probability / cutoff.

### 4.4 SET (steady state)

```
cLocate = 5·t        # 2× roaring_bitmap_add(nodes) + static hash probe + extra hash probe
cWrite  = 2·t + 5·T  # extras append + live flag + extra hash insert + O(1) counts
insert(SET) = cLocate + cWrite
```

The expensive O(n+m) SET work is entirely one-time → §3 (setup) and the
conversion cost.

## 5. Conversion (layout A→B), for reference

`conversionCost(A,B) = readCost(A) + metaBuildCost(B) + payloadBuildCost(B)`,

each `read`/`build` step = `ceil(c_bytes/L)·T` with:

| Step | CSR | PCSR | BCSR | SET |
|---|---|---|---|---|
| `readCost` | `8(n+1)+4m` | `8(n+1)+4g·m` | `4·n_b + 16m` | `8m` |
| `metaBuildCost` | `8(n+1)` | `8(n+1)` | `4·n_b` | `hash_cost(m)·m + c·m + ⌈4n/L⌉+⌈n/(8L)⌉` |
| `payloadBuildCost` | `4m` | `4g·m` | `16m` | `8m` |

`insertCost` is written as `insertCost(layout, n, m, csrFrac, bcsrFrac, hw)`;
Python mirrors take optional pinned `csr_frac` / `bcsr_frac` arguments.