#include "../execution_simplification_worklist.h"
#include <algorithm>
#include <cassert>
#include <iostream>
int main() {
  using namespace sgpl_execution;auto definitions=semanticRules();unsigned checks=0;
  std::vector<unsigned> order{0,1,2,3,4};
  do {
    std::vector<Rule> rules;for(auto i:order)rules.push_back(definitions[i]);
    for(unsigned subset=0;subset<32;++subset)for(unsigned proof=0;proof<8;++proof) {
      uint32_t requested=0;for(unsigned i=0;i<5;++i)if(subset&(1u<<i))requested|=definitions[i].id;
      uint64_t facts=LocalCallback|StableSnapshot|DestinationOwner;
      if(proof&1)facts|=ImmutableMinimumCandidate;if(proof&2)facts|=WholeClosure;
      if(proof&4)facts&=~StableSnapshot;
      auto p=simplify(facts,requested,rules);uint32_t expected=0;
      if(!(proof&4)) {
        expected=requested&(512u|1024u);if(proof&2)expected|=requested&2048u;
        if((proof&1)&&(expected&1536u))expected|=requested&96u;
      }
      assert(p.rules==expected);assert(p.rewrites<=5);
      auto again=simplify(p.facts,requested,rules);assert(again.facts==p.facts&&again.rules==p.rules);
      ++checks;
    }
  } while(std::next_permutation(order.begin(),order.end()));
  auto p=simplify(LocalCallback|StableSnapshot|DestinationOwner|ImmutableMinimumCandidate,
                  512u|96u,definitions);
  assert(p.visits>definitions.size()&&p.rules==(512u|96u));
  auto chain=simplify(LocalCallback,7,{{4,SourcesPruned,ContributionsPruned},
      {2,Traversal,SourcesPruned},{1,LocalCallback,Traversal}});
  assert(chain.rules==7&&(chain.facts&ContributionsPruned)&&chain.rewrites==3);
  auto cycle=simplify(0,3,{{1,Traversal,SourcesPruned},{2,SourcesPruned,Traversal}});
  assert(!cycle.rules&&!cycle.rewrites);
  unsigned composed=0;
  // All subsets, missing-premise combinations, rotations and reverse orders.
  // Compare with an independent exhaustive closure, not registration order.
  for(unsigned proof=0;proof<128;++proof)for(unsigned subset=0;subset<256;++subset) {
    uint64_t facts=LocalCallback|DestinationOwner;
    if(proof&1)facts|=StableSnapshot;if(proof&2)facts|=ImmutableMinimumCandidate;
    if(proof&4)facts|=WholeClosure;if(proof&8)facts|=UnobservedFrontier;
    if(proof&16)facts|=CirculatingFrontier;if(proof&32)facts|=WeightedTraversal;
    if(proof&64)facts&=~LocalCallback;
    unsigned requested=0;for(unsigned i=0;i<8;++i)if(subset&(1u<<i))requested|=definitions[i].id;
    uint64_t expectedFacts=facts;unsigned expectedRules=0;
    bool changed=true;while(changed){changed=false;for(auto r:definitions)if((requested&r.id)&&!(expectedRules&r.id)&&(expectedFacts&r.requires)==r.requires){expectedRules|=r.id;expectedFacts|=r.produces;changed=true;}}
    for(unsigned order=0;order<16;++order) {
      auto rules=definitions;std::rotate(rules.begin(),rules.begin()+(order%8),rules.end());if(order&8)std::reverse(rules.begin(),rules.end());
      auto got=simplify(facts,requested,rules);assert(got.rules==expectedRules&&got.facts==expectedFacts&&got.rewrites<=8);++composed;
    }
  }
  for(unsigned facts=0;facts<64;++facts)for(unsigned subset=0;subset<8;++subset) {
    uint64_t f=0;const uint64_t premises[]={SGPL_FACT_GATE_STABLE,SGPL_FACT_GATE_WITNESS,SGPL_FACT_SET_OUTPUT,SGPL_FACT_VALID_INPUT,SGPL_FACT_TOPOLOGY_UNIQUE,SGPL_FACT_WEIGHT_FIRST_MATCH};
    for(unsigned i=0;i<6;++i)if(facts&(1u<<i))f|=premises[i];
    unsigned wanted=(subset&1?SGPL_EXEC_R13:0)|(subset&2?SGPL_EXEC_R14:0)|(subset&4?SGPL_EXEC_R15:0);
    auto got=sgpl_proof_removals(f,wanted);
    assert(!!(got.rules&SGPL_EXEC_R13)==((subset&1)&&((facts&3)==3)));
    assert(!!(got.rules&SGPL_EXEC_R14)==((subset&2)&&((facts&12)==12)));
    assert(!!(got.rules&SGPL_EXEC_R15)==((subset&4)&&((facts&48)==48)));
    assert(got.rewrites<=6);
  }
  std::cout<<"PASS composable proof removals cases="<<composed<<" runtime_subsets=512\n";
  for(unsigned facts=0;facts<16;++facts)for(unsigned subset=0;subset<8;++subset) {
    uint64_t f=0;const uint64_t premises[]={SGPL_FACT_COUNT_PURE,SGPL_FACT_FINAL_USES,SGPL_FACT_MAP_LOCAL,SGPL_FACT_GATE_DISJOINT};
    for(unsigned i=0;i<4;++i)if(facts&(1u<<i))f|=premises[i];
    unsigned wanted=(subset&1?SGPL_EXEC_R16:0)|(subset&2?SGPL_EXEC_R17:0)|(subset&4?SGPL_EXEC_R18:0);
    auto got=sgpl_iteration_lifetimes(f,wanted);
    assert(!!(got.rules&SGPL_EXEC_R16)==!!((subset&1)&&(facts&1)));
    assert(!!(got.rules&SGPL_EXEC_R17)==!!((subset&2)&&(facts&2)));
    assert(!!(got.rules&SGPL_EXEC_R18)==!!((subset&4)&&((facts&12)==12)));
    assert(got.rewrites<=3);
  }
  std::cout<<"PASS iteration lifetime worklist premise_subsets=128\n";
  std::cout<<"PASS composition worklist checks="<<checks<<" registration_orders=120 subsets=32\n";
}
