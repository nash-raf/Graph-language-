# Validation report: DOALL, DOACROSS and the TDG universal thread budget

Machine: runpod CPU box (48 cores; sweeps capped at 24 threads), CPU backend
(`SGPL_GPU_BACKEND=0`), wall-clock timing, best of 3 runs per point unless stated.
All artifacts referenced below live under `p1/verify/` on the pod and were pulled
to the local machine as noted.

Contents:
1. DOALL cost model — method and results (P = 1..24)
2. DOACROSS cost model — method and results (P = 1..24)
3. TDG universal thread budget — method and results (every allocation, 24 threads)
4. Instrumentation added for the validation (all default-off)
5. What the numbers say / known limitations

---

## 1. DOALL cost model

### How it was done

**Subject.** `cases/parallel/doall_scaling.graph` on `fixtures/g20k.txt` — a
`for each vertex v` loop inside a `while (round < 200)` driver, N = 20000, 200
rounds. This is a real cost-model site; the debug name is
`task_1::foreach.cond16`, `loop_id=1`.

**Build.**
```bash
SGPL_GPU_BACKEND=0 GRAPH_FILE=<graph> bash 03_run.sh
```

**Measurement.** For each P in 1..24: 3 whole-program runs with
`SGPL_NUM_THREADS=P`, take the minimum wall time, divide by 200 rounds to get
ms/round. Then one extra run with `GRAPH_PARALLEL_DEBUG=1` to capture the model's
own line:

```
[parallel-runtime] cost-doall loop=task_1::foreach.cond16 loop_id=1 invocation=5
  choose=parallel N=20000 P=24 c_ns=... L_thread_ns=... L_path_ns=... L_total_ns=... \
  threshold=... samples=... c_state=stable
```

