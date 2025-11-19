#ifndef DEPGRAPH_DEPENDENCYGRAPH_H
#define DEPGRAPH_DEPENDENCYGRAPH_H

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"         
#include "llvm/Analysis/ScalarEvolution.h"  
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instruction.h"
#include <string>

namespace llvm {
  struct depEdge {
    unsigned source, target;
    std::string type;
  };

  struct DependencyGraph {
    DenseMap<const llvm::Instruction*, unsigned> IDMap;
    SmallVector<depEdge, 256> Edges;

    unsigned getInstID(const llvm::Instruction *I, unsigned &NextID);
    llvm::Instruction* getInstByID(unsigned ID) const;
  };

  struct DependencyGraphAnalysis : public AnalysisInfoMixin<DependencyGraphAnalysis> {
    static AnalysisKey Key;
    using Result = DependencyGraph;
    DependencyGraph run(Function &F, FunctionAnalysisManager &FAM);

    private:
      void analyzeLoopDependencies(Loop *L, DependenceInfo &DI, LoopInfo &LI,
                                  ScalarEvolution &SE, DependencyGraph &G,
                                  unsigned &NextID);
      void analyzeNonLoopDependencies(Function &F, DependenceInfo &DI, LoopInfo &LI,
                                      ScalarEvolution &SE, DependencyGraph &G,
                                      unsigned &NextID);
      void analyzeDependency(Instruction *I1, Instruction *I2, Dependence *dep,
                            Loop *curL, LoopInfo &LI, ScalarEvolution &SE,
                            DependencyGraph &G, unsigned &NextID);
      bool checkInductionVariableDependency(Instruction *I1, Instruction *I2,
                                          Loop *L, ScalarEvolution &SE);
      void addControlDependencies(Function &F, DependencyGraph &G, unsigned &NextID);
};

  
  struct DependencyGraphPrinter
    : public PassInfoMixin<DependencyGraphPrinter> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
      
      auto &FAM = MAM.template getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

      for (Function &F : M) {
        if (F.isDeclaration()) continue;

        
        auto &G = FAM.getResult<DependencyGraphAnalysis>(F);

        // emit one DOT graph per function:
        raw_ostream &OS = errs();
        std::string name = F.getName().str();
        OS << "digraph " << name << "_dep_graph {\n"
           << "  rankdir=TB;\n"
           << "  node [shape=box];\n";

        if (G.IDMap.empty()) {
          OS << "  node0 [label=\"No dependencies found\"];\n";
        } else {
          
          for (auto &P : G.IDMap) {
            std::string instStr;
            raw_string_ostream Ios(instStr);
            P.first->print(Ios);
            std::string esc;
            for (char c : Ios.str()) {
              switch(c){
               case '"': esc += "\\\""; break;
               case '\\': esc += "\\\\"; break;
               case '\n': esc += "\\n"; break;
               default: esc += c;
              }
            }
            OS << "  node" << P.second
               << " [label=\"" << esc << "\"];\n";
          }
          
          for (auto &E : G.Edges) {
            StringRef t = E.type;
            StringRef color = t.contains("loop_carried") ? "red"
                              : t.contains("loop_independent") ? "blue"
                              : t == "CONTROL"               ? "green"
                                                              : "black";
            StringRef style = t == "CONTROL" ? "dashed" : "solid";
            OS << "  node" << E.source << " -> node" << E.target
               << " [label=\"" << E.type
               << "\", color=\"" << color
               << "\", style=\"" << style << "\"];\n";
          }
        }
        OS << "}\n\n";
      }

      return PreservedAnalyses::all();
    }
  };
  
}

#endif // DEPGRAPH_DEPENDENCYGRAPH_H
