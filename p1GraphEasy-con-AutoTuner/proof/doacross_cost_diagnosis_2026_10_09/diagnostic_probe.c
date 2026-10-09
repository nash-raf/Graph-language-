/* Direct production DOACROSS cost-model validation, not a TDG allocation test.
 * Includes unchanged runtime to query its exact private equation. Kernels carry
 * real wait/post dependencies. Profile boundaries reproduce serial block timing
 * and the per-iteration profile wrapper used by the loop outliner.
 */
#ifndef DOACROSS_RUNTIME_SOURCE
#define DOACROSS_RUNTIME_SOURCE "../../parallel_runtime.c"
#endif
#include DOACROSS_RUNTIME_SOURCE
#include <assert.h>


/* Test-only observations. All production synchronization stays unchanged. */
static atomic_long diag_iter[128],diag_last_post[128],diag_max_post[SYNC_WINDOW];
static atomic_int diag_phase[128],diag_active,diag_run;
static _Atomic(sgpl_doacross_state *) diag_state;
static atomic_ullong diag_started;
static int diag_replay(void) {
    sgpl_doacross_state *s=sgpl_alloc_doacross_state(3,1);
    assert(s);doacross_init_state(s,3);
    doacross_post_state(s,7932,0);
    long before=atomic_load(sgpl_doacross_slot(s,0,7932%SYNC_WINDOW));
    doacross_post_state(s,3836,0);
    long after=atomic_load(sgpl_doacross_slot(s,0,7932%SYNC_WINDOW));
    printf("dependency=7932 slot=%d after_post_7932=%ld after_post_3836=%ld wait_predicate=%d\n",
        7932%SYNC_WINDOW,before,after,after<7932);
    assert(before==7932 && after==3836 && after<7932);
    sgpl_free_doacross_state(s);return 0;
}
static void *diag_watchdog(void *unused) {
    (void)unused;
    for (;;) {
        usleep(100000);
        if (!atomic_load(&diag_active)) continue;
        if (sgpl_now_ns()-atomic_load(&diag_started)<3000000000ULL) continue;
        sgpl_doacross_state *s=atomic_load(&diag_state);
        fprintf(stderr,"STALL run=%d state=%p window=%d\n",atomic_load(&diag_run),(void*)s,s?s->sync_window:0);
        if (s) for (int t=0;t<sgpl_configured_worker_count();++t) {
            long i=atomic_load(&diag_iter[t]),dep=i-3;
            int slot=dep%s->sync_window;
            fprintf(stderr,"lane=%d phase=%d iteration=%ld dependency=%ld slot=%d actual_slot=%ld maximum_posted=%ld last_lane_post=%ld\n",
                t,atomic_load(&diag_phase[t]),i,dep,slot,
                atomic_load(sgpl_doacross_slot(s,0,slot)),atomic_load(&diag_max_post[slot]),atomic_load(&diag_last_post[t]));
        }
        fflush(stderr);_Exit(90);
    }
}

typedef struct {
    int64_t trips;
    int distance, dependent_steps, independent_steps, after_post, streams;
    double *a, *b, *independent;
    sgpl_loop_profile_desc desc;
} kernel;

static double compute(double x,int steps) {
    for(int k=0;k<steps;++k) x=x*.99991 + 1.0/(1.0+x*x+k);
    return x;
}

static void independent_part(int64_t i,kernel *k) {
    k->independent[i]=compute((double)(i%97+1)*.01,k->independent_steps);
}

static void dependent_part(int64_t i,kernel *k) {
    k->a[i]=compute(k->a[i-k->distance]*.5+3.0+(double)(i%17)*.001,k->dependent_steps);
    if(k->streams==2) k->b[i]=compute(k->b[i-k->distance]*.25+2.0+(double)(i%19)*.001,k->dependent_steps);
}

