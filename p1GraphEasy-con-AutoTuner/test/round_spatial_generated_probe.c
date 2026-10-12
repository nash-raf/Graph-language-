#define _POSIX_C_SOURCE 200809L
#include "autotuner_runtime.h"
#include "parallel_runtime.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static sgpl_round_spatial_plan original;
static atomic_int started[32],finished[32],active[32],global_active,peak,errors,dispatching;
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static void maximum(atomic_int *a,int value){int x=atomic_load(a);while(x<value&&!atomic_compare_exchange_weak(a,&x,value)){} }
static int32_t setup(void *f,int64_t r){int status=original.setup(f,r);if(!status)atomic_fetch_add(&started[r],1);return status;}
static int32_t partition(void *f,int64_t p){
  /* The frame maps to its round through the reservation channel below. */
  extern int probe_frame_round(void *);
  int r=probe_frame_round(f);
  if(r<0||!atomic_load(&started[r])||atomic_load(&finished[r])||sgpl_current_thread_budget()!=1)atomic_fetch_add(&errors,1);
  atomic_fetch_add(&active[r],1);maximum(&peak,atomic_fetch_add(&global_active,1)+1);
  if(sgpl_configured_worker_count()>=4){double deadline=now()+2;struct timespec delay={0,100000};
    while(atomic_load(&global_active)<4&&now()<deadline)nanosleep(&delay,NULL);
  }
  int status=original.partition(f,p);
  atomic_fetch_sub(&global_active,1);atomic_fetch_sub(&active[r],1);return status;
}
static void *frames[32];static int rounds[32];static int frame_count;
int probe_frame_round(void *f){for(int i=frame_count-1;i>=0;--i)if(frames[i]==f)return rounds[i];return -1;}
static int32_t reserve(void *s,void *f,int64_t r){int i=frame_count++;frames[i]=f;rounds[i]=r;return original.reserve(s,f,r);}
static int32_t finalize(void *f,int64_t r){if(atomic_load(&active[r]))atomic_fetch_add(&errors,1);int status=original.finalize(f,r);atomic_fetch_add(&finished[r],1);return status;}
int32_t __real_sgpl_exec_dag_round_spatial(const sgpl_round_spatial_plan *,int32_t);
int32_t __wrap_sgpl_exec_dag_round_spatial(const sgpl_round_spatial_plan *p,int32_t budget){
 original=*p;sgpl_round_spatial_plan copy=*p;copy.reserve=reserve;copy.setup=setup;copy.partition=partition;copy.finalize=finalize;
 atomic_store(&dispatching,1);int status=__real_sgpl_exec_dag_round_spatial(&copy,budget);atomic_store(&dispatching,0);
 for(int r=0;r<2;++r)if(atomic_load(&started[r])!=1||atomic_load(&finished[r])!=1)atomic_fetch_add(&errors,1);
 if(sgpl_configured_worker_count()>=4&&atomic_load(&peak)!=4)atomic_fetch_add(&errors,1);
 if(status||atomic_load(&errors)){fprintf(stderr,"generated joint overlap failure: status=%d errors=%d peak=%d\n",status,atomic_load(&errors),atomic_load(&peak));exit(97);}
 fprintf(stderr,"generated joint overlap: peak=%d budget=%d\n",atomic_load(&peak),budget);return status;
}
int32_t __real_autograph_build_clean_cut(void *,int32_t);
int32_t __wrap_autograph_build_clean_cut(void *g,int32_t p){if(atomic_load(&dispatching))atomic_fetch_add(&errors,1);return __real_autograph_build_clean_cut(g,p>0?p:2);}
int32_t __real_sgpl_exec_spatial_partition(sgpl_exec_ctx *,int32_t);
int32_t __wrap_sgpl_exec_spatial_partition(sgpl_exec_ctx *ctx,int32_t p){
 if(ctx->temporal_scope!=SGPL_TEMPORAL_ROUND||ctx->dag_spatial!=1||ctx->dag_temporal!=1)atomic_fetch_add(&errors,1);
 return __real_sgpl_exec_spatial_partition(ctx,p);
}
