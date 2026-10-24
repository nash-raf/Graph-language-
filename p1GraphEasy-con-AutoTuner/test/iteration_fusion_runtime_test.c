#define SGPL_LIFETIME_TEST_MAIN previous_lifetime_main
#include "iteration_lifetime_runtime_test.c"
static atomic_int fusion_maps,fusion_kernels;
static void fusion_map(void *s,sgpl_exec_ctx *c,int64_t hi,const int64_t *rows,const uint8_t *gate,void *out,int64_t lo) {
  atomic_fetch_add(&fusion_maps,1);iteration_map(s,c,hi,rows,gate,out,lo);
}
static void fusion_kernel(void *s,sgpl_exec_ctx *c,int32_t lo,int32_t hi,const int64_t *rows,
    const int32_t *sources,const void *storage,double seed,const int64_t *native) {
  (void)s;check(!c->spatial_source_gate,"fusion requires entailed source gate");atomic_fetch_add(&fusion_kernels,1);
  const double *values=storage;double *out=iteration_destination;
  for(int32_t v=lo;v<hi;++v) {
    double rank=seed;for(int64_t e=rows[v];e<rows[v+1];++e)rank=values[sources[e]]+rank;
    if(native[v+1]>native[v])out[v]=rank/(double)(native[v+1]-native[v]);
  }
}
static void fusion_cases(ExecTestGraph *g) {
  unsigned cases=0;
  for(unsigned data=0;data<5;++data)for(int rounds=1;rounds<=20;rounds+=rounds<3?1:17)
   for(unsigned subset=0;subset<4;++subset)for(unsigned live=0;live<4;++live)for(int mode=0;mode<16;++mode) {
    double initial[]={1e16,-1e16,1.0,-0.0,0.0,INFINITY,NAN},a[7],b[7],want[7];
    if(data==1)initial[5]=0;if(data==2)initial[0]=NAN;if(data==3){initial[0]=-0.0;initial[5]=-INFINITY;}
    if(data==4){uint64_t nan=UINT64_C(0x7ff8000000000123);memcpy(initial+1,&nan,8);}
    memcpy(a,initial,sizeof a);memset(b,0,sizeof b);iteration_source=a;iteration_destination=b;witness_false=mode==10;
    sgpl_iteration_memory_plan_v1 p={1,sizeof(p),(void *)iteration_kernel,witness_gate,&iteration_source,&iteration_destination,iteration_kernel,fusion_map,8};
    check(sgpl_try_iteration_memory_v1(g,&p,-0.0,rounds)==1,"fusion baseline executes");memcpy(want,a,sizeof a);
    memcpy(a,initial,sizeof a);memset(b,0,sizeof b);atomic_store(&fusion_maps,0);atomic_store(&fusion_kernels,0);
    sgpl_iteration_gate_plan_v1 gate={1,sizeof(gate),SGPL_EXEC_R13,0,&p,p.baseline_pair,ungated_kernel,fusion_map};
    unsigned rules=(subset&1?SGPL_EXEC_R17:0)|(subset&2?SGPL_EXEC_R18:0);
    sgpl_iteration_storage_plan_v1 storage={1,sizeof(storage),rules,live,&p,p.baseline_pair,&gate};
    sgpl_iteration_fusion_plan_v1 extra={1,sizeof(extra),SGPL_EXEC_R19,0,&p,p.baseline_pair,&gate,&storage,fusion_kernel};
    if(mode==1)++extra.version;if(mode==2)--extra.bytes;if(mode==3)extra.rules|=SGPL_EXEC_R18;
    if(mode==4)extra.reserved=1;if(mode==5)extra.baseline=NULL;if(mode==6)extra.baseline_pair=NULL;
    if(mode==7)extra.kernel=NULL;if(mode==8)setenv("SGPL_NO_ITERATION_FUSION","1",1);
    if(mode==9)setenv("SGPL_NO_PROOF_REMOVALS","1",1);
    if(mode==11){extra.storage=NULL;extra.gate=NULL;}
    if(mode==12)gate.baseline_pair=NULL;
    if(mode==13)setenv("SGPL_NO_ITERATION_LIFETIMES","1",1);
    if(mode==14)extra.storage=NULL;
    if(mode==15)storage.live_arrays|=4;
    check(sgpl_try_iteration_fusion_v1(g,&p,&extra,-0.0,rounds)==1,"fusion/refusal retains execution");
    unsetenv("SGPL_NO_ITERATION_FUSION");unsetenv("SGPL_NO_PROOF_REMOVALS");unsetenv("SGPL_NO_ITERATION_LIFETIMES");
    int compatible=mode==0||mode>=8;
    unsigned observed=compatible&&(subset&1)&&mode!=11&&mode!=13&&mode!=14&&mode!=15?live:3;
    check(!(observed&1)||!memcmp(a,want,sizeof a),"fusion source exact FP bits");
    check(!(observed&2)||!memcmp(b,want,sizeof b),"fusion destination exact FP bits");
    check(iteration_source==a&&iteration_destination==b,"fusion original identities restored");
    int active=(mode==0||mode==13||mode==14||mode==15);
    check((atomic_load(&fusion_kernels)>0)==(active&&rounds>1),"fusion dispatch requires all descriptor/gate/enable premises");
    if(active)check(atomic_load(&fusion_maps)>0&&atomic_load(&fusion_maps)<=6,"fusion maps only once per invocation");
    ++cases;
   }
  witness_false=0;
  for(int failure=1;failure<=5;++failure) {
    double a[7]={1,2,3,4,5,6,7},b[7]={0},old[7];memcpy(old,a,sizeof a);iteration_source=a;iteration_destination=b;
    sgpl_iteration_memory_plan_v1 p={1,sizeof(p),(void *)iteration_kernel,witness_gate,&iteration_source,&iteration_destination,iteration_kernel,fusion_map,8};
    sgpl_iteration_gate_plan_v1 gate={1,sizeof(gate),SGPL_EXEC_R13,0,&p,p.baseline_pair,ungated_kernel,fusion_map};
    sgpl_iteration_fusion_plan_v1 extra={1,sizeof(extra),SGPL_EXEC_R19,0,&p,p.baseline_pair,&gate,NULL,fusion_kernel};
    atomic_store(&fail_malloc,failure);int ran=sgpl_try_iteration_fusion_v1(g,&p,&extra,-0.0,3);atomic_store(&fail_malloc,0);
    if(!ran)check(!memcmp(a,old,sizeof a)&&b[0]==0,"fusion allocation refusal before effects");
  }
  printf("PASS fusion native exact comparisons=%u\n",cases);
}
int main(void) {
  if(previous_lifetime_main())return 1;
  int64_t rows[]={0,2,4,5,6,7,8,8};int32_t columns[]={2,1,1,1,1,3,2,2};int32_t pairs[]={0,2,0,1,1,1,1,1,2,1,3,3,4,2,5,2};
  ExecTestGraph g={.n=7,.m=8,.row_ptr=rows,.col_idx=columns,.directed=1};native_pairs=pairs;
  autograph_init(&g,7,8,NULL,NULL,pairs);autograph_ensure_layout(&g,7,8,rows,columns,&fixture_nodes,&fixture_edges,pairs,LAYOUT_CSR);autograph_build_clean_cut(&g,6);
  fusion_cases(&g);autograph_update_csr_pointers(&g,rows,columns);fusion_cases(&g);
  printf("iteration fusion runtime failures=%d\n",failures);return failures?1:0;
}
