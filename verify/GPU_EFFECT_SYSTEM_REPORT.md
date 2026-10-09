# Effect system on the GPU — realization report

How the same certificate that governs the CPU path is *mirrored* on the device:
what is emitted, what admits it at run time, how the two axes appear on a GPU, and
what deliberately stays on the CPU. Theory first, then the code for exactly that
theory. Code blocks are verbatim from the tree (trimmed with `…`); the few
pseudocode blocks are labelled. Compiler: `graph_frontier_lowering.cpp`; gates:
`autotuner_runtime.c`; driver runtime: `gpu_runtime.c`; device module assembly:
`parallel_loop_outline.cpp`; driver: `main.cpp`.

---

## 1. Theory

The device is not a second semantics. It is **one realization of the same
certificate**, and three rules make that true:

1. **The mirror is admitted by the certificate, never by a heuristic.** The
   compiler emits a device kernel only for shapes whose witness set is clear on
   the axis the kernel implements; the runtime re-checks the same facts before
   launching and falls back to the CPU partitions on any mismatch. Nothing runs
   on the device because it looked cheap.
2. **The work list is the same list.** The kernel walks exactly the CleanCut
   slices / destination rows the CPU partitions walk (`src_pairs` resp.
   `push_rp·ci·indir`), and the owner-computes assignment is the no-conflict
   guarantee on both sides: one pair per thread, no two threads own the same slot.
3. **The irreversible facts are reproduced by the host.** The device may *mark* a
   claim transition (`claimed[v] = 1`, idempotent) but must not own the frontier:
   the runtime replays exactly the transitions it marked (append + `dest_seen` +
   one `fetch_add` on the head). The round envelope and the combine stay on the
   host.

**Two shapes qualify today**, both from the certificate's own vocabulary:
destination-owned **activation** (`gpu_step_v_*`: claim + frontier append, one
pair body, no phase ops, no partial/private) and source-owned **full-domain**
(`gpu_step_*`: no membership restriction, no combine, no source lifecycle).
Everything else refuses with a *named reason* and runs on the CPU.

**Axes on the device.** *Spatial* = the grid/block decomposition of the walk plus
the **resident-block budget** — the CPU ledger's `max(1, W/A)` share mapped onto
grid sizing. *Temporal* = **launch order on one stream**, while the round
sequence itself stays host-driven: the device step never knows about rounds.

