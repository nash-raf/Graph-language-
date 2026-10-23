#ifndef SGPL_EXECUTION_PROOF_WORKLIST_H
#define SGPL_EXECUTION_PROOF_WORKLIST_H
#include <stdint.h>
#include <stddef.h>
/* Shared by compiler and C runtime. No allocation or mutation during planning.
 * Consumers are requeued only when a producer adds a required fact. */
typedef struct { uint32_t id; uint64_t requires,produces; } SgplProofRule;
typedef struct { uint64_t facts; uint32_t rules,visits,rewrites; } SgplProofPlan;
static inline SgplProofPlan sgpl_proof_simplify(uint64_t facts,uint32_t requested,
    const SgplProofRule *rules,size_t count) {
  SgplProofPlan p={facts,0,0,0};
  size_t queue[64],head=0,tail=0,used=0;unsigned char queued[64]={0};
  if(count>64)return p; /* unknown/unsupported rule table refuses */
  for(size_t i=0;i<count;++i){queue[tail++%64]=i;queued[i]=1;++used;}
  while(used) {
    size_t i=queue[head++%64];--used;queued[i]=0;
    const SgplProofRule *r=rules+i;++p.visits;
    if(!(requested&r->id)||(p.rules&r->id)||(p.facts&r->requires)!=r->requires)continue;
    uint64_t added=r->produces&~p.facts;p.facts|=r->produces;p.rules|=r->id;++p.rewrites;
    for(size_t j=0;j<count;++j)if(!queued[j]&&!(p.rules&rules[j].id)&&(rules[j].requires&added)) {
      queued[j]=1;queue[tail++%64]=j;++used;
    }
  }
  return p;
}
#define SGPL_EXEC_R13 4096u
#define SGPL_EXEC_R14 8192u
#define SGPL_EXEC_R15 16384u
/* Internal producer IDs do not appear in descriptors or user controls. */
#define SGPL_PROOF_GATE (1u<<24)
#define SGPL_PROOF_FRONTIER (1u<<25)
#define SGPL_PROOF_WEIGHT (1u<<26)
#define SGPL_FACT_GATE_STABLE (UINT64_C(1)<<16)
#define SGPL_FACT_GATE_WITNESS (UINT64_C(1)<<17)
#define SGPL_FACT_GATE_ENTAILED (UINT64_C(1)<<18)
#define SGPL_FACT_NO_GATE_STORAGE (UINT64_C(1)<<19)
#define SGPL_FACT_SET_OUTPUT (UINT64_C(1)<<20)
#define SGPL_FACT_VALID_INPUT (UINT64_C(1)<<21)
#define SGPL_FACT_BOUNDED_OUTPUT (UINT64_C(1)<<22)
#define SGPL_FACT_NO_DEGREE_SCAN (UINT64_C(1)<<23)
#define SGPL_FACT_TOPOLOGY_UNIQUE (UINT64_C(1)<<24)
#define SGPL_FACT_WEIGHT_FIRST_MATCH (UINT64_C(1)<<25)
#define SGPL_FACT_WEIGHT_IDENTITY (UINT64_C(1)<<26)
#define SGPL_FACT_DIRECT_WEIGHTS (UINT64_C(1)<<27)
static inline SgplProofPlan sgpl_proof_removals(uint64_t facts,uint32_t requested) {
  const SgplProofRule rules[]={
    {SGPL_EXEC_R13,SGPL_FACT_GATE_ENTAILED,SGPL_FACT_NO_GATE_STORAGE},
    {SGPL_EXEC_R14,SGPL_FACT_BOUNDED_OUTPUT,SGPL_FACT_NO_DEGREE_SCAN},
    {SGPL_EXEC_R15,SGPL_FACT_WEIGHT_IDENTITY,SGPL_FACT_DIRECT_WEIGHTS},
    {SGPL_PROOF_GATE,SGPL_FACT_GATE_STABLE|SGPL_FACT_GATE_WITNESS,SGPL_FACT_GATE_ENTAILED},
    {SGPL_PROOF_FRONTIER,SGPL_FACT_SET_OUTPUT|SGPL_FACT_VALID_INPUT,SGPL_FACT_BOUNDED_OUTPUT},
    {SGPL_PROOF_WEIGHT,SGPL_FACT_TOPOLOGY_UNIQUE|SGPL_FACT_WEIGHT_FIRST_MATCH,SGPL_FACT_WEIGHT_IDENTITY}};
  return sgpl_proof_simplify(facts,requested|SGPL_PROOF_GATE|SGPL_PROOF_FRONTIER|SGPL_PROOF_WEIGHT,rules,6);
}
#endif
