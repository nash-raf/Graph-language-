/* Invocation-owned CUDA storage. No data in the legacy step caches is used. */
extern int32_t autograph_validate_dag(const sgpl_dag_template *) __attribute__((weak));
extern int32_t autograph_execute_dag_async(const sgpl_dag_template *,void *,sgpl_dag_node_fn,sgpl_dag_node_fn) __attribute__((weak));
extern int32_t sgpl_round_spatial_host_call(sgpl_round_work_fn,void *,int64_t) __attribute__((weak));
extern int32_t sgpl_exec_spatial_begin(sgpl_exec_ctx *) __attribute__((weak));
extern int32_t sgpl_exec_spatial_publish(sgpl_exec_ctx *) __attribute__((weak));
extern int32_t sgpl_exec_spatial_finish(sgpl_exec_ctx *) __attribute__((weak));
#ifndef SGPL_GPU_MAX_TASKS
#define SGPL_GPU_MAX_TASKS 65536
#endif
static int32_t gpu_check_noop(void *state, int64_t index) { (void)state; (void)index; return 0; }
typedef CUresult (*sgpl_stream_destroy_fn)(CUstream);
typedef CUresult (*sgpl_stream_query_fn)(CUstream);
typedef struct gpu_mirror {
    uintptr_t lo, hi;
    uint32_t flags;
    CUdeviceptr device;
    uint8_t *staging;
    gpu_resident *resident;
} gpu_mirror;
struct sgpl_gpu_invocation {
    sgpl_gpu_plan plan;
    sgpl_gpu_buffer *buffers;
    sgpl_gpu_relocation *relocations;
    void **original_pointers;
    sgpl_gpu_task *tasks;
    sgpl_dag_relation_desc *relations;
    gpu_mirror *mirrors;
    uint32_t mirror_count;
    uint32_t *mapping;
    CUfunction *functions;
    CUstream *streams;
    int32_t slots;
    CUdeviceptr status;
    int32_t *host_status;
    sgpl_stream_destroy_fn stream_destroy;
    sgpl_stream_query_fn stream_query;
    int uploaded, started, joined, failed;
    sgpl_gpu_graph *owned_graphs;
    uint32_t owned_graph_count;
};

static int gpu_fault(const char *phase, int64_t index) {
    const char *fault = getenv("SGPL_GPU_FAIL");
    if (!fault) return 0;
    if (!strcmp(fault, phase)) return 1;
    char name[64]; snprintf(name, sizeof(name), "%s:%lld", phase, (long long)index);
    return !strcmp(fault, name);
}
static CUdeviceptr gpu_buffer_address(sgpl_gpu_invocation *s, uint32_t b) {
    gpu_mirror *m = &s->mirrors[s->mapping[b]];
    return m->device + (uintptr_t)s->buffers[b].host - m->lo;
}
void sgpl_gpu_release(sgpl_gpu_invocation *s) {
    if (!s) return;
    if (g_cuda_available) p_cuCtxSetCurrent(g_context);
    if (s->streams) for (int32_t i=0;i<s->slots;++i) if (s->streams[i]) {
        p_cuStreamSynchronize(s->streams[i]);
        if (s->stream_destroy) s->stream_destroy(s->streams[i]);
    }
    for (uint32_t i=0;i<s->mirror_count;++i) {
        if (s->mirrors[i].resident) {
            if(s->failed) {s->mirrors[i].resident->valid=0;s->mirrors[i].resident->device_dirty=0;}
        } else if (s->mirrors[i].device) p_cuMemFree(s->mirrors[i].device);
        free(s->mirrors[i].staging);
    }
    if (s->status) p_cuMemFree(s->status);
    for(uint32_t i=0;i<s->owned_graph_count;++i) {free(s->owned_graphs[i].rows);free(s->owned_graphs[i].columns);}
    free(s->owned_graphs);
    free(s->buffers); free(s->relocations); free(s->original_pointers); free(s->tasks); free(s->relations);
    free(s->mirrors); free(s->mapping); free(s->functions);
    free(s->streams); free(s->host_status);
    free(s);
}

int32_t sgpl_gpu_reserve(const sgpl_gpu_plan *p, sgpl_gpu_invocation **out) {
    if (out) *out=NULL;
    if (!out || !p || p->version!=SGPL_GPU_PLAN_VERSION || p->task_count<0 ||
        p->task_count>SGPL_GPU_MAX_TASKS || p->relation_count<0 ||
        (p->buffer_count && !p->buffers) || (p->relocation_count && !p->relocations) ||
        (p->task_count && !p->tasks) || (p->relation_count && !p->relations)) return SGPL_GPU_ERR_INVALID;
    /* A malformed DAG is rejected before any device launch. */
    if (p->task_count) {
        sgpl_dag_template t={0};
        t.node_count=p->task_count; t.relations=p->relations; t.relation_count=p->relation_count;
        t.node_fn=gpu_check_noop;
        if (!autograph_validate_dag) return SGPL_GPU_UNAVAILABLE;
        int validation=autograph_validate_dag(&t);
        if(validation<0) return validation==SGPL_DAG_ERR_NOMEM?SGPL_GPU_UNAVAILABLE:SGPL_GPU_ERR_INVALID;
    }
    if(gpu_fault("driver",0)) {if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-capability] device unavailable (injected); CPU before effects\n");return SGPL_GPU_UNAVAILABLE;}
    if (!load_cuda() || !p_cuStreamCreate || !p_cuStreamSynchronize || !autograph_execute_dag_async) {
        if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-capability] driver, device, stream API, or DAG runtime unavailable; CPU before effects\n");return SGPL_GPU_UNAVAILABLE;
    }
    if (p_cuCtxSetCurrent(g_context)!=CUDA_SUCCESS) return SGPL_GPU_UNAVAILABLE;
    CUmodule module=gpu_fault("module",0)?NULL:load_module(); if (!module) return SGPL_GPU_UNAVAILABLE;
    sgpl_gpu_invocation *s=calloc(1,sizeof(*s)); if (!s) return SGPL_GPU_ERR_ALLOC;
    s->plan=*p;
    /* One stream per device task. CUDA controls execution resources. Legacy
     * worker/block budget fields are compatibility data and have no device use. */
    s->slots=p->task_count;
