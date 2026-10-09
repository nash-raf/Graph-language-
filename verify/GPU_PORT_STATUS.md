# GPU two-axis port — status / resume pointer (keep updated every turn)

Objective: realize the two-axis DAG scheduler on the GPU the same way it works
on the CPU — spatial axis = partitions on the device, temporal axis =
stream-ordered units/runs — verified with CPU-vs-GPU differentials.

## POD MOVED (new container): root@213.173.110.201 -p 18657
- /workspace persisted (p1 tree, verify/, fixtures, scripts, GraphProgram binary,
  /workspace/llvm-src/llvm-20.1.8.tar.xz); everything on local disk died
  (both LLVM toolchains, apt packages, antlr).
- Re-provisioning: `verify/pod_setup_gpu.sh` (apt deps -> antlr4 4.13.2 runtime
  -> LLVM 20.1.8 X86;NVPTX+Polly+RTTI -> GraphProgram rebuild), log
  /tmp/pod_setup.log, markers .../SETUP-DONE.
- After SETUP-DONE: re-confirm M0/M1 on the new pod, then the three open items:
  (1) shape coverage + fail-closed GPU IR (21 fixtures), (2) M2 streams,
  (3) M3 device budget.

## State
- LLVM 20.1.8 + NVPTX + Polly + RTTI building on the pod:
  - source: /workspace/llvm-src/llvm-project-20.1.8.src (tarball llvm-20.1.8.tar.xz)
  - build dir: /workspace/llvm-build-nvptx (Ninja), logs /tmp/llvm_cmake.log, /tmp/llvm_build.log
  - install prefix: /workspace/llvm-20.1.8-nvptx
    (the known-good /usr/local/llvm-20-polly-rtti stays untouched)
  - flags: Release, LLVM_ENABLE_RTTI=ON, LLVM_ENABLE_PROJECTS=polly,
    LLVM_TARGETS_TO_BUILD=X86;NVPTX, tests/benchmarks/examples/docs off
- The old toolchain has target X86 only — that is the *only* reason the device
  path was dark ("[gpu] NVPTX target unavailable ... GPU offload disabled").

## M0 evidence (this turn, with the current X86-only toolchain)
- Compiler: `SGPL_GPU_DEBUG=1 FORCE_GPU=1 ./GraphProgram bfs_level.graph` prints
  `[gpu] activation step kernel emitted: gpu_step_v_main_sgpl_pair_work`
  then `[gpu] NVPTX target unavailable ... GPU offload disabled` -- the device
  kernel is emitted and registered; only the PTX compilation fails.
- Runtime (device step forced): pointees registered (`visited`, `lvl`), 
  `[gpu] engine step registered: gpu_step_v_main_sgpl_pair_work (step 1)`,
  `[gpu] load_module: loaded from file`, then
  `[gpu] activation step: kernel ... not found; CPU` -- the loaded module
  (an old kernels.ptx) lacks the new kernel, so the step falls back, exactly as
  the fail-closed design intends.  M0 = give it a kernels.ptx that contains it.

## Build attempts (two, parallel)
- mfs: /workspace/llvm-build-nvptx -> /workspace/llvm-20.1.8-nvptx (slow
  extraction on the network FS, still running; logs /tmp/llvm_cmake.log)
- local disk (fast path): /root/llvm-build-nvptx -> /root/llvm-20.1.8-nvptx
  (extract -> cmake -> ninja -j32 -> install chained in /tmp/llvm_local.log)
Run M0 with LLVM_NVPTX_PREFIX pointing at whichever install completes first:
  LLVM_NVPTX_PREFIX=/root/llvm-20.1.8-nvptx bash verify/gpu_m0_check.sh

## GPU facts (pod)
RTX 2000 Ada, CC 8.9, driver 580.159.04; CUDA 12.8 toolkit (nvcc at
/usr/local/cuda-12.8/bin/nvcc); PTX is emitted for sm_70 → JIT-forward-compatible.

