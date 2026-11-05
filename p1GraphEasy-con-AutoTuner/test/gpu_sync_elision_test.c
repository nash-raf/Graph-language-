/* Deterministic completion/error contracts, without requiring CUDA hardware. */
#include "../gpu_runtime.c"
#include <assert.h>
static int query_result, waits, copies, context_waits;
static CUresult current(CUcontext c) {(void)c;return CUDA_SUCCESS;}
static CUresult query(CUstream s) {(void)s;return query_result;}
static CUresult wait_stream(CUstream s) {(void)s;++waits;return CUDA_SUCCESS;}
static CUresult wait_context(void) {++context_waits;return CUDA_SUCCESS;}
static CUresult copy_back(void *to,CUdeviceptr from,size_t bytes) {
    ++copies;memcpy(to,(void*)(uintptr_t)from,bytes);return CUDA_SUCCESS;
}
static CUresult destroy(CUstream s) {(void)s;return CUDA_SUCCESS;}
static CUresult free_device(CUdeviceptr p) {(void)p;return CUDA_SUCCESS;}
static int32_t host_noop(void *p,int64_t i) {(void)p;(void)i;return 0;}
static int32_t host_fail(void *p,int64_t i) {(void)p;(void)i;return -1;}
static sgpl_gpu_invocation *invocation(int joined,int failed) {
    sgpl_gpu_invocation *s=calloc(1,sizeof(*s));
    s->slots=1;s->streams=calloc(1,sizeof(*s->streams));s->streams[0]=(CUstream)(uintptr_t)1;
    s->stream_destroy=destroy;s->sync_elide=1;s->joined=joined;s->failed=failed;return s;
}
int main(void) {
    p_cuCtxSetCurrent=current;p_cuStreamSynchronize=wait_stream;
    p_cuCtxSynchronize=wait_context;p_cuMemcpyDtoH=copy_back;p_cuMemFree=free_device;
    CUstream stream=(CUstream)(uintptr_t)1;int32_t device_status=0,host_status=-1;
    sgpl_gpu_invocation s={.streams=&stream,.stream_query=query,.sync_elide=1,
        .status=(CUdeviceptr)(uintptr_t)&device_status,.host_status=&host_status};
    query_result=600;assert(gpu_invocation_poll(&s,0)==SGPL_DAG_PENDING&&waits==0&&copies==0);
    query_result=CUDA_SUCCESS;assert(gpu_invocation_poll(&s,0)==0&&waits==0&&copies==1&&host_status==0);
    device_status=-7;assert(gpu_invocation_poll(&s,0)==SGPL_GPU_ERR_EXEC&&waits==0);
    device_status=0;query_result=123;assert(gpu_invocation_poll(&s,0)==SGPL_GPU_ERR_EXEC&&waits==1);
    query_result=CUDA_SUCCESS;s.stream_query=NULL;assert(gpu_invocation_poll(&s,0)==0&&waits==2);
    s.stream_query=query;s.sync_elide=0;assert(gpu_invocation_poll(&s,0)==0&&waits==3);
    s.sync_elide=1;setenv("SGPL_GPU_FAIL","sync:0",1);
    assert(gpu_invocation_poll(&s,0)==SGPL_GPU_ERR_EXEC&&waits==3);unsetenv("SGPL_GPU_FAIL");
    waits=0;sgpl_gpu_release(invocation(1,0));assert(waits==0);
    sgpl_gpu_release(invocation(1,1));assert(waits==1);
    sgpl_gpu_release(invocation(0,0));assert(waits==2);
    sgpl_gpu_invocation *baseline=invocation(1,0);baseline->sync_elide=0;
    sgpl_gpu_release(baseline);assert(waits==3);
    int value=0,device_value=42;
    gpu_resident *r=calloc(1,sizeof(*r));r->lo=(uintptr_t)&value;r->hi=r->lo+sizeof(value);
    r->device=(CUdeviceptr)(uintptr_t)&device_value;r->device_dirty=1;r->quiescent=1;
    gpu_residents=r;gpu_residency_depth=1;unsetenv("SGPL_GPU_SYNC_ELIDE");
    sgpl_gpu_host_access(&value,sizeof(value),0);assert(value==42&&context_waits==0&&!r->device_dirty);
    r->device_dirty=1;r->quiescent=0;sgpl_gpu_host_access(&value,sizeof(value),0);assert(context_waits==1);
    r->device_dirty=1;r->quiescent=1;setenv("SGPL_GPU_SYNC_ELIDE","0",1);
    sgpl_gpu_host_access(&value,sizeof(value),0);assert(context_waits==2);
    unsetenv("SGPL_GPU_SYNC_ELIDE");sgpl_gpu_unregister_buffer(&value);assert(context_waits==2&&!gpu_residents);
    gpu_residency_depth=0;
    gpu_resident nested={.active_uses=1};gpu_mirror mirror={.resident=&nested};
    sgpl_gpu_task task={.host_fn=host_noop};
    sgpl_gpu_invocation inner={.plan={.task_count=1},.tasks=&task,.mirrors=&mirror,.mirror_count=1};
    assert(sgpl_gpu_execute(&inner)==0&&nested.active_uses==1&&!nested.quiescent);
    nested.active_uses=0;inner.started=0;
    assert(sgpl_gpu_execute(&inner)==0&&nested.active_uses==0&&nested.quiescent);
    inner.started=0;task.host_fn=host_fail;
    assert(sgpl_gpu_execute(&inner)<0&&nested.active_uses==0&&!nested.quiescent);
    puts("PASS sync elision: pending, query errors, missing query, device errors, failure drains, dirty publication and control");
}
