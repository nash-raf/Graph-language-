#define SGPL_SPATIAL_FIXTURE_ONLY
#include "spatial_exec_test.c"
static atomic_int prep_calls,prep_errors,nested_calls;
static void probe_child(void *state,int32_t worker,int32_t width) {
  (void)state;if(worker!=0||width!=1||sgpl_current_thread_budget()!=1)atomic_fetch_add(&prep_errors,1);
  atomic_fetch_add(&nested_calls,1);
}
static void probe_team(void *state,int32_t worker,int32_t width) {
  (void)state;
  if(worker<0||worker>=width||sgpl_current_thread_budget()!=1)atomic_fetch_add(&prep_errors,1);
  atomic_fetch_add(&prep_calls,1);
  sgpl_run_preparation(probe_child,NULL,8);
}
static atomic_int large_calls;
static atomic_int large_errors;
static atomic_int upgrade_pending,cache_probe;
static int32_t large_begins[2048],large_ends[2048];
static void large_begin(sgpl_runtime_op *op,sgpl_exec_ctx *ctx,int32_t u) {
  (void)op;(void)u;++large_begins[autograph_exec_ctx_occurrence(ctx)];
  if(atomic_exchange(&upgrade_pending,0)&&autograph_build_clean_cut(ctx->graph,8)!=8)
    atomic_fetch_add(&large_errors,1);
}
static void large_end(sgpl_runtime_op *op,sgpl_exec_ctx *ctx,int32_t u) {
  (void)op;(void)u;++large_ends[autograph_exec_ctx_occurrence(ctx)];
}
static void large_pair(void *state,sgpl_exec_ctx *ctx,int32_t u,int32_t v) {
  (void)state;atomic_fetch_add(&large_calls,1);
  int expected=2*u+(v==(u+1)%128?1:2);
  if(autograph_exec_ctx_edge_weight(ctx,ctx->graph,u,v)!=expected)atomic_fetch_add(&large_errors,1);
  autograph_frontier_activate(ctx,v);
}
static void count_only(void *state,sgpl_exec_ctx *ctx,int32_t u,int32_t v) {
  (void)u;(void)v;atomic_fetch_add(&large_calls,1);
  if(state&&atomic_exchange(&cache_probe,0)) {
    if(autograph_build_clean_cut(state,8)!=8||autograph_build_clean_cut(ctx->graph,8)!=8)
      atomic_fetch_add(&large_errors,1);
  }
}
static void check_rejected_preparation(ExecTestGraph *graph,const int32_t *input,int count,int sites,int source,const char *message) {
  autograph_init(graph,graph->n,graph->m,NULL,NULL,NULL);
  check(autograph_build_array_cut(graph,8)==8,"prepare malformed-input test ranges");
  int32_t *output=NULL,head=0;int64_t capacity=0;
  sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,count_only,NULL,NULL,NULL,NULL,NULL,0),*ops[]={op};
  sgpl_exec_ctx *ctx=autograph_exec_ctx_create(graph,source?SGPL_TRAVERSE_OWNER_U:SGPL_TRAVERSE_OWNER_V,
      SGPL_DOMAIN_FRONTIER,NULL,NULL,NULL,0,&head,NULL,0,ops,1,-1);
  atomic_store(&large_calls,0);
  check(!autograph_exec_ctx_bind_array_frontier(ctx,input,count,&output,&capacity,sites),"bind rejection fixture");
  int group=source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
  check(autograph_exec_ctx_prepare_spatial(ctx,group,group,1)==SGPL_DAG_ERR_INVALID&&
      !atomic_load(&large_calls)&&!sgpl_debug_reserved_threads(),message);
  autograph_exec_ctx_destroy(ctx);sgpl_frontier_release_buffers();
}
int main(int argc,char **argv) {
  int cold=argc>1&&!strcmp(argv[1],"cold-failure");
  enum { N=128, M=256, F=2047 };
  int64_t rows[N+1];int32_t cols[M],input[F],weights[M];
  for(int u=0;u<N;++u) {rows[u]=2*u;cols[2*u]=(u+1)%N;cols[2*u+1]=(u+7)%N;
    weights[2*u]=2*u+1;weights[2*u+1]=2*u+2;}
  rows[N]=M;for(int i=0;i<F;++i)input[i]=(i*13)%N;
  ExecTestGraph graph={.n=N,.m=M,.row_ptr=rows,.col_idx=cols,.weights=weights,.directed=1};
  autograph_init(&graph,N,M,NULL,NULL,NULL);
  for(int source=0;source<2;++source) {
    check(autograph_build_array_cut(&graph,8)==8,"prepare array-only partition boundaries");
    int32_t *output=NULL,head=0;int64_t capacity=0;
    memset(large_begins,0,sizeof large_begins);memset(large_ends,0,sizeof large_ends);
    sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR|SGPL_OP_SOURCE_BEGIN|SGPL_OP_SOURCE_END,
        NULL,large_pair,NULL,large_begin,large_end,NULL,NULL,0),*ops[]={op};
    sgpl_exec_ctx *ctx=autograph_exec_ctx_create(&graph,source?SGPL_TRAVERSE_OWNER_U:SGPL_TRAVERSE_OWNER_V,
        SGPL_DOMAIN_FRONTIER,NULL,NULL,NULL,0,&head,NULL,0,ops,1,-1);
    int group=source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
    check(!autograph_exec_ctx_bind_array_frontier(ctx,input,F,&output,&capacity,1),"bind repeated-source preparation fixture");
    if(cold&&!source)atomic_store(&fail_thread,1);
    check(!autograph_exec_ctx_prepare_spatial(ctx,group,group,1)&&!atomic_load(&large_calls),
        "parallel preparation completes without executing a user callback");
    check(!sgpl_debug_reserved_threads(),"preparation returns every ledger reservation");
    autograph_exec_ctx_set_dag_axes(ctx,1,-1);
    atomic_store(&upgrade_pending,source);
    check(autograph_frontier_execute(&graph,ctx)==2*F&&atomic_load(&large_calls)==2*F,
        "prepared callbacks run exactly once per pair occurrence");
    int bad=0;for(int i=0;i<F;++i)bad+=output[2*i]!=(input[i]+1)%N||output[2*i+1]!=(input[i]+7)%N;
    check(!bad,"parallel chunks retain original source and neighbor order including duplicates");
    bad=0;for(int i=0;i<F;++i)bad+=large_begins[i]!=1||large_ends[i]!=1;
    check(!bad&&!atomic_load(&large_errors),"parallel source hooks and CSR weight hints retain occurrence identity");
    autograph_exec_ctx_destroy(ctx);sgpl_frontier_release_buffers();atomic_store(&large_calls,0);
  }
  sgpl_push_thread_budget(2);sgpl_run_preparation(probe_team,NULL,8);sgpl_pop_thread_budget();
  check(atomic_load(&prep_calls)<=2&&atomic_load(&prep_calls)>0&&!atomic_load(&prep_errors)&&
        atomic_load(&nested_calls)==atomic_load(&prep_calls)&&!sgpl_debug_reserved_threads(),
      "preparation respects the parent grant, serial nested dispatch and balanced release");
  ExecTestGraph other=graph;autograph_init(&other,N,M,NULL,NULL,NULL);
  check(autograph_build_clean_cut(&other,8)==8,"prepare a second independent graph's full views");
  check(autograph_build_array_cut(&graph,4)==4&&autograph_build_array_cut(&graph,8)==8,
      "prepare lightweight ranges before a full-view consumer");
  sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR,&other,count_only,NULL,NULL,NULL,NULL,NULL,0),*ops[]={op};
  sgpl_exec_ctx *ctx=autograph_exec_ctx_create(&graph,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_ALL_VERTICES,
      NULL,NULL,NULL,0,NULL,NULL,0,ops,1,-1);
  check(!autograph_exec_ctx_prepare_spatial(ctx,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0),
      "non-array consumer upgrades ranges to full CleanCut views before effects");
  atomic_store(&cache_probe,1);autograph_exec_ctx_set_dag_axes(ctx,1,-1);
  check(autograph_frontier_execute(&graph,ctx)==0&&atomic_load(&large_calls)==M,
      "lazy full-view upgrade retains every original adjacency entry");
  check(!atomic_load(&large_errors)&&!atomic_load(&cache_probe),
      "an independent graph's cache lookup cannot rebuild views held by an active callback");
  autograph_exec_ctx_destroy(ctx);
  check(autograph_build_array_cut(&graph,4)==4&&autograph_build_array_cut(&graph,8)==8,
      "restore lightweight ranges before a direct GPU record consumer");
  op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,count_only,NULL,NULL,NULL,NULL,NULL,0);ops[0]=op;
  ctx=autograph_exec_ctx_create(&graph,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_ALL_VERTICES,
      NULL,NULL,NULL,0,NULL,NULL,0,ops,1,-1);
  check(!sgpl_gpu_prepare_spatial_records(ctx)&&ctx->gpu_record_count==M,
      "direct GPU record preparation upgrades lightweight ranges before reading full views");
  autograph_exec_ctx_destroy(ctx);atomic_store(&large_calls,0);
  input[F/2]=N;
  int32_t *output=NULL,head=0;int64_t capacity=0;
  op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,count_only,NULL,NULL,NULL,NULL,NULL,0);ops[0]=op;
  ctx=autograph_exec_ctx_create(&graph,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,
      NULL,NULL,NULL,0,&head,NULL,0,ops,1,-1);
  check(!autograph_exec_ctx_bind_array_frontier(ctx,input,F,&output,&capacity,1),
      "bind malformed preparation input");
  check(autograph_exec_ctx_prepare_spatial(ctx,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,1)==SGPL_DAG_ERR_INVALID&&
      !atomic_load(&large_calls)&&!sgpl_debug_reserved_threads(),
      "invalid parallel preparation executes no effects and releases its entire grant");
  autograph_exec_ctx_destroy(ctx);sgpl_frontier_release_buffers();
  input[F/2]=0;
  ExecTestGraph invalid=graph;invalid.weights=NULL;
  for(int source=0;source<2;++source) {
    int64_t saved_row=rows[64];rows[64]=-1;
    check_rejected_preparation(&invalid,input,F,1,source,"counting rejects negative row bounds before filling or effects");
    rows[64]=rows[65]+1;
    check_rejected_preparation(&invalid,input,F,1,source,"counting rejects decreasing row bounds before filling or effects");
    rows[64]=M+1;
    check_rejected_preparation(&invalid,input,F,1,source,"counting rejects rows beyond the CSR edge span");rows[64]=saved_row;
    int32_t saved_col=cols[127];cols[127]=-1;
    check_rejected_preparation(&invalid,input,F,1,source,"counting rejects negative destinations before filling or effects");
    cols[127]=N;
    check_rejected_preparation(&invalid,input,F,1,source,"counting rejects destinations beyond the vertex span");cols[127]=saved_col;
    check_rejected_preparation(&invalid,input,1,INT32_MAX,source,"row-degree check rejects append ordinal overflow without large allocations");
    check_rejected_preparation(&invalid,input,129,INT32_MAX/256,source,"host prefix rejects aggregate append overflow across individually valid chunks");
  }
  printf("parallel array preparation: %d failures\n",failures);return failures?1:0;
}
