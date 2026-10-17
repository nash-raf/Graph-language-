#define SGPL_SPATIAL_FIXTURE_ONLY
#include "spatial_exec_test.c"
#include "autotuner_runtime.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static int errors;
#define CHECK(C) do {if(!(C)){fprintf(stderr,"failed line %d\n",__LINE__);++errors;}}while(0)
static void cleanup(sgpl_exec_ctx *c){for(int i=0;i<c->spatial_snapshot_count;i++)free(c->spatial_snapshots[i]);free(c->spatial_snapshots);free(c->spatial_snapshot_bytes);}
int main(void) {
 sgpl_exec_ctx a={0},b={0};int32_t ids[]={3,120,3};int32_t values[1024];for(int i=0;i<1024;i++)values[i]=i*7+1;
 a.array_sources=ids;a.array_source_count=3;b.array_sources=ids;b.array_source_count=3;
 CHECK(!autograph_exec_ctx_bind_source_snapshot_range(&a,0,values,1024,4));
 int32_t *frozen=a.spatial_snapshots[0];CHECK(frozen[3]==22&&frozen[120]==841);
 values[3]=999;values[120]=777;CHECK(!autograph_exec_ctx_bind_source_snapshot_range(&b,0,values,1024,4));
 CHECK(frozen[3]==22&&frozen[120]==841&&((int32_t*)b.spatial_snapshots[0])[3]==999);CHECK(a.spatial_snapshots[0]!=b.spatial_snapshots[0]);
 ids[1]=1024;CHECK(autograph_exec_ctx_bind_source_snapshot_range(&a,0,values,1024,4)<0);ids[1]=120;
 CHECK(autograph_exec_ctx_bind_source_snapshot_range(&a,0,NULL,1024,4)<0);
 int32_t small[]={8,7,6,5,4,3,2,1};int32_t dense_ids[]={0,1,2};a.array_sources=dense_ids;
 CHECK(!autograph_exec_ctx_bind_source_snapshot_range(&a,1,small,8,4));CHECK(!memcmp(a.spatial_snapshots[1],small,sizeof small));
 double reals[1024];for(int i=0;i<1024;i++)reals[i]=i*.25;a.array_sources=ids;
 CHECK(!autograph_exec_ctx_bind_source_snapshot_range(&a,2,reals,1024,8));CHECK(((double*)a.spatial_snapshots[2])[120]==30.);
 cleanup(&a);cleanup(&b);printf("source snapshot contract: %d failures\n",errors);return errors?1:0;
}
