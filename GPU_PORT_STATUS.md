# GPU two-axis port — status / resume pointer (keep updated every turn)

Objective: realize the two-axis DAG scheduler on the GPU the same way it works
on the CPU — spatial axis = partitions on the device, temporal axis =
stream-ordered units/runs — verified with CPU-vs-GPU differentials.

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

## Milestones
- M0 (now): build LLVM+NVPTX → rebuild GraphProgram with the new prefix →
  BFS fixture with `--ir-backend=gpu` emits kernels.ptx → run with the device
  step forced (SGPL_GPU_ENGINE_STEP=1, SGPL_GPU_ENGINE_MIN_PAIRS=0) and
  differential vs SGPL_NO_GPU_ENGINE_STEP=1 ("reached 20000 level_checksum
  75722" both ways).
- M1: generalize the spatial kernels to arbitrary partition pair bodies.
- M2: temporal axis on streams/events through the host-side DAG launcher.
- M3: block/SM budget from the same parallel_runtime ledger.
- M4 (optional): device-side persistent scheduler with a global ready queue.
Verification mirrors the CPU work: differential on/off, worker/grid sweeps,
repeat-run determinism, compute-sanitizer racecheck on the kernels.