What a verdict *licenses* (the guarantees an executor must preserve): single
writer per key; first-wins exactness; round-stable reads; associative-only folds
in the fixed order ("associativity is the law; permutation invariance is never
assumed"); deterministic composition. A device path that cannot honour all five
for a shape refuses — that is the whole fail-closed story.

| verdict / fact | CPU realization | GPU realization | guarantee preserved by |
|---|---|---|---|
| `Independent` (DOALL) | outlined pool loop | outlined kernel, one thread/iteration, grid-strided | disjoint iteration space |
| `Carried doacross_dist=d` | ordered recurrence in one worker | wave kernel + cooperative grid sync | explicit sync edge |
| `Claim`(V)+append (activation) | destination rows, serial per row, CAS+ticket | one thread per owned row, membership gate, **byte mark**; host compacts marks ascending | single ownership ⇒ first-wins exact; marks idempotent (**no atomics in the module**) |
| source-owned step | flat `src_pairs` slices, worker per partition | one thread per source, its pair run in order | read-modify-write exact only per source (per-arc device sums collapsed, measured) |
| `SameRoundRead` (snapshot) | `Snapshot(A)` publishes a frozen buffer at round begin | same buffer registered as device **pointee**, re-uploaded every launch | value is the round-boundary value (upload-once was a real bug) |
| reduction (partials + combine) | ascending-partition fold | outlined path only; **engine steps refuse** | fold order is a contract |
| privatised copies | per-partition private copies | outlined path only | per-unit copies |
| fork/join children | two host threads + join | **not ported** (no device fork/join) | results unaffected; overlap lost |

---

## 2. Compiler side: what is emitted, and for which shapes

### 2.1 The two emitters and their guards

`graph_frontier_lowering.cpp:5401-5412` — source-owned (`gpu_step_*`): free of
round separation, claims, frontier append, reductions, accumulating
source-reduction, and the 4-argument pair ABI:

```cpp
static Function *emitGpuEngineStep(Function &F, Module *Mod, LLVMContext &Ctx,
                                   IRBuilder<> &RegB, NeighborLoopInfo &Info,
                                   Function *PairWF, StringRef Tag, int32_t StepId)
{
    if (!PairWF || !gpuBackendSelected(Mod))
        return nullptr;
    if (!Info.RoundSepBases.empty() || Info.HasFirstWins || Info.HasFrontierAppend)
        return nullptr;
    if (Info.ReducePtr || Info.AccConsumeStore)
        return nullptr;
    if (PairWF->arg_size() != 4)
        return nullptr;
```

`graph_frontier_lowering.cpp:5611-5620` — activation (`gpu_step_v_*`): V-shaped
*and* simple, *and* the step must actually append; then the argument-use
restriction that keeps the body device-resolvable:

```cpp
    if (!IsV || !IsSimple)
        return nullptr; /* destination-owned, no reduction/private slot machinery */
    if (!Info.HasFrontierAppend)
        return nullptr; /* the activation envelope */
    if (PairWF->arg_size() != 4)
        return nullptr;
    /* The body may not read its state slot and may touch the context only to
     * activate: the device resolves that call to its own definition, so any
     * other use of state/context (or any other callee) refuses. */
```

The pair body is the **same function** the CPU calls (`sgpl_pair_work`), cloned
into the device module — arithmetic and guards identical by construction. The two
kernel signatures:

```
gpu_step_<fn>_<pairfn>(i32* pairs, i64 npairs, i32* rows, i64 nrows, i64* begins, i8* env)
gpu_step_v_<fn>_<pairfn>(i32* rowsrc, i64 nrows, i64* rowptr, i32* arcs, i8* mem, i8* env)
```

### 2.2 The kernel walk (flat 1-D over the same work list)

Both emitters produce the same mapping — one thread per owned row,
`ctaid.x`-major (`:5454-5461` source-owned, `:5682-5690` activation):

```cpp
    Value *Bx = KB.CreateZExt(gpuReadSreg(KB, "ctaid.x"), I64, "bx64");
    Value *Tx = KB.CreateZExt(gpuReadSreg(KB, "tid.x"),  I64, "tx64");
    Value *Bd = KB.CreateZExt(gpuReadSreg(KB, "ntid.x"), I64, "bd64");
    Value *Lin = KB.CreateAdd(KB.CreateMul(Bx, Bd, "off"), Tx, "lin");
    Value *InRange = KB.CreateICmpULT(Lin, NrowsA, "in.range");
    KB.CreateCondBr(InRange, Body, Done);
```

Activation variant (`:5693-5700`) — source `u`, arc range, membership null-check
queued before the gate:

```cpp
    Value *RowsP = BB.CreateBitCast(RowsSrcA, I32P, "rowsrc32");
    Value *UPtr  = BB.CreateGEP(I32, RowsP, Lin, "row.ptr");
    Value *U     = BB.CreateLoad(I32, UPtr, "u");
    Value *PtrP  = BB.CreateBitCast(RowPtrA, I64P, "rowptr64");
    Value *Begin = BB.CreateLoad(I64, BB.CreateGEP(I64, PtrP, Lin, "begin.ptr"), "begin");
    Value *End   = BB.CreateLoad(I64, BB.CreateGEP(I64, PtrP,
                       BB.CreateAdd(Lin, ConstantInt::get(I64, 1), "row1"), "end.ptr"), "end");
    Value *MemNull = BB.CreateICmpEQ(MemA, ConstantPointerNull::get(cast<PointerType>(I8P)), "mem.null");
    BB.CreateCondBr(MemNull, Check, Gate);
```

There is **no stride loop** in this mapping — a fact M3 must respect (§4).

### 2.3 Registration, pointees, module

`graph_frontier_lowering.cpp:5498-5505` — the registration call sits in the loop
preheader (so it executes once per round) and every pointer global the body
touches gets a pointee registration:

```cpp
    FunctionCallee RegisterStep = Mod->getOrInsertFunction(
        "autograph_gpu_step_register",
        FunctionType::get(Type::getVoidTy(Ctx), {I8P, Type::getInt32Ty(Ctx)}, false));
    Value *NameStr = gpuCString(Mod, KName, "gpu.step.name." + KName);
    SmallVector<Value *, 3> StepArgs{NameStr, ConstantInt::get(Type::getInt32Ty(Ctx), StepId)};
    RegB.CreateCall(RegisterStep, StepArgs);
    emitGpuStepPointeeRegs(Mod, Ctx, PairWF);
```

The module is written at compile time (`main.cpp:1410`,
`emitGpuKernels(*M, "kernels.ptx")`) and read at run time
(`gpu_runtime.c:266-341`):

```c
static char *read_ptx_file(size_t *len_out) { FILE *fp = fopen("kernels.ptx", "rb"); … }
…
    CUresult r = p_cuModuleLoadDataEx(&mod, ptx, 0, NULL, NULL);
    if (r != CUDA_SUCCESS || !mod) { … return NULL; }
```

Device module assembly (`parallel_loop_outline.cpp`): kernel + callees + the
globals they reference are cloned; external callees are re-homed; the device
defines its own activation primitive — the **mark**, no CAS, no ticket:

```llvm
@sgpl_gpu_claimed = external global i8*      ; patched by the runtime, per launch
define i32 @autograph_frontier_activate(ptr %ctx, i32 %v) {
  %base = load ptr, ptr @sgpl_gpu_claimed
  %slot = getelementptr i8, ptr %base, i64 (sext i32 %v to i64)
  store i8 1, ptr %slot                        ; idempotent mark
  ret i32 1
}
```

(`atom.` count in the emitted PTX = 0 — verified earlier by instruction count.)
The activation runtime additionally requires that the claim global exists in the
module before it launches (`gpu_runtime.c:337-341`):

```c
        if (p_cuModuleGetGlobal(&cg, &cgs, mod, "sgpl_gpu_claimed") != CUDA_SUCCESS ||
            !cg || cgs < sizeof(CUdeviceptr)) {
            if (getenv("SGPL_GPU_DEBUG"))
                fprintf(stderr, "[gpu] activation step: module has no sgpl_gpu_claimed; CPU\n");
            return 0;
        }
```

### 2.4 Eligibility is read from the operation's own resource table

Each emitted operation carries `{mask, access}` pairs, and the context carries
traversal + domain kind — the gate reads these, never a loop name:

```c
/* resource masks (autotuner_runtime.h) */
SGPL_RES_MEMBERSHIP   SGPL_RES_DEST_SEEN   SGPL_RES_NEXT_FRONTIER
SGPL_RES_SNAPSHOT     SGPL_RES_PARTIAL     SGPL_RES_PRIVATE     SGPL_RES_CLAIM
/* access modes */
SGPL_ACCESS_READ = 0, WRITE = 1, ATOMIC_WRITE = 2, PRIVATE = 3
/* traversal / domain */
SGPL_TRAVERSE_OWNER_U = 0, SGPL_TRAVERSE_OWNER_V = 1
SGPL_DOMAIN_ALL_VERTICES = 0, SGPL_DOMAIN_FRONTIER = 1
```

A BFS activation op declares exactly `MEMBERSHIP:READ, DEST_SEEN:ATOMIC_WRITE,
NEXT_FRONTIER:ATOMIC_WRITE, SNAPSHOT:READ, CLAIM:ATOMIC_WRITE`.

---

## 3. Runtime side: gates, then the mirror

### 3.1 The dispatch point

`sgpl_exec_step_dispatch` (`autotuner_runtime.c:3586-3650`) tries the device
before any CPU work; everything after it — combine, coverage, `next_size`,
round-end — is unchanged engine code:

```c
  if (sgpl_gpu_step_try_device(ctx, meta)) {
    return;
  }
```

### 3.2 The activation gate (V path)

`autotuner_runtime.c:3390-3398`:

```c
  name = autograph_gpu_step_name_for(ctx->step_id);
  if (!name) {
    sgpl_gpu_step_refuse(ctx->step_id, "no device kernel registered for this step id");
    return 0;
  }
  if (strncmp(name, "gpu_step_v_", 11) != 0) { … /* see 3.5: routing */ }
  if (!ctx->dest_seen || !ctx->next_frontier || !ctx->append_head) {
    sgpl_gpu_step_refuse(ctx->step_id, "destination envelope not wired (dest_seen/next_frontier/append_head)");
    return 0;
  }
```

Behind it, all one-shot via `sgpl_gpu_step_refuse` (`:3367/:3378`, printed under
`SGPL_GPU_DEBUG`): destination rows not built (`:3411`), more than one pair body
(`:3420`), a "source/partition/round phase op present (not mirrored on the
device)" (`:3429`), partial/private slot resources (`:3439`), envelope facts not
declared (`:3445`), cost model small (`:3457`).

