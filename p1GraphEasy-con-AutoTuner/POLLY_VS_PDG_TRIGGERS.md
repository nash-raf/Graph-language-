# Polly vs PDG / TDG automatic parallelization — trigger research

**Compiler:** p1GraphEasy-con-AutoTuner (`GraphProgram`)  
**Date:** 2026-08-22  
**Harness:** `bench_polly_vs_pdg_triggers.py`  
**Raw run:** `benchmark_logs/polly_vs_pdg_20260822_190905/`

## Executive answer

| Question | Answer |
|----------|--------|
| Do normal PDG DOALL / DOACROSS still run when Polly is linked? | **Yes**, but only on loops Polly did **not** consume. |
| Does Polly replace PDG on affine nests? | **Yes.** Polly runs **first**; with `-polly-parallel` it emits `GOMP_parallel_loop_runtime_start`. Those nests are gone before PDG. |
| Can both fire in one program? | **Yes**, on **different** nests (e.g. Polly on matmul + PDG/outliner on leftover init / non-affine gather). |
| Can both parallelize the **same** nest? | **No** (by pipeline order). |
| Non-affine / CSR: who fires? | Polly: **0** transforms. PDG: still **classifies** DOALL/SEQUENTIAL; outlining often **skips** opaque neighbor walks. |

## Pipeline (why they rarely “combine” on one loop)

```
IRGen → mem2reg/canonicalize
      → Polly O3  (+ -polly-parallel unless GRAPH_POLLY_NO_PARALLEL)
      → Autotuner (representation selector)
      → PDG  (annotates my.loop.parallel = DOALL | DOACROSS | SEQUENTIAL)
      → reconstructParallelIR
      → loop outliner → parallel_for_runtime / _ex
```

Comments in `main.cpp` state this explicitly: after Polly, measured matmul IR drops from hundreds of `polly.*` refs to opaque OpenMP tasks; the outliner then sees **no** original nest.

**Env knobs**

| Env | Effect |
|-----|--------|
| `GRAPH_DISABLE_POLLY=1` | Skip Polly → PDG/outliner own all remaining loops |
| `GRAPH_POLLY_NO_PARALLEL=1` | Polly may still tile/xform; no GOMP; residual loops can still hit PDG |
| (default) | Polly + `-polly-parallel` |

There is **no** `GRAPH_DISABLE_PDG` today; “Polly-only” is the natural outcome when Polly parallelizes every profitable nest.

## Experiment

Seven hand-written `.graph` kernels (N=48 vertices), three configs each:

1. `polly_off` — `GRAPH_DISABLE_POLLY=1`
2. `polly_on` — default Polly + parallel
3. `polly_nopar` — Polly on, no `-polly-parallel`

Detection from IR dumps (`DUMP_LLVM_BC_PRE/POST/PDG/AFTER_OUTLINE`):

- SCoPs: `opt -polly-print-detect` on PRE
- Polly: `polly.*`, `GOMP_parallel*`
- PDG: `parallel.type=DOALL|DOACROSS|SEQUENTIAL`
- Outliner: `parallel_for_runtime` call sites

## Results matrix

| kernel | nature | polly_off | polly_on | polly_nopar |
|--------|--------|-----------|----------|-------------|
| **affine_matmul** | affine triple nest | PDG DOALL + **4× outline** | **Polly GOMP (6)** + residual PDG DOALL + **3× outline** | Polly xform (no GOMP) + PDG DOALL + outline |
| **affine_saxpy** | tiny affine DOALL | PDG DOALL + outline | SCoP seen, **no** Polly xform; PDG DOALL; **no** outline | same as on |
| **affine_prefix** | affine + carried dep | **DOALL + DOACROSS** + outline (doacross mode args) | Polly xform; PDG DOALL only; no outline | SCoP; PDG DOALL; no outline |
| **nonaffine_indirect** | `A[B[i]]` | **0 SCoPs**; PDG DOALL (not outlined) | Polly **NONE**; PDG DOALL | same |
| **nonaffine_branch** | data-dep if | 1 SCoP (Polly can model simple if); PDG DOALL + outline | SCoP, no xform; PDG DOALL | same |
| **csr_neighbor** | `for each neighbor` | **0 SCoPs**; PDG classifies DOALL; **not outlined** | Polly NONE; same PDG | same |
| **mixed_affine_indirect** | matmul + gather | PDG outlines affine parts | **Polly GOMP on affine** + **PDG/outline on residual** | Polly xform + PDG/outline |

