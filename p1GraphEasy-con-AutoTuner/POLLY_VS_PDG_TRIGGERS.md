# Polly vs PDG / TDG automatic parallelization — trigger research

**Compiler:** p1GraphEasy-con-AutoTuner (`GraphProgram`)  
**Date:** 2026-08-27 (expanded from 7 → 17 kernels)  
**Harness:** `bench_polly_vs_pdg_triggers.py`  
**Latest run:** `benchmark_logs/polly_vs_pdg_20260827_104442/`  
**Related:** DOACROSS PHI suite (`bench_doacross_phi_fix.py`) — 10 kernels × 2 configs = 20 cases

## Executive answer

| Question | Answer |
|----------|--------|
| Do normal PDG DOALL / DOACROSS still run when Polly is linked? | **Yes**, but only on loops Polly did **not** consume. |
| Does Polly replace PDG on affine nests? | **Yes.** Polly runs **first**; with `-polly-parallel` it emits `GOMP_parallel_loop_runtime_start`. Those nests are gone before PDG. |
| Can both fire in one program? | **Yes**, on **different** nests (e.g. Polly on matmul + PDG/outliner on saxpy / prefix / gather). |
| Can both parallelize the **same** nest? | **No** (by pipeline order). |
| Non-affine / CSR: who fires? | Polly: **0** SCoPs. PDG: often **classifies** DOALL; outlining often **skips** opaque neighbor walks. |
| PDG classifies but nothing outlines? | **Common** with Polly on at small N or opaque loops (`affine_saxpy`, `csr_neighbor`, `reduction_sum`). |
| Polly transforms but no GOMP? | **Yes** — prefix/fib/saxpy+prefix: `POLLY_XFORM` without `GOMP`; PDG still attaches DOACROSS. |
| Neither engine fires? | **Yes** — `dense_ffl_count` (nested if in fill), `bfs_frontier` with Polly on (PDG → SEQUENTIAL). |

## Pipeline (why they rarely “combine” on one loop)

```
IRGen → mem2reg/canonicalize
      → Polly O3  (+ -polly-parallel unless GRAPH_POLLY_NO_PARALLEL)
      → Autotuner (representation selector)
      → PDG  (annotates my.loop.parallel = DOALL | DOACROSS | SEQUENTIAL)
      → reconstructParallelIR
      → loop outliner → parallel_for_runtime / _ex
```

**Env knobs**

| Env | Effect |
|-----|--------|
| `GRAPH_DISABLE_POLLY=1` | Skip Polly → PDG/outliner own all remaining loops |
| `GRAPH_POLLY_NO_PARALLEL=1` | Polly may still tile/xform; no GOMP; residual loops can still hit PDG |
| (default) | Polly + `-polly-parallel` |

## Experiment

**17** hand-written `.graph` kernels (N=48 vertices), **3** configs each → **51** compile-time rows:

1. `polly_off` — `GRAPH_DISABLE_POLLY=1`
2. `polly_on` — default Polly + parallel
3. `polly_nopar` — Polly on, no `-polly-parallel`

Detection from IR dumps (`DUMP_LLVM_BC_PRE/POST/PDG/AFTER_OUTLINE`):

- SCoPs: `opt -polly-print-detect` on PRE
- Polly: `polly.*`, `GOMP_parallel*`
- PDG: `parallel.type=DOALL|DOACROSS|SEQUENTIAL`
- Outliner: `parallel_for_runtime` call sites

## Trigger quadrants (polly_on)

| Pattern | Kernels | What happens |
|---------|---------|--------------|
| **Polly GOMP + PDG + outline** | `combo_triple`, `mixed_affine_indirect`, `affine_matmul` | Matmul nest → GOMP; saxpy/gather/residual → PDG outline |
| **Polly GOMP + PDG, no outline** | `transpose_2d` | Polly parallelizes transpose; PDG metadata on leftovers; too small to outline |
| **Polly xform + PDG DOACROSS, no outline** | `affine_prefix`, `fib_doacross`, `saxpy_prefix_combo` | Polly reshapes carried loop; PDG still marks DOACROSS; outliner skips |
| **Polly SCoP only + PDG DOALL, no outline** | `affine_saxpy`, `stencil_1d`, `reduction_sum`, `nonaffine_branch` | SCoP detected; no GOMP at N=48; PDG classifies; no `parallel_for_runtime` |
| **Polly NONE + PDG DOALL, no outline** | `csr_neighbor`, `nonaffine_indirect`, `edge_only_walk`, `2d_init_only` | 0 SCoPs or no transform; PDG classifies only |
| **Neither parallelizes** | `dense_ffl_count`, `bfs_frontier` (polly_on) | SCoP+ifs or opaque BFS → PDG SEQUENTIAL; no GOMP, no outline |

### polly_off vs polly_on flip (important)

| Kernel | polly_off | polly_on | Lesson |
|--------|-----------|----------|--------|
| `affine_matmul` | PDG DOALL+DOACROSS + **4× outline** | **GOMP=6** + PDG DOALL + 3× outline | Polly steals matmul nest |
| `affine_prefix` | DOACROSS + **outline** | Polly xform; DOACROSS **metadata only** | Polly transforms carried loop; outline drops |
| `fib_doacross` | DOALL+DOACROSS + outline | Polly xform; **DOACROSS only** | Init loop DOALL gone after Polly |
| `csr_neighbor` | PDG DOALL (no outline) | same | Polly never sees CSR |
| `bfs_frontier` | PDG DOALL | **PDG SEQUENTIAL** | Complex control + neighbor opaque |
| `combo_triple` | 6× outline | GOMP=6 + DOALL + DOACROSS + 3× outline | All three engines in one program |

