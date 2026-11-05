/* Pure preparation contracts. No CUDA launch, timer, benchmark or profiling. */
#include "../autotuner_runtime.c"
#include <assert.h>
#ifdef SGPL_GPU_ALLOCATION_FAILURE_TEST
static int fail_malloc,fail_calloc;
void *__real_malloc(size_t);void *__real_calloc(size_t,size_t);
void *__wrap_malloc(size_t n){if(fail_malloc){--fail_malloc;return NULL;}return __real_malloc(n);}
void *__wrap_calloc(size_t n,size_t s){if(fail_calloc){--fail_calloc;return NULL;}return __real_calloc(n,s);}
#endif
static sgpl_exec_ctx *context(void *graph) {
  sgpl_exec_ctx *c=calloc(1,sizeof(*c));assert(c);
  c->graph=graph;c->traversal_kind=SGPL_TRAVERSE_OWNER_U;
  c->domain_kind=SGPL_DOMAIN_ALL_VERTICES;c->spatial_group=SGPL_SPATIAL_SOURCE;return c;
}
static void same(const sgpl_exec_ctx *a,const sgpl_exec_ctx *b) {
  assert(a->gpu_record_count==b->gpu_record_count&&a->gpu_group_count==b->gpu_group_count);
  assert(!memcmp(a->gpu_records,b->gpu_records,(size_t)a->gpu_record_count*12));
  assert(!memcmp(a->gpu_record_offsets,b->gpu_record_offsets,((size_t)a->gpu_partition_count+1)*8));
  assert(!memcmp(a->gpu_group_offsets,b->gpu_group_offsets,((size_t)a->gpu_group_count+1)*8));
  assert(!memcmp(a->gpu_partition_groups,b->gpu_partition_groups,((size_t)a->gpu_partition_count+1)*8));
}
int main(void) {
  int64_t rows[]={0,3,4,5,6,7},bounds[]={0,2,5},sizes[]={4,3};
  int32_t columns[]={0,2,2,3,1,4,0},p0[]={0,0,0,2,0,2,1,3},p1[]={2,1,3,4,4,0};
  int32_t *pairs[]={p0,p1};
  struct {int64_t n,m;int64_t *rows;int32_t *columns;} graph={5,7,rows,columns};
  g_meta_count=1;AutoGraphMeta *m=&g_meta[0];
  *m=(AutoGraphMeta){.graph_ptr=&graph,.current_layout=LAYOUT_CSR,.csr_n=5,.csr_m=7,
    .csr_row_ptr=rows,.csr_col_idx=columns,.partition_count=2,.partition_start=bounds,
    .src_pairs=pairs,.src_pair_count=sizes,.cc_cache_valid=1};
  setenv("SGPL_GPU_REUSE_RECORDS","0",1);
  sgpl_exec_ctx *a=context(&graph);assert(sgpl_gpu_prepare_spatial_records(a)==0);
  unsetenv("SGPL_GPU_REUSE_RECORDS");
  sgpl_exec_ctx *b=context(&graph);assert(sgpl_gpu_prepare_spatial_records(b)==0);same(a,b);
  SgplGpuPreparation *cache=sgpl_gpu_preparation_cache();assert(cache&&cache->meta==m);
  uint64_t hits=cache->hits;
  sgpl_exec_ctx *c=context(&graph);assert(sgpl_gpu_prepare_spatial_records(c)==0);same(b,c);
  assert(cache->hits==hits+1&&c->gpu_records!=b->gpu_records);
#ifdef SGPL_GPU_ALLOCATION_FAILURE_TEST
  sgpl_exec_ctx *failed=context(&graph);failed->gpu_parallel_groups=1;
  fail_malloc=1;assert(sgpl_gpu_template_load(cache,failed,m,0,0)==0&&!failed->gpu_records&&!failed->gpu_record_offsets);
  int32_t *template_pointer=cache->records_data;fail_malloc=1;
  sgpl_gpu_template_store(cache,b,m,0,0);assert(cache->records_data==template_pointer);
  size_t capacity=cache->capacity;int64_t *count_pointer=cache->counts;fail_calloc=1;
  assert(!sgpl_gpu_group_scratch(cache,capacity+16)&&cache->counts==count_pointer&&cache->capacity==capacity);
  assert(sgpl_gpu_template_load(cache,failed,m,0,0)==1);autograph_exec_ctx_destroy(failed);
#endif
  b->gpu_records[1]=99;assert(c->gpu_records[1]==0&&cache->records_data[1]==0);
  autograph_exec_ctx_destroy(a);autograph_exec_ctx_destroy(b);autograph_exec_ctx_destroy(c);
  ++m->layout_epoch;hits=cache->hits;
  c=context(&graph);assert(sgpl_gpu_prepare_spatial_records(c)==0&&cache->hits==hits);autograph_exec_ctx_destroy(c);
  p0[3]=4;columns[1]=4;atomic_fetch_add(&g_cc_topology_generation,1);hits=cache->hits;
  c=context(&graph);assert(sgpl_gpu_prepare_spatial_records(c)==0&&cache->hits==hits&&c->gpu_records[4]==4);
  int64_t work=-1;assert(sgpl_gpu_spatial_preflight(c,&work)==0&&work==c->gpu_record_count);autograph_exec_ctx_destroy(c);
  int64_t shifted_bound[]={0,3,5},shifted_size[]={5,2};int32_t shifted_first[10];
  memcpy(shifted_first,p0,32);memcpy(shifted_first+8,p1,8);int32_t *shifted_pairs[]={shifted_first,p1+2};
  m->partition_start=shifted_bound;m->src_pair_count=shifted_size;m->src_pairs=shifted_pairs;hits=cache->hits;
  c=context(&graph);assert(sgpl_gpu_prepare_spatial_records(c)==0&&cache->hits==hits&&c->gpu_record_offsets[1]==5);
  autograph_exec_ctx_destroy(c);m->partition_start=bounds;m->src_pair_count=sizes;m->src_pairs=pairs;
  int64_t one_bound[]={0,5},one_size[]={7};int32_t combined[14];memcpy(combined,p0,32);memcpy(combined+8,p1,24);int32_t *one_pairs[]={combined};
  m->partition_count=1;m->partition_start=one_bound;m->src_pairs=one_pairs;m->src_pair_count=one_size;hits=cache->hits;
  c=context(&graph);assert(sgpl_gpu_prepare_spatial_records(c)==0&&cache->hits==hits&&c->gpu_partition_count==1);autograph_exec_ctx_destroy(c);
  m->partition_count=2;m->partition_start=bounds;m->src_pairs=pairs;m->src_pair_count=sizes;
  c=context(&graph);c->ops=calloc(1,sizeof(*c->ops));c->op_count=1;c->ops[0].capabilities=SGPL_OP_SOURCE_END;
  assert(sgpl_gpu_prepare_spatial_records(c)==0&&c->gpu_record_count==12);autograph_exec_ctx_destroy(c);
  c=context(&graph);c->spatial_group=SGPL_SPATIAL_PAIR_OCCURRENCE;
  assert(sgpl_gpu_prepare_spatial_records(c)==0&&c->gpu_group_count==7);autograph_exec_ctx_destroy(c);
  int32_t frontier[]={0,0,4};
  c=context(&graph);c->domain_kind=SGPL_DOMAIN_FRONTIER;c->array_append_sites=1;c->array_sources=frontier;c->array_source_count=3;
  assert(!sgpl_gpu_template_eligible(c,m));
  assert(sgpl_gpu_spatial_preflight(c,&work)==0&&work==7);
  assert(sgpl_gpu_prepare_spatial_records(c)==0&&c->gpu_record_count==7);
  assert(sgpl_gpu_spatial_preflight(c,&work)==0&&work==7);
  autograph_exec_ctx_destroy(c);
  frontier[1]=3;c=context(&graph);c->domain_kind=SGPL_DOMAIN_FRONTIER;c->array_append_sites=1;c->array_sources=frontier;c->array_source_count=3;
  assert(sgpl_gpu_spatial_preflight(c,&work)==0&&work==5);
  assert(sgpl_gpu_prepare_spatial_records(c)==0&&c->gpu_record_count==5);autograph_exec_ctx_destroy(c);
  c=context(&graph);m->partition_count=65;assert(sgpl_gpu_spatial_preflight(c,&work)==1);m->partition_count=2;
  c->array_compact_records=6;assert(sgpl_gpu_spatial_preflight(c,&work)==0&&work==-1);c->array_compact_records=0;
  autograph_exec_ctx_destroy(c);
  sgpl_owner_map_free(pthread_getspecific(sgpl_owner_map_key));pthread_setspecific(sgpl_owner_map_key,NULL);
  puts("PASS GPU records: baseline equality, duplicates/self edges, independent contexts, generation/layout invalidation, changing frontier and exact preflight");
}
