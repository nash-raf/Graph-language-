# Analytic Reuse-Distance Estimation and Cache Hit Fractions for BCSR/CSR Insertion

Technical report on the replay-free (offline) estimator in `analytic_rd.py`, its
theory, its mirror implementations in the compiler (`IRGenVisitor.cpp`,
`AutoTunerPass.cpp`) and cost model (`cost_model.py`), and its validation gates.

---

## 1. Theory

### 1.1 Reuse distance as a cache-miss predictor

The model is built on the classic **stack distance (reuse distance)** measure,
defined authoritatively in `rd_hist.py`:

> `RD(line ℓ at reference τ)` = the number of *distinct* cache lines referenced
> after the previous occurrence of `ℓ` and before the current occurrence.
> First references get `RD = INF` (compulsory / cold-miss bin).

Under an LRU replacement policy with a cache of `C` lines, a line is still
resident when it is re-referenced iff the number of distinct lines referenced
between the two references is strictly less than `C`. Hence the cache hit
fraction at capacity `C` is the empirical tail count

```
h(C) = #{references with RD < C} / N
```

This is the cap-based hit-ratio identity: it converts a *distribution* of reuse
distances (which is what the memory system actually cares about) into a scalar
hit fraction, without modelling associativity, replacement policy details, or
set conflicts. The estimator applies it at two capacities simultaneously —
the L2 and L3 sizes — giving `h2 = h(L2_LINES)` and `h3 = h(L3_LINES)` with

```
L2_LINES = 1,310,720 / 64 = 20,480
L3_LINES = 12,582,912 / 64 = 196,608     (i5-1235U, rd_hist.py:64-65)
```

### 1.2 The workload model

Each "op" is one directed insertion of a `(u, v)` edge into a BCSR (or CSR)
graph. A directed edge insertion into vertex `u`:

