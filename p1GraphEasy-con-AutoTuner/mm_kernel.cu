/* Kernel source for gpu_warm.ptx (the keep-warm matrix multiply).
 *
 *   clang++-20 -x cuda --cuda-device-only --cuda-gpu-arch=sm_89 \
 *              -nocudainc -nocudalib -O3 -S mm_kernel.cu -o gpu_warm.ptx
 *
 * The thread-index functions are clang builtins, so --nocudainc (no CUDA
 * toolkit) is enough.  One thread per C element: C[i][j] = sum_k A[i][k]B[k][j]. */
extern "C" __attribute__((global)) void mm_kernel(const float *a, const float *b, float *c,
                                                  unsigned n)
{
    unsigned i = __nvvm_read_ptx_sreg_ctaid_x() * __nvvm_read_ptx_sreg_ntid_x() +
                 __nvvm_read_ptx_sreg_tid_x();
    if (i >= n * n)
        return;
    unsigned row = i / n, col = i % n;
    float acc = 0.0f;
    for (unsigned k = 0; k < n; ++k)
        acc += a[row * n + k] * b[k * n + col];
    c[row * n + col] = acc;
}
