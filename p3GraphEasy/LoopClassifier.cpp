#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "DependencyGraph.h"


enum class ParallelType {
    DOALL,
    DOACROSS,
    SEQUENTIAL
};

static llvm::StringRef parallelTypeToString(ParallelType type) {
    switch (type) {
        case ParallelType::DOALL: return "DOALL";
        case ParallelType::DOACROSS: return "DOACROSS";
        default: return "SEQUENTIAL";
    }
}

static ParallelType classifyLoop(llvm::Loop *L, llvm::DependencyGraph &G) {

    llvm::SmallPtrSet<const llvm::Instruction*, 32> loopInsts;

    for(llvm::BasicBlock *BB: L->getBlocks()) {
        for(llvm::Instruction &I: *BB) {
            loopInsts.insert(&I);
        }
    }

    bool hasRaw = false;
    bool hasWarOrWaw = false;

    for(auto &E: G.Edges) {
        if(!llvm::StringRef(E.type).contains("loop_carried")) continue;
        llvm::Instruction *src = G.getInstByID(E.source);
        llvm::Instruction *tgt = G.getInstByID(E.target);

        if(!src || !tgt) continue;

        if(!loopInsts.count(src) || !loopInsts.count(tgt)) continue;

        if(llvm::StringRef(E.type).contains("RAW")) {
            hasRaw = true;
        } else if(llvm::StringRef(E.type).contains("WAR") || llvm::StringRef(E.type).contains("WAW")) {
            hasWarOrWaw = true;
        }
    }

    if(!hasRaw && !hasWarOrWaw) {
        return ParallelType::DOALL;
    } else if(hasRaw && !hasWarOrWaw) {
        return ParallelType::DOACROSS;
    } else {
        return ParallelType::SEQUENTIAL;
    }

}

struct LoopClassifier: public llvm::AnalysisInfoMixin<LoopClassifier> {
    static llvm::AnalysisKey Key;

  /// Result holds a map Loop* -> ParallelType
  using Result = llvm::DenseMap<const llvm::Loop*, ParallelType>;

  Result run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {

    auto &DG = FAM.template getResult<llvm::DependencyGraphAnalysis>(F);
    auto &LI = FAM.template getResult<llvm::LoopAnalysis>(F);

    Result R;

    for (llvm::Loop *L : LI) {
      R[L] = classifyLoop(L, DG);
      for (llvm::Loop *Sub : L->getSubLoops())
        R[Sub] = classifyLoop(Sub, DG);
    }

    return R;
  }
};

struct LoopClassifierPrinter : public llvm::PassInfoMixin<LoopClassifierPrinter> {
  // runs over each Function in the Module
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
    // Pull in classifier result
    auto &ClassMap = FAM.template getResult<LoopClassifier>(F);

    if (ClassMap.empty())
      return llvm::PreservedAnalyses::all();  

    llvm::errs() << "=== Loop classification for function '" 
           << F.getName() << "' ===\n";

    for (auto &KV : ClassMap) {
      const llvm::Loop *L = KV.first;
      ParallelType C = KV.second;

      // Print the loop header name and its class
      llvm::errs() << "  Loop at block '" 
             << L->getHeader()->getName() 
             << "': " << parallelTypeToString(C) << "\n";
    }
    llvm::errs() << "=== end ===\n\n";

    return llvm::PreservedAnalyses::all();
  }
};

struct LoopMetadataInjector: llvm::PassInfoMixin<LoopMetadataInjector> {
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
    auto &classMap = FAM.getResult<LoopClassifier>(F);
    llvm::LLVMContext &ctx = F.getContext();

    for(auto &[loop, type]: classMap) {
      llvm::BasicBlock *header = loop->getHeader();
      if(!header) continue;
      llvm::Instruction *terminator = header->getTerminator();
      if(!terminator) continue;

      llvm::SmallVector<llvm::Metadata*, 2> metadata;

      if(type == ParallelType::DOALL) {
        metadata.push_back(llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, "llvm.loop.parallelize.enable"), llvm::ConstantAsMetadata::get(llvm::ConstantInt::getTrue(ctx))}));

        metadata.push_back(llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, "llvm.loop.vectorize.enable"), llvm::ConstantAsMetadata::get(llvm::ConstantInt::getTrue(ctx))}));

        llvm::MDNode* loopID = llvm::MDNode::getDistinct(ctx, metadata);
        terminator->setMetadata("llvm.loop", loopID);

      }
      // metadata.push_back(llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, "llvm.loop.parallelizable.enable"),
      // llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(llvm::ConstantInt::getTrue(ctx)));
    }
    return llvm::PreservedAnalyses::none();
  }
};


llvm::AnalysisKey LoopClassifier::Key;

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "CombinedPlugin",
    LLVM_VERSION_STRING,
    [](llvm::PassBuilder &PB) {
      // Register analyses
      PB.registerAnalysisRegistrationCallback(
        [](llvm::FunctionAnalysisManager &FAM) {
          FAM.registerPass([] { return llvm::DependencyGraphAnalysis(); });
          FAM.registerPass([] { return LoopClassifier(); });
        });

      // MODULE-LEVEL passes (these see the whole module at once)
      PB.registerPipelineParsingCallback(
        [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
           llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
          if (Name == "dep-graph-builder") {
            MPM.addPass(llvm::DependencyGraphPrinter());
            return true;
          }
          return false;
        });

      // FUNCTION-LEVEL passes (these run for each function)
      PB.registerPipelineParsingCallback(
        [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
           llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
          if (Name == "loop-classify-print") {
            FPM.addPass(LoopClassifierPrinter());
            return true;
          }
          if (Name == "loop-metadata-injector") {
            FPM.addPass(LoopMetadataInjector());
            return true;
          }
          return false;
        });
    }
  };
}

