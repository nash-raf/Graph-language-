#ifndef SGPL_PARALLEL_RUNTIME_H
#define SGPL_PARALLEL_RUNTIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SGPL_LOOP_DOALL = 0,
    SGPL_LOOP_DOACROSS = 1,
} sgpl_loop_mode_t;

typedef enum
{
    SGPL_RUNTIME_PLAIN = 0,
    SGPL_RUNTIME_PRIVATIZED = 1,
} sgpl_runtime_kind_t;

typedef struct
{
    int32_t loop_id;
    int32_t mode;
    int32_t runtime_kind;
    int32_t reserved;
    int64_t env_size;
    const int64_t *priv_offsets;
    int32_t num_priv_targets;
    int32_t reserved2;
    const char *debug_name;
    int32_t doacross_waits_per_iter;
    int32_t doacross_posts_per_iter;
    int32_t has_doacross_profile;
    int32_t reserved3;
    int32_t doacross_num_sync_ids;
    int32_t reserved4;
    int64_t reserved5;
} sgpl_loop_profile_desc;

uint64_t sgpl_now_ns(void);

int32_t sgpl_should_parallelize_doall(
    const sgpl_loop_profile_desc *desc,
    int64_t start,
    int64_t end,
    int64_t step);

void sgpl_record_doall_serial_sample(
    const sgpl_loop_profile_desc *desc,
    int64_t start,
    int64_t end,
    int64_t step,
    uint64_t elapsed_ns);

void sgpl_record_doacross_serial_sample(
    const sgpl_loop_profile_desc *desc,
    int64_t start,
    int64_t end,
    int64_t step,
    uint64_t dep_elapsed_ns,
    uint64_t ind_elapsed_ns);

void sgpl_record_doacross_sync_sample(
    const sgpl_loop_profile_desc *desc,
    uint64_t wait_elapsed_ns_total,
    uint64_t post_elapsed_ns_total,
    uint64_t wait_count,
    uint64_t post_count);

int32_t sgpl_should_parallelize_doacross(
    const sgpl_loop_profile_desc *desc,
    int64_t start,
    int64_t end,
    int64_t step);

void sgpl_doacross_profile_enter(const sgpl_loop_profile_desc *desc);
void sgpl_doacross_profile_exit(const sgpl_loop_profile_desc *desc);

void parallel_for_runtime(
    int64_t start,
    int64_t end,
    int64_t step,
    void (*body)(int64_t, void *),
    void *env,
    int32_t needs_doacross,
    int32_t doacross_num_sync_ids);

void parallel_for_runtime_ex(
    int64_t start,
    int64_t end,
    int64_t step,
    void (*body)(int64_t, void *),
    void *env,
    int64_t env_size,
    const int64_t *priv_offsets,
    int32_t num_priv_targets,
    int32_t needs_doacross,
    int32_t doacross_num_sync_ids);

#ifdef __cplusplus
}
#endif

#endif