static void body(int64_t i,void *argument) {
    kernel *k=argument;
    int t=sgpl_current_worker_index();
    atomic_store(&diag_state,g_tls_doacross_state);
    atomic_store(&diag_iter[t],i);
    atomic_store(&diag_phase[t],1);
    sgpl_doacross_profile_enter(&k->desc);
    if(k->independent_steps && !k->after_post) independent_part(i,k);
    atomic_store(&diag_phase[t],2);
    doacross_wait(i,k->distance,0);
    atomic_store(&diag_phase[t],3);
    if(k->streams==2) doacross_wait(i,k->distance,1);
    dependent_part(i,k);
    doacross_post(i,0);
    atomic_store(&diag_last_post[t],i);
    long old=atomic_load(&diag_max_post[i%SYNC_WINDOW]);
    while(old<i && !atomic_compare_exchange_weak(&diag_max_post[i%SYNC_WINDOW],&old,i)) {}
    atomic_store(&diag_phase[t],4);
    if(k->streams==2) doacross_post(i,1);
    if(k->independent_steps && k->after_post) independent_part(i,k);
    sgpl_doacross_profile_exit(&k->desc);
}

static void reset(kernel *k) {
    memset(k->a,0,(size_t)(k->trips+k->distance)*sizeof(double));
    memset(k->b,0,(size_t)(k->trips+k->distance)*sizeof(double));
    memset(k->independent,0,(size_t)(k->trips+k->distance)*sizeof(double));
    for(int i=0;i<k->distance;++i) k->a[i]=k->b[i]=(i%7+1)*.01;
}

static uint64_t serial_profile(kernel *k) {
    uint64_t dep=0,ind=0,begin=sgpl_now_ns();
    for(int64_t i=k->distance;i<k->trips+k->distance;++i) {
        if(k->independent_steps && !k->after_post) {
            uint64_t t=sgpl_now_ns(); independent_part(i,k); ind+=sgpl_now_ns()-t;
        }
        uint64_t t=sgpl_now_ns(); dependent_part(i,k); dep+=sgpl_now_ns()-t;
        if(k->independent_steps && k->after_post) {
            t=sgpl_now_ns(); independent_part(i,k); ind+=sgpl_now_ns()-t;
        }
    }
    uint64_t elapsed=sgpl_now_ns()-begin;
    sgpl_record_doacross_serial_sample(&k->desc,k->distance,k->distance+k->trips,1,dep,ind);
    return elapsed;
}

static void verify(kernel *k,const double *expected_a,const double *expected_b,const double *expected_ind,int p) {
    (void)p;
    for(int64_t i=0;i<k->trips+k->distance;++i) {
        assert(fabs(k->a[i]-expected_a[i])<=1e-12*fmax(1.0,fabs(expected_a[i])));
        if(k->streams==2) assert(fabs(k->b[i]-expected_b[i])<=1e-12*fmax(1.0,fabs(expected_b[i])));
        if(k->independent_steps) assert(fabs(k->independent[i]-expected_ind[i])<=1e-12*fmax(1.0,fabs(expected_ind[i])));
    }
    assert(sgpl_debug_reserved_threads()==0);
}

static uint64_t parallel_run(kernel *k) {
    assert(sgpl_dispatch_thread_request(sgpl_loop_effective_decision_threads(),k->trips)==sgpl_configured_worker_count());
    sgpl_set_pending_loop_id(k->desc.loop_id);
    for(int t=0;t<128;++t) { atomic_store(&diag_phase[t],0);atomic_store(&diag_iter[t],0);atomic_store(&diag_last_post[t],-1); }
    for(int slot=0;slot<SYNC_WINDOW;++slot) atomic_store(&diag_max_post[slot],-1);
    atomic_fetch_add(&diag_run,1);
    atomic_store(&diag_started,sgpl_now_ns());atomic_store(&diag_active,1);
    uint64_t begin=sgpl_now_ns();
    /* Direct dispatch lets us measure parallel service even if the per-loop
     * chooser would prefer serial. It does not change the predicted equation. */
    parallel_for_runtime(k->distance,k->distance+k->trips,1,body,k,1,k->streams);
    atomic_store(&diag_active,0);
    return sgpl_now_ns()-begin;
}

