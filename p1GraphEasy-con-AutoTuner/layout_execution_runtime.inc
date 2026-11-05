/* Execute the compiler's accepted owner groups through the selected layout.
 * No conflict inference or change of proof witness occurs here. Destination
 * tasks filter native source-order traversal; source tasks own whole rows.
 * This first general implementation intentionally favors an auditable order
 * over a topology-dependent reordering or an unproved source transformation.
 */
typedef struct {
  AutoGraphMeta *meta;
  int64_t n, source_count, *prefix;
  int32_t layout, epoch, partitions, *sources;
  uint64_t generation;
} SgplLayoutExecution;

static void sgpl_layout_execution_free(sgpl_exec_ctx *c) {
  SgplLayoutExecution *s=c?c->layout_execution:NULL;
  if(!s)return;
  free(s->prefix);free(s->sources);free(s);c->layout_execution=NULL;
}

static int sgpl_layout_execution_matches(const sgpl_exec_ctx *c) {
  const SgplLayoutExecution *s=c->layout_execution;
  AutoGraphMeta *m=find_meta(c->graph);
  return s&&m==s->meta&&m->csr_n==s->n&&m->current_layout==s->layout&&
      m->layout_epoch==s->epoch&&m->partition_count==s->partitions&&
      atomic_load_explicit(&g_cc_topology_generation,memory_order_acquire)==s->generation;
}

static int32_t sgpl_layout_prepare_work(sgpl_exec_ctx *c) {
  SgplLayoutExecution *s=c->layout_execution;AutoGraphMeta *m=find_meta(c->graph);
  if(!s||!m||m->csr_n<0||m->csr_n>INT32_MAX||m->partition_count<0)
    return SGPL_DAG_ERR_INVALID;
  s->meta=m;s->n=m->csr_n;s->layout=m->current_layout;s->epoch=m->layout_epoch;
  s->partitions=m->partition_count;
  s->generation=atomic_load_explicit(&g_cc_topology_generation,memory_order_acquire);
  if(!c->array_append_sites)return 0;
  /* Retain existing compiler-certified occurrence simplifications. Their
   * worklist builder already reads native topology and needs no CleanCut. */
  if(sgpl_relax_state(c)&&m->current_layout==LAYOUT_CSR)
    return sgpl_array_records(c,m,sgpl_exec_has_cap(c,SGPL_OP_SOURCE_BEGIN),
                                sgpl_exec_has_cap(c,SGPL_OP_SOURCE_END));
  int64_t count=c->array_source_count;
  if(count<0||(count&&!c->array_sources)||(uint64_t)count>=SIZE_MAX/8)
    return SGPL_DAG_ERR_INVALID;
  s->source_count=count;
  s->prefix=calloc((size_t)count+1,sizeof(*s->prefix));
  s->sources=malloc((size_t)(count?count:1)*sizeof(*s->sources));
  if(!s->prefix||!s->sources)return SGPL_DAG_ERR_NOMEM;
  if(count)memcpy(s->sources,c->array_sources,(size_t)count*sizeof(*s->sources));
  for(int64_t i=0;i<count;++i) {
    int32_t u=s->sources[i],v;int64_t degree=0;
    if(u<0||u>=m->csr_n)return SGPL_DAG_ERR_INVALID;
    if(m->current_layout==LAYOUT_CSR) {
      if(!m->csr_row_ptr)return SGPL_DAG_ERR_INVALID;
      int64_t a=m->csr_row_ptr[u],b=m->csr_row_ptr[u+1];
      if(a<0||b<a||b>m->csr_m)return SGPL_DAG_ERR_INVALID;
      degree=b-a;
    }else{
      AutoNeighborIter it;autograph_neighbor_iter_init(c->graph,u,&it);
      while(autograph_neighbor_iter_next(&it,&v)) {
        if(v<0||v>=m->csr_n||degree==INT64_MAX)return SGPL_DAG_ERR_INVALID;
        ++degree;
      }
    }
    if(degree>INT32_MAX/c->array_append_sites-s->prefix[i])return SGPL_DAG_ERR_INVALID;
    s->prefix[i+1]=s->prefix[i]+degree;
  }
  c->array_sources=s->sources;c->array_work_count=s->prefix[count];
  size_t slots=(size_t)c->array_work_count*c->array_append_sites;
  if(slots>SIZE_MAX/8)return SGPL_DAG_ERR_NOMEM;
  c->array_append_log=sgpl_execution_buffer(c,SGPL_BUF_APPEND,(slots?slots:1)*8,1);
  if(!c->array_append_log)return SGPL_DAG_ERR_NOMEM;
  c->array_log_pristine=1;
  return 0;
}

/* The compiler has already checked legacy composition's traversal/schedule.
 * This binds a representation only, without inventing a spatial proof. */
