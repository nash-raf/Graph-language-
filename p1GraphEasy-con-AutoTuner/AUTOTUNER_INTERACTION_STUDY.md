# Interaction Study: Effect Algebra + CleanCut Pipeline vs AutoTuner/ARS

Date: 2026-09-04. Compiler: `/mnt/d/sgpl/Graph-language-/p1GraphEasy-con-AutoTuner/GraphProgram`
(rewrite ON by default; `GRAPH_FRONTIER_REWRITE_OFF=1` = opt-out).
All measurements from the main repo unless noted. No compiler code was modified
for this study (bench binaries = the pre-change compiler saved at
`/mnt/d/sgpl/bench/p1GraphEasy-con-AutoTuner/GraphProgram`, 2026-09-03 20:43).

## 1. What the AutoTuner/ARS is (as observed)

- **AutoTunerPass (compile-time)**: turns `autotuner.traverse`-marked loops
  into region events (`RegionType::Traverse` with a `graphPtr` + an estimated
  per-visit cost); bakes the prediction into the emitted
  `autograph_profile_region_enter(region, kind, layout, predicted_ns)` call.
- **Runtime region machinery**: the region enter/exit calls bracket the loop;
  `g_profile_regions[i]` accumulate `measured_ns`; on exit the report prints
  per-region `predicted_ns` vs `measured_ns`, totals, `pure_kernel_ns`
  (from `autograph_profile_record_kernel_ns`), and injected layout conversions.
- **Layout picking**: driven by the events/cost model; conversions
  SET <-> CSR <-> PCSR/BCSR (`autograph_convert_layout`).

## 2. What the new pipeline does to it (mechanism)

Pipeline order in `runPdgAndOutliner`:

```
GraphFrontierLoweringPass  (CleanCut: detect -> effect algebra -> rewrite / mark sequential)
  -> AutoTunerModulePass   (region events)
  -> PDG (classifies; reads sgpl.frontier.nested.sequential -> hard SEQUENTIAL)
  -> reconstructParallelIR (task split; sequential nests left inline)
```

Consequence chain for every graph-adjacent loop (iterator **or** data-indexed):

1. The pass marks the loop (and its whole round nest)
   `sgpl.frontier.nested.sequential` — this marking is now **unconditional**
   (applied even with rewrite off).
2. PDG hard-classifies it SEQUENTIAL.
3. The AutoTuner's runtime **region versioning** (multi-versioning, see the
   `mv.serial.exit.*` blocks in the final IR) then picks the **serial
   fallback** for the loop — the parallel/region version (the one containing
   `autograph_profile_region_enter/exit`) is never executed.

**Empirical proof (gdb, break on `autograph_profile_region_enter`):**
- baseline (pre-change) binary: region enter fires (region 0, Traverse, seen).
- current binary (default AND `REWRITE_OFF=1`): **zero hits** — the region
  machinery never activates for graph loops.

## 3. Evidence

### 3.1 pagerank_100k (n=100k, 1 edge — degenerate but deterministic)

| build | regions | predicted_ns | measured_ns | pure_kernel | layout |
|---|---|---|---|---|---|
| baseline (pre-change) | region=0 Traverse visits=100001 | 14,666,260 | 15,120,343 | 2.48 ms | CSR |
| current (default) | none | 0 | 0 | 7.97 ms | CSR |
| current (REWRITE_OFF=1) | none | 0 | 0 | 2.41 ms | CSR |

Baseline cost model: predicted 14.67 ms vs measured 15.12 ms — the model was
alive and accurate on the count region. Current: model has no inputs.

### 3.2 pagerank_1m (n=1M, m=2M arcs)

- **baseline: did not complete in >20 minutes** (region-runtime execution of
  the 1M graph loops — the pathology that motivated CleanCut).
- current (default): no regions, `pure_kernel_ns = 124.8 ms` — the measured
  time of the CleanCut steps (recorded inside the step functions).
- current (REWRITE_OFF=1): no regions, `pure_kernel_ns = 1307.7 ms` — the
  serial iterator path through the runtime.

So: the **measurement channel works** (steps record into `pure_kernel`), but
there is no per-region attribution, no prediction, no comparison.

### 3.3 saxpy_1000000 (vertex-enumeration program, NO graph iterator)

| build | regions | wall (12T) |
|---|---|---|
| baseline | region=0 Traverse visits=1000001, predicted 3.52 ms vs measured 12.52 ms | **0.06 s** |
| current (default) | none (profile never even prints) | **0.29 s** |

The unconditional sequential marking (data-indexed vertex writes, per the
agreed rule) makes saxpy-class programs run **serially by default** — a ~5x
wall regression vs the old AutoTuner-driven DOALL, and CleanCut does not
rewrite them (no iterator). The AutoTuner goes dormant exactly where it used
to provide value.

## 4. Does the program use the representation the AutoTuner picks?

- Pagerank (current): yes, trivially — the AutoTuner injects **0 layout
  conversions** everywhere (CSR stays), and the CleanCut steps read the
  canonical `meta->csr_row_ptr/csr_col_idx` (CSR). Consistent.
