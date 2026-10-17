/* Stable filtering of joined append records. Dispatch depends only on CPU
 * features; validity epochs, duplicate values, and occurrence order are kept.
 * Callers prove capacity for every input slot before using the vector path. */
#if defined(__GNUC__) && defined(__ELF__)
#define SGPL_COMPACTION_FN __attribute__((noinline,section(".sgpl_compaction")))
#else
#define SGPL_COMPACTION_FN
#endif
static SGPL_COMPACTION_FN int64_t sgpl_append_compact_scalar(const int32_t *log,
    int64_t slots,uint32_t epoch,int32_t *out,int64_t at) {
  for(int64_t i=0;i<slots;++i) {
    /* The array tail remains language-visible. Rejected records must not
     * write it, even when backing capacity would permit speculative stores. */
    if(((const uint32_t *)log)[2*i]==epoch)out[at++]=log[2*i+1];
  }
  return at;
}
#if (defined(__x86_64__) || defined(__i386__)) && defined(__GNUC__)
#include <immintrin.h>
static const int32_t sgpl_append_permutation[16][8]={
  {0,0,0,0,0,0,0,0},
  {1,0,0,0,0,0,0,0},
  {3,0,0,0,0,0,0,0},
  {1,3,0,0,0,0,0,0},
  {5,0,0,0,0,0,0,0},
  {1,5,0,0,0,0,0,0},
  {3,5,0,0,0,0,0,0},
  {1,3,5,0,0,0,0,0},
  {7,0,0,0,0,0,0,0},
  {1,7,0,0,0,0,0,0},
  {3,7,0,0,0,0,0,0},
  {1,3,7,0,0,0,0,0},
  {5,7,0,0,0,0,0,0},
  {1,5,7,0,0,0,0,0},
  {3,5,7,0,0,0,0,0},
  {1,3,5,7,0,0,0,0},
};
static const int32_t sgpl_append_counts[16]={0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
__attribute__((target("avx2"))) static SGPL_COMPACTION_FN int64_t
sgpl_append_compact_avx2(const int32_t *log,int64_t slots,uint32_t epoch,
    int32_t *out,int64_t at) {
  int64_t i=0;
  __m256i tag=_mm256_set1_epi32((int32_t)epoch);
  __m128i lanes=_mm_setr_epi32(0,1,2,3);
  for(;slots-i>=4;i+=4) {
    __m256i records=_mm256_loadu_si256((const __m256i *)(log+2*i));
    unsigned bits=(unsigned)_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(records,tag)));
    unsigned mask=(bits&1)|((bits>>1)&2)|((bits>>2)&4)|((bits>>3)&8);
    if(!mask)continue;
    int32_t count=sgpl_append_counts[mask];
    __m256i order=_mm256_loadu_si256((const __m256i *)sgpl_append_permutation[mask]);
    __m128i values=_mm256_castsi256_si128(_mm256_permutevar8x32_epi32(records,order));
    /* Masked stores touch only selected values, including the last block
     * beside an allocation/page boundary. */
    _mm_maskstore_epi32(out+at,_mm_cmpgt_epi32(_mm_set1_epi32(count),lanes),values);
    at+=count;
  }
  for(;i<slots;++i)if(((const uint32_t *)log)[2*i]==epoch)out[at++]=log[2*i+1];
  return at;
}
#endif
static SGPL_COMPACTION_FN int64_t sgpl_append_compact_full(const int32_t *log,
    int64_t slots,uint32_t epoch,int32_t *out,int64_t at) {
#if (defined(__x86_64__) || defined(__i386__)) && defined(__GNUC__) && !defined(SGPL_COMPACTION_SCALAR_ONLY)
  if(__builtin_cpu_supports("avx2"))return sgpl_append_compact_avx2(log,slots,epoch,out,at);
#endif
  return sgpl_append_compact_scalar(log,slots,epoch,out,at);
}
