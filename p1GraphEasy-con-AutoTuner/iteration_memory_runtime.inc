#include "iteration_memory_plan.h"
typedef struct {
  const sgpl_iteration_memory_plan_v1 *plan;
  SgplIncomingIndex *index;sgpl_exec_ctx ctx;
  void *values;uint8_t *gates;double seed;
} SgplIterationJob;
static void sgpl_iteration_sources(void *opaque,int32_t worker,int32_t width) {
  SgplIterationJob *j=opaque;int64_t lo=j->index->n*worker/width,hi=j->index->n*(worker+1)/width;
  memset(j->gates+lo,1,(size_t)(hi-lo));
  if(j->plan->gate)for(int64_t u=lo;u<hi;++u)j->plan->gate(NULL,&j->ctx,(int32_t)u);
  j->plan->map(NULL,&j->ctx,hi,j->index->native_rows,j->gates,j->values,lo);
}
static void sgpl_iteration_destinations(void *opaque,int32_t worker,int32_t width) {
  SgplIterationJob *j=opaque;
  j->plan->kernel(NULL,&j->ctx,(int32_t)(j->index->n*worker/width),(int32_t)(j->index->n*(worker+1)/width),
      j->index->incoming_rows,j->index->sources,j->values,j->seed);
}
int32_t sgpl_try_iteration_memory_v1(void *graph,const sgpl_iteration_memory_plan_v1 *p,double seed,int32_t rounds) {
  if(!p||p->version!=1||p->bytes!=sizeof(*p)||!p->baseline_pair||!p->kernel||!p->map||
     !p->source||!p->destination||p->source==p->destination||!p->stride||p->stride>128||rounds<=0||
     getenv("SGPL_NO_EXEC_SIMPLIFY")||getenv("SGPL_NO_FULLGRAPH")||getenv("SGPL_NO_ITERATION_MEMORY")||
     (getenv("SGPL_GPU_BACKEND")&&strcmp(getenv("SGPL_GPU_BACKEND"),"0")))return 0;
  AutoGraphMeta *m=find_meta(graph);
  if(!m||m->current_layout!=LAYOUT_CSR||m->csr_n<0||m->csr_n>INT32_MAX||*(int64_t *)graph!=m->csr_n||
      m->csr_row_ptr!=*(int64_t **)((char *)graph+16)||m->csr_col_idx!=*(int32_t **)((char *)graph+24)||
      (uint64_t)m->csr_n>SIZE_MAX/p->stride||(uint64_t)m->csr_n>SIZE_MAX/8)return 0;
  size_t bytes=(size_t)m->csr_n*8;
  void *original_source=*p->source,*original_destination=*p->destination;
  if((bytes&&(!original_source||!original_destination))||
     sgpl_relax_overlap((uintptr_t)original_source,bytes,(uintptr_t)original_destination,bytes))return 0;
  SgplIncomingIndex *index=sgpl_incoming_acquire(m);if(!index)return 0;
  void *values=malloc((size_t)(m->csr_n?m->csr_n:1)*p->stride);
  uint8_t *gates=malloc((size_t)(m->csr_n?m->csr_n:1));
  if(!values||!gates){free(values);free(gates);sgpl_incoming_release(index);return 0;}
  int parts=sgpl_configured_worker_count(),budget=sgpl_current_thread_budget();
  if(budget>0&&parts>budget)parts=budget;if(parts<1||sgpl_is_worker_pool_thread())parts=1;
  SgplIterationJob j={.plan=p,.index=index,.values=values,.gates=gates,.seed=seed};
  j.ctx.graph=graph;j.ctx.spatial_source_gate=gates;
  // No fallible preparation remains. Independent map cells precede all owned
  // destination writes; the join precedes buffer rotation and the next round.
  for(int32_t round=0;round<rounds;++round) {
    sgpl_run_preparation(sgpl_iteration_sources,&j,parts);
    sgpl_run_preparation(sgpl_iteration_destinations,&j,parts);
    void *old=*p->source;*p->source=*p->destination;*p->destination=old;
  }
  void *final=*p->source;
  if(bytes&&final!=original_source)memcpy(original_source,final,bytes);
  if(bytes&&final!=original_destination)memcpy(original_destination,final,bytes);
  *p->source=original_source;*p->destination=original_destination;
  if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[iteration-memory] seed/rotate/map rounds=%d vertices=%lld\n",rounds,(long long)m->csr_n);
  free(values);free(gates);sgpl_incoming_release(index);return 1;
}
