// wrapper.c
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

void wrapper(int64_t idx, void *env)
{
    if (!env)
    {
        fprintf(stderr, "wrapper: env is NULL\n");
        return;
    }

    // In out.ll, %a was the alloca [10 x i32], and your pass passed %a as 'env'.
    // We treat env as pointer to the first element (i32*).
    int32_t *a = (int32_t *)env;

    // // Sanity: guard against out-of-bounds weirdness (optional)
    // if (idx < 0 || idx >= 1000000)
    // { // 1000 just a safety cap
    //     fprintf(stderr, "wrapper: suspicious index %" PRId64 "\n", idx);
    //     return;
    // }

    // // a[(int)idx] = (int32_t)idx;

    // For debugging/verification, print the write:
    // fprintf(stderr, "wrapper: wrote a[%" PRId64 "] = %d\n", idx, (int)a[idx]);
}
