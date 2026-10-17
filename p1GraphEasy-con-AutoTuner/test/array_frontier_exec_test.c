#define SGPL_SPATIAL_FIXTURE_ONLY
#include "spatial_exec_test.c"
static atomic_int occurrence_calls;
static int skip_appends;
static int begins_by_occurrence[16],ends_by_occurrence[16];
static int64_t weight_rp[]={0,2,4,5,5,6,7,7};
static int32_t weight_ci[]={1,2,2,2,3,1,6},weight_data[]={7,8,40,41,9,10,11};
int32_t graph_get_edge_weight(void *graph,int32_t u,int32_t v) {
  /* This link stub models the original weight query. PCSR has holes and
   * BCSR/SET do not publish a loader CSR, so use the immutable fixture rows. */
  ExecTestGraph *g=graph;
  if(u<0||u>=g->n)return INT32_MAX;
  for(int64_t j=weight_rp[u];j<weight_rp[u+1];++j)
    if(weight_ci[j]==v)return weight_data[j];
  return INT32_MAX;
}
static atomic_int weight_errors;
static void array_pair(void *unused,sgpl_exec_ctx *ctx,int32_t u,int32_t v) {
  (void)unused;(void)u;atomic_fetch_add(&occurrence_calls,1);
  if(autograph_exec_ctx_edge_weight(ctx,ctx->graph,u,v)!=graph_get_edge_weight(ctx->graph,u,v))
    atomic_fetch_add(&weight_errors,1);
  /* A query on a different destination must not consume this pair's hint. */
  if(autograph_exec_ctx_edge_weight(ctx,ctx->graph,u,6)!=graph_get_edge_weight(ctx->graph,u,6))
    atomic_fetch_add(&weight_errors,1);
  if(skip_appends)return;
  autograph_frontier_activate(ctx,v);autograph_frontier_activate(ctx,v);
}
static void array_begin(sgpl_runtime_op *op,sgpl_exec_ctx *ctx,int32_t u) {
  (void)op;(void)u;++begins_by_occurrence[autograph_exec_ctx_occurrence(ctx)];
}
static void array_end(sgpl_runtime_op *op,sgpl_exec_ctx *ctx,int32_t u) {
  (void)op;(void)u;++ends_by_occurrence[autograph_exec_ctx_occurrence(ctx)];
}
static void *destroy_snapshot_context(void *opaque) {
  autograph_exec_ctx_destroy(opaque);return NULL;
}
#ifndef SGPL_ARRAY_FRONTIER_TEST_MAIN
#define SGPL_ARRAY_FRONTIER_TEST_MAIN main
#endif
int SGPL_ARRAY_FRONTIER_TEST_MAIN(void) {
  int64_t rp[8]={0,2,4,5,5,6,7,7};int32_t ci[7]={1,2,2,2,3,1,6};
  int32_t weights[]={7,8,40,41,9,10,11};
  ExecTestGraph g={.n=7,.m=7,.row_ptr=rp,.col_idx=ci,.weights=weights,.directed=1};
  autograph_init(&g,7,7,NULL,NULL,pairs);
  int32_t input[]={4,1,3,0,1,6};
  int32_t expected_output[]={1,1,2,2,2,2,1,1,2,2,2,2,2,2};
  for(int layout=0;layout<4;++layout)for(int source=0;source<2;++source) {
    autograph_ensure_layout(&g,7,7,rp,ci,&fixture_nodes,&fixture_edges,pairs,layout);
    check(autograph_build_clean_cut(&g,3)==3,"prepare array CleanCut topology");
    int32_t initial[2]={99,88},*out=initial,head=0;int64_t cap=2;
    atomic_store(&occurrence_calls,0);memset(begins_by_occurrence,0,sizeof begins_by_occurrence);
    skip_appends=0;
    memset(ends_by_occurrence,0,sizeof ends_by_occurrence);
    sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR|SGPL_OP_SOURCE_BEGIN|SGPL_OP_SOURCE_END,
        NULL,array_pair,NULL,array_begin,array_end,NULL,NULL,0);
    sgpl_runtime_op *ops[]={op};
    sgpl_exec_ctx *ctx=autograph_exec_ctx_create(&g,source?SGPL_TRAVERSE_OWNER_U:SGPL_TRAVERSE_OWNER_V,
        SGPL_DOMAIN_FRONTIER,NULL,NULL,out,0,&head,NULL,0,ops,1,-1);
    check(!autograph_exec_ctx_bind_array_frontier(ctx,input,6,&out,&cap,2),"bind array frontier");
    int key=source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
    check(!autograph_exec_ctx_prepare_spatial(ctx,key,key,1),"reserve ordered occurrence log");
    check(ctx->gpu_group_offsets==NULL,"CPU preparation defers device grouping");
    if(layout==LAYOUT_CSR)check(ctx->array_record_edges!=NULL,"native weighted CSR prepares direct edge hints");
    check(autograph_frontier_execute(&g,ctx)==14,"retain duplicate append cardinality");
    check(atomic_load(&occurrence_calls)==7,"duplicate sources and edges execute exactly once per occurrence");
    check(cap>=14&&!memcmp(out,expected_output,sizeof expected_output),"compact in original source and neighbor order");
    for(int i=0;i<6;++i)check(begins_by_occurrence[i]==1&&ends_by_occurrence[i]==1,"source hooks include zero-neighbor occurrences");
    skip_appends=1;
    check(autograph_frontier_execute(&g,ctx)==0&&atomic_load(&occurrence_calls)==14,
        "reused context clears prior append validity without replaying a dispatch");
    check(!sgpl_gpu_prepare_spatial_records(ctx)&&ctx->gpu_group_offsets&&ctx->gpu_partition_groups,
        "device grouping can be prepared lazily on the same context");
    skip_appends=0;
    check(autograph_frontier_execute(&g,ctx)==14&&!memcmp(out,expected_output,sizeof expected_output),
        "CPU fallback after device grouping preserves every original append occurrence");
    autograph_exec_ctx_destroy(ctx);sgpl_frontier_release_buffers();
    if(layout==LAYOUT_CSR&&!source) {
      ci[1]=1;
      autograph_record_adjacency_state(&g,7,7,rp,ci);
      int32_t one[]={0};out=initial;cap=2;head=0;
      op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,array_pair,NULL,NULL,NULL,NULL,NULL,0);ops[0]=op;
      ctx=autograph_exec_ctx_create(&g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,NULL,NULL,out,0,&head,NULL,0,ops,1,-1);
      check(!autograph_exec_ctx_bind_array_frontier(ctx,one,1,&out,&cap,2)&&
            !autograph_exec_ctx_prepare_spatial(ctx,key,key,1),"rebuild weight hints after same-pointer topology publication");
      check(autograph_frontier_execute(&g,ctx)==4&&out[0]==1&&out[1]==1&&out[2]==1&&out[3]==1&&
            !atomic_load(&weight_errors),"mutation invalidates the prior first-match weight index");
      autograph_exec_ctx_destroy(ctx);sgpl_frontier_release_buffers();
      ci[1]=2;autograph_record_adjacency_state(&g,7,7,rp,ci);
    }
    out=initial;cap=2;head=0;
    op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,array_pair,NULL,NULL,NULL,NULL,NULL,0);
    ops[0]=op;
    ctx=autograph_exec_ctx_create(&g,source?SGPL_TRAVERSE_OWNER_U:SGPL_TRAVERSE_OWNER_V,
        SGPL_DOMAIN_FRONTIER,NULL,NULL,out,2,&head,NULL,0,ops,1,-1);
    check(!autograph_exec_ctx_bind_array_frontier(ctx,input,6,&out,&cap,2)&&
          !autograph_exec_ctx_prepare_spatial(ctx,key,key,1),"prepare nonempty output prefix");
    if(layout==LAYOUT_CSR)
      check(ctx->array_compact_records&&!ctx->gpu_record_sources&&!ctx->gpu_record_work,
            "pair-only CSR batches use compact records without ordinal side arrays");
    check(autograph_frontier_execute(&g,ctx)==16&&out[0]==99&&out[1]==88&&
          !memcmp(out+2,expected_output,sizeof expected_output),"growth preserves prefix and every append occurrence");
    uint64_t expected_groups=0;
    for(int j=0;j<6;++j)for(int64_t e=weight_rp[input[j]];e<weight_rp[input[j]+1];++e)
      expected_groups|=UINT64_C(1)<<(source?input[j]:weight_ci[e]);
    check(ctx->spatial_occupied_groups==(uint64_t)__builtin_popcountll(expected_groups),
          "occupied groups equal the reference traversal's distinct endpoints");
    skip_appends=1;
    check(autograph_frontier_execute(&g,ctx)==2 &&
          ctx->spatial_occupied_groups==(uint64_t)__builtin_popcountll(expected_groups),
          "context reuse resets group counts and preserves the output prefix");
    skip_appends=0;
    if(ctx->array_compact_records) {
      atomic_store(&fail_malloc,1);
      check(sgpl_gpu_prepare_spatial_records(ctx)==SGPL_DAG_ERR_NOMEM&&ctx->array_compact_records,
            "failed device expansion preserves the compact CPU representation");
      check(autograph_frontier_execute(&g,ctx)==16&&!memcmp(out+2,expected_output,sizeof expected_output),
            "CPU execution remains valid after device expansion failure");
    }
    check(!sgpl_gpu_prepare_spatial_records(ctx)&&!ctx->array_compact_records&&ctx->gpu_record_sources&&ctx->gpu_record_work,
          "device grouping expands compact records and preserves occurrence metadata");
    check(autograph_frontier_execute(&g,ctx)==16&&out[0]==99&&out[1]==88&&
          !memcmp(out+2,expected_output,sizeof expected_output),
          "CPU fallback after expansion retains the prefix, order and duplicate appends");
    autograph_exec_ctx_destroy(ctx);sgpl_frontier_release_buffers();
  }
  check(!atomic_load(&weight_errors),"CSR hints and duplicate/layout/device fallbacks preserve first-match edge weights");
  int32_t old[]={7,8},*data=old;int64_t capacity=2;
  check(!sgpl_frontier_ensure_i32(&data,&capacity,2)&&data!=old&&data[0]==7&&data[1]==8,
      "fallback grows and copies existing stack buffer");
  setenv("SGPL_FRONTIER_FAIL","grow",1);
  int32_t *before=data;int64_t before_capacity=capacity;
  check(sgpl_frontier_ensure_i32(&data,&capacity,capacity)==SGPL_DAG_ERR_NOMEM&&data==before&&capacity==before_capacity,
      "failed growth preserves the old allocation and its capacity");
  unsetenv("SGPL_FRONTIER_FAIL");sgpl_frontier_release_buffers();
  data=old;capacity=2;
  check(!sgpl_frontier_reserve_i32(&g,input,6,&data,&capacity,2,1)&&capacity==15&&data[0]==7,
      "symbolic traversal bound includes duplicate source occurrences and the existing prefix");
  sgpl_frontier_release_buffers();data=old;capacity=2;
  setenv("SGPL_FRONTIER_FAIL","alloc",1);
  check(sgpl_frontier_reserve_i32(&g,input,6,&data,&capacity,2,1)==SGPL_DAG_ERR_NOMEM&&data==old,
      "reservation failure leaves the original buffer before any effect");
  unsetenv("SGPL_FRONTIER_FAIL");
  int32_t live_a[]={11,22},live_b[]={33,44},live_c[]={55,66};
  sgpl_exec_ctx *a=autograph_exec_ctx_create(&g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,
      NULL,NULL,NULL,0,NULL,NULL,0,NULL,0,-1);
  sgpl_exec_ctx *b=autograph_exec_ctx_create(&g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,
      NULL,NULL,NULL,0,NULL,NULL,0,NULL,0,-1);
  check(a&&b&&a->execution_scratch&&b->execution_scratch&&a->execution_scratch!=b->execution_scratch,
      "simultaneous contexts have exclusive scratch leases");
  check(!autograph_exec_ctx_bind_snapshot_range(a,0,live_a,2,4)&&
        !autograph_exec_ctx_bind_snapshot_range(b,0,live_b,2,4),"reserve separate live snapshots");
  int32_t *snapshot_a=autograph_exec_ctx_snapshot(a,0),*snapshot_b=autograph_exec_ctx_snapshot(b,0);
  live_a[0]=99;live_b[0]=99;
  check(snapshot_a!=snapshot_b&&snapshot_a[0]==11&&snapshot_b[0]==33,
      "overlapping context snapshots remain frozen and isolated");
  void *lease=a->execution_scratch;autograph_exec_ctx_destroy(a);
  sgpl_exec_ctx *c=autograph_exec_ctx_create(&g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,
      NULL,NULL,NULL,0,NULL,NULL,0,NULL,0,-1);
  check(c&&c->execution_scratch==lease&&
        !autograph_exec_ctx_bind_snapshot_range(c,0,live_c,2,4),"reuse the returned lease for a later context");
  check(autograph_exec_ctx_snapshot(c,0)==snapshot_a&&snapshot_b[0]==33,
      "reused snapshot capacity cannot overwrite another active context");
  atomic_store(&fail_malloc,1);
  check(autograph_exec_ctx_reserve_snapshot(c,0,10000,4)==SGPL_DAG_ERR_NOMEM&&
        autograph_exec_ctx_snapshot(c,0)==snapshot_a&&snapshot_a[0]==55,
      "failed scratch growth retains the current snapshot");
  pthread_t release_thread;
  check(!pthread_create(&release_thread,NULL,destroy_snapshot_context,b),"transfer lease destruction to another thread");
  pthread_join(release_thread,NULL);
  check(snapshot_a[0]==55,"thread-exit cleanup preserves other live leases");
  autograph_exec_ctx_destroy(c);
  for(int parts=2;parts<=5;parts+=3) {
    check(autograph_build_clean_cut(&g,parts)==parts,"change partition boundaries between invocations");
    int32_t *output=NULL,append_head=0;int64_t output_capacity=0;
    sgpl_runtime_op *pair_op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,array_pair,NULL,NULL,NULL,NULL,NULL,0);
    sgpl_runtime_op *pair_ops[]={pair_op};
    sgpl_exec_ctx *partitioned=autograph_exec_ctx_create(&g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,
        NULL,NULL,NULL,0,&append_head,NULL,0,pair_ops,1,-1);
    check(!autograph_exec_ctx_bind_array_frontier(partitioned,input,6,&output,&output_capacity,2)&&
          !autograph_exec_ctx_prepare_spatial(partitioned,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,1),
        "prepare array records after repartitioning");
    skip_appends=0;
    check(autograph_frontier_execute(&g,partitioned)==14&&
          !memcmp(output,expected_output,sizeof expected_output),
        "cached ownership follows changed boundaries and preserves occurrence order");
    autograph_exec_ctx_destroy(partitioned);sgpl_frontier_release_buffers();
  }
  printf("ordered array frontier: %d failures\n",failures);return failures?1:0;
}
