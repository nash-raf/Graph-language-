// Reproducible synthetic graph generator for the /algo benchmarks.
//   gcc -O2 -o gen_graph gen_graph.c
//   ./gen_graph 2000000 16000000 out_unweighted.txt out_weighted.txt
// Random spanning tree (=> connected, zero isolated vertices) + uniform random
// extra edges, deduplicated.  Both output files carry the identical edge list.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint64_t s0, s1;
static inline uint64_t rnd(void){            // xoroshiro128+
    uint64_t a=s0, b=s1, r=a+b; b^=a;
    s0=((a<<55)|(a>>9))^b^(b<<14); s1=(b<<36)|(b>>28); return r;
}
static int cmp_u64(const void *x, const void *y){
    uint64_t a=*(const uint64_t*)x, b=*(const uint64_t*)y;
    return a<b ? -1 : (a>b);
}
int main(int argc, char **argv){
    if (argc < 5){ fprintf(stderr,"usage: %s n m out_unweighted out_weighted\n",argv[0]); return 1; }
    uint64_t n = strtoull(argv[1],0,10), m = strtoull(argv[2],0,10);
    s0 = 0x9E3779B97F4A7C15ULL ^ n; s1 = 0xBF58476D1CE4E5B9ULL ^ m;

    uint32_t *perm = malloc(n*4);
    for (uint64_t i=0;i<n;i++) perm[i]=i;
    for (uint64_t i=n-1;i>0;i--){ uint64_t j=rnd()%(i+1); uint32_t t=perm[i];perm[i]=perm[j];perm[j]=t; }

    uint64_t *key = malloc(m*8), cnt = 0;
    for (uint64_t i=1;i<n;i++){                       // spanning tree
        uint64_t a=perm[i], b=perm[rnd()%i];
        key[cnt++] = (a<b) ? a*n+b : b*n+a;
    }
    while (cnt < m){                                  // extra random edges
        uint64_t a=rnd()%n, b=rnd()%n;
        if (a==b) continue;
        key[cnt++] = (a<b) ? a*n+b : b*n+a;
    }
    qsort(key, cnt, 8, cmp_u64);
    // NB: dedup only -- do NOT trim the sorted array to m, that biases toward
    // low vertex ids and can delete spanning-tree edges, breaking connectivity.
    uint64_t uniq=0;
    for (uint64_t i=0;i<cnt;i++) if (i==0 || key[i]!=key[i-1]) key[uniq++]=key[i];
    fprintf(stderr, "kept %llu unique edges\n", (unsigned long long)uniq);

    FILE *fu=fopen(argv[3],"w"), *fw=fopen(argv[4],"w");
    char *bu=malloc(1<<22), *bw=malloc(1<<22);
    setvbuf(fu,bu,_IOFBF,1<<22); setvbuf(fw,bw,_IOFBF,1<<22);
    for (uint64_t i=0;i<uniq;i++){
        uint64_t a=key[i]/n, b=key[i]%n; uint32_t wt=1+rnd()%100;
        fprintf(fu, "%llu %llu\n", (unsigned long long)a,(unsigned long long)b);
        fprintf(fw, "%llu %llu %u\n", (unsigned long long)a,(unsigned long long)b,wt);
    }
    fclose(fu); fclose(fw);
    return 0;
}
