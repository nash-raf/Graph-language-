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
  std::cout<<"PASS composition worklist checks="<<checks<<" registration_orders=120 subsets=32\n";
}
