# Validating p1's format/schedule models against WACO

Reference: *WACO: Learning Workload-Aware Co-optimization of the Format and
Schedule of a Sparse Tensor Program*, ASPLOS 2023, DOI 10.1145/3575693.3575742.
Method: a learned cost model (WA-CONet — a sparse CNN over the sparsity
pattern) + a schedule template ("SuperSchedule") that couples format and
schedule, searched with approximate nearest neighbour; evaluated on SpMV,
SpMM, SDDMM and MTTKRP on CPU over 726 sparsity patterns, reporting speedups
over MKL, BestFormat, TACO-default and ASpT (1.43x / 1.18x / 1.14x / 1.27x).

## What maps onto what (p1 only)

| WACO | p1 analogue | where |
|---|---|---|
| format decision | layout DP: SET is the base, CSR/PCSR/BCSR are transient per-phase layouts | `AutoTunerPass.cpp` (`suffixCost`, `layoutFeasible`, `conversionCost`), runtime in `autotuner_runtime.c` |
| schedule decision | thread/TDG budget + GPU offload gates; DOACROSS wave kernel vs CPU | `parallel_runtime.c`, `gpu_runtime.c` (`sgpl_gpu_policy_verdict`) |
| learned, workload-aware cost model | hand-tuned, hardware-calibrated unit costs `H·totalOps·(f_T·uTrav(L) + f_I·uIns(L))` + conversion costs | `AutoTunerPass.cpp:1026`, `AUTOTUNER_HW_CALIB` |
| FixedCSR baseline | CSR-only build: `AUTOTUNER_FORCE_LAYOUT=CSR` | validation instrument (see the note below) |
| format-only / schedule-only / co-optimization ablation | (a) forced layout vs model-chosen, (b) TDG/threads off vs on, (c) both | this study |
| decision accuracy vs best | model's chosen layout vs the **measured-best** layout per workload | E3 below |
| cost-model accuracy (predicted vs measured) | `predicted_op_total_ns` per region vs measured region time | E4 below |
| tuning overhead | compile-time DP (milliseconds) vs WACO's autotuning search | E5 below |

## What is *not* comparable (stated up front)

* Different workload domain: graph analytics (traversal/peeling/relaxation)
  versus sparse linear algebra (SpMV/SpMM/SDDMM/MTTKRP).
* Different corpus size: our graph corpus is tens of graphs, not 726 matrices.
* Our model is static and hand-tuned (with hardware calibration); theirs is
  learned.  Treat the comparison as *methodological*, not head-to-head on their
  benchmarks.
* Their evaluation is CPU-side, which matches our format study (the layout
  machinery is a CPU structure); our GPU-side comparisons are separate.

## Experiments (with live status)

* **E1 — schedule cost-model sweep (GPU).** `verify/gpu_cost_sweep.sh`:
  device forced with the gates disabled across sizes, median of 3 warm-off runs,
  emitting `cost_model_evidence.csv`; `plot_cost_model.py` renders the speedup
  curve against the gate thresholds.  Running on the GPU box.
* **E2 — format matrix.** `verify/p1_format_probe.sh <case.graph> [graph] [tag]`:
  builds the same program five times (cost-model, CSR, PCSR, BCSR, SET) and
  reports median time, answer, injected conversions and conversion time.  All
  five must produce the *same* answer.
* **E3 — decision accuracy.** From E2: the model's choice (read off the emitted
  IR: the `autograph_ensure_layout(..., i32 K)` constants) versus the fastest
  measured layout per workload; report accuracy and the regret
  (measured_best / measured_model).
* **E4 — predicted vs measured cost.** Enable the pass's region profile print
  (currently compiled out with `#if 0` in `AutoTunerPass.cpp`) behind an env and
  correlate predicted against measured per region.
* **E5 — overheads.** Compile-time cost of the layout DP (with/without the
  pass) and the runtime conversion cost already reported by the program
  (`injected N layout conversions total, conversion_ns=...`).

## Finding: the force-layout instrument was not wired

`AUTOTUNER_FORCE_LAYOUT` existed but only relaxed the injection guards
(`AutoTunerPass.cpp:1950,1953`) and was a dead variable in the pass entry
(`:2152`); the DP's `chosen` layout was never constrained, so forcing a layout
that the DP did not itself pick installed *nothing* (measured: 0 injected
conversions under `FORCE_LAYOUT=BCSR`, IR had no `ensure_layout` call).  Fixed
by constraining feasibility for the *choice* region types (Traverse/Insert) to
the forced layout, while regions whose layout the operations require
(`SetQuery` → SET, `CSRQuery` → CSR) keep their requirement — forcing those
would be a correctness bug, not a schedule.  After the fix, `FORCE_LAYOUT=BCSR`
emits `autograph_ensure_layout(..., i32 2)` and the runtime reports a real
conversion.