#define GPU_COPY(F,N) do { if ((N)) { s->F=malloc((size_t)(N)*sizeof(*s->F)); if(!s->F) goto alloc_fail; memcpy(s->F,p->F,(size_t)(N)*sizeof(*s->F)); } } while(0)
    GPU_COPY(buffers,p->buffer_count); GPU_COPY(relocations,p->relocation_count);
    GPU_COPY(tasks,p->task_count); GPU_COPY(relations,p->relation_count);
#undef GPU_COPY
    s->mirrors=calloc(p->buffer_count?p->buffer_count:1,sizeof(*s->mirrors));
    s->mapping=calloc(p->buffer_count?p->buffer_count:1,sizeof(*s->mapping));
    s->functions=calloc(p->task_count?p->task_count:1,sizeof(*s->functions));
    s->streams=calloc(s->slots?s->slots:1,sizeof(*s->streams));
    s->host_status=calloc(p->task_count?p->task_count:1,sizeof(int32_t));
    s->original_pointers=calloc(p->relocation_count?p->relocation_count:1,sizeof(void*));
    if (!s->mirrors||!s->mapping||!s->functions||!s->streams||!s->host_status||!s->original_pointers) goto alloc_fail;
    *(void **)(&s->stream_destroy)=dlsym(g_cuda_lib,"cuStreamDestroy_v2");
    if (!s->stream_destroy) *(void **)(&s->stream_destroy)=dlsym(g_cuda_lib,"cuStreamDestroy");
    if (!s->stream_destroy) { sgpl_gpu_release(s); return SGPL_GPU_UNAVAILABLE; }
    *(void **)(&s->stream_query)=dlsym(g_cuda_lib,"cuStreamQuery");
    for (uint32_t i=0;i<p->buffer_count;++i) {
        uintptr_t lo=(uintptr_t)p->buffers[i].host;
        uint64_t bytes=p->buffers[i].bytes;
        if (!lo || !bytes || bytes>SIZE_MAX || bytes>UINTPTR_MAX-lo ||
            (p->buffers[i].flags & ~(SGPL_GPU_BUFFER_READ|SGPL_GPU_BUFFER_WRITE|SGPL_GPU_BUFFER_LAYOUT))||
            ((p->buffers[i].flags&SGPL_GPU_BUFFER_LAYOUT)&&(p->buffers[i].flags&SGPL_GPU_BUFFER_WRITE))) goto invalid;
        if((p->buffers[i].flags&SGPL_GPU_BUFFER_LAYOUT)&&!(p->buffers[i].flags&SGPL_GPU_BUFFER_READ)) goto invalid;
        uintptr_t end=lo+(uintptr_t)bytes,base=lo;
        if(gpu_residency_depth) gpu_registered_range(lo,end,&base,&end);
        s->mirrors[s->mirror_count++]=(gpu_mirror){base,end,p->buffers[i].flags,0,NULL,NULL};
    }
    /* Merge overlapping address ranges. Alias information is preserved,
     * never used to decide independence. Buffers must describe valid storage. */
    for (uint32_t i=0;i<s->mirror_count;++i) for (uint32_t j=i+1;j<s->mirror_count;) {
        gpu_mirror *a=&s->mirrors[i],*b=&s->mirrors[j];
        if (a->lo<b->hi && b->lo<a->hi) {
            if (b->lo<a->lo) a->lo=b->lo;
            if (b->hi>a->hi) a->hi=b->hi;
            a->flags|=b->flags; s->mirrors[j]=s->mirrors[--s->mirror_count];
            j=i+1; /* The enlarged range can intersect an earlier candidate. */
        } else ++j;
    }
    for (uint32_t i=0;i<p->buffer_count;++i) {
        uintptr_t lo=(uintptr_t)s->buffers[i].host; int found=0;
        for (uint32_t j=0;j<s->mirror_count;++j) if(lo>=s->mirrors[j].lo && lo+s->buffers[i].bytes<=s->mirrors[j].hi) {
            s->mapping[i]=j;found=1;break;
        }
        if(!found) goto invalid;
    }
    for (uint32_t i=0;i<p->relocation_count;++i) {
        const sgpl_gpu_relocation *r=&p->relocations[i];
        if(r->owner>=p->buffer_count||r->target>=p->buffer_count||
           s->buffers[r->owner].bytes<sizeof(void*)||r->offset>s->buffers[r->owner].bytes-sizeof(void*)||
           r->target_offset>s->buffers[r->target].bytes) goto invalid;
    }
    for (int32_t i=0;i<p->task_count;++i) {
        const sgpl_gpu_task *t=&p->tasks[i];
        if(t->operation>SGPL_GPU_TASK_DOWNLOAD) goto invalid;
        if(t->operation) continue;
        if(t->host_fn) continue;
        if(!t->kernel||t->root_buffer>=p->buffer_count||t->root_offset>=p->buffers[t->root_buffer].bytes) goto invalid;
        if(p_cuModuleGetFunction(&s->functions[i],module,t->kernel)!=CUDA_SUCCESS) {
            if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-capability] missing kernel=%s; CPU before effects\n",t->kernel);
            sgpl_gpu_release(s);return SGPL_GPU_UNAVAILABLE;
        }
    }
    if(gpu_fault("reserve",0)) goto alloc_fail;
    for (uint32_t i=0;i<s->mirror_count;++i) {
        size_t bytes=s->mirrors[i].hi-s->mirrors[i].lo;
        s->mirrors[i].staging=malloc(bytes);
        gpu_mirror *m=&s->mirrors[i];int pointers=0;
        for(uint32_t j=0;j<p->relocation_count;++j) if(s->mapping[s->relocations[j].owner]==i) pointers=1;
        uintptr_t base=0,end=0;
        int registered=gpu_registered_range(m->lo,m->hi,&base,&end);
        int read_only=(m->flags&SGPL_GPU_BUFFER_READ)&&!(m->flags&SGPL_GPU_BUFFER_WRITE);
        int immutable=read_only&&!registered?((m->flags&SGPL_GPU_BUFFER_LAYOUT)?2:1):0;
        if(!pointers&&(read_only||registered))
            m->resident=gpu_resident_acquire(m->lo,m->hi,immutable);
        if(m->resident) m->device=m->resident->device;
        if(!m->staging||(!m->device&&p_cuMemAlloc(&m->device,bytes)!=CUDA_SUCCESS)) goto alloc_fail;
    }
    if(p->task_count && p_cuMemAlloc(&s->status,(size_t)p->task_count*sizeof(int32_t))!=CUDA_SUCCESS) goto alloc_fail;
    for(int i=0;i<s->slots;++i) if(s->functions[i] &&
        (gpu_fault("stream",i)||p_cuStreamCreate(&s->streams[i],1)!=CUDA_SUCCESS)) goto alloc_fail;
    *out=s; return SGPL_GPU_DONE;