**Prediction (taken from the model's own algebra in `parallel_runtime.c`).**
```
pred_serial(P) = c_ns * N
pred_P(P)      = L_total(P) + c_ns * N / P
eta            = pred_P / measured            (1.00x = perfect)
```

**Calibration detail that matters.** A site stops printing its calibrated line
once it starts choosing `parallel` — only the P = 1 run keeps sampling long
enough to reach `c_state=stable`. So `c_ns` is read from the stable P = 1 line
(84.81 ns/iter) and used for every P, while `L_total(P)` is the launch-overhead
term read from each P's own decision line (1560 ns at P=2 up to 5520 ns at P=24).

**Drive script.** `eta_p24_sweep.py` (pod), subset of results then re-extracted
with the stable-site rule. Interior per-P rows and the figure were produced by a
small post-processing pass (`/tmp/fix24.py` on the pod).

### Results (g20k, N = 20000, c_ns = 84.81 ns/iter)

| P | pred serial (ms) | pred P (ms) | measured (ms) | eta |
|---|---|---|---|---|
| 1 | 1.6962 | 1.6962 | 2.8622 | 0.593 |
| 2 | 1.6962 | 0.8497 | 2.8325 | 0.300 |
| 3 | 1.6962 | 0.5671 | 4.9889 | 0.114 |
| 4 | 1.6962 | 0.4260 | 4.7513 | 0.090 |
| 5 | 1.6962 | 0.3413 | 4.9862 | 0.068 |
| 6 | 1.6962 | 0.2850 | 4.7300 | 0.060 |
| 7 | 1.6962 | 0.2448 | 4.9845 | 0.049 |
| 8 | 1.6962 | 0.2147 | 4.9994 | 0.043 |
| 9 | 1.6962 | 0.1913 | 4.7453 | 0.040 |
| 10 | 1.6962 | 0.1726 | 4.9729 | 0.035 |
| 11 | 1.6962 | 0.1574 | 5.0139 | 0.031 |
| 12 | 1.6962 | 0.1447 | 4.7272 | 0.031 |
| 13 | 1.6962 | 0.1340 | 4.9738 | 0.027 |
| 14 | 1.6962 | 0.1249 | 4.9699 | 0.025 |
| 15 | 1.6962 | 0.1170 | 4.9924 | 0.023 |
| 16 | 1.6962 | 0.1101 | 4.9838 | 0.022 |
| 17 | 1.6962 | 0.1040 | 4.9727 | 0.021 |
| 18 | 1.6962 | 0.0987 | 4.7063 | 0.021 |
| 19 | 1.6962 | 0.0939 | 5.0279 | 0.019 |
| 20 | 1.6962 | 0.0896 | 4.9778 | 0.018 |
| 21 | 1.6962 | 0.0858 | 4.7179 | 0.018 |
| 22 | 1.6962 | 0.0823 | 5.0071 | 0.016 |
| 23 | 1.6962 | 0.0791 | 5.0005 | 0.016 |
| 24 | 1.6962 | 0.0762 | 4.9838 | 0.015 |

Sanity points from an earlier 16-thread pass on the same subject class:

| subject | N | P | pred (ms) | measured (ms) | eta |
|---|---|---|---|---|---|
| ia-dbpedia | 365493 | 2 | 15.5 | 16.1 | **0.963** |
| ia-dbpedia | 365493 | 4 | 7.8 | 17.1 | 0.454 |
| ia-dbpedia | 365493 | 8 | 3.9 | 15.5 | 0.251 |

### Reading

- Work-dominated subjects are predicted well at low P (eta = 0.96 at P=2 for
  N=365k), then over-predicted as P grows (the measured curve flattens).
- This subject is overhead-dominated: measured is a flat ~4.7-5.0 ms/round for
  P >= 3 while the model keeps predicting a 1/P decrease, so eta collapses to
  0.02-0.09. Parallelising this subject is actually slower than serial
  (2.86 ms/round at P=1 vs ~5 ms/round at P>=3) — the model has no term for the
  per-dispatch pool floor.

Artifacts: `p24_out/eta_p24_fixed.csv`, `p24_out/eta_p24_table.md`,
`p24_out/eta_p24_fixed.png/.svg`, `pod_eta/eta_doall_table.md`,
`pod_eta/eta_doall_pod.png/.svg`.

---

## 2. DOACROSS cost model

### How it was done

**Getting a calibratable subject was the hard part.** The natural subject
(`cases/parallel/doacross_while_d1.graph`, a distance-1 recurrence with a literal
bound) classifies as `DOACROSS carrier=1 positive constant distance=1`, but it
runs **once per process**, so the DOACROSS sampler only ever sees
`invocation=1, c_state=warming, c_dep_ns=0.00` — the first decision is taken
before any sample exists and there is no second dispatch to calibrate from.

Path taken:
1. Wrapping the loop in a `while (round < 200)` driver is required to get
   repeated dispatches. The first generated wrapper was malformed (the inner
   `while` header was dropped by the generator regex) — it degenerated to
   `last 0` and classified `DOALL`. After fixing the generator, the inner loop
   is verbatim and classifies `depth=2 classification=DOACROSS
   hasProvenCarriedDep=1` (the outer driver is `SEQUENTIAL`, which is expected).
2. The wrapped fixture still produced no site until the outliner's flat floors
   were lowered **at build time** (see section 4):
   `SGPL_OUTLINER_MIN_EFF=1 SGPL_OUTLINER_MIN_TRIP=2` (defaults unchanged; the
   fixture body is smaller than the flat CPU floor of 8 effective instructions).
3. With both fixes the site calibrates:
   `cost-doacross loop_id=1 invocation=199/200 choose=serial N=8191 P=4 bucket=5
   c_state=stable c_dep_ns=30.69 c_ind_ns=0.00 ...`

**Subject used.** `cases/parallel/doacross_repeat_d1.graph` — the distance-1
recurrence, inner loop verbatim, wrapped 200x, N = 8191. Build:
```bash
SGPL_GPU_BACKEND=0 SGPL_OUTLINER_MIN_EFF=1 SGPL_OUTLINER_MIN_TRIP=2 \
  GRAPH_FILE=<graph> bash 03_run.sh
```

**Measurement.** For each P in 1..24:
- 3 unforced runs (`SGPL_NUM_THREADS=P`) — the model's own decision runs (it
  chooses `serial` for a chain); best of 3, per-dispatch = wall/200.
