# TDG and the universal thread-budget model vs. effect-system parallelism

**Status: Plan B implemented and verified; Plan A partially implemented (identity
glue + root causes measured), remaining steps specified in Part 5.**
Two questions were checked read-only against the tree, and a plan is given for
each. Every measurement in Part 1/2 and Part 4 is reproducible with the commands
in Appendix A.

> **Update (implementation session).** Plan B landed in the runtime
> (`parallel_runtime.c/.h`) with the harness `tdg_budget_test.c` (T1–T6, all
> passing) and no race regression on the graph programs. Plan A's identity glue
> landed in `parallel_loop_outline.cpp` (every live dispatch now registers its
> loop id). Two of the plan's assumptions were **corrected by measurement** —
> see Part 4: (a) the earlier "0 debug lines" evidence was confounded because the
> debug helpers are compiled out (now re-enabled behind `SGPL_TDG_DEBUG`);
> (b) the real blocker is that the TDG level collector keys on *wrapper calls*
> which the live outliner never emits, and that level *sites* must be profiled
> before the chooser will plan them.

Setup: compiler `p1GraphEasy-con-AutoTuner`, threads via `SGPL_NUM_THREADS` /
`OMP_NUM_THREADS`, debug via `SGPL_TDG_DEBUG=1`, `SGPL_BUDGET_DEBUG=1`,
`GRAPH_PARALLEL_DEBUG=1`.

---

## Part 0 — How TDG and the budget model fit together today

The pieces, in the order they run:

| stage | where | what it does |
|---|---|---|
| loop descriptor | `parallel_loop_outline.cpp:2863-2918` (`buildLoopProfileDescriptor`), called from `finalizeLoopDispatch` (`:2922`, guard `Transform.ParallelReady`, call site `:3037`) | one `sgpl.loop.desc.<id>` global per outlined loop: loop id, DOALL/DOACROSS, privatized flag, env size, priv offsets, wait/post counts, debug name |
| level collection + split | `pdg.cpp:4151` (scan for `sgpl.loop.desc.*` reachable from task functions) and `pdg.cpp:4200-4210` (`sgpl_run_tdg_level` emission per sibling **CallGroup**) | groups sibling outlined launches into a *level* and hands it to the runtime |
| level optimiser | `parallel_runtime.c:2845+` (`[tdg.opt-input] loop_budget=… loop_sites=… objective=sum-cost solver=nlopt`) | NLOPT splits the level's thread budget across the level's loops (consumed vs idle), with repair passes |
| level pool | `parallel_runtime.c:2333-2400` (`sgpl_loop_pool_try_acquire`) + `sgpl_tdg_enter_budget_scope` (`:697-716`) | per-level ledger of `remaining_threads`; each loop id may take `assigned_threads` once |
| generic dispatch | `parallel_for_runtime` (`parallel_runtime.c:5193`) | `nthreads = sgpl_loop_effective_decision_threads()` (`:2260-2285`: pool assignment, else `min(runtime_threads, budget_available)`), then `sgpl_loop_pool_try_acquire(loop_id, nthreads)` |
| budget API | `sgpl_current_thread_budget` / `sgpl_push_thread_budget` / `sgpl_pop_thread_budget` (`:272-330`), scope TLS `g_tls_thread_budget_scope_reserved`, global ledger `g_sgpl_reserved_threads` | caps, shares and reservations; `sgpl_budget_try_reserve` (`:373`) is the CAS-style reservation |
| per-loop chooser | `sgpl_choose_tdg_threads` / `…_with_loop_budget` (`parallel_runtime.h:110-118`) | **no compiler call site anywhere** (runtime-only) |

Effect-system loops never appear in this table: the graph-frontier lowering emits
engine calls (`autograph_frontier_execute`, `autograph_frontier_step`,
`autograph_edgemap`) *inside `main`*, with no loop id and no descriptor. The
engine then dispatches through the same `parallel_for_runtime`.

---

## Part 1 — Q1: does TDG work on effect algebra?

**Verdict: no — and today it does not fire for graph-DSL programs at all, engine
steps included.**

Measured (one compile + one run each):

| program | `[tdg.*]` lines | `[budget.*]` lines | `sgpl.loop.desc` in binary | `sgpl_run_tdg_level` called | dispatches |
|---|---|---|---|---|---|
| `verify/cases/algo/pagerank.graph` (effect-system, g20k) | 0 | 0 | 0 | no | 8 × `threads=4` |
| `verify/cases/parallel/array_doall.graph` | 0 | 0 | 0 | no | 0 printed |
| `verify/cases/parallel/doall_scaling.graph` | 0 | 0 | 0 | no | 9 × `threads=4` |

