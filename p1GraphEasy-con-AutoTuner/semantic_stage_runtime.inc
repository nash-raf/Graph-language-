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
int32_t sgpl_try_semantic_stage_v1(void *graph,const sgpl_semantic_stage_plan_v1 *p,
    const int32_t *input,int32_t count,int32_t **output,int64_t *capacity,void *values,int64_t extent) {
  if(!p||p->version!=1||p->bytes!=sizeof(*p)||p->context_bytes!=sizeof(sgpl_exec_ctx)||
      !p->rules||(p->rules&~(512u|1024u|2048u))||p->reserved||!p->pair||
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
  if(m->csr_row_ptr[0]!=0||m->csr_row_ptr[n]!=m->csr_m)return -1;
  for(int64_t u=0;u<n;++u)if(m->csr_row_ptr[u]<0||m->csr_row_ptr[u+1]<m->csr_row_ptr[u]||m->csr_row_ptr[u+1]>m->csr_m)return -1;
  for(int64_t e=0;e<m->csr_m;++e)if(m->csr_col_idx[e]<0||m->csr_col_idx[e]>=n)return -1;
  int full=count==n;
  for(int32_t i=0;i<count;++i) {
    int32_t u=input[i];if(u<0||u>=n)return -1;full&=u==i;
    int64_t degree=m->csr_row_ptr[u+1]-m->csr_row_ptr[u];if(degree>INT32_MAX-pairs)return -1;pairs+=degree;
  }
  unsigned mode=(p->rules&2048u)?12:(p->rules&1024u)&&p->row&&full?11:(p->rules&512u)?10:0;
  if(!mode)return -1;
  int32_t *work=NULL,*pos=NULL,*sources=NULL,*log=NULL;int64_t *rows=NULL,*edges=NULL;
  void *snapshot=NULL;void **snapshots=NULL;
  if(mode==12) {
    work=malloc((size_t)(n?n:1)*4);pos=malloc((size_t)(n?n:1)*4);
    if(!work||!pos)goto refuse;
    sgpl_semantic_fixed_point(m,input,count,values,p->join,work,pos);
  } else {
    snapshot=malloc(bytes?bytes:1);snapshots=calloc((size_t)p->snapshot_slot+1,sizeof(void *));
    if(!snapshot||!snapshots)goto refuse;
    if(mode==11) {
      rows=calloc((size_t)n+1,8);pos=calloc((size_t)(n?n:1),4);
      sources=malloc((size_t)(pairs?pairs:1)*4);edges=malloc((size_t)(pairs?pairs:1)*8);
      log=malloc((size_t)(pairs?pairs:1)*4);
      if(!rows||!pos||!sources||!edges||!log)goto refuse;
      for(int64_t e=0;e<pairs;++e){++rows[m->csr_col_idx[e]+1];log[e]=-1;}
      for(int64_t v=0;v<n;++v)rows[v+1]+=rows[v];
      for(int32_t u=0;u<n;++u)for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e) {
        int32_t v=m->csr_col_idx[e];int64_t at=rows[v]+pos[v]++;sources[at]=u;edges[at]=e;
      }
    }
    /* This is the original worst-case output reservation, before effects. */
    if(sgpl_frontier_resize(output,capacity,pairs,0)<0)goto refuse;
    memcpy(snapshot,values,bytes);snapshots[p->snapshot_slot]=snapshot;
    int32_t head=0;sgpl_exec_ctx ctx={0};ctx.graph=graph;ctx.next_frontier=*output;
    ctx.append_head=&head;ctx.array_append_sites=1;ctx.array_work_count=pairs;
    ctx.array_append_log=log;ctx.array_edge_index=-1;
    ctx.spatial_snapshots=snapshots;ctx.spatial_snapshot_count=(int32_t)p->snapshot_slot+1;
    if(mode==11) {
      for(int32_t v=0;v<n;++v)p->row(&ctx,v,sources,edges,rows[v],rows[v+1]);
      for(int64_t e=0;e<pairs;++e)if(log[e]>=0)(*output)[head++]=log[e];
    } else {
      int64_t ordinal=0;
      for(int32_t i=0;i<count;++i) {
        int32_t u=input[i];
        for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e) {
          ctx.array_work_ordinal=ordinal++;ctx.array_append_seq=0;
          p->pair(NULL,&ctx,u,m->csr_col_idx[e]);
        }
      }
    }
    free(snapshot);free(snapshots);free(rows);free(pos);free(sources);free(edges);free(log);
    if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[semantic-stage] R%u complete pairs=%lld output=%d\n",mode,(long long)pairs,head);
    return head;
  }
  free(work);free(pos);
  if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[semantic-stage] R12 complete (whole fixed point)\n");
  return 0;
refuse:
  free(snapshot);free(snapshots);free(rows);free(pos);free(sources);free(edges);free(log);free(work);return -1;
}