### 3.3 The source-owned gate (U path)

`autotuner_runtime.c:3529-3574`: resolve name; refuse an *activation* kernel here
(`:3535`); refuse membership-restricted steps — "source slices carry every arc"
(`:3541`); refuse when a combine phase is present — "partials are folded on the
host" (`:3546`); refuse per-source lifecycle ops (`:3554`); refuse when the cost
model says small (`:3569`).

### 3.4 What the device marks, the host replays

`autotuner_runtime.c:3481-3496` — exactly the CPU's `autograph_frontier_activate`
semantics (claim CAS 0→1, then append under the head), performed in one place:

```c
  /* The CPU's activate() sets dest_seen[v] and appends v under the atomic head;
   * the device marked exactly the transitions, so reproduce both here. */
  if (claimed) {
    int32_t appended = 0;
    int64_t v;
    for (v = 0; v < nv; ++v)
      if (claimed[v]) {
        ctx->next_frontier[ctx->initial_next_size + appended] = (int32_t)v;
        ctx->dest_seen[v] = 1;
        ++appended;
      }
    if (appended > 0 && ctx->append_head)
      __atomic_fetch_add(ctx->append_head, appended, __ATOMIC_RELAXED);
    if (getenv("SGPL_GPU_DEBUG"))
      fprintf(stderr, "[gpu] activation step appended %d of %lld vertices\n", …);
  }
```

