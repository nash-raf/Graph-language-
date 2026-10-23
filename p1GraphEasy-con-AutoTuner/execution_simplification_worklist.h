#ifndef SGPL_EXECUTION_SIMPLIFICATION_WORKLIST_H
#define SGPL_EXECUTION_SIMPLIFICATION_WORKLIST_H
#include <cstdint>
#include <cstddef>
#include <deque>
#include <vector>
/* Callback certificates remain immutable. Plan rewrites only add structural
 * capabilities. A callback rewrite needs a new proof and a fresh plan. Unknown
 * facts are absent. Each rule adds its license once: at most rules.size()
 * rewrites, with no cloning or irreversible IR lowering during convergence. */
namespace sgpl_execution {
enum Fact : uint64_t {
  LocalCallback=1, StableSnapshot=2, DestinationOwner=4,
  ImmutableMinimumCandidate=8, WholeClosure=16, Traversal=32,
  OrderedDestinationState=64, Closure=128, SourcesPruned=256,
  ContributionsPruned=512
};
struct Rule {uint32_t id;uint64_t requires,produces;};
struct Plan {uint64_t facts;uint32_t rules=0,visits=0,rewrites=0;};
inline Plan simplify(uint64_t facts,uint32_t requested,const std::vector<Rule> &rules) {
  Plan p{facts};std::deque<size_t> work;std::vector<bool> queued(rules.size(),true);
  for(size_t i=0;i<rules.size();++i)work.push_back(i);
  while(!work.empty()) {
    size_t i=work.front();work.pop_front();queued[i]=false;const Rule &r=rules[i];++p.visits;
    if(!(requested&r.id)||(p.rules&r.id)||(p.facts&r.requires)!=r.requires)continue;
    uint64_t added=r.produces&~p.facts;p.facts|=r.produces;p.rules|=r.id;++p.rewrites;
    for(size_t j=0;j<rules.size();++j)if(!queued[j]&&!(p.rules&rules[j].id)&&(rules[j].requires&added)) {
      queued[j]=true;work.push_back(j);
    }
  }
  return p;
}
inline std::vector<Rule> semanticRules() {
  const uint64_t callback=LocalCallback|StableSnapshot|DestinationOwner;
  // Consumers precede producers to exercise revisitation, not lucky ordering.
  return {{32,ImmutableMinimumCandidate|Traversal,SourcesPruned},
          {64,ImmutableMinimumCandidate|Traversal,ContributionsPruned},
          {512,callback,Traversal},{1024,callback,Traversal|OrderedDestinationState},
          {2048,callback|WholeClosure,Closure}};
}
}
#endif
