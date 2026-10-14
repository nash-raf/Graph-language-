/* Preparation writes infrastructure only. Chunks cover consecutive original
 * source ordinals; prefix offsets concatenate them in that same order. */
typedef struct {
  sgpl_exec_ctx *ctx;
  AutoGraphMeta *meta;
  const int32_t *owners;
  const int64_t *weights;
  int64_t *positions, *work;
  int32_t chunks,parts,source,begins,ends,pass;
  _Atomic int32_t error;
} SgplArrayPreparation;
static void sgpl_array_prepare_chunks(void *opaque,int32_t worker,int32_t width) {
  SgplArrayPreparation *j=opaque;sgpl_exec_ctx *c=j->ctx;AutoGraphMeta *m=j->meta;
  for(int32_t chunk=worker;chunk<j->chunks;chunk+=width) {
    int64_t lo=c->array_source_count*chunk/j->chunks,hi=c->array_source_count*(chunk+1)/j->chunks;
    int64_t *positions=j->positions+(size_t)chunk*j->parts;
    int64_t ordinal=j->pass?j->work[chunk]:0;
    for(int64_t i=lo;i<hi;++i) {
      if(atomic_load_explicit(&j->error,memory_order_relaxed))return;
      int32_t u=c->array_sources[i];
      if(u<0||u>=m->csr_n)goto invalid;
      int64_t first=m->csr_row_ptr[u],last=m->csr_row_ptr[u+1];
      if(first<0||last<first||last>m->csr_m)goto invalid;
      int32_t source_part=j->owners[u];
      if(j->source&&j->begins) {
        int64_t at=positions[source_part]++;
        if(j->pass) {
          c->gpu_records[3*at]=u;c->gpu_records[3*at+1]=-1;c->gpu_records[3*at+2]=1;
          c->gpu_record_sources[at]=i;c->gpu_record_work[at]=-1;
          if(c->array_record_edges)c->array_record_edges[at]=-1;
        }
      }
      for(int64_t edge=first;edge<last;++edge) {
        int32_t v=m->csr_col_idx[edge];
        if(v<0||v>=m->csr_n||ordinal>=INT32_MAX/c->array_append_sites)goto invalid;
        int32_t p=j->source?source_part:j->owners[v];
        int64_t at=positions[p]++;
        if(j->pass) {
          c->gpu_records[3*at]=u;c->gpu_records[3*at+1]=v;c->gpu_records[3*at+2]=0;
          c->gpu_record_sources[at]=i;c->gpu_record_work[at]=ordinal;
          if(c->array_record_edges)c->array_record_edges[at]=j->weights[edge];
        }
        ++ordinal;
      }
      if(j->source&&j->ends) {
        int64_t at=positions[source_part]++;
        if(j->pass) {
          c->gpu_records[3*at]=u;c->gpu_records[3*at+1]=-1;c->gpu_records[3*at+2]=2;
          c->gpu_record_sources[at]=i;c->gpu_record_work[at]=-1;
          if(c->array_record_edges)c->array_record_edges[at]=-1;
        }
      }
    }
    if(!j->pass)j->work[chunk+1]=ordinal;
  }
  return;
invalid:
  atomic_store_explicit(&j->error,SGPL_DAG_ERR_INVALID,memory_order_relaxed);
}
static int32_t sgpl_array_records_csr(sgpl_exec_ctx *c,AutoGraphMeta *m,int begins,int ends) {
  int32_t width=sgpl_configured_worker_count();
  if(c->array_source_count<128||getenv("SGPL_ARRAY_PREP_SERIAL"))width=1;
  if(width<1)width=1;
  int32_t chunks=(int32_t)((int64_t)width*4>64?64:(int64_t)width*4);
  if(chunks>c->array_source_count)chunks=(int32_t)c->array_source_count;
  if(chunks<1)chunks=1;
  int32_t parts=m->partition_count,rc=SGPL_DAG_ERR_NOMEM;
  if(parts<0||(uint64_t)chunks*(uint32_t)parts>SIZE_MAX/8)return SGPL_DAG_ERR_INVALID;
  int64_t *positions=calloc((size_t)chunks*(parts?parts:1),8),*work=calloc((size_t)chunks+1,8);
  int32_t *owned_map=NULL;
  const int32_t *owners=NULL;
  if(!positions||!work)goto done;
  if(SGPL_EXEC_CACHE_OWNERS) {
    rc=sgpl_array_owner_map(m,&owners);if(rc<0)goto done;
  } else {
    if((uint64_t)m->csr_n>SIZE_MAX/sizeof(int32_t))goto done;
    owned_map=malloc((size_t)(m->csr_n?m->csr_n:1)*sizeof(int32_t));
    if(!owned_map)goto done;
    for(int32_t p=0;p<parts;++p)
      for(int64_t u=m->partition_start[p];u<m->partition_start[p+1];++u)owned_map[u]=p;
    owners=owned_map;
  }
  const int64_t *weight_indices=NULL;
  int native_weighted=m->csr_weights&&m->csr_row_ptr==*(int64_t **)((char *)c->graph+16)&&
      m->csr_col_idx==*(int32_t **)((char *)c->graph+24);
  if(native_weighted) {rc=sgpl_array_weight_indices(m,&weight_indices);if(rc<0)goto done;}
  SgplArrayPreparation job={.ctx=c,.meta=m,.owners=owners,.weights=weight_indices,
      .positions=positions,.work=work,.chunks=chunks,.parts=parts,
      .source=c->traversal_kind==SGPL_TRAVERSE_OWNER_U,.begins=begins,.ends=ends};
  sgpl_run_preparation(sgpl_array_prepare_chunks,&job,width);
  rc=atomic_load_explicit(&job.error,memory_order_relaxed);if(rc<0)goto done;
  for(int32_t k=0;k<chunks;++k) {
    if(work[k+1]>INT32_MAX/c->array_append_sites-work[k]) {rc=SGPL_DAG_ERR_INVALID;goto done;}
    work[k+1]+=work[k];
  }
  int64_t pair_count=work[chunks],records=pair_count+(job.source?(begins+ends)*c->array_source_count:0);
  if((uint64_t)records>SIZE_MAX/12||(uint64_t)pair_count>SIZE_MAX/8/c->array_append_sites) {rc=SGPL_DAG_ERR_NOMEM;goto done;}
  c->array_work_count=pair_count;
  c->array_append_log=sgpl_execution_buffer(c,SGPL_BUF_APPEND,(size_t)(pair_count?pair_count:1)*c->array_append_sites*8,1);
  c->gpu_records=sgpl_execution_buffer(c,SGPL_BUF_RECORDS,(size_t)(records?records:1)*12,0);
  c->gpu_record_sources=sgpl_execution_buffer(c,SGPL_BUF_SOURCES,(size_t)(records?records:1)*8,0);
  c->gpu_record_work=sgpl_execution_buffer(c,SGPL_BUF_WORK,(size_t)(records?records:1)*8,0);
  if(native_weighted)c->array_record_edges=sgpl_execution_buffer(c,SGPL_BUF_EDGES,(size_t)(records?records:1)*8,0);
  c->gpu_record_offsets=sgpl_execution_buffer(c,SGPL_BUF_OFFSETS,((size_t)parts+1)*8,1);
  if(!c->array_append_log||!c->gpu_records||!c->gpu_record_sources||!c->gpu_record_work||!c->gpu_record_offsets||
      (native_weighted&&!c->array_record_edges)) {rc=SGPL_DAG_ERR_NOMEM;goto done;}
  int64_t at=0;
  for(int32_t p=0;p<parts;++p) {
    c->gpu_record_offsets[p]=at;
    for(int32_t k=0;k<chunks;++k) {
      int64_t count=positions[(size_t)k*parts+p];positions[(size_t)k*parts+p]=at;at+=count;
    }
  }
  c->gpu_record_offsets[parts]=at;
  if(at!=records) {rc=SGPL_DAG_ERR_INVALID;goto done;}
  job.pass=1;sgpl_run_preparation(sgpl_array_prepare_chunks,&job,width);
  rc=atomic_load_explicit(&job.error,memory_order_relaxed);
  if(!rc) {
    c->gpu_record_count=records;c->array_log_pristine=1;
    c->array_pair_records=!job.source||(!begins&&!ends);
  }
done:
  free(positions);free(work);free(owned_map);return rc;
}
