/* Benchmark-only backup of the removed two-phase policy. Include after the
 * test runtime. Production sources and binaries never include this file.
 * Full pre-promotion sources are in proof/tdg_dynamic_default_2026_10_09/before. */
static void sweep_ordinary_first(const sgpl_tdg_task_desc *tasks, int32_t count,
                                  int64_t work, int64_t span)
{
    if (!tasks || count<=0) return;
    int32_t budget=sgpl_budget_try_reserve(sgpl_budget_available_threads());
    int ordinary=0;
    for(int j=0;j<count;++j) ordinary+=tasks[j].num_loop_sites<=0 || !tasks[j].loop_site_ids;
    if(count<=budget || ordinary==0 || ordinary==count) {
        sgpl_run_tdg_fifo_reserved(tasks,count,work,span,budget);
    } else {
        sgpl_tdg_task_desc *ordered=malloc((size_t)count*sizeof(*ordered));
        if(!ordered) sgpl_run_tdg_fifo_reserved(tasks,count,work,span,budget);
        else {
            int oi=0,li=ordinary;
            for(int j=0;j<count;++j)
                ordered[(tasks[j].num_loop_sites<=0 || !tasks[j].loop_site_ids) ? oi++ : li++]=tasks[j];
            sgpl_run_tdg_fifo_reserved(ordered,ordinary,work,span,budget);
            sgpl_run_tdg_fifo_reserved(ordered+ordinary,count-ordinary,work,span,budget);
            free(ordered);
        }
    }
    sgpl_budget_release(budget);
}
