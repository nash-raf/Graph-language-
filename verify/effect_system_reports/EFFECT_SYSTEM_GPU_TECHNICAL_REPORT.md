# Effect System — GPU technical report

Complete description of the implemented effect system on the device path, in the
same shape as the CPU report: **theory first**, then **the code**, then the
**logic**.  Files: `gpu_runtime.[ch]` (device runtime: cost/policy models, the
two-axis boundary, registry, launchers, M2 streams, M3 device budget, pointees),
`autotuner_runtime.c` (the two device decision sites, the refusal trace, the
dispatch budget interaction), `graph_frontier_lowering.cpp` (device module
assembly: step kernels + PTX), harnesses `verify/gpu_device_diff.sh`,
`verify/gpu_corpus_diff.sh`, `test/tdg_budget_test.c` (T10/T11).

---

## 0. The device as a realization of the same certificate

**Theory.**  The device path is not a second semantics.  A device step is one
*realization* of the same per-axis result the CPU stage realizes, so:

1. the **certificate decides first**, the cost model is only a tiebreaker;
2. the *spatial* axis gates the device (the device dispatch runs internally
   parallel blocks, so it needs the stage's own dispatch to be licensed
   concurrent);
3. the *temporal* axis never gates — a temporal witness must not silently
   disable the device step, because a device step is one whole round and the
   round order belongs to the caller;
4. every refusal **falls back to the CPU path**, whose answers are identical by
   construction, and every refusal is traced with its reason.

**Code.**

```c
/* gpu_runtime.c -- the two-axis contract at the device boundary (pure) */
int sgpl_gpu_step_schedule_ok(int32_t spatial, int32_t temporal)
{
    (void)temporal;
    return spatial >= 0;
}
```

**Logic** — the semantic table this encodes:

| declaration | decision | why |
|---|---|---|
| `spatial = 1` (concurrent) | device licensed | the stage's own dispatch is parallel, so the device's internal parallelism realizes it |
| `spatial = 0` (undeclared, legacy callers) | device allowed | historical behavior for callers without a certificate |
| `spatial = -1` (held in order — witness-backed) | **refused** → CPU | an internally parallel device dispatch would realize concurrency the certificate does not license |
| any `temporal` | never affects the decision | a temporal witness must not remove device work; the round order is the caller's |

---

## 1. Cost and policy models (pure, unit-tested)

**Theory.**  A device step pays a launch, per-round state copies (slices,
membership, claim marks, pointees, write-back) and, for cooperative kernels, one
grid sync per wave.  The models that decide *when the device pays off* are pure
functions so they can be tested without a GPU.

**Code.**

```c
/* Engine-step cost gate: below the arc floor the step stays on the CPU. */
int sgpl_gpu_engine_step_verdict(int64_t arcs, int64_t min_pairs)
{
    if (min_pairs > 0 && arcs < min_pairs) return SGPL_GPU_SMALL_TRIPS;
    return SGPL_GPU_OFFLOAD;
}
```

```c
/* DOACROSS policy: a distance-1 recurrence needs one grid sync per iteration,
 * so its wave count (trip / distance) is refused on cost grounds and the CPU
 * doacross path runs it.  SGPL_GPU_OFFLOAD | SGPL_GPU_SMALL_TRIPS |
 * SGPL_GPU_WAVE_STORM. */
int sgpl_gpu_policy_verdict(int64_t trip, int32_t needs_doacross,
                            int64_t doacross_dist, int64_t min_trips,
                            int64_t max_waves);
```

**Logic.**  Both are *floor* models: `min_pairs <= 0` disables the gate (the
harnesses use that to force device execution); `SGPL_GPU_ENGINE_MIN_PAIRS`
overrides the floor at run time.  Their known mismatch with reality
(model light=7/heavy=7 vs measured light=1/heavy=13 on the pod) is why the
*DAG* path takes its work estimate from measured per-partition pair counts
instead (CPU report §10) and why the cost gate is consulted **after** the
certificate gate, never before it.

---

## 2. Device decision sites in the runtime

