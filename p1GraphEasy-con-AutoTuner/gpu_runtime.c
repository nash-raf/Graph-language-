#define _GNU_SOURCE
#include "gpu_runtime.h"
#include "parallel_runtime.h"

#include <dlfcn.h>
#include <pthread.h>
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
typedef CUresult (*cuCtxSetCurrentFn)(CUcontext);
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

/* ── pointee registry ──────────────────────────────────────────────
 * The language models a data array as a *pointer* variable, so a device global
 * of pointer type cannot be copied: its bytes are a host address, and the
 * first load/store through it faults (CUDA error 700) and poisons the context.
 * The generated code therefore registers every array a device kernel can reach
 * (`sgpl_gpu_register_pointee(name, base, bytes)`, emitted where the array's
 * size is known) and the runtime materialises the pointee on the device:
 *
 *     allocate a device buffer, H2D the bytes, store the device address into
 *     the device global, and D2H the bytes back after the launch.
 *
 * A pointer-typed global without a registration refuses the offload (the CPU
 * fallback keeps that shape correct) rather than dereferencing a host address. */
#define GPUP_MAX_POINTEES 64

typedef struct
{
    char name[96];
    void *base;
    int64_t bytes;
} gpu_pointee_desc;

static gpu_pointee_desc g_pointees[GPUP_MAX_POINTEES];
static int g_num_pointees = 0;

void sgpl_gpu_register_pointee(const char *name, void *base, int64_t bytes)
{
    if (!name || !name[0] || !base || bytes <= 0)
        return;
    for (int i = 0; i < g_num_pointees; ++i)
    {
        if (strncmp(g_pointees[i].name, name, sizeof(g_pointees[i].name) - 1) == 0)
        {
            /* Re-registration: the array was reallocated (a larger graph was
             * loaded); the latest base/size wins. */
            g_pointees[i].base = base;
            g_pointees[i].bytes = bytes;
            return;
        }
    }
    if (g_num_pointees >= GPUP_MAX_POINTEES)
        return;
    if (strlen(name) >= sizeof(g_pointees[0].name))
    {
        /* A truncated name would silently miss the module global lookup and
         * leave the device reading a null pointer: refuse it instead. */
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] pointee name too long (%zu bytes): ignored\n",
                    strlen(name));
        return;
    }
    gpu_pointee_desc *D = &g_pointees[g_num_pointees++];
    memcpy(D->name, name, strlen(name) + 1);
    D->base = base;
    D->bytes = bytes;
    if (getenv("SGPL_GPU_DEBUG"))
        fprintf(stderr, "[gpu] pointee registered: %s base=%p bytes=%lld\n", name, base,
                (long long)bytes);
}

/* Value-keyed buffer registry: the generated code registers the sized objects
 * it hands to an env (see sgpl_gpu_register_buffer).  Fields the compiler could
 * not size statically -- their base arrives through the task ABI -- carry cap
 * offset -3 and are resolved here by the pointer value they hold. */
#define GPUB_MAX_BUFFERS 1024

typedef struct
{
    void *base;
    int64_t bytes;
} gpu_buffer_desc;

static gpu_buffer_desc g_buffers[GPUB_MAX_BUFFERS];
static int g_num_buffers = 0;

void sgpl_gpu_register_buffer(void *base, int64_t bytes)
{
    if (!base || bytes <= 0)
        return;
    for (int i = 0; i < g_num_buffers; ++i)
    {
        if (g_buffers[i].base == base)
        {
            g_buffers[i].bytes = bytes; /* re-registered after a reallocation */
            return;
        }
    }
    if (g_num_buffers >= GPUB_MAX_BUFFERS)
        return;
    g_buffers[g_num_buffers].base = base;
    g_buffers[g_num_buffers].bytes = bytes;
    ++g_num_buffers;
    if (getenv("SGPL_GPU_DEBUG"))
        fprintf(stderr, "[gpu] buffer registered: base=%p bytes=%lld\n", base, (long long)bytes);
}

static const gpu_buffer_desc *gpu_lookup_buffer(const void *base)
{
    if (!base)
        return NULL;
    for (int i = 0; i < g_num_buffers; ++i)
        if (g_buffers[i].base == base)
            return &g_buffers[i];
    return NULL;
}

static const gpu_pointee_desc *gpu_lookup_pointee(const char *name)
{
    if (!name)
        return NULL;
    for (int i = 0; i < g_num_pointees; ++i)
        if (strncmp(g_pointees[i].name, name, sizeof(g_pointees[i].name) - 1) == 0)
            return &g_pointees[i];
    return NULL;
}

static void *g_cuda_lib = NULL;
static cuInitFn p_cuInit = NULL;
static cuDeviceGetCountFn p_cuDeviceGetCount = NULL;
static cuDeviceGetFn p_cuDeviceGet = NULL;
static cuCtxCreateFn p_cuCtxCreate = NULL;
static cuCtxSetCurrentFn p_cuCtxSetCurrent = NULL;
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
    /* Making the shared context current per call is what lets *worker* threads
     * (not just the one that created it) launch kernels: without it every other
     * thread's cuModuleGetFunction/cuMemAlloc fails and the loop silently falls
     * back to the CPU. */
    LOAD_SYM(cuCtxSetCurrent);
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

