# The effect system on the GPU — how the port works

p1GraphEasy-con-AutoTuner · technical report · 2026-10-04

---

## 0. Scope and one-paragraph summary

This report explains **what the effect system is**, **what a verdict licenses**, and **how
those verdicts are realized on the GPU** in p1 — the code that does it, and the exact flow of
a round from the compiler's decision to a device launch and back.  It closes with what was
verified, what is deliberately still on the CPU, and why.

The short version: the effect system stays a *compile-time decision layer* and its verdicts are
**unchanged** for the device; what we built is a second set of *executors* that honor the same
guarantees.  A device kernel exists only for a shape whose declared facts the device realization
can honor, and every other shape falls back to the CPU partitions.  The verified property is
**bit-identical results** between the CPU and device executions of the same program.

---

## Part I — Theory

### 1.1 What the effect system is

For every loop the compiler asks two questions: *may this run concurrently?* and *what shape does
that concurrency have?*  It answers from **effect facts about memory**, never from the loop's name
or its text.  An *effect* summarizes one operation's interaction with a *resource*:

| component | meaning | examples in p1 |
|---|---|---|
| resource | the thing touched | a vertex-indexed array, the frontier membership, the destination set, the append buffer, a partial slot, a private copy |
| access mode | how it is touched | read, write, atomic write, private |
| index space | what it is keyed by | the pair's source `u`, the pair's destination `v`, the loop iteration |
| temporal label | when the value comes from | same round, previous round |

The temporal label is the part that is easy to forget and hard to fake.  `SameRoundRead` means "I
read a value another iteration may be writing in this same round"; `PreviousRoundRead` means "I
read the value as of the round boundary".  A *round-separated* base is one that must be frozen
before the round begins.

### 1.2 The algebra, and the vocabulary it produces

Per-iteration effects are combined into a loop-level verdict by rules over those facts:

* conflicting writes on the same key with no ordering → **dependence**; if regular, the extraction
  also yields a **distance `d`** (a *Carried* recurrence);
* a previous-round read next to a same-round write → admissible only with **round separation**
  (a snapshot);
* a guarded write whose loser must not proceed → a **Claim**, admissible when the key has a single
  owner (first-wins);
* a fold with an associative operator → **reduction**, realized over partial slots plus a combine;
* anything that cannot be discharged → **refusal, with a named reason**.

The output vocabulary is small and explicit:

```
Independent                      -- all iterations commute
Carried doacross_dist = d        -- ordered recurrence with distance d
Activation (Claim + Append)      -- first-wins on a keyed object, plus a frontier append
Reduction / Privatised           -- fold over slots / per-unit copies
<refusal with a reason>          -- e.g. "same-round read without a snapshot"
```

### 1.3 What a verdict licenses (the guarantees an executor must preserve)

A verdict is not a scheduling suggestion; it is a *licence to execute concurrently*, valid only if
the executor preserves the guarantees the verdict was proved from:

1. **Single writer per key** — for an owner-computes decomposition, each key's writer is unique.
2. **First-wins claims** — where a `Claim` resource exists, exactly one writer may win the
   transition; the lost writers must not proceed.
3. **Round-stable reads** — every `SameRoundRead` must observe the round-boundary value.
4. **Associative-only folds** — reductions may be re-associated but **not** permuted arbitrarily:
   the CPU folds partition partials in ascending partition order, and that order is part of the
   contract ("associativity is the law; permutation invariance is never assumed").
5. **Deterministic composition** — the observable result must not depend on scheduling.

### 1.4 The CPU executor these guarantees were written against

The CPU realization in p1 is a **CleanCut** engine (the name and the partitioning idea come from
Graptor, ICS 2020: *"a graph partitioning approach that rules out inter-thread race conditions"*):

* the graph is partitioned by the **owner** of the write — sources for source-owned shapes
  (`OWNER_U`), destinations for destination-owned shapes (`OWNER_V`);