The frontier stays a *set* on both sides: the device may mark in any order, the
host fixes the order.

### 3.5 Routing: the compile-time decision is authoritative

A V-shaped step *without* the activation envelope legitimately gets the
source-owned kernel; when the runtime reaches it through the activation path it
hands the step to the source-owned device attempt instead of refusing
(`autotuner_runtime.c:3390-3401`):

```c
  if (strncmp(name, "gpu_step_v_", 11) != 0) {
    /* … the compile-time decision is authoritative -- run it on the source-owned
     * device path instead of refusing the step outright. */
    if (getenv("SGPL_GPU_DEBUG"))
      fprintf(stderr, "[gpu] step %d: kernel %s is source-owned; routed to the source-owned device path\n",
              (int)ctx->step_id, name);
    return sgpl_gpu_step_try_device(ctx, meta);
  }
```

### 3.6 Policy, registry, pointees

**Policy** (pure, unit-tested without a GPU):

```c
int sgpl_gpu_policy_verdict(int64_t trip, int needs_doacross, int64_t dist,
                            int64_t min_trips, int64_t max_waves);
int sgpl_gpu_engine_step_verdict(int64_t arcs, int64_t min_pairs);
```

Defaults: `SGPL_GPU_MIN_TRIPS = 4096`, `SGPL_GPU_MAX_WAVES = 8192`,
`SGPL_GPU_ENGINE_MIN_PAIRS = 2 000 000`; each knob takes `0` to disable the bound,
which is how the verification harness forces device execution. The registry
deduplicates `(name, step_id)` and is looked up by the dispatching context's own
step id; pointees and the claim map are refreshed **per launch** (an upload-once
cache made round 2 read round-1 state — a real bug).

