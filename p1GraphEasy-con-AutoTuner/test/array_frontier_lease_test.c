#define SGPL_ARRAY_FRONTIER_TEST_MAIN existing_array_contract_checks
#include "array_frontier_exec_test.c"
static sgpl_exec_ctx *lease_context(ExecTestGraph *g,const int32_t *input,int32_t **out,int64_t *capacity,int32_t *head) {
  sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,array_pair,NULL,NULL,NULL,NULL,NULL,0);
  sgpl_runtime_op *ops[]={op};
  sgpl_exec_ctx *c=autograph_exec_ctx_create(g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,
      NULL,NULL,NULL,0,head,NULL,0,ops,1,-1);
  check(c&&!autograph_exec_ctx_bind_array_frontier(c,input,1,out,capacity,2)&&
      !autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,1),
      "prepare independently leased array context");
  return c;
}
int main(void) {
  if(existing_array_contract_checks())return 1;
  int64_t rp[]={0,2,4,5,5,6,7,7};int32_t ci[]={1,2,2,2,3,1,6};
  ExecTestGraph g={.n=7,.m=7,.row_ptr=rp,.col_idx=ci,.directed=1};
  autograph_init(&g,7,7,NULL,NULL,pairs);check(autograph_build_array_cut(&g,3)==3,"array cut");
  int32_t ia[]={0},ib[]={4},ic[]={1};int32_t *oa=NULL,*ob=NULL,*oc=NULL,ha=0,hb=0,hc=0;
  int64_t ca=0,cb=0,cc=0;
  sgpl_exec_ctx *a=lease_context(&g,ia,&oa,&ca,&ha),*b=lease_context(&g,ib,&ob,&cb,&hb);
  uint8_t *groups=a->spatial_groups_seen,*gate=a->spatial_source_gate;
  check(groups!=b->spatial_groups_seen&&gate!=b->spatial_source_gate,"live contexts cannot alias mutable lease buffers");
  autograph_exec_ctx_set_source_gate(a,0,0);
  skip_appends=0;
  check(autograph_frontier_execute(&g,a)==0,"source gate can suppress one context");
  check(autograph_frontier_execute(&g,b)==2&&ob[0]==1&&ob[1]==1,"other context's gate and output remain independent");
  a->spatial_groups_seen[6]=1; /* a stale endpoint outside the next domain */
  autograph_exec_ctx_destroy(a);
  sgpl_exec_ctx *c=lease_context(&g,ic,&oc,&cc,&hc);
  check(c->spatial_groups_seen==groups&&c->spatial_source_gate==gate,"sequential contexts reuse the returned scratch lease");
  check(c->spatial_groups_seen[6]==1,"cached bytes outside the live domain need no eager clear");
  check(c->spatial_source_gate[0]==1&&b->spatial_groups_seen[1]==1,"reuse initializes gates and preserves another live context's groups");
  check(autograph_frontier_execute(&g,c)==4&&oc[0]==2&&oc[3]==2,"duplicate sources/edges remain occurrences after reuse");
  check(autograph_frontier_execute(&g,c)==4&&c->spatial_occupied_groups==1,"repeated execution resets occupied groups");
  check(!sgpl_gpu_prepare_spatial_records(c),"lazy device grouping transfers every owned lease buffer");
  for(int i=0;i<7;++i)check(!c->spatial_groups_seen[i],"device preparation materializes a clean full-domain view");
  check(autograph_frontier_execute(&g,c)==4&&oc[0]==2&&oc[3]==2,"CPU fallback after ownership transfer retains output semantics");
  autograph_exec_ctx_destroy(c);autograph_exec_ctx_destroy(b);sgpl_frontier_release_buffers();
  printf("array scratch lease contract: %d failures\n",failures);return failures?1:0;
}
