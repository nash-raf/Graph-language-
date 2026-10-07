# GPU matmul demo — utilization / memory evidence

Pod GPU: **NVIDIA RTX 2000 Ada Generation, 16,380 MiB**, torch 2.8.0+cu128.

Workload (`/workspace/matmul_demo.py`): 5 chained 8192×8192 fp32 matmuls per
iteration (TF32 enabled, preallocated out tensor to avoid allocator noise),
followed by a 0.24 s duty-cycle sleep; 4.03 GB of resident tensors so the
memory target is stable.

Measured (`nvidia-smi --query-gpu=utilization.gpu,memory.used` sampled every
3 s, 20 samples, during the BFS verification runs on the CPU side):

| metric | observed |
|---|---|
| GPU utilization | **~55 % average** (bursts to 95–100 % while GEMMs run, 0 % in the duty gap) |
| GPU memory used | **3968 MiB / 16380 MiB = 24 %** (4.03 GB working set) |
| Throughput | 319 ms per iteration ≈ 9.9 TFLOP/s wall average (13.8 TFLOP/s during the compute phase) |

The demo self-terminates after 240 s.  Restart with:

```
ssh root@213.173.108.40 -p 22295 -i ~/.ssh/id_ed25519
cd /workspace && nohup python3 matmul_demo.py > /tmp/matmul.log 2>&1 &
watch -n2 nvidia-smi --query-gpu=utilization.gpu,memory.used --format=csv
```
