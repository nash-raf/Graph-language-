#include "semantic_stage_plan.h"
/* No predictor, size threshold, sampling or calibration. Every decision is a
 * semantic/representation check, an explicit disable, or allocation refusal.
 * -1 means refused before effects; a nonnegative result is fully committed. */
int32_t sgpl_semantic_activate_v1(sgpl_exec_ctx *c,int32_t v) {
  if(c->array_append_log)c->array_append_log[c->array_work_ordinal]=v;
  else c->next_frontier[(*c->append_head)++]=v;
  return 1;
}
static int sgpl_semantic_join(uint32_t kind,int32_t a,int32_t b,int32_t *out) {
  switch(kind) {
    case SGPL_JOIN_SMIN:*out=a<b?a:b;break;
    case SGPL_JOIN_SMAX:*out=a>b?a:b;break;
    case SGPL_JOIN_OR:*out=(int32_t)((uint32_t)a|(uint32_t)b);break;
    case SGPL_JOIN_AND:*out=(int32_t)((uint32_t)a&(uint32_t)b);break;
    default:return 0;
  }
  return *out!=a;
}
static int sgpl_semantic_before(const int32_t *values,int32_t a,int32_t b,int maximum) {
  return maximum?values[a]>values[b]:values[a]<values[b];
}
static void sgpl_semantic_heap_up(int32_t *heap,int32_t *pos,const int32_t *values,int32_t at,int maximum) {
  int32_t v=heap[at];
  while(at) {int32_t parent=(at-1)/2;if(!sgpl_semantic_before(values,v,heap[parent],maximum))break;
    heap[at]=heap[parent];pos[heap[at]]=at;at=parent;}
  heap[at]=v;pos[v]=at;
}
/* Identity-transfer signed extrema admit label-setting: after the least (or
 * greatest) pending label is removed, no later propagation can improve it.
 * Bitwise joins instead use a fair pending-set queue on a finite-height lattice.
 * All work is serial and owned here: live reads introduce no parallel race. */
static void sgpl_semantic_fixed_point(AutoGraphMeta *m,const int32_t *input,int32_t count,
    int32_t *values,uint32_t kind,int32_t *work,int32_t *positions) {
  int32_t n=(int32_t)m->csr_n,used=0;
  for(int32_t v=0;v<n;++v)positions[v]=-1;
  int heap=kind==SGPL_JOIN_SMIN||kind==SGPL_JOIN_SMAX,maximum=kind==SGPL_JOIN_SMAX;
  for(int32_t i=0;i<count;++i)if(positions[input[i]]<0) {
    int32_t v=input[i];work[used]=v;positions[v]=used++;
    if(heap)sgpl_semantic_heap_up(work,positions,values,used-1,maximum);
  }
  if(heap)while(used) {
    int32_t u=work[0],last=work[--used];positions[u]=-2;
    if(used) {
      int32_t at=0;
      while(2*(int64_t)at+1<used) {
        int32_t child=2*at+1;
        if(child+1<used&&sgpl_semantic_before(values,work[child+1],work[child],maximum))++child;
        if(!sgpl_semantic_before(values,work[child],last,maximum))break;
        work[at]=work[child];positions[work[at]]=at;at=child;
      }
      work[at]=last;positions[last]=at;
    }
    for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e) {
      int32_t v=m->csr_col_idx[e],next;
      if(sgpl_semantic_join(kind,values[v],values[u],&next)) {
        values[v]=next;
        if(positions[v]<0){work[used]=v;positions[v]=used++;}
        sgpl_semantic_heap_up(work,positions,values,positions[v],maximum);
      }
    }
  } else {
    int32_t head=0,tail=used==n?0:used;
    while(used) {
      int32_t u=work[head];head=(head+1==n?0:head+1);--used;positions[u]=-1;
      for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e) {
        int32_t v=m->csr_col_idx[e],next;
        if(sgpl_semantic_join(kind,values[v],values[u],&next)) {
          values[v]=next;
          if(positions[v]<0){work[tail]=v;tail=(tail+1==n?0:tail+1);positions[v]=0;++used;}
        }
      }
    }
  }
}
/* Topology facts belong to the preparation thread. Native mutation entry points
 * bump the existing generation, including in-place edits; layout/pointer/size
 * changes also refuse reuse. No user callback runs while a view is built. */
