#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/PassManager.h"
#include "DependencyGraph.h"

using namespace llvm;

namespace {
  struct depEdge {
    unsigned source, target;
    std::string type;
  };
} 

namespace llvm {

AnalysisKey DependencyGraphAnalysis::Key;

DependencyGraph DependencyGraphAnalysis::run(Function &F, FunctionAnalysisManager &FAM) {
  DependencyGraph G;
  unsigned NextID = 1;

  auto &LI = FAM.getResult<LoopAnalysis>(F);
  auto &AA = FAM.getResult<AAManager>(F);
  auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

  DependenceInfo DI(&F, &AA, &SE, &LI);

  for (Loop *L : LI)
    analyzeLoopDependencies(L, DI, LI, SE, G, NextID);

  analyzeNonLoopDependencies(F, DI, LI, SE, G, NextID);
  addControlDependencies(F, G, NextID);

  return G;
}

unsigned DependencyGraph::getInstID(const llvm::Instruction *I, unsigned &NextID) {
  auto It = IDMap.find(I);
  if (It != IDMap.end()) {
    return It->second;
  }
  unsigned NewID = NextID++;
  IDMap[I] = NewID;
  return NewID;
}

llvm::Instruction* DependencyGraph::getInstByID(unsigned ID) const {
  for (const auto &Entry : IDMap) {
    if (Entry.second == ID) {
      return const_cast<llvm::Instruction*>(Entry.first);
    }
  }
  return nullptr;
}

void DependencyGraphAnalysis::analyzeLoopDependencies(
    Loop *L, DependenceInfo &DI, LoopInfo &LI, ScalarEvolution &SE,
    DependencyGraph &G, unsigned &NextID) {
    
  SmallVector<Instruction*, 64> memInsts;
  Function *F1 = L->getHeader()->getParent();

  for (BasicBlock &BB : *F1) {
    if (!L->contains(&BB)) continue;
    for (Instruction &I : BB)
      if (I.mayReadOrWriteMemory())
        memInsts.push_back(&I);
  }

  for (size_t i = 0; i < memInsts.size(); ++i)
    for (size_t j = i+1; j < memInsts.size(); ++j)
      if (auto dep = DI.depends(memInsts[i], memInsts[j], false))
        analyzeDependency(memInsts[i], memInsts[j], dep.get(), L, LI, SE, G, NextID);

  for (Loop *Sub : L->getSubLoops())
    analyzeLoopDependencies(Sub, DI, LI, SE, G, NextID);
}

void DependencyGraphAnalysis::analyzeNonLoopDependencies(
    Function &F, DependenceInfo &DI, LoopInfo &LI, ScalarEvolution &SE,
    DependencyGraph &G, unsigned &NextID) {
    
  for (BasicBlock &BB : F) {
    if (LI.getLoopFor(&BB)) continue;
    SmallVector<Instruction*, 32> memInsts;
    for (Instruction &I : BB)
      if (I.mayReadOrWriteMemory())
        memInsts.push_back(&I);

    for (size_t i = 0; i < memInsts.size(); ++i)
      for (size_t j = i+1; j < memInsts.size(); ++j)
        if (auto dep = DI.depends(memInsts[i], memInsts[j], false))
          analyzeDependency(memInsts[i], memInsts[j], dep.get(), nullptr, LI, SE, G, NextID);
  }
}

void DependencyGraphAnalysis::analyzeDependency(
    Instruction *I1, Instruction *I2, Dependence *dep, Loop *curL,
    LoopInfo &LI, ScalarEvolution &SE, DependencyGraph &G, unsigned &NextID) {
    
  std::string depType = "UNKNOWN";
  if      (dep->isFlow())   depType = "RAW";
  else if (dep->isAnti())   depType = "WAR";
  else if (dep->isOutput()) depType = "WAW";
  else if (dep->isInput())  depType = "RAR";

  bool isLoopCarried = false;
  std::string loopInfo;

  if (curL) {
    for (unsigned lvl = 1; lvl <= dep->getLevels(); ++lvl) {
      if (auto dist = dep->getDistance(lvl)) {
        if (!dist->isZero()) {
          isLoopCarried = true;
          std::string ds;
          raw_string_ostream os(ds);
          os << *dist;
          loopInfo = "_loop_carried[L" + std::to_string(lvl) + ",dist=" + os.str() + "]";
          break;
        }
      } else {
        unsigned dir = dep->getDirection(lvl);
        if (dir == Dependence::DVEntry::LT || dir == Dependence::DVEntry::GT) {
          isLoopCarried = true;
          loopInfo = "_loop_carried[L" + std::to_string(lvl) + ",dir=" + 
                    (dir == Dependence::DVEntry::LT ? "forward" : "backward") + "]";
          break;
        }
      }
    }

    if (!isLoopCarried && dep->getLevels() == 0) {
      if (checkInductionVariableDependency(I1, I2, curL, SE)) {
        isLoopCarried = true;
        loopInfo = "_loop_carried[induction_var]";
      }
    }

    depType += isLoopCarried ? loopInfo : "_loop_independent";
  } else {
    depType += "_sequential";
  }

  unsigned s = G.getInstID(I1, NextID);
  unsigned d = G.getInstID(I2, NextID);
  G.Edges.push_back({s, d, depType});
}

bool DependencyGraphAnalysis::checkInductionVariableDependency(
    Instruction *I1, Instruction *I2, Loop *L, ScalarEvolution &SE) {
    
  if (!I1 || !I2 || !L) return false;
  
  Value *ptr1 = nullptr, *ptr2 = nullptr;
  
  if (auto *load1 = dyn_cast<LoadInst>(I1)) {
    ptr1 = load1->getPointerOperand();
  } else if (auto *store1 = dyn_cast<StoreInst>(I1)) {
    ptr1 = store1->getPointerOperand();
  }
  
  if (auto *load2 = dyn_cast<LoadInst>(I2)) {
    ptr2 = load2->getPointerOperand();
  } else if (auto *store2 = dyn_cast<StoreInst>(I2)) {
    ptr2 = store2->getPointerOperand();
  }
  
  if (!ptr1 || !ptr2) return false;

  auto* gep1 = dyn_cast<GetElementPtrInst>(ptr1);
  auto* gep2 = dyn_cast<GetElementPtrInst>(ptr2);
  
  if (gep1 && gep2) {
    if (gep1->getPointerOperand() == gep2->getPointerOperand()) {
      return true;
    }
  }
  
  return false;
}

void DependencyGraphAnalysis::addControlDependencies(
    Function &F, DependencyGraph &G, unsigned &NextID) {
      
  for (BasicBlock &BB : F)
    for (Instruction &I : BB)
      if (isa<BranchInst>(I) || isa<CallInst>(I))
        for (User *U : I.users())
          if (auto *UI = dyn_cast<Instruction>(U)) {
            unsigned s = G.getInstID(&I, NextID);
            unsigned d = G.getInstID(UI, NextID);
            G.Edges.push_back({s, d, "CONTROL"});
          }
}

} 