**Theory.**  A device step must be derived from the operation's own resource
facts, not from a strategy name: the stage must declare the destination
envelope (claim mark + append) and must not declare partial/private resources;
per-source lifecycle operations belong to the CPU partition body.

**Code** (`autotuner_runtime.c`, step runner — the destination-owned gate):

```c
if (!have_pair || !have_seen || !have_next) {
    sgpl_gpu_step_refuse(ctx->step_id,
        "the destination envelope facts (claim + append) are not declared");
    return 0;
}

/* Two-axis contract at the device boundary: the certificate decides before
 * the cost model does. */
if (!sgpl_gpu_step_schedule_ok(sgpl_ctx_dag_spatial(ctx),
                               sgpl_ctx_dag_temporal(ctx))) {
    sgpl_gpu_step_refuse(ctx->step_id,
        "schedule: the spatial axis is held in order (theorem); "
        "the device dispatch cannot preserve it");
    return 0;
}

/* Cost gate ... */
if (sgpl_gpu_engine_step_verdict(arcs, min_pairs) == SGPL_GPU_SMALL_TRIPS) {
    snprintf(why, ..., "cost model: %lld arcs < min_pairs=%lld", ...);
    sgpl_gpu_step_refuse(ctx->step_id, why);
    return 0;
}
```

and the source-owned gate adds the lifecycle exclusion before the same two
gates:

```c
sgpl_gpu_step_refuse(ctx->step_id,
    "per-source lifecycle ops are part of the CPU partition body");
return 0;
```

Refusals are printed once per step id and only under `SGPL_GPU_DEBUG`:

```c
static void sgpl_gpu_step_refuse(int32_t step_id, const char *why) {
    static int32_t seen_id[8]; static int seen_n = 0;
    if (!getenv("SGPL_GPU_DEBUG")) return;
    for (i = 0; i < seen_n; ++i) if (seen_id[i] == step_id) return;
    ...
}
```

**Logic — guard order (normative).**
1. resource facts (envelope / lifecycle) — *is a device step possible at all?*
2. **schedule** (`sgpl_gpu_step_schedule_ok`) — *does the certificate license it?*
3. cost model — *does it pay off?*
4. per-partition launch (`gpup_step_try` / `gpup_step_v_try`), each returning 0
   on failure → the caller keeps the CPU path.

---

## 3. Launchers, fallback contract, M2/M3 resources

**Theory.**  The launchers must be transparent: on any failure (no driver, no
device, ineligible body, refused policy) the loop runs on the CPU thread pool
via the provided body, with identical results.  Two optional resources exist:
**streams (M2)** for temporal overlap and a **device block budget (M3)**; both
degrade to defaults when unavailable.

**Code.**

```c
/* gpu_parallel_for_runtime(...) -- DOALL on the device, CPU fallback via `body`;
   DOACROSS is launched cooperatively (cuLaunchCooperativeKernel, grid capped at
   the occupancy limit so grid.sync() is legal).  PTX comes from the embedded
   `gpu_embedded_ptx` symbol, else "kernels.ptx" in the CWD. */
```

```c
/* M2: streams for the temporal axis; absent -> default stream. */
/* load_cuda(): cuStreamCreate / cuStreamSynchronize resolved optionally */

/* M3: device block budget */
void gpup_set_block_budget(int32_t blocks)
{
    g_block_budget = blocks > 0 ? blocks : 0;
    if (getenv("SGPL_GPU_DEBUG"))
        fprintf(stderr, "[gpu] block budget set (M3): %d blocks\n", g_block_budget);
}

/* pointees: the language models an array as a pointer variable, so the device
   copy stores the *pointee* buffer registered for that name; unregistered
   pointer globals refuse the offload. */
void sgpl_gpu_register_pointee(const char *name, void *base, int64_t bytes);
void sgpl_gpu_register_buffer(void *base, int64_t bytes);
```

Step kernels are addressed by name/id through a small registry:

