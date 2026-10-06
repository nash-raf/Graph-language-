#!/usr/bin/env python3
"""GPU keep-warm without a CUDA toolkit and without torch.

A driver-API SGEMM written in embedded PTX and driven through libcuda via
ctypes (the same driver-API style the compiler's device runtime uses -- no nvcc,
no cuBLAS, no python GPU packages).  Control interface matches mm_warm.py:

    /tmp/gpu_warm.target   percent of the wall clock to keep the device busy
                           (default 70)
    /tmp/mm_warm.stop      touch it to exit

The kernel is verified against a host reference before any warming, so a broken
kernel can never masquerade as load: if the check fails the script prints the
mismatch and exits non-zero.
"""
import ctypes
import os
import sys
import time

PTX = r"""
.version 6.0
.target sm_70
.address_size 64

.visible .entry sgemm(
    .param .u64 p_a,
    .param .u64 p_b,
    .param .u64 p_c,
    .param .u32 p_n
)
{
    .reg .pred %p<2>;
    .reg .b32  %r<14>;
    .reg .b64  %rd<10>;
    .reg .f32  %f<4>;

    ld.param.u64 %rd1, [p_a];
    ld.param.u64 %rd2, [p_b];
    ld.param.u64 %rd3, [p_c];
    ld.param.u32 %r1, [p_n];

    cvta.to.global.u64 %rd1, %rd1;
    cvta.to.global.u64 %rd2, %rd2;
    cvta.to.global.u64 %rd3, %rd3;

    mov.u32 %r2, %ctaid.x;
    mov.u32 %r3, %ntid.x;
    mov.u32 %r4, %tid.x;
    mad.lo.u32 %r5, %r2, %r3, %r4;        // tid
    mul.lo.u32 %r6, %r1, %r1;             // n*n
    setp.ge.u32 %p1, %r5, %r6;
    @%p1 bra DONE;

    div.u32 %r7, %r5, %r1;                // i
    rem.u32 %r8, %r5, %r1;                // j

    mov.f32 %f1, 0f00000000;              // acc = 0
    mov.u32 %r9, 0;                       // k = 0
LOOP:
    setp.ge.u32 %p1, %r9, %r1;
    @%p1 bra ENDL;

    mad.lo.u32 %r10, %r7, %r1, %r9;       // i*n + k
    mul.wide.u32 %rd4, %r10, 4;
    add.s64 %rd5, %rd1, %rd4;
    ld.global.f32 %f2, [%rd5];

    mad.lo.u32 %r11, %r9, %r1, %r8;       // k*n + j
    mul.wide.u32 %rd6, %r11, 4;
    add.s64 %rd7, %rd2, %rd6;
    ld.global.f32 %f3, [%rd7];

    fma.rn.f32 %f1, %f2, %f3, %f1;
    add.u32 %r9, %r9, 1;
    bra LOOP;
ENDL:
    mad.lo.u32 %r12, %r7, %r1, %r8;       // i*n + j
    mul.wide.u32 %rd8, %r12, 4;
    add.s64 %rd9, %rd3, %rd8;
    st.global.f32 [%rd9], %f1;
DONE:
    ret;
}
"""

CUDA_SUCCESS = 0


class Cuda:
    def __init__(self):
        self.lib = ctypes.CDLL("libcuda.so.1")
        self.check(self.lib.cuInit(0), "cuInit")
        dev = ctypes.c_int()
        self.check(self.lib.cuDeviceGet(ctypes.byref(dev), 0), "cuDeviceGet")
        self.dev = dev
        self.ctx = ctypes.c_void_p()
        for sym in ("cuCtxCreate_v2", "cuCtxCreate"):
            fn = getattr(self.lib, sym, None)
            if fn is None:
                continue
            rc = fn(ctypes.byref(self.ctx), 0, dev)
            if rc == CUDA_SUCCESS:
                break
        else:
            raise RuntimeError("cuCtxCreate unavailable")
        self.mod = ctypes.c_void_p()
        self.check(self.lib.cuModuleLoadDataEx(
            ctypes.byref(self.mod), ctypes.c_char_p(PTX.encode()), 0, None, None),
            "cuModuleLoadDataEx")
        self.fn = ctypes.c_void_p()
        self.check(self.lib.cuModuleGetFunction(
            ctypes.byref(self.fn), self.mod, b"sgemm"), "cuModuleGetFunction")

    @staticmethod
    def check(rc, what):
        if rc != CUDA_SUCCESS:
            raise RuntimeError(f"{what} failed: rc={rc}")

    def alloc(self, nbytes):
        p = ctypes.c_void_p()
        self.check(self.lib.cuMemAlloc_v2(ctypes.byref(p), nbytes), "cuMemAlloc")
        return p

    def free(self, p):
        self.lib.cuMemFree_v2(p)

    def htod(self, p, buf):
        self.check(self.lib.cuMemcpyHtoD_v2(p, ctypes.c_char_p(buf), len(buf)), "HtoD")

    def dtoh(self, buf, p):
        # buf is a writable ctypes buffer (c_char_Array): pass its address, not
        # a c_char_p view of it
        self.check(self.lib.cuMemcpyDtoH_v2(
            ctypes.cast(buf, ctypes.c_void_p), p, len(buf)), "DtoH")

    def sgemm(self, n, a, b, c):
        grid = (n * n + 255) // 256
        # Each kernel parameter is an 8-byte value (the device address) or a
        # 4-byte u32; the params array holds *addresses of those values*.
        pa = ctypes.c_uint64(a.value)
        pb = ctypes.c_uint64(b.value)
        pc = ctypes.c_uint64(c.value)
        pn = ctypes.c_uint32(n)
        params = (ctypes.c_void_p * 4)(
            ctypes.cast(ctypes.byref(pa), ctypes.c_void_p),
            ctypes.cast(ctypes.byref(pb), ctypes.c_void_p),
            ctypes.cast(ctypes.byref(pc), ctypes.c_void_p),
            ctypes.cast(ctypes.byref(pn), ctypes.c_void_p))
        self.check(self.lib.cuLaunchKernel(self.fn, grid, 1, 1, 256, 1, 1, 0,
                                           None, params, None), "cuLaunchKernel")

    def sync(self):
        self.check(self.lib.cuCtxSynchronize(), "cuCtxSynchronize")