* one **worker per partition**, and inside a worker the rows/sources are walked **sequentially**;
* `source_begin` / `source_end` lifecycle callbacks run around each source's pairs (`OWNER_U`) or
  as coverage passes (`OWNER_V`);
* the frontier append is an **atomic ticket** and the claim an **atomic CAS**
  (`autograph_frontier_activate`), because the append buffer and the claim set are shared across
  partitions;
* `SameRoundRead` is served by a published **snapshot** (the shadow array), refreshed once per
  round before any traversal.

Because ownership is per key and the owner works serially, guarantee (1) holds by construction and
(2) holds *by schedule* — the CAS is a belt-and-braces second check, not the primary reason.

### 1.5 What is genuinely different on a GPU

The GPU does not invalidate the guarantees; it changes which mechanisms can provide them:

| machine property | consequence for the port |
|---|---|
| SIMT execution: a warp advances only when all its lanes agree | irregular work (degree skew, hub vertices) causes divergence and load imbalance; a *work assignment* designed for it is needed |
| no shared host memory: arguments, state and results must be copied | per-round copies are a real cost; they dominate below a few hundred thousand arcs |
| blocks cannot synchronize with each other without a **cooperative grid sync** | an ordered recurrence cannot be "pinned to a worker" as on the CPU; it becomes a bulk-synchronous **wave** kernel |
| thousands of threads, no persistent identity | "one worker per partition, walked in order" becomes "one thread per owned row/source, walked in order" — the same *ownership*, a much finer *unit* |
| atomics are available but contended; **idempotent writes are free** | first-wins can be realized by ownership + idempotent same-value writes instead of a CAS |
| launches are coarse, and each launch is a barrier | rounds become launches; a device-resident loop is the way to avoid that (not yet done) |
| a cost model is needed because offload can lose | the device path is gated by a **pure policy** with measured thresholds |

### 1.6 The design principle adopted

**Verdicts identical, mechanisms analogous, and eligibility derived from facts.**

* The device executor consumes the *same* effect verdicts; no GPU-specific verdict exists.
* The device *mechanism* for a verdict may differ from the CPU's (wave kernel vs ordered partition,
  marks vs CAS+ticket, thread-per-row vs worker-per-partition) as long as the guarantee is
  preserved.
* Whether a step may run on the device is decided by **reading the step's own resource table** —
  the masks a compiler emits next to each operation — never by matching a name or a shape string.
* Anything not provably honorable on the device **falls back to the CPU** (fail-closed).  A wrong
  answer must never be reachable by default; that is why the one kernel variant that misbehaved
  (per-arc pull activation) was removed rather than left behind an environment variable.

---

## Part II — Verdict → realization mapping