1. locates block `k = u // 64` and the insert position `ip` inside the block's
   row (binary-search position among the block's local rows),
2. **moves** the pair-array tail: everything from the block-row start `a_t`
   down to the end of `bcol` is shifted by `memmove` (one line may be
   split/rewritten at the insert position),
3. **scans/rewrites** the block-row region touched,
4. **updates `brow`**: the row-pointer prefix from block `k` to the end is
   read-modified-written (suffix R-M-W),
5. touches the **struct/header** line (graph metadata).

The workload process is assumed to be:

- **Uniform vertex selection** — every vertex equally likely to be inserted
  into, so the block is drawn iid with `P(blk = k) = verts_k / n` (block weight
  proportional to its vertex count),
- **Within-block insert position** distributed according to the block's degree
  vector (rows with more edges are likelier insert positions; `E[ip]` is
  computed exactly from the degree vector),
- **Insertion drift** — every op in a block *below* `k` shifts block `k`'s row
  start down the array by 2 int32 (BCSR pairs are 2 int32), i.e. by 1/8 cache
  line. The expected drift after `t−1` ops is `(t−1) · cumsum(P)/8`.

### 1.3 Why the reuse distance is a per-op constant

The key structural fact (documented in `rd_hist.py:24-51`): an op's touched
lines in each array form a **contiguous interval** — `[a_t, b_t]` in `bcol`
(and `[a_br_t, b_br]` in `brow`). Consequently the union of the intervals
touched by *any* window of intervening ops is again a single interval
`[min a_w, b]`. The number of distinct lines standing between two references
to a line therefore has a closed form in terms of:

- the interval endpoints of the current op (`a_t`, `b_t`),
- the previous op that covered the line (`p = t_prev(ℓ)`),
- the running minimum of block-row starts (`min_a_w`) over the window,
- the fixed brow maximum `b_br`.

Because the reference events for the lines of block `k` are "an insertion
lands in block `k`", the distance between consecutive references to block
`k`'s row lines is the span swept by all intervening ops: exactly

```
RD_k(t) = (b − a_t(k,t)) + (b_br − a_br[k]) + 1        (cache lines)
```

`(b − a_t(k,t))` is not "distance to the end of the array" — it is the number
of pair-array lines that every intervening op re-sweeps (the memmove touches
each of them), which by definition is the number of distinct lines between the
two references. The `brow` suffix term and the `+1` (struct/header line)
account for the other two things touched inside that window.

---

## 2. Equations

### 2.1 BCSR geometry

`m` = number of undirected edges. Each edge is stored twice (both directions)
as a 2-int32 pair → `T0 = 4m` int32 in `bcol`. With 16 int32 per 64-byte line
and base misalignment `o` (element units, `o = base_mod64 // 16`):

```
b        = (4m + 1 + o) // 16      array write-end line (append convention;
                                   mirrors the oracle b_t = (T + 1 + o)//16)
brow_line[k] = (brow[k] + o) // 16 block-row start line (brow = int32 prefix)
row_lines[k] = ceil((brow[k+1] − brow[k]) / 16)
b_br     = (nb − 1 + o_br) // 16   brow max line (brow is int32)
a_br[k]  = (k + 1 + o_br) // 16    brow line of block k's prefix entry
```

### 2.2 The per-op reuse distance

```
a_t(k,t) = brow_line[k] + (t−1) · drift,      drift = cumsum(P) / 8
RD_k(t)  = (b − a_t(k,t)) + (b_br − a_br[k]) + 1
```

The implementation (which matches the docstring form after taking the
expectation over the covering block `p`) writes this as

```
RD_k(t) = (b − brow_line[k]) + bw_const − (t−1)·drift
bw_const = b_br + 2 − E[a_br[p]]
```

where `E[a_br[p]]` is the expected brow line of the last covering op `p`,
weighted by block col-line spans:

```
E[a_br[p]] = Σ_j rl[j]·cumP_j·(Σ_{i≤j} P_i·a_br[i] / cumP_j) / Σ_j rl[j]·cumP_j
```

### 2.3 Per-op line weights (given blk = k)

Each op touches `w_k` lines per access class:

```
w_scan[k] = row_lines[k]                 dup-scan region of the block row
w_move[k] = b − ip[k] + 1                memmove tail from the insert position
w_brow[k] = b_br − a_br[k] + 1           brow suffix R-M-W
w_struct[k] = 1                          struct/header line
ip[k] = E[insert-position line | blk=k] = (brow[k] + 2·E[ip_pair] + o)//16
```

### 2.4 First-reference (INF) lines

Lines below the running minimum `M_{t−1}` of block-row starts seen so far have
never been touched by any prior sweep → first reference → `RD = INF`, charged
to DRAM. Their *expected* count is computed exactly from order statistics of
the iid block-start draws, via the survival function over distinct values:

```
S(v) = P(start > v)                     per-draw survival
P(M_{t−1} = x_j) = S_j^{t−1} − S_{j+1}^{t−1}     (order statistic pmf)
INF_t(k) = E[(M_{t−1} − y_k)⁺]          expected first-reference lines
```

with `y = ip` for the move class, `y = bl` (with `bl_end = b + 1` complement)
for scan, `y = a_br` for brow. Struct has no INF region. At `t = 1` every line
is first-reference (`inf_*[0] = w`).

### 2.5 RD → tiers → hit fractions

Per op `t` and class, with `rd_t = RD_k(t)`:

```
l2   = w · 1[rd_t <  L2_LINES]
l3   = w · 1[L2_LINES ≤ rd_t < L3_LINES]
dram = w · 1[rd_t ≥ L3_LINES]
```

INF lines are the lowest-start (highest-RD) lines; they are subtracted from L2
first, then L3, and always counted to DRAM:

```
t2 = max(l2 − ei, 0),  rem = max(ei − l2, 0),  t3 = max(l3 − rem, 0),  td = dram + ei
```

Aggregation over the workload (expectation over blocks and ops):

```
h_C(class) = (1/N) Σ_t Σ_k P(blk=k) · (w_k − INF_t(k)) · 1[RD_k(t) < C]
```

```
c[i] = Σ_t Σ_k P(k) · tier_i(k, t)          i ∈ {L2, L3, DRAM}
h2   = c[L2] / N_s,   h3 = (c[L2] + c[L3]) / N_s,   N_s = Σ c
```

The compile-time payload is per directed insert: `(N_s/n_ops, h2, h3)` per
class — 12 doubles for the 4-class BCSR payload.

### 2.6 Final cost model

```
T = Σ_s N_s [ h_{2,s}·r_{s,L2} + (h_{3,s} − h_{2,s})·r_{s,L3} + (1 − h_{3,s})·r_{s,DRAM} ]
```

with measured per-class per-tier rates `r_{s,tier}` (ns/line) from
`class_calib.c → class_calib.json`: `seq` (scan), `rmw` (move/brow),
`rand` (struct).

### 2.7 CSR variants

CSR geometry: `col_idx` int32 with `T0 = 2m` directed cols, `line(i) = (i+o)//16`,
end line `b = (2m+o)//16`; `row_ptr` int64 `(n+1)`, `line(i) = (i+o_rp)//8`,
max line `b_rp = (n+o_rp)//8`. Drift is halved: `drift = cumsum(P)/16`
(1 int32 per op). Three classes (no scan):

```
move   : RD_k(t) = (b − bl[k]) + bw_const − (t−1)·drift
brow   : RD_k(t) = (b − bl[k]) + (b_rp − a_rp[k] + 2) − (t−1)·drift
struct : RD_k    = (b − ip[k] + 1) + (b_rp − a_rp[k] + 1)        (no drift)
```

Two deliberate deviations from the strict window union, both justified by
exact-replay agreement: brow uses the block's *own* suffix
`(b_rp − a_rp[k] + 2)` (the global window would push every brow line past L2
on large graphs, while the replay keeps recently-covered lines in L2), and
struct's RD is the op's own move+brow traffic (the header line is
re-referenced every op with only its own traffic in between). INF machinery
is identical, with survival over `ip` (move) and `a_rp` (brow).

---

## 3. Technical implementation

### 3.1 `analytic_rd.py` — the replay-free estimator

- `block_structure_from_edges(edge_file, o, o_br)` — single pass over the edge
  file building the degree vector, block prefix `brow`, and all line geometry
  (`brow_line`, `row_lines`, `ip`, `b`, `b_br`). This is the only graph input.
- `expected_ip(st)` — exact `E[insert-position line | blk]` from the degree
  vector (uniform within-block vertex draws).
- `AnalyticBCSR.__init__` — builds `P`, `drift = cumsum(P)/8`, the
  brow-window constant `bw_const = b_br + 2 − E[a_br[p]]`, `rd0 = b − bl + bw_const`,
  per-class weights `w`, and the survival functions.
- `_min_excess_matrix(xs, S, y, n_ops)` — closed-form `E[(M_{t−1} − y)⁺]`
  for every op position `t` via the order-statistic pmf
  `P(M = xs_j) = S_j^{t−1} − S_{j+1}^{t−1}` (prefix/suffix sums over the
  sorted distinct starts; no Monte Carlo).
- `expected_tiers()` — the per-op/tier accumulation of §2.5, vectorized over
  blocks, summed over `t = 1..n_ops` with the drift shrinkage and INF
  subtraction order (L2 first, then L3).
- `cost(rates)` / `per_class_per_dir()` — `(N_s, h2, h3)` per class per
  directed insert and the total predicted ns.
- `bootstrap_h2_sd(cls, n_draws=3000)` — sampling-noise characterization of a
  single 100-op realized workload (independent iid block draws).
- `AnalyticCSR` — the CSR mirror with the §2.7 geometry and RDs, same INF
  machinery, `rd0_brow` and `struct_rd` handled as separate RD vectors.

### 3.2 `rd_hist.py` — thresholds, oracle, and single source of truth

- `L2_LINES`/`L3_LINES` — the only place the capacity thresholds live.
- `rd_to_tier`, `hit_ratios`, `hit_ratios_from_tiers` — the only place RD
  histograms / tier counts are converted to `(h2, h3)`.
- **Exact oracle** (ground truth): `per_op_bcol_range` (`b_t = (T+1+o)//16`),
  `iter_op_line_rd` (the per-line closed form §1.3), `exact()` — the
  materialized ordered line-reference stream reduced by a Fenwick tree into an
  exact RD histogram (stack-distance computation), and `csr_class_counts` for
  CSR.

### 3.3 `trace_gen.py` — trace generation

Builds the BCSR/CSR block structure from the edge file, replays the kernel's
insert sequence (`replay_adds`: `start/end/ip/T/blk` records with exact
brow/bcol updates), and probes the real glibc base alignment
(`probe_base_mod64`) so the oracle runs with the actual `o` of the process.

### 3.4 `access_class_cost.py` — oracle cost

Maps each op's touched lines to classes (`line_mask` over `[start,end]`,
`[ip, T+2]`, brow suffix, struct), tiers them via `rd_hist.rd_to_tier`, and
produces per-class tier counts for the replay path.

