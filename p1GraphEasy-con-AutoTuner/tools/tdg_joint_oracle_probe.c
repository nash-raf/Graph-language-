#define SGPL_ENABLE_JOINT_SCHEDULER
#include "../parallel_runtime.c"

/* Small scheduling instances exported for an independent exhaustive oracle.
 * Abstract durations are not coefficients used by the runtime. */
int main(void)
{
    sgpl_configured_worker_count();
    unsigned seed=20261008;
    for(int c=0;c<48;++c) {
        int budget=c%2?6:4;
        sgpl_joint_job jobs[4]={0};
        sgpl_tdg_task_desc tasks[4]={0};
        int ids[2]={600,601};
        for(int j=0;j<4;++j) {
            seed=1664525u*seed+1013904223u;
            double serial=(1+(seed%17))*1000000.0;
            tasks[j].static_work_units=100;
            if(j<2) {
                tasks[j].num_loop_sites=1; tasks[j].loop_site_ids=&ids[j];
                jobs[j].count=1;
                jobs[j].events[0].id=ids[j];
                jobs[j].events[0].mode=(c%3==0 && j==1)?SGPL_LOOP_DOACROSS:SGPL_LOOP_DOALL;
                jobs[j].events[0].costs[1]=serial;
                for(int p=2;p<budget;++p)
                    jobs[j].events[0].costs[p]=serial/p+70000*(p+1);
            } else jobs[j].tail_ns=serial;
        }
        sgpl_joint_plan plan=sgpl_joint_optimize(tasks,jobs,4,budget,0);
        printf("{\"case\":%d,\"budget\":%d,\"jobs\":[",c,budget);
        for(int j=0;j<4;++j) {
            if(j) printf(",");
            printf("{\"loop\":%d,\"mode\":%d,\"costs\":[",jobs[j].count,
                   jobs[j].count?jobs[j].events[0].mode:0);
            for(int p=1;p<budget;++p) {
                if(p>1) printf(",");
                printf("%.9g",jobs[j].count?jobs[j].events[0].costs[p]:jobs[j].tail_ns);
            }
            printf("]}");
        }
        printf("],\"score\":%.9g,\"evaluations\":%d,\"backfill\":%d,\"parents\":%d,\"order\":[%d,%d,%d,%d],\"widths\":[%d,%d,1,1]}\n",
               plan.prediction_ns,plan.evaluations,plan.backfill,plan.max_parents,plan.order[0],plan.order[1],
               plan.order[2],plan.order[3],plan.widths[0][0],plan.widths[1][0]);
    }
    return 0;
}
