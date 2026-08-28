#define _GNU_SOURCE
#include "gpu_runtime.h"
#include "parallel_runtime.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// PTX text embedded in the executable by emitGpuKernels (see
// "gpu_embedded_ptx" in parallel_loop_outline.cpp). Weak so this builds even
// when the compiler did not emit the symbol; NULL then means "use the file".
extern const char gpu_embedded_ptx[] __attribute__((weak));

typedef int CUresult;
typedef int CUdevice;
typedef void *CUcontext;
typedef void *CUmodule;
typedef void *CUfunction;
typedef void *CUstream;
typedef uint64_t CUdeviceptr;

#define CUDA_SUCCESS 0

typedef CUresult (*cuInitFn)(unsigned int);
typedef CUresult (*cuDeviceGetCountFn)(int *);
typedef CUresult (*cuDeviceGetFn)(CUdevice *, int);
typedef CUresult (*cuCtxCreateFn)(CUcontext *, unsigned int, CUdevice);
typedef CUresult (*cuModuleLoadDataExFn)(CUmodule *, const void *, unsigned int, int *, void **);
typedef CUresult (*cuModuleGetFunctionFn)(CUfunction *, CUmodule, const char *);
typedef CUresult (*cuModuleGetGlobalFn)(CUdeviceptr *, size_t *, CUmodule, const char *);
typedef CUresult (*cuModuleUnloadFn)(CUmodule);
typedef CUresult (*cuMemAllocFn)(CUdeviceptr *, size_t);
typedef CUresult (*cuMemFreeFn)(CUdeviceptr);
typedef CUresult (*cuMemcpyHtoDFn)(CUdeviceptr, const void *, size_t);
typedef CUresult (*cuMemcpyDtoHFn)(void *, CUdeviceptr, size_t);
typedef CUresult (*cuLaunchKernelFn)(CUfunction, unsigned int, unsigned int, unsigned int,
                                     unsigned int, unsigned int, unsigned int, unsigned int,
                                     CUstream, void **, void **);
typedef CUresult (*cuCtxSynchronizeFn)(void);
typedef CUresult (*cuLaunchCooperativeKernelFn)(CUfunction, unsigned int, unsigned int, unsigned int,
                                                unsigned int, unsigned int, unsigned int, unsigned int,
                                                CUstream, void **, void **);
typedef CUresult (*cuOccupancyMaxActiveBlocksPerMultiprocessorFn)(int *, CUfunction, int, size_t);
typedef CUresult (*cuDeviceGetAttributeFn)(int *, int, CUdevice);

#define CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT 16

static int g_cuda_loaded = 0;
static int g_cuda_available = 0;
static CUdevice g_device = 0;
static CUcontext g_context = NULL;
static CUmodule g_cached_module = NULL;

static void *g_cuda_lib = NULL;
static cuInitFn p_cuInit = NULL;
static cuDeviceGetCountFn p_cuDeviceGetCount = NULL;
static cuDeviceGetFn p_cuDeviceGet = NULL;
static cuCtxCreateFn p_cuCtxCreate = NULL;
static cuModuleLoadDataExFn p_cuModuleLoadDataEx = NULL;
static cuModuleGetFunctionFn p_cuModuleGetFunction = NULL;
static cuModuleGetGlobalFn p_cuModuleGetGlobal = NULL;
static cuModuleUnloadFn p_cuModuleUnload = NULL;
static cuMemAllocFn p_cuMemAlloc = NULL;
static cuMemFreeFn p_cuMemFree = NULL;
static cuMemcpyHtoDFn p_cuMemcpyHtoD = NULL;
static cuMemcpyDtoHFn p_cuMemcpyDtoH = NULL;
static cuLaunchKernelFn p_cuLaunchKernel = NULL;
static cuCtxSynchronizeFn p_cuCtxSynchronize = NULL;
static cuLaunchCooperativeKernelFn p_cuLaunchCooperativeKernel = NULL;
static cuOccupancyMaxActiveBlocksPerMultiprocessorFn p_cuOccupancyMaxActiveBlocksPerMultiprocessor = NULL;
static cuDeviceGetAttributeFn p_cuDeviceGetAttribute = NULL;