- 3 runs with `SGPL_FORCE_DOACROSS_PARALLEL=1` — the parallel path measured
  directly; best of 3, per-dispatch = wall/200. (Under force no decision line is
  printed, so predicted fields come from the unforced debug run at the same P.)
- one debug run with `SGPL_NO_WARMUP_DECISION_CACHE=1 GRAPH_PARALLEL_DEBUG=1`
  to read the calibrated fields at that P.

**Prediction (the model's own algebra, `sgpl_should_parallelize_doacross`).**
```
pred_serial = N * (c_dep + c_ind)
pred_P      = L_total + N*c_dep + N*c_ind/P + N*sync_per_iter
choose_parallel = pred_serial > pred_P
eta_serial  = pred_serial / measured_unforced
```

### Results (repeat_d1, N = 8191, c_dep = 30.7 ns/iter stable; model decides serial)

| P | pred P (ms) | measured unforced/serial (ms) | eta serial | measured forced-parallel (ms) |
|---|---|---|---|---|
| 1 | 0.2520 | 0.6335 | 0.398 | 4.6767 |
| 2 | 0.2714 | 0.6336 | 0.428 | 6.7618 |
| 3 | 0.2521 | 0.6361 | 0.396 | 6.4841 |
| 4 | 0.2515 | 0.6372 | 0.395 | 5.9024 |
| 5 | 0.2523 | 0.6278 | 0.402 | 6.3685 |
| 6 | 0.2530 | 0.6417 | 0.394 | 5.1473 |
| 7 | 0.2509 | 0.6475 | 0.387 | 6.3775 |
| 8 | 0.2515 | 0.6293 | 0.400 | 4.6968 |
| 9 | 0.2549 | 0.6507 | 0.392 | 5.2569 |
| 10 | 0.2517 | 0.6529 | 0.386 | 4.6992 |
| 11 | 0.2528 | 0.6324 | 0.400 | 6.8551 |
| 12 | 0.2517 | 0.6359 | 0.396 | 6.3989 |
| 13 | 0.2526 | 0.6377 | 0.396 | 5.3145 |
| 14 | 0.2510 | 0.6440 | 0.390 | 6.5231 |
| 15 | 0.2518 | 0.6372 | 0.395 | 6.4775 |
| 16 | 0.2514 | 0.6433 | 0.391 | 6.2970 |
| 17 | 0.2547 | 0.5964 | 0.427 | 6.4116 |
| 18 | 0.2539 | 0.6518 | 0.390 | 6.4463 |
| 19 | 0.2511 | 0.6384 | 0.393 | 4.1091 |
| 20 | 0.2533 | 0.6387 | 0.397 | 5.0495 |
| 21 | 0.2516 | 0.6475 | 0.389 | 6.8242 |
| 22 | 0.2509 | 0.6294 | 0.399 | 6.4059 |
| 23 | 0.2527 | 0.6354 | 0.398 | 4.4487 |
| 24 | 0.2533 | 0.6470 | 0.391 | 6.4703 |

### Reading

- The **decision** is right: for a distance-1 chain the model refuses to
  parallelise, and the forced-parallel measurement confirms it (4.1-6.9 ms per
  dispatch vs 0.63 ms serial — the wave/sync cost is real).
- But the model's own **serial estimate is ~2.5x low**: `N*c_dep` = 0.25 ms
  predicted vs 0.64 ms measured per dispatch, i.e. `c_dep` (30.7 ns/iter) is well
  below the measured ~78 ns/iter. The eta_serial column sits at 0.39-0.43 across
  all P — a stable, reproducible bias, not noise.

Artifacts: `p24_out/eta_p24_fixed.csv` (columns `c_dep_ns`, `pred_P_ms`,
`measured_serial_ms`, `eta_serial`, `measured_ms`), `p24_out/eta_p24_fixed.png`.

---

## 3. TDG universal thread budget

### How it was done

**Case design.** Two registered loop sites with unequal work (light = 101,
heavy = 102, heavy:light ratio 3 by construction, measured costs fed to the
planner) dispatched as **one 2-task TDG level** via the real
`sgpl_run_tdg_level(tasks, 2, work, span)` entry point. Harness:
`budget_alloc_test.c` (bodies do real arithmetic, `SGPL_ALLOC_TRIPS=1<<20`
trips/round, `SGPL_ALLOC_ROUNDS=40`, best of 3 whole-program wall times).

**The model's own arithmetic, read live.** With `SGPL_BUDGET_DEBUG=1` /
`SGPL_TDG_PLAN_DUMP=1` the runner prints the plan it built:

```
[tdg.plan] level entries=2 budget=24 loop_budget=22 idle=0 dominant=101 total_model_ns=...
[tdg.plan]   slot=0 loop_id=101 assigned_threads=11 c_rank_ns=... model_ns=...
[tdg.plan]   slot=1 loop_id=102 assigned_threads=11 c_rank_ns=... model_ns=...
```

i.e. level budget 24, level width 2 (one thread per task), **loop budget 22**
distributed across the two task loops — exactly the "subtract the TDG width,
distribute the remaining threads" rule.

**Enumerating every allocation.** `SGPL_FORCE_WIDTHS="101:w1,102:w2"` pins the
per-loop widths inside the level plan (implemented in
`sgpl_tdg_apply_forced_widths`, applied before the plan is handed to the loop
pool), so for every split w1 + w2 = 22 the level really runs with that split —
verified by the pool lines (`[budget.loop-pool.reserve] ... granted=w1`) and the
per-site worker counts in the CSV. The model's unforced run supplies its own
chosen split; each run also reports per-task self-timed durations
(`last_light_ns`, `last_heavy_ns`), which is how the concurrency effect is shown.

**Drive.** `budget_alloc_sweep.py --threads 24 --rounds 40 --reps 3` on the pod.

### Results — every allocation (24 threads)

Level budget 24, level width 2, loop budget 22. Model chose **11 + 11**;
measured optimum **21 + 1**. Verdict: **MISMATCH**.

| light (101) | heavy (102) | time (ms) | ms/round | note |
|---|---|---|---|---|
| 1 | 21 | 10091.62 | 252.291 | |
| 2 | 20 | 13680.11 | 342.003 | |
| 3 | 19 | 13510.93 | 337.773 | |
| 4 | 18 | 13707.61 | 342.690 | |
| 5 | 17 | 14481.92 | 362.048 | |
| 6 | 16 | 13315.91 | 332.898 | |
| 7 | 15 | 14599.82 | 364.996 | |
| 8 | 14 | 13758.30 | 343.957 | |
| 9 | 13 | 14120.14 | 353.004 | |
| 10 | 12 | 14212.47 | 355.312 | |
| 11 | 11 | 13270.55 | 331.764 | **model choice** |
| 12 | 10 | 13293.38 | 332.335 | |
| 13 | 9 | 14565.48 | 364.137 | |
| 14 | 8 | 14590.29 | 364.757 | |
| 15 | 7 | 13048.82 | 326.221 | |
| 16 | 6 | 13912.08 | 347.802 | |
| 17 | 5 | 14578.24 | 364.456 | |
| 18 | 4 | 13599.19 | 339.980 | |
| 19 | 3 | 13730.45 | 343.261 | |
| 20 | 2 | 14052.07 | 351.302 | |
| 21 | 1 | 9494.58 | 237.365 | measured optimum |

Reproduction at 16 threads (same harness): model chose 7 + 7 = 11854.72 ms,
measured optimum 2 + 12 = 7304.04 ms — same shape, so the mismatch is not a
one-off of the budget size.

### Reading

- The measured curve is U-shaped with the minima at the edges (21+1 = 9.49 s,
  1+21 = 10.09 s) and the middle at ~13-14.6 s. The model's choice (11+11,
  13.27 s) sits in the slow middle — ~40% slower than the optimum.