invalid: sgpl_gpu_release(s);return SGPL_GPU_ERR_INVALID;
alloc_fail: sgpl_gpu_release(s);return SGPL_GPU_ERR_ALLOC;
}

int32_t sgpl_gpu_upload(sgpl_gpu_invocation *s) {
    if(!s||s->uploaded) return SGPL_GPU_ERR_INVALID;
    if(p_cuCtxSetCurrent(g_context)!=CUDA_SUCCESS||gpu_fault("upload",0)) return SGPL_GPU_ERR_EXEC;
    for(uint32_t i=0;i<s->mirror_count;++i) {
        gpu_mirror *m=&s->mirrors[i];size_t bytes=m->hi-m->lo;
        if(m->flags&SGPL_GPU_BUFFER_READ) memcpy(m->staging,(void*)m->lo,bytes); else memset(m->staging,0,bytes);
    }
    for(uint32_t i=0;i<s->plan.relocation_count;++i) {
        sgpl_gpu_relocation *r=&s->relocations[i];gpu_mirror *m=&s->mirrors[s->mapping[r->owner]];
        memcpy(&s->original_pointers[i],(uint8_t*)s->buffers[r->owner].host+r->offset,sizeof(void*));
        CUdeviceptr ptr=gpu_buffer_address(s,r->target)+r->target_offset;
        memcpy(m->staging+(uintptr_t)s->buffers[r->owner].host-m->lo+r->offset,&ptr,sizeof(ptr));
    }
    uint64_t transferred=0,resident_bytes=0;
    for(uint32_t i=0;i<s->mirror_count;++i) {
        gpu_mirror *m=&s->mirrors[i];
        if(m->resident&&m->resident->valid) {resident_bytes+=m->hi-m->lo;continue;}
        if(p_cuMemcpyHtoD(m->device,m->staging,m->hi-m->lo)!=CUDA_SUCCESS) return SGPL_GPU_ERR_EXEC;
        transferred+=m->hi-m->lo;
        if(m->resident) {m->resident->valid=1;m->resident->device_dirty=0;}
    }
    if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-transfer] upload_bytes=%llu resident_bytes=%llu\n",(unsigned long long)transferred,(unsigned long long)resident_bytes);
    if(s->plan.task_count && p_cuMemcpyHtoD(s->status,s->host_status,(size_t)s->plan.task_count*sizeof(int32_t))!=CUDA_SUCCESS) return SGPL_GPU_ERR_EXEC;
    s->uploaded=1;return SGPL_GPU_DONE;
}
static int32_t gpu_invocation_submit(void *state,int64_t instance) {
    sgpl_gpu_invocation *s=state;
    sgpl_gpu_task *t=&s->tasks[instance];
    if(t->operation==SGPL_GPU_TASK_UPLOAD) return sgpl_gpu_upload(s);
    if(t->operation==SGPL_GPU_TASK_DOWNLOAD) return sgpl_gpu_download(s);
    if(t->host_fn) {
        sgpl_push_thread_budget(1);int rc=t->host_fn(t->host_state,t->index);sgpl_pop_thread_budget();return rc;
    }
    if(!s->uploaded) return SGPL_GPU_ERR_INVALID;
    CUdeviceptr root=gpu_buffer_address(s,t->root_buffer)+t->root_offset;
    CUdeviceptr status=s->status+(size_t)instance*sizeof(int32_t);
    int64_t index=t->index;void *args[]={&root,&index,&status};
    if(p_cuCtxSetCurrent(g_context)!=CUDA_SUCCESS||gpu_fault("launch",instance)) return SGPL_GPU_ERR_EXEC;
    int spatial=!strncmp(t->kernel,"gpu_spatial_component_",22);
    if(p_cuLaunchKernel(s->functions[instance],spatial?32:1,1,1,spatial?128:1,1,1,0,s->streams[instance],args,NULL)!=CUDA_SUCCESS)
        return SGPL_GPU_ERR_EXEC;
    return SGPL_DAG_PENDING;
}
static int32_t gpu_invocation_poll(void *state,int64_t instance) {
    sgpl_gpu_invocation *s=state;
    if(p_cuCtxSetCurrent(g_context)!=CUDA_SUCCESS) return SGPL_GPU_ERR_EXEC;
    CUresult sync=s->stream_query?s->stream_query(s->streams[instance]):CUDA_SUCCESS;
    if(sync==600) return SGPL_DAG_PENDING; /* CUDA_ERROR_NOT_READY */
    /* Join before reporting any failure. Pending peers are drained by the DAG. */
    CUresult joined=p_cuStreamSynchronize(s->streams[instance]);
    int rc=SGPL_GPU_DONE;
    CUdeviceptr status=s->status+(size_t)instance*sizeof(int32_t);
    if(sync!=CUDA_SUCCESS||joined!=CUDA_SUCCESS||gpu_fault("sync",instance)) rc=SGPL_GPU_ERR_EXEC;
    else if(p_cuMemcpyDtoH(&s->host_status[instance],status,sizeof(int32_t))!=CUDA_SUCCESS) rc=SGPL_GPU_ERR_EXEC;
    else if(s->host_status[instance]!=0) rc=SGPL_GPU_ERR_EXEC;
    if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-task] kernel=%s index=%lld status=%d\n",
        s->tasks[instance].kernel,(long long)s->tasks[instance].index,rc);
    return rc;
}
int32_t sgpl_gpu_execute(sgpl_gpu_invocation *s) {
    if(!s||s->started) return SGPL_GPU_ERR_INVALID;
    s->started=1;
    if (!s->plan.task_count) { s->joined=1; return SGPL_GPU_DONE; }
    sgpl_dag_template t={0};t.node_count=s->plan.task_count;t.node_fn=gpu_check_noop;
    t.relations=s->relations;t.relation_count=s->plan.relation_count;
    int rc=autograph_execute_dag_async(&t,s,gpu_invocation_submit,gpu_invocation_poll);
    s->joined=1;s->failed=rc<0;return rc;
}
int32_t sgpl_gpu_download(sgpl_gpu_invocation *s) {
    if(!s||!s->uploaded||s->failed) return SGPL_GPU_ERR_INVALID;
    if(p_cuCtxSetCurrent(g_context)!=CUDA_SUCCESS||gpu_fault("download",0)) return SGPL_GPU_ERR_EXEC;
    /* Stage every transfer before publication. A failed transfer publishes none. */
    for(uint32_t i=0;i<s->mirror_count;++i) {
        gpu_mirror *m=&s->mirrors[i];if(!(m->flags&SGPL_GPU_BUFFER_WRITE)) continue;
        if(m->resident) {m->resident->device_dirty=1;continue;}
        if(p_cuMemcpyDtoH(m->staging,m->device,m->hi-m->lo)!=CUDA_SUCCESS) return SGPL_GPU_ERR_EXEC;
    }
    /* Device pointers never escape into host frames. */
    for(uint32_t i=0;i<s->plan.relocation_count;++i) {
        sgpl_gpu_relocation *r=&s->relocations[i];gpu_mirror *m=&s->mirrors[s->mapping[r->owner]];
        void *ptr=s->original_pointers[i];
        memcpy(m->staging+(uintptr_t)s->buffers[r->owner].host-m->lo+r->offset,&ptr,sizeof(ptr));
    }
    for(uint32_t i=0;i<s->mirror_count;++i) {
        gpu_mirror *m=&s->mirrors[i];if((m->flags&SGPL_GPU_BUFFER_WRITE)&&!m->resident) memcpy((void*)m->lo,m->staging,m->hi-m->lo);
    }
    s->uploaded=0;
    return SGPL_GPU_DONE;
}
int32_t sgpl_gpu_execute_plan(const sgpl_gpu_plan *p) {
    sgpl_gpu_invocation *s=NULL;int rc=sgpl_gpu_reserve(p,&s);
    /* Reserve/upload failure precedes all user effects in this convenience API. */
    if(rc!=SGPL_GPU_DONE) return rc==SGPL_GPU_ERR_INVALID?rc:SGPL_GPU_UNAVAILABLE;
    rc=p->explicit_transfer_epochs?0:sgpl_gpu_upload(s);
    if(rc==SGPL_GPU_DONE) { rc=sgpl_gpu_execute(s);if(rc==SGPL_GPU_DONE&&!p->explicit_transfer_epochs) rc=sgpl_gpu_download(s); }
    else rc=SGPL_GPU_UNAVAILABLE;
    sgpl_gpu_release(s);return rc;
}