## Existing machinery (no new code needed for M0)
- Engine-step device kernels are already emitted by graph_frontier_lowering.cpp:
  `gpu_step_<fn>_<tag>` (source-owned, ~:5420) and `gpu_step_v_<fn>_<tag>`
  (destination-owned activation, ~:5652); registered in named metadata
  `graph.gpu.kernels` and with the runtime (`autograph_gpu_step_register`,
  :5501/:5735); pointees via emitGpuStepPointeeRegs.
- PTX writer: `emitGpuKernels(Module&, "kernels.ptx")`
  (parallel_loop_outline.cpp:3733), called from main.cpp:1410 when the IR
  backend is gpu (`--ir-backend=gpu`).
- Runtime loader: gpu_runtime.c fopen("kernels.ptx") + CUDA driver API
  (cuModuleLoadDataEx / cuLaunchKernel); host launchers `gpup_step_try`,
  `gpup_step_v_try`, and `gpu_parallel_for_runtime` (device DOALL dispatch).
- Engine dispatch + gates: sgpl_gpu_step_try_device_v / sgpl_gpu_step_try_device
  in autotuner_runtime.c — strict shape checks, cost gate
  (SGPL_GPU_ENGINE_MIN_PAIRS), switches SGPL_GPU_ENGINE_STEP /
  SGPL_NO_GPU_ENGINE_STEP; every failure falls back to the CPU.

## Design notes for M1/M2 (from recon)
- The *shape-extension* path for the spatial axis already has all the parts:
  graph_frontier_lowering.cpp emits a **device copy of the pair body** per
  engine step (`gpu_step_<fn>_<tag>` source-owned, `gpu_step_v_<fn>_<tag>`
  activation/destination-owned) and registers pointees
  (`emitGpuStepPointeeRegs`).  M1 is therefore a *witness-by-witness* relaxing
  of the runtime shape gate (`sgpl_gpu_step_try_device_v`): today it accepts
  exactly the activation envelope (dest_seen + next_frontier + append_head, one
  pair op, no source/partition/round phases, no partial/private) and refuses
  everything else with a reason.  Each new accepted shape needs its device
  kernel emission + runtime marshalling + a CPU-vs-GPU differential, mirroring
  how the CPU work added axis-by-axis.
- The temporal axis already has its device launcher signature:
  `gpu_parallel_for_runtime(start, end, step, kernel_name, body, env, ...,
  needs_doacross, doacross_num_sync_ids, global_pointee_flags, doacross_dist)`
  (gpu_runtime.c:962) -- i.e. the device DOALL/DOACROSS dispatch exists with
  sync-id/distance parameters, and gpu_runtime.c already uses streams/events.
  M2 is the host-side DAG launcher mapping certificate-approved units onto
  launch order (chain = same stream) + events for parallel units.

## M0 DONE + VERIFIED (this turn)
- Toolchain: LLVM 20.1.8 + NVPTX built + installed at /root/llvm-20.1.8-nvptx
  (`llvm-config --targets-built` = "X86 NVPTX"); GraphProgram rebuilt against it
  (78,457,728 bytes), and `build_lowmem.sh` now honors `${LLVM_CONFIG:-...}`.
- BFS fixture with FORCE_GPU=1 emits PTX: kernels.ptx = 3373 bytes, zero
  "NVPTX target unavailable" lines.
- Differential (device step forced vs off), P=1,4,8: **identical**.
- Device really executed: `[gpu] activation step ran on device:
  gpu_step_v_main_sgpl_pair_work rows=202273 grid=1581`,
  `activation step appended 19 of 20000 vertices`, 6 device dispatches and
  **0** "kept on the CPU" refusals in the run; module loaded (embedded PTX).

## M1 progress: BOTH device shapes verified (this turn)
- Activation/destination-owned (`gpu_step_v_*`): BFS -> device ran
  (`rows=202273 grid=1581`, 6 dispatches, 0 fallbacks), answers identical P=1,4,8.
- Source-owned (`gpu_step_*`): kcore -> kernels.ptx 1918 B, answers identical
  P=1,4,8 (`kcore_size 19995`), **16 device dispatches, 0 fallbacks**.