/* ── device cost model ────────────────────────────────────────────────
 * Pure policy so the bounds are unit-testable (tdg_budget_test, driven by
 * validate_tdg_budget.sh) without a GPU:
 *
 *   SGPL_GPU_OFFLOAD      the device runs it
 *   SGPL_GPU_SMALL_TRIPS  below the launch/copy break-even, host pool wins
 *   SGPL_GPU_WAVE_STORM   a DOACROSS wave kernel would grid-sync per wave and
 *                         trip/distance waves exceed the sync budget
 */
int sgpl_gpu_policy_verdict(int64_t trip, int32_t needs_doacross, int64_t doacross_dist,
                            int64_t min_trips, int64_t max_waves)
{
    if (trip <= 0)
        return SGPL_GPU_SMALL_TRIPS;
    if (min_trips > 0 && trip < min_trips)
        return SGPL_GPU_SMALL_TRIPS;
    if (needs_doacross)
    {
        int64_t dist = doacross_dist > 0 ? doacross_dist : 1;
        int64_t waves = (trip + dist - 1) / dist;
        if (max_waves > 0 && waves > max_waves)
            return SGPL_GPU_WAVE_STORM;
    }
    return SGPL_GPU_OFFLOAD;
}

/* Engine-step cost gate (pure, unit-tested by tdg_budget_test): a device step
 * pays one launch plus the per-round state copies (slices, membership, claim
 * marks, pointees), so a step below the arc floor stays on the CPU partitions.
 * min_pairs <= 0 disables the gate. */
int sgpl_gpu_engine_step_verdict(int64_t arcs, int64_t min_pairs)
{
    if (min_pairs > 0 && arcs < min_pairs)
        return SGPL_GPU_SMALL_TRIPS;
    return SGPL_GPU_OFFLOAD;
}

/* ── device engine step (stage 1) ─────────────────────────────────────
 * The CleanCut source-owned slices (AutoGraphMeta::src_pairs) are flat
 * [(u,v),(u,v),...] lists, one per partition, built by enumerating whatever
 * layout the autotuner chose.  The owner-computes assignment is exactly the
 * guarantee that no two partitions write the same slot -- the same reason the
 * CPU runs those partitions in parallel -- so a registered step kernel runs the
 * identical work on the device, partition by partition, and every failure falls
 * back to the CPU dispatch (whose answers are identical by construction). */
#define GPUP_MAX_STEPS 8

static char g_step_names[GPUP_MAX_STEPS][96];
static int32_t g_step_ids[GPUP_MAX_STEPS];
static int g_num_steps = 0;

void autograph_gpu_step_register(const char *name, int32_t step_id)
{
    if (!name || !name[0])
        return;
    /* The compiler emits this in the program's loop preheader, so it runs once
     * per round: the registry is a set of (name, step) pairs, and re-registering
     * must not grow it -- the engine reads the registry to decide whether the
     * step it is dispatching has a device kernel, and a growing registry used to
     * trip its "exactly one registered step" guard from the second round on. */
    for (int i = 0; i < g_num_steps; ++i)
        if (g_step_ids[i] == step_id &&
            strncmp(g_step_names[i], name, sizeof(g_step_names[0]) - 1) == 0)
            return;
    if (g_num_steps >= GPUP_MAX_STEPS)
        return;
    snprintf(g_step_names[g_num_steps], sizeof(g_step_names[0]), "%s", name);
    g_step_ids[g_num_steps] = step_id;
    ++g_num_steps;
    if (getenv("SGPL_GPU_DEBUG"))
        fprintf(stderr, "[gpu] engine step registered: %s (step %d)\n", name, (int)step_id);
}

int autograph_gpu_step_count(void) { return g_num_steps; }

const char *autograph_gpu_step_name(void)
{
    return g_num_steps > 0 ? g_step_names[0] : NULL;
}

int32_t autograph_gpu_step_id(void)
{
    return g_num_steps > 0 ? g_step_ids[0] : -1;
}

/* The step registry is keyed by step id: a context knows which step it is
 * dispatching, so a program with several device-eligible steps resolves each
 * one by its own id.  An id registered under two different names is ambiguous
 * and resolves to none (the caller keeps that step on the CPU). */
const char *autograph_gpu_step_name_for(int32_t step_id)
{
    const char *found = NULL;
    for (int i = 0; i < g_num_steps; ++i)
    {
        if (g_step_ids[i] != step_id)
            continue;
        if (found && strncmp(found, g_step_names[i], sizeof(g_step_names[0]) - 1) != 0)
        {
            if (getenv("SGPL_GPU_DEBUG"))
                fprintf(stderr,
                        "[gpu] step %d registered under two names (%s, %s): CPU\n",
                        (int)step_id, found, g_step_names[i]);
            return NULL;
        }
        found = g_step_names[i];
    }
    return found;
}