def verify(cu, n=32):
    import struct
    A = [((i * 7 + k) % 13) * 0.125 for i in range(n) for k in range(n)]
    B = [((k * 5 + j) % 11) * 0.25 for k in range(n) for j in range(n)]
    bufA = struct.pack(f"{n*n}f", *A)
    bufB = struct.pack(f"{n*n}f", *B)
    bufC = b"\0" * (4 * n * n)
    da, db, dc = cu.alloc(len(bufA)), cu.alloc(len(bufB)), cu.alloc(len(bufC))
    cu.htod(da, bufA)
    cu.htod(db, bufB)
    cu.htod(dc, bufC)
    cu.sgemm(n, da, db, dc)
    cu.sync()
    out = ctypes.create_string_buffer(4 * n * n)
    cu.dtoh(out, dc)
    C = struct.unpack(f"{n*n}f", out.raw)
    ref = [sum(A[i * n + k] * B[k * n + j] for k in range(n)) for i in range(n) for j in range(n)]
    worst = max(abs(C[t] - ref[t]) for t in range(n * n))
    scale = max(1.0, max(abs(v) for v in ref))
    ok = worst <= 1e-2 * scale
    print(f"self-check n={n}: worst abs error {worst:.3e} (scale {scale:.1f}) -> {'OK' if ok else 'MISMATCH'}")
    cu.free(da); cu.free(db); cu.free(dc)
    return ok


def target_percent():
    try:
        with open("/tmp/gpu_warm.target") as fh:
            return max(0, min(100, int((fh.read().split() or ["70"])[0])))
    except Exception:
        return 70


def main():
    cu = Cuda()
    if not verify(cu):
        sys.exit("keep-warm kernel failed verification; refusing to warm")
    n = 1024
    nbytes = 4 * n * n
    import struct
    buf = struct.pack(f"{n*n}f", *[0.5] * (n * n))
    da, db, dc = cu.alloc(nbytes), cu.alloc(nbytes), cu.alloc(nbytes)
    for d in (da, db, dc):
        cu.htod(d, buf)
    cu.sgemm(n, da, db, dc)
    cu.sync()
    t0 = time.time()
    for _ in range(3):
        cu.sgemm(n, da, db, dc)
    cu.sync()
    per_launch = (time.time() - t0) / 3
    tflops = 2 * n**3 / per_launch / 1e12
    print(f"busy unit: one {n}x{n}x{n} sgemm in {per_launch*1e3:.1f} ms -> {tflops:.2f} TFLOP/s")
    print("target file /tmp/gpu_warm.target, stop file /tmp/mm_warm.stop")
    cycle = 1.0
    while not os.path.exists("/tmp/mm_warm.stop"):
        pct = target_percent()
        busy = cycle * pct / 100.0
        # Launches are asynchronous: queue exactly as many as the device will
        # take in `busy` seconds (measured per-launch time), then wait, so the
        # duty cycle is the device's, not the host's queueing rate.
        launches = max(1, int(round(busy / max(per_launch, 1e-6))))
        t_start = time.time()
        for _ in range(launches):
            cu.sgemm(n, da, db, dc)
        cu.sync()
        spent = time.time() - t_start
        if spent > 0 and launches:
            print(f"warm target={pct}% busy={spent:.2f}s launches={launches} "
                  f"({2*n**3*launches/spent/1e12:.2f} TFLOP/s)", flush=True)
        time.sleep(max(0.0, cycle - (time.time() - t_start)))
    print("stop file seen; exiting")


if __name__ == "__main__":
    main()
