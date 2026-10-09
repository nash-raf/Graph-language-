/* SDK modules still call the normal symbol. This alias forwards to the ONE
 * frozen runtime in a reference executable; it owns no runtime state. */
#include "../parallel_runtime.h"

extern void sgpl_run_tdg_level_ordinary_first_reference(
    const sgpl_tdg_task_desc *, int32_t, int64_t, int64_t);

void sgpl_run_tdg_level(const sgpl_tdg_task_desc *tasks, int32_t count,
                        int64_t work, int64_t span)
{
    sgpl_run_tdg_level_ordinary_first_reference(tasks,count,work,span);
}

/* Production graph/GPU levels use FIFO. Reference executables deliberately
 * benchmark the old implementation through this test-only forwarding shim. */
void sgpl_run_tdg_level_fifo(const sgpl_tdg_task_desc *tasks, int32_t count,
                            int64_t work, int64_t span)
{
    sgpl_run_tdg_level_ordinary_first_reference(tasks,count,work,span);
}
