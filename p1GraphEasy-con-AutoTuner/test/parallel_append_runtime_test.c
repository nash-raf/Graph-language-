#define _POSIX_C_SOURCE 200809L

#include "../parallel_runtime.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct RoaringBitmap RoaringBitmap;

RoaringBitmap *roaring_bitmap_create(size_t arena_size, size_t initial_capacity)
{
    (void)arena_size;
    (void)initial_capacity;
    return NULL;
}

RoaringBitmap *roaring_bitmap_create_like(const RoaringBitmap *prototype)
{
    (void)prototype;
    return NULL;
}

void roaring_bitmap_or_inplace(RoaringBitmap *dst, RoaringBitmap *src)
{
    (void)dst;
    (void)src;
}

void roaring_bitmap_free(RoaringBitmap *bm)
{
    (void)bm;
}

void roaring_bitmap_set_thread_local_overrides(RoaringBitmap **originals,
                                               RoaringBitmap **replacements,
                                               int32_t count)
{
    (void)originals;
    (void)replacements;
    (void)count;
}

void roaring_bitmap_clear_thread_local_overrides(void)
{
}

typedef struct
{
    int32_t *values;
    int32_t *size;
    int32_t capacity;
} append_env;

static void append_iteration(int64_t value, void *raw_env)
{
    append_env *env = (append_env *)raw_env;
    env->values[(*env->size)++] = (int32_t)value;
}

static int run_order_test(int32_t count, int32_t needs_doacross)
{
    int32_t *values = (int32_t *)malloc((size_t)count * sizeof(int32_t));
    int32_t size = 0;
    append_env env = {values, &size, count};
    const int64_t offsets[] = {offsetof(append_env, values)};
    const int32_t kinds[] = {SGPL_PRIV_INT_APPEND};
    const int64_t aux[] = {
        offsetof(append_env, size),
        offsetof(append_env, capacity),
    };
    int32_t expected = 0;
    int32_t tid = 0;

    if (!values)
        return 0;

    parallel_for_runtime_ex(0, count, 1, append_iteration, &env, sizeof(env),
                            offsets, kinds, aux, 1, needs_doacross, needs_doacross);

    if (size != count)
    {
        fprintf(stderr, "size mismatch: got %d, expected %d\n", size, count);
        free(values);
        return 0;
    }

    for (tid = 0; tid < 4; ++tid)
    {
        int32_t value = tid;
        for (; value < count; value += 4, ++expected)
        {
            if (values[expected] != value)
            {
                fprintf(stderr,
                        "order mismatch at %d: got %d, expected %d\n",
                        expected, values[expected], value);
                free(values);
                return 0;
            }
        }
    }

    free(values);
    return 1;
}

static int run_capacity_test(void)
{
    int32_t values[10] = {0};
    int32_t size = 0;
    append_env env = {values, &size, 10};
    const int64_t offsets[] = {offsetof(append_env, values)};
    const int32_t kinds[] = {SGPL_PRIV_INT_APPEND};
    const int64_t aux[] = {
        offsetof(append_env, size),
        offsetof(append_env, capacity),
    };

    parallel_for_runtime_ex(0, 20, 1, append_iteration, &env, sizeof(env),
                            offsets, kinds, aux, 1, 0, 0);
    if (size != 0)
    {
        fprintf(stderr, "overflow merge published size %d\n", size);
        return 0;
    }
    return 1;
}

static int run_nonempty_destination_test(void)
{
    int32_t values[40] = {-7, -8, -9};
    int32_t size = 3;
    append_env env = {values, &size, 40};
    const int64_t offsets[] = {offsetof(append_env, values)};
    const int32_t kinds[] = {SGPL_PRIV_INT_APPEND};
    const int64_t aux[] = {
        offsetof(append_env, size),
        offsetof(append_env, capacity),
    };
    int32_t expected = 3;

    parallel_for_runtime_ex(0, 32, 1, append_iteration, &env, sizeof(env),
                            offsets, kinds, aux, 1, 0, 0);
    if (size != 35 || values[0] != -7 || values[1] != -8 || values[2] != -9)
        return 0;

    for (int32_t tid = 0; tid < 4; ++tid)
        for (int32_t value = tid; value < 32; value += 4, ++expected)
            if (values[expected] != value)
                return 0;
    return 1;
}

int main(void)
{
    if (setenv("SGPL_NUM_THREADS", "4", 1) != 0)
        return 1;

    if (!run_order_test(32, 0) ||
        !run_order_test(65536, 0) ||
        !run_order_test(65536, 1) ||
        !run_nonempty_destination_test() ||
        !run_capacity_test())
        return 1;

    puts("parallel append runtime tests passed");
    return 0;
}
