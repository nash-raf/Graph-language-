/* Model/profile regressions. No graph-layout autotuner linked. */
#ifndef DOACROSS_RUNTIME_SOURCE
#define DOACROSS_RUNTIME_SOURCE "../../parallel_runtime.c"
#endif
#include DOACROSS_RUNTIME_SOURCE
#include <assert.h>

static sgpl_loop_profile_desc descriptor(int id) {
    return (sgpl_loop_profile_desc){.loop_id=id,.mode=SGPL_LOOP_DOACROSS,
        .has_doacross_profile=1,.doacross_waits_per_iter=1,.doacross_posts_per_iter=1,
        .doacross_num_sync_ids=1,.debug_name="doacross_model_test"};
}

static void sample_body(int64_t i,void *env) {
    (void)i;
    sgpl_record_doacross_sync_sample(env,100*(uint64_t)(sgpl_current_worker_index()+1),50,1,1);
}

static void batching_and_keys(void) {
    sgpl_loop_profile_desc d=descriptor(700);
    sgpl_loop_runtime_state *s=sgpl_get_loop_state(&d);
    sgpl_parallel_launch_plain_ephemeral(0,64,1,sample_body,&d,1,1,2);
    sgpl_doacross_sync_costs two=sgpl_get_doacross_sync_costs(&d,s,64,2);
    assert(two.samples==1 && two.wait_ns==150 && two.post_ns==50);
    assert(s->doacross_sync_samples==1); /* one batch, not 64 racing writes */
    sgpl_parallel_launch_plain_ephemeral(0,64,1,sample_body,&d,1,1,4);
    sgpl_doacross_sync_costs four=sgpl_get_doacross_sync_costs(&d,s,64,4);
    assert(four.samples==1 && four.wait_ns==250 && four.post_ns==50);
    two=sgpl_get_doacross_sync_costs(&d,s,64,2);
    assert(two.wait_ns==150 && two.samples==1);
    sgpl_doacross_sync_costs unseen=sgpl_get_doacross_sync_costs(&d,s,64,3);
    assert(unseen.samples==0 && unseen.seeded_wait && unseen.seeded_post);
    unseen=sgpl_get_doacross_sync_costs(&d,s,20000,2);
    assert(unseen.samples==0); /* different trip-count regime */
    d.doacross_waits_per_iter=2;
    unseen=sgpl_get_doacross_sync_costs(&d,s,64,2);
    assert(unseen.samples==0); /* changed synchronization signature */
    d.doacross_waits_per_iter=1;
    sgpl_run_loop_serial(0,64,1,sample_body,&d,1,1);
    sgpl_doacross_sync_costs one=sgpl_get_doacross_sync_costs(&d,s,64,1);
    assert(one.samples==1 && one.wait_ns==100);
    assert(sgpl_get_doacross_sync_costs(&d,s,64,4).wait_ns==250);

    /* Snapshot stays frozen when later calibration changes the live profile. */
    sgpl_loop_runtime_state frozen=*s;
    sgpl_record_doacross_sync_batch(&d,2,sgpl_trip_count_bucket(64),10000,1000,1,1);
    assert(sgpl_get_doacross_sync_costs(&d,&frozen,64,2).wait_ns==150);
    assert(sgpl_get_doacross_sync_costs(&d,s,64,2).wait_ns>150);

    sgpl_loop_profile_desc evict=descriptor(701);
    for(int p=1;p<=SGPL_DOACROSS_WIDTH_PROFILES+1;++p)
        sgpl_record_doacross_sync_batch(&evict,p,5,100,50,1,1);
    assert(sgpl_get_doacross_sync_costs(&evict,sgpl_get_loop_state(&evict),20000,1).samples==0);
    assert(sgpl_get_doacross_sync_costs(&evict,sgpl_get_loop_state(&evict),20000,
        SGPL_DOACROSS_WIDTH_PROFILES+1).wait_ns==100);
}

