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
/* Separate independently written chunk cursors, without changing logical
 * partition counts or record order. Override for targets with larger lines. */
#ifndef SGPL_PREP_CACHE_LINE_BYTES
#define SGPL_PREP_CACHE_LINE_BYTES 64
#endif
_Static_assert(SGPL_PREP_CACHE_LINE_BYTES>=sizeof(int64_t)&&
    !(SGPL_PREP_CACHE_LINE_BYTES&(SGPL_PREP_CACHE_LINE_BYTES-1)),
    "preparation cache line must be a power of two containing whole counters");
typedef struct {
  sgpl_exec_ctx *ctx;
  AutoGraphMeta *meta;
  const int32_t *owners;
  const int64_t *weights;
  int64_t *positions, *work;
  size_t position_stride;
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
    int64_t *positions=j->positions+(size_t)chunk*j->position_stride;
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
      for(int64_t edge=first;!source&&edge<last;++edge) {
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
/* Counting validated every source and row, and the host prefix checked
 * the total ordinal limit. Topology and the input frontier are immutable until
 * preparation joins; no user callback runs between counting and filling.
 * Destination ownership validates columns while counting. Source ownership
 * obtains counts from row degrees and validates columns once while filling. */
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
    int64_t *positions=j->positions+(size_t)chunk*j->position_stride;
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
        if(source&&(v<0||v>=m->csr_n)) {
          atomic_store_explicit(&j->error,SGPL_DAG_ERR_INVALID,memory_order_relaxed);
          return;
        }
        int64_t at=positions[p]++;
        if(c->array_compact_records) {
          records[4*at]=u;records[4*at+1]=v;
          records[4*at+2]=(int32_t)i;records[4*at+3]=(int32_t)ordinal++;
        } else {
          records[3*at]=u;records[3*at+1]=v;records[3*at+2]=0;
          sources[at]=i;work[at]=ordinal++;
        }
        if(edges)edges[at]=weights[edge];
      }
      if(ends) {
        int64_t at=positions[source_part]++;
        records[3*at]=u;records[3*at+1]=-1;records[3*at+2]=2;
        sources[at]=i;work[at]=-1;if(edges)edges[at]=-1;
      }
    }
  }
}