- Cause, from the same CSV: the objective (`sgpl_nlopt_loop_objective`) is
  `sum_i relaxed_cost_i(x_i)` — a sum of per-loop modelled costs — but the level
  runs its two tasks **concurrently** (per-task times show the round is close to
  max(task), not the sum), and each dispatch carries a fixed pool/launch cost
  the model does not represent. The plan has no term for either.

Artifacts: `budget24_out/budget_alloc.csv`, `budget_alloc_table.md`,
`budget_alloc.png/.svg`; 16-thread version in `budget_out/`.

---

## 4. Instrumentation added for this validation (all default-off)

Runtime, `parallel_runtime.c`:

| knob | effect |
|---|---|
| `SGPL_FORCE_WIDTHS="<id>:<w>[,<id>:<w>...]"` | pins a loop site's width (plan level; drives both decision and dispatch) — used for the allocation sweep |
| `SGPL_NO_WARMUP_DECISION_CACHE=1` | does not cache a decision taken while the sampler is still warming (the DOACROSS path otherwise freezes its first serial verdict for the process lifetime) |
| `SGPL_TDG_PLAN_DUMP=1` | prints `[tdg.plan]` — the level's chosen allocation per loop (`assigned_threads`, `model_ns`, budget, loop_budget) |
| `[tdg.plan]` print | also auto-enabled when `SGPL_FORCE_WIDTHS` is set, so a forced sweep always logs the model's own numbers |

