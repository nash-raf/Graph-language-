// One PageRank scatter step, three ways, on the real edge list.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

int main(int argc, char **argv) {
    const char *path = argv[1];
    long n = atol(argv[2]);
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    fread(buf, 1, sz, f); buf[sz] = 0; fclose(f);

    long cap = 40000000, m = 0;
    int *su = malloc(cap * 4), *sv = malloc(cap * 4);
    char *p = buf;
    while (*p) {                                   // parse "u v" per line
        long a = 0, b = 0;
        while (*p == ' ' || *p == '\n') p++;
        if (!*p) break;
        while (*p >= '0' && *p <= '9') a = a * 10 + (*p++ - '0');
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') b = b * 10 + (*p++ - '0');
        while (*p && *p != '\n') p++;
        su[m] = a; sv[m] = b; m++;
    }
    // symmetrise: every undirected edge scatters in both directions
    long M = 2 * m;
    int *src = malloc(M * 4), *dst = malloc(M * 4);
    for (long i = 0; i < m; i++) { src[i]=su[i]; dst[i]=sv[i]; src[m+i]=sv[i]; dst[m+i]=su[i]; }

    long *deg = calloc(n, sizeof(long));
    for (long j = 0; j < M; j++) deg[src[j]]++;
    double *contrib = malloc(n * sizeof(double));
    for (long i = 0; i < n; i++) contrib[i] = deg[i] ? 0.85 * (1.0/n) / deg[i] : 0.0;

    double *a1 = calloc(n, sizeof(double));
    double *a2 = calloc(n, sizeof(double));
    double *a3 = calloc(n, sizeof(double));

    for (long j = 0; j < M; j++) a1[dst[j]] += contrib[src[j]];          // SERIAL (truth)

    #pragma omp parallel for schedule(static)
    for (long j = 0; j < M; j++) a2[dst[j]] += contrib[src[j]];          // PARALLEL, NO atomics

    #pragma omp parallel for schedule(static)
    for (long j = 0; j < M; j++)
        #pragma omp atomic
        a3[dst[j]] += contrib[src[j]];                                   // PARALLEL + atomic

    double s1=0, s2=0, s3=0; long bad2=0, bad3=0;
    for (long i = 0; i < n; i++) {
        s1 += a1[i]; s2 += a2[i]; s3 += a3[i];
        if (a2[i] != a1[i]) bad2++;
        if (a3[i] != a1[i]) bad3++;
    }
    printf("  edges (directed)      : %ld   threads: %d\n", M, omp_get_max_threads());
    printf("  serial          total : %.15f\n", s1);
    printf("  parallel NO atomic    : %.15f   lost %.4f%%   wrong entries: %ld\n",
           s2, 100.0*(s1-s2)/s1, bad2);
    printf("  parallel WITH atomic  : %.15f   diff %.3g%%     wrong entries: %ld\n",
           s3, 100.0*(s1-s3)/s1, bad3);
    return 0;
}