```c
void autograph_gpu_step_register(const char *name, int32_t step_id);
int  autograph_gpu_step_count(void);
const char *autograph_gpu_step_name(void);
int32_t autograph_gpu_step_id(void);
```

**Logic — budget interaction with the scheduler.**  A device step executes
inside a scheduler node (or a plain round); its *host-side* work (slice setup,
write-back, and the CPU fallback) goes through the same ledger clamp as any
other dispatch (`sgpl_current_thread_budget`), so a device step inside a DAG
node can never exceed the node's grant.  The M3 block budget is a *device*
resource and is deliberately separate from the host thread budget.

---

## 4. Compiler side: device emission

**Theory.**  A device step is emitted only where the shape's own facts say the
device realization can honour the verdict; the emission carries the step kernel
(the destination-owned activation kernel, one per step) and the registration
that ties the kernel name to the step id.

**Code** (`graph_frontier_lowering.cpp`).

```cpp
static Function *emitGpuEngineStep (...);   // destination-owned activation kernel
static Function *emitGpuEngineStepV(...);   // the layout/V variant
...
if (getenv("SGPL_GPU_EMIT_DEBUG")) ...      // prints the emission facts
```

**Logic.**  The device module is assembled by `parallel_loop_outline.cpp`
(activate stub, claimed global, PTX `sm_70`), embedded into the executable
(`gpu_embedded_ptx`) and optionally written as `kernels.ptx`; a build without
the NVPTX toolchain prints `NVPTX target unavailable` and the runtime falls back
to the CPU.  The **dual/staged path emits no kernel** — which is why
`dual_same_base`'s differential reports 0 device dispatches: the staged phases
are plain CPU partition dispatches, and the device path is not claimed for them
today (a deliberate scope, not a refusal).

---

## 5. Evidence (GPU)

| check | result |
|---|---|
| `tdg_budget_test` T10 (engine-step gate) | floor semantics: just-below → CPU, at/above → device, non-positive floor disables the gate |
| `tdg_budget_test` **T11** (two-axis boundary) | `(1,1)→1`, `(-1,1)→0`, `(1,-1)→1`, `(-1,-1)→0`, `(0,0)→1` — all pass in all three configs of `validate_tdg_budget.sh` (63 PASS / 0 FAIL) |
| `gpu_device_diff.sh carried_read_state` (pod) | P=1/4/8 identical; `kernels.ptx` 1918 B; **16 device dispatches / 1 CPU fallback** |
| `gpu_device_diff.sh shadow_snapshot` (pod) | P=1/4/8 identical; 16 dispatches / 1 fallback |
| `gpu_device_diff.sh dual_same_base` (pod) | P=1/4/8 identical; 0 device dispatches (no kernel emitted for the staged path — by design) |
| `gpu_corpus_diff.sh` (pod, 83 rows) | **0 DIFF** — device-on == device-off for every fixture that ran (76 rows); 6 rows `SIGKILL(rc=137: the program run was killed -- container memory limit)` reproduced and attributed; **0 BUILD-FAILED** after the honest labeling fix |
| environment | pod is a quota-capped container: 5.1 CPUs (`nproc` 48) and 31 GB (host 251 GB); NVPTX LLVM without Polly; `/usr/bin/time` absent (the harness falls back to `date` and reports environment SKIPs with the numbers) |

---

## 6. Limits and explicit scope

- The device **never** consumes the temporal axis: a temporal witness cannot
  disable device work, and the device step cannot violate temporal order because
  it is a whole-round realization executed by the caller's round sequence.
- The cost models are floors only; the measured mismatch (light=7/heavy=7 vs
  light=1/heavy=13) is bypassed on the DAG path via measured pair counts and is
  never allowed to override the certificate.
- Undeclared schedules keep the legacy device behavior (backward compatible);
  the two-axis gate is a *monotone* constraint — it can only remove device
  attempts for stages whose spatial axis is explicitly held in order.
- Device-vs-CPU parity is enforced by the differential harnesses rather than by
  argument: every fixture must be byte-identical with the device step off and
  on, and the corpus diff must stay at 0 DIFF.