static void chooser_and_equation(void) {
    sgpl_loop_profile_desc d=descriptor(702);
    const int64_t n=1000000;
    for(int r=0;r<8;++r)
        sgpl_record_doacross_serial_sample(&d,0,n,1,0,n*100);
    sgpl_loop_runtime_state *s=sgpl_get_loop_state(&d);
    for(int p=2;p<=4;p+=2)
        sgpl_record_doacross_sync_batch(&d,p,5,150,50,1,1);
    double launch=sgpl_get_launch_overhead_detail(&d,4).total_ns;
    double predicted=sgpl_loop_parallel_model_time_ns(&d,s,n,4);
    assert(fabs(predicted-(launch+75000000.0))<.001);
    /* At four workers 75ms+launch beats 100ms serial; at two it cannot. */
    assert(sgpl_should_parallelize_doacross(&d,0,n,1)==1);
    assert(sgpl_should_parallelize_doacross(&d,0,n,1)==1); /* cache hit */
    assert(sgpl_loop_parallel_model_time_ns(&d,s,n,2)>100000000.0);
    sgpl_record_doacross_sync_batch(&d,4,5,3350,50,1,1);
    assert(sgpl_should_parallelize_doacross(&d,0,n,1)==0); /* sync epoch invalidates cache */
    assert(fabs(sgpl_loop_parallel_model_time_ns(&d,s,n,1)-100000000.0)<.001);
    assert(sgpl_debug_reserved_threads()==0);
}

typedef struct { sgpl_loop_profile_desc *desc; int width; } team;
static void *concurrent_team(void *arg) {
    team *t=arg;
    for(int r=0;r<20;++r)
        sgpl_parallel_launch_plain_ephemeral(0,64,1,sample_body,t->desc,1,1,t->width);
    return NULL;
}
static void concurrent_updates(void) {
    sgpl_loop_profile_desc d=descriptor(703);
    team teams[2]={{&d,2},{&d,3}};
    pthread_t threads[2];
    for(int j=0;j<2;++j) assert(pthread_create(&threads[j],NULL,concurrent_team,&teams[j])==0);
    for(int j=0;j<2;++j) assert(pthread_join(threads[j],NULL)==0);
    sgpl_loop_runtime_state *s=sgpl_get_loop_state(&d);
    assert(s->doacross_sync_samples==40);
    assert(sgpl_get_doacross_sync_costs(&d,s,64,2).samples==20);
    assert(sgpl_get_doacross_sync_costs(&d,s,64,3).samples==20);
}

static void planner_cache_inputs(void) {
    sgpl_loop_profile_desc d=descriptor(704);
    sgpl_record_doacross_serial_sample(&d,0,64,1,6400,0);
    sgpl_record_doacross_sync_batch(&d,2,sgpl_trip_count_bucket(64),100,50,1,1);
    sgpl_joint_sync_snapshot old=sgpl_joint_capture_sync(d.loop_id,64,4);
    sgpl_record_doacross_sync_batch(&d,2,sgpl_trip_count_bucket(64),110,50,1,1);
    sgpl_joint_sync_snapshot current=sgpl_joint_capture_sync(d.loop_id,64,4);
    assert(!sgpl_joint_sync_drifted(&current,&old)); /* minor drift keeps cached plan */
    sgpl_record_doacross_sync_batch(&d,8,sgpl_trip_count_bucket(64),1000,50,1,1);
    current=sgpl_joint_capture_sync(d.loop_id,64,4);
    assert(!sgpl_joint_sync_drifted(&current,&old)); /* width outside budget */
    sgpl_record_doacross_sync_batch(&d,3,sgpl_trip_count_bucket(64),100,50,1,1);
    current=sgpl_joint_capture_sync(d.loop_id,64,4);
    assert(sgpl_joint_sync_drifted(&current,&old)); /* newly learned relevant width */
    old=current;
    sgpl_record_doacross_sync_batch(&d,2,sgpl_trip_count_bucket(64),2000,50,1,1);
    current=sgpl_joint_capture_sync(d.loop_id,64,4);
    assert(sgpl_joint_sync_drifted(&current,&old)); /* material drift */
}

int main(void) {
    assert(sgpl_configured_worker_count()==4);
    batching_and_keys();chooser_and_equation();planner_cache_inputs();concurrent_updates();
    puts("PASS width/regime isolation, batch counts, frozen state, eviction, chooser/planner caches, concurrent teams");
    return 0;
}