Supporting facts:

* the loops **are** outlined (`outlined_task_1_foreach.cond16.parallel`,
  `range_task_1_foreach.cond16.parallel` symbols exist, `sgpl_set_pending_range_body`
  is called), yet **no descriptor survives to the binary** and no level call is
  emitted;
* `sgpl_choose_tdg_threads` — the per-loop TDG chooser — has **no compiler call
  site**; TDG is only ever the level path;
* TDG *has* run historically, but on the synthetic research kernels, not the
  DSL: `benchmark_logs/doacross_fix_*/…/after.ll` contain `sgpl.loop.desc.*`
  (kernels `01_prefix_pure`, `06_saxpy_doall`, `08_fib_like`, …);
* for engine steps the situation is structural: no loop id ⇒ no pool entry ⇒
  `sgpl_loop_effective_decision_threads_for_loop_id(-1)` returns
  `min(runtime_threads, budget_available)` (`parallel_runtime.c:2260-2285`) and
  `sgpl_loop_pool_try_acquire(-1, n)` returns 1 if a level pool is active
  (`:2341`). So an engine step gets raw machine width at top level and **one
  thread** inside a level.

Consequence: engine-step parallelism is *ungoverned* — no unit-cost/launch-cost/
HT-policy/min-gain gate applies to it, and when it is nested under a level it
cannot draw on the level's budget at all.

### Plan A — bring effect-system steps under TDG

Ordered, smallest-first; each step independently verifiable.

