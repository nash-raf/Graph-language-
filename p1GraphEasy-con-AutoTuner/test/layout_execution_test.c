/* Reuse the independent fixture's expected sequences, ownership probes and
 * lifecycle checks, but route preparation exclusively through native views. */
#define SGPL_SPATIAL_FIXTURE_ONLY
#define autograph_build_clean_cut autograph_build_layout_batches
#define autograph_exec_ctx_prepare_spatial autograph_exec_ctx_prepare_layout
#include "spatial_exec_test.c"
#undef autograph_build_clean_cut
#undef autograph_exec_ctx_prepare_spatial

static atomic_int append_calls;
static void append_pair(void *unused,sgpl_exec_ctx *c,int32_t u,int32_t v) {
  (void)unused;(void)u;atomic_fetch_add(&append_calls,1);
  autograph_frontier_activate(c,v);autograph_frontier_activate(c,v);
}
static void test_ordered_native(ExecTestGraph *g,int layout,int source) {
  autograph_ensure_layout(g,7,7,g->row_ptr,g->col_idx,&fixture_nodes,&fixture_edges,pairs,layout);
  check(autograph_build_layout_batches(g,3)==3,"native array ranges");
  int32_t input[]={4,1,3,0,1,6};
  int32_t expected_values[]={1,1,2,2,2,2,1,1,2,2,2,2,2,2};
  int32_t initial[2]={99,88},*output=initial,head=0;int64_t capacity=2;
  sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,append_pair,NULL,NULL,NULL,NULL,NULL,0);
  sgpl_runtime_op *ops[]={op};
  sgpl_exec_ctx *c=autograph_exec_ctx_create(g,source?SGPL_TRAVERSE_OWNER_U:SGPL_TRAVERSE_OWNER_V,
      SGPL_DOMAIN_FRONTIER,NULL,NULL,output,2,&head,NULL,0,ops,1,-1);
  autograph_exec_ctx_set_dag_axes(c,1,-1);
  check(!autograph_exec_ctx_bind_array_frontier(c,input,6,&output,&capacity,2),"native bind repeated sources");
  int group=source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
  check(!autograph_exec_ctx_prepare_layout(c,group,group,1),"native prepare array output");
  check(c->layout_execution&&!c->gpu_records&&!c->gpu_record_offsets&&!c->gpu_group_offsets,
        "native execution allocates no edge records or device groups");
  atomic_store(&append_calls,0);

  for(int repeat=0;repeat<2;++repeat) {
    check(autograph_frontier_execute(g,c)==16,"native append cardinality and prefix");
    check(output[0]==99&&output[1]==88&&!memcmp(output+2,expected_values,sizeof expected_values),
          "native output matches original duplicate occurrence order");
  }
  AutoNeighborIter inspect;autograph_neighbor_iter_init(g,0,&inspect);
  AutoGraphMeta *meta=inspect.meta;
  check(!meta->src_pairs&&!meta->push_rp&&!meta->push_ci&&!meta->push_indir,
        "native execution does not materialize CleanCut edge payloads");
  check(atomic_load(&append_calls)==14,"native context reuse executes exact pair count");
  autograph_exec_ctx_destroy(c);sgpl_frontier_release_buffers();
}

static void test_native_failures(ExecTestGraph *g) {
  autograph_ensure_layout(g,7,7,g->row_ptr,g->col_idx,&fixture_nodes,&fixture_edges,pairs,LAYOUT_CSR);
  autograph_build_layout_batches(g,3);
  for(int allocation=1;allocation<=8;++allocation) {
    spatial_state s={0};sgpl_exec_ctx *c=context(g,&s,0,0,NULL,NULL,NULL,NULL);
    atomic_store(&fail_malloc,allocation);
    int rc=autograph_exec_ctx_prepare_layout(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0);
    atomic_store(&fail_malloc,0);
    check(rc==0||rc==SGPL_DAG_ERR_NOMEM,"native preparation reports allocation failure");
    check(!atomic_load(&s.calls),"native preparation executes no pair effects");
    autograph_exec_ctx_destroy(c);
  }
  spatial_state s={0};sgpl_exec_ctx *c=context(g,&s,0,0,NULL,NULL,NULL,NULL);
  check(!autograph_exec_ctx_prepare_layout(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0),
        "native context prepared before topology publication");
  autograph_record_adjacency_state(g,g->n,g->m,g->row_ptr,g->col_idx);
  check(autograph_frontier_execute(g,c)==SGPL_DAG_ERR_INVALID&&!atomic_load(&s.calls),
        "stale native view fails before callbacks; no stale traversal or replay");
  autograph_exec_ctx_destroy(c);
}

int main(void) {
  int64_t rows[]={0,2,4,5,5,6,7,7};int32_t columns[]={1,2,2,2,3,1,6};
  ExecTestGraph g={.n=7,.m=7,.row_ptr=rows,.col_idx=columns,.directed=1};
  autograph_init(&g,7,7,NULL,NULL,pairs);
  for(int layout=0;layout<4;++layout)for(int source=0;source<2;++source) {
    fprintf(stderr,"TEST layout=%d source=%d\n",layout,source);
    for(int parts=1;parts<=7;parts+=3)for(int frontier=0;frontier<2;++frontier)
      test_layout(&g,layout,parts,source,frontier);
    test_ordered_native(&g,layout,source);


  }
  test_native_failures(&g);test_snapshots(&g);test_cardinality(&g);test_live_set(&g);
  test_undirected_duplicates();test_empty();
  printf("layout-native executor: %d failures\n",failures);
  return failures?1:0;
}