Pseudocode of the whole decision (the code is `:3367-3574`, quoted above at the
decision points):

```
sgpl_exec_step_dispatch(ctx, meta)
├─ OWNER_V → sgpl_gpu_step_try_device_v:  kernel=gpu_step_v_*; envelope wired;
│            rows built; exactly one pair body; no source/partition/round hook;
│            no PARTIAL/PRIVATE; DEST_SEEN+NEXT_FRONTIER declared; cost gate
│            └─ success → replay marks ascending: next_frontier[…] = v,
│                         dest_seen[v] = 1, append_head += k
├─ else    → sgpl_gpu_step_try_device:    kernel is source-owned; full domain;
│            no COMBINE, no SOURCE_BEGIN/END; cost gate → gpup_step_try
└─ any refusal → CPU partitions (reason printed once per step under SGPL_GPU_DEBUG)
```

---

## 4. Spatial axis on the device (M1 + M3)

**Decomposition.** One thread per owned row/source, `ctaid.x`-major (§2.2). The
owner-computes assignment is inherited, so data writes need no atomics; the claim
is an idempotent byte store, and the frontier counter is the host's.

**The budget (M3): the CPU ledger's `max(1, W/A)` as a resident-block cap.**
The launcher takes a block budget and walks the row domain in chunks, shifting
the base pointers per chunk — no kernel change, no work dropped
(`gpu_runtime.c`, activation launcher; the source-owned launcher is identical
with `(rows, begins)` in place of `(rowsrc, rowptr)`):

```c
    unsigned int budget = (unsigned int)gpup_block_budget();
    int64_t chunk_rows = budget ? (int64_t)budget * (int64_t)block : g_v_nrows;
    unsigned int grid_total = 0;
    CUresult lr = CUDA_SUCCESS;
    for (int64_t off = 0; off < g_v_nrows; off += chunk_rows)
    {
        int64_t rows_now = g_v_nrows - off;
        if (rows_now > chunk_rows)
            rows_now = chunk_rows;
        CUdeviceptr rowsrc_c = g_v_rowsrc_dev + (size_t)off * sizeof(int32_t);
        CUdeviceptr rowptr_c = g_v_rowptr_dev + (size_t)off * sizeof(int64_t);
        void *params[6] = {&rowsrc_c, &rows_now, &rowptr_c, &g_v_arcs_dev, &mem_arg, &env_arg};
        unsigned int grid = (unsigned int)((rows_now + (int64_t)block - 1) / (int64_t)block);
        grid_total = grid;
        lr = p_cuLaunchKernel(kfn, grid, 1, 1, block, 1, 1, 0, gpup_step_stream(), params, NULL);
        if (lr != CUDA_SUCCESS)
            break;
    }
```

Chunking rather than clamping is what makes the cap **sound**: the mapping has no
stride loop (§2.2), so a clamped grid would silently drop rows; sub-range pointer
shifts keep each chunk's rows exactly the global rows, and the arc indices
(`rowptr`/`begins`) stay absolute.