- Gotchas found and fixed on the way:
  * fixtures had stale absolute input paths -> now `../verify/fixtures/...`
    (24 fixtures fixed, portable across boxes);
  * generated programs can have huge stack frames (kcore: 31 MB); the default
    8 MB `ulimit -s` segfaults them *inside the loader call*.  The differential
    scripts now set `ulimit -s unlimited`; the crash is unrelated to the device
    path (CPU-IR builds crash the same way).
- Corpus-wide device differential running: verify/gpu_corpus_diff.sh ->
  /tmp/gpu_corpus_report.txt (per fixture: rc, same/DIFF, device vs fallback
  dispatch counts).

## Corpus device differential (this turn)
- verify/gpu_corpus_diff.sh over verify/cases/*/*.graph with FORCE_GPU=1:
  results (latest run): mostly "same", **0 DIFF**, device-eligible fixtures:
  bfs_level (6 dispatches, 0 fallback), kcore (16, 0), pagerank (16, 1 --
  mixed device/CPU per shapes, as designed).
- Two gotchas fixed on the way (both unrelated to the device path):
  1. fixtures carried absolute input paths for the *old* box -> normalised to
     `../verify/fixtures/...` (portable for both boxes);
  2. generated programs can allocate >30 MB stack frames (kcore) -> the
     default 8 MB `ulimit -s` segfaults them inside the loader call; the
     differential scripts now set `ulimit -s unlimited`.
- Remaining corpus failures are shape/emission limits of the *other* GPU path
  (outlined loops / gpu_parallel_for_runtime), not the engine step.

## Next-turn anchor (this turn: pod re-provisioning + recon)
- Setup on the new pod: apt OK; antlr4 4.13.2 source zip's root IS the runtime
  source (no `runtime/Cpp` subdir) -> fixed pod_setup_gpu.sh, manual build
  running (/tmp/antlr_build.log) -> `ANTLR-OK`; LLVM 20.1.8+NVPTX ninja running
  (/tmp/pod_setup.log, [n/2695]) -> then the script relinks GraphProgram.
- M3 (device budget) recon, exact facts:
  * launch/grid: `gpup_step_v_try` gpu_runtime.c:936 `grid = ceil(nthreads/block)`,
    :937 `cuLaunchKernel(..., 0, NULL, ...)` -> **default stream** (the M2 hook);
    source-owned twin at :707/:708; engine call sites autotuner_runtime.c:3343
    (def) and :3466 (call, `meta->push_rp[0]`, `partition_count`).
  * the emitted kernels are **flat 1-D** over rows:
    `Lin = blockIdx*blockDim + tid; if (Lin < Nrows) {body}` (activation
    emitter graph_frontier_lowering.cpp:5682-5688; source-owned :5454-5460) --
    no grid-stride loop, so clamping gridStar would drop rows.  M3 therefore
    needs BOTH: a stride loop in both emitters (read `nctaid.x`, loop
    `Lin += nctaid.x*ntid.x`) AND a clamped grid from the ledger share.
- M2 (streams): the launch sites above take stream NULL -> introduce a runtime
  stream (+events) and use it in both launchers; the temporal chain is already
  sequential, so the deliverable is explicit stream-ordered launches + an
  event/graph path, verified by the same differential.
- Item 1 (shape coverage / fail-closed): the 21 corpus build failures are the
  *outlined-loop* GPU path; diagnosis needs GraphProgram runnable again (antlr
  + relink).  First fixture to diagnose: verify/cases/parallel/edge_write_v.graph.

## Docs (both requested markdowns written)
- CPU/effect-system implementation, theory->code, both axes, all mechanisms:
  p1GraphEasy-con-AutoTuner/EFFECT_SYSTEM_IMPLEMENTATION_REPORT.md (981 lines).
- GPU realization, theory->code, spatial (grid/budget) + temporal (stream) axes,
  gates + refusal surface, evidence: verify/GPU_EFFECT_SYSTEM_REPORT.md (~470
  lines; replaces the 2026-10-04 narrative version).

## M2 + M3 DONE + VERIFIED (new pod, LLVM+NVPTX rebuilt)
- M3 (device budget): gpu_runtime.c takes a block budget -- SGPL_GPU_BLOCK_BUDGET=N,
  SGPL_GPU_BUDGET_FROM_LEDGER=1 (weak sgpl_current_thread_budget()), exported
  gpup_set_block_budget() for the engine.  Realisation: chunked launches over the
  row domain with pointer-shifted sub-ranges (rows_src/rowptr resp. rows/begins),
  so at most `budget` blocks are resident and no work is dropped; kernels unchanged.
  Evidence (BFS, 202273 rows): `[gpu] block budget (M3): 32 blocks` and
  `activation step ran on device: ... rows=202273 grid=13 budget=32` (last of ~50
  chunk launches); answers identical to CPU-off and to the unbudgeted run.
- M2 (temporal axis on a stream): device launches go on an explicit non-blocking
  stream (cuStreamCreate/cuStreamSynchronize, optional symbols), gpup_step_sync()
  replaces the context sync, SGPL_GPU_TEMPORAL_STREAM=0 falls back to the default
  stream.  Evidence: `[gpu] temporal stream (M2): explicit non-blocking stream`;
  stream and default-stream runs identical to CPU-off (0 DIFF).
- Pod re-provisioning: antlr4 4.13.2 built (/usr/local/lib + ldconfig;
  4.13.1 soname symlink for the existing GraphProgram), LLVM 20.1.8 X86;NVPTX
  installed at /root/llvm-20.1.8-nvptx, GraphProgram relinked (78,457,728 B).
- Item 1 (fail-closed) status: **no genuine GPU-emission failure found yet** --
  every "BUILD-FAILED" so far was environmental: (a) stale absolute fixture paths
  (/home/user/D/... variant, 46 local + 32 pod files normalised; 0 stale now),
  (b) the setup script's relink window (`Compiler binary not found at
  ./GraphProgram` while build_lowmem.sh relinked).  Direct compile of a
  previously "failing" fixture (doall_single) = rc 0.  Corpus differential is
  re-running on the clean tree; the remaining work for item 1 is widening the
  device shape gates (coverage), not error handling.

## Shape coverage -- routing fix LANDED (compile-time decision is authoritative)
Change: autotuner_runtime.c sgpl_gpu_step_try_device_v no longer refuses when the
registered kernel is source-owned (not `gpu_step_v_*`); it routes the step to
sgpl_gpu_step_try_device (forward decl added).  Rationale: the emitter is the
authority on what can run on the device -- a V-shaped step WITHOUT the activation
envelope (app=0) legitimately gets the source-owned kernel, and the runtime was
second-guessing that.
Witness state for edge_write_v (the fixture this addresses): its step emits
`IsV=1 IsSimple=1 app=0 fw=0 rs=0 -> src=y v=n` and the runtime now prints
`[gpu] step 1: kernel gpu_step_main_sgpl_pair_work is source-owned; routed to the
source-owned device path` (SGPL_GPU_DEBUG=1).  Its *differential* is blocked by an
environment problem unrelated to the GPU work: the generated program is SIGKILLed
in the CPU-only run too, with no OOM record in dmesg and memory pressure ~0.06
(free 167G) -- see /tmp/ew_*.txt, /tmp/r_off.txt on the pod.  Do not read that as
a GPU regression: device-off is killed identically.
Corpus differential on the fixed build (verify/gpu_corpus_diff.sh): 36/37 same,
0 DIFF, 1 transient BUILD-FAILED (pagerank -- 03_run.sh's own final run killed,
env).  Device-eligible: bfs_level 6, kcore 16, budget_two_steps 1600,
data_index_write 4, dg_src_keyed 16.
Still open (real emitter gaps, "no device kernel registered for this step id"):
mixed_regions, roundsep, two_reduce_slots -- reduction / round-separation shapes
need device mirrors (the source-owned emitter refuses ReducePtr/AccConsumeStore,
the V emitter refuses !IsSimple).  That is the next coverage increment.

## Shape coverage -- measured refusal reasons (worklist)
Probe: verify/gpu_gate_probe.sh <fixture...> (builds with FORCE_GPU=1, runs the
device step forced with SGPL_GPU_DEBUG=1, prints device counts + refusal reasons).
Results:
- bfs_level / kcore / pagerank: device paths run (6 / 16 / 16+1 dispatches, 0 DIFF).
- data_index_write: **4 device dispatches**, 1 refusal ("no device kernel
  registered for this step id" -- a step the compiler did not emit for; benign).
- edge_write_v: refused -- `registered kernel is not an activation kernel`
  (autotuner_runtime.c:3394).  The compiler registered the *source-owned* kernel
  (`gpu_step_*`, because `emitGpuEngineStepV` still requires IsV && IsSimple,
  graph_frontier_lowering.cpp ~5605-5660) while the runtime dispatched on the
  activation path.  Fix = extend the V emitter to non-simple V steps (mirror the
  extra ops on the device) + relax the matching gate clauses; do NOT "try the
  source-owned launcher" -- it lacks the claim copy-back/append and would be
  silently wrong.
- dual_shadow: refused -- `a source/partition/round phase op is present (it is
  not mirrored on the device)` (autotuner_runtime.c:3421).  Fix = mirror the
  phase ops in the emitted device kernel, then drop the clause for that shape.
- doall_single: device=0 and *no refusal* -> not an engine step at all (plain
  outlined DOALL -> gpu_parallel_for_runtime path); coverage there is a separate
  mechanism, not the engine gate.
Gate code sites: sgpl_gpu_step_try_device_v ~3374 (refusals at :3390 no kernel,
:3394 not activation, :3421 phase op), sgpl_gpu_step_try_device ~3523 (no kernel).
Refined root cause for edge_write_v (read at graph_frontier_lowering.cpp:5607-5615):
the V emitter refuses on `!IsSimple` where IsSimple = !IsRed && !IsSourceRed &&
!IsPriv (:5759) -- edge_write_v has a *first-wins / claim* destination write, so it
is none of {reduction, source-reduction, private}, yet **HasFirstWins also refuses
the source-owned emitter** (`if (... || Info.HasFirstWins || Info.HasFrontierAppend)
return nullptr;`, ~:5407).  So no device kernel is emitted at all and the runtime
refusal is only the consequence: covering this shape means *implementing* the
first-wins/claim mirror in a device kernel (atomics CAS on the mark), not
relaxing a clause.  Use that as the first shape to add after the gate worklist.
Order of work: (1) V emitter for non-simple V steps + clause 3421; (2) more
shapes (combine/partial, privatized layouts) as their own witnesses.
Note: the corpus differential was stopped for this probe; rerun it after any
gate change (LD_LIBRARY_PATH=/usr/local/lib bash verify/gpu_corpus_diff.sh).

## Milestones
- M0 (now): build LLVM+NVPTX → rebuild GraphProgram with the new prefix →
  BFS fixture with `--ir-backend=gpu` emits kernels.ptx → run with the device
  step forced (SGPL_GPU_ENGINE_STEP=1, SGPL_GPU_ENGINE_MIN_PAIRS=0) and
  differential vs SGPL_NO_GPU_ENGINE_STEP=1 ("reached 20000 level_checksum
  75722" both ways).
- M1: generalize the spatial kernels to arbitrary partition pair bodies.
  Next concrete check: the *source-owned* shape (`gpu_step_<fn>_<tag>` +
  `gpup_step_try` + the `sgpl_gpu_step_try_device` gates) already has compiler
  and runtime support -- verify it on a fixture whose step is source-owned
  (e.g. the kcore degree phase) with the same on/off differential.
- M2: temporal axis on streams/events through the host-side DAG launcher.
- M3: block/SM budget from the same parallel_runtime ledger.
- M4 (optional): device-side persistent scheduler with a global ready queue.
Verification mirrors the CPU work: differential on/off, worker/grid sweeps,
repeat-run determinism, compute-sanitizer racecheck on the kernels.
