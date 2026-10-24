#define SGPL_SIMPLIFICATION_TEST_MAIN previous_simplification_main
#include "execution_simplify_runtime_test.c"
static atomic_int inplace_calls;
static void lifetime_map(void *s,sgpl_exec_ctx *c,int64_t hi,const int64_t *rows,const uint8_t *gate,void *storage,int64_t lo) {
  if(storage==iteration_source)atomic_fetch_add(&inplace_calls,1);
  iteration_map(s,c,hi,rows,gate,storage,lo);
}
static void lifetime_cases(ExecTestGraph *g) {
  unsigned cases=0;
  for(int data=0;data<4;++data)for(int rounds=1;rounds<=20;rounds+=rounds<3?1:17)
    for(unsigned subset=0;subset<4;++subset)for(unsigned live=0;live<4;++live)for(int mode=0;mode<9;++mode) {
      double initial[]={1e16,-1e16,1.0,-0.0,0.0,INFINITY,NAN},a[7],b[7],want[7];
      if(data==1)initial[5]=0;if(data==2)initial[0]=NAN;if(data==3)initial[0]=-0.0;
      memcpy(a,initial,sizeof a);memset(b,0,sizeof b);iteration_source=a;iteration_destination=b;
      sgpl_iteration_memory_plan_v1 p={1,sizeof(p),(void *)iteration_kernel,iteration_gate,&iteration_source,&iteration_destination,iteration_kernel,lifetime_map,8};
      check(sgpl_try_iteration_memory_v1(g,&p,-0.0,rounds)==1,"retained lifetime reference executes");memcpy(want,a,sizeof a);
      memcpy(a,initial,sizeof a);memset(b,0,sizeof b);atomic_store(&inplace_calls,0);
      unsigned rules=(subset&1?SGPL_EXEC_R17:0)|(subset&2?SGPL_EXEC_R18:0);
      sgpl_iteration_storage_plan_v1 extra={1,sizeof(extra),rules,live,&p,p.baseline_pair,NULL};
      if(mode==1)++extra.version;if(mode==2)--extra.bytes;if(mode==3)extra.rules|=SGPL_EXEC_R16;
      if(mode==4)extra.live_arrays|=4;if(mode==5)extra.baseline=NULL;if(mode==6)extra.baseline_pair=NULL;
      if(mode==7)p.stride=16;if(mode==8)setenv("SGPL_NO_ITERATION_LIFETIMES","1",1);
      check(sgpl_try_iteration_storage_v1(g,&p,&extra,-0.0,rounds)==1,"invalid storage extension retains execution");
      unsetenv("SGPL_NO_ITERATION_LIFETIMES");
      int valid=mode==0||mode==7;unsigned observed=valid&&(subset&1)?live:3;
      check(!(observed&1)||!memcmp(a,want,sizeof a),"all observed source bits preserved");
      check(!(observed&2)||!memcmp(b,want,sizeof b),"all observed destination bits preserved");
      check(iteration_source==a&&iteration_destination==b,"original array identities restored for every liveness mask");
      check((atomic_load(&inplace_calls)>0)==(mode==0&&!!(subset&2)),"reuse requires compatible descriptor, exact stride and enabled rule");
      ++cases;
    }
  // Fallible preparation must finish before destructive source-map writes.
  for(int fail=1;fail<=4;++fail) {
    double a[7]={1,2,3,4,5,6,7},b[7]={0},original[7];memcpy(original,a,sizeof a);
    iteration_source=a;iteration_destination=b;
    sgpl_iteration_memory_plan_v1 p={1,sizeof(p),(void *)iteration_kernel,iteration_gate,&iteration_source,&iteration_destination,iteration_kernel,lifetime_map,8};
    sgpl_iteration_storage_plan_v1 extra={1,sizeof(extra),SGPL_EXEC_R18,3,&p,p.baseline_pair,NULL};
    atomic_store(&fail_malloc,fail);int result=sgpl_try_iteration_storage_v1(g,&p,&extra,0.0,3);atomic_store(&fail_malloc,0);
    if(!result)check(!memcmp(a,original,sizeof a)&&b[0]==0,"allocation refusal precedes destructive source effects");
  }
  printf("PASS lifetime runtime exact comparisons=%u\n",cases);
}
static void count_cases(ExecTestGraph *g) {
  for(int round=0;round<3;++round) {
    int32_t values[7]={INT32_MAX,-1,0,INT32_MIN,7,12,99},want[7];memcpy(want,values,sizeof want);void *slot=values;
    for(int32_t u=0;u<7;++u)for(int64_t e=g->row_ptr[u];e<g->row_ptr[u+1];++e) {
      uint32_t bits;memcpy(&bits,want+u,4);++bits;memcpy(want+u,&bits,4);
    }
    check(sgpl_try_source_count_v1(g,&slot)==1,"row count activates on current CSR");
    check(!memcmp(values,want,sizeof want),"modular counts preserve duplicate/self-loop multiplicity and empty rows");
    setenv("SGPL_NO_ITERATION_LIFETIMES","1",1);check(!sgpl_try_source_count_v1(g,&slot),"row count runtime disable");unsetenv("SGPL_NO_ITERATION_LIFETIMES");
    check(!memcmp(values,want,sizeof want),"disabled row count has no effects");
    --g->n;check(!sgpl_try_source_count_v1(g,&slot),"row count refuses stale topology extent");++g->n;
    int64_t saved=g->row_ptr[3];g->row_ptr[3]=-1;check(!sgpl_try_source_count_v1(g,&slot),"malformed row witness refuses before effects");g->row_ptr[3]=saved;
    check(!memcmp(values,want,sizeof want),"failed witness never partially counts");
    autograph_update_csr_pointers(g,g->row_ptr,g->col_idx);
  }
}
#ifndef SGPL_LIFETIME_TEST_MAIN
#define SGPL_LIFETIME_TEST_MAIN main
#endif
int SGPL_LIFETIME_TEST_MAIN(void) {
  if(previous_simplification_main())return 1;
  int64_t rows[]={0,2,4,5,6,7,8,8};int32_t columns[]={2,1,1,1,1,3,2,2};
  int32_t pairs[]={0,2,0,1,1,1,1,1,2,1,3,3,4,2,5,2};
  ExecTestGraph g={.n=7,.m=8,.row_ptr=rows,.col_idx=columns,.directed=1};native_pairs=pairs;
  autograph_init(&g,7,8,NULL,NULL,pairs);autograph_ensure_layout(&g,7,8,rows,columns,&fixture_nodes,&fixture_edges,pairs,LAYOUT_CSR);autograph_build_clean_cut(&g,6);
  count_cases(&g);lifetime_cases(&g);
  printf("iteration lifetimes runtime: %d failures\n",failures);return failures?1:0;
}
