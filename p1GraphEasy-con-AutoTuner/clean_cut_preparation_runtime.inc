/* Stable native-CSR partition construction. Consecutive source chunks retain
 * both source order and original row order. Each chunk owns its count/prefix
 * cells and disjoint output slices; no user callback runs during construction. */
#if defined(__GNUC__) && defined(__ELF__)
#define SGPL_CUT_FN __attribute__((noinline,section(".sgpl_cut_preparation")))
#else
#define SGPL_CUT_FN
#endif
#ifndef SGPL_CUT_PARALLEL_GRAIN
#define SGPL_CUT_PARALLEL_GRAIN 32768
#endif
typedef struct {
 AutoGraphMeta *meta;int32_t parts,chunks;int64_t *rowpos,*edgepos,*seen,*starts;
 int64_t **rp;int32_t **ci,**indir,**sci;_Atomic int error;
} SgplCutPreparation;
static SGPL_CUT_FN void sgpl_cut_count(void *opaque,int32_t worker,int32_t width) {
 SgplCutPreparation *j=opaque;AutoGraphMeta *m=j->meta;
 for(int32_t k=worker;k<j->chunks;k+=width) {
  int64_t *rows=j->rowpos+(size_t)k*j->parts,*edges=j->edgepos+(size_t)k*j->parts,*seen=j->seen+(size_t)k*j->parts;
  for(int64_t u=m->csr_n*k/j->chunks;u<m->csr_n*(k+1)/j->chunks;++u) {
   int64_t first=m->csr_row_ptr[u],last=m->csr_row_ptr[u+1];
   if(first<0||last<first||last>m->csr_m){atomic_store(&j->error,1);return;}
   for(int64_t e=first;e<last;++e) {
    int32_t v=m->csr_col_idx[e];if(v<0||v>=m->csr_n){atomic_store(&j->error,1);return;}
    int32_t p=(int32_t)((int64_t)v*j->parts/m->csr_n);++edges[p];
    if(seen[p]!=u+1){seen[p]=u+1;++rows[p];}
   }
  }
 }
}
static SGPL_CUT_FN void sgpl_cut_fill(void *opaque,int32_t worker,int32_t width) {
 SgplCutPreparation *j=opaque;AutoGraphMeta *m=j->meta;
 for(int32_t k=worker;k<j->chunks;k+=width) {
  int64_t *rows=j->rowpos+(size_t)k*j->parts,*edges=j->edgepos+(size_t)k*j->parts,*seen=j->seen+(size_t)k*j->parts;
  for(int64_t u=m->csr_n*k/j->chunks;u<m->csr_n*(k+1)/j->chunks;++u) {
   int32_t sp=(int32_t)(((u+1)*j->parts-1)/m->csr_n);
   for(int64_t e=m->csr_row_ptr[u];e<m->csr_row_ptr[u+1];++e) {
    int32_t v=m->csr_col_idx[e],p=(int32_t)((int64_t)v*j->parts/m->csr_n);
    if(seen[p]!=u+1){seen[p]=u+1;int64_t row=rows[p]++;j->indir[p][row]=(int32_t)u;j->rp[p][row]=edges[p];}
    j->ci[p][edges[p]++]=v;
    int64_t at=e-m->csr_row_ptr[j->starts[sp]];
    j->sci[sp][2*at]=(int32_t)u;j->sci[sp][2*at+1]=v;
   }
  }
 }
}
static SGPL_CUT_FN int32_t sgpl_build_csr_cut_parallel(AutoGraphMeta *m,int32_t parts) {
 if(!m||m->current_layout!=LAYOUT_CSR||!m->csr_row_ptr||!m->csr_col_idx||m->csr_n<=0||m->csr_n>INT32_MAX||m->csr_m<0)return 0;
 int32_t width=sgpl_configured_worker_count(),budget=sgpl_current_thread_budget();
 if(budget>0&&width>budget)width=budget;
 int64_t useful=m->csr_m/SGPL_CUT_PARALLEL_GRAIN;if(width>useful)width=(int32_t)useful;
 if(width<2)return 0;
 if(parts<=0) {
  int32_t configured=sgpl_configured_worker_count();if(configured<1)configured=1;
  const char *env=getenv("SGPL_CLEANCUT_PARTITIONS");parts=env?(int32_t)strtol(env,NULL,10):configured*4;
  if(parts<configured)parts=configured;
 }
 if(parts>m->csr_n)parts=(int32_t)m->csr_n;if(parts<1)return 0;
 int32_t chunks=width>16?64:width*4;if(chunks>m->csr_n)chunks=(int32_t)m->csr_n;
 /* Keep temporary count tables bounded independently of a user partition cap. */
 int64_t max_chunks=(8u*1024u*1024u)/((uint64_t)parts*24);
 if(max_chunks<1)return 0;if(chunks>max_chunks)chunks=(int32_t)max_chunks;
 size_t cells=(size_t)chunks*parts;
 int64_t *r=calloc(cells,8),*e=calloc(cells,8),*seen=calloc(cells,8),*start=NULL,*rc=NULL,*sc=NULL;
 int64_t **rp=NULL;int32_t **ci=NULL,**indir=NULL,**sci=NULL;int result=0;
 if(!r||!e||!seen)goto done;
 SgplCutPreparation job={.meta=m,.parts=parts,.chunks=chunks,.rowpos=r,.edgepos=e,.seen=seen};
 sgpl_run_preparation(sgpl_cut_count,&job,width);if(atomic_load(&job.error))goto done;
 start=calloc((size_t)parts+1,8);rc=calloc((size_t)parts,8);sc=calloc((size_t)parts,8);
 rp=calloc((size_t)parts,sizeof(*rp));ci=calloc((size_t)parts,sizeof(*ci));indir=calloc((size_t)parts,sizeof(*indir));sci=calloc((size_t)parts,sizeof(*sci));
 if(!start||!rc||!sc||!rp||!ci||!indir||!sci)goto done;
 for(int32_t p=0;p<=parts;++p)start[p]=(int64_t)p*m->csr_n/parts;
 for(int32_t p=0;p<parts;++p) {
  int64_t rows=0,edges=0;
  for(int32_t k=0;k<chunks;++k){size_t at=(size_t)k*parts+p;int64_t rn=r[at],en=e[at];r[at]=rows;e[at]=edges;rows+=rn;edges+=en;}
  sc[p]=m->csr_row_ptr[start[p+1]]-m->csr_row_ptr[start[p]];
  if((uint64_t)rows>SIZE_MAX/8-1||(uint64_t)edges>SIZE_MAX/4||(uint64_t)sc[p]>SIZE_MAX/8)goto done;
  rc[p]=rows;rp[p]=malloc(((size_t)rows+1)*8);ci[p]=malloc((size_t)(edges?edges:1)*4);
  indir[p]=malloc((size_t)(rows?rows:1)*4);sci[p]=malloc((size_t)(sc[p]?sc[p]:1)*8);
  if(!rp[p]||!ci[p]||!indir[p]||!sci[p])goto done;rp[p][rows]=edges;
 }
 memset(seen,0,cells*8);job.starts=start;job.rp=rp;job.ci=ci;job.indir=indir;job.sci=sci;
 sgpl_run_preparation(sgpl_cut_fill,&job,width);
 autograph_clean_cut_free(m);
 m->partition_start=start;m->partition_count=parts;m->push_rp=rp;m->push_ci=ci;m->push_indir=indir;m->push_row_count=rc;m->src_pairs=sci;m->src_pair_count=sc;
 start=NULL;rp=NULL;ci=NULL;indir=NULL;rc=NULL;sci=NULL;sc=NULL;result=parts;
done:
 if(rp)for(int32_t p=0;p<parts;++p)free(rp[p]);
 if(ci)for(int32_t p=0;p<parts;++p)free(ci[p]);
 if(indir)for(int32_t p=0;p<parts;++p)free(indir[p]);
 if(sci)for(int32_t p=0;p<parts;++p)free(sci[p]);
 free(rp);free(ci);free(indir);free(sci);free(start);free(rc);free(sc);free(r);free(e);free(seen);return result;
}
