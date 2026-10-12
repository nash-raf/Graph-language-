#define SGPL_SPATIAL_FIXTURE_ONLY
#ifdef SGPL_GPU_TEST
#define sgpl_gpu_engine_step_verdict fixture_gpu_verdict
#define sgpl_gpu_step_schedule_ok fixture_gpu_schedule
#define autograph_gpu_step_name_for fixture_gpu_name
#define gpup_step_try fixture_gpu_try
#define gpup_step_v_try fixture_gpu_v_try
#endif
#include "spatial_exec_test.c"
#include "../parallel_runtime.h"
void *joint_graph;
int joint_output[9];
void joint_test(void);
void *load_graph_from_file_fixture(void) { return joint_graph; }
int main(int argc,char **argv) {
  int64_t rp[8]={0,2,4,5,5,6,7,7};int32_t ci[7]={1,2,2,2,3,1,6};
  ExecTestGraph g={.n=7,.m=7,.row_ptr=rp,.col_idx=ci,.directed=1};
  autograph_init(&g,7,7,NULL,NULL,pairs);joint_graph=&g;
  joint_test();
  for(int r=0;r<9;++r) {
    int x[7]={1,2,3,4,5,6,7},old[7]={1,2,3,4,5,6,7};
    if(!strcmp(argv[1],"source_acc")){for(int i=0;i<7;++i)x[i]=0;}
    int frontier=!strcmp(argv[1],"frontier_source")||!strcmp(argv[1],"frontier_destination");
    int bitmap=0;
    for(int e=0;e<(((!strcmp(argv[1],"guard_phi")||!strcmp(argv[1],"unsafe_extent"))&&r==4)?0:7);++e){int u=pairs[2*e],v=pairs[2*e+1];
      if(frontier && (r%2) && u!=0 && u!=4)continue;
      if(frontier)bitmap|=1<<v;
      if((!strcmp(argv[1],"source")||!strcmp(argv[1],"dynamic_scratch")||!strcmp(argv[1],"exit_phi")||!strcmp(argv[1],"guard_phi")||!strcmp(argv[1],"frontier_source")||!strcmp(argv[1],"unsafe_extent")||!strcmp(argv[1],"prefix_phi")))x[u]=v;
      else if(!strcmp(argv[1],"destination")||!strcmp(argv[1],"frontier_destination"))x[v]=u;
      else if(!strcmp(argv[1],"source_acc"))x[u]+=v;
      else x[v]=x[v]+old[u];
    }
    int expected=r+bitmap;
    if(!strcmp(argv[1],"exit_phi")||!strcmp(argv[1],"guard_phi")||!strcmp(argv[1],"unsafe_extent"))expected+=((!strcmp(argv[1],"guard_phi")||!strcmp(argv[1],"unsafe_extent"))&&r==4)?-1:7;for(int i=0;i<7;++i)expected+=x[i];
    check(joint_output[r]==expected,"generated source/destination/snapshot program matches its semantic reference");
  }
  printf("generated %s: %d failures\n",argv[1],failures);return failures?1:0;
}