static void emit(const char *stage,int sample,int p,kernel *k,const sgpl_loop_runtime_state *state,
                 double prediction,uint64_t actual,double launch) {
    double sw=state->doacross_sync_samples && state->sigma_wait_ns_ewma>0 ?
        state->sigma_wait_ns_ewma:SGPL_DOACROSS_SIGMA_WAIT_SEED_NS;
    double sp=state->doacross_sync_samples && state->sigma_post_ns_ewma>0 ?
        state->sigma_post_ns_ewma:SGPL_DOACROSS_SIGMA_POST_SEED_NS;
    printf("%s,%d,%d,%lld,%d,%d,%d,%d,%d,%.9f,%llu,%.9f,%.9f,%.9f,%.9f,%.9f,%u\n",
        stage,sample,p,(long long)k->trips,k->distance,k->dependent_steps,k->independent_steps,
        k->after_post,k->streams,prediction,(unsigned long long)actual,
        state->c_dep_ns_per_iter_ewma,state->c_ind_ns_per_iter_ewma,sw,sp,launch,state->doacross_sync_samples);
}

int main(int argc,char **argv) {
    if(argc==2 && strcmp(argv[1],"--slot-replay")==0) return diag_replay();
    assert(argc==8);
    kernel k={.trips=atoll(argv[1]),.distance=atoi(argv[2]),.dependent_steps=atoi(argv[3]),
        .independent_steps=atoi(argv[4]),.after_post=atoi(argv[5]),.streams=atoi(argv[6])};
    pthread_t watcher;assert(pthread_create(&watcher,NULL,diag_watchdog,NULL)==0);
    int samples=atoi(argv[7]),p=sgpl_configured_worker_count();
    assert(k.trips>p && k.distance>0 && k.distance<SYNC_WINDOW && p>=2 && p<=128 && samples>=3);
    assert(k.streams==1 || k.streams==2);
    size_t bytes=(size_t)(k.trips+k.distance)*sizeof(double);
    k.a=malloc(bytes);k.b=malloc(bytes);k.independent=malloc(bytes);assert(k.a && k.b && k.independent);
    k.desc=(sgpl_loop_profile_desc){.loop_id=620,.mode=SGPL_LOOP_DOACROSS,
        .has_doacross_profile=1,.doacross_waits_per_iter=k.streams,.doacross_posts_per_iter=k.streams,
        .doacross_num_sync_ids=k.streams,.debug_name="doacross_cost_validation"};
    uint64_t last_serial=0;
    for(int r=0;r<8;++r) { reset(&k);last_serial=serial_profile(&k); }
    sgpl_loop_runtime_state *state=sgpl_get_loop_state(&k.desc);
    assert(state && state->c_sampling_state==SGPL_C_SAMPLING_STABLE && state->doacross_sync_samples==0);
    double *ea=malloc(bytes),*eb=malloc(bytes),*ei=malloc(bytes);assert(ea && eb && ei);
    memcpy(ea,k.a,bytes);memcpy(eb,k.b,bytes);memcpy(ei,k.independent,bytes);
    sgpl_loop_runtime_state seeded=*state;
    double launch=sgpl_get_launch_overhead_detail(&k.desc,p).total_ns;
    double seeded_prediction=sgpl_loop_parallel_model_time_ns(&k.desc,&seeded,k.trips,p);
    for(int r=0;r<4;++r) { reset(&k);parallel_run(&k);verify(&k,ea,eb,ei,p); }
    sgpl_loop_runtime_state trained=*state;
    assert(trained.doacross_sync_samples>0);
    double trained_prediction=sgpl_loop_parallel_model_time_ns(&k.desc,&trained,k.trips,p);
    assert(isfinite(seeded_prediction) && seeded_prediction>0 && isfinite(trained_prediction) && trained_prediction>0);
    for(int s=0;s<samples;++s) {
        reset(&k);uint64_t elapsed=parallel_run(&k);verify(&k,ea,eb,ei,p);
        emit("seeded-sync",s,p,&k,&seeded,seeded_prediction,elapsed,launch);
        emit("learned-sync",s,p,&k,&trained,trained_prediction,elapsed,launch);
    }
    assert(atomic_load(&g_joint_plans)==0);
    fprintf(stderr,"PASS full-output, real dependencies, workers=%d, serial_wall_ns=%llu, serial_samples=%u, frozen_training_sync_samples=%u, heldout=%d, no_TDG_plans\n",
        p,(unsigned long long)last_serial,state->doacross_serial_samples,trained.doacross_sync_samples,samples);
    free(k.a);free(k.b);free(k.independent);free(ea);free(eb);free(ei);
    return 0;
}