/* Pointee materialisation for the engine step: like the outlined path, a
 * pointer-valued global (the language's arrays) must have its *pointee* on the
 * device -- the runtime copies the registered bytes in, patches the device
 * global to the device buffer, and copies the buffers back after each launch so
 * the host sees the device's writes.  Buffers are cached per name so a round
 * loop uploads once and the device accumulates across partitions. */
#define GPUP_STEP_POINTEES 32

typedef struct
{
    char name[96];
    CUdeviceptr dev;
    void *base;
    int64_t bytes;
} gpu_step_pointee;

static gpu_step_pointee g_step_pointees[GPUP_STEP_POINTEES];
static int g_num_stepp = 0;

static int gpu_step_prepare_pointees(CUmodule mod)
{
    int live = 0;
    for (int i = 0; i < g_num_pointees; ++i)
    {
        const gpu_pointee_desc *P = &g_pointees[i];
        CUdeviceptr dptr = 0;
        size_t dsize = 0;
        if (p_cuModuleGetGlobal(&dptr, &dsize, mod, P->name) != CUDA_SUCCESS || !dptr ||
            dsize < sizeof(CUdeviceptr))
            continue; /* not a global of this module */
        gpu_step_pointee *S = NULL;
        for (int j = 0; j < g_num_stepp; ++j)
            if (strcmp(g_step_pointees[j].name, P->name) == 0)
                S = &g_step_pointees[j];
        if (!S)
        {
            if (g_num_stepp >= GPUP_STEP_POINTEES)
            {
                /* Running on with an unpatched device global would hand the
                 * kernel a null pointer: refuse the step instead (the caller
                 * falls back to the CPU partitions). */
                if (getenv("SGPL_GPU_DEBUG"))
                    fprintf(stderr,
                            "[gpu] step pointee registry full (%d): refusing the device step\n",
                            GPUP_STEP_POINTEES);
                return 0;
            }
            S = &g_step_pointees[g_num_stepp++];
            memcpy(S->name, P->name, strlen(P->name) + 1); /* lengths equal by construction */
            S->dev = 0;
        }
        if (S->base != P->base || S->bytes != P->bytes)
        {
            /* reallocated host array (a bigger graph, a new round buffer) */
            if (S->dev)
                p_cuMemFree(S->dev);
            S->dev = 0;
            S->base = P->base;
            S->bytes = P->bytes;
        }
        if (!S->dev)
        {
            if (p_cuMemAlloc(&S->dev, (size_t)S->bytes) != CUDA_SUCCESS)
            {
                S->dev = 0;
                return 0;
            }
        }
        /* Re-upload on every launch.  A round loop rewrites these arrays on the
         * host between steps (the Snapshot op refreshes the round-start shadow,
         * the DSL updates live state), and copy_back keeps the host side current
         * after each launch, so host -> device is always the authoritative sync.
         * Uploading only on first sight left the device reading round-1 data. */
        if (p_cuMemcpyHtoD(S->dev, S->base, (size_t)S->bytes) != CUDA_SUCCESS)
            return 0;
        if (p_cuMemcpyHtoD(dptr, &S->dev, sizeof(S->dev)) != CUDA_SUCCESS)
            return 0;
        ++live;
    }
    return live > 0 || g_num_pointees == 0;
}

static void gpu_step_copy_back(void)
{
    for (int i = 0; i < g_num_stepp; ++i)
        if (g_step_pointees[i].dev && g_step_pointees[i].base)
        {
            p_cuMemcpyDtoH(g_step_pointees[i].base, g_step_pointees[i].dev,
                           (size_t)g_step_pointees[i].bytes);
            if (getenv("SGPL_GPU_DEBUG"))
            {
                const int32_t *h = (const int32_t *)g_step_pointees[i].base;
                int64_t n = g_step_pointees[i].bytes / 4, sum = 0, nz = 0;
                for (int64_t k = 0; k < n; ++k)
                {
                    sum += h[k];
                    nz += (h[k] != 0);
                }
                fprintf(stderr, "[gpu] step copy-back %s: [%d %d %d %d] sum=%lld nonzero=%lld/%lld\n",
                        g_step_pointees[i].name, h[0], h[1], h[2], h[3],
                        (long long)sum, (long long)nz, (long long)n);
            }
        }
}

/* Run one partition's step on the device.  Returns 1 only when the kernel ran
 * and the context stayed healthy; any error returns 0 so the caller uses the
 * CPU partitions.
 *
 * Ownership is per *source*, so the kernel is one thread per source with its
 * pair run iterated sequentially -- exactly the CPU partition body's structure.
 * Parallelising per *pair* instead loses updates on read-modify-write bodies
 * (measured: each partition's share of the increments vanished).  The slice
 * layout groups equal sources contiguously, so the run structure is derived
 * here in one pass; every buffer is re-uploaded per launch because the runtime
 * reuses the slices across steps and rounds.  Kernel ABI:
 *   (i32* pairs, i64 npairs, i32* rows, i64 nrows, i64* begins, i8* env)
 */
