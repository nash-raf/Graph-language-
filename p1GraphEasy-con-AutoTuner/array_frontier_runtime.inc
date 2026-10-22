/* An append slot belongs to one original pair occurrence, never to a worker.
 * The join compacts slots in source/neighbor order. No set conversion occurs. */
int32_t autograph_exec_ctx_bind_array_frontier(sgpl_exec_ctx *c,const int32_t *input,
    int32_t count,int32_t **output,int64_t *capacity,int32_t sites) {
  if(!c||count<0||(!input&&count)||!output||!capacity||*capacity<0||sites<1||c->array_append_sites||
      c->initial_next_size<0||c->initial_next_size>*capacity)
    return SGPL_DAG_ERR_INVALID;
  c->array_sources=input;c->array_source_count=count;c->array_output_slot=output;
  c->array_output_capacity=capacity;c->array_append_sites=sites;
  return 0;
}
int64_t autograph_exec_ctx_occurrence(sgpl_exec_ctx *c) {
  return c&&c->array_append_sites?c->array_source_ordinal:0;
}
/* First-match indices are topology, not an independence certificate. Cache
 * them per preparation thread; a context copies its hints before dispatch.
 * Thread exit releases the cache, and mutation/layout changes invalidate it. */
typedef struct {
  AutoGraphMeta *meta;
  int64_t *rp, *first;
  int32_t *ci;
  int64_t n,m;
  int32_t epoch;
  uint64_t generation;
} SgplArrayWeightIndex;
static pthread_key_t sgpl_array_weight_key;
static pthread_once_t sgpl_array_weight_once=PTHREAD_ONCE_INIT;
static int sgpl_array_weight_key_status;
static void sgpl_array_weight_destroy(void *opaque) {
  SgplArrayWeightIndex *cache=opaque;free(cache->first);free(cache);
}
static void sgpl_array_weight_key_init(void) {
  sgpl_array_weight_key_status=pthread_key_create(&sgpl_array_weight_key,sgpl_array_weight_destroy);
}
static int sgpl_array_weight_indices(AutoGraphMeta *m,const int64_t **out) {
  pthread_once(&sgpl_array_weight_once,sgpl_array_weight_key_init);
  if(sgpl_array_weight_key_status)return SGPL_DAG_ERR_NOMEM;
  SgplArrayWeightIndex *cache=pthread_getspecific(sgpl_array_weight_key);
  if(!cache) {
    cache=calloc(1,sizeof(*cache));if(!cache)return SGPL_DAG_ERR_NOMEM;
    if(pthread_setspecific(sgpl_array_weight_key,cache)) {free(cache);return SGPL_DAG_ERR_NOMEM;}
  }
  uint64_t generation=atomic_load_explicit(&g_cc_topology_generation,memory_order_relaxed);
  if(cache->meta==m&&cache->rp==m->csr_row_ptr&&cache->ci==m->csr_col_idx&&
      cache->n==m->csr_n&&cache->m==m->csr_m&&cache->epoch==m->layout_epoch&&cache->generation==generation) {
    *out=cache->first;return 0;
  }
  if(m->csr_m<0||(uint64_t)m->csr_m>SIZE_MAX/8||(uint64_t)m->csr_n>SIZE_MAX/8)return SGPL_DAG_ERR_NOMEM;
  int64_t *first=malloc((size_t)(m->csr_m?m->csr_m:1)*8),*seen=malloc((size_t)(m->csr_n?m->csr_n:1)*8);
  if(!first||!seen) {free(first);free(seen);return SGPL_DAG_ERR_NOMEM;}
  memset(seen,255,(size_t)m->csr_n*8);
  for(int64_t u=0;u<m->csr_n;++u) {
    if(m->csr_row_ptr[u]<0||m->csr_row_ptr[u+1]<m->csr_row_ptr[u]||m->csr_row_ptr[u+1]>m->csr_m) {
      free(first);free(seen);return SGPL_DAG_ERR_INVALID;
    }
    for(int64_t j=m->csr_row_ptr[u];j<m->csr_row_ptr[u+1];++j) {
      int32_t v=m->csr_col_idx[j];
      if(v<0||v>=m->csr_n) {free(first);free(seen);return SGPL_DAG_ERR_INVALID;}
      if(seen[v]<m->csr_row_ptr[u])seen[v]=j;
      first[j]=seen[v];
    }
  }
  free(seen);free(cache->first);
  *cache=(SgplArrayWeightIndex){m,m->csr_row_ptr,first,m->csr_col_idx,m->csr_n,m->csr_m,m->layout_epoch,generation};
  *out=first;return 0;
}
static int32_t sgpl_array_record_endpoint(const sgpl_exec_ctx *c,int64_t i,int key) {
  return c->gpu_records[(c->array_compact_records==4?4:(c->array_compact_records==3?3:(c->array_compact_records==2?5:(c->array_compact_records?4:3))))*i+key];
}
/* Device grouping retains its original triples/ordinal arrays. Expansion is
 * transactional: allocation failure leaves the CPU representation usable. */