The budget source is the same ledger the CPU's `max(1, W/A)` uses — the thread's
share, read weakly so a build without `parallel_runtime.c` still links
(`gpu_runtime.c:644-661`):

```c
static int gpup_block_budget(void)
{
    if (g_block_budget < 0)
    {
        const char *e = getenv("SGPL_GPU_BLOCK_BUDGET");
        int v = e ? atoi(e) : 0;
        if (v <= 0 && getenv("SGPL_GPU_BUDGET_FROM_LEDGER"))
        {
            extern int32_t sgpl_current_thread_budget(void) __attribute__((weak));
            if (sgpl_current_thread_budget)
                v = (int)sgpl_current_thread_budget();
        }
        g_block_budget = v > 0 ? v : 0;
        …
    }
    return g_block_budget;
}
```

`gpup_set_block_budget(int32_t)` is the explicit setter for the engine; passing
the node's `max(1, W/A)` share is the intended wiring.

---

## 5. Temporal axis on the device (M2)

**One launch order is one stream order.** All device launches go to a single
explicit non-blocking stream, so the round sequence the host drives becomes a
stream-ordered sequence; the host still synchronizes at the round boundary. The
stream is created lazily, and the A/B switch is an env var
(`gpu_runtime.c:617-643`):

```c
static CUstream gpup_step_stream(void)
{
    static int cached = -1;
    if (cached < 0)
        cached = (getenv("SGPL_GPU_TEMPORAL_STREAM") && atoi(getenv("SGPL_GPU_TEMPORAL_STREAM")) == 0) ? 0 : 1;
    if (!cached)
        return NULL;
    if (!g_step_stream_tried)
    {
        g_step_stream_tried = 1;
        if (p_cuStreamCreate && p_cuStreamCreate(&g_step_stream, 0x1u /* NON_BLOCKING */) != CUDA_SUCCESS)
            g_step_stream = NULL;
        …
    }
    return g_step_stream;
}

static CUresult gpup_step_sync(void)
{
    CUstream s = gpup_step_stream();
    if (s && p_cuStreamSynchronize)
        return p_cuStreamSynchronize(s);
    return p_cuCtxSynchronize();
}
```

The stream symbols are loaded **optionally** (`gpu_runtime.c:237-240`, plain
`dlsym`), so a driver without them degrades to the default stream rather than
failing the load. The temporal *template* is untouched by the device: the round
body (`autograph_frontier_execute`) is the unit — begin/snapshot/combine/end stay
on the host — and only the dispatch inside it moves. That is why the device step
never has to know about rounds.

---

## 6. What is *not* mirrored (the coverage surface)

Refused by the **emitters**, so the runtime reports "no device kernel registered
for this step id": `ReducePtr` / `AccConsumeStore` (reductions,
source-reductions), `RoundSepBases` (round separation), `HasFirstWins` (claims),
`HasFrontierAppend` without a wired envelope (R7), and non-simple V shapes
(`IsV && IsSimple`).

Refused by the **runtime gates** even when a kernel exists: membership-restricted
steps, steps with a combine phase, per-source lifecycle ops, partial/private slot
resources, more than one pair body, undeclared envelope facts, and the cost
model's small-step threshold. Every refusal is printed once per step with its
reason, and the CPU answer is identical by construction — which is what the
differentials check.

---

## 7. Evidence

Commands (pod, from `p1GraphEasy-con-AutoTuner`; `LD_LIBRARY_PATH` for antlr,
`ulimit -s unlimited` for the generated frames):

```bash
export LD_LIBRARY_PATH=/usr/local/lib
FORCE_GPU=1 GRAPH_FILE=../verify/cases/algo/bfs_level.graph bash 03_run.sh
ulimit -s unlimited
SGPL_NUM_THREADS=4 ./final_program                                    # device off
SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program
SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 \
  SGPL_GPU_ENGINE_MIN_PAIRS=0 SGPL_GPU_BLOCK_BUDGET=32 ./final_program
```