static int g_prepare_done = 0;

static CUdeviceptr g_pairs_dev = 0;
static int64_t g_pairs_cap = 0;
static CUdeviceptr g_rows_dev = 0;
static CUdeviceptr g_begins_dev = 0;
static int64_t g_rows_cap = 0;
static int32_t *g_rows_host = NULL;
static int64_t *g_begins_host = NULL;
static int64_t g_begins_cap = 0;

/* Eager device bring-up (driver load, context, module).  The compiler profiles
 * the program by running it, so a first-call bring-up lands inside the profiled
 * region and inflates its measured time; callers that know a device path exists
 * can pay it up front.  Safe to call repeatedly. */
void sgpl_gpu_prepare(void)
{
    if (g_prepare_done)
        return;
    g_prepare_done = 1;
    if (!load_cuda())
        return;
    (void)load_module();
}

int gpup_step_try(const char *name, const int32_t *pairs, int64_t npairs)
{
    if (!name || !name[0] || !pairs || npairs <= 0)
        return 0;
    if (!load_cuda())
        return 0;
    CUmodule mod = load_module();
    if (!mod)
        return 0;
    if (p_cuCtxSetCurrent(g_context) != CUDA_SUCCESS)
        return 0;
    CUfunction kfn = NULL;
    if (p_cuModuleGetFunction(&kfn, mod, name) != CUDA_SUCCESS || !kfn)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] engine step: kernel %s not found; CPU\n", name);
        return 0;
    }
    if (!gpu_step_prepare_pointees(mod))
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] engine step: pointee materialisation failed; CPU\n");
        return 0;
    }

    /* Derive the per-source runs: equal sources are contiguous in the slice. */
    int64_t nrows = 0;
    for (int64_t e = 0; e < npairs; ++e)
        if (e == 0 || pairs[2 * e] != pairs[2 * (e - 1)])
            ++nrows;
    if (nrows <= 0 || nrows > npairs)
        return 0;
    if (nrows + 1 > g_begins_cap)
    {
        int64_t cap = nrows + 1;
        int32_t *r = (int32_t *)realloc(g_rows_host, (size_t)cap * sizeof(int32_t));
        int64_t *b = (int64_t *)realloc(g_begins_host, (size_t)cap * sizeof(int64_t));
        if (!r || !b)
        {
            g_rows_host = r ? r : g_rows_host;
            g_begins_host = b ? b : g_begins_host;
            return 0;
        }
        g_rows_host = r;
        g_begins_host = b;
        g_begins_cap = cap;
    }
    {
        int64_t i = 0;
        for (int64_t e = 0; e < npairs; ++e)
            if (e == 0 || pairs[2 * e] != pairs[2 * (e - 1)])
            {
                g_rows_host[i] = pairs[2 * e];
                g_begins_host[i] = e;
                ++i;
            }
        g_begins_host[i] = npairs;
    }

    size_t pairs_bytes = (size_t)npairs * 2 * sizeof(int32_t);
    size_t rows_bytes = (size_t)nrows * sizeof(int32_t);
    size_t begins_bytes = (size_t)(nrows + 1) * sizeof(int64_t);
    if (g_pairs_cap < (int64_t)pairs_bytes)
    {
        if (g_pairs_dev)
            p_cuMemFree(g_pairs_dev);
        g_pairs_dev = 0;
        g_pairs_cap = 0;
        if (p_cuMemAlloc(&g_pairs_dev, pairs_bytes) != CUDA_SUCCESS)
        {
            g_pairs_dev = 0;
            return 0;
        }
        g_pairs_cap = (int64_t)pairs_bytes;
    }
    if (g_rows_cap < (int64_t)(rows_bytes > begins_bytes ? rows_bytes : begins_bytes))
    {
        if (g_rows_dev)
            p_cuMemFree(g_rows_dev);
        if (g_begins_dev)
            p_cuMemFree(g_begins_dev);
        g_rows_dev = 0;
        g_begins_dev = 0;
        g_rows_cap = 0;
        if (p_cuMemAlloc(&g_rows_dev, rows_bytes) != CUDA_SUCCESS ||
            p_cuMemAlloc(&g_begins_dev, begins_bytes) != CUDA_SUCCESS)
        {
            if (g_rows_dev)
                p_cuMemFree(g_rows_dev);
            if (g_begins_dev)
                p_cuMemFree(g_begins_dev);
            g_rows_dev = 0;
            g_begins_dev = 0;
            return 0;
        }
        g_rows_cap = (int64_t)(rows_bytes > begins_bytes ? rows_bytes : begins_bytes);
    }
    if (p_cuMemcpyHtoD(g_pairs_dev, pairs, pairs_bytes) != CUDA_SUCCESS ||
        p_cuMemcpyHtoD(g_rows_dev, g_rows_host, rows_bytes) != CUDA_SUCCESS ||
        p_cuMemcpyHtoD(g_begins_dev, g_begins_host, begins_bytes) != CUDA_SUCCESS)
        return 0;

    void *env_arg = NULL; /* stage 1: the pair body takes no state or env */
    void *params[6] = {&g_pairs_dev, &npairs, &g_rows_dev, &nrows, &g_begins_dev, &env_arg};
    unsigned int block = 128;
    unsigned int grid = (unsigned int)((nrows + (int64_t)block - 1) / (int64_t)block);
    CUresult lr = p_cuLaunchKernel(kfn, grid, 1, 1, block, 1, 1, 0, NULL, params, NULL);
    if (lr != CUDA_SUCCESS)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] engine step launch failed r=%d; CPU\n", (int)lr);
        return 0;
    }
    if (p_cuCtxSynchronize() != CUDA_SUCCESS)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] engine step sync failed (context poisoned); CPU\n");
        return 0;
    }
    gpu_step_copy_back();
    if (getenv("SGPL_GPU_DEBUG"))
        fprintf(stderr, "[gpu] engine step ran on device: %s pairs=%lld rows=%lld grid=%u\n",
                name, (long long)npairs, (long long)nrows, grid);
    return 1;
}