typedef struct {
  AutoGraphMeta *meta;
  const int64_t *rows;
  const int32_t *columns;
  int32_t *sources;
  int64_t n,m;
  int epoch;
  uint64_t generation;
} SgplSemanticTopology;
static pthread_key_t sgpl_semantic_topology_key;
static pthread_once_t sgpl_semantic_topology_once=PTHREAD_ONCE_INIT;
static int sgpl_semantic_topology_status;
static void sgpl_semantic_topology_destroy(void *opaque) {
  SgplSemanticTopology *s=opaque;free(s->sources);free(s);
}
static void sgpl_semantic_topology_init(void) {
  sgpl_semantic_topology_status=pthread_key_create(&sgpl_semantic_topology_key,sgpl_semantic_topology_destroy);
}
static SgplSemanticTopology *sgpl_semantic_topology(AutoGraphMeta *m) {
  pthread_once(&sgpl_semantic_topology_once,sgpl_semantic_topology_init);
  if(sgpl_semantic_topology_status)return NULL;
  SgplSemanticTopology *s=pthread_getspecific(sgpl_semantic_topology_key);
  if(!s) {
    s=calloc(1,sizeof(*s));if(!s)return NULL;
    if(pthread_setspecific(sgpl_semantic_topology_key,s)){free(s);return NULL;}
  }
  uint64_t generation=atomic_load_explicit(&g_cc_topology_generation,memory_order_relaxed);
  if(s->meta==m&&s->rows==m->csr_row_ptr&&s->columns==m->csr_col_idx&&
      s->n==m->csr_n&&s->m==m->csr_m&&s->epoch==m->layout_epoch&&s->generation==generation)return s;
  if(m->csr_row_ptr[0]!=0||m->csr_row_ptr[m->csr_n]!=m->csr_m||
      (uint64_t)m->csr_m>SIZE_MAX/4)return NULL;
  int32_t *sources=malloc((size_t)(m->csr_m?m->csr_m:1)*4);
  if(!sources)return NULL;
  for(int32_t u=0;u<m->csr_n;++u) {
    int64_t first=m->csr_row_ptr[u],last=m->csr_row_ptr[u+1];
    if(first<0||last<first||last>m->csr_m){free(sources);return NULL;}
    for(int64_t e=first;e<last;++e) {
      if(m->csr_col_idx[e]<0||m->csr_col_idx[e]>=m->csr_n){free(sources);return NULL;}
      sources[e]=u;
    }
  }
  free(s->sources);
  *s=(SgplSemanticTopology){m,m->csr_row_ptr,m->csr_col_idx,sources,m->csr_n,m->csr_m,m->layout_epoch,generation};
  if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[semantic-stage] topology validated\n");
  return s;
}

/* Each destination has one owner. Eight-byte keys carry only the original pair
 * ordinal and CSR edge index; u/v and first-match weights come from topology.
 * Stable chunk prefixes preserve every destination's original operand order.
 * Distinct notification slots are compacted in original global pair order.
 * These facts require locality, not an associative or commutative operation. */