| verdict / fact | CPU realization | GPU realization | how the guarantee is preserved |
|---|---|---|---|
| `Independent` (a DOALL loop) | outlined parallel loop over a range, worker pool | **outlined kernel**, one thread per iteration, grid-strided | iteration space is disjoint by construction; verified 1thr == 4thr and across repeats |
| `Carried doacross_dist = d` | static partitioning with the recurrence ordered inside a worker; `doacross.wait/post` metadata | **wave kernel**: all threads compute wave *k*, cooperative **grid sync**, wave *k+1*; `waves = ceil(trips/d)` | the synchronisation edge is explicit; verified against CPU, plus a classification assertion that the loop really is DOACROSS with wait/post metadata |
| `Claim` on the destination set + frontier append (BFS activation) | destination-partitioned rows; first-wins by ownership and serial row order; CAS + atomic ticket in `autograph_frontier_activate` | **one thread per owned row**, arcs walked in order, frontier-membership gated; the append becomes a byte **mark** `claimed[v] = 1`; the host compacts the marks into the next frontier in ascending vertex order | single ownership per destination makes first-wins exact; concurrent claimers write the *same* value (round-homogeneous frontier), so the writes are idempotent; the mark is idempotent too — **no atomics anywhere in the module** |
| source-owned step (kcore degree phase, PageRank sweeps) | source-owned flat slices, one worker per partition | **one thread per source**, its pair run walked sequentially | ownership is per source; per-pair parallelism would lose read-modify-write updates (measured: device sums collapsed to ~1/16 of the arcs), which is why the unit is the *source* |
| `SameRoundRead` (snapshot) | `Snapshot(A)` op publishes a frozen buffer at round begin | the same buffer is **registered as a device pointee** under the name of the module global the body reads, materialized and re-uploaded **every launch** | the value read is the round-boundary value; re-upload is required because the buffer is refreshed per round (an upload-once cache was a real bug) |
| reduction (`partial` slots + combine) | per-worker partials folded in ascending partition order | partials + combine kernel in the **outlined** path (ported & verified); **not** ported for engine steps | fold order contract honoured in the outlined path; engine-step case refuses |
| privatised (`private` copies) | per-partition private copies | private copies in the outlined path (verified in cross-mode) | per-unit copies keep writes private |
| fork/join of two concurrent children | two host threads with a join, shared round resources | **not ported** — the runtime has no device fork/join; children stay on the CPU | results unaffected; only the overlap is lost |
| shapes needing staged per-source claim state | source lifecycle stages S[u][j]; pairs consume it | **not ported** — refused | requires device-side claim callbacks and a materialized state array |

Measured device-vs-CPU ratios for the same work (forced device, gates off, median of 3):

```
DOALL (1k .. 1M trips)                0.74 .. 1.16   ~ break-even
DOACROSS  waves <= 8192               0.09 .. 0.26   device loses badly
DOACROSS  waves == 8192               1.13           the one clear win
DOACROSS  waves == 16384              0.16           the cliff past the cap
engine step (source-owned, 320k arcs) 0.41 .. 0.62   device loses (copies dominate)
```

That table *is* the cost model's justification: `min_trips = 4096`, `max_waves = 8192`,
engine-step floor `2M arcs`.

---

## Part III — The code

### 3.1 Where the decisions live

| file | what it does |
|---|---|
| `graph_frontier_lowering.cpp` | the effect algebra and all emission: `summarizeEffects` → `supportedByAlgebra` → `deriveExprFacts` → `emitExprInterp`; builds the op descriptors and the execution context; emits the resource tables; emits the device step kernels (`emitGpuEngineStep` for source-owned, `emitGpuEngineStepV` for destination-owned activation), their registration call, and the pointee registrations; publishes round snapshots (`emitRoundSepShadow`) |
| `autotuner_runtime.c` | the CleanCut engine itself: `autograph_build_clean_cut` (partitions, slices, destination rows), `sgpl_exec_partition_body` (the CPU partition body), `sgpl_exec_step_dispatch` (the dispatch point), `autograph_frontier_activate` (CPU CAS + ticket), the snapshot publish implementation, and the **engine hook** where the device step is attempted |
| `gpu_runtime.c` / `.h` | the driver-API runtime: `dlopen("libcuda.so.1")`, context, module load (`kernels.ptx` or the embedded payload), the pure cost policies (`sgpl_gpu_policy_verdict`, `sgpl_gpu_engine_step_verdict`), the **step registry keyed by step id**, the launchers (source-owned `gpup_step_try`, activation `gpup_step_v_try`), pointee materialization, and the claimed-mark read-back |
| `parallel_loop_outline.cpp` | the **device module builder** `emitGpuKernels`: collects the kernel plus its callees and referenced globals, re-homes external declarations, defines the device side of `autograph_frontier_activate` (the mark writer) and the `sgpl_gpu_claimed` global, and emits PTX for `sm_70` |
| `main.cpp` | backend selection (`auto|cpu|gpu`, `FORCE_GPU`/`FORCE_CPU`), the `kernels.ptx` output, and the eager device bring-up before any profiling run |

### 3.2 The facts are machine-readable

