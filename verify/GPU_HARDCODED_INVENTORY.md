# GPU path: hardcoded values, literals and assumptions

Written 2026-10-02 (after the activation-step work).  Everything below is a
value that is *not* derived from the graph or the effect facts; each entry says
where it lives, what it does, and whether it is tunable.  Verified against the
tree at the commit state `ee7195e` + working changes.

## 1. Cost model — tunable via environment (defaults are the policy)

| knob | default | where | meaning |
|---|---|---|---|
| `SGPL_GPU_MIN_TRIPS` | 4096 | `gpu_runtime.c:994` | outlined loop below this many trips stays on the host pool |
| `SGPL_GPU_MAX_WAVES` | 8192 | `gpu_runtime.c:995` | DOACROSS: waves = ceil(trips/distance); above this is a wave storm -> CPU doacross. `0` disables the cap |
| `SGPL_GPU_ENGINE_MIN_PAIRS` | 2000000 | `autotuner_runtime.c:3443,3546` | engine step (source pairs / destination arcs) below this stays on the CPU partitions; `0` disables |

Measured boundary on this box, 20k-vertex fixtures: a device engine step costs
~2.5x the 4-thread CPU at 320k arcs (per-round state copies dominate), which is
why the 2M default exists; the gate direction is covered by
`gpu_cost_model_check.sh` (400000 -> CPU, 300000 -> device, `0` -> device).

## 2. Fixed constants (caps, geometry) — not tunable, each with a reason

| constant | value | where | reason |
|---|---|---|---|
| step registry size | 8 | `gpu_runtime.c:395` (`GPUP_MAX_STEPS`) | registered device steps; registry now keyed by step id |
| step pointee cache | 32 | `gpu_runtime.c:465` (`GPUP_STEP_POINTEES`) | hitting it refuses the step (fail-closed, `gpu_runtime.c:521`) |
| pointee registry | 64 | `gpu_runtime.c:71` (`GPUP_MAX_POINTEES`) | arrays materialised for device steps |
| env buffer registry | 1024 | `gpu_runtime.c:122` (`GPUB_MAX_BUFFERS`) | outlined-path buffer registry |
| engine-step block | 128 threads | `gpu_runtime.c:706` (source rows), `:933` (destination rows) | one thread per owned row; block size only affects occupancy |
| outlined-kernel block | 256 threads | `gpu_runtime.c:1241` | DOALL/DOACROSS kernels |
| frontend CUDA device | index 0 | `gpu_runtime.c:249` | always the first device |
| PTX target | `sm_70` | `parallel_loop_outline.cpp:4012` | driver JITs for every Volta+ card; verified on sm_89 and sm_120 |
| membership encoding | byte per vertex, nonzero = member | `autotuner_runtime.h` (`SGPL_DOMAIN_*`), kernel gate in `graph_frontier_lowering.cpp:4971+` | follows the runtime's scratch membership array |
| claim encoding | byte per vertex, value 1 | device activation stub emitted in `parallel_loop_outline.cpp:~3880` | idempotent mark; exactly one owner writes it |
| frontier counts | `int32_t` | `sgpl_exec_ctx` (`next_frontier`, `append_head`, `initial_next_size`) | same limit as the CPU activate path |
| vertex ids | `int32_t` | kernel ABIs | follows `csr_n`/`push_*` layout |

## 3. Platform literals (per machine / per driver)

* `libcuda.so.1`, fallback `libcuda.so` — `gpu_runtime.c:213,215`.
* `kernels.ptx` on disk, else the embedded payload (`"embedded"`) — `gpu_runtime.c:261`, `main.cpp:1410`, `parallel_loop_outline.cpp:4052`.
* GPU presence detection for backend `auto`: `nvidia-smi -L`, `rocm-smi -i 0 --showproductname`, `clinfo` — `main.cpp:145-147`.  Note the asymmetry: only a CUDA driver is actually used at run time, so a ROCm-only host selects the GPU backend and then falls back per call.
* Backend strings `"auto" | "cpu" | "gpu"` (`main.cpp:98-101`), env `FORCE_GPU` / `FORCE_CPU` (`main.cpp:154-183`).
* Local build assumes LLVM at `/usr/local/llvm-20-polly-rtti` (`build_lowmem.sh:8`); the box build (`build_remote.sh`) uses distribution LLVM.
* PTX kernel names are sanitised to `[A-Za-z0-9_]` on both sides (`graph_frontier_lowering.cpp:5018` for activation; shadow globals renamed to PTX-safe names at `:3228`).