#define SGPL_SEMANTIC_LANES 64
typedef struct {
  AutoGraphMeta *meta;
  const int32_t *input,*sources;
  const int64_t *weights;
  const int32_t *values;
  uint8_t *keep;
  uint32_t preparation;
  int32_t count,parts;
  int64_t positions[SGPL_SEMANTIC_LANES*SGPL_SEMANTIC_LANES];
  int64_t work[SGPL_SEMANTIC_LANES+1],offsets[SGPL_SEMANTIC_LANES+1];
  uint64_t *keys;
  sgpl_exec_ctx ctx;
  const sgpl_semantic_stage_plan_v1 *plan;
  const int32_t *row_sources;
  const int64_t *row_edges,*rows;
} SgplSemanticDispatch;
static int sgpl_semantic_keep_edge(const SgplSemanticDispatch *j,int32_t u,int64_t edge) {
  if(!(j->preparation&64u))return 1;
  int32_t candidate=j->values[u];
  if(j->plan->weighted)candidate=(int32_t)((uint32_t)candidate+
      (uint32_t)(j->meta->csr_weights?j->meta->csr_weights[j->weights[edge]]:1));
  // Only the independently certified strict signed minimum reaches this test.
  // Target values can only decrease; an initially ineffective candidate stays
  // ineffective. Candidate arithmetic is the original modular i32 expression.
  return candidate<j->values[j->meta->csr_col_idx[edge]];
}
static void sgpl_semantic_count(void *opaque,int32_t worker,int32_t width) {
  SgplSemanticDispatch *j=opaque;AutoGraphMeta *m=j->meta;
  for(int32_t chunk=worker;chunk<j->parts;chunk+=width) {
    int64_t *positions=j->positions+(size_t)chunk*j->parts,work=0;
    int64_t lo=(int64_t)j->count*chunk/j->parts,hi=(int64_t)j->count*(chunk+1)/j->parts;
    for(int64_t i=lo;i<hi;++i) {
      if(j->keep&&!j->keep[i])continue;
      int32_t u=j->input[i];
      for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e)if(sgpl_semantic_keep_edge(j,u,e)) {
        ++positions[m->csr_col_idx[e]%j->parts];++work;
      }
    }
    j->work[chunk+1]=work;
  }
}
static void sgpl_semantic_fill(void *opaque,int32_t worker,int32_t width) {
  SgplSemanticDispatch *j=opaque;AutoGraphMeta *m=j->meta;
  for(int32_t chunk=worker;chunk<j->parts;chunk+=width) {
    int64_t *positions=j->positions+(size_t)chunk*j->parts,ordinal=j->work[chunk];
    int64_t lo=(int64_t)j->count*chunk/j->parts,hi=(int64_t)j->count*(chunk+1)/j->parts;
    for(int64_t i=lo;i<hi;++i) {
      if(j->keep&&!j->keep[i])continue;
      int32_t u=j->input[i];
      for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e)if(sgpl_semantic_keep_edge(j,u,e)) {
        j->keys[positions[m->csr_col_idx[e]%j->parts]++]=((uint64_t)ordinal<<32)|(uint32_t)e;
        j->ctx.array_append_log[ordinal++]=-1;
      }
    }
  }
}
static void sgpl_semantic_execute(void *opaque,int32_t worker,int32_t width) {
  SgplSemanticDispatch *j=opaque;sgpl_exec_ctx ctx=j->ctx;
  for(int32_t part=worker;part<j->parts;part+=width) {
    if(j->rows) {
      for(int64_t v=part;v<j->meta->csr_n;v+=j->parts)
        j->plan->row(&ctx,(int32_t)v,j->row_sources,j->row_edges,j->rows[v],j->rows[v+1]);
    } else for(int64_t i=j->offsets[part];i<j->offsets[part+1];++i) {
      uint64_t key=j->keys[i];int32_t e=(int32_t)(uint32_t)key,u=j->sources[e],v=j->meta->csr_col_idx[e];
      ctx.array_work_ordinal=(int64_t)(key>>32);ctx.array_append_seq=0;
      ctx.array_edge_index=j->weights?j->weights[e]:e;
      j->plan->pair(NULL,&ctx,u,v);
    }
  }
}
int32_t sgpl_try_semantic_stage_v1(void *graph,const sgpl_semantic_stage_plan_v1 *p,
    const int32_t *input,int32_t count,int32_t **output,int64_t *capacity,void *values,int64_t extent) {
  if(!p||p->version!=SGPL_SEMANTIC_STAGE_VERSION||p->bytes!=sizeof(*p)||p->context_bytes!=sizeof(sgpl_exec_ctx)||
      !(p->rules&(512u|1024u|2048u))||(p->rules&~(32u|64u|512u|1024u|2048u))||p->reserved||!p->pair||
      ((p->rules&SGPL_SEMANTIC_PREPARATION_RULES)&&(!(p->rules&1536u)||p->element_bytes!=4))||
      (p->element_bytes!=4&&p->element_bytes!=8)||p->snapshot_slot>=64||p->weighted>1||
      ((p->rules&2048u)&&(p->join<SGPL_JOIN_SMIN||p->join>SGPL_JOIN_AND||p->weighted||p->element_bytes!=4))||
      count<0||(!input&&count)||!output||!capacity||*capacity<0||extent<0||!values||
      getenv("SGPL_NO_EXEC_SIMPLIFY")||getenv("SGPL_NO_SEMANTIC_STAGE")||
      (getenv("SGPL_GPU_BACKEND")&&strcmp(getenv("SGPL_GPU_BACKEND"),"0")))return -1;
  AutoGraphMeta *m=find_meta(graph);
  if(!m||m->current_layout!=LAYOUT_CSR||m->csr_n<0||m->csr_n>INT32_MAX||m->csr_m<0||
      m->csr_m>INT32_MAX||extent<m->csr_n||!m->csr_row_ptr||(!m->csr_col_idx&&m->csr_m)||
      (p->weighted&&m->csr_weights!=*(int32_t **)((char *)graph+32))||
      m->csr_row_ptr!=*(int64_t **)((char *)graph+16)||m->csr_col_idx!=*(int32_t **)((char *)graph+24))return -1;
  int64_t n=m->csr_n,pairs=0;size_t bytes=(size_t)n*p->element_bytes;
  if((uint64_t)n>SIZE_MAX/p->element_bytes||(uint64_t)count>SIZE_MAX/4||
      (uint64_t)*capacity>SIZE_MAX/4||
      sgpl_relax_overlap((uintptr_t)values,bytes,(uintptr_t)graph,64)||
      sgpl_relax_overlap((uintptr_t)values,bytes,(uintptr_t)m->csr_row_ptr,((size_t)n+1)*8)||
      sgpl_relax_overlap((uintptr_t)values,bytes,(uintptr_t)m->csr_col_idx,(size_t)m->csr_m*4)||
      (m->csr_weights&&sgpl_relax_overlap((uintptr_t)values,bytes,(uintptr_t)m->csr_weights,(size_t)m->csr_m*4))||
      sgpl_relax_overlap((uintptr_t)values,bytes,(uintptr_t)input,(size_t)count*4)||
      sgpl_relax_overlap((uintptr_t)values,bytes,(uintptr_t)*output,(size_t)*capacity*4)||
      sgpl_relax_overlap((uintptr_t)input,(size_t)count*4,(uintptr_t)*output,(size_t)*capacity*4))return -1;
  SgplSemanticTopology *topology=NULL;
  if(p->rules&2048u) {
    /* Retain the accepted R12 realization and its preflight. */
    if(m->csr_row_ptr[0]!=0||m->csr_row_ptr[n]!=m->csr_m)return -1;
    for(int64_t u=0;u<n;++u)if(m->csr_row_ptr[u]<0||m->csr_row_ptr[u+1]<m->csr_row_ptr[u]||m->csr_row_ptr[u+1]>m->csr_m)return -1;
    for(int64_t e=0;e<m->csr_m;++e)if(m->csr_col_idx[e]<0||m->csr_col_idx[e]>=n)return -1;
  } else if(!(topology=sgpl_semantic_topology(m)))return -1;
  int full=count==n;
  for(int32_t i=0;i<count;++i) {
    int32_t u=input[i];if(u<0||u>=n)return -1;full&=u==i;
    int64_t degree=m->csr_row_ptr[u+1]-m->csr_row_ptr[u];if(degree>INT32_MAX-pairs)return -1;pairs+=degree;
  }
  unsigned mode=(p->rules&2048u)?12:(p->rules&1024u)&&p->row&&full?11:(p->rules&512u)?10:0;
  if(!mode)return -1;
  int32_t *work=NULL,*pos=NULL,*sources=NULL,*log=NULL;int64_t *rows=NULL,*edges=NULL;
  void *snapshot=NULL;void **snapshots=NULL;
  SgplSemanticDispatch *dispatch=NULL;const int64_t *weight_indices=NULL;
  if(mode==12) {
    work=malloc((size_t)(n?n:1)*4);pos=malloc((size_t)(n?n:1)*4);
    if(!work||!pos)goto refuse;
    sgpl_semantic_fixed_point(m,input,count,values,p->join,work,pos);
  } else {
    if((uint64_t)pairs>SIZE_MAX/8)return -1;
    if(p->weighted&&sgpl_array_weight_indices(m,&weight_indices)<0)return -1;
    dispatch=calloc(1,sizeof(*dispatch));if(!dispatch)return -1;
    dispatch->meta=m;dispatch->input=input;dispatch->count=count;dispatch->sources=topology->sources;
    dispatch->weights=weight_indices;dispatch->plan=p;
    dispatch->values=values;
    dispatch->preparation=(getenv("SGPL_NO_EXEC_COMPOSITION")||getenv("SGPL_NO_RELAX_SIMPLIFY"))?0:p->rules&SGPL_SEMANTIC_PREPARATION_RULES;
    if(mode==10&&(dispatch->preparation&32u)) {
      uint64_t *seen=calloc((size_t)((n+63)/64?(n+63)/64:1),8);
      dispatch->keep=malloc((size_t)(count?count:1));
      if(!seen||!dispatch->keep){free(seen);goto refuse;}
      for(int32_t i=0;i<count;++i) {
        int32_t u=input[i];uint64_t mask=UINT64_C(1)<<(u&63),*word=seen+((uint32_t)u>>6);
        dispatch->keep[i]=!(*word&mask);*word|=mask;
      }
      free(seen);
    }
    int parts=sgpl_configured_worker_count(),budget=sgpl_current_thread_budget();
    if(budget>0&&parts>budget)parts=budget;
    if(parts<1||sgpl_is_worker_pool_thread())parts=1;
    if(parts>SGPL_SEMANTIC_LANES)parts=SGPL_SEMANTIC_LANES;
    dispatch->parts=parts;
    snapshot=malloc(bytes?bytes:1);snapshots=calloc((size_t)p->snapshot_slot+1,sizeof(void *));
    if(!snapshot||!snapshots)goto refuse;
    int64_t prepared=pairs;
    if(mode==11) {
      log=malloc((size_t)(pairs?pairs:1)*4);
      rows=calloc((size_t)n+1,8);pos=calloc((size_t)(n?n:1),4);
      if(!rows||!pos||!log)goto refuse;
      if(!log)goto refuse;
      for(int32_t u=0;u<n;++u)for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e) {
        if(sgpl_semantic_keep_edge(dispatch,u,e))++rows[m->csr_col_idx[e]+1];log[e]=-1;
      }
      for(int64_t v=0;v<n;++v)rows[v+1]+=rows[v];
      sources=malloc((size_t)(rows[n]?rows[n]:1)*4);edges=malloc((size_t)(rows[n]?rows[n]:1)*8);
      if(!sources||!edges)goto refuse;
      for(int32_t u=0;u<n;++u)for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e) {
        if(sgpl_semantic_keep_edge(dispatch,u,e)) {
          int32_t v=m->csr_col_idx[e];int64_t at=rows[v]+pos[v]++;sources[at]=u;edges[at]=e;
        }
      }
      prepared=rows[n];
    } else {
      sgpl_run_preparation(sgpl_semantic_count,dispatch,parts);
      for(int chunk=0;chunk<parts;++chunk)dispatch->work[chunk+1]+=dispatch->work[chunk];
      int64_t at=0;
      for(int part=0;part<parts;++part) {
        dispatch->offsets[part]=at;
        for(int chunk=0;chunk<parts;++chunk) {
          int64_t *position=&dispatch->positions[chunk*parts+part],size=*position;*position=at;at+=size;
        }
      }
      dispatch->offsets[parts]=at;
      prepared=at;
      dispatch->keys=malloc((size_t)(prepared?prepared:1)*8);
      log=malloc((size_t)(prepared?prepared:1)*4);
      if(!dispatch->keys||!log)goto refuse;
    }
    /* This is the original worst-case output reservation, before effects. */
    if(sgpl_frontier_resize(output,capacity,pairs,0)<0)goto refuse;
    memcpy(snapshot,values,bytes);snapshots[p->snapshot_slot]=snapshot;
    int32_t head=0;sgpl_exec_ctx ctx={0};ctx.graph=graph;ctx.next_frontier=*output;
    ctx.append_head=&head;ctx.array_append_sites=1;ctx.array_work_count=pairs;
    ctx.array_append_log=log;ctx.array_edge_index=-1;
    ctx.array_record_edges=(int64_t *)weight_indices;ctx.array_compact_records=p->weighted?4:0;
    ctx.spatial_snapshots=snapshots;ctx.spatial_snapshot_count=(int32_t)p->snapshot_slot+1;
    dispatch->ctx=ctx;dispatch->rows=rows;dispatch->row_sources=sources;dispatch->row_edges=edges;
    if(mode==10)sgpl_run_preparation(sgpl_semantic_fill,dispatch,parts);
    sgpl_run_preparation(sgpl_semantic_execute,dispatch,parts);
    for(int64_t e=0;e<(mode==11?pairs:prepared);++e)if(log[e]>=0)(*output)[head++]=log[e];
    if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[exec-compose] R%u rules=%u original=%lld prepared=%lld\n",mode,dispatch->preparation,(long long)pairs,(long long)prepared);
    free(dispatch->keep);free(dispatch->keys);free(dispatch);
    free(snapshot);free(snapshots);free(rows);free(pos);free(sources);free(edges);free(log);
    if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[semantic-stage] R%u complete pairs=%lld output=%d\n",mode,(long long)pairs,head);
    return head;
  }
  free(work);free(pos);
  if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[semantic-stage] R12 complete (whole fixed point)\n");
  return 0;
refuse:
  if(dispatch){free(dispatch->keep);free(dispatch->keys);free(dispatch);}
  free(snapshot);free(snapshots);free(rows);free(pos);free(sources);free(edges);free(log);free(work);return -1;
}
