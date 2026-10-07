import torch, time
torch.backends.cuda.matmul.allow_tf32 = True
d = 8192
dev = "cuda"
pairs = [(torch.randn(d, d, device=dev), torch.randn(d, d, device=dev)) for _ in range(5)]
extra = [torch.empty(d, d, device=dev) for _ in range(4)]
out = torch.empty(d, d, device=dev)
torch.cuda.synchronize()
mem = torch.cuda.memory_allocated() / 1e9
print(f"working set {mem:.2f} GB of {torch.cuda.get_device_properties(0).total_memory/1e9:.1f} GB", flush=True)
t0 = time.time(); flops = 0.0; n = 0
target_duty_sleep = 0.24
while time.time() - t0 < 240:
    t1 = time.time()
    for a, b in pairs:
        torch.mm(a, b, out=out)
    torch.cuda.synchronize()
    dt = time.time() - t1
    flops += 5 * 2 * d**3; n += 1
    if n % 20 == 0:
        el = time.time() - t0
        print(f"iter {n} compute {dt*1e3:.0f} ms achieved {flops/el/1e12:.1f} TFLOP/s avg", flush=True)
    time.sleep(target_duty_sleep)
print("done", n, "iters", f"{flops/(time.time()-t0)/1e12:.1f} TFLOP/s", flush=True)
