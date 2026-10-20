/* Realization of a compiler-proved symbolic DAG. No dependence inference is
 * done here, and a nonmatching invocation retains the generic scheduler. */
static int sgpl_static_schedule_matches(const sgpl_exec_ctx *c,
    const sgpl_static_schedule_v1 *p) {
  if(!c || !p || p->version!=SGPL_STATIC_SCHEDULE_VERSION ||
     p->kind!=SGPL_SCHEDULE_PREPARE_OWNERS_JOIN || p->obligations!=4 ||
     p->proofs!=(SGPL_SCHEDULE_EFFECT_PROOF|SGPL_SCHEDULE_CONTEXT_PROOF) ||
     c->membership || c->array_append_sites || c->partition_base ||
     c->traversal_kind!=SGPL_TRAVERSE_OWNER_V || !c->ops)return 0;
  unsigned pairs=0,begins=0;
  for(unsigned i=0;i<c->op_count;++i) {
    const sgpl_runtime_op *op=&c->ops[i];
    if(op->capabilities&~(SGPL_OP_PAIR|SGPL_OP_SOURCE_BEGIN|SGPL_OP_NO_SOURCE_ORDINAL))return 0;
    if(op->round_begin || op->round_end || op->source_end || op->snapshot ||
       op->partition_begin || op->partition_end || op->combine)return 0;
    if(op->pair){if((void *)op->pair!=p->pair)return 0;++pairs;}
    if(op->source_begin){if((void *)op->source_begin!=p->source_begin)return 0;++begins;}
  }
  return pairs==1 && begins==(p->source_begin?1u:0u);
}
void autograph_exec_ctx_bind_static_schedule_v1(sgpl_exec_ctx *c,
    const sgpl_static_schedule_v1 *p,int32_t version,uint64_t context_bytes,
    uint64_t operation_bytes) {
  autograph_execution_abi_check_v1(version,context_bytes,operation_bytes);
  if(c && sgpl_static_schedule_matches(c,p))c->static_schedule=p;
}
static int sgpl_static_prepare_owners(sgpl_exec_ctx *c,int32_t partitions,
    int32_t relations,int32_t budget) {
  if(!c->static_schedule)return 0;
  AutoGraphMeta *m=find_meta(c->graph);
  if(getenv("SGPL_NO_STATIC_SCHEDULE") || relations || !c->spatial_coverage_pending ||
     c->spatial_group!=SGPL_SPATIAL_DESTINATION ||
     c->spatial_runtime_group!=SGPL_SPATIAL_DESTINATION || sgpl_ctx_dag_spatial(c)<=0 ||
     !m || partitions!=m->partition_count ||
     !sgpl_static_schedule_matches(c,c->static_schedule))return 0;
  /* The original single ready preparation node receives the whole clamped
   * grant. Preserve that grant and restore the caller's ledger afterward. */
  sgpl_push_thread_budget(budget);
  sgpl_exec_source_coverage(c,1);
  if(c->array_compact_records==6 && !sgpl_exec_failed(c))sgpl_fullgraph_prepare_map(c);
  sgpl_pop_thread_budget();
  c->spatial_coverage_pending=0;
  if(getenv("SGPL_STATIC_SCHEDULE_DEBUG"))fprintf(stderr,
    "[static-schedule] prepare -> owners[%d] -> join budget=%d\n",partitions,budget);
  return 1;
}