#define LOAD_SYM(name)                                        \
    do                                                        \
    {                                                         \
        *(void **)(&p_##name) = dlsym(g_cuda_lib, #name);     \
        if (!p_##name)                                        \
        {                                                     \
            g_cuda_available = 0;                             \
            return 0;                                         \
        }                                                     \
    } while (0)

static int load_cuda(void)
{
    if (g_cuda_loaded)
        return g_cuda_available;

    g_cuda_loaded = 1;
    g_cuda_available = 0;

    g_cuda_lib = dlopen("libcuda.so.1", RTLD_LAZY);
    if (!g_cuda_lib)
        g_cuda_lib = dlopen("libcuda.so", RTLD_LAZY);
    if (!g_cuda_lib)
        return 0;

    LOAD_SYM(cuInit);
    LOAD_SYM(cuDeviceGetCount);
    LOAD_SYM(cuDeviceGet);
    LOAD_SYM(cuCtxCreate);
    LOAD_SYM(cuModuleLoadDataEx);
    LOAD_SYM(cuModuleGetFunction);
    LOAD_SYM(cuModuleGetGlobal);
    LOAD_SYM(cuModuleUnload);
    LOAD_SYM(cuMemAlloc);
    LOAD_SYM(cuMemFree);
    LOAD_SYM(cuMemcpyHtoD);
    LOAD_SYM(cuMemcpyDtoH);
    LOAD_SYM(cuLaunchKernel);
    LOAD_SYM(cuCtxSynchronize);
    LOAD_SYM(cuLaunchCooperativeKernel);
    LOAD_SYM(cuOccupancyMaxActiveBlocksPerMultiprocessor);
    LOAD_SYM(cuDeviceGetAttribute);

    if (p_cuInit(0) != CUDA_SUCCESS)
        return 0;

    int count = 0;
    if (p_cuDeviceGetCount(&count) != CUDA_SUCCESS || count <= 0)
        return 0;

    if (p_cuDeviceGet(&g_device, 0) != CUDA_SUCCESS)
        return 0;

    if (p_cuCtxCreate(&g_context, 0, g_device) != CUDA_SUCCESS)
        return 0;

    g_cuda_available = 1;
    return 1;
}

static char *read_ptx_file(size_t *len_out)
{
    FILE *fp = fopen("kernels.ptx", "rb");
    if (!fp)
        return NULL;

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return NULL;
    }

    long sz = ftell(fp);
    if (sz < 0)
    {
        fclose(fp);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        fclose(fp);
        return NULL;
    }

    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf)
    {
        fclose(fp);
        return NULL;
    }

    size_t rd = fread(buf, 1, (size_t)sz, fp);
    fclose(fp);
    if (rd != (size_t)sz)
    {
        free(buf);
        return NULL;
    }

    buf[sz] = '\0';
    if (len_out)
        *len_out = (size_t)sz;
    return buf;
}

static CUmodule load_module(void)
{
    if (g_cached_module)
        return g_cached_module;

    const char *ptx_src = "file";
    const char *ptx = NULL;
    char *file_ptx = NULL;

    if (gpu_embedded_ptx && gpu_embedded_ptx[0] != '\0')
    {
        ptx = gpu_embedded_ptx;
        ptx_src = "embedded";
    }
    else
    {
        size_t len = 0;
        file_ptx = read_ptx_file(&len);
        (void)len;
        if (!file_ptx)
        {
            if (getenv("SGPL_GPU_DEBUG"))
                fprintf(stderr, "[gpu] load_module: kernels.ptx read failed\n");
            return NULL;
        }
        ptx = file_ptx;
    }

    CUmodule mod = NULL;
    CUresult r = p_cuModuleLoadDataEx(&mod, ptx, 0, NULL, NULL);
    free(file_ptx);
    if (r != CUDA_SUCCESS || !mod)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] load_module: cuModuleLoadDataEx failed r=%d (%s)\n", (int)r, ptx_src);
        return NULL;
    }

    if (getenv("SGPL_GPU_DEBUG"))
        fprintf(stderr, "[gpu] load_module: loaded from %s\n", ptx_src);

    g_cached_module = mod;
    return mod;
}

