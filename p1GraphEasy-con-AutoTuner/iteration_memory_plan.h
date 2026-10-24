#ifndef SGPL_ITERATION_MEMORY_PLAN_H
#define SGPL_ITERATION_MEMORY_PLAN_H
#include <stdint.h>
#include "autotuner_runtime.h"
#include "execution_proof_worklist.h"
typedef void (*sgpl_iteration_kernel)(void *,sgpl_exec_ctx *,int32_t,int32_t,const int64_t *,const int32_t *,const void *,double);
typedef void (*sgpl_iteration_map)(void *,sgpl_exec_ctx *,int64_t,const int64_t *,const uint8_t *,void *,int64_t);
typedef void (*sgpl_iteration_gate)(void *,sgpl_exec_ctx *,int32_t);
typedef struct {
  uint32_t version,bytes;
  void *baseline_pair;
  sgpl_iteration_gate gate;
  void **source,**destination;
  sgpl_iteration_kernel kernel;
  sgpl_iteration_map map;
  uint64_t stride;
} sgpl_iteration_memory_plan_v1;
int32_t sgpl_try_iteration_memory_v1(void *,const sgpl_iteration_memory_plan_v1 *,double,int32_t);
/* Additive descriptor: the existing 64-byte plan and entry point stay valid. */
typedef struct {
  uint32_t version,bytes,rules,reserved;
  const sgpl_iteration_memory_plan_v1 *baseline;
  void *baseline_pair;
  sgpl_iteration_kernel kernel;
  sgpl_iteration_map map;
} sgpl_iteration_gate_plan_v1;
int32_t sgpl_try_iteration_gate_v1(void *,const sgpl_iteration_memory_plan_v1 *,
    const sgpl_iteration_gate_plan_v1 *,double,int32_t);
/* Additive, internal proof bundle. Existing descriptors and ABI stay intact.
 * live_arrays: bit 0 source, bit 1 destination; unknown bits refuse. */
typedef struct {
  uint32_t version,bytes,rules,live_arrays;
  const sgpl_iteration_memory_plan_v1 *baseline;
  void *baseline_pair;
  const sgpl_iteration_gate_plan_v1 *gate;
} sgpl_iteration_storage_plan_v1;
int32_t sgpl_try_iteration_storage_v1(void *,const sgpl_iteration_memory_plan_v1 *,
    const sgpl_iteration_storage_plan_v1 *,double,int32_t);
int32_t sgpl_try_source_count_v1(void *,void **);
#endif
