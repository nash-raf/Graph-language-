static SGPL_FULL_FN int32_t sgpl_dag_partition_node_full(void *state,int64_t instance) {
  sgpl_exec_ctx *c=state;
  if(sgpl_exec_failed(c))return __atomic_load_n(c->execution_error,__ATOMIC_ACQUIRE);
  AutoGraphMeta *m=find_meta(c->graph);
  if(c->spatial_group!=SGPL_SPATIAL_NONE&&m&&instance==m->partition_count) {
    sgpl_exec_source_coverage(c,1);if(!sgpl_exec_failed(c))sgpl_fullgraph_prepare_map(c);
  }else if(!sgpl_fullgraph_partition(c,(int32_t)instance))sgpl_exec_partition_body(instance,c);
  return c->execution_error?__atomic_load_n(c->execution_error,__ATOMIC_ACQUIRE):0;
}
static SGPL_FULL_FN void sgpl_independent_spatial_worker_full(void *opaque,int32_t index) {
  SgplIndependentSpatial *j=opaque;sgpl_set_current_worker_index(index);int32_t completed=0;
  for(;;) {
    if(atomic_load_explicit(&j->failed,memory_order_acquire))break;
    int32_t node=atomic_fetch_add_explicit(&j->next,1,memory_order_relaxed);if(node>=j->count)break;
    sgpl_push_thread_budget(1);int32_t rc=sgpl_dag_partition_node_full(j->ctx,j->nodes?j->nodes[node]:node);
    sgpl_pop_thread_budget();++completed;if(rc)atomic_store_explicit(&j->failed,1,memory_order_release);
  }
  if(completed)atomic_fetch_add_explicit(&j->completed,completed,memory_order_relaxed);
}
static SGPL_FULL_FN void sgpl_fullgraph_parallel(int64_t p,void *state) {
  if(!sgpl_fullgraph_partition(state,(int32_t)p))sgpl_exec_partition_body(p,state);
}