### 3.5 `cost_model.py` — production integration

`_CLASS_TIERS` caches `ard.class_tiers_from_edges(ef, o=0, o_br=0, n_ops=100)`
per graph; the estimator feeds either the `hybrid` model (structural equations
plus class-tier weighted per-line rates — the production default mirroring the
pass) or the `aware` model (`mem_penalty_cache_aware`: penalty evaluated at
`min(W, L2)`, `min(W, LLC)`, `W` weighted by `h2, h3−h2, 1−h3`). The
`class_tier` mode is the diagnostic standalone model. Falls back to the legacy
footprint-only equation when no edge file exists.

### 3.6 Compiler mirror — `IRGenVisitor.cpp` / `AutoTunerPass.cpp`

`estimateClassTiers(n, m, sourceDir, edgeFileName)` re-implements the entire
analytic estimator in C++ at compile time: degree pass over the edge file,
block prefix, `P`, `drift = cumsum(P)/8`, `expected_ip`, the weighted
`E[a_br[p]]`/`bw_const`, `rd0`, the survival-based INF matrix over `nOps = 100`,
and per-class `(N_s, h2, h3)` — with the compile-time convention `o = o_br = 0`.
The 12-double payload is emitted as the `autotuner.class_tiers` module
metadata (and `autotuner.class_tiers_csr` for CSR via `estimateCsrClassTiers`),
consumed by the runtime for per-region insertion-cost prediction. If the edge
file is unavailable the pass degrades to the legacy equation.

