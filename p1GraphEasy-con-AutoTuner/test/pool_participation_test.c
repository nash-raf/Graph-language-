#include "parallel_runtime.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct {
    atomic_uint seen;
    int width;
    uint64_t cookie;
} Job;
static void lane(void *opaque,int32_t index) {
    Job *j=opaque;
    assert(j->cookie==UINT64_C(0x826495bacde71302));
    assert(index>=0 && index<j->width);
    unsigned bit=1u<<index;
    assert(!(atomic_fetch_or_explicit(&j->seen,bit,memory_order_relaxed)&bit));
    assert(j->cookie==UINT64_C(0x826495bacde71302));
}
int main(void) {
    int max=atoi(getenv("SGPL_NUM_THREADS"));if(max<1)max=1;
    assert(sgpl_run_on_worker_pool(NULL,NULL,0)<0);
    for(int i=0;i<2400;++i) {
        int width=i%3==0?1:i%3==1?max:(max<2?1:2);
        Job j={.seen=0,.width=width,.cookie=UINT64_C(0x826495bacde71302)};
        assert(sgpl_run_on_worker_pool(lane,&j,width)==0);
        assert(atomic_load(&j.seen)==(1u<<width)-1);
        j.cookie=0; /* A completed submission no longer borrows this stack job. */
    }
    puts("PASS alternating narrow/wide pool participation and borrowed-state lifetime");
}
