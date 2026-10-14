/* Preparation writes infrastructure only. Chunks cover consecutive original
 * source ordinals; prefix offsets concatenate them in that same order. */
#if defined(__GNUC__) && defined(__ELF__)
/* Keep preparation out of .text, including its driver below. Expanding these
 * loops must not relocate the executor, DAG or worker-pool instruction streams.
 * The linker retains this as an executable section; no schedule or data layout
 * changes. Other toolchains keep the ordinary function placement. */
#define SGPL_PREPARATION_FN __attribute__((noinline,section(".sgpl_preparation")))
#else
#define SGPL_PREPARATION_FN
#endif
typedef struct {
  sgpl_exec_ctx *ctx;
  AutoGraphMeta *meta;
  const int32_t *owners;
  const int64_t *weights;
  int64_t *positions, *work;
  int64_t pair_limit;
  int32_t chunks,parts,source,begins,ends;
  _Atomic int32_t error;
} SgplArrayPreparation;
static SGPL_PREPARATION_FN void sgpl_array_count_chunks(void *opaque,int32_t worker,int32_t width) {
  SgplArrayPreparation *j=opaque;sgpl_exec_ctx *c=j->ctx;AutoGraphMeta *m=j->meta;
  const int source=j->source,hooks=j->begins+j->ends;
  const int64_t limit=j->pair_limit;
  const int32_t chunks=j->chunks,parts=j->parts;
  const int64_t count=c->array_source_count,n=m->csr_n,arcs=m->csr_m;
  const int64_t *rows=m->csr_row_ptr;
  const int32_t *input=c->array_sources,*columns=m->csr_col_idx,*owners=j->owners;
  for(int32_t chunk=worker;chunk<chunks;chunk+=width) {
    int64_t lo=count*chunk/chunks,hi=count*(chunk+1)/chunks;
    int64_t *positions=j->positions+(size_t)chunk*parts;
    int64_t ordinal=0;
    for(int64_t i=lo;i<hi;++i) {
      if(atomic_load_explicit(&j->error,memory_order_relaxed))return;
      int32_t u=input[i];
      if(u<0||u>=n)goto invalid;
      int64_t first=rows[u],last=rows[u+1];
      if(first<0||last<first||last>arcs)goto invalid;
      int64_t degree=last-first;
      if(degree>limit-ordinal)goto invalid;
      int32_t source_part=owners[u];
      if(source)positions[source_part]+=degree+hooks;
      for(int64_t edge=first;edge<last;++edge) {
        int32_t v=columns[edge];
        if(v<0||v>=n)goto invalid;
        if(!source)++positions[owners[v]];
      }
      ordinal+=degree;
    }
    j->work[chunk+1]=ordinal;
  }
  return;
invalid:
  atomic_store_explicit(&j->error,SGPL_DAG_ERR_INVALID,memory_order_relaxed);
}
/* Counting validated every source, row and column, and the host prefix checked
 * the total ordinal limit. Topology and the input frontier are immutable until
 * preparation joins; no user callback runs between counting and filling.
 * Filling therefore has no per-edge validation or phase branch. */
static SGPL_PREPARATION_FN void sgpl_array_fill_chunks(void *opaque,int32_t worker,int32_t width) {
  SgplArrayPreparation *j=opaque;sgpl_exec_ctx *c=j->ctx;AutoGraphMeta *m=j->meta;
  const int source=j->source,begins=source&&j->begins,ends=source&&j->ends;
  const int32_t chunks=j->chunks,parts=j->parts;
  const int64_t count=c->array_source_count;
  const int64_t *rows=m->csr_row_ptr,*weights=j->weights;
  const int32_t *input=c->array_sources,*columns=m->csr_col_idx,*owners=j->owners;
  int32_t *records=c->gpu_records;
  int64_t *sources=c->gpu_record_sources,*work=c->gpu_record_work,*edges=c->array_record_edges;
  for(int32_t chunk=worker;chunk<chunks;chunk+=width) {
    int64_t lo=count*chunk/chunks,hi=count*(chunk+1)/chunks;
    int64_t *positions=j->positions+(size_t)chunk*parts;
    int64_t ordinal=j->work[chunk];
    for(int64_t i=lo;i<hi;++i) {
      int32_t u=input[i],source_part=owners[u];
      int64_t first=rows[u],last=rows[u+1];
      if(begins) {
        int64_t at=positions[source_part]++;
        records[3*at]=u;records[3*at+1]=-1;records[3*at+2]=1;
        sources[at]=i;work[at]=-1;if(edges)edges[at]=-1;
      }
      for(int64_t edge=first;edge<last;++edge) {
        int32_t v=columns[edge],p=source?source_part:owners[v];
        int64_t at=positions[p]++;
        records[3*at]=u;records[3*at+1]=v;records[3*at+2]=0;
        sources[at]=i;work[at]=ordinal++;if(edges)edges[at]=weights[edge];
      }
      if(ends) {
        int64_t at=positions[source_part]++;
        records[3*at]=u;records[3*at+1]=-1;records[3*at+2]=2;
        sources[at]=i;work[at]=-1;if(edges)edges[at]=-1;
      }
    }
  }
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
      .pair_limit=INT32_MAX/c->array_append_sites,
      .source=c->traversal_kind==SGPL_TRAVERSE_OWNER_U,.begins=begins,.ends=ends};
  sgpl_run_preparation(sgpl_array_count_chunks,&job,width);
  rc=atomic_load_explicit(&job.error,memory_order_relaxed);if(rc<0)goto done;
  for(int32_t k=0;k<chunks;++k) {
    if(work[k+1]>job.pair_limit-work[k]) {rc=SGPL_DAG_ERR_INVALID;goto done;}
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
  sgpl_run_preparation(sgpl_array_fill_chunks,&job,width);
  c->gpu_record_count=records;c->array_log_pristine=1;
  c->array_pair_records=!job.source||(!begins&&!ends);rc=0;
done:
  free(positions);free(work);free(owned_map);return rc;
}
