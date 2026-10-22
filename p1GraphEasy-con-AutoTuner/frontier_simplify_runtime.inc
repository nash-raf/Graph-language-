#include "frontier_simplify_plan.h"
/* Plans belong to a context's exclusive scratch lease; no topology-dependent
 * pointers are cached. Every selection uses this invocation's native records. */
void sgpl_exec_bind_simplified_frontier_v1(sgpl_exec_ctx *c,
    const sgpl_frontier_simplify_plan_v1 *p) {
  if(!c||!c->execution_scratch||!p||p->version!=SGPL_FRONTIER_SIMPLIFY_VERSION||
     p->bytes!=sizeof(*p)||!(p->rules&SGPL_EXEC_SIMPLIFY_R4)||
     (p->rules&~(SGPL_EXEC_SIMPLIFY_R4|SGPL_EXEC_SIMPLIFY_R5))||p->reserved||
     p->context_bytes!=sizeof(*c)||p->operation_bytes!=sizeof(sgpl_runtime_op)||
     p->argument_bytes!=sizeof(sgpl_frontier_simplify_args_v1)||
     !p->pair||!p->plain||!p->weighted||
     (!!p->journal_pair!=!!p->journal_plain)||
     (!!p->journal_pair!=!!p->journal_weighted)||
     !c->ops||!c->op_count||(void *)c->ops[0].pair!=p->pair||
     !(c->ops[0].capabilities&SGPL_OP_NO_SOURCE_ORDINAL)||
     c->traversal_kind!=SGPL_TRAVERSE_OWNER_V||!c->array_append_sites)return;
  SgplExecutionScratch *s=c->execution_scratch;
  if(!s->fusion){s->fusion=calloc(1,sizeof(*s->fusion));if(!s->fusion)return;s->total+=sizeof(*s->fusion);}
  s->fusion->simplify_plan=p;
}
static sgpl_frontier_simplify_kernel_v1 sgpl_frontier_simplified_kernel(
    const sgpl_exec_ctx *c,int weighted) {
  SgplExecutionScratch *s=c->execution_scratch;
  const sgpl_frontier_simplify_plan_v1 *p=s&&s->fusion?s->fusion->simplify_plan:NULL;
  if(!p||getenv("SGPL_NO_EXEC_SIMPLIFY")||!c->array_pair_records||c->gpu_group_offsets||
     c->spatial_group!=SGPL_SPATIAL_DESTINATION||
     c->spatial_runtime_group!=SGPL_SPATIAL_DESTINATION||
     c->array_compact_records!=(weighted?4:3)||!c->ops||!c->op_count||
     !(c->ops[0].capabilities&SGPL_OP_NO_SOURCE_ORDINAL))return NULL;
  void *pair=(void *)c->ops[0].pair;
  if(pair==p->pair)return weighted?p->weighted:p->plain;
  if(p->journal_pair&&pair==p->journal_pair)return weighted?p->journal_weighted:p->journal_plain;
  return NULL;
}
