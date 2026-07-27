# Autotuner Comparison: `parallel` vs `p1GraphEasy-con-AutoTuner` vs `p2GraphEasy-con-AutoTuner`

Date: 2026-03-16

## 1. Similarities

| Topic | `parallel` | `p1GraphEasy-con-AutoTuner` | `p2GraphEasy-con-AutoTuner` | Code References |
|---|---|---|---|---|
| Pass architecture | `AutoTunerPass.cpp/.h` | Same | Same | `parallel/AutoTunerPass.h`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.h`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.h` |
| Layout space | `CSR, PCSR, BCSR, SET` | Same | Same | `parallel/AutoTunerPass.cpp:24`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:24`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:24` |
| Region taxonomy | `Traverse, Insert, Query, Compute` | Same | Same | `parallel/AutoTunerPass.cpp:31`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:31`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:31` |
| Feasibility constraints | Traversal/compute CSR-constrained, insert/query partially flexible | Same | Same | `parallel/AutoTunerPass.cpp:90`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:90`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:91` |
| Unit-cost matrix | Same `C[4][4]` coefficients | Same | Same | `parallel/AutoTunerPass.cpp:136`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:136`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:137` |
| Conversion model | Same `alphaFromCSR/alphaToCSR`, non-CSR routed via CSR hub | Same | Same | `parallel/AutoTunerPass.cpp:145`, `parallel/AutoTunerPass.cpp:164`, `parallel/AutoTunerPass.cpp:183`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:145`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:164`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:183`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:146`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:165`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:184` |
| Operation-cost equation form | Same structure over `fT,fI,fQ,fS`, `H`, `Im`, `log2(m)` | Same | Same | `parallel/AutoTunerPass.cpp:195`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:195`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:196` |
| DP scheduler | Same forward DP + backtracking | Same | Same | `parallel/AutoTunerPass.cpp:261`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:261`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:273` |
| Suffix horizon model | Same reverse `suffixCost` dynamic programming | Same | Same | `parallel/AutoTunerPass.cpp:321`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:321`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:334` |
| Region pipeline | classify calls -> collect events -> build/merge regions -> inject | Same | Same | `parallel/AutoTunerPass.cpp:442`, `parallel/AutoTunerPass.cpp:528`, `parallel/AutoTunerPass.cpp:570`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:442`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:528`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:570`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:457`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:555`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:599` |

## 2. Differences