### Bucket legend (from harness)

- `POLLY_PAR` — OpenMP from Polly (`GOMP_parallel*`)
- `POLLY_XFORM` — `polly.*` without GOMP
- `POLLY_SCOP_ONLY` / `POLLY_NONE` — detect-only or no SCoP
- `PDG_DOALL` / `PDG_DOACROSS` — metadata after PDG
- `OUTLINED` — `parallel_for_runtime` after outliner

## Case studies (IR-backed)

### 1. Only PDG (Polly off) — affine matmul

`affine_matmul/polly_off`: 0 `polly.*`, 0 GOMP, `parallel.type=DOALL` + `SEQUENTIAL` (inner), **4** `parallel_for_runtime` calls.  
→ Classic Autotuner path when Polly is disabled.

### 2. Only / primarily Polly — affine matmul with Polly on

`affine_matmul/polly_on` **POST**: 409 `polly.*`, **6** GOMP sites (`kernel_polly_subfn*`).  
**AFTER_OUTLINE**: GOMP **still present** (matmul nests), **plus** 3 `parallel_for_runtime` on leftover DOALL fragments (not the Polly nests).  
→ Same *program* shows both runtimes; **not** the same nest.

### 3. DOACROSS from PDG — prefix sum, Polly off

`affine_prefix/polly_off` PDG dump: `parallel.type=DOACROSS` and `DOALL`.  
Outliner emits `parallel_for_runtime(..., i32 1, i32 1)` — doacross mode (`SGPL_LOOP_DOACROSS`).  
With Polly on, the carried loop is transformed by Polly instead; DOACROSS metadata disappears.

### 4. Non-affine — Polly silent, PDG soft

`nonaffine_indirect` / `csr_neighbor`: **0 SCoPs**, **0** Polly transforms regardless of config.  
PDG still attaches DOALL/SEQUENTIAL, but **outliner does not emit** `parallel_for_runtime` (opaque neighbor runtime / unproven profit).  
→ “P1 stuff should trigger” for **classification**; **automatic parallel codegen** may still no-op.

### 5. Combined triggers — mixed kernel

`mixed_affine_indirect/polly_on`: GOMP on the affine product nest **and** `parallel_for_runtime` on a residual loop.  
This is the clean demonstration that Polly + PDG **compose across nests**, not within one nest.

## Practical takeaway for Autotuner + Polly

1. **Affine dense nests** (matmul-shaped, densified motifs): Polly owns parallelization when enabled. Disable Polly (`GRAPH_DISABLE_POLLY=1`) if you want to measure PDG DOALL/DOACROSS on those same nests.
2. **Non-affine / CSR / motif runtime**: Polly stays at 0; PDG is the only parallelization path — and may classify without outlining.
3. **Do not expect** “Polly tiling + PDG DOACROSS on the identical loop.” Choose which engine owns the nest via Polly on/off (or make the nest non-SCoP).
4. **Autotuner representation selection** is orthogonal and still runs between Polly and PDG.

## Reproduce

```bash
cd p1GraphEasy-con-AutoTuner
# ensure GraphProgram rebuilt (build_lowmem.sh) after DUMP_LLVM_BC_PDG hooks
TRIGGER_N=48 python3 bench_polly_vs_pdg_triggers.py
# → benchmark_logs/polly_vs_pdg_*/REPORT.md + summary.csv
```

IR dump env vars (now in `main.cpp`):  
`DUMP_LLVM_BC_PRE`, `DUMP_LLVM_BC_POST`, `DUMP_LLVM_BC_PDG`, `DUMP_LLVM_BC_AFTER_OUTLINE`.
