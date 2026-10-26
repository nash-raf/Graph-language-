#include <atomic>
#include <cstdio>
#include <sched.h>
#include <unistd.h>
#ifdef _OPENMP
#include <omp.h>
#define PARFOR _Pragma("omp parallel for") for
#else
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#define PARFOR cilk_for
#endif
int main() {
    std::atomic<unsigned long long> workers{0}, cpus{0};
    PARFOR (int i=0; i<65536; ++i) {
        if ((i&4095)==0) usleep(1000); // Probe only; never part of benchmarks.
#ifdef _OPENMP
        int w=omp_get_thread_num();
#else
        int w=__cilkrts_get_worker_number();
#endif
        workers.fetch_or(1ULL<<w,std::memory_order_relaxed);
        cpus.fetch_or(1ULL<<sched_getcpu(),std::memory_order_relaxed);
    }
    std::printf("{\"workers_mask\":%llu,\"cpu_mask\":%llu}\n",workers.load(),cpus.load());
}