## 4. Compiler <-> runtime ABI hardcodings (names, layouts)

These are the places where a change on one side silently breaks the other if
not mirrored:

* kernel *name prefixes* select the launcher and are matched literally:
  `gpu_kernel_` (outlined), `gpu_step_` (source-owned engine step),
  `gpu_step_v_` (destination-owned activation step; the runtime checks the
  11-character prefix at `autotuner_runtime.c:~3508`).
* kernel parameter order/types (both engine-step kernels):
  * source-owned: `(i32* pairs, i64 npairs, i32* rows, i64 nrows, i64* begins, i8* env)`
  * activation:   `(i32* rowsrc, i64 nrows, i64* rowptr, i32* arcs, i8* mem, i8* env)`
  launch params are built in exactly this order (`gpu_runtime.c:703-707`, `:930-935`).
* module metadata keys: `graph.gpu.kernels` (kernel list; operand 1 is the shape tag), `graph.ir.backend` (`"gpu"`).
* emitted runtime hooks: `autograph_gpu_step_register`, `sgpl_gpu_register_pointee`, `autograph_snapshot_publish(..., const char *name)`, `autograph_frontier_activate` (device module defines its own, host keeps the CAS version).
* device globals patched by name: `sgpl_gpu_claimed` (activation claim map) plus every registered pointee global.

## 5. Semantic assumptions in the device step (documented in code, listed here)

* Ownership is per source (source-owned steps) or per destination (activation
  steps); the kernel is one thread per owned row with its arcs walked
  sequentially, so no atomics are needed (`graph_frontier_lowering.cpp:4705+`,
  `:4971+`).
* Activation claim payloads are claimer-independent within a round (frontier
  homogeneity); the device relies on this exactly like the CPU's per-row order
  does.  Verified empirically (bit-identical answers) but not proven by the
  effect algebra.
* The device step mirrors the *pair phase only*: any source/partition/round
  hook, partial/private resource, membership-restricted source slice, or
  more-than-one pair body refuses the step (reasons printed under
  `SGPL_GPU_DEBUG`, `autotuner_runtime.c` `sgpl_gpu_step_refuse`).
* Pointees are re-uploaded on every launch (host may rewrite arrays between
  rounds: snapshot refresh, live state); copy-back keeps the host current.
* `push_rp/ci/indir` rows are sources with destination arcs (built by the
  cut); the activation kernel relies on that layout.

## 6. Test-side pins (what the checks encode)

* `verify/expected/golden.json` — expected outputs per case (suite-compared).
* `verify/gpu_check.sh` — case list, per-case env, and the literal answers for
  cases that are not compared against a CPU build.
* `verify/gpu_cost_model_check.sh` — boundary probes sized around the real
  fixture numbers: 320000 engine arcs, 200000 DOALL trips, 199999 DOACROSS
  waves; partition sweep `{1,3,7}` in the broad check.
* `verify/gpu_broad_cross_check.sh` — thread set `{1,4}`, two repeats per
  thread count, partition set `{1,3,7}`; builds are required to produce
  `final_program` or the case is reported as a build failure.
* `tdg_budget_test.c` — T1-T10 unit pins for the TDG budget and the pure device
  policy (`sgpl_gpu_policy_verdict`, `sgpl_gpu_engine_step_verdict`).

## 6b. Launch geometry limits (fail-closed, not tuned)

* grid is `ceil(rows / block)` as `unsigned int` (`gpu_runtime.c:707,934`); a
  step that exceeded the driver's grid limit would fail the launch and fall
  back to the CPU rather than truncate work.

## 7. Known, deliberate leftovers

* The two `class=`-era scripts and any old tooling that greps for the pre-merge
  verification vocabulary are not maintained here.
* Pre-existing `-Wdeprecated-declarations` warnings in `parallel_loop_outline.cpp`
  (upstream `Type::getPointerTo`) — untouched; nothing in the code added for the
  GPU work emits a warning (`gcc -Wall -Wextra` clean on `gpu_runtime.c`).
