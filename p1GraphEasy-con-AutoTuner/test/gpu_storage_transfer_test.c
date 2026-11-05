/* Driver stubs test storage/transfer semantics; no device timing or benchmark. */
#define dlsym gpu_test_lookup
#include "../gpu_runtime.c"
#undef dlsym
#include <assert.h>
static int allocations,frees,uploads,downloads,fail_allocation;
static CUresult test_current(CUcontext c){(void)c;return 0;}
static CUresult test_sync(void){return 0;}
static CUresult test_stream(CUstream *s,unsigned f){(void)f;*s=(void*)1;return 0;}
static CUresult test_stream_sync(CUstream s){(void)s;return 0;}
static CUresult test_destroy(CUstream s){(void)s;return 0;}
static CUresult test_alloc(CUdeviceptr *p,size_t n){if(fail_allocation){--fail_allocation;return 2;}void *v=malloc(n);if(!v)return 2;*p=(CUdeviceptr)(uintptr_t)v;++allocations;return 0;}
static CUresult test_free(CUdeviceptr p){if(p){free((void*)(uintptr_t)p);++frees;}return 0;}
static CUresult test_h2d(CUdeviceptr d,const void *s,size_t n){memcpy((void*)(uintptr_t)d,s,n);++uploads;return 0;}
static CUresult test_d2h(void *d,CUdeviceptr s,size_t n){memcpy(d,(void*)(uintptr_t)s,n);++downloads;return 0;}
void *gpu_test_lookup(void *h,const char *n){(void)h;if(!strcmp(n,"cuStreamDestroy_v2")||!strcmp(n,"cuStreamDestroy"))return (void*)test_destroy;return NULL;}
static sgpl_gpu_invocation *reserve(sgpl_gpu_buffer *b,unsigned n) {
  sgpl_gpu_plan p={.version=1,.buffers=b,.buffer_count=n};sgpl_gpu_invocation *s=NULL;
  assert(sgpl_gpu_reserve(&p,&s)==0&&s);return s;
}
static void finish(sgpl_gpu_invocation *s){assert(sgpl_gpu_execute(s)==0);assert(sgpl_gpu_download(s)==0);sgpl_gpu_release(s);}
int main(void) {
  g_cuda_loaded=g_cuda_available=1;g_cached_module=(CUmodule)(uintptr_t)1;
  p_cuCtxSetCurrent=test_current;p_cuCtxSynchronize=test_sync;p_cuStreamCreate=test_stream;p_cuStreamSynchronize=test_stream_sync;
  p_cuMemAlloc=test_alloc;p_cuMemFree=test_free;p_cuMemcpyHtoD=test_h2d;p_cuMemcpyDtoH=test_d2h;
  sgpl_gpu_residency_begin();
  gpu_resident *a=gpu_capacity_acquire(256),*b=gpu_capacity_acquire(128);assert(a&&b&&a!=b&&a->leased&&b->leased);
  gpu_capacity_release(a,1);gpu_resident *c=gpu_capacity_acquire(128);assert(c==a&&c!=b);
  gpu_capacity_release(c,0);gpu_resident *protected=gpu_capacity_acquire(64);assert(protected&&protected!=b);
  gpu_capacity_release(b,1);fail_allocation=1;CUdeviceptr extra=0;
  assert(gpu_device_allocate(&extra,1024)==0&&protected->leased&&gpu_capacity_entries==1);
  test_free(extra);gpu_capacity_release(protected,1);sgpl_gpu_residency_end();assert(gpu_capacity_bytes==0);
  unsigned char data[512];memset(data,7,sizeof(data));
  sgpl_gpu_buffer adjacent[]={{data,256,3},{data+256,256,3}};
  unsetenv("SGPL_GPU_TRANSFER_REDUCE");sgpl_gpu_residency_begin();
  sgpl_gpu_invocation *s=reserve(adjacent,2);assert(s->mirror_count==1);
  assert(gpu_buffer_address(s,1)-gpu_buffer_address(s,0)==256);assert(sgpl_gpu_upload(s)==0);
  ((unsigned char*)(uintptr_t)gpu_buffer_address(s,1))[5]=42;finish(s);assert(data[261]==42);
  int before=allocations;s=reserve(adjacent,2);assert(sgpl_gpu_upload(s)==0);assert(allocations==before);finish(s);
  setenv("SGPL_GPU_FAIL","upload",1);s=reserve(adjacent,2);assert(sgpl_gpu_upload(s)<0);sgpl_gpu_release(s);unsetenv("SGPL_GPU_FAIL");
  sgpl_gpu_residency_end();
  setenv("SGPL_GPU_TRANSFER_REDUCE","0",1);s=reserve(adjacent,2);assert(s->mirror_count==2);assert(sgpl_gpu_upload(s)==0);finish(s);
  unsetenv("SGPL_GPU_TRANSFER_REDUCE");
  adjacent[1].flags=SGPL_GPU_BUFFER_READ;s=reserve(adjacent,2);assert(s->mirror_count==2);sgpl_gpu_release(s);adjacent[1].flags=3;
  sgpl_gpu_buffer unaligned[]={{data,128,3},{data+128,128,3}};s=reserve(unaligned,2);assert(s->mirror_count==2);sgpl_gpu_release(s);
  sgpl_gpu_residency_begin();sgpl_gpu_register_resident_buffer(data,256);sgpl_gpu_register_resident_buffer(data+256,256);
  s=reserve(adjacent,2);assert(s->mirror_count==2);assert(sgpl_gpu_upload(s)==0);finish(s);
  s=reserve(adjacent,2);assert(s->mirrors[0].resident&&s->mirrors[0].resident->valid);
  memset(s->mirrors[0].staging,0xCC,256);before=uploads;
  assert(sgpl_gpu_upload(s)==0&&uploads==before&&s->mirrors[0].staging[0]==0xCC);finish(s);
  sgpl_gpu_unregister_buffer(data);sgpl_gpu_unregister_buffer(data+256);sgpl_gpu_residency_end();
  int values[4]={1,2,3,4};sgpl_gpu_buffer readonly={values,sizeof(values),SGPL_GPU_BUFFER_READ};
  s=reserve(&readonly,1);assert(sgpl_gpu_upload(s)==0);before=downloads;finish(s);assert(downloads==before);
  struct {void *p;} holder={values};sgpl_gpu_buffer fields[4]={{&holder,sizeof(holder),3},{values,sizeof(values),SGPL_GPU_BUFFER_READ}};
  sgpl_gpu_relocation rel[4];uint32_t nb=2,nr=0;
  assert(gpu_pointer_buffer_access(fields,&nb,rel,&nr,0,0,sizeof(values),3)==0);
  assert(fields[1].flags&SGPL_GPU_BUFFER_WRITE);
  assert(gpu_pointer_buffer_access(fields,&nb,rel,&nr,0,0,sizeof(values),SGPL_GPU_BUFFER_READ)==0);
  assert(fields[1].flags&SGPL_GPU_BUFFER_WRITE);
  assert(allocations==frees);
  puts("PASS GPU storage/transfers: exclusive capacity, nested reservations, failure discard, compatible coalescing/alignment, registered boundaries, valid-resident staging, read-only outputs and aliased writers");
}
