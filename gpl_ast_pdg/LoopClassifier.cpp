#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "DependencyGraph.h"

enum class ParallelType
{
  DOALL,
  DOACROSS,
  SEQUENTIAL
};

static llvm::StringRef parallelTypeToString(ParallelType type)
{
  switch (type)
  {
  case ParallelType::DOALL:
    return "DOALL";
  case ParallelType::DOACROSS:
    return "DOACROSS";
  default:
    return "SEQUENTIAL";
  }
}

static ParallelType classifyLoop(llvm::Loop *L, llvm::DependencyGraph &G)
{

  llvm::SmallPtrSet<const llvm::Instruction *, 32> loopInsts;

  for (llvm::BasicBlock *BB : L->getBlocks())
  {
    for (llvm::Instruction &I : *BB)
    {
      loopInsts.insert(&I);
    }
  }

  bool hasRaw = false;
  bool hasWarOrWaw = false;

  for (auto &E : G.Edges)
  {
    if (!llvm::StringRef(E.type).contains("loop_carried"))
      continue;
    llvm::Instruction *src = G.getInstByID(E.source);
    llvm::Instruction *tgt = G.getInstByID(E.target);

    if (!src || !tgt)
      continue;

    if (!loopInsts.count(src) || !loopInsts.count(tgt))
      continue;

    if (llvm::StringRef(E.type).contains("RAW"))
    {
      hasRaw = true;
    }
    else if (llvm::StringRef(E.type).contains("WAR") || llvm::StringRef(E.type).contains("WAW"))
    {
      hasWarOrWaw = true;
    }
  }

  if (!hasRaw && !hasWarOrWaw)
  {
    return ParallelType::DOALL;
  }
  else if (hasRaw && !hasWarOrWaw)
  {
    return ParallelType::DOACROSS;
  }
  else
  {
    return ParallelType::SEQUENTIAL;
  }
}

struct LoopClassifier : public llvm::AnalysisInfoMixin<LoopClassifier>
{
  static llvm::AnalysisKey Key;

  /// Result holds a map Loop* -> ParallelType
  using Result = llvm::DenseMap<const llvm::Loop *, ParallelType>;

  Result run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM)
  {

    auto &DG = FAM.template getResult<llvm::DependencyGraphAnalysis>(F);
    auto &LI = FAM.template getResult<llvm::LoopAnalysis>(F);

    Result R;

    for (llvm::Loop *L : LI)
    {
      R[L] = classifyLoop(L, DG);
      for (llvm::Loop *Sub : L->getSubLoops())
        R[Sub] = classifyLoop(Sub, DG);
    }

    return R;
  }
};

struct LoopClassifierPrinter : public llvm::PassInfoMixin<LoopClassifierPrinter>
{
  // runs over each Function in the Module
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM)
  {
    // Pull in classifier result
    auto &ClassMap = FAM.template getResult<LoopClassifier>(F);

    if (ClassMap.empty())
      return llvm::PreservedAnalyses::all();

    llvm::errs() << "=== Loop classification for function '"
                 << F.getName() << "' ===\n";

    for (auto &KV : ClassMap)
    {
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

struct LoopMetadataInjector : llvm::PassInfoMixin<LoopMetadataInjector>
{
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM)
  {
    auto &classMap = FAM.getResult<LoopClassifier>(F);
    llvm::LLVMContext &ctx = F.getContext();

    for (auto &[loop, type] : classMap)
    {
      if (!loop)
        continue;
      llvm::BasicBlock *header = loop->getHeader();
      if (!header)
        continue;
      llvm::Instruction *terminator = header->getTerminator();
      if (!terminator)
        continue;

      if (type != ParallelType::DOALL)
        continue;

      // --- Build new operand list, preserving existing operands ---
      // We will create: distinct !{!self, "DOALL", !{ "llvm.loop.parallelize.enable", i1 true }, !{ "llvm.loop.vectorize.enable", i1 true }, <existing-other-operands> }

      llvm::SmallVector<llvm::Metadata *, 8> ops;
      ops.push_back(nullptr); // placeholder for self-ref at index 0

      // Simple DOALL tag (direct MDString)
      ops.push_back(llvm::MDString::get(ctx, "DOALL"));

      // Boolean style parallelize enable node (keep for tools that expect it)
      ops.push_back(llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, "llvm.loop.parallelize.enable"),
                                            llvm::ConstantAsMetadata::get(llvm::ConstantInt::getTrue(ctx))}));

      // Vectorize hint (keep what you had)
      ops.push_back(llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, "llvm.loop.vectorize.enable"),
                                            llvm::ConstantAsMetadata::get(llvm::ConstantInt::getTrue(ctx))}));

      // If there's an existing loop ID (from prior passes), append its non-self operands
      if (llvm::MDNode *OldID = terminator->getMetadata("llvm.loop"))
      {
        for (unsigned i = 1, e = OldID->getNumOperands(); i < e; ++i)
        {
          llvm::Metadata *Op = OldID->getOperand(i);
          if (Op)
            ops.push_back(Op);
        }
      }

      // Create distinct loop ID and make it self-referential
      llvm::MDNode *NewID = llvm::MDNode::getDistinct(ctx, ops);
      NewID->replaceOperandWith(0, NewID);

      // Attach it to the terminator (canonical place)
      terminator->setMetadata("llvm.loop", NewID);

      llvm::errs() << "[meta] injected DOALL metadata for loop header '" << header->getName() << "'\n";
    }

    // We modified IR by adding metadata -> conservatively say we changed things
    return llvm::PreservedAnalyses::none();
  }
};

llvm::AnalysisKey LoopClassifier::Key;

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo()
{
  return {
      LLVM_PLUGIN_API_VERSION,
      "CombinedPlugin",
      LLVM_VERSION_STRING,
      [](llvm::PassBuilder &PB)
      {
        // Register analyses
        PB.registerAnalysisRegistrationCallback(
            [](llvm::FunctionAnalysisManager &FAM)
            {
              FAM.registerPass([]
                               { return llvm::DependencyGraphAnalysis(); });
              FAM.registerPass([]
                               { return LoopClassifier(); });
            });

        // MODULE-LEVEL passes (these see the whole module at once)
        PB.registerPipelineParsingCallback(
            [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
               llvm::ArrayRef<llvm::PassBuilder::PipelineElement>)
            {
              if (Name == "dep-graph-builder")
              {
                MPM.addPass(llvm::DependencyGraphPrinter());
                return true;
              }
              return false;
            });

        // FUNCTION-LEVEL passes (these run for each function)
        PB.registerPipelineParsingCallback(
            [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
               llvm::ArrayRef<llvm::PassBuilder::PipelineElement>)
            {
              if (Name == "loop-classify-print")
              {
                FPM.addPass(LoopClassifierPrinter());
                return true;
              }
              if (Name == "loop-metadata-injector")
              {
                FPM.addPass(LoopMetadataInjector());
                return true;
              }
              return false;
            });
      }};
}
