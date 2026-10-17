#define SGPL_ARRAY_FRONTIER_TEST_MAIN unused_array_contract_main
#include "array_frontier_exec_test.c"
static atomic_int partition_begins,partition_ends,combines,round_begins,round_ends;
static void pb(sgpl_runtime_op *op,sgpl_exec_ctx *c,int32_t p){(void)op;(void)c;(void)p;atomic_fetch_add(&partition_begins,1);}
static void pe(sgpl_runtime_op *op,sgpl_exec_ctx *c,int32_t p){(void)op;(void)c;(void)p;atomic_fetch_add(&partition_ends,1);}
static void co(void *s,sgpl_exec_ctx*c){(void)s;(void)c;atomic_fetch_add(&combines,1);}
static void rb(sgpl_runtime_op*o,sgpl_exec_ctx*c){(void)o;(void)c;atomic_fetch_add(&round_begins,1);}
static void re(sgpl_runtime_op*o,sgpl_exec_ctx*c){(void)o;(void)c;atomic_fetch_add(&round_ends,1);}
int main(void){
 int64_t rp[]={0,2,4,5,5,6,7,7};int32_t ci[]={1,2,2,2,3,1,6};ExecTestGraph g={.n=7,.m=7,.row_ptr=rp,.col_idx=ci,.directed=1};
 autograph_init(&g,7,7,NULL,NULL,pairs);check(autograph_build_array_cut(&g,3)==3,"array cut");
 int32_t empty[]={3},one[]={0},*out=NULL,head=0;int64_t capacity=0;
 for(int lifecycle=0;lifecycle<2;++lifecycle)for(int nonempty=0;nonempty<2;++nonempty){
  atomic_store(&partition_begins,0);atomic_store(&partition_ends,0);atomic_store(&combines,0);atomic_store(&round_begins,0);atomic_store(&round_ends,0);atomic_store(&occurrence_calls,0);
  uint64_t caps=SGPL_OP_PAIR|SGPL_OP_COMBINE|SGPL_OP_ROUND_BEGIN|SGPL_OP_ROUND_END;
  if(lifecycle)caps|=SGPL_OP_PARTITION_BEGIN|SGPL_OP_PARTITION_END;
  sgpl_runtime_op *op=autograph_exec_op_create(caps,NULL,array_pair,co,NULL,NULL,NULL,NULL,0),*ops[]={op};
  op->partition_begin=pb;op->partition_end=pe;op->round_begin=rb;op->round_end=re;
  sgpl_exec_ctx*c=autograph_exec_ctx_create(&g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,NULL,NULL,NULL,0,&head,NULL,0,ops,1,-1);
  autograph_exec_ctx_set_dag_axes(c,1,-1);autograph_exec_ctx_own_round(c,1,1);
  check(!autograph_exec_ctx_bind_array_frontier(c,nonempty?one:empty,1,&out,&capacity,2)&&!autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,1),"prepare empty nodes");
  check(autograph_frontier_execute(&g,c)==(nonempty?4:0),"stable output with empty nodes");
  check(atomic_load(&partition_begins)==(lifecycle?3:0)&&atomic_load(&partition_ends)==(lifecycle?3:0),"empty partitions retain observable partition lifecycle");
  check(atomic_load(&combines)==3&&atomic_load(&round_begins)==1&&atomic_load(&round_ends)==1,"combine and round lifecycle unchanged");
  check(atomic_load(&occurrence_calls)==(nonempty?2:0),"only live pairs execute");
  if(!nonempty&&!lifecycle){c->execution_status=SGPL_DAG_ERR_INVALID;check(sgpl_exec_dag_spatial(c,3,32)<0,"all-empty dispatch retains cancellation");}
  autograph_exec_ctx_destroy(c);
 }
 sgpl_frontier_release_buffers();printf("empty partition effects: %d failures\n",failures);return failures?1:0;
}