**This session (new pod, LLVM 20.1.8 `X86;NVPTX`):**

- **M2**: `[gpu] temporal stream (M2): explicit non-blocking stream`; with
  `SGPL_GPU_TEMPORAL_STREAM=0` the default stream is used; both runs identical to
  device-off.
- **M3**: `[gpu] block budget (M3): 32 blocks` and
  `activation step ran on device: … rows=202273 grid=13 budget=32` (last of ~50
  chunk launches, ≤32 blocks resident); identical to device-off and to the
  unbudgeted run.
- **Corpus differential** (`verify/gpu_corpus_diff.sh`, 63/78 fixtures at last
  read): **58 same, 0 DIFF**, 5 environmental failures (the generated program
  killed by the host on some fixtures — identically with the device off).
  Device-eligible: `bfs_level` 6 dispatches, `kcore` 16, `budget_two_steps`
  **1600**, `data_index_write` 4, `dg_src_keyed` 16.
- **Routing**: `edge_write_v` (measured emit flags `IsV=1 IsSimple=1 app=0 fw=0
  rs=0 → src=y v=n`) prints `[gpu] step 1: kernel gpu_step_main_sgpl_pair_work is
  source-owned; routed to the source-owned device path`; its full differential is
  blocked on that fixture's program being killed by the host even with the device
  off.

**Recorded earlier (2026-10-04 era; mechanisms unchanged, kept for the record):**
device policy boundaries T8/T9/T10 all PASS; device-execution proof 11/0/2;
cross-mode CPU==device with thread counts/repeats/partition sweep 11/11 and 50/50
broad; CPU==device on 780k- and 157k-edge graphs 3/3; `atom.` count in the module
= 0; measured device/CPU ratios: DOALL 0.74–1.16, DOACROSS-waves≤8192 0.09–0.26,
one clear win at waves==8192 (1.13), engine step (source-owned, 320k arcs)
0.41–0.62 — which is why the engine-step cost floor exists
(`SGPL_GPU_ENGINE_MIN_PAIRS`).

---

## Appendix — knobs, files, standing rule

**Environment knobs:** `SGPL_GPU_MIN_TRIPS` (4096), `SGPL_GPU_MAX_WAVES` (8192),
`SGPL_GPU_ENGINE_MIN_PAIRS` (2 000 000), `SGPL_GPU_ENGINE_STEP` /
`SGPL_NO_GPU_ENGINE_STEP`, `SGPL_GPU_DEBUG`, `SGPL_GPU_TEMPORAL_STREAM` (M2),
`SGPL_GPU_BLOCK_BUDGET` / `SGPL_GPU_BUDGET_FROM_LEDGER` (M3), `FORCE_GPU`,
`FORCE_CPU`, `SGPL_GPU_BACKEND`.

**Files:** `graph_frontier_lowering.cpp` (algebra, resource tables, step + kernel
emission, registration, pointees, snapshots) · `autotuner_runtime.c` (CleanCut
engine, dispatch hook, device gates, marks→frontier, snapshot publish) ·
`gpu_runtime.c/.h` (driver runtime, policies, registry, launchers M2/M3,
pointees) · `parallel_loop_outline.cpp` (device module assembly: activate stub,
claimed global, PTX `sm_70`) · `main.cpp` (backend selection, `kernels.ptx`) ·
harnesses: `verify/gpu_check.sh`, `verify/gpu_cross_mode_check.sh`,
`verify/gpu_device_diff.sh`, `verify/gpu_corpus_diff.sh`,
`verify/gpu_gate_probe.sh`, `verify/gpu_cost_model_check.sh`.

**Standing rule:** a shape runs on the device only when its own declared facts
say the device realization can honour the verdict; every refusal is printed with
its reason; anything else runs on the CPU, where the answer is identical by
construction.