Outliner, `parallel_loop_outline.cpp`:

| knob | effect |
|---|---|
| `SGPL_OUTLINER_MIN_TRIP=<n>` | lowers the constant-trip-count floor (default 32) |
| `SGPL_OUTLINER_MIN_EFF=<n>` | lowers the effective-body-instruction floor (default 8 flat / 3 graph / 1 GPU) |

Harnesses and drivers (repo `verify/`):

- `budget_alloc_test.c` — two-site 2-task-level timing harness (TDG section).
- `budget_alloc_sweep.py` — builds the harness, enumerates every split, best of
  N, reads the model's choice from `[tdg.plan]`, writes CSV + markdown +
  matplotlib figure with the model's allocation marked.
- `eta_p24_sweep.py` — the P = 1..24 predicted/measured sweeps for DOALL and
  DOACROSS; writes CSV + markdown + figure.
- Fixtures: `cases/parallel/doacross_repeat_d{1,2,4}.graph`;
  `cases/parallel/budget_two_loops.graph`, `budget_two_steps.graph` (design
  attempts that showed the level only forms for wrapped dispatches).

Defaults are unchanged with every knob unset — verified by rebuilding and
running without them (the original `validate_tdg_budget.sh` gate still passes).

---

## 5. What the numbers say

1. **DOALL**: accurate where work dominates (eta2 = 0.963 at N=365k), but it has
   no per-dispatch overhead term; on small subjects the measured time is a flat
   pool floor and eta collapses (0.02-0.09 at high P). It also over-predicts
   scaling at high P on large subjects (measured flattens, prediction keeps
   dropping).
2. **DOACROSS**: the decision logic is sound for chains (serial chosen, forced
   parallel is 7-10x worse), but the calibrated `c_dep` underestimates the real
   per-iteration dependency cost by ~2.5x, giving a stable eta_serial of ~0.39-0.43.
3. **TDG budget**: the allocator minimises the sum of per-loop modelled costs,
   while the level is concurrent and pays a fixed dispatch cost per loop; it
   therefore picks the balanced split, which is ~40% slower than the measured
   optimum at both 16 and 24 threads.

Known limitations of this validation: single subject per model (one graph for
DOALL, one distance for DOACROSS, one work ratio for the budget case); the
DOACROSS subject needs the outliner floors lowered at build time to become a
site; a DOALL site's calibrated line is only available from its serial (P=1)
sampling, since logging stops after the parallel decision.