extern int32_t sgpl_gpu_export_graph(void *,void *) __attribute__((weak));
extern int32_t sgpl_gpu_prepare_spatial_records(sgpl_exec_ctx *) __attribute__((weak));
static int gpu_host_buffer_index(const sgpl_gpu_buffer *b,uint32_t nb,const void *pointer,uint64_t bytes,uint64_t *offset) {
    uintptr_t address=(uintptr_t)pointer;
    for(uint32_t i=0;i<nb;++i) {
        uintptr_t base=(uintptr_t)b[i].host;
        if(address>=base && address-base<b[i].bytes && bytes<=b[i].bytes-(address-base)) {
            *offset=address-base;return i;
        }
    }
    return -1;
}
static int gpu_pointer_buffer(sgpl_gpu_buffer *b,uint32_t *nb,sgpl_gpu_relocation *r,uint32_t *nr,
    uint32_t owner,uint64_t offset,uint64_t bytes) {
    void *pointer=NULL;memcpy(&pointer,(uint8_t*)b[owner].host+offset,sizeof(pointer));
    if(!pointer) return 0;
    for(uint32_t i=0;i<*nb;++i) {
        uintptr_t base=(uintptr_t)b[i].host,p=(uintptr_t)pointer;
        if(p>=base && p-base<b[i].bytes && (!bytes||bytes<=b[i].bytes-(p-base))) {
            b[i].flags=(b[i].flags&~SGPL_GPU_BUFFER_LAYOUT)|SGPL_GPU_BUFFER_READ|SGPL_GPU_BUFFER_WRITE;
            r[(*nr)++]=(sgpl_gpu_relocation){owner,i,offset,p-base};return 0;
        }
    }
    if(!bytes) bytes=gpu_known_extent(pointer);
    if(!bytes) return -1;
    uint32_t target=(*nb)++;b[target]=(sgpl_gpu_buffer){pointer,bytes,3};
    r[(*nr)++]=(sgpl_gpu_relocation){owner,target,offset,0};return 0;
}
static int gpu_context_buffers(sgpl_exec_ctx *c,uint32_t target,sgpl_gpu_buffer *b,uint32_t *nb,
    sgpl_gpu_relocation *r,uint32_t *nr) {
    if(!sgpl_gpu_prepare_spatial_records||sgpl_gpu_prepare_spatial_records(c)<0||c->spatial_snapshot_count>64) return -1;
    uint64_t n=c->gpu_vertex_count,p=c->gpu_partition_count;
    if(p>64||c->op_count>64) return -1;
    if(c->dest_seen && !c->spatial_frontier_lanes) return -1;
    if(n>SIZE_MAX/4 || (n && p>SIZE_MAX/(n*4))) return -1;
#define GPU_CTX(F,BYTES) do {if(gpu_pointer_buffer(b,nb,r,nr,target,offsetof(sgpl_exec_ctx,F),(BYTES))<0) return -1;}while(0)
    GPU_CTX(gpu_records,c->gpu_record_count?c->gpu_record_count*12:4);
    GPU_CTX(gpu_record_offsets,(p+1)*8);
    GPU_CTX(gpu_group_offsets,(c->gpu_group_count+1)*8);
    GPU_CTX(gpu_partition_groups,(p+1)*8);
    GPU_CTX(gpu_group_times,c->gpu_group_count*24);
    /* These are immutable layout inputs, never device outputs. */
    for(uint32_t i=0;i<*nb;++i) if(b[i].host==c->gpu_records||b[i].host==c->gpu_record_offsets||
        b[i].host==c->gpu_group_offsets||b[i].host==c->gpu_partition_groups) b[i].flags=SGPL_GPU_BUFFER_READ|SGPL_GPU_BUFFER_LAYOUT;
    GPU_CTX(membership,n);GPU_CTX(dest_seen,n*4);GPU_CTX(next_frontier,n*4);
    GPU_CTX(append_head,4);GPU_CTX(partition_base,p*c->partition_stride);
    GPU_CTX(spatial_source_gate,n);GPU_CTX(spatial_lane_seen,n*p*4);GPU_CTX(spatial_groups_seen,n);
    GPU_CTX(execution_error,4);GPU_CTX(spatial_snapshots,c->spatial_snapshot_count*8);
    GPU_CTX(spatial_claims,c->gpu_claim_bytes);
    if(c->gpu_private_targets) {
        uint32_t ns=c->gpu_private_scalars,na=c->gpu_private_arrays;
        if(ns>8||na>8||!c->gpu_private_element_bytes||!c->partition_base||
           c->partition_stride<(int64_t)(8*(ns+na))) return -1;
        GPU_CTX(gpu_private_targets,8*(ns+2*na));
        uint64_t targets_offset=0,records_offset=0;
        int targets=gpu_host_buffer_index(b,*nb,c->gpu_private_targets,8*(ns+2*na),&targets_offset);
        int records=gpu_host_buffer_index(b,*nb,c->partition_base,p*c->partition_stride,&records_offset);
        if(targets<0||records<0) return -1;
        for(uint32_t i=0;i<ns+na;++i) {
            uint64_t extent=c->gpu_private_element_bytes[i];
            if(!extent) return -1;
            if(i>=ns) {
                uint64_t elements=0;memcpy(&elements,(uint8_t*)c->gpu_private_targets+8*(ns+na+i-ns),8);
                if(elements>SIZE_MAX/extent) return -1;
                extent*=elements;
                /* Empty private arrays have no addressable elements. */
                if(!extent) extent=1;
                for(uint64_t part=0;part<p;++part)
                    if(gpu_pointer_buffer(b,nb,r,nr,records,records_offset+part*c->partition_stride+8*i,extent)<0) return -1;
            }
            if(gpu_pointer_buffer(b,nb,r,nr,targets,targets_offset+8*i,extent)<0) return -1;
        }
    }
    /* Frontier lane data pointers need their own relocation records. */
    if(c->spatial_frontier_lanes) {
        struct gpu_lane { int32_t *data;int32_t size,capacity; } *lanes=c->spatial_frontier_lanes;
        GPU_CTX(spatial_frontier_lanes,p*sizeof(*lanes));uint64_t offset=0;
        int lane_index=gpu_host_buffer_index(b,*nb,lanes,p*sizeof(*lanes),&offset);if(lane_index<0)return -1;
        for(uint64_t i=0;i<p;++i) if(gpu_pointer_buffer(b,nb,r,nr,lane_index,offset+i*sizeof(*lanes),lanes[i].capacity?lanes[i].capacity*4:4)<0) return -1;
    }
    if(c->spatial_snapshots) {
        uint64_t offset=0;int table=gpu_host_buffer_index(b,*nb,c->spatial_snapshots,c->spatial_snapshot_count*8,&offset);if(table<0)return -1;
        for(int i=0;i<c->spatial_snapshot_count;++i) if(gpu_pointer_buffer(b,nb,r,nr,table,offset+i*8,c->spatial_snapshot_bytes[i])<0) return -1;
    }
    GPU_CTX(ops,c->op_count*sizeof(sgpl_runtime_op));uint64_t ops_offset=0;
    int ops=gpu_host_buffer_index(b,*nb,c->ops,c->op_count*sizeof(sgpl_runtime_op),&ops_offset);
    if(c->op_count&&ops<0) return -1;
    for(uint32_t i=0;i<c->op_count;++i)
        if(gpu_pointer_buffer(b,nb,r,nr,ops,ops_offset+i*sizeof(sgpl_runtime_op)+offsetof(sgpl_runtime_op,state),0)<0) return -1;
#undef GPU_CTX
    return 0;
}
static int32_t gpu_callback_prepare(const char *kernel,void **frames,uint64_t bytes,uint32_t roots,
    const sgpl_gpu_field *fields,uint32_t count,sgpl_gpu_task *work,int32_t tasks,int32_t budget,
    const sgpl_dag_relation_desc *edges,int32_t edge_count,uint32_t schedule,uint32_t scope,sgpl_gpu_invocation **out) {
    *out=NULL;
    if(!frames||!roots||!bytes||(count&&!fields)||tasks<0||count>4096||roots>4096/(count?count:1)) return SGPL_GPU_ERR_INVALID;
    uint32_t capacity=roots*(1+count*1024),nb=0,nr=0;
    sgpl_gpu_buffer *buffers=calloc(capacity,sizeof(*buffers));
    sgpl_gpu_relocation *rel=calloc(capacity,sizeof(*rel));
    sgpl_gpu_graph *graphs=calloc(roots*(count?count:1),sizeof(*graphs));
    void **graph_handles=calloc(roots*(count?count:1),sizeof(void*));
    uint32_t *root_indices=calloc(roots,sizeof(*root_indices));
    int rc=SGPL_GPU_UNAVAILABLE;
    int64_t estimated_work=0;
    if(!buffers||!rel||!graphs||!graph_handles||!root_indices) goto done;
    for(uint32_t root=0;root<roots;++root) {
    void *frame=frames[root];uint32_t owner=nb;root_indices[root]=nb;
    if(!frame){rc=SGPL_GPU_ERR_INVALID;goto done;}
    buffers[nb++]=(sgpl_gpu_buffer){frame,bytes,3};
    for(uint32_t i=0;i<count;++i) {
        const sgpl_gpu_field *f=&fields[i];void *pointer=NULL;
        uint32_t kind=f->kind&~SGPL_GPU_FIELD_READ_ONLY;
        uint32_t flags=f->kind&SGPL_GPU_FIELD_READ_ONLY?SGPL_GPU_BUFFER_READ:3;
        if(kind>SGPL_GPU_FIELD_GRAPH_POINTER){rc=SGPL_GPU_ERR_INVALID;goto done;}
        if(bytes<sizeof(pointer)||f->offset>bytes-sizeof(pointer)) {rc=SGPL_GPU_ERR_INVALID;goto done;}
        memcpy(&pointer,(uint8_t*)frame+f->offset,sizeof(pointer));
        if(!pointer) continue;
        uint32_t target=nb;
        if(kind==SGPL_GPU_FIELD_CONTEXT) {
            ((sgpl_exec_ctx*)pointer)->gpu_parallel_fold=!strncmp(kernel,"gpu_spatial_component_f_",24);
            buffers[nb++]=(sgpl_gpu_buffer){pointer,sizeof(sgpl_exec_ctx),3};
            if(gpu_context_buffers(pointer,target,buffers,&nb,rel,&nr)<0) goto done;
            int64_t amount=((sgpl_exec_ctx*)pointer)->gpu_record_count;
            estimated_work=amount>INT64_MAX-estimated_work?INT64_MAX:estimated_work+amount;
        } else if(kind==SGPL_GPU_FIELD_GRAPH||kind==SGPL_GPU_FIELD_GRAPH_POINTER) {
            sgpl_gpu_graph *g=&graphs[root*count+i];
            uint32_t graph_owner=owner;uint64_t graph_offset=f->offset;
            if(kind==SGPL_GPU_FIELD_GRAPH_POINTER) {
                buffers[nb++]=(sgpl_gpu_buffer){pointer,sizeof(void*),3};
                graph_owner=target;graph_offset=0;
                rel[nr++]=(sgpl_gpu_relocation){owner,target,f->offset,0};
                memcpy(&pointer,pointer,sizeof(pointer));target=nb;
            }
            uint32_t slot=root*count+i;int found=-1;
            for(uint32_t j=0;j<slot;++j) if(graph_handles[j]==pointer){found=j;break;}
            if(found>=0) g=&graphs[found];
            else {graph_handles[slot]=pointer;if(!sgpl_gpu_export_graph||sgpl_gpu_export_graph(pointer,g)<0)goto done;}
            if(g->n<0||g->m<0||
                (uint64_t)g->n>(SIZE_MAX/8)-1||(uint64_t)g->m>SIZE_MAX/4) goto done;
            estimated_work=g->m>INT64_MAX-estimated_work?INT64_MAX:estimated_work+g->m;
            buffers[nb++]=(sgpl_gpu_buffer){g,sizeof(*g),1};
            buffers[nb]=(sgpl_gpu_buffer){g->rows,((uint64_t)g->n+1)*8,SGPL_GPU_BUFFER_READ|SGPL_GPU_BUFFER_LAYOUT};
            rel[nr++]=(sgpl_gpu_relocation){target,nb++,offsetof(sgpl_gpu_graph,rows),0};
            if(g->m) {buffers[nb]=(sgpl_gpu_buffer){g->columns,(uint64_t)g->m*4,SGPL_GPU_BUFFER_READ|SGPL_GPU_BUFFER_LAYOUT};
                rel[nr++]=(sgpl_gpu_relocation){target,nb++,offsetof(sgpl_gpu_graph,columns),0};}
            if(kind==SGPL_GPU_FIELD_GRAPH_POINTER) {
                rel[nr++]=(sgpl_gpu_relocation){graph_owner,target,graph_offset,0};continue;
            }
        } else {
            uint64_t size=f->bytes;
            if(!size) {
                size=gpu_known_extent(pointer);
                if(!size) {if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-capability] missing buffer extent field=%u\n",i);goto done;}
            }
            buffers[nb++]=(sgpl_gpu_buffer){pointer,size,flags};
            if(kind==SGPL_GPU_FIELD_POINTER) {
                void *pointee=NULL;memcpy(&pointee,pointer,sizeof(pointee));
                if(pointee) {
                    uint64_t extent=gpu_known_extent(pointee);
                    if(!extent) {if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-capability] missing pointee extent field=%u\n",i);goto done;}
                    buffers[nb]=(sgpl_gpu_buffer){pointee,extent,flags};
                    rel[nr++]=(sgpl_gpu_relocation){target,nb++,0,0};
                }
            }
        }
        rel[nr++]=(sgpl_gpu_relocation){owner,target,f->offset,0};
    }
    }
    for(int i=0;i<tasks;++i) if(!work[i].host_fn) {
        if(work[i].root_buffer>=roots) {rc=SGPL_GPU_ERR_INVALID;goto done;}
        work[i].kernel=kernel;work[i].root_buffer=root_indices[work[i].root_buffer];
    }
    if(!estimated_work) estimated_work=tasks;
    int64_t minimum=getenv("SGPL_GPU_MIN_TRIPS")?atoll(getenv("SGPL_GPU_MIN_TRIPS")):4096;
    if(sgpl_gpu_policy_verdict(estimated_work,0,0,minimum,8192)!=SGPL_GPU_OFFLOAD) {
        if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-capability] schedule=%u scope=%u work=%lld below cost threshold=%lld; CPU\n",schedule,scope,(long long)estimated_work,(long long)minimum);
        goto done;
    }
    sgpl_gpu_plan plan={.version=1,.schedule=schedule,.temporal_scope=scope,.buffers=buffers,.buffer_count=nb,.relocations=rel,
        .relocation_count=nr,.tasks=work,.task_count=tasks,.relations=edges,.relation_count=edge_count};
    (void)budget; /* Retained call ABI; GPU execution has no resource grants. */
    rc=sgpl_gpu_reserve(&plan,out);
    if(rc==0) {(*out)->owned_graphs=graphs;(*out)->owned_graph_count=roots*count;graphs=NULL;}
