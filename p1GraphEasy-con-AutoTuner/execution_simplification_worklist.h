#ifndef SGPL_EXECUTION_SIMPLIFICATION_WORKLIST_H
#define SGPL_EXECUTION_SIMPLIFICATION_WORKLIST_H
#include <cstdint>
#include <cstddef>
#include <deque>
#include <vector>
#include "execution_proof_worklist.h"
/* Callback certificates remain immutable. Plan rewrites only add structural
 * capabilities. A callback rewrite needs a new proof and a fresh plan. Unknown
 * facts are absent. Each rule adds its license once: at most rules.size()
 * rewrites, with no cloning or irreversible IR lowering during convergence. */
namespace sgpl_execution {
enum Fact : uint64_t {
  LocalCallback=1, StableSnapshot=2, DestinationOwner=4,
  ImmutableMinimumCandidate=8, WholeClosure=16, Traversal=32,
  OrderedDestinationState=64, Closure=128, SourcesPruned=256,
  ContributionsPruned=512, UnobservedFrontier=1024, CirculatingFrontier=2048,
  WeightedTraversal=4096, RangeInductionEligible=8192, WeightIdentityEligible=16384,
  SparseTraversal=32768
};
using Rule=SgplProofRule;
using Plan=SgplProofPlan;
inline Plan simplify(uint64_t facts,uint32_t requested,const std::vector<Rule> &rules) {
  return sgpl_proof_simplify(facts,requested,rules.data(),rules.size());
}
inline std::vector<Rule> semanticRules() {
  const uint64_t callback=LocalCallback|StableSnapshot|DestinationOwner;
  // Consumers precede producers to exercise revisitation, not lucky ordering.
  return {{32,ImmutableMinimumCandidate|Traversal,SourcesPruned},
          {64,ImmutableMinimumCandidate|Traversal,ContributionsPruned},
          {512,callback,Traversal|SparseTraversal},{1024,callback,Traversal|OrderedDestinationState},
          {2048,callback|WholeClosure,Closure},
          {SGPL_EXEC_R14,SGPL_FACT_SET_OUTPUT|CirculatingFrontier|SparseTraversal,RangeInductionEligible},
          {SGPL_EXEC_R15,Traversal|WeightedTraversal,WeightIdentityEligible},
          {128,ImmutableMinimumCandidate|UnobservedFrontier|Traversal,SGPL_FACT_SET_OUTPUT}};
}
}
#endif
