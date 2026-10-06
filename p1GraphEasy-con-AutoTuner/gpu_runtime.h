#ifndef SGPL_GPU_RUNTIME_H
#define SGPL_GPU_RUNTIME_H

#include <stdint.h>

/* Registered by the generated code (see the pointee registry in gpu_runtime.c):
 * the array behind a pointer-typed device global, with the byte size known at
 * the registration site. */
void sgpl_gpu_register_pointee(const char *name, void *base, int64_t bytes);

/* Register a sized object by pointer value (see the buffer registry in
 * gpu_runtime.c).  Env fields whose base arrives through the task ABI are marked
 * with cap offset -3 in the descriptor and resolved against this table. */
void sgpl_gpu_register_buffer(void *base, int64_t bytes);

/* Device engine step (stage 1): the generated code registers the kernel that
 * runs one CleanCut source-pair slice, and the engine tries it (partition by
 * partition, over the same slices the CPU partitions use) before falling back.
 *   autograph_gpu_step_register : called by the generated program
 *   autograph_gpu_step_count/name: how many steps were registered
 *   gpup_step_try               : run one partition's slice on the device
 * Returns 1 only when the kernel ran and the context stayed healthy. */
void autograph_gpu_step_register(const char *kernel_name, int32_t step_id);
int autograph_gpu_step_count(void);
const char *autograph_gpu_step_name(void);
int32_t autograph_gpu_step_id(void);
/* Name of the kernel registered for this step id (NULL when no step with that
 * id is registered, or when the id is ambiguous). */
const char *autograph_gpu_step_name_for(int32_t step_id);
int gpup_step_try(const char *kernel_name, const int32_t *pairs, int64_t npairs);
/* Device activation step (destination-owned rows): one thread per partition row
 * with its arcs walked sequentially, the claim's first-wins decided by the
 * destination's single owner exactly as on the CPU -- no atomics.  The frontier
 * append becomes a byte mark per claimed destination; the marks are read back in
 * *claimed_out (ascending vertex order) for the caller to compact. */
/* Pass a name prefixed gpu_step_vp_ for the pull variant (one thread per
 * arc; the launcher then builds and uploads the per-arc source array). */
int gpup_step_v_try(const char *kernel_name, const void *layout_sig, int32_t npart,
                    int64_t *const *rp, const int32_t *const *ci,
                    const int32_t *const *indir, const int64_t *row_counts,
                    const uint8_t *mem, int64_t nmem, const uint8_t **claimed_out);

/* Device cost model (pure; unit-tested by tdg_budget_test via
 * validate_tdg_budget.sh).  Returns SGPL_GPU_OFFLOAD when the device should run
 * the loop, SGPL_GPU_SMALL_TRIPS when the launch overhead dominates, and
 * SGPL_GPU_WAVE_STORM when a DOACROSS wave kernel would grid-sync
 * trip/distance times -- beyond max_waves syncs the CPU doacross path wins. */
enum
{
    SGPL_GPU_OFFLOAD = 0,
    SGPL_GPU_SMALL_TRIPS = 1,
    SGPL_GPU_WAVE_STORM = 2
};

int sgpl_gpu_engine_step_verdict(int64_t arcs, int64_t min_pairs);
int sgpl_gpu_policy_verdict(int64_t trip, int32_t needs_doacross, int64_t doacross_dist,
                            int64_t min_trips, int64_t max_waves);

#ifdef __cplusplus
extern "C" {
#endif

// GPU-accelerated replacement for parallel_for_runtime for DOALL loops.
//
// On success the loop body runs on the device; on any failure (no CUDA driver,
// no device, ineligible body, negative step) this transparently falls back to
// the CPU thread-pool implementation via `body`.
//
//   start/end/step : loop bounds (positive step expected for the GPU path)
//   kernel_name    : name of the NVPTX kernel in the loaded PTX module
//   body           : host wrapper used for the CPU fallback
//   env            : live-in/live-out environment struct
//   env_size       : size of `env` in bytes
//   ptr_offsets    : byte offset of each pointer field in `env`
//   ptr_sizes      : constant byte size of each pointer field (0 => runtime-sized)
//   ptr_cap_offsets: for runtime-sized fields, offset of an int32 capacity field
//                    in `env` (-1 for constant-sized fields)
//   ptr_elem_sizes : element size for runtime-sized fields
//   num_ptr_fields : number of entries in the four descriptor arrays above
//   global_names   : [num_globals] names of device globals (matched against the
//                    loaded PTX module via cuModuleGetGlobal)
//   global_ptrs    : [num_globals] host addresses of the corresponding globals
//   global_sizes   : [num_globals] byte sizes of the corresponding globals
//   num_globals    : number of entries in the three descriptor arrays above
//   needs_doacross : 1 for DOACROSS loops (cooperative wave kernel with
//                    grid.sync()); 0 for plain DOALL loops
//   doacross_num_sync_ids : number of doacross sync ids (passed through to the
//                    CPU fallback)
//   doacross_dist  : the proven constant dependence distance (iterations) for a
//                    DOACROSS loop, 0 for DOALL.  The wave kernel's cost is the
//                    number of cooperative grid syncs, trip/dist, so a
//                    distance-1 recurrence (one sync per iteration) is refused
//                    on cost grounds and run by the CPU doacross path instead.
//   global_pointee_flags : [num_globals] 1 for a global whose type is a pointer
//                    (the language models an array as a pointer variable).  Its
//                    bytes are a host address, so the runtime copies the
//                    *pointee* registered for that name by the generated code
//                    (sgpl_gpu_register_pointee) and stores the device address
//                    of that buffer into the device global; an unregistered
//                    pointer global refuses the offload.  NULL means "no
//                    pointer globals".
//
// DOACROSS loops are launched cooperatively (cuLaunchCooperativeKernel): the
// grid is capped at the occupancy limit so every block is resident and the
// kernel's internal grid.sync() barriers are legal. Any failure (driver too
// old, launch rejected, ...) transparently falls back to the CPU thread-pool
// implementation via `body`.
//
// The PTX text is loaded from the `gpu_embedded_ptx` symbol that the compiler
// embeds into the executable (external linkage constant in the host IR). If
// that symbol is absent (older binaries) the runtime falls back to reading
// "kernels.ptx" from the current working directory.
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
    int64_t doacross_dist);

#ifdef __cplusplus
}
#endif

#endif