static int32_t sgpl_array_expand_records(sgpl_exec_ctx *c) {
  if(!c->array_compact_records)return 0;
  int64_t n=c->gpu_record_count,*prefix=NULL;
  if((uint64_t)n>SIZE_MAX/12)return SGPL_DAG_ERR_NOMEM;
  int lean=c->array_compact_records==3||c->array_compact_records==4;
  int packed=c->array_compact_records==2||c->array_compact_records==4;
  int stride=lean?(packed?4:3):(packed?5:4);
  int32_t *records=malloc((size_t)(n?n:1)*12);
  int64_t *sources=malloc((size_t)(n?n:1)*8),*work=malloc((size_t)(n?n:1)*8);
  int64_t *edges=packed?malloc((size_t)(n?n:1)*8):NULL;
  int rc=SGPL_DAG_ERR_NOMEM;
  if(!records||!sources||!work||(packed&&!edges))goto fail;
  if(lean) {
    /* Cold consumer boundary only. A certified context does not escape the
     * compiler's immutable-CSR stage. Recover source occurrence from original
     * work ordinals, retaining duplicate sources and zero-degree rows.
     * Regular CPU execution allocates no per-source recovery metadata. */
    AutoGraphMeta *m=find_meta(c->graph);
    if(!m||!m->csr_row_ptr||c->array_source_count<0||
       (uint64_t)c->array_source_count>=SIZE_MAX/8){rc=SGPL_DAG_ERR_INVALID;goto fail;}
    prefix=malloc(((size_t)c->array_source_count+1)*8);if(!prefix)goto fail;
    prefix[0]=0;
    for(int64_t k=0;k<c->array_source_count;++k) {
      int32_t u=c->array_sources[k];
      if(u<0||u>=m->csr_n){rc=SGPL_DAG_ERR_INVALID;goto fail;}
      int64_t first=m->csr_row_ptr[u],last=m->csr_row_ptr[u+1];
      if(first<0||last<first||last>m->csr_m||last-first>INT32_MAX-prefix[k]){rc=SGPL_DAG_ERR_INVALID;goto fail;}
      prefix[k+1]=prefix[k]+last-first;
    }
    if(prefix[c->array_source_count]!=c->array_work_count){rc=SGPL_DAG_ERR_INVALID;goto fail;}
  }
  for(int64_t i=0;i<n;++i) {
    records[3*i]=c->gpu_records[stride*i];records[3*i+1]=c->gpu_records[stride*i+1];records[3*i+2]=0;
    work[i]=c->gpu_records[stride*i+(lean?2:3)];
    if(lean) {
      if(work[i]<0||work[i]>=c->array_work_count){rc=SGPL_DAG_ERR_INVALID;goto fail;}
      int64_t lo=0,hi=c->array_source_count;
      while(lo<hi){int64_t mid=lo+(hi-lo)/2;if(prefix[mid+1]<=work[i])lo=mid+1;else hi=mid;}
      if(lo>=c->array_source_count||c->array_sources[lo]!=records[3*i]){rc=SGPL_DAG_ERR_INVALID;goto fail;}
      sources[i]=lo;
    } else sources[i]=c->gpu_records[stride*i+2];
    if(packed)edges[i]=c->gpu_records[stride*i+(lean?3:4)];
  }
  free(prefix);
  sgpl_execution_scratch_detach(c);
  free(c->gpu_records);c->gpu_records=records;
  c->gpu_record_sources=sources;c->gpu_record_work=work;if(packed)c->array_record_edges=edges;c->array_compact_records=0;
  return 0;
fail:
  free(prefix);free(records);free(sources);free(work);free(edges);return rc;
}
#include "array_preparation_runtime.inc"
static SGPL_PREPARATION_FN int32_t sgpl_array_records(sgpl_exec_ctx *c,AutoGraphMeta *m,int begins,int ends) {
  if(m->current_layout==LAYOUT_CSR&&m->csr_row_ptr&&m->csr_col_idx)
    return sgpl_array_records_csr(c,m,begins,ends);
  int64_t work=0,records=0;
  int source=c->traversal_kind==SGPL_TRAVERSE_OWNER_U;
  int64_t *counts=calloc((size_t)m->partition_count,8),*positions=calloc((size_t)m->partition_count,8);
  if((uint64_t)m->csr_n>SIZE_MAX/sizeof(int32_t)) {free(counts);free(positions);return SGPL_DAG_ERR_NOMEM;}
  int32_t *owned_map=NULL;
  const int32_t *owners=NULL;
  int rc=SGPL_DAG_ERR_NOMEM;
  if(!counts||!positions)goto done;
  if(SGPL_EXEC_CACHE_OWNERS) {
    rc=sgpl_array_owner_map(m,&owners);if(rc<0)goto done;
    rc=SGPL_DAG_ERR_NOMEM;
  } else {
    owned_map=malloc((size_t)(m->csr_n?m->csr_n:1)*sizeof(int32_t));
    if(!owned_map)goto done;
    for(int p=0;p<m->partition_count;++p)
      for(int64_t u=m->partition_start[p];u<m->partition_start[p+1];++u)owned_map[u]=p;
    owners=owned_map;
  }
  const int csr=m->current_layout==LAYOUT_CSR&&m->csr_row_ptr&&m->csr_col_idx;
  /* The language weight query reads the loader's CSR, which can differ from
   * a converted layout. Never use an index from another representation. */
  const int native_weighted=csr&&m->csr_weights&&
      m->csr_row_ptr==*(int64_t **)((char *)c->graph+16)&&
      m->csr_col_idx==*(int32_t **)((char *)c->graph+24);
  const int64_t *weight_indices=NULL;
  if(native_weighted) {
    rc=sgpl_array_weight_indices(m,&weight_indices);if(rc<0)goto done;
    rc=SGPL_DAG_ERR_NOMEM;
  }
  /* Two topology passes form stable batches; neither pass inspects conflicts. */
  for(int pass=0;pass<2;++pass) {
    int64_t ordinal=0;
    for(int64_t i=0;i<c->array_source_count;++i) {
      int32_t u=c->array_sources[i],v;AutoNeighborIter it;
      if(u<0||u>=m->csr_n) {rc=SGPL_DAG_ERR_INVALID;goto done;}
      for(int kind=1;kind<=2;++kind) {
        if(kind==2) {
          int64_t edge=csr?m->csr_row_ptr[u]:0,last=csr?m->csr_row_ptr[u+1]:0;
          if(!csr)autograph_neighbor_iter_init(c->graph,u,&it);
          while(csr?edge<last:autograph_neighbor_iter_next(&it,&v)) {
            if(csr)v=m->csr_col_idx[edge++];
            if(v<0||v>=m->csr_n||ordinal>=INT32_MAX/c->array_append_sites) {rc=SGPL_DAG_ERR_INVALID;goto done;}
            int32_t p=owners[source?u:v];
            if(!pass)++counts[p];
            else {int64_t at=positions[p]++;c->gpu_records[3*at]=u;c->gpu_records[3*at+1]=v;
              c->gpu_records[3*at+2]=0;c->gpu_record_sources[at]=i;c->gpu_record_work[at]=ordinal;
              if(c->array_record_edges)c->array_record_edges[at]=weight_indices[edge-1];}
            ++ordinal;
          }
        }
        if(!source||!(kind==1?begins:ends))continue;
        int32_t p=owners[u];
        if(!pass)++counts[p];
        else {int64_t at=positions[p]++;c->gpu_records[3*at]=u;c->gpu_records[3*at+1]=-1;
          c->gpu_records[3*at+2]=kind;c->gpu_record_sources[at]=i;c->gpu_record_work[at]=-1;
          if(c->array_record_edges)c->array_record_edges[at]=-1;}
      }
    }
    if(!pass) {
      work=ordinal;records=work+(source?(begins+ends)*c->array_source_count:0);
      if((uint64_t)records>SIZE_MAX/12||(uint64_t)work>SIZE_MAX/8/c->array_append_sites)goto done;
      c->array_work_count=work;
      c->array_append_log=sgpl_execution_buffer(c,SGPL_BUF_APPEND,(size_t)(work?work:1)*c->array_append_sites*8,1);
      c->gpu_records=sgpl_execution_buffer(c,SGPL_BUF_RECORDS,(size_t)(records?records:1)*12,0);
      c->gpu_record_sources=sgpl_execution_buffer(c,SGPL_BUF_SOURCES,(size_t)(records?records:1)*8,0);
      c->gpu_record_work=sgpl_execution_buffer(c,SGPL_BUF_WORK,(size_t)(records?records:1)*8,0);
      if(native_weighted)c->array_record_edges=sgpl_execution_buffer(c,SGPL_BUF_EDGES,(size_t)(records?records:1)*8,0);
      c->gpu_record_offsets=sgpl_execution_buffer(c,SGPL_BUF_OFFSETS,((size_t)m->partition_count+1)*8,1);
      if(!c->array_append_log||!c->gpu_records||!c->gpu_record_sources||!c->gpu_record_work||!c->gpu_record_offsets||
          (native_weighted&&!c->array_record_edges))goto done;
      for(int p=0;p<m->partition_count;++p) {
        c->gpu_record_offsets[p+1]=c->gpu_record_offsets[p]+counts[p];positions[p]=c->gpu_record_offsets[p];
      }
    }
  }
  c->gpu_record_count=records;c->array_log_pristine=1;
  c->array_pair_records=!source||(!begins&&!ends);rc=0;
done:
  free(counts);free(positions);free(owned_map);return rc;
}
extern int32_t graph_get_edge_weight(void *,int32_t,int32_t) __attribute__((weak));
int32_t autograph_exec_ctx_edge_weight(sgpl_exec_ctx *c,void *graph,int32_t u,int32_t v) {
  if(c&&c->graph==graph&&(c->array_record_edges||c->array_compact_records==2||c->array_compact_records==4)&&c->array_edge_index>=0&&
      u>=0&&u<*(int64_t *)graph) {
    int64_t *rp=*(int64_t **)((char *)graph+16),index=c->array_edge_index;
    int32_t *ci=*(int32_t **)((char *)graph+24),*weights=*(int32_t **)((char *)graph+32);
    if(index>=rp[u]&&index<rp[u+1]&&ci[index]==v)return weights?weights[index]:1;
  }
  return graph_get_edge_weight?graph_get_edge_weight(graph,u,v):INT32_MAX;
}
#include "stable_compaction_runtime.inc"
typedef struct {
  const int32_t *log;int32_t *out;int64_t slots,prefix[65];uint32_t epoch;int32_t chunks;
} SgplParallelCompact;
static void sgpl_compact_count_chunks(void *opaque,int32_t worker,int32_t width) {
  SgplParallelCompact *j=opaque;
  for(int32_t k=worker;k<j->chunks;k+=width) {
    int64_t lo=j->slots*k/j->chunks,hi=j->slots*(k+1)/j->chunks,n=0;
    for(int64_t i=lo;i<hi;++i)n+=((const uint32_t*)j->log)[2*i]==j->epoch;
    j->prefix[k+1]=n;
  }
}
static void sgpl_compact_write_chunks(void *opaque,int32_t worker,int32_t width) {
  SgplParallelCompact *j=opaque;
  for(int32_t k=worker;k<j->chunks;k+=width) {
    int64_t lo=j->slots*k/j->chunks,hi=j->slots*(k+1)/j->chunks;
    sgpl_append_compact_full(j->log+2*lo,hi-lo,j->epoch,j->out,j->prefix[k]);
  }
}
static int64_t sgpl_append_compact_joined(const int32_t *log,int64_t slots,
    uint32_t epoch,int32_t *out,int64_t at) {
  if(sgpl_is_worker_pool_thread())return sgpl_append_compact_full(log,slots,epoch,out,at);
  int32_t width=sgpl_configured_worker_count(),budget=sgpl_current_thread_budget();
  if(budget>0&&width>budget)width=budget;
  int64_t useful=slots/65536;
  if(width>useful)width=(int32_t)useful;
  if(width<2)return sgpl_append_compact_full(log,slots,epoch,out,at);
  if(width>32)width=32;
  SgplParallelCompact job={.log=log,.out=out,.slots=slots,.epoch=epoch,.chunks=width*2};
  sgpl_run_preparation(sgpl_compact_count_chunks,&job,width);
  job.prefix[0]=at;
  for(int32_t k=0;k<job.chunks;++k)job.prefix[k+1]+=job.prefix[k];
  sgpl_run_preparation(sgpl_compact_write_chunks,&job,width);
  return job.prefix[job.chunks];
}

