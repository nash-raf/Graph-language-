#define SGPL_SPATIAL_FIXTURE_ONLY
#define SGPL_FIXTURE_EDGE_COUNT(g) (((ExecTestGraph *)(g))->m)
#include "spatial_exec_test.c"
static atomic_int context_pair_errors;
static void context_pair(void *s,sgpl_exec_ctx *c,int32_t u,int32_t v) {
  (void)s;(void)u;(void)v;
  if(c->array_work_ordinal<0||c->array_work_ordinal>=c->array_work_count)
    atomic_fetch_add(&context_pair_errors,1);
}
int main(void) {
  int64_t rows[]={0,16,32,48,48,52,56,56};int32_t columns[56],weights[56],edges[112];
  for(int32_t u=0;u<7;++u)for(int64_t e=rows[u];e<rows[u+1];++e) {
    columns[e]=(e*3+1)%7;weights[e]=e+7;edges[2*e]=u;edges[2*e+1]=columns[e];
  }
  native_pairs=edges;ExecTestGraph g={.n=7,.m=56,.row_ptr=rows,.col_idx=columns,.weights=weights,.directed=1};
  autograph_init(&g,7,56,NULL,NULL,edges);
  autograph_ensure_layout(&g,7,56,rows,columns,&fixture_nodes,&fixture_edges,edges,LAYOUT_CSR);
  autograph_build_clean_cut(&g,3);
  int32_t input[]={4,1,3,0,1,6,4,1,3,0,1,6,4,1,3,0,1,6,4,1,3,0,1,6};
  for(int lean=0;lean<2;++lean) {
    int32_t output[600],*out=output,head=0;int64_t capacity=600;
    for(int i=0;i<600;++i)output[i]=700+i;
    sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR|(lean?SGPL_OP_NO_SOURCE_ORDINAL:0),NULL,context_pair,NULL,NULL,NULL,NULL,NULL,0);
    sgpl_runtime_op *ops[]={op};
    sgpl_exec_ctx *c=autograph_exec_ctx_create(&g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,NULL,NULL,out,2,&head,NULL,0,ops,1,-1);
    check(c!=NULL,"create context-copy stress case");autograph_exec_ctx_set_dag_axes(c,1,-1);
    check(!autograph_exec_ctx_bind_array_frontier(c,input,24,&out,&capacity,1),"bind duplicate work");
    check(!autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0),"prepare generic or lean ownership");
    for(int round=0;round<128;++round) {
      check(autograph_frontier_execute(&g,c)==2&&c->spatial_occupied_groups==7,
        "concurrent context snapshots preserve joined group counts and append prefix");
    }
    for(int i=0;i<600;++i)check(output[i]==700+i,"unused output remains untouched");
    autograph_exec_ctx_destroy(c);
  }
  check(!atomic_load(&context_pair_errors),"all original work ordinals remain valid");
  printf("context-copy contracts: %d failures\n",failures);return failures?1:0;
}