- **Latent hazard (real, unobserved)**: the SET->PCSR/BCSR conversion path
  (`autotuner_runtime.c` ~line 1124-1155) builds a *temporary* CSR, converts
  it, and **frees it** (`free(tmp_rp); free(tmp_ci);`) — `meta->csr_row_ptr`
  is left NULL/stale. `autograph_build_clean_cut` starts with
  `if (!meta->csr_row_ptr || !meta->csr_col_idx ...) return 0;` and the steps
  then return early — i.e., **if the cost model ever fires and picks
  PCSR/BCSR for a graph that runs CleanCut steps, the kernel silently stops
  updating** (no error). 0 conversions were observed on these programs, so it
  is latent, not active.

## 5. Is the cost-model representation picking still making sense?

- **For graph programs: no.** The model's inputs (region events/predictions)
  are gone — `predicted_ns = 0`, no events, no per-region measurements. The
  dominant cost (the CleanCut step: an O(E), memory-bound partitioned walk)
  has **no event and no prediction** — only raw `pure_kernel` totals. Any
  layout decision would be made on nothing (or on the residual serial loops),
  so the AutoTuner's representation selection is **inert** for graph programs.
- **For non-graph programs**: the pass is a no-op (no iterator, no
  graph-data writes) — the AutoTuner path is unchanged (untested here, but
  the code path is untouched).

## 6. Problems caused / incompleteness (summary)

1. **AutoTuner dormant for graph programs**: regions never execute → no
   profile, no prediction, no layout decisions.
2. **Cost model blind** for the dominant kernel (the steps) → representation
   picking cannot function on graph kernels.
3. **Serial-marking regression** on non-iterator vertex programs (saxpy
   ~5x slower by default; CleanCut does not cover them).
4. **Latent PCSR/BCSR hazard**: if the model ever fires and converts layout,
   CleanCut steps silently no-op (canonical CSR freed).
5. The measurement side (`pure_kernel_ns`) still works and now reflects the
   CleanCut steps — but as an unattributed total.

## 7. Recommendations (no code changed)

1. Feed the steps to the AutoTuner: have each CleanCut step record a region
   event (kind=Traverse, layout=CSR, predicted cost = O(E) partition walk) so
   predicted-vs-measured and layout decisions operate on the real kernel.
2. Either hold a canonical CSR copy for the steps regardless of transient
   layouts (SET->PCSR/BCSR currently frees it), or exclude graphs running
   CleanCut steps from PCSR/BCSR conversion.
3. Reconsider the marking scope for non-iterator vertex programs: an
   identity permutation (`perm[v]=v`) is provably safe (no aliasing) — a
   data-aliasing check could keep saxpy-class programs parallel (or route
   them through a data-keyed CleanCut step) instead of serial-by-default.
4. Note the baseline 1M >20min: the old region path is unusable at scale —
   CleanCut fixes execution; the AutoTuner needs to be re-pointed at the
   steps (item 1) to keep its predictive machinery meaningful.

## 8. Resolution status (implemented 2026-09-04)

- **Items 1 and 2 are done.** CleanCut partitions are now **layout-agnostic**
  (`autotuner_runtime.c`): `autograph_build_clean_cut` builds from the
  canonical CSR when present, otherwise reconstructs it from the static base
  pairs (`graph_get_edge_pairs`), and the source step reads prebuilt
  per-partition slices instead of the transient CSR. The AutoTuner pass
  **annotates both IR versions**: each CleanCut step call receives a paired
  Traverse region (same graph, same totalOps prediction) with enter/exit
  bracketing the call.
- **Validation (main repo):**
  - Layout matrix on 1M x 1M (undirected): CSR/PCSR/BCSR/SET —
    `DEG_MATCH=YES RANK_MATCH=YES` on all four (push + source steps).
  - Directed (1M x 1M and single-edge): all four layouts exact.
  - Scaling (CSR, 20 rounds): 2.59x at 8T, 1.78x at 12T.
  - Determinism: pagerank_100k = 0.000010, pagerank_1m = 0.000000, 1T = 12T.
  - AutoTunerProfile alive again: e.g. pagerank_1m — region 1 (source step,
    visits=1) and region 2 (push step, visits=20), predicted 146.7 ms vs
    measured 140.2 ms on the push region (accurate), totals 293.3 ms predicted
    vs 142.6 ms measured.
- **Fixed bugs found during the work:** type-mismatched `build_canonical_csr_from_base`
  signature (TBAA miscompile, heap corruption), and the source-slice fill used
  `floor(u*P/n)` instead of the exact partition-range inverse
  `floor(((u+1)*P-1)/n)` (first row of each partition mis-assigned → overrun).
- **Outstanding (item 3, saxpy):** deferred by decision — the provenance
  lattice correctly reports non-graph (the loop-var alloca load is Top, not D),
  and the saxpy regression is a runtime decision-layer matter, not the CleanCut
  pass. No action taken.

## 9. Artifacts

- Current compiler + binaries: main repo `GraphProgram`, `ab_on_pagerank_100k`,
  `ab_off_pagerank_100k`, `ab_on_pagerank_1m`, `ab_off_pagerank_1m`,
  `sax_1m`.
- Baseline compiler + binaries: bench `GraphProgram` (2026-09-03 20:43),
  `baseline_pagerank_100k`, `sax_base`.
- Raw logs: `/tmp/ab_profile_out.txt`, `/tmp/baseline_out.txt`,
  `/tmp/saxbase_out.txt`.