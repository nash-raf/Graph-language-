#ifndef SGPL_EXECUTION_SIMPLIFY_PLAN_H
#define SGPL_EXECUTION_SIMPLIFY_PLAN_H
#include <stdint.h>
/* Private, additive compiler/runtime contract. Does not change sgpl_exec_ctx. */
#define SGPL_EXEC_SIMPLIFY_VERSION 1u
#define SGPL_EXEC_SIMPLIFY_R1 1u
#define SGPL_EXEC_SIMPLIFY_R2 2u
#define SGPL_EXEC_SIMPLIFY_R3 4u
typedef struct sgpl_execution_simplify_plan_v1 {
  uint32_t version, bytes, rules, reserved;
  void *baseline_kernel, *baseline_map, *kernel, *map;
} sgpl_execution_simplify_plan_v1;
#endif