done:
    if(graphs) for(uint32_t i=0;i<roots*count;++i) {free(graphs[i].rows);free(graphs[i].columns);}
    free(buffers);free(rel);free(graphs);free(graph_handles);free(root_indices);return rc;
}
int32_t sgpl_gpu_callback_execute_scope(const char *kernel,void *frame,uint64_t bytes,
    const sgpl_gpu_field *fields,uint32_t count,int32_t tasks,int32_t budget,uint32_t schedule,uint32_t scope) {
    if(tasks<0||tasks>SGPL_GPU_MAX_TASKS)return SGPL_GPU_ERR_INVALID;
    if(tasks==0) return 0;
    sgpl_gpu_task *work=calloc(tasks,sizeof(*work));if(!work) return SGPL_GPU_UNAVAILABLE;
    for(int i=0;i<tasks;++i) work[i]=(sgpl_gpu_task){.index=i};
    sgpl_gpu_invocation *s=NULL;int rc=gpu_callback_prepare(kernel,&frame,bytes,1,fields,count,work,tasks,budget,NULL,0,schedule,scope,&s);
    free(work);
    if(rc) return rc==SGPL_GPU_ERR_INVALID?rc:SGPL_GPU_UNAVAILABLE;
    rc=sgpl_gpu_upload(s);
    if(rc==0) {rc=sgpl_gpu_execute(s);if(rc==0) rc=sgpl_gpu_download(s);} else rc=SGPL_GPU_UNAVAILABLE;
    sgpl_gpu_release(s);return rc;
}
int32_t sgpl_gpu_callback_execute(const char *kernel,void *frame,uint64_t bytes,
    const sgpl_gpu_field *fields,uint32_t count,int32_t tasks,int32_t budget) {
    return sgpl_gpu_callback_execute_scope(kernel,frame,bytes,fields,count,tasks,budget,SGPL_GPU_LEGACY_DAG,0);
}