static int sgpl_journal_compact(sgpl_exec_ctx *,int32_t *);
static int32_t sgpl_array_compact(sgpl_exec_ctx *c) {
  int32_t journal_result=0;if(sgpl_journal_compact(c,&journal_result))return journal_result;
  int64_t slots=c->array_work_count*c->array_append_sites,kept=c->initial_next_size;
  uint32_t epoch=sgpl_array_append_epoch(c);
  /* Normal reservation covers every append slot. Count first only when a
   * caller supplied less space; no user callback is replayed on growth. */
  int full_capacity=slots<=*c->array_output_capacity-kept;
  if(!full_capacity) {
    for(int64_t i=0;i<slots;++i)kept+=((uint32_t *)c->array_append_log)[2*i]==epoch;
    if(kept>INT32_MAX)return SGPL_DAG_ERR_INVALID;
    int32_t rc=sgpl_frontier_resize(c->array_output_slot,c->array_output_capacity,kept,1);
    if(rc<0)return rc;
  } else kept+=slots;
  c->next_frontier=*c->array_output_slot;
  if(sgpl_gpu_host_access&&kept)sgpl_gpu_host_access(c->next_frontier,(uint64_t)kept*4,1);
  int64_t at=c->initial_next_size;
  if(full_capacity)at=sgpl_append_compact_joined(c->array_append_log,slots,epoch,c->next_frontier,at);
  else for(int64_t i=0;i<slots;++i)if(((uint32_t *)c->array_append_log)[2*i]==epoch)c->next_frontier[at++]=c->array_append_log[2*i+1];
  if(c->append_head)*c->append_head=(int32_t)(at-c->initial_next_size);
  return 0;
}

/* Select the execution-helper definitions without editing protected ARS code. */
#define sgpl_exec_partition_body sgpl_exec_partition_body_legacy_context_copy
#define sgpl_exec_partition_body_packed sgpl_exec_partition_body_packed_legacy_context_copy
