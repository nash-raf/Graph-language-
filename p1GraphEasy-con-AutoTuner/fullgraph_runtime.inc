/* Stable incoming native occurrences. Format 6 tags a private full-graph
 * payload in the existing context pointer; the public layout stays unchanged. */
#if defined(__GNUC__) && defined(__ELF__)
#define SGPL_FULL_FN __attribute__((noinline,section(".sgpl_full_runtime")))
#else
#define SGPL_FULL_FN
#endif
typedef void (*SgplFullKernel)(void *,sgpl_exec_ctx *,int32_t,int32_t,const int64_t *,const int32_t *,const void *);
typedef void (*SgplFullMap)(void *,sgpl_exec_ctx *,int64_t,const int64_t *,const uint8_t *,void *);
typedef struct SgplIncomingIndex {
  _Atomic unsigned refs;AutoGraphMeta *meta;
  int64_t n,m,*incoming_rows,*native_rows;int32_t *sources;
  const int64_t *published_rows;const int32_t *published_columns;
  int32_t epoch;uint64_t generation;
} SgplIncomingIndex;
typedef struct {
  SgplFullKernel kernel;SgplFullMap map;size_t stride;
  SgplIncomingIndex *index;void *values;int ready;unsigned pair_slot;
} SgplFullState;
static SGPL_FULL_FN SgplFullState *sgpl_fullgraph_state(const sgpl_exec_ctx *c) {
  return c&&c->array_compact_records==6?(SgplFullState *)c->gpu_records:NULL;
}
#include "append_journal_runtime.inc"
static pthread_mutex_t sgpl_incoming_lock=PTHREAD_MUTEX_INITIALIZER;
static SgplIncomingIndex *sgpl_incoming_cache;
static SGPL_FULL_FN void sgpl_incoming_release(SgplIncomingIndex *s) {
  if(!s||atomic_fetch_sub_explicit(&s->refs,1,memory_order_acq_rel)!=1)return;
  free(s->incoming_rows);free(s->native_rows);free(s->sources);free(s);
}
__attribute__((destructor)) static SGPL_FULL_FN void sgpl_incoming_destroy(void){sgpl_incoming_release(sgpl_incoming_cache);}
#include "fullgraph_index_parallel.inc"
static SGPL_FULL_FN SgplIncomingIndex *sgpl_incoming_build(AutoGraphMeta *m) {
  if(m->csr_n<0||m->csr_n>INT32_MAX||m->csr_m<0||
     (uint64_t)m->csr_n>=SIZE_MAX/8||(uint64_t)m->csr_m>SIZE_MAX/4)return NULL;
  SgplIncomingIndex *s=calloc(1,sizeof(*s));if(!s)return NULL;atomic_init(&s->refs,1);
  s->meta=m;s->n=m->csr_n;s->m=m->csr_m;s->published_rows=m->csr_row_ptr;s->published_columns=m->csr_col_idx;
  s->epoch=m->layout_epoch;s->generation=atomic_load_explicit(&g_cc_topology_generation,memory_order_acquire);
  s->incoming_rows=calloc((size_t)s->n+1,8);s->native_rows=malloc(((size_t)s->n+1)*8);
  s->sources=malloc((size_t)(s->m?s->m:1)*4);int64_t *cursor=malloc((size_t)(s->n?s->n:1)*8);
  if(!s->incoming_rows||!s->native_rows||!s->sources||!cursor)goto fail;
  if(!m->csr_row_ptr||(s->m&&!m->csr_col_idx))goto fail;
  memcpy(s->native_rows,m->csr_row_ptr,((size_t)s->n+1)*8);
  if(s->native_rows[0]!=0||s->native_rows[s->n]!=s->m)goto fail;
  /* Validate the native row domain before either representation walk. */
  for(int64_t u=0;u<s->n;++u)if(s->native_rows[u]<0||s->native_rows[u+1]<s->native_rows[u]||s->native_rows[u+1]>s->m)goto fail;
  int owned=sgpl_incoming_partitioned(m,s,cursor);if(owned<0)goto fail;
  if(owned){free(cursor);return s;}
  for(int64_t u=0;u<s->n;++u) {
    if(s->native_rows[u]<0||s->native_rows[u+1]<s->native_rows[u]||s->native_rows[u+1]>s->m)goto fail;
    for(int64_t e=s->native_rows[u];e<s->native_rows[u+1];++e) {
      int32_t v=m->csr_col_idx[e];if(v<0||v>=s->n)goto fail;++s->incoming_rows[v+1];
    }
  }
  for(int64_t v=0;v<s->n;++v){s->incoming_rows[v+1]+=s->incoming_rows[v];cursor[v]=s->incoming_rows[v];}
  /* Ascending source and native edge order is the original contribution order
   * within every destination. Parallel owners cannot reorder its additions. */
  for(int64_t u=0;u<s->n;++u)for(int64_t e=s->native_rows[u];e<s->native_rows[u+1];++e)
    s->sources[cursor[m->csr_col_idx[e]]++]=(int32_t)u;
  free(cursor);return s;
fail:free(cursor);sgpl_incoming_release(s);return NULL;
}
static SGPL_FULL_FN int sgpl_incoming_matches(SgplIncomingIndex *s,AutoGraphMeta *m) {
  return s&&m&&m->current_layout==LAYOUT_CSR&&s->meta==m&&s->n==m->csr_n&&s->m==m->csr_m&&s->epoch==m->layout_epoch&&
    s->published_rows==m->csr_row_ptr&&s->published_columns==m->csr_col_idx&&
    s->generation==atomic_load_explicit(&g_cc_topology_generation,memory_order_acquire);
}
static SGPL_FULL_FN SgplIncomingIndex *sgpl_incoming_acquire(AutoGraphMeta *m) {
  pthread_mutex_lock(&sgpl_incoming_lock);SgplIncomingIndex *s=sgpl_incoming_cache;
  if(!sgpl_incoming_matches(s,m)) {
    SgplIncomingIndex *next=sgpl_incoming_build(m);
    if(!next){pthread_mutex_unlock(&sgpl_incoming_lock);return NULL;}
    sgpl_incoming_cache=next;sgpl_incoming_release(s);s=next;
  }
  atomic_fetch_add_explicit(&s->refs,1,memory_order_relaxed);pthread_mutex_unlock(&sgpl_incoming_lock);return s;
}
static SGPL_FULL_FN void sgpl_fullgraph_release(sgpl_exec_ctx *c) {
  SgplFullState *s=sgpl_fullgraph_state(c);if(!s)return;
  sgpl_incoming_release(s->index);free(s->values);free(s);c->gpu_records=NULL;c->array_compact_records=0;
}
/* Versioned compiler/runtime descriptor; validate before reading payload. */
SGPL_FULL_FN void autograph_exec_ctx_bind_fullgraph_v1(sgpl_exec_ctx *c,void *kernel,void *map,uint64_t stride,
    int32_t version,uint64_t context_bytes,uint64_t operation_bytes) {
  autograph_execution_abi_check_v1(version,context_bytes,operation_bytes);
  if(!c||!kernel||getenv("SGPL_NO_FULLGRAPH")||c->gpu_records||c->array_compact_records||c->array_append_sites||c->membership||
     c->traversal_kind!=SGPL_TRAVERSE_OWNER_V)return;
  AutoGraphMeta *m=find_meta(c->graph);
  if(!m||m->current_layout!=LAYOUT_CSR||m->csr_row_ptr!=*(int64_t **)((char *)c->graph+16)||
     m->csr_col_idx!=*(int32_t **)((char *)c->graph+24)||!m->csr_row_ptr)return;
  unsigned pair_slot=0,pairs=0;
  for(unsigned i=0;i<c->op_count;++i) {
    sgpl_runtime_op *op=&c->ops[i];
    if(op->capabilities&~(SGPL_OP_PAIR|SGPL_OP_SOURCE_BEGIN|SGPL_OP_NO_SOURCE_ORDINAL))return;
    if(op->round_begin||op->round_end||op->source_end||op->partition_begin||op->partition_end||op->combine||op->snapshot)return;
    if(op->pair){pair_slot=i;++pairs;}
  }
  if(pairs!=1||stride>SIZE_MAX||(stride&&(uint64_t)m->csr_n>SIZE_MAX/stride))return;
  SgplFullState *s=calloc(1,sizeof(*s));if(!s)return;
  s->index=sgpl_incoming_acquire(m);if(!s->index){free(s);return;}
  if(stride){s->values=malloc((size_t)(m->csr_n?m->csr_n:1)*(size_t)stride);if(!s->values){sgpl_incoming_release(s->index);free(s);return;}}
  s->kernel=(SgplFullKernel)kernel;s->map=(SgplFullMap)map;s->stride=(size_t)stride;s->pair_slot=pair_slot;c->gpu_records=(int32_t *)s;c->array_compact_records=6;
}
static SGPL_FULL_FN int32_t sgpl_fullgraph_begin(sgpl_exec_ctx *c,AutoGraphMeta *m) {
  SgplFullState *s=sgpl_fullgraph_state(c);if(!s)return 0;
  if(!m||s->index->n!=m->csr_n){sgpl_fullgraph_release(c);return SGPL_DAG_ERR_INVALID;}
  if(c->spatial_group!=SGPL_SPATIAL_DESTINATION||c->spatial_runtime_group!=SGPL_SPATIAL_DESTINATION||!sgpl_incoming_matches(s->index,m)) {
    sgpl_fullgraph_release(c);
    if(m->partition_count>0&&autograph_build_clean_cut(c->graph,m->partition_count)<1)return SGPL_DAG_ERR_NOMEM;
    return 0;
  }
  s->ready=0;return 0;
}
static SGPL_FULL_FN void sgpl_fullgraph_prepare_map(sgpl_exec_ctx *c) {
  SgplFullState *s=sgpl_fullgraph_state(c);if(!s||s->ready)return;
  if(s->index->n>0&&s->map)s->map(c->ops[s->pair_slot].state,c,s->index->n,s->index->native_rows,c->spatial_source_gate,s->values);
  s->ready=1;
}
static SGPL_FULL_FN int sgpl_fullgraph_partition(sgpl_exec_ctx *c,int32_t p) {
  SgplFullState *s=sgpl_fullgraph_state(c);if(!s||!s->ready)return 0;
  AutoGraphMeta *m=find_meta(c->graph);if(!m||p<0||p>=m->partition_count)return 0;
  sgpl_exec_ctx local;sgpl_exec_context_copy(&local,c);local.spatial_partition=p;
  local.partition_state=local.partition_base&&local.partition_stride>0?(char *)local.partition_base+(int64_t)p*local.partition_stride:local.partition_base;
  s->kernel(c->ops[s->pair_slot].state,&local,(int32_t)m->partition_start[p],(int32_t)m->partition_start[p+1],s->index->incoming_rows,s->index->sources,s->values);
  return 1;
}