The verdict is not left implicit: each emitted operation carries a **resource table** of
`{mask, access}` pairs, and the context carries the traversal kind and the domain kind.  This is
what makes fact-based eligibility possible:

```c
/* resource masks (autotuner_runtime.h) */
SGPL_RES_MEMBERSHIP   SGPL_RES_DEST_SEEN   SGPL_RES_NEXT_FRONTIER
SGPL_RES_SNAPSHOT     SGPL_RES_PARTIAL     SGPL_RES_PRIVATE     SGPL_RES_CLAIM
/* access modes */
SGPL_ACCESS_READ = 0, WRITE = 1, ATOMIC_WRITE = 2, PRIVATE = 3
/* traversal */
SGPL_TRAVERSE_OWNER_U = 0   /* source-owned flat slices  (src_pairs)          */
SGPL_TRAVERSE_OWNER_V = 1   /* destination-owned rows   (push_rp/ci/indir)   */
/* domain */
SGPL_DOMAIN_ALL_VERTICES = 0, SGPL_DOMAIN_FRONTIER = 1
```

For example, a BFS activation op declares exactly
`MEMBERSHIP:READ, DEST_SEEN:ATOMIC_WRITE, NEXT_FRONTIER:ATOMIC_WRITE, SNAPSHOT:READ, CLAIM:ATOMIC_WRITE`
— the facts that say "destination-owned first-wins claim with a frontier gate, plus an append, plus
a round-stable read".  The device gate reads that table; it never asks what the loop looks like.

### 3.3 The device step: emission

Two kernel shapes are emitted by the frontier lowering, together with their registration:

```
/* source-owned step (U) -- one thread per source, its pair run in order */
gpu_step_<fn>_<pairfn>(i32* pairs, i64 npairs, i32* rows, i64 nrows, i64* begins, i8* env)

/* destination-owned activation step (V) -- one thread per owned row */
gpu_step_v_<fn>_<pairfn>(i32* rowsrc, i64 nrows, i64* rowptr, i32* arcs, i8* mem, i8* env)
```

* The **pair body is the same function** the CPU calls (`sgpl_pair_work`), cloned into the device
  module — so the arithmetic and the guards are identical by construction.
* Emission also emits `autograph_gpu_step_register(<name>, <step id>)` into the program, and
  `sgpl_gpu_register_pointee(<global name>, <base>, <bytes>)` for every pointer global the body
  touches (the DSL arrays, the round shadow).
* The body's call to `autograph_frontier_activate(ctx, v)` is left alone in the host module; the
  device module *defines* its own version (below).  The eligibility check verifies that the body
  uses its state/context arguments *only* for that call.

### 3.4 The device step: module assembly

`emitGpuKernels` builds a fresh module containing only what the kernels need:

1. collect each kernel plus its callees and the globals they reference (`graph.gpu.kernels`
   metadata lists the kernels; names containing `sgpl.` are host-only and skipped);
2. **re-home external callees** — the pair body calls `autograph_frontier_activate`, which in the
   host module is only a declaration (its CPU definition lives in the runtime object).  The device
   module gets a declaration with the same signature, so the cloned IR is valid;
3. **define the device activation primitive** and the claim map it writes:

```llvm
; emitted into the device module only
@sgpl_gpu_claimed = external global i8*          ; patched by the runtime, per launch

define i32 @autograph_frontier_activate(ptr %ctx, i32 %v) {
  %base = load ptr, ptr @sgpl_gpu_claimed
  %idx  = sext i32 %v to i64
  %slot = getelementptr i8, ptr %base, i64 %idx
  store i8 1, ptr %slot          ; no CAS, no ticket: the mark is idempotent
  ret i32 1
}
```

4. hand the module to the NVPTX backend targeting `sm_70` (the driver JITs it for any Volta or
   newer card).

### 3.5 The runtime: policy, registry, launchers

**Policy** (pure, unit-tested without a GPU):