void gpu_parallel_for_runtime(
    int64_t start,
    int64_t end,
    int64_t step,
    const char *kernel_name,
    void (*body)(int64_t, void *),
    void *env,
    int64_t env_size,
    const int64_t *ptr_offsets,
    const int64_t *ptr_sizes,
    const int64_t *ptr_cap_offsets,
    const int64_t *ptr_elem_sizes,
    int32_t num_ptr_fields,
    const char *const *global_names,
    void *const *global_ptrs,
    const int64_t *global_sizes,
    int32_t num_globals,
    int32_t needs_doacross,
    int32_t doacross_num_sync_ids)
{
    if (step <= 0)
    {
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    int64_t trip = (end - start + step - 1) / step;
    if (trip <= 0)
        return;

    if (!load_cuda())
    {
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    CUmodule mod = load_module();
    if (!mod)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] fallback: module load failed (%s)\n", kernel_name);
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    CUfunction kfn = NULL;
    if (p_cuModuleGetFunction(&kfn, mod, kernel_name) != CUDA_SUCCESS || !kfn)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] fallback: function lookup failed (%s)\n", kernel_name);
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    size_t nfields = num_ptr_fields > 0 ? (size_t)num_ptr_fields : 1;
    CUdeviceptr *d_bufs = (CUdeviceptr *)calloc(nfields, sizeof(CUdeviceptr));
    int64_t *d_sizes = (int64_t *)calloc(nfields, sizeof(int64_t));
    void **host_ptrs = (void **)calloc(nfields, sizeof(void *));
    if (!d_bufs || !d_sizes || !host_ptrs)
    {
        free(d_bufs);
        free(d_sizes);
        free(host_ptrs);
        parallel_for_runtime(start, end, step, body, env, 0, 0);
        return;
    }

    CUdeviceptr d_env = 0;
    if (p_cuMemAlloc(&d_env, (size_t)env_size) != CUDA_SUCCESS)
    {
        free(d_bufs);
        free(d_sizes);
        free(host_ptrs);
        parallel_for_runtime(start, end, step, body, env, 0, 0);
        return;
    }

    if (p_cuMemcpyHtoD(d_env, env, (size_t)env_size) != CUDA_SUCCESS)
    {
        p_cuMemFree(d_env);
        free(d_bufs);
        free(d_sizes);
        free(host_ptrs);
        parallel_for_runtime(start, end, step, body, env, 0, 0);
        return;
    }

    int setup_ok = 1;
    int32_t i;
    for (i = 0; i < num_ptr_fields; ++i)
    {
        int64_t off = ptr_offsets[i];
        void *host_ptr = *(void **)((char *)env + off);
        host_ptrs[i] = host_ptr;
        int64_t size = ptr_sizes[i];

        if (size <= 0)
        {
            int64_t cap_off = ptr_cap_offsets[i];
            int64_t elem = ptr_elem_sizes[i];
            if (cap_off < 0 || elem <= 0)
            {
                setup_ok = 0;
                break;
            }
            int32_t cap = *(int32_t *)((char *)env + cap_off);
            if (cap < 0)
                cap = 0;
            size = (int64_t)cap * elem;
        }

        if (!host_ptr || size <= 0)
        {
            CUdeviceptr null_d = 0;
            if (p_cuMemcpyHtoD(d_env + (size_t)off, &null_d, sizeof(null_d)) != CUDA_SUCCESS)
            {
                setup_ok = 0;
                break;
            }
            d_bufs[i] = 0;
            d_sizes[i] = 0;
            continue;
        }

        CUdeviceptr d_buf = 0;
        if (p_cuMemAlloc(&d_buf, (size_t)size) != CUDA_SUCCESS)
        {
            setup_ok = 0;
            break;
        }
        if (p_cuMemcpyHtoD(d_buf, host_ptr, (size_t)size) != CUDA_SUCCESS)
        {
            p_cuMemFree(d_buf);
            setup_ok = 0;
            break;
        }
        if (p_cuMemcpyHtoD(d_env + (size_t)off, &d_buf, sizeof(d_buf)) != CUDA_SUCCESS)
        {
            p_cuMemFree(d_buf);
            setup_ok = 0;
            break;
        }

        d_bufs[i] = d_buf;
        d_sizes[i] = size;
    }

    int device_ran = 0;
    if (setup_ok)
    {
        // Copy referenced global arrays host -> device. The device module owns
        // globals with the same names; cuModuleGetGlobal resolves their device
        // addresses. Any mismatch aborts the GPU path (kernel would otherwise
        // read uninitialized device globals).
        for (i = 0; i < num_globals; ++i)
        {
            CUdeviceptr dptr = 0;
            size_t dsize = 0;
            if (!global_names[i] || !global_ptrs[i] || global_sizes[i] <= 0 ||
                p_cuModuleGetGlobal(&dptr, &dsize, mod, global_names[i]) != CUDA_SUCCESS ||
                !dptr || dsize < (size_t)global_sizes[i])
            {
                setup_ok = 0;
                break;
            }
            if (p_cuMemcpyHtoD(dptr, global_ptrs[i], (size_t)global_sizes[i]) != CUDA_SUCCESS)
            {
                setup_ok = 0;
                break;
            }
        }
    }

    if (setup_ok)
    {
        unsigned int block = 256;
        unsigned int grid = (unsigned int)((trip + (int64_t)block - 1) / (int64_t)block);
        int64_t k_start = start;
        int64_t k_end = end;
        int64_t k_step = step;

        void *kparams[4];
        kparams[0] = &k_start;
        kparams[1] = &k_end;
        kparams[2] = &k_step;
        kparams[3] = &d_env;

        CUresult launch_r = CUDA_SUCCESS;
        if (needs_doacross)
        {
            // DOACROSS uses the cooperative wave kernel: the whole grid must be
            // resident so grid.sync() (bar.sync 0) can synchronize every block.
            // Cap the grid at the occupancy limit; the kernel loops over waves
            // internally, so a smaller grid is correct.
            int occ = 0;
            int num_sm = 0;
            CUresult r_occ = p_cuOccupancyMaxActiveBlocksPerMultiprocessor(&occ, kfn, (int)block, 0);
            CUresult r_sm = p_cuDeviceGetAttribute(&num_sm, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, g_device);
            if (r_occ != CUDA_SUCCESS || r_sm != CUDA_SUCCESS || occ <= 0 || num_sm <= 0)
            {
                if (getenv("SGPL_GPU_DEBUG"))
                    fprintf(stderr, "[gpu] cooperative occupancy unavailable (occ=%d sm=%d r=%d/%d); CPU fallback\n",
                            occ, num_sm, (int)r_occ, (int)r_sm);
                setup_ok = 0;
            }
            else
            {
                unsigned int max_blocks = (unsigned int)(occ * num_sm);
                if (max_blocks < 1)
                    max_blocks = 1;
                if (grid > max_blocks)
                    grid = max_blocks;
                launch_r = p_cuLaunchCooperativeKernel(kfn, grid, 1, 1, block, 1, 1, 0, NULL, kparams, NULL);
                if (launch_r != CUDA_SUCCESS && getenv("SGPL_GPU_DEBUG"))
                    fprintf(stderr, "[gpu] cooperative launch failed r=%d; CPU fallback\n", (int)launch_r);
            }
        }
        else
        {
            launch_r = p_cuLaunchKernel(kfn, grid, 1, 1, block, 1, 1, 0, NULL, kparams, NULL);
        }

        if (setup_ok && launch_r == CUDA_SUCCESS)
        {
            p_cuCtxSynchronize();
            if (getenv("SGPL_GPU_DEBUG"))
                fprintf(stderr, "[gpu] launched %s grid=%u block=%u start=%lld end=%lld step=%lld globals=%d%s\n",
                        kernel_name, grid, block, (long long)start, (long long)end, (long long)step,
                        num_globals, needs_doacross ? " cooperative" : "");
            p_cuMemcpyDtoH(env, d_env, (size_t)env_size);
            for (i = 0; i < num_ptr_fields; ++i)
            {
                if (d_bufs[i] && d_sizes[i] > 0 && host_ptrs[i])
                    p_cuMemcpyDtoH(host_ptrs[i], d_bufs[i], (size_t)d_sizes[i]);
                // Restore the original host pointer in the env struct (the whole
                // struct was copied back and its pointer fields now hold device
                // addresses).
                if (host_ptrs[i])
                    *(void **)((char *)env + ptr_offsets[i]) = host_ptrs[i];
            }
            for (i = 0; i < num_globals; ++i)
            {
                CUdeviceptr dptr = 0;
                size_t dsize = 0;
                if (p_cuModuleGetGlobal(&dptr, &dsize, mod, global_names[i]) == CUDA_SUCCESS && dptr)
                    p_cuMemcpyDtoH(global_ptrs[i], dptr, (size_t)global_sizes[i]);
            }
            device_ran = 1;
        }
    }

    for (i = 0; i < num_ptr_fields; ++i)
        if (d_bufs[i])
            p_cuMemFree(d_bufs[i]);
    free(d_bufs);
    free(d_sizes);
    free(host_ptrs);
    p_cuMemFree(d_env);

    if (!device_ran)
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
}