/* Format versioning is outside the loop; the original filler stays intact. */
static SGPL_PREPARATION_FN void sgpl_array_fill_chunks_packed(void *opaque,int32_t worker,int32_t width) {
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
    int64_t *positions=j->positions+(size_t)chunk*j->position_stride;
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
        if(source&&(v<0||v>=m->csr_n)) {
          atomic_store_explicit(&j->error,SGPL_DAG_ERR_INVALID,memory_order_relaxed);
          return;
        }
        int64_t at=positions[p]++;
        if(1) {
          records[5*at]=u;records[5*at+1]=v;
          records[5*at+2]=(int32_t)i;records[5*at+3]=(int32_t)ordinal++;
          records[5*at+4]=(int32_t)weights[edge];
        } else {
          records[3*at]=u;records[3*at+1]=v;records[3*at+2]=0;
          sources[at]=i;work[at]=ordinal++;
        }
        if(edges)edges[at]=weights[edge];
      }
      if(ends) {
        int64_t at=positions[source_part]++;
        records[3*at]=u;records[3*at+1]=-1;records[3*at+2]=2;
        sources[at]=i;work[at]=-1;if(edges)edges[at]=-1;
      }
    }
  }
}
/* Dead source-ordinal layouts: keep pair/work order and original count pass. */
static SGPL_PREPARATION_FN void sgpl_array_fill_chunks_lean(void *opaque,int32_t worker,int32_t width) {
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
    int64_t *positions=j->positions+(size_t)chunk*j->position_stride;
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
        if(source&&(v<0||v>=m->csr_n)) {
          atomic_store_explicit(&j->error,SGPL_DAG_ERR_INVALID,memory_order_relaxed);
          return;
        }
        int64_t at=positions[p]++;
        if(1) {
          records[3*at]=u;records[3*at+1]=v;
          records[3*at+2]=(int32_t)ordinal++;
          
        } else {
          records[3*at]=u;records[3*at+1]=v;records[3*at+2]=0;
          sources[at]=i;work[at]=ordinal++;
        }
        if(edges)edges[at]=weights[edge];
      }
      if(ends) {
        int64_t at=positions[source_part]++;
        records[3*at]=u;records[3*at+1]=-1;records[3*at+2]=2;
        sources[at]=i;work[at]=-1;if(edges)edges[at]=-1;
      }
    }
  }
}
static SGPL_PREPARATION_FN void sgpl_array_fill_chunks_lean_weighted(void *opaque,int32_t worker,int32_t width) {
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
    int64_t *positions=j->positions+(size_t)chunk*j->position_stride;
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
        if(source&&(v<0||v>=m->csr_n)) {
          atomic_store_explicit(&j->error,SGPL_DAG_ERR_INVALID,memory_order_relaxed);
          return;
        }
        int64_t at=positions[p]++;
        if(1) {
          records[4*at]=u;records[4*at+1]=v;
          records[4*at+2]=(int32_t)ordinal++;
          records[4*at+3]=(int32_t)weights[edge];
        } else {
          records[3*at]=u;records[3*at+1]=v;records[3*at+2]=0;
          sources[at]=i;work[at]=ordinal++;
        }
        if(edges)edges[at]=weights[edge];
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
  /* Preparation is infrastructure, not the graph callback schedule. Two
   * pool dispatches cannot amortize on a few edges. Keep the configured graph
   * worker budget; require at least 1024 edges per lane for these two passes. */
  if(width>1) {
    int64_t minimum=(int64_t)width*1024,work=0;
    for(int64_t i=0;i<c->array_source_count && work<minimum;++i) {
      int32_t u=c->array_sources[i];
      if(u<0||u>=m->csr_n){width=1;break;}
      int64_t first=m->csr_row_ptr[u],last=m->csr_row_ptr[u+1];
      if(first<0||last<first||last>m->csr_m){width=1;break;}
      int64_t degree=last-first;
      if(degree>=minimum-work){work=minimum;break;}
      work+=degree;
    }
    /* Preparation is pure infrastructure: choose a smaller team when its
     * work cannot fill the configured team at the existing grain. The graph
     * callback schedule and its TDG/thread budget remain unchanged. */
    int32_t useful=(int32_t)(work/1024);
    if(useful<1)useful=1;
    if(useful<width)width=useful;
  }
  if(width<1)width=1;
  int32_t chunks=(int32_t)((int64_t)width*4>64?64:(int64_t)width*4);
  if(chunks>c->array_source_count)chunks=(int32_t)c->array_source_count;
  if(chunks<1)chunks=1;
  int32_t parts=m->partition_count,rc=SGPL_DAG_ERR_NOMEM;
  if(parts<0||(uint64_t)(parts?parts:1)>SIZE_MAX/sizeof(int64_t))return SGPL_DAG_ERR_INVALID;
  size_t row_bytes=(size_t)(parts?parts:1)*sizeof(int64_t);
  int align_positions=!getenv("SGPL_NO_CURSOR_ALIGNMENT");
  if(align_positions) {
    if(row_bytes>SIZE_MAX-(SGPL_PREP_CACHE_LINE_BYTES-1))return SGPL_DAG_ERR_INVALID;
    row_bytes=(row_bytes+SGPL_PREP_CACHE_LINE_BYTES-1)&~(size_t)(SGPL_PREP_CACHE_LINE_BYTES-1);
  }
  if((size_t)chunks>SIZE_MAX/row_bytes)return SGPL_DAG_ERR_INVALID;
  void *positions_allocation=NULL;
  int64_t *positions=align_positions?
      sgpl_execution_aligned_buffer(c,SGPL_BUF_PREP_POSITIONS,(size_t)chunks*row_bytes,
          SGPL_PREP_CACHE_LINE_BYTES,1,&positions_allocation):
      (positions_allocation=sgpl_execution_buffer(c,SGPL_BUF_PREP_POSITIONS,(size_t)chunks*row_bytes,1));
  int64_t *work=sgpl_execution_buffer(c,SGPL_BUF_PREP_WORK,((size_t)chunks+1)*8,1);
  int32_t *owned_map=NULL;
  const int32_t *owners=NULL;
  if(!positions||!work)goto done;
#ifdef SGPL_PREPARATION_TEST
  extern void sgpl_test_preparation_cursors(const int64_t *,size_t,int32_t,int32_t,int);
  sgpl_test_preparation_cursors(positions,row_bytes/sizeof(int64_t),parts,chunks,align_positions);
#endif
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
      .positions=positions,.work=work,.position_stride=row_bytes/sizeof(int64_t),.chunks=chunks,.parts=parts,
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
  c->array_compact_records=SGPL_EXEC_PAIR_BATCH&&!begins&&!ends&&c->op_count==1&&
      (c->ops[0].capabilities&SGPL_OP_PAIR)&&c->ops[0].pair;
  /* The row validator and m bound prove every first-match hint fits i32.
   * Lifecycle records retain their original representation. */
  if(c->array_compact_records && native_weighted && m->csr_m>=0 && m->csr_m<=INT32_MAX)
    c->array_compact_records=2;
  /* The all-ones gate storage exists even without a gate operation. The
   * compiler proof and lifecycle checks certify absence of observable gating. */
  int lean=c->array_compact_records && !getenv("SGPL_NO_LEAN_RECORDS") &&
      !begins && !ends && c->op_count==1 &&
      c->ops[0].capabilities==(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL) &&
      !c->ops[0].round_begin && !c->ops[0].round_end && !c->ops[0].snapshot &&
      !c->ops[0].source_begin && !c->ops[0].source_end &&
      !c->ops[0].partition_begin && !c->ops[0].partition_end && !c->ops[0].combine;
  if(lean)c->array_compact_records=c->array_compact_records==2?4:3;
  size_t record_bytes=c->array_compact_records==4?16:(c->array_compact_records==3?12:
      (c->array_compact_records==2?20:(c->array_compact_records?16:12)));
  if(lean && getenv("SGPL_RECORD_LIVENESS_DEBUG"))
    fprintf(stderr,"[lean-records] format=%d pairs=%lld bytes=%zu\n",c->array_compact_records,(long long)pair_count,record_bytes*(size_t)records);
  if((uint64_t)records>SIZE_MAX/record_bytes) {rc=SGPL_DAG_ERR_NOMEM;goto done;}
  c->array_append_log=sgpl_execution_buffer(c,SGPL_BUF_APPEND,(size_t)(pair_count?pair_count:1)*c->array_append_sites*8,1);
  c->gpu_records=sgpl_execution_buffer(c,SGPL_BUF_RECORDS,(size_t)(records?records:1)*record_bytes,0);
  if(!c->array_compact_records) {
    c->gpu_record_sources=sgpl_execution_buffer(c,SGPL_BUF_SOURCES,(size_t)(records?records:1)*8,0);
    c->gpu_record_work=sgpl_execution_buffer(c,SGPL_BUF_WORK,(size_t)(records?records:1)*8,0);
  }
  if(native_weighted&&c->array_compact_records!=2&&c->array_compact_records!=4)c->array_record_edges=sgpl_execution_buffer(c,SGPL_BUF_EDGES,(size_t)(records?records:1)*8,0);
  c->gpu_record_offsets=sgpl_execution_buffer(c,SGPL_BUF_OFFSETS,((size_t)parts+1)*8,1);
  if(!c->array_append_log||!c->gpu_records||(!c->array_compact_records&&(!c->gpu_record_sources||!c->gpu_record_work))||!c->gpu_record_offsets||
      (native_weighted&&c->array_compact_records!=2&&c->array_compact_records!=4&&!c->array_record_edges)) {rc=SGPL_DAG_ERR_NOMEM;goto done;}
  int64_t at=0;
  for(int32_t p=0;p<parts;++p) {
    c->gpu_record_offsets[p]=at;
    for(int32_t k=0;k<chunks;++k) {
      size_t index=(size_t)k*job.position_stride+p;
      int64_t count=positions[index];positions[index]=at;at+=count;
    }
  }
  c->gpu_record_offsets[parts]=at;
  if(at!=records) {rc=SGPL_DAG_ERR_INVALID;goto done;}
  sgpl_run_preparation(c->array_compact_records==4?sgpl_array_fill_chunks_lean_weighted:(c->array_compact_records==3?sgpl_array_fill_chunks_lean:(c->array_compact_records==2?sgpl_array_fill_chunks_packed:sgpl_array_fill_chunks)),&job,width);
  rc=atomic_load_explicit(&job.error,memory_order_relaxed);if(rc<0)goto done;
  c->gpu_record_count=records;c->array_log_pristine=1;
  c->array_pair_records=!job.source||(!begins&&!ends);rc=0;
done:
  if(!c->execution_scratch){free(positions_allocation);free(work);}
  free(owned_map);return rc;
}
