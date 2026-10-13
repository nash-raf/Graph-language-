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
static int32_t sgpl_array_records(sgpl_exec_ctx *c,AutoGraphMeta *m,int begins,int ends) {
  int64_t work=0,records=0;
  int source=c->traversal_kind==SGPL_TRAVERSE_OWNER_U;
  for(int64_t i=0;i<c->array_source_count;++i) {
    int32_t u=c->array_sources[i],v;
    if(u<0||u>=m->csr_n)return SGPL_DAG_ERR_INVALID;
    AutoNeighborIter it;autograph_neighbor_iter_init(c->graph,u,&it);
    while(autograph_neighbor_iter_next(&it,&v)) {
      if(work>=INT32_MAX/c->array_append_sites)return SGPL_DAG_ERR_INVALID;
      ++work;
    }
  }
  records=work+(source?(begins+ends)*c->array_source_count:0);
  if((uint64_t)records>SIZE_MAX/12||(uint64_t)work>SIZE_MAX/8/c->array_append_sites)
    return SGPL_DAG_ERR_NOMEM;
  c->array_work_count=work;
  c->array_append_log=calloc((size_t)(work?work:1)*c->array_append_sites,8);
  c->gpu_records=malloc((size_t)(records?records:1)*12);
  c->gpu_record_sources=malloc((size_t)(records?records:1)*8);
  c->gpu_record_work=malloc((size_t)(records?records:1)*8);
  c->gpu_record_offsets=calloc((size_t)m->partition_count+1,8);
  int64_t *counts=calloc((size_t)m->partition_count,8),*positions=calloc((size_t)m->partition_count,8);
  if(!c->array_append_log||!c->gpu_records||!c->gpu_record_sources||!c->gpu_record_work||
      !c->gpu_record_offsets||!counts||!positions) {free(counts);free(positions);return SGPL_DAG_ERR_NOMEM;}
  /* Two topology passes form stable batches; neither pass inspects conflicts. */
  for(int pass=0;pass<2;++pass) {
    int64_t ordinal=0;
    for(int64_t i=0;i<c->array_source_count;++i) {
      int32_t u=c->array_sources[i],v;AutoNeighborIter it;
      for(int kind=1;kind<=2;++kind) {
        if(kind==2) {
          autograph_neighbor_iter_init(c->graph,u,&it);
          while(autograph_neighbor_iter_next(&it,&v)) {
            int32_t key=source?u:v,p=0;
            while(p+1<m->partition_count&&key>=m->partition_start[p+1])++p;
            if(!pass)++counts[p];
            else {int64_t at=positions[p]++;c->gpu_records[3*at]=u;c->gpu_records[3*at+1]=v;
              c->gpu_records[3*at+2]=0;c->gpu_record_sources[at]=i;c->gpu_record_work[at]=ordinal;}
            ++ordinal;
          }
        }
        if(!source||!(kind==1?begins:ends))continue;
        int32_t p=0;while(p+1<m->partition_count&&u>=m->partition_start[p+1])++p;
        if(!pass)++counts[p];
        else {int64_t at=positions[p]++;c->gpu_records[3*at]=u;c->gpu_records[3*at+1]=-1;
          c->gpu_records[3*at+2]=kind;c->gpu_record_sources[at]=i;c->gpu_record_work[at]=-1;}
      }
    }
    if(!pass)for(int p=0;p<m->partition_count;++p) {
      c->gpu_record_offsets[p+1]=c->gpu_record_offsets[p]+counts[p];positions[p]=c->gpu_record_offsets[p];
    }
  }
  free(counts);free(positions);c->gpu_record_count=records;
  return 0;
}
static int32_t sgpl_array_compact(sgpl_exec_ctx *c) {
  int64_t slots=c->array_work_count*c->array_append_sites,kept=c->initial_next_size;
  for(int64_t i=0;i<slots;++i)kept+=c->array_append_log[2*i]!=0;
  if(kept>INT32_MAX)return SGPL_DAG_ERR_INVALID;
  int32_t rc=sgpl_frontier_resize(c->array_output_slot,c->array_output_capacity,kept,1);
  if(rc<0)return rc;
  c->next_frontier=*c->array_output_slot;
  if(sgpl_gpu_host_access&&kept)sgpl_gpu_host_access(c->next_frontier,(uint64_t)kept*4,1);
  int64_t at=c->initial_next_size;
  for(int64_t i=0;i<slots;++i)if(c->array_append_log[2*i])c->next_frontier[at++]=c->array_append_log[2*i+1];
  if(c->append_head)*c->append_head=(int32_t)(kept-c->initial_next_size);
  return 0;
}
