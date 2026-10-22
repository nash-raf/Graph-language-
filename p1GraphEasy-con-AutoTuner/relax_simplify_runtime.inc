/* Compiler-certified private CPU minimum invocations. Proof storage must not
 * introduce a contended per-destination operation into edge preparation. */
#include "relax_simplify_plan.h"
typedef struct {
  const sgpl_relax_simplify_plan_v1 *plan;
  int32_t *values;
  uint64_t *seen,*pruned,*best,*improve;
  uint8_t *keep_source;
  size_t seen_capacity,source_capacity,pruned_capacity,best_capacity,improve_capacity,words;
  unsigned rules;
  int32_t source_slot;
  int source_verified,tracking_elided;
  uint8_t *saved_seen;
  int chunks,active,consumed;
  AutoGraphMeta *meta;
  int64_t *rows;
  int32_t *columns;
  int layout_epoch;
  uint64_t generation;
  int64_t original,retained,unique,value_count,ordered_groups;
} SgplRelaxState;
static void sgpl_relax_release(void *opaque) {
  SgplRelaxState *s=opaque;if(!s)return;
  free(s->seen);free(s->pruned);free(s->best);free(s->improve);free(s->keep_source);free(s);
}
static SgplRelaxState *sgpl_relax_state(const sgpl_exec_ctx *c) {
  SgplExecutionScratch *s=c?c->execution_scratch:NULL;
  return s&&s->fusion&&s->fusion->relax_plan?s->fusion->relax_state:NULL;
}
static int sgpl_relax_disabled(void) {
  const char *v=getenv("SGPL_NO_EXEC_SIMPLIFY"),*r=getenv("SGPL_NO_RELAX_SIMPLIFY");
  return (v&&!strcmp(v,"1"))||(r&&!strcmp(r,"1"));
}
static void *sgpl_relax_buffer(sgpl_exec_ctx *c,void *buffer,size_t *capacity,size_t bytes) {
  if(bytes>64u*1024u*1024u)return NULL;
  if(*capacity<bytes) {
    void *next=malloc(bytes);if(!next)return NULL;
    free(buffer);buffer=next;
    ((SgplExecutionScratch *)c->execution_scratch)->total+=bytes-*capacity;*capacity=bytes;
  }
  return buffer;
}
static int sgpl_relax_overlap(uintptr_t a,size_t an,uintptr_t b,size_t bn) {
  if(a>UINTPTR_MAX-an||b>UINTPTR_MAX-bn)return 1;
  return (an&&bn&&a<b+bn&&b<a+an)||(a&&a==b);
}
void sgpl_exec_bind_relax_simplification_v1(sgpl_exec_ctx *c,
    const sgpl_relax_simplify_plan_v1 *p,int32_t *values) {
  (void)c;(void)p;(void)values; /* No extent witness: retained baseline. */
}
void sgpl_exec_bind_relax_simplification_v2(sgpl_exec_ctx *c,
    const sgpl_relax_simplify_plan_v2 *p,int32_t *values,int64_t value_count) {
  if(!c||!p||!values||value_count<0||p->version!=2||p->bytes!=sizeof(*p)||
      !p->rules||(p->rules&~480u)||p->weighted>1||p->context_bytes!=sizeof(*c)||
      p->operation_bytes!=sizeof(sgpl_runtime_op)||!p->pair||!c->execution_scratch||
      c->op_count!=1||(void *)c->ops[0].pair!=p->pair||
      c->ops[0].capabilities!=(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL)||
      c->ops[0].round_begin||c->ops[0].round_end||c->ops[0].snapshot||
      c->ops[0].source_begin||c->ops[0].source_end||c->ops[0].partition_begin||
      c->ops[0].partition_end||c->ops[0].combine||c->array_append_sites!=1||
      c->initial_next_size||c->gpu_records)return;
  SgplExecutionScratch *scratch=c->execution_scratch;
  if(!scratch->fusion){scratch->fusion=calloc(1,sizeof(*scratch->fusion));if(!scratch->fusion)return;scratch->total+=sizeof(*scratch->fusion);}
  SgplExecutionFusionLease *lease=scratch->fusion;SgplRelaxState *s=lease->relax_state;
  if(!s){s=calloc(1,sizeof(*s));if(!s)return;lease->relax_state=s;lease->relax_release=sgpl_relax_release;scratch->total+=sizeof(*s);}
  s->plan=p;s->values=values;s->value_count=value_count;s->active=s->consumed=0;
  s->source_slot=-1;s->source_verified=s->tracking_elided=0;s->saved_seen=NULL;lease->relax_plan=p;
}
void sgpl_exec_bind_ordered_source_v1(sgpl_exec_ctx *c,int32_t slot) {
  SgplRelaxState *s=sgpl_relax_state(c);
  if(s&&!c->gpu_records&&slot>=0&&(s->plan->rules&SGPL_EXEC_SIMPLIFY_R9)&&!s->plan->weighted) {
    s->source_slot=slot;s->source_verified=0;
  }
}
static void sgpl_relax_note_snapshot(sgpl_exec_ctx *c,int32_t slot,const void *base,
    int64_t count,int64_t bytes,int status) {
  SgplRelaxState *s=sgpl_relax_state(c);
  if(s&&slot==s->source_slot)s->source_verified=status>=0&&s->active&&
      (s->rules&SGPL_EXEC_SIMPLIFY_R9)&&base==s->values&&count==s->value_count&&bytes==4;
}
static void sgpl_relax_elide_tracking(sgpl_exec_ctx *c) {
  SgplRelaxState *s=sgpl_relax_state(c);
  if(s&&s->active&&(s->rules&SGPL_EXEC_SIMPLIFY_R9)) {
    s->saved_seen=c->spatial_groups_seen;s->tracking_elided=1;
    c->spatial_groups_seen=NULL;c->spatial_occupied_groups=s->ordered_groups;
  }
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
      s->value_count<m->csr_n||s->consumed)return NULL;
  s->rules=s->plan->rules;
  if(s->plan->weighted)s->rules&=~SGPL_EXEC_SIMPLIFY_R9;
  if(!s->rules)return NULL;
  size_t n=(size_t)m->csr_n,arcs=(size_t)m->csr_m;
  if(!c->array_output_slot||!c->array_output_capacity)return NULL;
  uintptr_t a=(uintptr_t)s->values;uint64_t output=(uint64_t)*c->array_output_capacity;
  if(output>SIZE_MAX/4||sgpl_relax_overlap(a,n*4,(uintptr_t)*c->array_output_slot,(size_t)output*4)||
      sgpl_relax_overlap(a,n*4,(uintptr_t)c->graph,64)||
      sgpl_relax_overlap(a,n*4,(uintptr_t)m->csr_row_ptr,(n+1)*8)||
      sgpl_relax_overlap(a,n*4,(uintptr_t)m->csr_col_idx,arcs*4)||
      (m->csr_weights&&sgpl_relax_overlap(a,n*4,(uintptr_t)m->csr_weights,arcs*4)))return NULL;
  /* A fixed profitability floor avoids O(vertex-domain) scratch work on a
   * tiny traversal. It changes selection only, never the semantic proof. */
  int64_t minimum=4096;const char *floor=getenv("SGPL_RELAX_MIN_WORK");
  if(floor){char *end=NULL;minimum=strtoll(floor,&end,10);if(!end||*end||minimum<0||minimum>INT32_MAX)return NULL;}
  int64_t estimate=0;
  for(int64_t i=0;i<c->array_source_count&&estimate<minimum;++i) {
    int32_t u=c->array_sources[i];if(u<0||u>=m->csr_n)return NULL;
    int64_t first=m->csr_row_ptr[u],last=m->csr_row_ptr[u+1];
    if(first<0||last<first||last>m->csr_m)return NULL;
    estimate+=last-first;
  }
  if(estimate<minimum) {
    if(getenv("SGPL_RELAX_SIMPLIFY_STATS"))fprintf(stderr,"[relax-simplify] baseline reason=small-work original=%lld\n",(long long)estimate);
    return NULL;
  }
  size_t seen_bytes=((n+63)/64)*8;
  void *seen=sgpl_relax_buffer(c,s->seen,&s->seen_capacity,seen_bytes);
  if(seen_bytes&&!seen)return NULL;s->seen=seen;
  void *keep=sgpl_relax_buffer(c,s->keep_source,&s->source_capacity,(size_t)c->array_source_count);
  if(c->array_source_count&&!keep)return NULL;s->keep_source=keep;
  if(seen_bytes)memset(s->seen,0,seen_bytes);
  int32_t previous=INT32_MIN;int ordered=1;
  for(int64_t i=0;i<c->array_source_count;++i) {
    int32_t u=c->array_sources[i];if(u<0||u>=m->csr_n)return NULL;
    uint64_t mask=UINT64_C(1)<<(u&63);uint64_t *word=&s->seen[(uint32_t)u>>6];
    s->keep_source[i]=!(*word&mask);*word|=mask;
    if(s->rules&SGPL_EXEC_SIMPLIFY_R9){int32_t value=s->values[u];if(value<previous)ordered=0;previous=value;}
  }
  if(!ordered)s->rules&=~SGPL_EXEC_SIMPLIFY_R9;
  if(!s->rules)return NULL;
  if(s->rules&SGPL_EXEC_SIMPLIFY_R8) {
    void *best=sgpl_relax_buffer(c,s->best,&s->best_capacity,n*8);
    if(n&&!best)return NULL;s->best=best;
    if(n)memset(s->best,255,n*8);
  }
  s->meta=m;s->rows=m->csr_row_ptr;s->columns=m->csr_col_idx;s->layout_epoch=m->layout_epoch;
  s->generation=atomic_load_explicit(&g_cc_topology_generation,memory_order_relaxed);s->active=1;return s;
}
static int sgpl_relax_prepare_groups(sgpl_exec_ctx *c,SgplRelaxState *s,int chunks) {
  s->chunks=chunks;s->words=(((size_t)s->meta->csr_n+63)/64+7)&~(size_t)7;
  if(!(s->rules&(SGPL_EXEC_SIMPLIFY_R7|SGPL_EXEC_SIMPLIFY_R8|SGPL_EXEC_SIMPLIFY_R9))){s->words=0;return 1;}
  if(s->words>SIZE_MAX/8/(size_t)chunks)return 0;
  size_t bytes=s->words*8*(size_t)chunks;
  void *pruned=sgpl_relax_buffer(c,s->pruned,&s->pruned_capacity,bytes);
  if(bytes&&!pruned)return 0;s->pruned=pruned;
  if(bytes)memset(s->pruned,0,bytes);
  if(s->rules&SGPL_EXEC_SIMPLIFY_R9) {
    void *improve=sgpl_relax_buffer(c,s->improve,&s->improve_capacity,bytes);
    if(bytes&&!improve)return 0;s->improve=improve;if(bytes)memset(s->improve,0,bytes);
  }
  return 1;
}
static inline int sgpl_relax_replay(const SgplRelaxState *s,int32_t u,int64_t i) {
  (void)u;return (s->rules&SGPL_EXEC_SIMPLIFY_R6)&&!s->keep_source[i];
}
static inline int32_t sgpl_relax_candidate(const SgplRelaxState *s,const AutoGraphMeta *m,
    const int64_t *weights,int32_t u,int32_t v,int64_t edge) {
  (void)v;int32_t candidate=s->values[u];
  if(s->plan->weighted)candidate=(int32_t)((uint32_t)candidate+(uint32_t)(m->csr_weights?m->csr_weights[weights[edge]]:1));
  return candidate;
}
static inline int sgpl_relax_bound(const SgplRelaxState *s,const AutoGraphMeta *m,
    const int64_t *weights,int32_t u,int32_t v,int64_t edge) {
  return (s->rules&SGPL_EXEC_SIMPLIFY_R7)&&sgpl_relax_candidate(s,m,weights,u,v,edge)>=s->values[v];
}
static inline void sgpl_relax_track(SgplRelaxState *s,int32_t v,int chunk) {
  s->pruned[(size_t)chunk*s->words+((uint32_t)v>>6)]|=UINT64_C(1)<<(v&63);
}
static void sgpl_relax_begin_groups(sgpl_exec_ctx *c,int sparse) {
  SgplRelaxState *s=sgpl_relax_state(c);if(!s||!s->active)return;
  SgplExecutionScratch *scratch=c->execution_scratch;int64_t count=0;
  if(s->rules&SGPL_EXEC_SIMPLIFY_R9)count=s->ordered_groups;
  else for(size_t w=0;w<s->words;++w) {
    uint64_t bits=0;for(int k=0;k<s->chunks;++k)bits|=s->pruned[(size_t)k*s->words+w];
    while(bits){int32_t v=(int32_t)(64*w+__builtin_ctzll(bits));bits&=bits-1;++count;
      if(c->spatial_groups_seen)c->spatial_groups_seen[v]=1;
      if(sparse&&scratch->group_capacity>=(size_t)s->meta->csr_n&&scratch->group_tags)scratch->group_tags[v]=scratch->group_epoch;
    }
  }
  if(sparse)c->spatial_occupied_groups=count;s->consumed=1;
  if(getenv("SGPL_RELAX_SIMPLIFY_STATS"))fprintf(stderr,
      "[relax-simplify] rules=%u original=%lld unique=%lld retained=%lld groups=%lld\n",
      s->rules,(long long)s->original,(long long)s->unique,(long long)s->retained,(long long)count);
}
