#ifndef SGPL_GPU_RUNTIME_H
#define SGPL_GPU_RUNTIME_H

#include <stdint.h>

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
    int32_t doacross_num_sync_ids);

#ifdef __cplusplus
}
#endif

#endif