typedef struct gpu_round_window {
    const sgpl_round_spatial_plan *plan;
    void **frames;
    int64_t *rounds;
    sgpl_gpu_invocation *device;
} gpu_round_window;
static int32_t gpu_round_setup(void *opaque,int64_t round) {
    gpu_round_window *w=opaque;return sgpl_round_spatial_host_call(w->plan->setup,w->frames[round],w->rounds[round]);
}
static int32_t gpu_round_finalize(void *opaque,int64_t round) {
    gpu_round_window *w=opaque;return sgpl_round_spatial_host_call(w->plan->finalize,w->frames[round],w->rounds[round]);
}
static int32_t gpu_window_upload(void *opaque,int64_t index) {(void)index;return sgpl_gpu_upload(((gpu_round_window*)opaque)->device);}
static int32_t gpu_window_download(void *opaque,int64_t index) {(void)index;return sgpl_gpu_download(((gpu_round_window*)opaque)->device);}
int32_t sgpl_gpu_round_spatial_execute(const sgpl_round_spatial_plan *p,int32_t budget,const char *kernel,
    uint64_t bytes,const void *field_data,uint32_t field_count,void *const *globals,uint32_t nglobals) {
    if(!p||!p->reserve||!p->release||!p->setup||!p->finalize||p->round_count<0||p->partitions<0||
       !p->frame_bytes||bytes<p->frame_bytes||bytes>SIZE_MAX||nglobals>(bytes-p->frame_bytes)/8||
       (nglobals&&!globals)||(p->round_count && p->first_round>INT64_MAX-p->round_count+1)) return SGPL_GPU_ERR_INVALID;
    if(!p->round_count) return 0;
    if(!sgpl_round_spatial_host_call) return SGPL_GPU_UNAVAILABLE;
    if(!load_cuda()) {
        if(getenv("SGPL_GPU_DEBUG")) fprintf(stderr,"[gpu-capability] RoundSpatial: CUDA driver or device unavailable; CPU before effects\n");
        return SGPL_GPU_UNAVAILABLE;
    }
    (void)budget;
    int batches=p->partitions;
    if(batches>SGPL_GPU_MAX_TASKS-4)batches=SGPL_GPU_MAX_TASKS-4;
    int groups=(SGPL_GPU_MAX_TASKS-2)/(batches+2);
    /* These are descriptor representation limits, not resource grants. */
    int frame_limit=4096/(field_count?field_count:1);
    if(groups>frame_limit)groups=frame_limit;
    if(groups>p->round_count)groups=(int)p->round_count;
    if(groups<1||bytes>SIZE_MAX/(uint64_t)groups) return SGPL_GPU_ERR_INVALID;
    uint64_t stride=(bytes+15)&~UINT64_C(15);if(stride<bytes||stride>SIZE_MAX/(uint64_t)groups) return SGPL_GPU_ERR_INVALID;
    unsigned char *storage=calloc(groups,stride);void **frames=calloc(groups,sizeof(void*));
    int64_t *rounds=calloc(groups,sizeof(int64_t));
    int max_tasks=groups*(batches+2)+2,max_edges=groups*(2*batches+2)+2;
    sgpl_gpu_task *tasks=calloc(max_tasks,sizeof(*tasks));sgpl_dag_relation_desc *edges=calloc(max_edges,sizeof(*edges));
    if(!storage||!frames||!rounds||!tasks||!edges) {free(storage);free(frames);free(rounds);free(tasks);free(edges);return SGPL_GPU_UNAVAILABLE;}
    int64_t q=p->round_count/groups,rem=p->round_count%groups;int rc=0,started=0;
    for(int64_t member=0;member<q+(rem!=0)&&!rc;++member) {
        int live=0,reserved=0;memset(storage,0,groups*stride);
        for(int g=0;g<groups;++g) if(member<q+(g<rem)) {
            frames[live]=storage+(uint64_t)live*stride;
            rounds[live++]=p->first_round+(int64_t)g*q+(g<rem?g:rem)+member;
        }
        for(int r=0;r<live;++r) {
            ++reserved;rc=p->reserve(p->shared,frames[r],rounds[r]);if(rc) break;
            for(uint32_t i=0;i<nglobals;++i) memcpy((uint8_t*)frames[r]+p->frame_bytes+i*8,&globals[i],8);
        }
        gpu_round_window window={p,frames,rounds,NULL};
        int upload=live,first_batch=upload+1,download=first_batch+live*batches,first_finish=download+1;
        int ntasks=first_finish+live,ne=0;memset(tasks,0,max_tasks*sizeof(*tasks));memset(edges,0,max_edges*sizeof(*edges));
#define GPU_EDGE(A,B) do {edges[ne++]=(sgpl_dag_relation_desc){.source_node=(A),.sink_node=(B),.flags=SGPL_DAG_REL_FLAG_REALIZATION,.axis_mask=SGPL_DAG_AXIS_PRODUCT};}while(0)
        tasks[upload]=(sgpl_gpu_task){.host_fn=gpu_window_upload,.host_state=&window};
        tasks[download]=(sgpl_gpu_task){.host_fn=gpu_window_download,.host_state=&window};
        for(int r=0;r<live;++r) {
            tasks[r]=(sgpl_gpu_task){.index=r,.host_fn=gpu_round_setup,.host_state=&window};GPU_EDGE(r,upload);
            tasks[first_finish+r]=(sgpl_gpu_task){.index=r,.host_fn=gpu_round_finalize,.host_state=&window};GPU_EDGE(download,first_finish+r);
            for(int b=0;b<batches;++b) {
                int part_q=p->partitions/batches,part_rem=p->partitions%batches;
                uint32_t begin=b*part_q+(b<part_rem?b:part_rem),count=part_q+(b<part_rem);
                int task=first_batch+r*batches+b;
                tasks[task]=(sgpl_gpu_task){.root_buffer=r,.index=(int64_t)(((uint64_t)count<<32)|begin)};
                GPU_EDGE(upload,task);GPU_EDGE(task,download);
            }
        }
        if(!batches) GPU_EDGE(upload,download);
#undef GPU_EDGE
        if(!rc) rc=gpu_callback_prepare(kernel,frames,bytes,live,field_data,field_count,tasks,ntasks,0,edges,ne,SGPL_GPU_ROUND_SPATIAL,SGPL_TEMPORAL_ROUND,&window.device);
        if(!rc) {started=1;rc=sgpl_gpu_execute(window.device);}
        sgpl_gpu_release(window.device);
        for(int r=0;r<reserved;++r) p->release(frames[r]);
        if(rc && !started) rc=SGPL_GPU_UNAVAILABLE;
    }
    free(storage);free(frames);free(rounds);free(tasks);free(edges);return rc;
}