---

## 4. Validations

### 4.1 `validate_analytic.py` (BCSR)

Per graph, per class, compares the **exact replay oracle** (trace →
`iter_op_line_rd` → `rd_to_tier` → per-class tier counts, via
`access_class_cost.per_graph_cost`) against the analytic `expected_tiers`:

- per-class `N_exact` vs `N_analytic`, `h2`, `h3` (with `bootstrap_h2_sd`
  printed as the ± expected sampling noise of a single 100-op workload),
- a structural check that `brow_line[blk]` from the edge file equals the
  trace's per-op start lines (validates the drift-free geometry),
- `T_replay` vs `T_analytic` vs **measured** kernel ns from
  `real_world_runs10_merged.csv`, reporting median `|ratio − 1|` for both and
  the Spearman correlation of analytic per-op cost against measured cost.

### 4.2 `validate_csr_analytic.py` (CSR)

Same comparison for CSR with a Monte-Carlo oracle: because a single 100-op
workload is a *noisy realization* of the analytic expectation (high-variance
insert-position distribution), the oracle is the mean over `N_SEEDS = 8`
independent replay seeds, and the gate tolerances scale with the Monte-Carlo
standard error:

```
N_TOL = 2.0 lines/op,  H_TOL = 0.02 absolute,  SE_K = 3.0 (tolerance + 3·SE)
```

