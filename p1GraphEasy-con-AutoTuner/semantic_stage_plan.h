#ifndef SGPL_SEMANTIC_STAGE_PLAN_H
#define SGPL_SEMANTIC_STAGE_PLAN_H
#include <stdint.h>
#include "autotuner_runtime.h"
#define SGPL_EXEC_SIMPLIFY_R10 512u
#define SGPL_EXEC_SIMPLIFY_R11 1024u
#define SGPL_EXEC_SIMPLIFY_R12 2048u
enum { SGPL_JOIN_NONE, SGPL_JOIN_SMIN, SGPL_JOIN_SMAX, SGPL_JOIN_OR, SGPL_JOIN_AND };
typedef void (*sgpl_semantic_pair_fn)(void *,sgpl_exec_ctx *,int32_t,int32_t);
typedef void (*sgpl_semantic_row_fn)(sgpl_exec_ctx *,int32_t,const int32_t *,const int64_t *,int64_t,int64_t);
/* Private and additive. Algebra licenses no traversal or convergence change
 * unless the independently proved rule bit is present. */
typedef struct {
  uint32_t version,bytes,rules,join;
  uint64_t context_bytes;
  sgpl_semantic_pair_fn pair;
  sgpl_semantic_row_fn row;
  uint32_t element_bytes,snapshot_slot,weighted,reserved;
} sgpl_semantic_stage_plan_v1;
int32_t sgpl_semantic_activate_v1(sgpl_exec_ctx *,int32_t);
int32_t sgpl_try_semantic_stage_v1(void *,const sgpl_semantic_stage_plan_v1 *,
    const int32_t *,int32_t,int32_t **,int64_t *,void *,int64_t);
#endif