| Topic | `parallel` | `p1GraphEasy-con-AutoTuner` | `p2GraphEasy-con-AutoTuner` | Code References |
|---|---|---|---|---|
| Global schedule gate | Relative improvement gate `kMinScheduleImprovement=0.005` | Same | Ratio gate `kMinBenefitRatio=1.2` | `parallel/AutoTunerPass.cpp:74`, `parallel/AutoTunerPass.cpp:709`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:74`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:709`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:75`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:743` |
| Local switch gate | `sw < stay` | `sw < stay` | `sw < stay / 1.2` | `parallel/AutoTunerPass.cpp:646`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:646`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:675` |
| Event collection scope | `main` only | `main` only | all non-declaration functions in module | `parallel/AutoTunerPass.cpp:676`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:676`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:706` |
| Callee resolution robustness | Direct `getCalledFunction()` | Direct `getCalledFunction()` | Adds `resolveCallee()` for indirect/bitcasted calls | `parallel/AutoTunerPass.cpp:355`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:355`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:231`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:365` |
| Graph pointer alias handling | Basic provenance tracking | Basic provenance tracking | Adds strip-casts + single-graph fallback heuristics | `parallel/AutoTunerPass.cpp:470`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:470`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:495`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:510` |
| Injector initial assumed layout | `LAYOUT_SET` | `LAYOUT_CSR` | `LAYOUT_CSR` | `parallel/AutoTunerPass.cpp:617`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:617`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:646` |
| Injection behavior when schedule is weak/all-CSR | Still calls `injectConversions` for existing regions | Gated by `shouldSkip` and all-CSR check | Same as p1 (gated) | `parallel/AutoTunerPass.cpp:720`; `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:715`; `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:749` |
| Pass-level logging | Prints injected conversion count | No pass-level final log | No pass-level final log | `parallel/AutoTunerPass.cpp:724` |
| Runtime API richness | Minimal runtime API | Adds canonical mutation APIs and metadata handles | Same richer API as p1 | `parallel/autotuner_runtime.h:70`; `p1GraphEasy-con-AutoTuner/autotuner_runtime.h:80`, `p1GraphEasy-con-AutoTuner/autotuner_runtime.h:82`, `p1GraphEasy-con-AutoTuner/autotuner_runtime.h:92`; `p2GraphEasy-con-AutoTuner/autotuner_runtime.h:80`, `p2GraphEasy-con-AutoTuner/autotuner_runtime.h:82`, `p2GraphEasy-con-AutoTuner/autotuner_runtime.h:92` |
| Runtime metadata model | Basic fields | Adds `static_pair_count`, extra canonical edge arrays, `csr_owned` | Same enriched model | `parallel/autotuner_runtime.h:23`; `p1GraphEasy-con-AutoTuner/autotuner_runtime.h:30`, `p1GraphEasy-con-AutoTuner/autotuner_runtime.h:33`, `p1GraphEasy-con-AutoTuner/autotuner_runtime.h:43`; `p2GraphEasy-con-AutoTuner/autotuner_runtime.h:30`, `p2GraphEasy-con-AutoTuner/autotuner_runtime.h:33`, `p2GraphEasy-con-AutoTuner/autotuner_runtime.h:43` |
| Set->CSR builder style | Simpler `MAX_EDGE_ID` scanning model | Canonical meta builder with max-label semantics | Canonical meta builder with dense remapping (`qsort` + hash map) | `parallel/autotuner_runtime.c:90`; `p1GraphEasy-con-AutoTuner/autotuner_runtime.c:252`; `p2GraphEasy-con-AutoTuner/autotuner_runtime.c:253`, `p2GraphEasy-con-AutoTuner/autotuner_runtime.c:266`, `p2GraphEasy-con-AutoTuner/autotuner_runtime.c:281` |
| PCSR->CSR sanitization | Basic GAP check | Basic GAP check | Adds bounds checks `v >= 0 && v < n` | `parallel/autotuner_runtime.c:228`; `p1GraphEasy-con-AutoTuner/autotuner_runtime.c:576`; `p2GraphEasy-con-AutoTuner/autotuner_runtime.c:595`, `p2GraphEasy-con-AutoTuner/autotuner_runtime.c:613` |
| Benchmarking/tooling around autotuner | Ablation scripts in `parallel` | mutation-heavy template generator + switch-case CSV | compare-three benchmark suite + analyses | `parallel/run_ablation_bench.py`, `parallel/run_ablation_bench_five_types.py`, `parallel/generate_ablation_workloads.py`; `p1GraphEasy-con-AutoTuner/gen_mutation_heavy_templates.py`, `p1GraphEasy-con-AutoTuner/benchmark_switch_speedup_cases.csv`; `p2GraphEasy-con-AutoTuner/benchmark_compare_three.csv`, `p2GraphEasy-con-AutoTuner/benchmark_compare_three_analysis.md` |

## 3. Equation Appendix (for write-up)

### 3.1 Shared operation-cost model (all three)

For region `r` under layout `L`:

```text
OpCost(r, L) = H * totalOps * (
    fT * Ct(L) * m
  + fI * Ci(L) * Im(L, m)
  + fQ * Cq(L) * log2(max(2,m))
  + fS * Cs(L) * m
)
```

Where:
- `H = max(1, execCount)`
- `Im(L,m) = sqrt(m)` for CSR/BCSR, else `1`
- `Ct,Ci,Cq,Cs` come from the same `C[4][4]` matrix in all three passes.

References: `parallel/AutoTunerPass.cpp:195`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:195`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:196`

### 3.2 Shared conversion-cost model (all three)

```text
Conv(from,to) = 0                        if from == to
Conv(from,to) = alphaFromCSR(to)*(n+m)  if from == CSR
Conv(from,to) = alphaToCSR(from)*(n+m)  if to   == CSR
Conv(from,to) = (alphaToCSR(from)+alphaFromCSR(to))*(n+m) otherwise
```

References: `parallel/AutoTunerPass.cpp:183`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:183`, `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:184`

### 3.3 Decision gate difference (critical)

- `parallel/p1` global schedule acceptance:

```text
chosenCost < allCSR * (1 - 0.005)
```

Refs: `parallel/AutoTunerPass.cpp:709`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:709`

- `p2` global schedule acceptance:

```text
chosenCost < allCSR / 1.2
```

Ref: `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:743`

- `parallel/p1` local switch gate:

```text
sw < stay
```

Refs: `parallel/AutoTunerPass.cpp:646`, `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:646`

- `p2` local switch gate:

```text
sw < stay / 1.2
```

Ref: `p2GraphEasy-con-AutoTuner/AutoTunerPass.cpp:675`
