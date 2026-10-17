#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "stable_compaction_runtime.inc"
static uint32_t seed=2358;
static uint32_t random32(void){seed=seed*1664525u+1013904223u;return seed;}
int main(void){
 size_t page=(size_t)sysconf(_SC_PAGESIZE);int checks=0;
 for(int n=0;n<=130;++n)for(int trial=0;trial<40;++trial){
  uint32_t epoch=trial&1?UINT32_MAX:17u;int prefix=trial%4;
  int32_t *log=calloc((size_t)(n?n:1)*2,4),expect[140];int64_t kept=prefix;
  for(int i=0;i<prefix;++i)expect[i]=-100-i;
  for(int i=0;i<n;++i){unsigned accepted=trial<16?((unsigned)trial>>(i%4))&1:random32()%3==0;
   ((uint32_t*)log)[2*i]=accepted?epoch:epoch-1;log[2*i+1]=(int32_t)(random32()%9);
   if(accepted)expect[kept++]=log[2*i+1];}
  for(int mode=0;mode<3;++mode){
   void *mapping=mmap(0,page*2,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
   if(mapping==MAP_FAILED||mprotect((char*)mapping+page,page,PROT_NONE))abort();
   int32_t *out=(int32_t*)((char*)mapping+page)-(n+prefix);
   for(int i=0;i<prefix;++i)out[i]=expect[i];
   int64_t got;
   if(mode==0)got=sgpl_append_compact_scalar(log,n,epoch,out,prefix);
#if (defined(__x86_64__) || defined(__i386__)) && defined(__GNUC__)
   else if(mode==1&&__builtin_cpu_supports("avx2"))got=sgpl_append_compact_avx2(log,n,epoch,out,prefix);
#endif
   else got=sgpl_append_compact_full(log,n,epoch,out,prefix);
   if(got!=kept||memcmp(out,expect,(size_t)kept*4)){fprintf(stderr,"mismatch n=%d trial=%d mode=%d\n",n,trial,mode);abort();}
   for(int i=(int)kept;i<n+prefix;++i)if(out[i]!=0){fprintf(stderr,"inactive array tail changed\n");abort();}
   munmap(mapping,page*2);++checks;
  }
  free(log);
 }
 printf("stable compaction: %d cases passed (all masks, tails, prefixes, duplicate values, high epochs, guard pages)\n",checks);return 0;
}
