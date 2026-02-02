#ifndef PDG_H
#define PDG_H

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include <string>

namespace llvm
{
    struct depEdge
    {
        unsigned source, target;
        std::string type;
        bool isLoop = false;
    };

    struct dependencyGraph
    {
        DenseMap<Instruction *, int> nodes;
        SmallVector<depEdge> edges;
    };
    dependencyGraph runPDGOnModule(Module &M);

    /// Convenience wrapper: run PDG and also write DOT to dotFilename.
    dependencyGraph runPDGOnModuleAndWriteDot(Module &M);

}

#endif