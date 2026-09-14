#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
static double now(){ return omp_get_wtime(); }
int main(int argc, char **argv){
    const char *path=argv[1]; long n=atol(argv[2]);
    FILE *f=fopen(path,"rb"); fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET);
    char *buf=malloc(sz+1); if(fread(buf,1,sz,f)!=(size_t)sz){} buf[sz]=0; fclose(f);
    long cap=40000000,m=0; int *su=malloc(cap*4),*sv=malloc(cap*4); char *p=buf;
    while(*p){ long a=0,b=0; while(*p==' '||*p=='\n')p++; if(!*p)break;
        while(*p>='0'&&*p<='9')a=a*10+(*p++-'0'); while(*p==' ')p++;
        while(*p>='0'&&*p<='9')b=b*10+(*p++-'0'); while(*p&&*p!='\n')p++;
        su[m]=a; sv[m]=b; m++; }
    long M=2*m; int *src=malloc(M*4),*dst=malloc(M*4);
    for(long i=0;i<m;i++){src[i]=su[i];dst[i]=sv[i];src[m+i]=sv[i];dst[m+i]=su[i];}
    long *deg=calloc(n,sizeof(long)); for(long j=0;j<M;j++)deg[src[j]]++;
    double *c=malloc(n*sizeof(double));
    for(long i=0;i<n;i++)c[i]=deg[i]?0.85*(1.0/n)/deg[i]:0.0;
    int T=omp_get_max_threads();
    double *a1=calloc(n,sizeof(double)),*a2=calloc(n,sizeof(double)),*a3=calloc(n,sizeof(double));
    double t;
    t=now(); for(long j=0;j<M;j++) a1[dst[j]]+=c[src[j]];              double ts=now()-t;
    t=now();
    #pragma omp parallel for schedule(static)
    for(long j=0;j<M;j++)
        #pragma omp atomic
        a2[dst[j]]+=c[src[j]];                                          double ta=now()-t;
    t=now();
    {   double **priv=malloc(T*sizeof(double*));
        #pragma omp parallel
        { int id=omp_get_thread_num(); priv[id]=calloc(n,sizeof(double));
          #pragma omp for schedule(static)
          for(long j=0;j<M;j++) priv[id][dst[j]]+=c[src[j]];
        }
        for(int k=0;k<T;k++) for(long i=0;i<n;i++) a3[i]+=priv[k][i];   // fixed order -> deterministic
    }                                                                   double tp=now()-t;
    double s1=0,s2=0,s3=0; long d2=0,d3=0;
    for(long i=0;i<n;i++){s1+=a1[i];s2+=a2[i];s3+=a3[i]; if(a2[i]!=a1[i])d2++; if(a3[i]!=a1[i])d3++;}
    printf("  threads=%d  directed_edges=%ld\n",T,M);
    printf("  serial              : %.3fs  total=%.15f\n",ts,s1);
    printf("  atomic              : %.3fs  total=%.15f  speedup=%.2fx  entries!=serial=%ld\n",ta,s2,ts/ta,d2);
    printf("  per-thread partials : %.3fs  total=%.15f  speedup=%.2fx  entries!=serial=%ld\n",tp,s3,ts/tp,d3);
    return 0;
}
