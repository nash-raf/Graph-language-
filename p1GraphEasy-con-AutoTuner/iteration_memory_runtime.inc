#include "iteration_memory_plan.h"
typedef struct {
  const sgpl_iteration_memory_plan_v1 *plan;
  SgplIncomingIndex *index;sgpl_exec_ctx ctx;
  void *values;uint8_t *gates;double seed;
  sgpl_iteration_kernel kernel;sgpl_iteration_map map;
} SgplIterationJob;
static void sgpl_iteration_sources(void *opaque,int32_t worker,int32_t width) {
  SgplIterationJob *j=opaque;int64_t lo=j->index->n*worker/width,hi=j->index->n*(worker+1)/width;
  if(j->gates) {
    memset(j->gates+lo,1,(size_t)(hi-lo));
    if(j->plan->gate)for(int64_t u=lo;u<hi;++u)j->plan->gate(NULL,&j->ctx,(int32_t)u);
  }
  j->map(NULL,&j->ctx,hi,j->index->native_rows,j->gates,j->values,lo);
}
static void sgpl_iteration_destinations(void *opaque,int32_t worker,int32_t width) {
  SgplIterationJob *j=opaque;
  j->kernel(NULL,&j->ctx,(int32_t)(j->index->n*worker/width),(int32_t)(j->index->n*(worker+1)/width),
      j->index->incoming_rows,j->index->sources,j->values,j->seed);
}
int32_t sgpl_try_source_count_v1(void *graph,void **slot) {
  if(!slot||getenv("SGPL_NO_ITERATION_LIFETIMES")||getenv("SGPL_NO_EXEC_SIMPLIFY")||
      (getenv("SGPL_GPU_BACKEND")&&strcmp(getenv("SGPL_GPU_BACKEND"),"0")))return 0;
  AutoGraphMeta *m=find_meta(graph);
  if(!m||m->current_layout!=LAYOUT_CSR||m->csr_n<0||m->csr_n>INT32_MAX||m->csr_m<0||
     *(int64_t *)graph!=m->csr_n||m->csr_row_ptr!=*(int64_t **)((char *)graph+16)||
     m->csr_col_idx!=*(int32_t **)((char *)graph+24)||!m->csr_row_ptr||(m->csr_m&&!m->csr_col_idx)||(!*slot&&m->csr_n))return 0;
  const int64_t *row=m->csr_row_ptr;
  // Validate the entire witness before any array effect. No incoming index,
  // allocation, neighbor visits, deduplication or inferred positivity.
  if(row[0]!=0||row[m->csr_n]!=m->csr_m)return 0;
  for(int64_t u=0;u<m->csr_n;++u)if(row[u]<0||row[u+1]<row[u]||row[u+1]>m->csr_m)return 0;
  for(int64_t u=0;u<m->csr_n;++u)if(row[u+1]!=row[u]) {
    uint32_t value;memcpy(&value,(char *)*slot+4*u,4);
    value+=(uint32_t)((uint64_t)row[u+1]-(uint64_t)row[u]);
    memcpy((char *)*slot+4*u,&value,4);
  }
  if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[iteration-lifetime] R16 activate: modular row counts; no neighbor traversal\n");
  return 1;
}
static int32_t sgpl_iteration_execute(void *graph,const sgpl_iteration_memory_plan_v1 *p,
    const sgpl_iteration_gate_plan_v1 *extra,const sgpl_iteration_storage_plan_v1 *storage,double seed,int32_t rounds) {
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
  uint32_t live=3;int reuse=0;
  if(storage&&storage->version==1&&storage->bytes==sizeof(*storage)&&storage->baseline==p&&
      storage->baseline_pair==p->baseline_pair&&!(storage->rules&~(SGPL_EXEC_R17|SGPL_EXEC_R18))&&
      storage->rules&&!(storage->live_arrays&~3u)&&!getenv("SGPL_NO_ITERATION_LIFETIMES")) {
    uint64_t facts=SGPL_FACT_FINAL_USES;
    // Compatible compiler descriptors carry all three static reuse premises;
    // stride is also checked here before selecting the aliased map storage.
    if(p->stride==8)facts|=SGPL_FACT_MAP_LOCAL|SGPL_FACT_GATE_DISJOINT|SGPL_FACT_KERNEL_DISJOINT;
    SgplProofPlan composed=sgpl_iteration_lifetimes(facts,storage->rules);
    if(composed.rules&SGPL_EXEC_R17)live=storage->live_arrays;
    reuse=!!(composed.rules&SGPL_EXEC_R18);
  }
  SgplIncomingIndex *index=sgpl_incoming_acquire(m);if(!index)return 0;
  void *values=reuse?original_source:malloc((size_t)(m->csr_n?m->csr_n:1)*p->stride);
  uint8_t *gates=malloc((size_t)(m->csr_n?m->csr_n:1));
  if((!values&&m->csr_n)||!gates){if(!reuse)free(values);free(gates);sgpl_incoming_release(index);return 0;}
  int parts=sgpl_configured_worker_count(),budget=sgpl_current_thread_budget();
  if(budget>0&&parts>budget)parts=budget;if(parts<1||sgpl_is_worker_pool_thread())parts=1;
  SgplIterationJob j={.plan=p,.index=index,.values=values,.gates=gates,.seed=seed,.kernel=p->kernel,.map=p->map};
  j.ctx.graph=graph;j.ctx.spatial_source_gate=gates;
  if(extra&&extra->version==1&&extra->bytes==sizeof(*extra)&&extra->rules==SGPL_EXEC_R13&&!extra->reserved&&
      extra->baseline==p&&extra->baseline_pair==p->baseline_pair&&extra->kernel&&extra->map&&!getenv("SGPL_NO_PROOF_REMOVALS")) {
    /* The compiler proves stable, disjoint private reads. Observe the actual
     * predicate once, before any array effects; this handles modular degree
     * overflow and arbitrary immutable filters without assuming positivity. */
    memset(gates,1,(size_t)m->csr_n);
    if(p->gate)for(int32_t u=0;u<m->csr_n;++u)p->gate(NULL,&j.ctx,u);
    int entailed=1;for(int64_t u=0;u<m->csr_n;++u)
      if(index->native_rows[u+1]>index->native_rows[u]&&!gates[u]){entailed=0;break;}
    SgplProofPlan composed=sgpl_proof_removals(SGPL_FACT_GATE_STABLE|(entailed?SGPL_FACT_GATE_WITNESS:0),extra->rules);
    if(composed.rules&SGPL_EXEC_R13) {
      j.kernel=extra->kernel;j.map=extra->map;j.gates=NULL;j.ctx.spatial_source_gate=NULL;free(gates);gates=NULL;
    }
    if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[proof-removals] R13 %s: immutable gate %s on edge-bearing sources; rewrites=%u\n",entailed?"activate":"refuse",entailed?"entailed":"false",composed.rewrites);
  }
  // No fallible preparation remains. Independent map cells precede all owned
  // destination writes; the join precedes buffer rotation and the next round.
  for(int32_t round=0;round<rounds;++round) {
    if(reuse)j.values=*p->source;
    sgpl_run_preparation(sgpl_iteration_sources,&j,parts);
    sgpl_run_preparation(sgpl_iteration_destinations,&j,parts);
    void *old=*p->source;*p->source=*p->destination;*p->destination=old;
  }
  void *final=*p->source;
  if(bytes&&(live&1)&&final!=original_source)memcpy(original_source,final,bytes);
  if(bytes&&(live&2)&&final!=original_destination)memcpy(original_destination,final,bytes);
  *p->source=original_source;*p->destination=original_destination;
  if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[iteration-memory] seed/rotate/map rounds=%d vertices=%lld\n",rounds,(long long)m->csr_n);
  if(getenv("SGPL_SEMANTIC_STAGE_STATS")) {
    if(live!=3)fprintf(stderr,"[iteration-lifetime] R17 activate: live-array mask=%u; unobserved restoration omitted\n",live);
    if(reuse)fprintf(stderr,"[iteration-lifetime] R18 activate: source-local contribution storage; no scratch allocation\n");
  }
  if(!reuse)free(values);free(gates);sgpl_incoming_release(index);return 1;
}
int32_t sgpl_try_iteration_gate_v1(void *graph,const sgpl_iteration_memory_plan_v1 *p,
    const sgpl_iteration_gate_plan_v1 *extra,double seed,int32_t rounds) {
  return sgpl_iteration_execute(graph,p,extra,NULL,seed,rounds);
}
int32_t sgpl_try_iteration_storage_v1(void *graph,const sgpl_iteration_memory_plan_v1 *p,
    const sgpl_iteration_storage_plan_v1 *storage,double seed,int32_t rounds) {
  const sgpl_iteration_gate_plan_v1 *gate=NULL;
  if(storage&&storage->version==1&&storage->bytes==sizeof(*storage)&&storage->baseline==p&&
      p&&storage->baseline_pair==p->baseline_pair&&!(storage->rules&~(SGPL_EXEC_R17|SGPL_EXEC_R18))&&
      !(storage->live_arrays&~3u))gate=storage->gate;
  return sgpl_iteration_execute(graph,p,gate,storage,seed,rounds);
}
int32_t sgpl_try_iteration_memory_v1(void *graph,const sgpl_iteration_memory_plan_v1 *p,double seed,int32_t rounds) {
  return sgpl_try_iteration_gate_v1(graph,p,NULL,seed,rounds);
}
