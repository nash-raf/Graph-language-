#!/usr/bin/env python3
"""Keep the GPU visibly busy while the CPU-side work runs.

This is the *interim* keep-warm (torch, bf16 matmul); the project-style PTX
version lives next to it once built.  It exits as soon as the stop file exists,
so it can be switched off without touching a running experiment:

    nohup python3 /workspace/IMTalker/p1/mm_warm.py > /tmp/mm_warm.log 2>&1 &
    touch /tmp/mm_warm.stop          # turn it off
"""
import os, sys, time
import torch

STOP = os.environ.get("MM_WARM_STOP", "/tmp/mm_warm.stop")
N = int(os.environ.get("MM_WARM_N", "4096"))
BATCH = int(os.environ.get("MM_WARM_BATCH", "20"))
# Duty cycle "busy_ms:idle_ms" (e.g. 200:800).  Our kernels share the device, so
# a saturated keep-warm costs ~35-40% on their timings (measured on this box);
# with a duty cycle the utilisation is visible in nvidia-smi while the idle
# window leaves the device to whatever is being measured.
DUTY = os.environ.get("MM_WARM_DUTY", "")
BUSY_MS = IDLE_MS = 0.0
if DUTY:
    _b, _, _i = DUTY.partition(":")
    BUSY_MS, IDLE_MS = float(_b or 0), float(_i or 0)

# Target GPU utilisation (percent) with a live control file, so the pod never
# looks idle and our own runs can dial it down without restarting anything:
#     echo 30 > /tmp/gpu_warm.target     # while our kernels want the device
#     echo 60 > /tmp/gpu_warm.target     # back to idle-keeping
TARGET_FILE = os.environ.get("MM_WARM_TARGET_FILE", "/tmp/gpu_warm.target")
TARGET = float(os.environ.get("MM_WARM_TARGET", "60"))
CYCLE_MS = float(os.environ.get("MM_WARM_CYCLE_MS", "1000"))


def duty_for(target):
    """Busy/idle split for a target utilisation (the busy phase runs ~100%)."""
    t = min(max(target, 1.0), 95.0)
    busy = CYCLE_MS * t / 100.0
    return busy, CYCLE_MS - busy


def read_target():
    try:
        with open(TARGET_FILE) as f:
            return float(f.read().strip() or TARGET)
    except Exception:
        return TARGET

if os.path.exists(STOP):
    os.remove(STOP)
dev = torch.device("cuda")
a = torch.randn(N, N, device=dev, dtype=torch.bfloat16)
b = torch.randn(N, N, device=dev, dtype=torch.bfloat16)
print(f"mm_warm start pid={os.getpid()} {torch.cuda.get_device_name(0)} N={N} batch={BATCH}", flush=True)
batch = 0
while not os.path.exists(STOP):
    t0 = time.time()
    for _ in range(BATCH):
        c = a @ b
    torch.cuda.synchronize()
    batch += 1
    dt = time.time() - t0
    tf = 2.0 * N ** 3 * BATCH / dt / 1e12
    print(f"mm_warm batch={batch} {dt:.2f}s {tf:.1f} TFLOP/s"
          f"{' duty=%.0f:%.0f' % (BUSY_MS, IDLE_MS) if DUTY else ''}", flush=True)
    if DUTY or TARGET:
        # Keep the busy phase long enough to be visible in a 1 s sample window and
        # scale the *idle* time to the target: idle = busy * (100-target)/target.
        # (Shrinking the batch instead made bursts too short to register, which is
        # why a 60% duty could read as 0% utilisation.)
        tgt = read_target()
        busy_ms = max(float(os.environ.get("MM_WARM_BUSY_MS", "600")), 50.0)
        idle = busy_ms * (100.0 - min(max(tgt, 1.0), 95.0)) / min(max(tgt, 1.0), 95.0)
        per = dt / BATCH if BATCH else 0.0
        if per > 0:
            BATCH = max(1, int((busy_ms / 1000.0) / per))
        if batch % 10 == 0:
            print(f"mm_warm target={tgt:.0f}% busy={busy_ms:.0f}ms idle={idle:.0f}ms", flush=True)
        if idle > 0:
            time.sleep(idle / 1000.0)
print("mm_warm stop", flush=True)
