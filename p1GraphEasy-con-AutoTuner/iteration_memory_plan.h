#ifndef SGPL_ITERATION_MEMORY_PLAN_H
#define SGPL_ITERATION_MEMORY_PLAN_H
#include <stdint.h>
#include "autotuner_runtime.h"
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
#endif