/* ── device activation step (destination-owned) ─────────────────────────
 * The engine's activation step walks destination-partitioned rows: partition p
 * owns the destinations in [start[p], start[p+1]), and its rows are the sources
 * with arcs into those destinations (row r is source indir[r], its arcs are
 * ci[rp[r] .. rp[r+1])).  The CPU is correct without a claim atomic because a
 * destination has exactly one owner, and that owner walks its rows in order --
 * so the pair body's plain "if (dest_seen[v] == 0) dest_seen[v] = 1" has one
 * winner.  The device mirrors the assignment: one thread per row, arcs walked
 * sequentially.  Two concurrent rows may test-and-set the same destination, but
 * both write the same value (the frontier is a set and the claim payload is
 * round-stable), so the writes are idempotent -- no CAS.
 *
 * The frontier append (an atomic ticket on the CPU) becomes a byte mark written
 * by the claimant: claimed[v] = 1.  The caller compacts the marks into
 * next_frontier in ascending vertex order -- deterministic, and the frontier is
 * an unordered set, so any order is equal.  No atomics anywhere.
 * Kernel ABI: (i32* rowsrc, i64 nrows, i64* rowptr, i32* arcs, i8* mem, i8* env)
 */
static uint8_t *g_v_claim_host = NULL;
static uint8_t *g_v_claim_zero = NULL;
static int64_t g_v_claim_cap = 0;
static CUdeviceptr g_v_claim_dev = 0;
static CUdeviceptr g_v_mem_dev = 0;
static int64_t g_v_mem_cap = 0;
static CUdeviceptr g_v_rowsrc_dev = 0;
static CUdeviceptr g_v_rowptr_dev = 0;
static CUdeviceptr g_v_arcs_dev = 0;
static int32_t *g_v_rowsrc_host = NULL;
static int64_t *g_v_rowptr_host = NULL;
static int32_t *g_v_arcs_host = NULL;
static int64_t g_v_rows_cap = 0;
static int64_t g_v_arcs_cap = 0;
static int64_t g_v_nrows = -1;