**A1. Fix descriptor wiring for DSL loops (prerequisite).**
Run `SGPL_OUTLINER_DEBUG=1` on `doall_scaling`; determine whether
`finalizeLoopDispatch` builds a descriptor and whether anything in the task
function references it (the `pdg.cpp` collector only finds descriptors it can
reach through a task's operands). If it is created and then dropped by
`globaldce`, mark it `llvm.used` or reference it from the task's launch record.
Exit criterion: `nm final_program | grep -c sgpl.loop.desc` > 0 and
`[tdg.opt-input]` appears for a DSL program.

**A2. Give each engine step an identity.** In `graph_frontier_lowering.cpp`, at
the point where the engine call is emitted, create a `sgpl.loop.desc.<synthetic
id>` for the step: kind (traverse / edgemap / owner-push), debug name
(`fn::driver-loop`), work units derived from the autotuner's per-region
prediction (`predictedNs`), and env/priv fields as applicable; reference it from
the call site so it survives.

**A3. Admit engine calls to a level.** Extend the `pdg.cpp` collector
(`~4200-4210`) with a new member type for engine-step calls — they have no
`collectLaunchSetups`/witness contract, so do **not** reuse the outlined-launch
path. Minimal version: wrap the step's driver loop (`while frontier not empty`)
in a `sgpl_run_tdg_level` whose single task is the step, so every round shares
one pool.

**A4. Runtime glue.** If A3 does not use the level runner, add
`sgpl_set_pending_loop_id(int32_t)` next to `sgpl_set_pending_range_body`
(`parallel_runtime.c:514`) and emit it immediately before the engine call, so
the pool entry is found and `sgpl_loop_pool_try_acquire` grants the assigned
width instead of 1.

**A5. Cost model inputs.** Map step work to TDG units honestly (edges ×
per-pair ns from the autotuner model, or the predicted region time converted to
units) so the NLOPT split prices engine steps against sibling loops. Kill switch
`SGPL_NO_TDG_ENGINE=1`.

**A6. Validation.** (i) `[tdg.opt-input]` lists the engine site in
`loop_sites`; (ii) `[budget.loop-pool.reserve]` shows `granted > 1` for the step;
(iii) the suite's race matrix stays bit-identical 1thr==4thr; (iv) pagerank/cc/
bfs timings at 1/2/4/8 threads show no regression; (v) ledger stays balanced
(`sgpl_debug_reserved_threads` never exceeds the machine width).

**Risk.** The level machinery assumes launch/witness bookkeeping; keep engine
steps a separate member type. TDG decisions must only change *thread counts*,
never results — pin that with the race matrix.

---

## Part 2 — Q2: does the universal thread-budget interface work with
effect-system loops?

**Verdict: partially.** The engine dispatch does go through the budget code, but
only in the degenerate way, and one case is actively capped.

What already works (code-verified and observed):

* `parallel_for_runtime` computes `nthreads = min(runtime_threads,
  sgpl_budget_available_threads())` — the TLS budget cap and the global ledger
  are consulted;
* with **no** active level pool it reserves through `sgpl_budget_try_reserve`
  (`:2341`), so nested dispatches see `total − reserved` (no oversubscription);
* observed on pagerank/doall_scaling: dispatches run at `threads=4`, reservation
  succeeds silently (no `[budget.*]` lines because the ledger path prints only
  on contention).

Gaps:

1. **Pool-active callers are forced to one thread** (`parallel_runtime.c:2341`:
   `if (loop_id < 0) return 1;`) and never touch `pool->remaining_threads` — an
   engine step nested under a TDG level cannot draw on the level's budget.
2. **No scope publication for a step's own grant.** After
   `parallel_for_runtime` grants *g* threads, nothing enters
   `sgpl_tdg_enter_budget_scope` for the region body, so nested work inside pair
   functions only sees the global ledger remainder (safe, but not
   share-of-parent semantics).
3. **No way to state a step's desired width.** The compiler cannot annotate
   "this step wants up to 4 threads, at least 2" the way the range-body TLS
   mechanism annotates a body, so a level split cannot see engine steps at all.

### Plan B — make the budget interface first-class for engine steps

**B1. Stop the forced 1** (`parallel_runtime.c:2341`). For `loop_id < 0` with an
active pool: first `sgpl_budget_try_reserve(min(requested,
pool->remaining_threads))` (or carve a small "unregistered" share of the pool at
level entry), fall back to 1 only when nothing is available. This alone removes
the stall and keeps the reservation CAS as the single source of truth.

**B2. Publish the grant as a scope.** Around the parallel region in
`parallel_for_runtime`, enter `sgpl_tdg_enter_budget_scope(granted, granted, …)`
and exit on every path including the serial fallback, so nested parallelism
inside a step's pair function takes a share of the step's grant.

**B3. Desired-width annotation.** Add a TLS annotation
(`sgpl_thread_budget_desired(min, max)`) mirroring the pending-range-body
pattern (`parallel_runtime.c:500-520`) so the compiler can state a step's intent
without new signatures.

**B4. Intersect with Plan A.** Once steps have a loop id and a pool entry, B1
becomes the fallback for *unknown* callers only, and the level split (A5) can
price them.

**B5. Tests and the missing fixture.** The corpus has **no** case where an
effect step sits inside an outlined parallel loop. Add one (a graph program with
a DOALL loop whose body calls a frontier step), then assert with
`SGPL_BUDGET_DEBUG=1`: reserved never exceeds machine width; grants balance over
rounds; 1thr==4thr bit-identical; `SGPL_NO_ENGINE_BUDGET=1` restores the old
behaviour.

**Risk.** Relaxing the forced-1 rule can oversubscribe if any path bypasses the
ledger; keep every claim through `sgpl_budget_try_reserve` and assert the ledger
in tests.

**Recommended order:** Plan B first (two code sites, removes the stall, unifies
semantics), then Plan A (adds identity, decision and cost model). Both need the
fixture from B5 before they can be validated end-to-end.

---

## Part 5 — Implementation status, evidence, and what remains

### 5.1 What landed

**Plan B — complete.** `parallel_runtime.c/.h`:

| change | where | switch |
|---|---|---|
| unregistered callers inside an active level pool share `remaining_threads` (decision + acquire + release) | `sgpl_loop_effective_decision_threads_for_loop_id`, `sgpl_loop_pool_try_acquire`, `sgpl_loop_pool_release` | `SGPL_NO_UNREGISTERED_POOL_SHARE=1` |
| the pool lookup no longer forces `loop_id < 0 → 1` | `sgpl_loop_pool_lookup_assigned_threads` | same |
| serial-decided levels advertise their idle capacity (`granted − chosen`) | `sgpl_run_tdg_level` after `sgpl_loop_pool_init` | same |
| a parallel region publishes its grant as a budget scope (master + `worker_main`) | `parallel_for_runtime(_ex)`, `worker_main` | `SGPL_NO_REGION_SCOPE=1` |
| annotated/derived width: never more threads than trips, `sgpl_set_desired_threads(min,max)` | `sgpl_dispatch_thread_request` | — |
| `sgpl_set_pending_loop_id(int)` (public, mirrors the range-body TLS) | header + runtime | — |
| `sgpl_debug_nested_parallel_calls()` introspection | header + runtime | — |
| TDG/budget debug re-enabled (it was compiled out) | `tdg_debug_enabled`, `budget_debug_enabled` | `SGPL_TDG_DEBUG=1` |

Harness `tdg_budget_test.c` (built like the other runtime tests): **T1** trip
clamp, **T2** desired clamp, **T3** unregistered share (3 workers; kill switch →
1), **T5** nested dispatch bounded (`nested_parallel_calls=0`), **T6** registered
two-site level, **T7** single-site level, **T4** ledger balanced — all pass at 4
threads, at 4 threads with the sharing switch off, and at 1 thread.

**Plan A — identity glue + the missing chooser capability + the engine step
wired end to end.**

* `parallel_loop_outline.h`: one **shared id space** (`nextParallelSiteId()`),
  used by the outliner *and* the frontier lowering, so an outlined loop and an
  engine step can never share an id (ids are allocated, never guessed).
* `parallel_loop_outline.cpp` (`finalizeLoopDispatch`): every live dispatch emits
  `sgpl_set_pending_loop_id(LoopId)` before the versioned dispatch.
* `graph_frontier_lowering.cpp`: each single-stage `autograph_exec_ctx_create`
  passes a fresh id from that space; fork/join children pass `-1` (their
  parallelism *is* the fork/join; their inner dispatch keeps the ledger
  discipline — documented, structural, not name-based).
* `autotuner_runtime.{c,h}` + `parallel_runtime.{c,h}`: `sgpl_exec_ctx` carries
  `step_id`; `autograph_frontier_execute` dispatches every step through
  `sgpl_exec_step_dispatch`, which
  1. runs a **bounded calibration** of honest single-thread passes (the
     runtime's own `SGPL_C_INITIAL_BATCH_SAMPLES`-driven state machine decides
     when it is done: `sgpl_step_site_ready`),
  2. records the stage's **own measured pass time** as the site's cost sample
     (work estimate = the graph's arc count, span = the partition count — all
     derived, no per-stage constants),
  3. then wraps the dispatch in a **single-site TDG level**: the planner decides
     the width, the pool grants it, and the ledger bounds it.
* `parallel_runtime.c`: the pool rule is now general — **any caller without a
  plan entry** (unregistered, or a site the plan did not enable) shares the
  level's remaining budget instead of being serialised.

Verified on pagerank (4 threads): `[tdg.single-site] chosen_workers=1
site_threads=3 loop_id=2 level_budget=4`, output unchanged
(`rank_total 1`, `rank_0 5.792550846249e-05`); the harness gate passes in all
three configurations; pagerank/cc/bfs_level/nested_gather/doall_scaling remain
bit-identical 1thr vs 4thr.

### 5.2 Root causes measured this session (correcting the plan)

1. **The TDG level collector keys on wrapper calls that the live path never
   emits.** `groups=0` for both test programs (`[pdg-tdg] extracted=5/16 tasks
   levels=1`), while the wrapper function exists in the binary
   (`wrapper_task_1_foreach.cond16.parallel`) and `main` calls only
   `sgpl_should_parallelize_doall` + `parallel_for_runtime`. The historic
   `benchmark_logs/…/after.ll` shows `call @task_5_wrapper` + one
   `sgpl_run_tdg_level` call; the current DSL binaries have **zero** such calls
   (objdump) and zero surviving `sgpl.loop.desc` globals.
2. **Level sites must be profiled before the chooser will plan them**
   (`regime_valid && c_sampling_state == STABLE && c_rank_ns > 0`). Measured in
   the harness: a site without serial samples yields `loop_sites=0`, no entries,
   and its dispatch falls back to one thread; after training the site, T7 plans
   it. An engine step therefore needs profiling hooks (or compiler-emitted
   samples) to become plannable.
3. **Do not wrap a single-step level naively**: single-task levels were decided
   serial *by design*, and (before the T7 change) that serialised the site inside
   them.

### 5.3 Residuals — closed with evidence

1. **Joint planning of sibling sites is superseded, not missing.** The historic
   level design ran sibling loop *tasks concurrently* on the level's workers
   (`sgpl_tdg_worker_main` pulls tasks by atomic index), which the live inline
   versioned dispatch deliberately replaced: today sibling loops run
   sequentially, each with its own per-loop TDG decision, and engine steps get
   their own single-site levels.  Re-introducing concurrent sibling execution
   would revert that dispatch refactor and change the algorithms' execution
   structure without fixing a correctness or budget gap (the ledger already
   bounds every dispatch).  Recorded here so the `groups=0` state is a decision,
   not an open bug.
2. **A step inside an outlined loop is refused by design, with a pinned check.**
   `verify/cases/parallel/nested_step.graph` (new) nests a traversal inside a
   `while` loop: the classifier reports `call barrier: stateful call
   autograph_build_clean_cut` -> `SEQUENTIAL` for the outer loop (its body holds
   the rewrite's setup call), while the step inside is still TDG-planned (12
   `[tdg.single-site]` decisions, `site_threads=3`) and the answer is
   `checksum 200` at 1 and 4 threads.  The suite check `parallel/nested_step`
   asserts all three facts on every run.
3. **Fork/join stages stay `-1`** (their parallelism is the fork/join; their
   inner dispatch keeps the ledger discipline) — documented at the emission
   site.
4. **The kill switch stays.** `SGPL_NO_TDG_ENGINE=1` restores the pre-change
   dispatch exactly (verified: identical output), consistent with the project
   rule that every behaviour change ships with an A/B switch.

**Race/regression evidence for the landed changes** (all on the current tree):

* Full suite, clean serial run: **71 passed / 17 failed / 1 skipped**; every
  failure is the pre-existing `class=` vocabulary gap (16 `class/*` checks plus
  `race/dual_shadow`).  The skip is the load-aware scaling guard.
* pagerank, cc, bfs_level, nested_gather and doall_scaling are bit-identical
  1thr vs 4thr.
* **The two priv fixtures now pass deterministically** (`priv/mixed_regions` =
  340000, `priv/data_index_big` = `cntsum 160000 cnt1 1335`, 5/5 runs each at
  4 threads × 3 partitions).  The reason is the dispatch clamp introduced here:
  a single-stage dispatch never requests more threads than work items, so a
  partition's state can no longer be touched by two workers at once — that was
  the multi-writer condition behind the deficits.  **Caveat:** the underlying
  emit gap documented in `PRIVATIZATION_REDIRECT_BUG.md` (the pair body writes
  the shared array instead of the partition's private copy — still visible in
  the disassembly) is unchanged, so cross-partition element collisions remain a
  latent hazard; the clamp only removes the per-partition one.
* Engine-step evidence: pagerank with `SGPL_TDG_DEBUG=1` shows one
  `[tdg.single-site] chosen_workers=1 site_threads=3 loop_id=2 level_budget=4`
  per post-calibration step execution (16 in one run) and identical output with
  the integration on and off (`SGPL_NO_TDG_ENGINE=1`).

---

## Appendix A — reproduce every measurement

```bash
cd p1GraphEasy-con-AutoTuner

# pagerank (effect-system only)
rm -f final_program
GRAPH_FILE=../verify/cases/algo/pagerank.graph bash ./03_run.sh
SGPL_TDG_DEBUG=1 GRAPH_PARALLEL_DEBUG=1 ./final_program >/dev/null 2>/tmp/pr.err
grep -c '\[tdg' /tmp/pr.err ; grep -c '\[budget' /tmp/pr.err
grep -o 'threads=[0-9]*' /tmp/pr.err | sort | uniq -c
nm final_program | grep -c 'sgpl.loop.desc'

# the same for a PDG-outlined DSL program
rm -f final_program
GRAPH_FILE=../verify/cases/parallel/doall_scaling.graph bash ./03_run.sh
SGPL_TDG_DEBUG=1 GRAPH_PARALLEL_DEBUG=1 ./final_program >/dev/null 2>/tmp/ds.err
grep -c '\[tdg' /tmp/ds.err ; nm final_program | grep -c 'sgpl.loop.desc'
nm final_program | grep -o 'outlined_task_[^ ]*' | head

# historical TDG activity lives in the research kernels, not the DSL:
grep -rl 'sgpl.loop.desc' benchmark_logs | head
```

Raw results seen on 2026-09-24: pagerank 0/0 TDG/budget lines, 8 dispatches at
`threads=4`; array_doall 0/0; doall_scaling 0/0, 9 dispatches at `threads=4`;
descriptor count 0 in all three binaries; `benchmark_logs/doacross_fix_*` contain
descriptors.

## Appendix B — code index

| thing | file:line |
|---|---|
| descriptor builder | `parallel_loop_outline.cpp:2863`, call `:2937`, guard + caller `:2922`, `:3037` |
| level collector + `sgpl_run_tdg_level` emission | `pdg.cpp:4151`, `:4200-4210` |
| effective threads for a loop id | `parallel_runtime.c:2260-2285` |
| level pool acquire (forced 1 for unregistered) | `parallel_runtime.c:2333-2400` (the `loop_id < 0` case at `:2341`) |
| budget API + ledger | `parallel_runtime.c:272-460`, `:5193-5290` |
| scope enter/exit helpers | `parallel_runtime.c:697-716` |
| pending-range TLS (pattern to mirror) | `parallel_runtime.c:500-520` |
| engine entry points | `graph_frontier_lowering.cpp` (emission), `autotuner_runtime.c` (`autograph_frontier_execute` → `parallel_for_runtime`) |