```c
int sgpl_gpu_policy_verdict(int64_t trip, int needs_doacross, int64_t dist,
                            int64_t min_trips, int64_t max_waves);
/*  trip <= 0                                   -> SMALL_TRIPS (CPU)
 *  min_trips > 0 && trip < min_trips           -> SMALL_TRIPS (CPU)
 *  doacross && ceil(trip/dist) > max_waves     -> WAVE_STORM  (CPU doacross)
 *  otherwise                                   -> OFFLOAD                              */

int sgpl_gpu_engine_step_verdict(int64_t arcs, int64_t min_pairs);
/*  min_pairs > 0 && arcs < min_pairs           -> SMALL_TRIPS (CPU partitions)
 *  otherwise                                   -> OFFLOAD                              */
```

Defaults: `SGPL_GPU_MIN_TRIPS = 4096`, `SGPL_GPU_MAX_WAVES = 8192`,
`SGPL_GPU_ENGINE_MIN_PAIRS = 2 000 000`; each knob takes `0` to disable the bound, which is how the
verification harness forces device execution.

**Registry.**  The registration call sits in the program's loop preheader and therefore executes
*once per round*; the registry is a set of `(name, step id)` pairs, deduplicated on re-registration,
and looked up **by the dispatching context's own step id** (`autograph_gpu_step_name_for(id)`).
(An earlier gate refused whenever more than one step was registered at all, which silently disabled
multi-step programs; and before deduplication the registry grew every round and tripped that same
gate from round 2 onward — both were real bugs.)

**Pointee materialization.**  A device step's arrays live on the host; the runtime keeps a device
buffer per registered name, and on **every launch** copies host → device, patches the module's
global to the device buffer, and after the launch copies device → host.  The upload cannot be
cached: the runtime rewrites these arrays between rounds (the snapshot refresh especially), and an
upload-once cache made round 2 read round-1 state.

**Claimed marks.**  For activation steps the runtime allocates a byte-per-vertex claim map, zeroes
it, patches `sgpl_gpu_claimed`, launches, copies the map back, and clears it for the next round.

### 3.6 The engine hook: the whole decision in one place

`sgpl_exec_step_dispatch` — the function the engine calls to execute one step — begins with the
device attempt and falls through to the CPU partitions on any refusal or failure:

```
sgpl_exec_step_dispatch(ctx, meta)
├─ env switch (SGPL_GPU_ENGINE_STEP / SGPL_NO_GPU_ENGINE_STEP)
├─ resolve the registered kernel for ctx->step_id   (no name → refuse)
├─ OWNER_V ?  → sgpl_gpu_step_try_device_v(ctx, meta)
│               ├─ kernel name must be an activation kernel (gpu_step_v_)
│               ├─ envelope must be wired: dest_seen, next_frontier, append_head
│               ├─ destination rows must be built (push_rp/ci/indir/row_count)
│               ├─ per-op facts: exactly one pair body; no source/partition/round hook;
│               │                 no PARTIAL/PRIVATE resource; DEST_SEEN and
│               │                 NEXT_FRONTIER both declared
│               ├─ cost gate: Σ arcs vs SGPL_GPU_ENGINE_MIN_PAIRS
│               └─ gpup_step_v_try(...)  →  on success:
│                    compact claimed marks ascending v:
│                       next_frontier[initial + k] = v ; dest_seen[v] = 1 ; k++
│                    append_head += k        (matching the CPU's activate)
└─ else OWNER_U → sgpl_gpu_step_try_device(ctx, meta)
                 ├─ kernel name must be a source-owned kernel (not gpu_step_v_)
                 ├─ full domain only (a membership-restricted step stays on the CPU)
                 ├─ no COMBINE cap, no SOURCE_BEGIN/SOURCE_END caps
                 ├─ cost gate: Σ src_pair_count vs floor
                 └─ gpup_step_try(...) per partition
   each refusal prints a once-per-step audit line under SGPL_GPU_DEBUG:
       [gpu] step 1 kept on the CPU: cost model: 320000 arcs < min_pairs=2000000 …
```

