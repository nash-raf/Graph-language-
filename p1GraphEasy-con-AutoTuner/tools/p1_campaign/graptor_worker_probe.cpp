#include "graptor/graptor.h"
#include <atomic>
#include <iostream>
#include <sched.h>
#include <cstdlib>
static std::atomic<uint64_t> worker_mask{0},sum{0};
static std::atomic<unsigned> bad_affinity{0};
int main(int argc,char **argv) {
  if(argc!=2)return 2;
  unsigned width=std::strtoul(argv[1],nullptr,10);
  cpu_set_t allowed;CPU_ZERO(&allowed);
  const char *cpus=std::getenv("GRAPTOR_PROBE_CPUS");if(!cpus)return 3;
  while(*cpus){char *end=nullptr;unsigned c=std::strtoul(cpus,&end,10);if(end==cpus||c>=CPU_SETSIZE)return 4;CPU_SET(c,&allowed);cpus=*end?end+1:end;}
  parallel_loop(uint64_t(0),uint64_t(262144),64,[&](uint64_t i) {
#if GRAPTOR_PARALLEL == BACKEND_cilk
    int worker=__cilkrts_get_worker_number();
#else
    int worker=omp_get_thread_num();
#endif
    if(worker<0||worker>=int(width)){bad_affinity.fetch_add(1);return;}
    worker_mask.fetch_or(uint64_t(1)<<worker,std::memory_order_relaxed);
    sum.fetch_add(i,std::memory_order_relaxed);
    if(i%4096==0) {
      // This is a correctness probe, not a timing sample. Give newly started
      // workers time to steal a chunk before a tiny loop finishes on lane zero.
      usleep(1000);
      cpu_set_t actual;
      if(sched_getaffinity(0,sizeof(actual),&actual))bad_affinity.fetch_add(1);
      else for(int c=0;c<CPU_SETSIZE;++c)if(CPU_ISSET(c,&actual)&&!CPU_ISSET(c,&allowed))bad_affinity.fetch_add(1);
    }
  });
  auto mask=worker_mask.load();unsigned observed=__builtin_popcountll(mask);
  bool passed=graptor_num_threads()==width&&observed==width&&!bad_affinity.load()&&sum.load()==uint64_t(262144)*262143/2;
  std::cout<<"{\"passed\":"<<(passed?"true":"false")<<",\"configured_workers\":"<<graptor_num_threads()<<",\"observed_workers\":"<<observed<<",\"worker_mask\":"<<mask<<",\"affinity_violations\":"<<bad_affinity.load()<<"}\n";
  return passed?0:1;
}
