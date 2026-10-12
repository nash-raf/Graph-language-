/* A compiler-owned host invocation owns this cache. No mutable module globals
 * or automatic conflict tests establish independence. Host access hooks only
 * maintain storage coherence. Uninstrumented clients retain the copy ABI. */
typedef struct gpu_resident {
    uintptr_t lo, hi;
    CUdeviceptr device;
    unsigned char *content;
    int immutable, valid, device_dirty;
    struct gpu_resident *next;
} gpu_resident;
static _Thread_local gpu_resident *gpu_residents;
static _Thread_local unsigned gpu_residency_depth;
typedef struct gpu_language_region {void *base;int64_t bytes;int stack;} gpu_language_region;
static _Thread_local gpu_language_region gpu_resident_regions[GPUB_MAX_BUFFERS];
static _Thread_local unsigned gpu_resident_region_count;
static int gpu_registered_range(uintptr_t lo,uintptr_t hi,uintptr_t *base,uintptr_t *end) {
    int found=0;
    /* Only compiler-registered language allocations have instrumented host
     * accesses. Stack frames and runtime-owned scratch retain eager copies. */
    for(unsigned i=0;i<gpu_resident_region_count&&!found;++i) {
        uintptr_t b=(uintptr_t)gpu_resident_regions[i].base,e=b+gpu_resident_regions[i].bytes;
        if(lo>=b&&hi<=e) {*base=b;*end=e;found=1;}
    }
    return found;
}
void sgpl_gpu_residency_begin(void) {++gpu_residency_depth;}
static void gpu_register_language_buffer(void *base,uint64_t bytes,int stack) {
    if(!gpu_residency_depth||!base||!bytes||bytes>INT64_MAX||bytes>UINTPTR_MAX-(uintptr_t)base) return;
    for(unsigned i=0;i<gpu_resident_region_count;++i) if(gpu_resident_regions[i].base==base) {
        /* A changed allocation extent starts a new storage generation. */
        if((uint64_t)gpu_resident_regions[i].bytes==bytes) return;
        sgpl_gpu_unregister_buffer(base);break;
    }
    if(gpu_resident_region_count<GPUB_MAX_BUFFERS)
        gpu_resident_regions[gpu_resident_region_count++]=(gpu_language_region){base,(int64_t)bytes,stack};
    sgpl_gpu_register_buffer(base,(int64_t)bytes);
}
void sgpl_gpu_register_resident_buffer(void *base,uint64_t bytes) {gpu_register_language_buffer(base,bytes,0);}
void sgpl_gpu_register_stack_buffer(void *base,uint64_t bytes) {gpu_register_language_buffer(base,bytes,1);}
void sgpl_gpu_stack_restore(void *stack_pointer) {
    /* Native CUDA hosts use a downward-growing stack. Only language stack
     * allocations after this save expire. Global arrays remain registered. */
    for(unsigned i=0;i<gpu_resident_region_count;) {
        gpu_language_region *r=&gpu_resident_regions[i];
        if(r->stack&&(uintptr_t)r->base<(uintptr_t)stack_pointer) sgpl_gpu_unregister_buffer(r->base);
        else ++i;
    }
}
void sgpl_gpu_refresh_pointee(const char *name,void *base) {
    uint64_t bytes=gpu_known_extent(base);
    if(bytes&&bytes<=INT64_MAX) sgpl_gpu_register_pointee(name,base,(int64_t)bytes);
}
static void gpu_coherence_failure(void) {
    fputs("GPU host synchronization failed after execution. No CPU replay.\n",stderr);abort();
}
void sgpl_gpu_host_access(void *address,uint64_t bytes,int32_t write) {
    if(!gpu_residency_depth||!address||!bytes) return;
    uintptr_t lo=(uintptr_t)address,hi=bytes>UINTPTR_MAX-lo?UINTPTR_MAX:lo+bytes;
    for(gpu_resident *r=gpu_residents;r;r=r->next) if(!r->immutable&&lo<r->hi&&r->lo<hi) {
        if(r->device_dirty) {
            if(p_cuCtxSetCurrent(g_context)!=CUDA_SUCCESS||p_cuCtxSynchronize()!=CUDA_SUCCESS||
               p_cuMemcpyDtoH((void*)r->lo,r->device,r->hi-r->lo)!=CUDA_SUCCESS) gpu_coherence_failure();
            r->device_dirty=0;
            if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-resident] host-read bytes=%llu\n",(unsigned long long)(r->hi-r->lo));
        }
        if(write) r->valid=0;
    }
}
void sgpl_gpu_host_barrier(void) {
    for(gpu_resident *r=gpu_residents;r;r=r->next) if(!r->immutable)
        sgpl_gpu_host_access((void*)r->lo,r->hi-r->lo,1);
}
void sgpl_gpu_unregister_buffer(void *base) {
    for(unsigned i=0;i<gpu_resident_region_count;) if(gpu_resident_regions[i].base==base)
        gpu_resident_regions[i]=gpu_resident_regions[--gpu_resident_region_count];else ++i;
    gpu_resident **link=&gpu_residents;
    while(*link) {
        gpu_resident *r=*link;
        if(!r->immutable&&r->lo==(uintptr_t)base) {
            if(p_cuCtxSetCurrent(g_context)!=CUDA_SUCCESS||p_cuCtxSynchronize()!=CUDA_SUCCESS) gpu_coherence_failure();
            *link=r->next;p_cuMemFree(r->device);free(r->content);free(r);
        } else link=&r->next;
    }
    pthread_mutex_lock(&g_registry_lock);
    for(int i=0;i<g_num_buffers;) if(g_buffers[i].base==base) g_buffers[i]=g_buffers[--g_num_buffers];else ++i;
    for(int i=0;i<g_num_pointees;) if(g_pointees[i].base==base) g_pointees[i]=g_pointees[--g_num_pointees];else ++i;
    pthread_mutex_unlock(&g_registry_lock);
}
void sgpl_gpu_residency_end(void) {
    if(!gpu_residency_depth||--gpu_residency_depth) return;
    /* Temporarily keep hooks enabled during publication. */
    gpu_residency_depth=1;sgpl_gpu_host_barrier();gpu_residency_depth=0;
    if(g_cuda_available) p_cuCtxSetCurrent(g_context);
    while(gpu_residents) {
        gpu_resident *r=gpu_residents;gpu_residents=r->next;
        p_cuMemFree(r->device);free(r->content);free(r);
    }
    gpu_resident_region_count=0;
}
static gpu_resident *gpu_resident_acquire(uintptr_t lo,uintptr_t hi,int immutable) {
    if(!gpu_residency_depth) return NULL;
    unsigned immutable_entries=0;
    for(gpu_resident *r=gpu_residents;r;r=r->next) {
        immutable_entries+=r->immutable!=0;
        if(r->immutable!=immutable||r->hi-r->lo!=hi-lo) continue;
        if(immutable?((immutable==2||r->lo==lo)&&memcmp(r->content,(void*)lo,hi-lo)==0):r->lo==lo) return r;
    }
    /* Changing topology must not grow an unbounded history of immutable
     * versions. A full cache keeps the normal invocation-owned copy path. */
    if(immutable&&immutable_entries>=32) return NULL;
    gpu_resident *r=calloc(1,sizeof(*r));if(!r) return NULL;
    r->lo=lo;r->hi=hi;r->immutable=immutable;
    if(immutable) {r->content=malloc(hi-lo);if(!r->content){free(r);return NULL;}memcpy(r->content,(void*)lo,hi-lo);}
    if(p_cuMemAlloc(&r->device,hi-lo)!=CUDA_SUCCESS) {free(r->content);free(r);return NULL;}
    r->next=gpu_residents;gpu_residents=r;return r;
}