## Full results matrix (51 rows)

See `benchmark_logs/polly_vs_pdg_20260827_104442/REPORT.md` for the complete table.

### Original 7 kernels (unchanged behavior)

| kernel | nature | polly_off | polly_on | polly_nopar |
|--------|--------|-----------|----------|-------------|
| **affine_matmul** | affine triple nest | PDG DOALL+DOACROSS + outline | **GOMP=6** + PDG DOALL + outline | POLLY_XFORM + PDG + outline |
| **affine_saxpy** | tiny affine DOALL | PDG DOALL + outline | SCoP only; PDG DOALL; **no outline** | same |
| **affine_prefix** | carried dep | DOALL+DOACROSS + outline | POLLY_XFORM + DOACROSS md | SCoP + DOACROSS md |
| **nonaffine_indirect** | `A[B[i]]` | 0 SCoPs; PDG DOALL | POLLY_NONE; PDG DOALL | same |
| **nonaffine_branch** | data-dep if | PDG DOALL + outline | SCoP only; no outline | same |
| **csr_neighbor** | `for each neighbor` | 0 SCoPs; PDG DOALL | POLLY_NONE; PDG DOALL | same |
| **mixed_affine_indirect** | matmul + gather | PDG + outline | **GOMP + PDG + outline** | POLLY_XFORM + PDG + outline |

### New 10 kernels (2026-08-27)

| kernel | nature | polly_off | polly_on | Notes |
|--------|--------|-----------|----------|-------|
| **fib_doacross** | `a[i]=a[i-1]+a[i-2]` | DOALL+DOACROSS + outline | POLLY_XFORM + DOACROSS only | Polly removes init DOALL metadata |
| **dense_ffl_count** | dense triple + if-fill | PDG **SEQUENTIAL** | SCoP only; **neither** outlines | if-nests block both engines |
| **transpose_2d** | affine 2D copy | DOALL+DOACROSS + outline | **GOMP=4** + DOALL; no outline | Polly owns transpose nest |
| **bfs_frontier** | array BFS + neighbors | PDG DOALL | **SEQUENTIAL only** | Opaque frontier + neighbor walk |
| **reduction_sum** | `sum += A[i]` | PDG DOALL | SCoP + DOALL; no outline | Reduction not outlined at N=48 |
| **saxpy_prefix_combo** | saxpy + prefix | DOALL+DOACROSS + outline | POLLY_XFORM + both md | No matmul; Polly xform only |
| **combo_triple** | matmul+saxpy+prefix+gather | 6× outline | **GOMP=6** + DOALL + DOACROSS + outline | Same as DOACROSS suite case 10 |
| **stencil_1d** | Jacobi smooth | PDG DOALL + outline | SCoP + DOALL; no outline | Carried stencil; small N |
| **edge_only_walk** | `for each edge` | PDG DOALL | POLLY_NONE + DOALL | Like csr but single loop |
| **2d_init_only** | double init nest | DOALL+DOACROSS + outline | POLLY_NONE + DOALL | 0 SCoPs at N=48; no GOMP |

## Bucket legend

- `POLLY_PAR` — OpenMP from Polly (`GOMP_parallel*`)
- `POLLY_XFORM` — `polly.*` without GOMP (tiling/scheduling on carried loops)
- `POLLY_SCOP_ONLY` — SCoP detected, no transform emitted
- `POLLY_NONE` — Polly enabled but 0 SCoPs
- `PDG_DOALL` / `PDG_DOACROSS` — `my.loop.parallel` metadata after PDG
- `OUTLINED` — `parallel_for_runtime` after outliner
- Combined `POLLY_*+PDG_*` on **mixed** kernels = **different nests**, not the same loop

## Practical takeaway

1. **Affine dense nests** (matmul, transpose): Polly GOMP when enabled; disable Polly to force PDG DOALL/DOACROSS on same nest.
2. **Carried recurrences** (prefix, fib): Polly may xform without GOMP; PDG DOACROSS metadata survives; outlining often drops with Polly on.
3. **Graph / CSR / BFS**: Polly silent; PDG may classify DOALL but not outline; BFS can fall to SEQUENTIAL with Polly on.
4. **Nested if in SCoP** (`dense_ffl_count`): SCoP detected but neither Polly nor PDG parallelizes — SEQUENTIAL.
5. **Small loops at N=48**: SCoP without GOMP, PDG classifies without outline — common for saxpy/reduction/stencil.
6. **Autotuner** (CSR/PCSR/BCSR/SET) runs between Polly and PDG — orthogonal to this matrix.

## Reproduce

```bash
cd p1GraphEasy-con-AutoTuner
TRIGGER_N=48 python3 bench_polly_vs_pdg_triggers.py
# → benchmark_logs/polly_vs_pdg_*/REPORT.md + summary.csv
```

DOACROSS-only regression (20 cases):

```bash
python3 bench_doacross_phi_fix.py
# → benchmark_logs/doacross_fix_*/REPORT.md
```