int gpup_step_v_try(const char *name, const void *layout_sig, int32_t npart,
                    int64_t *const *rp, const int32_t *const *ci,
                    const int32_t *const *indir, const int64_t *row_counts,
                    const uint8_t *mem, int64_t nmem, const uint8_t **claimed_out)
{
    if (!name || !name[0] || !rp || !ci || !indir || !row_counts || npart <= 0)
        return 0;
    if (claimed_out)
        *claimed_out = NULL;
    if (!load_cuda())
        return 0;
    CUmodule mod = load_module();
    if (!mod)
        return 0;
    if (p_cuCtxSetCurrent(g_context) != CUDA_SUCCESS)
        return 0;
    CUfunction kfn = NULL;
    if (p_cuModuleGetFunction(&kfn, mod, name) != CUDA_SUCCESS || !kfn)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] activation step: kernel %s not found; CPU\n", name);
        return 0;
    }
    if (!gpu_step_prepare_pointees(mod))
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] activation step: pointee materialisation failed; CPU\n");
        return 0;
    }

    /* Flatten the partitions' rows and re-upload them on every launch.  The
     * cut reuses buffer addresses across rebuilds, so a pointer-keyed cache can
     * serve a stale layout -- a wrong-answer path -- while the upload itself is
     * a linear pass over the step's arcs. */
    (void)layout_sig;
    {
        int64_t nrows = 0, narcs = 0;
        for (int32_t p = 0; p < npart; ++p)
        {
            nrows += row_counts[p];
            narcs += rp[p] ? rp[p][row_counts[p]] : 0;
        }
        if (nrows <= 0)
            return 0;
        if (nrows + 1 > g_v_rows_cap)
        {
            int32_t *r = (int32_t *)realloc(g_v_rowsrc_host, (size_t)nrows * sizeof(int32_t));
            int64_t *q = (int64_t *)realloc(g_v_rowptr_host, (size_t)(nrows + 1) * sizeof(int64_t));
            if (!r || !q)
            {
                g_v_rowsrc_host = r ? r : g_v_rowsrc_host;
                g_v_rowptr_host = q ? q : g_v_rowptr_host;
                return 0;
            }
            g_v_rowsrc_host = r;
            g_v_rowptr_host = q;
            g_v_rows_cap = nrows + 1;
        }
        if (narcs > g_v_arcs_cap)
        {
            int32_t *a = (int32_t *)realloc(g_v_arcs_host, (size_t)(narcs > 0 ? narcs : 1) * sizeof(int32_t));
            if (!a)
                return 0;
            g_v_arcs_host = a;
            g_v_arcs_cap = narcs;
        }
        {
            int64_t ro = 0, ao = 0;
            g_v_rowptr_host[0] = 0;
            for (int32_t p = 0; p < npart; ++p)
            {
                int64_t rows = row_counts[p];
                const int64_t *prp = rp[p];
                const int32_t *pci = ci[p];
                const int32_t *pi = indir[p];
                for (int64_t r = 0; r < rows; ++r)
                {
                    g_v_rowsrc_host[ro + r] = pi[r];
                    for (int64_t j = prp[r]; j < prp[r + 1]; ++j)
                        g_v_arcs_host[ao++] = pci[j];
                    g_v_rowptr_host[ro + r + 1] = ao;
                }
                ro += rows;
            }
            g_v_nrows = nrows;
        }
        size_t rows_bytes = (size_t)g_v_nrows * sizeof(int32_t);
        size_t rp_bytes = (size_t)(g_v_nrows + 1) * sizeof(int64_t);
        int64_t narcs_now = g_v_rowptr_host[g_v_nrows];
        size_t arcs_bytes = (size_t)(narcs_now > 0 ? narcs_now : 1) * sizeof(int32_t);
        if (!g_v_rowsrc_dev && p_cuMemAlloc(&g_v_rowsrc_dev, rows_bytes) != CUDA_SUCCESS)
            return 0;
        if (!g_v_rowptr_dev && p_cuMemAlloc(&g_v_rowptr_dev, rp_bytes) != CUDA_SUCCESS)
            return 0;
        if (!g_v_arcs_dev && p_cuMemAlloc(&g_v_arcs_dev, arcs_bytes) != CUDA_SUCCESS)
            return 0;
        if (p_cuMemcpyHtoD(g_v_rowsrc_dev, g_v_rowsrc_host, rows_bytes) != CUDA_SUCCESS ||
            p_cuMemcpyHtoD(g_v_rowptr_dev, g_v_rowptr_host, rp_bytes) != CUDA_SUCCESS ||
            p_cuMemcpyHtoD(g_v_arcs_dev, g_v_arcs_host, arcs_bytes) != CUDA_SUCCESS)
            return 0;
    }

    /* Membership (the frontier gate) is rewritten every round. */
    if (mem && nmem > 0)
    {
        if (g_v_mem_cap < nmem)
        {
            if (g_v_mem_dev)
                p_cuMemFree(g_v_mem_dev);
            g_v_mem_dev = 0;
            g_v_mem_cap = 0;
            if (p_cuMemAlloc(&g_v_mem_dev, (size_t)nmem) != CUDA_SUCCESS)
            {
                g_v_mem_dev = 0;
                return 0;
            }
            g_v_mem_cap = nmem;
        }
        if (p_cuMemcpyHtoD(g_v_mem_dev, mem, (size_t)nmem) != CUDA_SUCCESS)
            return 0;
    }

    /* Claim marks: zero on the device, written by the kernel, read back and
     * re-zeroed for the next round. */
    if (nmem <= 0)
        return 0;
    if (g_v_claim_cap < nmem)
    {
        uint8_t *h = (uint8_t *)realloc(g_v_claim_host, (size_t)nmem);
        uint8_t *z = (uint8_t *)realloc(g_v_claim_zero, (size_t)nmem);
        if (!h || !z)
        {
            g_v_claim_host = h ? h : g_v_claim_host;
            g_v_claim_zero = z ? z : g_v_claim_zero;
            return 0;
        }
        g_v_claim_host = h;
        g_v_claim_zero = z;
        memset(g_v_claim_zero, 0, (size_t)nmem);
        if (g_v_claim_dev)
            p_cuMemFree(g_v_claim_dev);
        g_v_claim_dev = 0;
        g_v_claim_cap = 0;
        if (p_cuMemAlloc(&g_v_claim_dev, (size_t)nmem) != CUDA_SUCCESS)
        {
            g_v_claim_dev = 0;
            return 0;
        }
        g_v_claim_cap = nmem;
    }
    {
        CUdeviceptr cg = 0;
        size_t cgs = 0;
        if (p_cuModuleGetGlobal(&cg, &cgs, mod, "sgpl_gpu_claimed") != CUDA_SUCCESS ||
            !cg || cgs < sizeof(CUdeviceptr))
        {
            if (getenv("SGPL_GPU_DEBUG"))
                fprintf(stderr, "[gpu] activation step: module has no sgpl_gpu_claimed; CPU\n");
            return 0;
        }
        if (p_cuMemcpyHtoD(cg, &g_v_claim_dev, sizeof(CUdeviceptr)) != CUDA_SUCCESS)
            return 0;
    }
    if (p_cuMemcpyHtoD(g_v_claim_dev, g_v_claim_zero, (size_t)nmem) != CUDA_SUCCESS)
        return 0;

    CUdeviceptr dummy_mem = 0;
    CUdeviceptr mem_arg = (mem && nmem > 0) ? g_v_mem_dev : 0;
    void *env_arg = NULL;
    unsigned int block = 128;
    void *params[6] = {&g_v_rowsrc_dev, &g_v_nrows, &g_v_rowptr_dev,
                       &g_v_arcs_dev, &mem_arg, &env_arg};
    int64_t nthreads = g_v_nrows;
    unsigned int grid = (unsigned int)((nthreads + (int64_t)block - 1) / (int64_t)block);
    CUresult lr = p_cuLaunchKernel(kfn, grid, 1, 1, block, 1, 1, 0, NULL, params, NULL);
    (void)dummy_mem;
    if (lr != CUDA_SUCCESS)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] activation step launch failed r=%d; CPU\n", (int)lr);
        return 0;
    }
    if (p_cuCtxSynchronize() != CUDA_SUCCESS)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] activation step sync failed (context poisoned); CPU\n");
        return 0;
    }
    if (p_cuMemcpyDtoH(g_v_claim_host, g_v_claim_dev, (size_t)nmem) != CUDA_SUCCESS)
        return 0;
    gpu_step_copy_back();
    if (claimed_out)
        *claimed_out = g_v_claim_host;
    if (getenv("SGPL_GPU_DEBUG"))
        fprintf(stderr, "[gpu] activation step ran on device: %s rows=%lld grid=%u\n",
                name, (long long)nthreads, grid);
    return 1;
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
    int32_t doacross_num_sync_ids,
    const int32_t *global_pointee_flags,
    int64_t doacross_dist)
{
    if (step <= 0)
    {
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    int64_t trip = (end - start + step - 1) / step;
    if (trip <= 0)
        return;

    /* Cost model: the bounds and the verdict live in sgpl_gpu_policy_verdict
     * (unit-tested without a GPU); here we only read the tunables and report. */
    const int64_t min_trips = getenv("SGPL_GPU_MIN_TRIPS") ? atoll(getenv("SGPL_GPU_MIN_TRIPS")) : 4096;
    const int64_t max_waves = getenv("SGPL_GPU_MAX_WAVES") ? atoll(getenv("SGPL_GPU_MAX_WAVES")) : 8192;
    const int policy = sgpl_gpu_policy_verdict(trip, needs_doacross, doacross_dist, min_trips, max_waves);
    if (policy != SGPL_GPU_OFFLOAD)
    {
        if (getenv("SGPL_GPU_DEBUG"))
        {
            if (policy == SGPL_GPU_SMALL_TRIPS)
                fprintf(stderr, "[gpu] policy: trips=%lld < min_trips=%lld -> CPU\n",
                        (long long)trip, (long long)min_trips);
            else
                fprintf(stderr, "[gpu] policy: doacross waves=%lld (dist=%lld) > max_waves=%lld -> CPU doacross\n",
                        (long long)((trip + (doacross_dist > 0 ? doacross_dist : 1) - 1) /
                                    (doacross_dist > 0 ? doacross_dist : 1)),
                        (long long)doacross_dist, (long long)max_waves);
        }
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    if (!load_cuda())
    {
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    CUresult ctx_r = p_cuCtxSetCurrent(g_context);
    if (ctx_r != CUDA_SUCCESS && getenv("SGPL_GPU_DEBUG"))
        fprintf(stderr, "[gpu] cuCtxSetCurrent failed r=%d tid=%lu\n", (int)ctx_r,
                (unsigned long)pthread_self());

    CUmodule mod = load_module();
    if (!mod)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] fallback: module load failed (%s)\n", kernel_name);
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    CUfunction kfn = NULL;
    CUresult fn_r = p_cuModuleGetFunction(&kfn, mod, kernel_name);
    if (fn_r != CUDA_SUCCESS || !kfn)
    {
        if (getenv("SGPL_GPU_DEBUG"))
            fprintf(stderr, "[gpu] fallback: function lookup failed (%s) r=%d tid=%lu mod=%p\n",
                    kernel_name, (int)fn_r, (unsigned long)pthread_self(), (void *)mod);
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    size_t nfields = num_ptr_fields > 0 ? (size_t)num_ptr_fields : 1;
    size_t nglob = num_globals > 0 ? (size_t)num_globals : 1;
    CUdeviceptr *d_bufs = (CUdeviceptr *)calloc(nfields, sizeof(CUdeviceptr));
    CUdeviceptr *d_pointees = (CUdeviceptr *)calloc(nglob, sizeof(CUdeviceptr));
    int64_t *d_sizes = (int64_t *)calloc(nfields, sizeof(int64_t));
    void **host_ptrs = (void **)calloc(nfields, sizeof(void *));
    if (!d_bufs || !d_sizes || !host_ptrs)
    {
        free(d_bufs);
        free(d_pointees);
        free(d_sizes);
        free(host_ptrs);
        parallel_for_runtime(start, end, step, body, env, needs_doacross,
                             doacross_num_sync_ids);
        return;
    }

    CUdeviceptr d_env = 0;
    if (p_cuMemAlloc(&d_env, (size_t)env_size) != CUDA_SUCCESS)
    {
        free(d_bufs);
        free(d_pointees);
        free(d_sizes);
        free(host_ptrs);
        parallel_for_runtime(start, end, step, body, env, needs_doacross,
                             doacross_num_sync_ids);
        return;
    }

    if (p_cuMemcpyHtoD(d_env, env, (size_t)env_size) != CUDA_SUCCESS)
    {
        p_cuMemFree(d_env);
        free(d_bufs);
        free(d_pointees);
        free(d_sizes);
        free(host_ptrs);
        parallel_for_runtime(start, end, step, body, env, needs_doacross,
                             doacross_num_sync_ids);
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
            if (cap_off == -2)
            {
                /* The compiler marked this field as "not read by the device
                 * body": pass a null device pointer instead of refusing the
                 * launch.  -1 with a non-positive size stays a hard error (it
                 * means the compiler meant to size the field and could not). */
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
            if (cap_off == -3)
            {
                /* Size not knowable at compile time (the base arrives through
                 * the task ABI): resolve it against the value-keyed registry the
                 * generated code populated where the object was created. */
                const gpu_buffer_desc *Buf = gpu_lookup_buffer(host_ptr);
                if (!Buf)
                {
                    if (getenv("SGPL_GPU_DEBUG"))
                        fprintf(stderr, "[gpu] fallback: field %d base=%p is not a registered buffer\n",
                                (int)i, host_ptr);
                    setup_ok = 0;
                    break;
                }
                size = Buf->bytes;
            }
            else if (cap_off < 0 || elem <= 0)
            {
                setup_ok = 0;
                break;
            }
            else
            {
                int32_t cap = *(int32_t *)((char *)env + cap_off);
                if (cap < 0)
                    cap = 0;
                size = (int64_t)cap * elem;
            }
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
            if (global_pointee_flags && global_pointee_flags[i])
            {
                /* Pointer-typed global: its bytes are a host address, so copy
                 * the *pointee* instead and store the device address of that
                 * buffer into the device global. */
                const gpu_pointee_desc *P = gpu_lookup_pointee(global_names[i]);
                if (!P)
                {
                    if (getenv("SGPL_GPU_DEBUG"))
                        fprintf(stderr, "[gpu] fallback: pointer global %s has no registered pointee\n",
                                global_names[i]);
                    setup_ok = 0;
                    break;
                }
                if (dsize < sizeof(CUdeviceptr))
                {
                    setup_ok = 0;
                    break;
                }
                CUdeviceptr dbuf = 0;
                if (p_cuMemAlloc(&dbuf, (size_t)P->bytes) != CUDA_SUCCESS ||
                    p_cuMemcpyHtoD(dbuf, P->base, (size_t)P->bytes) != CUDA_SUCCESS ||
                    p_cuMemcpyHtoD(dptr, &dbuf, sizeof(dbuf)) != CUDA_SUCCESS)
                {
                    if (dbuf)
                        p_cuMemFree(dbuf);
                    setup_ok = 0;
                    break;
                }
                d_pointees[i] = dbuf;
                continue;
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
            {
                int null_fields = 0;
                for (i = 0; i < num_ptr_fields; ++i)
                    if (!d_bufs[i] || d_sizes[i] <= 0)
                        ++null_fields;
                int npointee = 0;
                for (i = 0; i < num_globals; ++i)
                    if (d_pointees[i])
                        ++npointee;
                fprintf(stderr, "[gpu] launched %s grid=%u block=%u start=%lld end=%lld step=%lld globals=%d"
                                " pointees=%d%s fields=%d null=%d env_bytes=%lld\n",
                        kernel_name, grid, block, (long long)start, (long long)end, (long long)step,
                        num_globals, npointee, needs_doacross ? " cooperative" : "",
                        (int)num_ptr_fields, null_fields, (long long)env_size);
            }
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
                if (d_pointees[i])
                {
                    const gpu_pointee_desc *P = gpu_lookup_pointee(global_names[i]);
                    if (P)
                        p_cuMemcpyDtoH(P->base, d_pointees[i], (size_t)P->bytes);
                    continue;
                }
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
    for (i = 0; i < num_globals; ++i)
        if (d_pointees[i])
            p_cuMemFree(d_pointees[i]);
    free(d_pointees);
    free(d_bufs);
    free(d_sizes);
    free(host_ptrs);
    p_cuMemFree(d_env);

    if (!device_ran)
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
}
