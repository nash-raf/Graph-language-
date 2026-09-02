# CSR and BCSR Insert-Cost Equations

Current production model (mirrored exactly by `AutoTunerPass.cpp` and
`cost_model.py`; parity verified at worst rel ~1e-9).

## Notation

| symbol | meaning | default |
|---|---|---|
| `n` | vertices | |
| `m` | undirected edges (directed = `2m`) | |
| `d = 2m/n` | average directed degree | |
| `L` | cache-line size (bytes) | 64 |
| `t` | random-access (redirection) latency | 1.0 |
| `T` | sequential cache-line transfer cost | 1.0 |
| `R` | realloc cap (page-remap cost), ns | 10000 |
| `Tm` | per-line cost of cache-resident overlapping memmove | 2.5 |
| `P` | DRAM-bound memmove penalty vs cache-resident | 4.0 |
| `b = 64` | BCSR block size; `nb = ⌈n/b⌉` | |
| `csrFrac` | expected CSR col_idx memmove-tail fraction (degree vector) | 1.0 |
| `bcsrFrac` | expected BCSR bcol memmove-tail fraction (degree vector) | 1.0 |
| `p(W)` | `memPenalty(W) = curvePerLine(W)/Tm` — measured memmove per-line cost at footprint W (log-log interpolation of `memmove_curve`; fallback `Tm·ramp(W, ramp_lo, ramp_hi, P)`) | |
| `h2_s, h3_s` | Sen/Wood (RD/MRC) residency fractions of class `s`: `h2 = P(RD < L2)`, `h3 = P(RD < L3)`, INF = miss | |

## Cache-aware penalty (mode `aware`)

```
F(W, h2, h3) = h2·p(min(W, L2_bytes)) + (h3 − h2)·p(min(W, LLC_bytes)) + (1 − h3)·p(W)
```

- `L2_bytes = 1,310,720` (1.25 MiB), `LLC_bytes` from `hw_calib.json` (12 MiB).
- Limits: `h3=0 → p(W)` (exact legacy); `h2=1 → p(min(W,L2))` (cache-resident floor); `W ≤ L2 → p(W)`.
- Modes: `AUTOTUNER_CACHE_MODEL ∈ {legacy, aware, class_tier}`; `legacy` uses `p(W)` everywhere; `class_tier` replaces the whole memory terms with the experimental class × residency rate model (diagnostics only).

## CSR — per undirected add (2 × `csr_add_directed`)

```
C_CSR = C_locate + C_write + C_move + C_realloc

C_locate  = 2t + ⌈d·4/L⌉·T                          (row_ptr[from+1] read + scan)
C_write   = t + ⌈8n/L⌉·2T·Penalty_brow(8n)          (row_ptr[from+1..n] prefix R-M-W,
                                                      n int64 entries, 2T/line)
C_move    = ⌈4m·csrFrac/L⌉·Tm·Penalty_move(8m + 4m·csrFrac)
                                                      (col_idx tail memmove, 2 dirs × ~m/2 cols × 4 B)
C_realloc = R + ⌈4m/L⌉·T                             (col_idx growth write + realloc cap)
```

with

```
Penalty_brow(W) = F(W, h2_brow, h3_brow)   in aware mode, else p(W)
Penalty_move(W) = F(W, h2_move, h3_move)   in aware mode, else p(W)
```

(`h2/h3` from the `autotuner.class_tiers_csr` payload; absent the payload → `p(W)`.)

## BCSR — per undirected add (2 × `autograph_bcsr_add_edge`)

```
C_BCSR = C_locate + C_write + C_move + C_realloc

C_locate  = 4t + 2·⌈8bd/L⌉·T                        (2 dirs × (2 brow reads + block-row scan))
C_write   = ⌈4·nb/L⌉·2T                              (brow prefix R-M-W, nb int32 entries — no penalty, unchanged)
C_move    = ⌈16m·bcsrFrac/L⌉·Tm·Penalty_move(16m·bcsrFrac)
                                                      (bcol tail memmove, 2 dirs × 8 B per shifted pair)
C_realloc = min(⌈16m/L⌉·Tm·p(16m), R)·2              (bcol backing = 16m B, ×2 dirs)
```

with

```
Penalty_move(W) = F(W, h2_move, h3_move)   in aware mode, else p(W)
```

(`h2/h3` from the `autotuner.class_tiers` payload; absent the payload → `p(W)`. `C_write` intentionally carries no `memPenalty`, so no correction is applied there.)

## Region level

```
operationCost(R, L) = setup + H·totalOps·(fT·uTrav(L) + fI·uIns(L))
```

where `uIns` is the per-add cost above and `setup` covers one-time lazy init (e.g., SET hash build) per region. Traversal, conversion, and ARS/DP equations are unchanged by the cache-aware penalty.