Everything after the hook — the combine phase, the coverage passes, `next_size` computation,
round-end ops — is the *unchanged* CPU engine code, which is why a device-executed step composes
with the rest of the engine without special cases.

### 3.7 The flow of one round, end to end (BFS activation)

```
1  compile time
   summarizeEffects → verdict: Claim(V) + Append + SameRoundRead(level)
   → emitSingleStage emits, in program order:
        Snapshot op    (publishes the round-start shadow)
        Pair op        (resources: MEMBERSHIP, DEST_SEEN, NEXT_FRONTIER, SNAPSHOT, CLAIM)
        ctx            (traversal = OWNER_V, domain = FRONTIER, step id = 1)
        activation kernel gpu_step_v_*  +  registration(name, 1)
        pointee registrations: visited, lvl, main_lvl_shadow_0
   → emitGpuKernels clones the kernel + pair body + globals into the device module,
     defines the mark-writing activate, emits PTX (sm_70), embedded in the binary
2  program start
   autograph_build_clean_cut builds the destination-partitioned rows;
   autograph_prepare_frontier_array fills the membership bytes for this round;
   the snapshot op runs on the host and publishes the shadow buffer, which is
   registered for the device under the module global's name
3  round r, engine dispatch
   sgpl_exec_step_dispatch → OWNER_V gate passes (facts + cost) →
   gpup_step_v_try:
        flatten partitions' rows → (rowsrc, rowptr, arcs) on device
        upload membership bytes; zero and patch the claim map; patch pointees
        launch: ceil(nrows/128) blocks × 128 threads
4  device
   thread i: u = rowsrc[i]; if (membership[u] == 0) return;
             for j in row i's arcs: pair(u, v):
                 if (visited[v] == 0) { visited[v] = 1;
                                        lvl[v] = shadow[u] + 1;
                                        claimed[v] = 1; }        /* mark, no atomics */
5  back on the host
   copy claimed back; walk v ascending; write next_frontier[initial + k];
   dest_seen[v] = 1; append_head += k
   copy pointees back (visited, lvl, shadow) so the host state is current
6  the unchanged engine continues: next_size = initial + append_head, round-end ops,
   and the DSL loop either runs another round or terminates on an empty frontier
```

The source-owned flow (kcore's degree phase, PageRank's sweeps) is the same shape with a different
unit: one thread per *source* over the source-owned slices (`gpup_step_try`, one launch per
partition), because the body's read-modify-write is only exact when a source is owned by one
thread and walked in order.

### 3.8 What the device must never do

* It must not run a step whose facts it cannot honor (lifecycle callbacks, slot resources,
  membership-gated source slices, an ambiguous step id) — those refuse with a printed reason.
* It must not run a shape whose earlier instantiation was wrong — the per-arc "pull" activation
  kernel is gone for that reason, not hidden.
* It must not answer when a launch, sync, module load or pointee materialization fails — every
  failure returns to the CPU partitions, whose answers are identical by construction.

---

## Part IV — What is verified, and how

| property | method | result |
|---|---|---|
| effect verdicts | harness assertions (`space=Independent`, `space=Carried`) | PASS |
| CPU budget model invariants | `tdg_budget_test` T1–T7 (clamp, desired width, pool sharing + kill switches, ledger balance, bounded nesting, planned width, single-site plan) × 3 configs | ALL PASS |
| device policy boundaries | T8/T9 (trips, distance → waves) + T10 (engine floor) | ALL PASS |
| device policy direction, end to end | gate the device across its threshold, require the audit line and the answer | 14/14 (box), 8/0/6 (CPU-only box) |
| device execution actually happens | per-case debug proof (`engine step ran`, `activation step ran`) | 11/0/2 harness |
| device == CPU, threads, repeats, partitions | cross-mode check: CPU 1=4 thr, device 1=4 thr ×2, `{1,3,7}` partitions | 11/11 both boxes; 50/50 broad |
| device == CPU on large graphs | 780k-edge and 157k-edge graphs | 3/3 |
| no atomics in the claim path | instruction count in the module's PTX | `atom.` count = 0 |
| above the engine floor (≥2M arcs) | the regime the default gate admits | **not measured** — for the 16M-edge build the program never registers a step (blocker identified, next work item) |
| region-model accuracy | predicted vs measured per region | 1.0–3.0× on the calibration machine; machine-dependent on pods; the model does not account for device time it will incur |

