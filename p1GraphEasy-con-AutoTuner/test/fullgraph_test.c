#define SGPL_SPATIAL_FIXTURE_ONLY
#define SGPL_FIXTURE_EDGE_COUNT(g) (((ExecTestGraph *)(g))->m?8:0)
#include "spatial_exec_test.c"
#include <math.h>
#include <stdint.h>
extern void autograph_exec_ctx_bind_fullgraph_v1(sgpl_exec_ctx *,void *,void *,uint64_t,int32_t,uint64_t,uint64_t);
typedef struct {double source[7],result[7];int32_t order[7][16],count[7];int maps;} FullTest;
static void full_test_pair(void *state,sgpl_exec_ctx *ctx,int32_t u,int32_t v) {
  FullTest *s=state;s->order[v][s->count[v]++]=u;s->result[v]=s->source[u]+s->result[v];(void)ctx;
}
static void full_test_gate(sgpl_runtime_op *op,sgpl_exec_ctx *ctx,int32_t u) {
  (void)op;autograph_exec_ctx_set_source_gate(ctx,u,u!=4);
}
static void full_test_map(void *state,sgpl_exec_ctx *ctx,int64_t n,const int64_t *rp,const uint8_t *gate,void *map) {
  FullTest *s=state;++s->maps;double *out=map;
  for(int64_t u=0;u<n;++u)if(rp[u+1]>rp[u]&&(!gate||gate[u]))out[u]=s->source[u];(void)ctx;
}
static void full_test_kernel(void *state,sgpl_exec_ctx *ctx,int32_t lo,int32_t hi,const int64_t *rp,const int32_t *sources,const void *map) {
  FullTest *s=state;
  for(int32_t v=lo;v<hi;++v)for(int64_t e=rp[v];e<rp[v+1];++e){int32_t u=sources[e];
    if(ctx->execution_error&&__atomic_load_n(ctx->execution_error,__ATOMIC_ACQUIRE)<0)return;
    if(ctx->spatial_source_gate&&!ctx->spatial_source_gate[u])continue;
    if(ctx->spatial_groups_seen)ctx->spatial_groups_seen[v]=1;
    s->order[v][s->count[v]++]=u;s->result[v]=((const double *)map)[u]+s->result[v];
  }
}
static sgpl_exec_ctx *full_test_context(ExecTestGraph *g,FullTest *s,int enabled,int inject) {
  memset(s,0,sizeof(*s));s->source[0]=1e16;s->source[1]=1;s->source[2]=-1e16;s->source[3]=-0.;s->source[4]=INFINITY;s->source[5]=NAN;
  s->result[3]=-0.;
  sgpl_runtime_op *gate=autograph_exec_op_create(SGPL_OP_SOURCE_BEGIN,NULL,NULL,NULL,full_test_gate,NULL,NULL,NULL,0);
  sgpl_runtime_op *pair=autograph_exec_op_create(SGPL_OP_PAIR,s,full_test_pair,NULL,NULL,NULL,NULL,NULL,0);
  sgpl_runtime_op *ops[]={gate,pair};
  sgpl_exec_ctx *c=autograph_exec_ctx_create(g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_ALL_VERTICES,NULL,NULL,NULL,0,NULL,NULL,0,ops,2,-1);
  check(c!=NULL,"create full-domain callback context");autograph_exec_ctx_set_dag_axes(c,1,-1);
  check(!autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0),"prepare destination ownership");
  if(enabled){if(inject)atomic_store(&fail_malloc,1);autograph_exec_ctx_bind_fullgraph_v1(c,full_test_kernel,full_test_map,8,SGPL_EXECUTION_ABI_VERSION,sizeof(*c),sizeof(sgpl_runtime_op));}
  return c;
}
static void full_test_equal(FullTest *a,FullTest *b) {
  check(!memcmp(a->result,b->result,sizeof(a->result)),"exact cancellation, NaN/Inf and signed-zero contribution order");
  check(!memcmp(a->order,b->order,sizeof(a->order))&&!memcmp(a->count,b->count,sizeof(a->count)),"duplicate native incoming occurrence order");
}
#ifndef SGPL_FULLGRAPH_TEST_MAIN
#define SGPL_FULLGRAPH_TEST_MAIN main
#endif
int SGPL_FULLGRAPH_TEST_MAIN(void) {
  int64_t rows[]={0,2,4,5,6,7,8,8};int32_t columns[]={2,1,1,1,1,3,2,2};
  int32_t pairs[]={0,2,0,1,1,1,1,1,2,1,3,3,4,2,5,2};
  ExecTestGraph g={.n=7,.m=8,.row_ptr=rows,.col_idx=columns,.directed=1};native_pairs=pairs;
  autograph_init(&g,7,8,NULL,NULL,pairs);autograph_ensure_layout(&g,7,8,rows,columns,&fixture_nodes,&fixture_edges,pairs,LAYOUT_CSR);autograph_build_clean_cut(&g,6);
  for(int fallback=0;fallback<3;++fallback) {
    FullTest a,b;sgpl_exec_ctx *x=full_test_context(&g,&a,0,0),*y=full_test_context(&g,&b,1,fallback==1);
    check((y->array_compact_records==6)==(fallback!=1),"bind eligible incoming traversal or transactional allocation fallback");
    if(fallback==2)autograph_update_csr_pointers(&g,rows,columns);
    check(autograph_frontier_execute(&g,x)==0&&autograph_frontier_execute(&g,y)==0,"complete generic and incoming operations");full_test_equal(&a,&b);
    check(b.maps==(fallback==0),"map runs only after source gates and never after invalidation/fallback");
    if(!fallback) {
      a.source[0]=INFINITY;b.source[0]=INFINITY;
      memset(a.count,0,sizeof a.count);memset(b.count,0,sizeof b.count);memset(a.order,0,sizeof a.order);memset(b.order,0,sizeof b.order);
      check(autograph_frontier_execute(&g,x)==0&&autograph_frontier_execute(&g,y)==0,"reuse context recomputes stable source values");full_test_equal(&a,&b);check(b.maps==2,"one source map per round");
    }
    autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);
  }
  FullTest changed;sgpl_exec_ctx *changed_ctx=full_test_context(&g,&changed,1,0);
  int64_t larger_rows[]={0,2,4,5,6,7,8,8,8};g.n=8;g.row_ptr=larger_rows;
  autograph_record_adjacency_state(&g,8,8,larger_rows,columns);
  check(autograph_frontier_execute(&g,changed_ctx)==SGPL_DAG_ERR_INVALID&&changed_ctx->execution_status==SGPL_DAG_ERR_INVALID&&!changed.maps,"vertex-span publication fails before touching old context storage");
  autograph_exec_ctx_destroy(changed_ctx);g.n=7;g.row_ptr=rows;g.col_idx=columns;autograph_record_adjacency_state(&g,7,8,rows,columns);autograph_build_clean_cut(&g,6);
  for(int layout=1;layout<4;++layout) {
    autograph_ensure_layout(&g,7,8,rows,columns,&fixture_nodes,&fixture_edges,pairs,layout);autograph_build_clean_cut(&g,6);
    FullTest a,b;sgpl_exec_ctx *x=full_test_context(&g,&a,0,0),*y=full_test_context(&g,&b,1,0);
    check(y->array_compact_records!=6,"unsupported layout retains generic operation");
    check(autograph_frontier_execute(&g,x)==0&&autograph_frontier_execute(&g,y)==0,"execute layout fallback");full_test_equal(&a,&b);
    autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);
  }
  int64_t empty_rows[]={0};ExecTestGraph empty={.n=0,.m=0,.row_ptr=empty_rows,.directed=1};
  autograph_init(&empty,0,0,NULL,NULL,NULL);autograph_ensure_layout(&empty,0,0,empty_rows,NULL,&fixture_nodes,&fixture_edges,NULL,LAYOUT_CSR);autograph_build_clean_cut(&empty,6);
  FullTest empty_state;sgpl_exec_ctx *empty_ctx=full_test_context(&empty,&empty_state,1,0);
  check(autograph_frontier_execute(&empty,empty_ctx)==0&&!empty_state.maps,"empty graph invokes no map or pair operation");autograph_exec_ctx_destroy(empty_ctx);
  printf("full graph contracts: %d failures\n",failures);return failures?1:0;
}
