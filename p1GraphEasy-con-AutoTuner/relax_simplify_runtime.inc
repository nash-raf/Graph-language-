/* Private preparation witnesses. No public execution-context field changes.
 * The compiler certifies a bounded min/update/notification callback and a
 * nonescaping, single-use CPU invocation with no external error publisher. */
#include "relax_simplify_plan.h"
typedef struct {
  const sgpl_relax_simplify_plan_v1 *plan;
  int32_t *values,*touched;
  uint64_t *first,*best;
  _Atomic uint32_t *tags;
  size_t capacity,best_capacity;
  uint32_t epoch;
  _Atomic int64_t touched_count;
  int active,consumed;
  AutoGraphMeta *meta;
  int64_t *rows;
  int32_t *columns;
  int layout_epoch;
  uint64_t generation;
  int64_t original,retained,unique;
} SgplRelaxState;
static void sgpl_relax_release(void *opaque) {
  SgplRelaxState *s=opaque;if(!s)return;
  free(s->first);free(s->best);free(s->tags);free(s->touched);free(s);
}
static SgplRelaxState *sgpl_relax_state(const sgpl_exec_ctx *c) {
  SgplExecutionScratch *s=c?c->execution_scratch:NULL;
  return s&&s->fusion&&s->fusion->relax_plan?s->fusion->relax_state:NULL;
}
static int sgpl_relax_disabled(void) {
  const char *v=getenv("SGPL_NO_EXEC_SIMPLIFY"),*r=getenv("SGPL_NO_RELAX_SIMPLIFY");
  return (v&&!strcmp(v,"1"))||(r&&!strcmp(r,"1"));
}
void sgpl_exec_bind_relax_simplification_v1(sgpl_exec_ctx *c,
    const sgpl_relax_simplify_plan_v1 *p,int32_t *values) {
  if(!c||!p||!values||p->version!=1||p->bytes!=sizeof(*p)||
      !p->rules||(p->rules&~224u)||p->weighted>1||p->context_bytes!=sizeof(*c)||
      p->operation_bytes!=sizeof(sgpl_runtime_op)||!p->pair||!c->execution_scratch||
      c->op_count!=1||(void *)c->ops[0].pair!=p->pair||
      c->ops[0].capabilities!=(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL)||
      c->ops[0].round_begin||c->ops[0].round_end||c->ops[0].snapshot||
      c->ops[0].source_begin||c->ops[0].source_end||c->ops[0].partition_begin||
      c->ops[0].partition_end||c->ops[0].combine||c->array_append_sites!=1||
      c->initial_next_size||c->gpu_records)return;
  SgplExecutionScratch *scratch=c->execution_scratch;
  if(!scratch->fusion){scratch->fusion=calloc(1,sizeof(*scratch->fusion));if(!scratch->fusion)return;scratch->total+=sizeof(*scratch->fusion);}
  SgplExecutionFusionLease *lease=scratch->fusion;
  SgplRelaxState *s=lease->relax_state;
  if(!s){s=calloc(1,sizeof(*s));if(!s)return;lease->relax_state=s;lease->relax_release=sgpl_relax_release;scratch->total+=sizeof(*s);}
  s->plan=p;s->values=values;s->active=s->consumed=0;lease->relax_plan=p;
}
static SgplRelaxState *sgpl_relax_prepare(sgpl_exec_ctx *c,AutoGraphMeta *m,int begins,int ends) {
  SgplRelaxState *s=sgpl_relax_state(c);if(s)s->active=0;
  const char *gpu=getenv("SGPL_GPU_BACKEND");
  if(!s||sgpl_relax_disabled()||getenv("SGPL_NO_LEAN_RECORDS")||!SGPL_EXEC_PAIR_BATCH||
      !gpu||strcmp(gpu,"0")||begins||ends||c->traversal_kind!=SGPL_TRAVERSE_OWNER_V||
      c->spatial_group!=SGPL_SPATIAL_DESTINATION||c->spatial_runtime_group!=SGPL_SPATIAL_DESTINATION||
      c->op_count!=1||(void *)c->ops[0].pair!=s->plan->pair||c->array_append_sites!=1||
      c->array_source_count<0||m->csr_n<0||m->csr_n>INT32_MAX||m->csr_m<0||m->csr_m>INT32_MAX||
      m->csr_row_ptr!=*(int64_t **)((char *)c->graph+16)||
      m->csr_col_idx!=*(int32_t **)((char *)c->graph+24)||
      (s->plan->weighted&&m->csr_weights!=*(int32_t **)((char *)c->graph+32))||
      (uint64_t)m->csr_n>SIZE_MAX/sizeof(uint64_t)||s->consumed)return NULL;
  size_t n=(size_t)m->csr_n;
  /* Distinct language slots can still hold aliases. Reservation must never
   * reallocate the accumulator or overwrite it while publishing the frontier. */
  if(!c->array_output_slot||!c->array_output_capacity)return NULL;
  uintptr_t a=(uintptr_t)s->values,b=(uintptr_t)*c->array_output_slot;
  uint64_t output=(uint64_t)*c->array_output_capacity;
  if(output>SIZE_MAX/4||n>SIZE_MAX/4||a>UINTPTR_MAX-n*4||b>UINTPTR_MAX-output*4||
      (a<b+output*4&&b<a+n*4))return NULL;
  if(s->capacity<n) {
    if(n>64u*1024u*1024u/16)return NULL;
    uint64_t *first=calloc(n?n:1,sizeof(*first));
    _Atomic uint32_t *tags=calloc(n?n:1,sizeof(*tags));
    int32_t *touched=malloc((n?n:1)*sizeof(*touched));
    if(!first||!tags||!touched){free(first);free(tags);free(touched);return NULL;}
    free(s->first);free(s->tags);free(s->touched);
    ((SgplExecutionScratch *)c->execution_scratch)->total+=(n-s->capacity)*16;
    s->first=first;s->tags=tags;s->touched=touched;s->capacity=n;s->epoch=0;
  }
  if((s->plan->rules&SGPL_EXEC_SIMPLIFY_R8)&&s->best_capacity<n) {
    if(n>64u*1024u*1024u/24)return NULL;
    uint64_t *best=malloc(n*sizeof(*best));if(!best)return NULL;
    free(s->best);s->best=best;
    ((SgplExecutionScratch *)c->execution_scratch)->total+=(n-s->best_capacity)*8;s->best_capacity=n;
  }
  if(++s->epoch==0){if(s->capacity){memset(s->first,0,s->capacity*8);memset(s->tags,0,s->capacity*4);}s->epoch=1;}
  atomic_store_explicit(&s->touched_count,0,memory_order_relaxed);
  for(int64_t i=0;i<c->array_source_count;++i) {
    int32_t u=c->array_sources[i];if(u<0||u>=m->csr_n)return NULL;
    if((s->first[u]>>32)!=s->epoch)s->first[u]=((uint64_t)s->epoch<<32)|(uint32_t)i;
  }
  s->meta=m;s->rows=m->csr_row_ptr;s->columns=m->csr_col_idx;s->layout_epoch=m->layout_epoch;
  s->generation=atomic_load_explicit(&g_cc_topology_generation,memory_order_relaxed);
  if(n&&(s->plan->rules&SGPL_EXEC_SIMPLIFY_R8))memset(s->best,255,n*8);
  s->active=1;return s;
}
static inline int sgpl_relax_replay(const SgplRelaxState *s,int32_t u,int64_t i) {
  return (s->plan->rules&SGPL_EXEC_SIMPLIFY_R6)&&(uint32_t)s->first[u]!=(uint32_t)i;
}
static inline int32_t sgpl_relax_candidate(const SgplRelaxState *s,const AutoGraphMeta *m,
    const int64_t *weights,int32_t u,int32_t v,int64_t edge) {
  int32_t candidate=s->values[u];
  if(s->plan->weighted) {
    int32_t weight=m->csr_weights?m->csr_weights[weights[edge]]:1;
    candidate=(int32_t)((uint32_t)candidate+(uint32_t)weight);
  }
  return candidate;
}
static inline int sgpl_relax_bound(const SgplRelaxState *s,const AutoGraphMeta *m,
    const int64_t *weights,int32_t u,int32_t v,int64_t edge) {
  return (s->plan->rules&SGPL_EXEC_SIMPLIFY_R7)&&sgpl_relax_candidate(s,m,weights,u,v,edge)>=s->values[v];
}
static inline void sgpl_relax_track(SgplRelaxState *s,int32_t v) {
  if(atomic_load_explicit(&s->tags[v],memory_order_relaxed)!=s->epoch&&
      atomic_exchange_explicit(&s->tags[v],s->epoch,memory_order_relaxed)!=s->epoch) {
    int64_t at=atomic_fetch_add_explicit(&s->touched_count,1,memory_order_relaxed);s->touched[at]=v;
  }
}
/* Called after the ordinary group generation starts, before owner work.
 * Only private, error-free invocations qualify, so the final group accounting
 * is the sole observation of these marks. Kept records use ordinary kernels. */
static void sgpl_relax_begin_groups(sgpl_exec_ctx *c,int sparse) {
  SgplRelaxState *s=sgpl_relax_state(c);if(!s||!s->active)return;
  SgplExecutionScratch *scratch=c->execution_scratch;
  int64_t count=atomic_load_explicit(&s->touched_count,memory_order_relaxed);
  if(c->spatial_groups_seen)for(int64_t i=0;i<count;++i) {
    int32_t v=s->touched[i];c->spatial_groups_seen[v]=1;
    if(sparse&&scratch->group_capacity>=(size_t)s->meta->csr_n&&scratch->group_tags)scratch->group_tags[v]=scratch->group_epoch;
  }
  if(sparse)c->spatial_occupied_groups=count;
  s->consumed=1;
  if(getenv("SGPL_RELAX_SIMPLIFY_STATS"))fprintf(stderr,
      "[relax-simplify] rules=%u original=%lld unique=%lld retained=%lld groups=%lld\n",
      s->plan->rules,(long long)s->original,(long long)s->unique,(long long)s->retained,(long long)count);
}