---

## Part V — What is deliberately left on the CPU, and why

The unifying reason: the port covers shapes whose semantics are a **pure function of the round and
the owned key**.  What remains needs **host-side callback state that has no device counterpart**,
or is an efficiency lever rather than a correctness gap.

1. **Staged per-source claim state** (`SOURCE_BEGIN` claim callbacks consumed by the pair body):
   the claim array and the callback bodies live on the host.  Requires emitting the callbacks as
   device functions with an exact first-wins claim, and materializing the state array.
2. **Partial/private slot resources inside engine steps**: the outlined path has them; the engine
   step refuses.  Requires device slots plus a fold that reproduces the ascending-partition order
   the CPU contract fixes.
3. **Membership-restricted source-owned steps**: the U slices carry all arcs of a partition's
   sources.  Cheap to add (upload membership, gate per source) — currently refused conservatively.
4. **Fork/join of concurrent children**: no device fork/join or inter-context barrier exists.
   Results are unaffected by running children on the CPU; only the overlap is lost.
5. **Efficiency levers** (not correctness): device-resident rounds with a device worklist (removes
   the per-round copies that cause the measured 2–2.5× loss below the floor); size-classed rows /
   hub splitting (one thread per row lets a hub serialize its warp); device-side compaction instead
   of host compaction; and the pull direction, which needs its arc→source mapping unit-checked
   against the row kernel's gate before another attempt.
6. **Cost-model coupling**: the region predictor assumes CPU execution, so an offloaded region is
   predicted wrongly (observed as a 118× apparent error that collapses to 2.96× when built
   CPU-only).  The fix is to teach the model about the offload it will actually take.

---

## Appendix — map, knobs, artifacts

**Environment knobs (all optional):** `SGPL_GPU_MIN_TRIPS` (4096), `SGPL_GPU_MAX_WAVES` (8192),
`SGPL_GPU_ENGINE_MIN_PAIRS` (2 000 000), `SGPL_GPU_ENGINE_STEP` / `SGPL_NO_GPU_ENGINE_STEP`,
`SGPL_GPU_DEBUG` (per-step audit lines, launch parameters, copy diagnostics), `FORCE_GPU`,
`FORCE_CPU`, `SGPL_GPU_BACKEND` (build-time backend), `AUTOTUNER_FORCE_LAYOUT` (CPU format study).

**Where to look:**

```
graph_frontier_lowering.cpp   effect algebra, resource tables, step + kernel emission
autotuner_runtime.c           CleanCut engine, dispatch hook, device gates, marks → frontier
gpu_runtime.c/.h              driver runtime, policies, registry, launchers, pointees
parallel_loop_outline.cpp     device module assembly (activate stub, claimed global, PTX)
verify/gpu_check.sh           device execution proof + equality + fallback
verify/gpu_cross_mode_check.sh  CPU vs device, thread counts, repeats, partition sweep
verify/gpu_broad_cross_check.sh every fixture, both backends
verify/gpu_cost_model_check.sh  policy direction, end to end
verify/plot_validation*.py    the figures (dashboard, loops, DOALL predicted-vs-actual)
```

**Standing rule:** a shape runs on the device only when its own declared facts say the device
realization can honor the verdict, and every refusal is printed with its reason; anything else runs
on the CPU, where the answer is identical by construction.