Gates per-class agreement *before* comparing `T_analytic` to measured CSR
insertion time (prevents cancellation of errors).

### 4.3 Result summary

Across the real-graph suite the analytic estimator reproduces the exact
replay per-class tier counts within its sampling noise, tracks measured
insertion time with median |ratio−1| near the replay oracle's, and preserves
rank correlation across graphs (Spearman ρ) — i.e. the autotuner's
layout-choice decision is unaffected by swapping the replay for the analytic
payload.

---

## 5. Limitations

1. **Workload-process assumptions.** Uniform vertex selection (no
   degree-biased insertion), iid block draws (no temporal correlation between
   consecutive ops), and degree-vector insert positions. A workload that
   violates these — e.g. hot-vertex insertion streams — can deviate from the
   expectation; the `bootstrap_h2_sd` quantifies only the iid sampling noise,
   not model bias.
2. **Compile-time `o = 0` convention.** The production payload and
   `cost_model.py` assume 64-byte-aligned bases; validation scripts feed the
   *probed* runtime misalignment. The same graph can shift hit rates purely
   from base alignment.
3. **Fixed workload length** `n_ops = 100` (50 undirected inserts) in the
   analytic payload and C++ mirror; drift shrinkage and INF accumulation are
   extrapolated from this window.
4. **Machine-specific constants and rates.** `L2_LINES`/`L3_LINES` and the
   calibrated per-class rates (`seq`/`rmw`/`rand`, `class_calib.json`) are
   measured on the i5-1235U; transferring to another machine requires
   re-calibration.
5. **INF expectation vs realization.** The analytic INF count is an
   expectation over order statistics; a realized workload's INF bin has
   variance (bounded by the bootstrap SD).
6. **Convention artefacts.** The first op is charged entirely to DRAM
   (all-first-reference), struct's first op uses a tier-2 convention, and the
   `+1` end-line convention (`b = (4m+1+o)//16`) overcounts by one line when
   the array ends exactly on a line boundary (aligned `m ≡ 0 mod 4`).
7. **Class approximations.** BCSR struct RD is modeled as an op-local
   constant (no window/drift dependence); CSR brow RD deliberately uses the
   block's own suffix rather than the global window — both are justified
   empirically by exact-replay agreement but are not exact under the strict
   RD definition.
8. **Scan class is BCSR-only** (dup-scan absent in CSR); the CSR payload
   emits `scan = (0, 0, 0)` for consumers expecting the uniform 4-class
   payload.

---

## Appendix: symbol table

| Symbol | Meaning |
|---|---|
| `m` | undirected edges |
| `n` | vertices |
| `nb` | 64-vertex blocks |
| `P(k)` | `verts_k / n`, block-selection probability |
| `o`, `o_br`, `o_rp` | base misalignment in element units (`base_mod64 // 16` int32, `// 8` int64) |
| `brow_line[k]`, `b_br`, `a_br[k]` | BCSR brow line geometry |
| `bl[k]`, `b_rp`, `a_rp[k]`, `rl[k]` | CSR row-pointer/col geometry |
| `a_t(k,t)` | block-row start line incl. drift |
| `drift` | `cumsum(P)/8` (BCSR), `cumsum(P)/16` (CSR) |
| `bw_const` | `b_br + 2 − E[a_br[p]]`, expected brow window constant |
| `w_scan/move/brow/struct` | per-op class line weights |
| `INF_t(k)` | `E[(M_{t−1} − y_k)⁺]`, expected first-reference lines |
| `h2`, `h3` | hit fractions at L2 / L3 capacity |
| `r_{s,tier}` | calibrated ns/line rate for class `s` at a tier |