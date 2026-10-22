#include "frontier_simplify_plan.h"
extern int32_t sgpl_frontier_journal_activate_v1(sgpl_exec_ctx *,int32_t);
#if defined(__GNUC__) && defined(__ELF__)
#define SGPL_ORDERED_MIN_FN __attribute__((noinline,section(".sgpl_ordered_min")))
#else
#define SGPL_ORDERED_MIN_FN
#endif
/* R9 has one record per destination and proves its candidate strictly wins.
 * The verified snapshot and target bases remain stable throughout invocation.
 * No target read, comparison, pair dispatch or group bitmap is needed. */
static SGPL_ORDERED_MIN_FN int64_t sgpl_ordered_copy_min_kernel(const sgpl_frontier_simplify_args_v1 *a) {
  sgpl_exec_ctx *c=a->context;SgplRelaxState *s=sgpl_relax_state(c);
  const int32_t *source=c->spatial_snapshots[s->source_slot],*records=a->records;
  int32_t *target=s->values;int stride=c->array_compact_records==4?4:3;
  if(a->error&&__atomic_load_n(a->error,__ATOMIC_ACQUIRE)<0)return -1;
#ifdef SGPL_RELAX_SIMPLIFY_TEST
  extern void sgpl_test_ordered_kernel(const sgpl_frontier_simplify_args_v1 *);
  sgpl_test_ordered_kernel(a);
#endif
  for(int64_t i=a->first;i<a->last;++i) {
    int32_t u=records[stride*i],v=records[stride*i+1];
    c->array_work_ordinal=records[stride*i+2];c->array_append_seq=0;
    target[v]=source[u];sgpl_frontier_journal_activate_v1(c,v);
  }
  return 0;
}
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
  SgplRelaxState *ordered=sgpl_relax_state(c);
  if(ordered&&ordered->active&&(ordered->rules&SGPL_EXEC_SIMPLIFY_R9)&&
      ordered->tracking_elided&&ordered->source_verified&&!getenv("SGPL_NO_EXEC_SIMPLIFY")&&
      ordered->source_slot>=0&&ordered->source_slot<c->spatial_snapshot_count&&
      c->spatial_snapshots[ordered->source_slot]&&
      c->spatial_snapshot_bytes[ordered->source_slot]>=ordered->meta->csr_n*4&&
      c->array_compact_records==(weighted?4:3)&&!c->gpu_group_offsets&&c->op_count==1) {
    void *pair=(void *)c->ops[0].pair;
    const sgpl_append_journal_plan_v1 *journal=s&&s->fusion?s->fusion->journal_plan:NULL;
    if(pair==ordered->plan->pair||(journal&&journal->pair==ordered->plan->pair&&pair==journal->journal_pair))
      return sgpl_ordered_copy_min_kernel;
  }
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