int32_t autograph_exec_ctx_bind_layout(sgpl_exec_ctx *c) {
  if(!c||c->layout_execution||c->array_append_sites)return SGPL_DAG_ERR_INVALID;
  c->layout_execution=calloc(1,sizeof(SgplLayoutExecution));
  if(!c->layout_execution)return SGPL_DAG_ERR_NOMEM;
  c->execution_error=&c->execution_status;
  int32_t rc=sgpl_layout_prepare_work(c);
  if(rc<0)sgpl_layout_execution_free(c);
  return rc;
}

static void sgpl_layout_pair(sgpl_exec_ctx *shared,sgpl_exec_ctx *local,
                            int32_t u,int32_t v) {
  if(local->spatial_group==SGPL_SPATIAL_PAIR_OCCURRENCE)
    __atomic_fetch_add(&shared->spatial_occupied_groups,1,__ATOMIC_RELAXED);
  else if(local->spatial_groups_seen)
    local->spatial_groups_seen[local->spatial_group==SGPL_SPATIAL_SOURCE?u:v]=1;
  sgpl_exec_pair_ops(local,u,v);
}

static void sgpl_layout_partition(int64_t index,sgpl_exec_ctx *c) {
  if(sgpl_exec_failed(c))return;
  if(!sgpl_layout_execution_matches(c)) {
    __atomic_store_n(c->execution_error,SGPL_DAG_ERR_INVALID,__ATOMIC_RELEASE);return;
  }
  SgplLayoutExecution *s=c->layout_execution;AutoGraphMeta *m=s->meta;
  int32_t p=(int32_t)index;
  if(p<0||p>=m->partition_count) {
    __atomic_store_n(c->execution_error,SGPL_DAG_ERR_INVALID,__ATOMIC_RELEASE);return;
  }
  sgpl_exec_ctx local;sgpl_exec_context_copy(&local,c);local.spatial_partition=p;
  local.partition_state=c->partition_base&&c->partition_stride>0?
      (char *)c->partition_base+(int64_t)p*c->partition_stride:c->partition_base;
  if(c->spatial_lane_seen)local.dest_seen=c->spatial_lane_seen+(int64_t)p*s->n;
  int source=c->traversal_kind==SGPL_TRAVERSE_OWNER_U;
  int64_t lo=m->partition_start[p],hi=m->partition_start[p+1];
  /* Full-domain destination batches historically use floor(v*P/n), whereas
   * source ranges and ordered-array batches use floor(p*n/P) boundaries.
   * Retain the original assignment for private partial/fold ordering. */
  if(!source&&!c->array_append_sites) {
    lo=((int64_t)p*s->n+m->partition_count-1)/m->partition_count;
    hi=((int64_t)(p+1)*s->n+m->partition_count-1)/m->partition_count;
  }
  int64_t first=c->array_append_sites?0:(source?lo:0);
  int64_t last=c->array_append_sites?s->source_count:(source?hi:s->n);
  sgpl_exec_partition_begin_ops(&local,p);
  for(int64_t i=first;i<last&&!sgpl_exec_failed(&local);++i) {
    int32_t u=c->array_append_sites?s->sources[i]:(int32_t)i;
    if(source&&(u<lo||u>=hi))continue;
    if(local.membership&&!local.membership[u])continue;
    local.array_source_ordinal=i;
    if(source) {
      local.source_state=NULL;
      if(c->spatial_group==SGPL_SPATIAL_SOURCE&&c->spatial_groups_seen&&
         sgpl_exec_has_cap(c,SGPL_OP_SOURCE_BEGIN|SGPL_OP_SOURCE_END))
        c->spatial_groups_seen[u]=1;
      sgpl_exec_source_begin_ops(&local,u);
    }
    int gate_index=c->array_append_sites?i:u;
    if(!sgpl_exec_failed(&local)&&(!local.spatial_source_gate||local.spatial_source_gate[gate_index])) {
      int64_t ordinal=c->array_append_sites?s->prefix[i]:0;
      AutoNeighborIter it;int32_t v;
      int csr=m->current_layout==LAYOUT_CSR;
      int64_t edge=csr?m->csr_row_ptr[u]:0,end=csr?m->csr_row_ptr[u+1]:0;
      if(!csr)autograph_neighbor_iter_init(c->graph,u,&it);
      while(!sgpl_exec_failed(&local)&&(csr?edge<end:autograph_neighbor_iter_next(&it,&v))) {
        if(csr)v=m->csr_col_idx[edge++];
        local.array_work_ordinal=ordinal++;local.array_append_seq=0;
        local.array_edge_index=-1; /* Preserve the original first-match weight query. */
        if(v<0||v>=s->n) {
          __atomic_store_n(c->execution_error,SGPL_DAG_ERR_INVALID,__ATOMIC_RELEASE);return;
        }
        if(!source&&(v<lo||v>=hi))continue;
        sgpl_layout_pair(c,&local,u,v);
      }
    }
    if(source&&!sgpl_exec_failed(&local))sgpl_exec_source_end_ops(&local,u);
  }
  if(!sgpl_exec_failed(&local))sgpl_exec_partition_end_ops(&local,p);
}
