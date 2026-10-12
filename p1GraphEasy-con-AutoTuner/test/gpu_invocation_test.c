#include "gpu_runtime.h"
#include "autotuner_runtime.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SGPL_SPATIAL_FIXTURE_ONLY
#define sgpl_gpu_engine_step_verdict fixture_gpu_verdict
#define sgpl_gpu_step_schedule_ok fixture_gpu_schedule
#define autograph_gpu_step_name_for fixture_gpu_name
#define gpup_step_try fixture_gpu_try
#define gpup_step_v_try fixture_gpu_v_try
#include "spatial_exec_test.c"
#undef sgpl_gpu_engine_step_verdict
#undef sgpl_gpu_step_schedule_ok
#undef autograph_gpu_step_name_for
#undef gpup_step_try
#undef gpup_step_v_try
/* One ordered task writes through two aliased captures. */
const char gpu_embedded_ptx[] =
".version 7.0\n.target sm_50\n.address_size 64\n"
".visible .entry component(.param .u64 root,.param .u64 idx,.param .u64 status) {\n"
".reg .b64 %r<9>; .reg .b32 %v<3>;\n"
"ld.param.u64 %r0,[root];ld.param.u64 %r1,[idx];ld.param.u64 %r2,[status];\n"
"ld.u64 %r3,[%r0];ld.u64 %r4,[%r0+8];shl.b64 %r5,%r1,2;add.u64 %r6,%r3,%r5;\n"
"cvt.u32.u64 %v0,%r1;add.u32 %v0,%v0,11;st.u32 [%r6],%v0;\n"
"add.u64 %r7,%r4,%r5;ld.u32 %v1,[%r7];add.u32 %v1,%v1,1;st.u32 [%r7],%v1;\n"
"st.u32 [%r2],0;ret; }\n"
/* One ordered component. %globaltimer measures device overlap directly. */
".visible .entry timed(.param .u64 root,.param .u64 idx,.param .u64 status) {\n"
".reg .b64 %r<16>; .reg .b32 %v<3>; .reg .pred %p;\n"
"ld.param.u64 %r0,[root];ld.param.u64 %r1,[idx];ld.param.u64 %r2,[status];\n"
"and.b64 %r1,%r1,4294967295;ld.u64 %r3,[%r0];ld.u64 %r4,[%r0+8];ld.u64 %r8,[%r0+16];\n"
"shl.b64 %r5,%r1,3;add.u64 %r6,%r3,%r5;add.u64 %r7,%r4,%r5;\n"
"shl.b64 %r9,%r1,4;add.u64 %r10,%r8,%r9;mov.u64 %r11,%globaltimer;st.u64 [%r10],%r11;\n"
"add.u64 %r12,%r11,500000000;wait:mov.u64 %r13,%globaltimer;setp.lt.u64 %p,%r13,%r12;@%p bra wait;\n"
"cvt.u32.u64 %v0,%r1;mul.lo.u32 %v0,%v0,2;add.u32 %v0,%v0,11;st.u32 [%r6],%v0;\n"
"ld.u32 %v1,[%r7];add.u32 %v1,%v1,1;st.u32 [%r7],%v1;\n"
"mov.u64 %r14,%globaltimer;st.u64 [%r10+8],%r14;st.u32 [%r2],0;ret; }\n"
".visible .entry ordered(.param .u64 root,.param .u64 idx,.param .u64 status) {\n"
".reg .b64 %r<3>; .reg .b32 %v;ld.param.u64 %r0,[root];ld.u64 %r1,[%r0];\n"
"ld.u32 %v,[%r1];add.u32 %v,%v,1;st.u32 [%r1],%v;ld.param.u64 %r2,[status];st.u32 [%r2],0;ret;}\n"
".visible .entry distinct(.param .u64 root,.param .u64 idx,.param .u64 status) {\n"
".reg .b64 %r<4>; .reg .b32 %v; .reg .pred %p;ld.param.u64 %r0,[root];ld.u64 %r1,[%r0];ld.u64 %r2,[%r0+8];\n"
"setp.eq.u64 %p,%r1,%r2;selp.u32 %v,1,0,%p;st.u32 [%r0+16],%v;ld.param.u64 %r3,[status];st.u32 [%r3],0;ret;}\n"
/* Compatibility ABI for the existing DOALL and cooperative launch routes. */
".visible .entry legacy_for(.param .u64 first,.param .u64 last,.param .u64 step,.param .u64 env) {\n"
".reg .b64 %r<10>; .reg .b32 %v<5>; .reg .pred %p;\n"
"ld.param.u64 %r0,[first];ld.param.u64 %r1,[last];ld.param.u64 %r2,[step];ld.param.u64 %r3,[env];ld.u64 %r4,[%r3];\n"
"mov.u32 %v0,%ctaid.x;mov.u32 %v1,%ntid.x;mov.u32 %v2,%tid.x;mad.lo.u32 %v0,%v0,%v1,%v2;\n"
"cvt.u64.u32 %r5,%v0;mul.lo.u64 %r5,%r5,%r2;add.u64 %r5,%r5,%r0;\n"
"mov.u32 %v3,%nctaid.x;mul.lo.u32 %v3,%v3,%v1;cvt.u64.u32 %r6,%v3;mul.lo.u64 %r6,%r6,%r2;\n"
"legacy_loop:setp.ge.u64 %p,%r5,%r1;@%p bra legacy_done;shl.b64 %r7,%r5,2;add.u64 %r8,%r4,%r7;\n"
"ld.u32 %v4,[%r8];add.u32 %v4,%v4,1;st.u32 [%r8],%v4;add.u64 %r5,%r5,%r6;bra legacy_loop;\n"
"legacy_done:ret;}\n";
static atomic_int legacy_cpu_calls;
static void legacy_body(int64_t index,void *state) {
    atomic_fetch_add(&legacy_cpu_calls,1);(*(int32_t**)state)[index]++;
}
static void legacy_launches(void) {
    int32_t values[513]={0};int32_t *frame=values;
    int64_t offset=0,bytes=sizeof(values);
    for(int cooperative=0;cooperative<2;++cooperative) {
        memset(values,0,sizeof(values));legacy_cpu_calls=0;
        gpu_parallel_for_runtime(0,513,1,"legacy_for",legacy_body,&frame,sizeof(frame),
            &offset,&bytes,NULL,NULL,1,NULL,NULL,NULL,0,cooperative,0,NULL,513);
        assert(legacy_cpu_calls==0&&frame==values);
        for(int i=0;i<513;++i) assert(values[i]==1);
    }
    puts("PASS existing DOALL and cooperative GPU launch compatibility");
}
typedef struct {int32_t *a,*b;uint64_t *times;int64_t round;} device_frame;
typedef struct { device_frame frame;sgpl_gpu_field fields[3]; } nested_gpu_fixture;
static int32_t nested_gpu_node(void *opaque,int64_t node) {
    if(node)return 0;
    nested_gpu_fixture *s=opaque;
    return sgpl_gpu_callback_execute("timed",&s->frame,sizeof(s->frame),s->fields,3,2,1);
}
typedef struct {
    int32_t values[10][9]; uint64_t times[10][6];
    atomic_int setups,finishes,reserves,releases,cpu_calls;
    int fail_setup,fail_finalize,fail_reserve_round;
} lifecycle;
static int32_t reserve_round(void *opaque,void *frame,int64_t round) {
    lifecycle *s=opaque;atomic_fetch_add(&s->reserves,1);
    *(device_frame*)frame=(device_frame){s->values[round],s->values[round]+1,s->times[round],round};
    return round==s->fail_reserve_round?-71:0;
}
static int32_t setup_round(void *frame,int64_t round) {
    device_frame *f=frame;lifecycle *s=(lifecycle*)((char*)f->a-round*sizeof(((lifecycle*)0)->values[0]));
    atomic_fetch_add(&s->setups,1);memset(f->a,0,9*sizeof(int32_t));
    return s->fail_setup?-72:0;
}
static int32_t finish_round(void *frame,int64_t round) {
    device_frame *f=frame;lifecycle *s=(lifecycle*)((char*)f->a-round*sizeof(((lifecycle*)0)->values[0]));
    assert(f->a[0]==11&&f->a[1]==1&&f->a[2]==13&&f->a[3]==1);
    assert(f->times[0]&&f->times[2]);atomic_fetch_add(&s->finishes,1);
    return s->fail_finalize?-73:0;
}
static void release_round(void *frame) {
    device_frame *f=frame;if(!f->a)return;
    lifecycle *s=(lifecycle*)((char*)f->a-f->round*sizeof(((lifecycle*)0)->values[0]));atomic_fetch_add(&s->releases,1);
}
static int32_t cpu_batch(void *frame,int64_t partition) {(void)partition;device_frame *f=frame;
    lifecycle *s=(lifecycle*)((char*)f->a-f->round*sizeof(((lifecycle*)0)->values[0]));atomic_fetch_add(&s->cpu_calls,1);return 0;}
