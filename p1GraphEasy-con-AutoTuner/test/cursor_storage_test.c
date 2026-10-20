#define _POSIX_C_SOURCE 200809L
#include "../autotuner_runtime.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Exercise the actual scratch allocator without GPU host callbacks. */
static void (*sgpl_gpu_host_access)(void *,uint64_t,int32_t);
static int inject_malloc;
void *__real_malloc(size_t);
void *__wrap_malloc(size_t n) {if(inject_malloc){inject_malloc=0;return NULL;}return __real_malloc(n);}
#include "../execution_scratch_runtime.inc"
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"cursor storage failed at %d: %s\n",__LINE__,#x);return 1;}} while(0)
int main(void) {
  for(int cached=0;cached<2;++cached) {
    sgpl_exec_ctx c={0};if(cached)c.execution_scratch=sgpl_execution_scratch_acquire();
    REQUIRE(!cached||c.execution_scratch);
    void *raw=NULL,*next_raw=NULL;
    unsigned char *view=sgpl_execution_aligned_buffer(&c,SGPL_BUF_PREP_POSITIONS,128,64,1,&raw);
    REQUIRE(view&&raw&&!((uintptr_t)view%64));
    for(int i=0;i<128;++i)REQUIRE(!view[i]);
    memset(view,0x5a,128);
    inject_malloc=1;
    REQUIRE(!sgpl_execution_aligned_buffer(&c,SGPL_BUF_PREP_POSITIONS,4096,64,1,&next_raw)&&!next_raw&&!inject_malloc);
    for(int i=0;i<128;++i)REQUIRE(view[i]==0x5a);
    if(cached) {
      void *same=sgpl_execution_aligned_buffer(&c,SGPL_BUF_PREP_POSITIONS,128,64,1,&next_raw);
      REQUIRE(same==view&&next_raw==raw);
      for(int i=0;i<128;++i)REQUIRE(!view[i]);
    }
    REQUIRE(!sgpl_execution_aligned_buffer(&c,SGPL_BUF_PREP_POSITIONS,SIZE_MAX,64,1,&next_raw)&&!next_raw);
    REQUIRE(!sgpl_execution_aligned_buffer(&c,SGPL_BUF_PREP_POSITIONS,128,3,1,&next_raw)&&!next_raw);
    if(cached)sgpl_execution_scratch_release(c.execution_scratch,0);else free(raw);
  }
  puts("cursor storage failure, overflow and raw-ownership contracts: PASS");return 0;
}
