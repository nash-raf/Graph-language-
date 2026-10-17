#define _GNU_SOURCE
/* Expose static launch-cost entry points while reusing the DAG link stubs. */
#define main legacy_dag_test_main
#include "dag_scheduler_test.c"
#undef main
#include "../parallel_runtime.c"
static atomic_int tls_errors,tls_calls;
static void tls_probe(void *unused,int32_t lane) {
  (void)unused;
  if(!sgpl_is_worker_pool_thread()||g_tls_worker_index!=lane||g_tls_active_loop_pool||
      g_tls_pending_loop_id!=-1||g_tls_doacross_state||g_tls_doacross_profile_desc||
      g_tls_doacross_sync_lane||g_tls_doacross_wait_count||g_tls_doacross_post_count||
      g_tls_current_tdg_task_slot!=-1||g_tls_desired_threads_min||g_tls_desired_threads_max||
      g_tls_dispatch_depth||g_tls_pending_range_body)atomic_fetch_add(&tls_errors,1);
  if(!lane) {g_tls_pending_loop_id=99;g_tls_doacross_wait_count=99;}
  atomic_fetch_add(&tls_calls,1);
}
int main(int argc,char **argv) {
  if(argc>1&&!strcmp(argv[1],"tls")) {
    sgpl_level_loop_pool level={0};sgpl_doacross_state sync={0};sgpl_loop_profile_desc profile={0};
    g_tls_active_loop_pool=&level;g_tls_pending_loop_id=17;g_tls_doacross_state=&sync;
    g_tls_doacross_profile_desc=&profile;g_tls_doacross_wait_count=7;g_tls_doacross_post_count=8;
    g_tls_current_tdg_task_slot=3;g_tls_desired_threads_min=2;g_tls_desired_threads_max=4;g_tls_dispatch_depth=3;
    sgpl_push_thread_budget(2);
    int rc=sgpl_run_on_worker_pool(tls_probe,NULL,4);
    int ok=!rc&&!atomic_load(&tls_errors)&&atomic_load(&tls_calls)==2&&
        g_tls_active_loop_pool==&level&&g_tls_pending_loop_id==17&&g_tls_doacross_state==&sync&&
        g_tls_doacross_profile_desc==&profile&&g_tls_doacross_wait_count==7&&g_tls_doacross_post_count==8&&
        g_tls_current_tdg_task_slot==3&&g_tls_desired_threads_min==2&&g_tls_desired_threads_max==4&&
        g_tls_dispatch_depth==3&&sgpl_current_thread_budget()==2;
    sgpl_pop_thread_budget();
    puts(ok?"PASS caller DOACROSS/TDG state and parent budget":"FAIL caller TLS restoration");
    return !ok;
  }
  sgpl_loop_profile_desc desc={0};
  desc.runtime_kind=SGPL_RUNTIME_PLAIN;
  desc.mode=SGPL_LOOP_DOACROSS;
  desc.doacross_num_sync_ids=argc>1?2:1;
  sgpl_launch_overhead_detail cost=sgpl_get_launch_overhead_detail(&desc,4);
  if(!isfinite(cost.thread_ns)||cost.thread_ns<=0||!isfinite(cost.total_ns)||cost.total_ns<cost.thread_ns)return 1;
  printf("launch costs: %.2f %.2f\n",cost.thread_ns,cost.path_ns);
  return 0;
}