static int32_t cpu_node(void *s,int64_t index) {(void)index;atomic_fetch_add((atomic_int*)s,1);return 0;}
static int32_t multiply_host(void *state,int64_t index) {(void)index;*(int32_t*)state*=10;return 0;}
static void transfer_epochs(void) {
    int32_t value=1;void *frame=&value;
    sgpl_gpu_buffer buffers[]={{&frame,8,3},{&value,4,3}};sgpl_gpu_relocation rel={0,1,0,0};
    sgpl_gpu_task tasks[7]={0};
    tasks[0].operation=tasks[4].operation=SGPL_GPU_TASK_UPLOAD;
    tasks[2].operation=tasks[6].operation=SGPL_GPU_TASK_DOWNLOAD;
    tasks[1].kernel=tasks[5].kernel="ordered";
    tasks[3].host_fn=multiply_host;tasks[3].host_state=&value;
    sgpl_dag_relation_desc edges[6]={0};for(int i=0;i<6;++i) {edges[i].source_node=i;edges[i].sink_node=i+1;edges[i].flags=SGPL_DAG_REL_FLAG_REALIZATION;}
    sgpl_gpu_plan p={.version=1,.schedule=5,.buffers=buffers,.buffer_count=2,.relocations=&rel,.relocation_count=1,
        .tasks=tasks,.task_count=7,.relations=edges,.relation_count=6,.worker_budget=4,.explicit_transfer_epochs=1};
    assert(sgpl_gpu_execute_plan(&p)==0&&value==21&&frame==&value);
    puts("PASS legacy DAG: device, host, device transfer epochs retain carried values");
}
static int overlap(uint64_t a,uint64_t b,uint64_t c,uint64_t d) {return a<d&&c<b;}
static void temporal_and_product(void) {
    sgpl_gpu_field fields[]={{0,36,0},{8,32,0},{16,48,0}};
    lifecycle s={.fail_reserve_round=-1};
    device_frame warm={s.values[0],s.values[0]+1,s.times[0],0};
    assert(sgpl_gpu_callback_execute("timed",&warm,sizeof(warm),fields,3,1,1)==0);
    memset(s.values,0,sizeof(s.values));memset(s.times,0,sizeof(s.times));
    sgpl_round_spatial_plan plan={.shared=&s,.first_round=0,.round_count=2,.frame_bytes=sizeof(device_frame),.partitions=2,
        .reserve=reserve_round,.setup=setup_round,.partition=cpu_batch,.finalize=finish_round,.release=release_round};
    assert(sgpl_exec_dag_round_spatial_gpu(&plan,4,"timed",sizeof(device_frame),fields,3,NULL,0)==0);
    assert(s.setups==2&&s.finishes==2&&s.reserves==2&&s.releases==2&&s.cpu_calls==0);
    fprintf(stderr,"GPU intervals: r0 [%llu,%llu] [%llu,%llu], r1 [%llu,%llu] [%llu,%llu]\n",
        (unsigned long long)s.times[0][0],(unsigned long long)s.times[0][1],(unsigned long long)s.times[0][2],(unsigned long long)s.times[0][3],
        (unsigned long long)s.times[1][0],(unsigned long long)s.times[1][1],(unsigned long long)s.times[1][2],(unsigned long long)s.times[1][3]);
    assert(overlap(s.times[0][0],s.times[0][1],s.times[0][2],s.times[0][3])||overlap(s.times[1][0],s.times[1][1],s.times[1][2],s.times[1][3]));
    int across=0;for(int a=0;a<2;++a)for(int b=0;b<2;++b)across|=overlap(s.times[0][2*a],s.times[0][2*a+1],s.times[1][2*b],s.times[1][2*b+1]);assert(across);
    puts("PASS device timer: two rounds and two batches overlap in one DAG");
    /* Three source phases, joined before the next source frame is reset. */
    int phase_overlap=0;
    for(int source=0;source<3;++source) {
        device_frame f={s.values[source],s.values[source]+1,s.times[source],source};memset(f.a,0,36);memset(f.times,0,48);
        assert(sgpl_gpu_callback_execute("timed",&f,sizeof(f),fields,3,3,4)==0);
        assert(f.a[0]==11&&f.a[2]==13&&f.a[4]==15);
        for(int a=0;a<3;++a)for(int b=a+1;b<3;++b)phase_overlap|=overlap(f.times[2*a],f.times[2*a+1],f.times[2*b],f.times[2*b+1]);
    }
    assert(phase_overlap);
    memset(&s,0,sizeof(s));s.fail_reserve_round=-1;
    device_frame serial={s.values[0],s.values[0]+1,s.times[0],0};
    sgpl_push_thread_budget(1);
    assert(sgpl_gpu_callback_execute("timed",&serial,sizeof(serial),fields,3,2,1)==0);
    assert(sgpl_current_thread_budget()==1);sgpl_pop_thread_budget();
    assert(overlap(serial.times[0],serial.times[1],serial.times[2],serial.times[3]));
    puts("PASS source-phase overlap ignores CPU grants and retains source joins");
    memset(s.values,0,sizeof(s.values));memset(s.times,0,sizeof(s.times));
    nested_gpu_fixture nested={.frame={s.values[0],s.values[0]+1,s.times[0],0}};
    memcpy(nested.fields,fields,sizeof(fields));
    sgpl_dag_template host={.node_count=2,.node_fn=nested_gpu_node};
    assert(autograph_execute_dag(&host,&nested,2)==0);
    assert(overlap(nested.frame.times[0],nested.frame.times[1],nested.frame.times[2],nested.frame.times[3]));
    puts("PASS GPU dispatch overlaps when called from a CPU scheduler node");
    memset(&s,0,sizeof(s));s.fail_reserve_round=-1;plan.round_count=9;
    assert(sgpl_exec_dag_round_spatial_gpu(&plan,4,"timed",sizeof(device_frame),fields,3,NULL,0)==0);
    assert(s.setups==9&&s.finishes==9&&s.reserves==9&&s.releases==9);
    memset(&s,0,sizeof(s));s.fail_reserve_round=1;plan.round_count=2;
    assert(sgpl_gpu_round_spatial_execute(&plan,4,"timed",sizeof(device_frame),fields,3,NULL,0)==1);
    assert(s.setups==0&&s.finishes==0&&s.reserves==2&&s.releases==2);
    memset(&s,0,sizeof(s));s.fail_reserve_round=1;plan.round_count=9;
    assert(sgpl_exec_dag_round_spatial_gpu(&plan,4,"timed",sizeof(device_frame),fields,3,NULL,0)<0);
    /* Test build uses a 32-node descriptor capacity. Seven groups fit in the
     * first window; round 1 belongs to the next contiguous-group window. */
    assert(s.setups==7&&s.finishes==7&&s.cpu_calls==0&&s.reserves==s.releases);
    for(const char **fault=(const char*[]){"upload","launch:3","sync:3","download",NULL};*fault;++fault) {
        memset(&s,0,sizeof(s));s.fail_reserve_round=-1;plan.round_count=2;setenv("SGPL_GPU_FAIL",*fault,1);
        assert(sgpl_exec_dag_round_spatial_gpu(&plan,4,"timed",sizeof(device_frame),fields,3,NULL,0)<0);
        assert(s.finishes==0&&s.cpu_calls==0&&s.reserves==s.releases);
    }
    unsetenv("SGPL_GPU_FAIL");puts("PASS bounded windows and failure without CPU replay");
}
static void resident_iterations(void) {
    int32_t *data=calloc(9,sizeof(int32_t));assert(data);
    sgpl_gpu_register_pointee("resident_test",data,9*sizeof(int32_t));
    struct {int32_t *a;} frame={data};
    sgpl_gpu_buffer buffers[]={{&frame,sizeof(frame),3},{data,9*sizeof(int32_t),3}};
    sgpl_gpu_relocation rel={0,1,0,0};sgpl_gpu_task task={.kernel="ordered",.index=0};
    sgpl_gpu_plan plan={.version=1,.schedule=SGPL_GPU_SPATIAL,.buffers=buffers,.buffer_count=2,
        .relocations=&rel,.relocation_count=1,.tasks=&task,.task_count=1};
    sgpl_gpu_residency_begin();
    sgpl_gpu_register_resident_buffer(data,9*sizeof(int32_t));
    for(int i=0;i<20;++i) assert(sgpl_gpu_execute_plan(&plan)==0);
    /* The host version is still unchanged. No eager D2H occurred. */
    assert(data[0]==0);sgpl_gpu_host_access(data,sizeof(*data),0);assert(data[0]==20);
    sgpl_gpu_host_access(data,sizeof(*data),1);data[0]+=7;
    for(int i=0;i<20;++i) assert(sgpl_gpu_execute_plan(&plan)==0);
    sgpl_gpu_host_access(data,sizeof(*data),0);assert(data[0]==47);
    /* Interior aliases use the same complete allocation mirror. */
    frame.a=data+1;buffers[1]=(sgpl_gpu_buffer){data+1,8*sizeof(int32_t),3};
    for(int i=0;i<3;++i) assert(sgpl_gpu_execute_plan(&plan)==0);
    sgpl_gpu_residency_end();assert(data[0]==47&&data[1]==3&&frame.a==data+1);
    sgpl_gpu_unregister_buffer(data);free(data);
    puts("PASS resident iterations, host writes, deferred download, aliases and scope publication");
    int32_t a[8]={0},b[8]={0};struct {int32_t *a,*b;int32_t same;} identity={a,b,0};
    sgpl_gpu_buffer readonly[]={{&identity,sizeof(identity),3},{a,sizeof(a),1},{b,sizeof(b),1}};
    sgpl_gpu_relocation pointers[]={{0,1,0,0},{0,2,8,0}};
    task.kernel="distinct";plan.buffers=readonly;plan.buffer_count=3;plan.relocations=pointers;plan.relocation_count=2;
    sgpl_gpu_residency_begin();assert(sgpl_gpu_execute_plan(&plan)==0);assert(identity.same==0);
    identity.b=a;pointers[1].target=1;assert(sgpl_gpu_execute_plan(&plan)==0);assert(identity.same==1);
    sgpl_gpu_residency_end();puts("PASS read-only storage retains distinct pointer identity and true aliases");
    int32_t left[4]={0},right[4]={0};frame.a=left;buffers[1]=(sgpl_gpu_buffer){left,sizeof(left),3};
    plan=(sgpl_gpu_plan){.version=1,.schedule=SGPL_GPU_SPATIAL,.buffers=buffers,.buffer_count=2,
        .relocations=&rel,.relocation_count=1,.tasks=&task,.task_count=1};task.kernel="ordered";
    sgpl_gpu_residency_begin();sgpl_gpu_register_resident_buffer(left,sizeof(left));sgpl_gpu_register_resident_buffer(right,sizeof(right));
    for(int i=0;i<20;++i) {
        int32_t *target=i%2?right:left;frame.a=target;buffers[1]=(sgpl_gpu_buffer){target,sizeof(left),3};
        assert(sgpl_gpu_execute_plan(&plan)==0);
    }
    assert(left[0]==0&&right[0]==0);sgpl_gpu_residency_end();assert(left[0]==10&&right[0]==10);
    sgpl_gpu_unregister_buffer(left);sgpl_gpu_unregister_buffer(right);
    puts("PASS changed pointer bindings retain both device versions across 20 iterations");
    static int32_t global_data[4];int32_t stack_a[4]={0},stack_b[4]={0};
    int32_t *older=(uintptr_t)stack_a>(uintptr_t)stack_b?stack_a:stack_b;
    int32_t *expired=older==stack_a?stack_b:stack_a;
    sgpl_gpu_residency_begin();sgpl_gpu_register_stack_buffer(older,sizeof(stack_a));
    sgpl_gpu_register_stack_buffer(expired,sizeof(stack_a));sgpl_gpu_register_resident_buffer(global_data,sizeof(global_data));
    for(int i=0;i<3;++i) {
        int32_t *target=i==0?older:i==1?expired:global_data;
        frame.a=target;buffers[1]=(sgpl_gpu_buffer){target,sizeof(stack_a),3};assert(sgpl_gpu_execute_plan(&plan)==0);
    }
    sgpl_gpu_stack_restore(older);sgpl_gpu_residency_end();
    assert(older[0]==1&&expired[0]==0&&global_data[0]==1);
    sgpl_gpu_unregister_buffer(older);sgpl_gpu_unregister_buffer(global_data);
    puts("PASS stack restore expires younger allocations and preserves older and global arrays");
}
int main(void) {
    setenv("SGPL_CLEANCUT_PARTITIONS","3",1);assert(autograph_gpu_clean_cut_partitions()==3);
    unsetenv("SGPL_CLEANCUT_PARTITIONS");assert(autograph_gpu_clean_cut_partitions()==64);
    int32_t values[9]={0};struct { int32_t *a,*b; } frame={values,values+1};
    sgpl_gpu_buffer buffers[]={{&frame,sizeof(frame),3},{values,sizeof(values),3},{values+1,8*sizeof(int32_t),3}};
    sgpl_gpu_relocation rel[]={{0,1,0,0},{0,2,8,0}};
    sgpl_gpu_task tasks[4];
    for(int i=0;i<4;++i) tasks[i]=(sgpl_gpu_task){"component",0,0,i*2};
    sgpl_gpu_plan p={.version=1,.schedule=4,.buffers=buffers,.buffer_count=3,
        .relocations=rel,.relocation_count=2,.tasks=tasks,.task_count=4,.worker_budget=4};
    assert(sgpl_gpu_execute_plan(&p)==0);
    assert(frame.a==values && frame.b==values+1);
    for(int i=0;i<4;++i) {assert(values[i*2]==i*2+11);assert(values[i*2+1]==1);}
    for(const char **fault=(const char*[]){"driver","module","stream:0","upload",NULL};*fault;++fault) {
        setenv("SGPL_GPU_FAIL",*fault,1);assert(sgpl_gpu_execute_plan(&p)==SGPL_GPU_UNAVAILABLE);
    }
    unsetenv("SGPL_GPU_FAIL");
    atomic_store(&fail_calloc,1);assert(sgpl_gpu_execute_plan(&p)==SGPL_GPU_UNAVAILABLE);
    /* GPU submission never creates or reserves a CPU worker team. */
    atomic_store(&fail_thread,1);assert(sgpl_gpu_execute_plan(&p)==0);
    assert(atomic_load(&fail_thread)==1);atomic_store(&fail_thread,0);
    setenv("SGPL_GPU_FAIL","reserve",1); assert(sgpl_gpu_execute_plan(&p)==SGPL_GPU_UNAVAILABLE);
    setenv("SGPL_GPU_FAIL","launch:0",1); assert(sgpl_gpu_execute_plan(&p)<0);
    setenv("SGPL_GPU_FAIL","sync:0",1); assert(sgpl_gpu_execute_plan(&p)<0);
    setenv("SGPL_GPU_FAIL","download",1); assert(sgpl_gpu_execute_plan(&p)<0);
    unsetenv("SGPL_GPU_FAIL");
    tasks[0].kernel="missing";assert(sgpl_gpu_execute_plan(&p)==SGPL_GPU_UNAVAILABLE);
    tasks[0].kernel="component";
    p.device_block_budget=1;p.worker_budget=1;
    assert(sgpl_gpu_execute_plan(&p)==0);p.device_block_budget=0;
    sgpl_push_thread_budget(1);assert(sgpl_gpu_execute_plan(&p)==0);sgpl_pop_thread_budget();
    sgpl_dag_relation_desc edges[2]={0};
    edges[0].source_node=0;edges[0].sink_node=1;edges[0].flags=SGPL_DAG_REL_FLAG_REALIZATION;
    edges[1]=edges[0];edges[1].source_node=1;edges[1].sink_node=0;
    p.relations=edges;p.relation_count=2;assert(sgpl_gpu_execute_plan(&p)==SGPL_GPU_ERR_INVALID);
    edges[1].source_node=1;edges[1].sink_node=2;tasks[0].kernel=tasks[1].kernel=tasks[2].kernel="ordered";
    p.task_count=3;values[0]=0;assert(sgpl_gpu_execute_plan(&p)==0);assert(values[0]==3);
    atomic_int replay=0;tasks[0].kernel="missing";
    assert(sgpl_exec_dag_gpu(&p,cpu_node,&replay)==0&&replay==3);
    tasks[0].kernel="ordered";replay=0;setenv("SGPL_GPU_FAIL","launch:0",1);
    assert(sgpl_exec_dag_gpu(&p,cpu_node,&replay)<0&&replay==0);unsetenv("SGPL_GPU_FAIL");
    setenv("SGPL_GPU_BLOCK_BUDGET","1",1);
    setenv("SGPL_GPU_BUDGET_FROM_LEDGER","1",1);
    gpup_set_block_budget(1);
    resident_iterations();legacy_launches();transfer_epochs();temporal_and_product();
    puts("PASS GPU invocation, alias relocation, and failure propagation");
}