typedef struct gpu_spatial_run {sgpl_exec_ctx *ctx;sgpl_gpu_invocation *device;int32_t result;} gpu_spatial_run;
static int32_t gpu_spatial_setup(void *opaque,int64_t index) {
    (void)index;gpu_spatial_run *r=opaque;int rc=sgpl_exec_spatial_begin(r->ctx);return rc<0?rc:sgpl_exec_spatial_publish(r->ctx);
}
static int32_t gpu_spatial_upload(void *opaque,int64_t index) {(void)index;return sgpl_gpu_upload(((gpu_spatial_run*)opaque)->device);}
static int32_t gpu_spatial_download(void *opaque,int64_t index) {(void)index;return sgpl_gpu_download(((gpu_spatial_run*)opaque)->device);}
void sgpl_gpu_report_spatial(sgpl_exec_ctx *c) {
    if(c->gpu_group_times) {
        int lanes[4096]={0},active=0,overlap=0;
        for(int p=0;p<c->gpu_partition_count;++p) {
            uint64_t best_start=0,best_end=0;
            for(int64_t i=c->gpu_partition_groups[p];i<c->gpu_partition_groups[p+1];++i) {
                uint64_t start=c->gpu_group_times[3*i],end=c->gpu_group_times[3*i+1],lane=c->gpu_group_times[3*i+2];
                if(!start||!end) continue;
                if(lane<4096&&!lanes[lane]) {lanes[lane]=1;++active;}
                if(start<best_end&&end>best_start) overlap=1;
                if(end>best_end) {best_start=start;best_end=end;}
            }
        }
        fprintf(stderr,"[gpu-groups] groups=%lld active_lanes=%d within_batch_overlap=%d parallel=%d\n",(long long)c->gpu_group_count,active,overlap,c->gpu_parallel_groups);
    }
}
static int32_t gpu_spatial_finish(void *opaque,int64_t index) {
    (void)index;gpu_spatial_run *r=opaque;
    r->result=sgpl_exec_spatial_finish(r->ctx);return r->result<0?r->result:0;
}
int32_t sgpl_gpu_spatial_execute(sgpl_exec_ctx *ctx,const char *kernel,const void *fields,uint32_t count,
    void *const *globals,uint32_t nglobals,int32_t budget) {
    if(!ctx||nglobals>4096) return SGPL_GPU_ERR_INVALID;
    ctx->gpu_parallel_fold=!strncmp(kernel,"gpu_spatial_component_f_",24);
    if(!sgpl_exec_spatial_begin||!sgpl_exec_spatial_publish||!sgpl_exec_spatial_finish) return SGPL_GPU_UNAVAILABLE;
    if(!sgpl_gpu_prepare_spatial_records||sgpl_gpu_prepare_spatial_records(ctx)<0) return SGPL_GPU_UNAVAILABLE;
    int p=ctx->gpu_partition_count;if(p<1) return SGPL_GPU_UNAVAILABLE;
    uint64_t bytes=8+(uint64_t)nglobals*8;void **frame=calloc(nglobals+1,8);
    sgpl_gpu_task *tasks=calloc(p+4,sizeof(*tasks));sgpl_dag_relation_desc *edges=calloc(2*p+3,sizeof(*edges));
    if(!frame||!tasks||!edges) {free(frame);free(tasks);free(edges);return SGPL_GPU_UNAVAILABLE;}
    frame[0]=ctx;for(uint32_t i=0;i<nglobals;++i) frame[i+1]=globals[i];
    gpu_spatial_run run={ctx,NULL,0};int ne=0;
    tasks[0]=(sgpl_gpu_task){.host_fn=gpu_spatial_setup,.host_state=&run};
    tasks[1]=(sgpl_gpu_task){.host_fn=gpu_spatial_upload,.host_state=&run};
    tasks[p+2]=(sgpl_gpu_task){.host_fn=gpu_spatial_download,.host_state=&run};
    tasks[p+3]=(sgpl_gpu_task){.host_fn=gpu_spatial_finish,.host_state=&run};
#define SPATIAL_GPU_EDGE(A,B) do {edges[ne++]=(sgpl_dag_relation_desc){.source_node=(A),.sink_node=(B),.flags=SGPL_DAG_REL_FLAG_REALIZATION,.axis_mask=SGPL_DAG_AXIS_SPATIAL};}while(0)
    SPATIAL_GPU_EDGE(0,1);SPATIAL_GPU_EDGE(p+2,p+3);
    for(int i=0;i<p;++i) {tasks[i+2]=(sgpl_gpu_task){.index=i};SPATIAL_GPU_EDGE(1,i+2);SPATIAL_GPU_EDGE(i+2,p+2);}
#undef SPATIAL_GPU_EDGE
    int rc=gpu_callback_prepare(kernel,(void**)&frame,bytes,1,fields,count,tasks,p+4,budget,edges,ne,SGPL_GPU_SPATIAL,0,&run.device);
    if(!rc) rc=sgpl_gpu_execute(run.device);else rc=SGPL_GPU_UNAVAILABLE;
    sgpl_gpu_release(run.device);free(frame);free(tasks);free(edges);return rc;
}
