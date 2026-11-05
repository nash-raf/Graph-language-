/* Diagnostic build only. Included after the original CUDA pointer declarations. */
#include "gpu_profile_scope.h"
static const char *gp_names[GP_COUNT]={"other_host","cpu_preparation","cpu_finalize",
 "plan_and_descriptors","resident_cache","upload_staging","download_publication",
 "host_coherence","cleanup","execute_wait_and_scheduler","poll_cpu","cuda_init",
 "cuda_module_and_lookup","device_allocation","device_free","host_to_device",
 "device_to_host","status_download","stream_management","context_set","stream_query",
 "explicit_sync","launch_api","profiler_events","prep_clean_cut","prep_records",
 "prep_snapshot","prep_context","prep_frontier","prep_begin","prep_publish"};
static uint64_t gp_total[GP_COUNT],gp_region[GP_COUNT],gp_calls[GP_COUNT],gp_rcalls[GP_COUNT];
static uint64_t gp_bytes[GP_COUNT],gp_rbytes[GP_COUNT],gp_last,gp_start,gp_cstart,gp_cstop;
static int gp_enabled,gp_current=GP_HOST,gp_in_compute,gp_timer_calls;
static uint64_t gp_other_thread_entries,gp_first_launch_ns,gp_launch_count;
static uint64_t gp_other_buckets[GP_COUNT];
static pthread_t gp_owner;
static uint64_t gp_now(void) {struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint64_t)t.tv_sec*1000000000+t.tv_nsec;}
static void gp_charge(uint64_t now) {
    if(gp_last){uint64_t delta=now-gp_last;gp_total[gp_current]+=delta;if(gp_in_compute)gp_region[gp_current]+=delta;}
    gp_last=now;
}
gp_scope gp_enter(int bucket) {
    gp_scope s={0,0};if(!gp_enabled)return s;
    if(!pthread_equal(pthread_self(),gp_owner)){
        __atomic_fetch_add(&gp_other_thread_entries,1,__ATOMIC_RELAXED);
        __atomic_fetch_add(&gp_other_buckets[bucket],1,__ATOMIC_RELAXED);return s;
    }
    gp_charge(gp_now());s.previous=gp_current;s.active=1;gp_current=bucket;
    ++gp_calls[bucket];if(gp_in_compute)++gp_rcalls[bucket];return s;
}
void gp_leave(gp_scope *s) {if(s->active){gp_charge(gp_now());gp_current=s->previous;}}
void gp_bytes_add(int bucket,uint64_t bytes) {
    if(gp_enabled&&pthread_equal(pthread_self(),gp_owner)) {
        gp_bytes[bucket]+=bytes;if(gp_in_compute)gp_rbytes[bucket]+=bytes;
    }
}
double __wrap_timer_runtime(void) {
    uint64_t now=gp_now();
    if(gp_enabled&&pthread_equal(pthread_self(),gp_owner)) {
        gp_charge(now);
        if(gp_timer_calls==0){gp_cstart=now;gp_in_compute=1;}
        else if(gp_timer_calls==1){gp_cstop=now;gp_in_compute=0;}
        ++gp_timer_calls;
    }
    return (double)now*1e-9;
}
typedef CUresult (*gp_event_create_fn)(void **,unsigned);
typedef CUresult (*gp_event_record_fn)(void *,CUstream);
typedef CUresult (*gp_event_elapsed_fn)(float *,void *,void *);
typedef CUresult (*gp_event_destroy_fn)(void *);
static gp_event_create_fn gp_event_create;
static gp_event_record_fn gp_event_record;
static gp_event_elapsed_fn gp_event_elapsed;
static gp_event_destroy_fn gp_event_destroy;
typedef struct {void *start,*end;CUstream stream;int done,region;} gp_event_pair;
static gp_event_pair gp_events[8192];
static unsigned gp_event_count,gp_event_collected,gp_event_errors;
static double gp_device_ms,gp_device_region_ms;
static int gp_events_requested;
static int gp_event_init(void) {
    if(gp_event_create)return gp_event_record&&gp_event_elapsed&&gp_event_destroy;
    *(void**)&gp_event_create=dlsym(g_cuda_lib,"cuEventCreate");
    *(void**)&gp_event_record=dlsym(g_cuda_lib,"cuEventRecord");
    *(void**)&gp_event_elapsed=dlsym(g_cuda_lib,"cuEventElapsedTime");
    *(void**)&gp_event_destroy=dlsym(g_cuda_lib,"cuEventDestroy_v2");
    if(!gp_event_destroy)*(void**)&gp_event_destroy=dlsym(g_cuda_lib,"cuEventDestroy");
    return gp_event_create&&gp_event_record&&gp_event_elapsed&&gp_event_destroy;
}
static void gp_event_collect(CUstream stream) {
    if(!gp_events_requested)return;
    GP_SCOPE(GP_INSTRUMENT);
    for(unsigned i=0;i<gp_event_count;++i) {
        gp_event_pair *e=&gp_events[i];if(e->done||e->stream!=stream)continue;
        float ms=0;CUresult rc=gp_event_elapsed(&ms,e->start,e->end);
        if(rc==0){gp_device_ms+=ms;if(e->region)gp_device_region_ms+=ms;++gp_event_collected;}
        else ++gp_event_errors;
        gp_event_destroy(e->start);gp_event_destroy(e->end);e->done=1;
    }
}
static CUresult gp_launch(CUfunction f,unsigned gx,unsigned gy,unsigned gz,unsigned bx,unsigned by,unsigned bz,
 unsigned shared,CUstream stream,void **args,void **extra) {
    if(!pthread_equal(pthread_self(),gp_owner))return GP_CALL(GP_LAUNCH,p_cuLaunchKernel(f,gx,gy,gz,bx,by,bz,shared,stream,args,extra));
    GP_SCOPE(GP_INSTRUMENT);
    gp_event_pair *e=NULL;
    if(gp_events_requested&&gp_event_count<8192&&gp_event_init()) {
        e=&gp_events[gp_event_count++];e->stream=stream;e->region=gp_in_compute;
        if(gp_event_create(&e->start,0)||gp_event_create(&e->end,0)||gp_event_record(e->start,stream)){++gp_event_errors;e=NULL;}
    }
    uint64_t begin=gp_now();
    CUresult rc=GP_CALL(GP_LAUNCH,p_cuLaunchKernel(f,gx,gy,gz,bx,by,bz,shared,stream,args,extra));
    uint64_t elapsed=gp_now()-begin;if(!gp_launch_count)gp_first_launch_ns=elapsed;++gp_launch_count;
    if(e&&gp_event_record(e->end,stream))++gp_event_errors;
    return rc;
}
static CUresult gp_status_download(void *to,CUdeviceptr from,size_t bytes) {
    GP_SCOPE(GP_STATUS);gp_bytes_add(GP_STATUS,bytes);return p_cuMemcpyDtoH(to,from,bytes);
}
extern int __real_main(int,char **);
int __wrap_main(int argc,char **argv) {
    gp_owner=pthread_self();gp_enabled=1;gp_start=gp_last=gp_now();
    gp_events_requested=getenv("SGPL_PROFILE_EVENTS")!=NULL;
    int rc=__real_main(argc,argv);gp_charge(gp_now());uint64_t end=gp_last;gp_enabled=0;
    const char *path=getenv("SGPL_PROFILE_JSON");FILE *f=path?fopen(path,"w"):NULL;
    if(f) {
        fprintf(f,"{\"main_ms\":%.9f,\"compute_ms\":%.9f,\"timer_calls\":%d,\"other_thread_entries\":%llu,\"first_launch_api_ms\":%.9f,\"launches\":%llu,\"event_ms\":%.9f,\"event_compute_ms\":%.9f,\"event_count\":%u,\"event_collected\":%u,\"event_errors\":%u,\"buckets\":{",
          (end-gp_start)*1e-6,(gp_cstop-gp_cstart)*1e-6,gp_timer_calls,(unsigned long long)gp_other_thread_entries,
          gp_first_launch_ns*1e-6,(unsigned long long)gp_launch_count,gp_device_ms,gp_device_region_ms,gp_event_count,gp_event_collected,gp_event_errors);
        for(int i=0;i<GP_COUNT;++i)fprintf(f,"%s\"%s\":{\"main_ms\":%.9f,\"compute_ms\":%.9f,\"calls\":%llu,\"compute_calls\":%llu,\"bytes\":%llu,\"compute_bytes\":%llu}",i?",":"",gp_names[i],gp_total[i]*1e-6,gp_region[i]*1e-6,(unsigned long long)gp_calls[i],(unsigned long long)gp_rcalls[i],(unsigned long long)gp_bytes[i],(unsigned long long)gp_rbytes[i]);
        fprintf(f,"},\"other_thread_buckets\":{");
        for(int i=0;i<GP_COUNT;++i)fprintf(f,"%s\"%s\":%llu",i?",":"",gp_names[i],(unsigned long long)gp_other_buckets[i]);
        fprintf(f,"}}\n");fclose(f);
    }
    return rc;
}
/* Function-like macros preserve LOAD_SYM's pointer assignments. */
#define p_cuInit(...) GP_CALL(GP_INIT,p_cuInit(__VA_ARGS__))
#define p_cuDeviceGetCount(...) GP_CALL(GP_INIT,p_cuDeviceGetCount(__VA_ARGS__))
#define p_cuDeviceGet(...) GP_CALL(GP_INIT,p_cuDeviceGet(__VA_ARGS__))
#define p_cuCtxCreate(...) GP_CALL(GP_INIT,p_cuCtxCreate(__VA_ARGS__))
#define p_cuCtxSetCurrent(...) GP_CALL(GP_CONTEXT,p_cuCtxSetCurrent(__VA_ARGS__))
#define p_cuModuleLoadDataEx(...) GP_CALL(GP_MODULE,p_cuModuleLoadDataEx(__VA_ARGS__))
#define p_cuModuleGetFunction(...) GP_CALL(GP_MODULE,p_cuModuleGetFunction(__VA_ARGS__))
#define p_cuModuleGetGlobal(...) GP_CALL(GP_MODULE,p_cuModuleGetGlobal(__VA_ARGS__))
#define p_cuMemAlloc(P,N) GP_CALL(GP_ALLOC,(gp_bytes_add(GP_ALLOC,N),p_cuMemAlloc(P,N)))
#define p_cuMemFree(...) GP_CALL(GP_FREE,p_cuMemFree(__VA_ARGS__))
#define p_cuMemcpyHtoD(D,S,N) GP_CALL(GP_H2D,(gp_bytes_add(GP_H2D,N),p_cuMemcpyHtoD(D,S,N)))
#define p_cuMemcpyDtoH(D,S,N) GP_CALL(GP_D2H,(gp_bytes_add(GP_D2H,N),p_cuMemcpyDtoH(D,S,N)))
#define p_cuLaunchKernel(...) gp_launch(__VA_ARGS__)
#define p_cuCtxSynchronize(...) GP_CALL(GP_SYNC,p_cuCtxSynchronize(__VA_ARGS__))
#define p_cuStreamSynchronize(...) GP_CALL(GP_SYNC,p_cuStreamSynchronize(__VA_ARGS__))
#define p_cuStreamCreate(...) GP_CALL(GP_STREAM,p_cuStreamCreate(__VA_ARGS__))
