#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Frontend/OpenMP/OMPIRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/Local.h" // removeUnreachableBlocks
#include "llvm/IR/InstIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/IR/BasicBlock.h"
#include <cassert>
#include "parallel_loop_outline.h"

static void collectInnermostLoops(llvm::Loop *L, llvm::SmallVectorImpl<llvm::Loop *> &Out)
{
    if (L->getSubLoops().empty())
    {
        Out.push_back(L);
        return;
    }
    for (llvm::Loop *Sub : L->getSubLoops())
        collectInnermostLoops(Sub, Out);
}

llvm::Function *outlineLoop(llvm::Function &F, llvm::LoopInfo &LI, llvm::DominatorTree &DT, llvm::ScalarEvolution &SE, llvm::Value *&StartV, llvm::Value *&EndV, llvm::Value *&StepV)
{
    llvm::SmallVector<llvm::Loop *, 8> Innermost;

    for (llvm::Loop *Top : LI)
        collectInnermostLoops(Top, Innermost);
    if (Innermost.empty())
    {
        // llvm::outs() << "innermost empty";
        return nullptr;
    }

    llvm::CodeExtractorAnalysisCache CEAC(F);
    for (llvm::Loop *L : Innermost)
    {
        bool doall_check = false;

        // llvm::errs() << "Found loop\n";

        if (llvm::Instruction *Term = L->getHeader()->getTerminator())
        {
            if (llvm::MDNode *LoopMD = Term->getMetadata("my.loop.parallel"))
            {
                // llvm::errs() << "Found my.loop.parallel metadata node (operands = "
                //  << LoopMD->getNumOperands() << ")\n";

                for (unsigned i = 0; i < LoopMD->getNumOperands(); ++i)
                {
                    llvm::Metadata *Op = LoopMD->getOperand(i);
                    if (!Op)
                        continue;

                    // Case 1: MDString -> e.g. "parallel.type=DOALL"
                    if (auto *MDS = llvm::dyn_cast<llvm::MDString>(Op))
                    {
                        llvm::StringRef S = MDS->getString();
                        // llvm::errs() << "  MDString: " << S << "\n";
                        if (S.starts_with("parallel.type="))
                        {
                            llvm::StringRef Val = S.substr(strlen("parallel.type="));
                            if (Val.equals_insensitive("DOALL"))
                            {
                                // llvm::errs() << "  -> Detected DOALL parallel loop\n";
                                doall_check = true;
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }
                    // Case 2: nested MDNode (tuple-like metadata)
                    else if (auto *Inner = llvm::dyn_cast<llvm::MDNode>(Op))
                    {
                        // llvm::errs() << "  Nested MDNode with " << Inner->getNumOperands()
                        //              << " operands — recursing one level\n";
                        for (unsigned j = 0; j < Inner->getNumOperands(); ++j)
                        {
                            if (auto *S = llvm::dyn_cast_or_null<llvm::MDString>(Inner->getOperand(j)))
                            {
                                // llvm::errs() << "    nested MDString: " << S->getString() << "\n";
                            }
                            else if (auto *CAM = llvm::dyn_cast_or_null<llvm::ConstantAsMetadata>(Inner->getOperand(j)))
                            {
                                if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(CAM->getValue()))
                                {
                                    // llvm::errs() << "    nested ConstantInt: " << CI->getZExtValue() << "\n";
                                }
                            }
                            // extend handlers if needed
                        }
                    }
                    // Case 3: Constant-as-metadata (numbers attached as metadata)
                    else if (auto *CAM = llvm::dyn_cast<llvm::ConstantAsMetadata>(Op))
                    {
                        if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(CAM->getValue()))
                        {
                            bool Parallel = CI->getZExtValue() != 0;
                            // llvm::errs() << "  ConstantInt metadata -> parallel = " << Parallel << "\n";
                        }
                        else
                        {
                            // llvm::errs() << "  ConstantAsMetadata with non-int constant\n";
                        }
                    }
                    else
                    {
                        // llvm::errs() << "  Unknown metadata operand kind\n";
                    }
                } // for operands
            }
            else
            {
                // llvm::errs() << "No my.loop.parallel metadata on terminator\n";
            }
        }
        else
        {
            // llvm::errs() << "Header has no terminator?!\n";
        }

        if (!doall_check)
        {
            continue;
        }

        llvm::SmallVector<llvm::BasicBlock *, 8> LoopBody(L->blocks().begin(), L->blocks().end());
        if (LoopBody.empty())
        {
            // llvm::outs() << "empty";
            continue;
        }
        llvm::BasicBlock *Preheader = L->getLoopPreheader();
        llvm::BasicBlock *ExitBlock = L->getExitBlock();
        llvm::PHINode *indvar = nullptr;
        for (auto &I : *L->getHeader())
        {
            if (auto *phi = llvm::dyn_cast<llvm::PHINode>(&I))
            {
                // check if one incoming is from preheader and other from latch
                if (L->contains(phi->getIncomingBlock(1)) &&
                    phi->getIncomingBlock(0) == L->getLoopPreheader())
                {
                    indvar = phi;
                    break;
                }
            }
        }

        if (!indvar)
        {
            // llvm::outs() << "No canonical induction variable\n";
            return nullptr;
        }
        const llvm::SCEV *BackedgeCount = SE.getBackedgeTakenCount(L);
        const llvm::SCEV *Start = nullptr, *Step = nullptr, *End = nullptr;

        if (const llvm::SCEVAddRecExpr *AR = llvm::dyn_cast<llvm::SCEVAddRecExpr>(SE.getSCEV(indvar)))
        {
            Start = AR->getStart();
            Step = AR->getStepRecurrence(SE);
            End = SE.getAddExpr(Start, SE.getMulExpr(Step, BackedgeCount));
        }
        llvm::SCEVExpander Exp(SE, F.getParent()->getDataLayout(), "scevexp");
        StartV = Exp.expandCodeFor(Start, indvar->getType(), Preheader->getTerminator());
        StepV = Exp.expandCodeFor(Step, indvar->getType(), Preheader->getTerminator());
        EndV = Exp.expandCodeFor(End, indvar->getType(), Preheader->getTerminator());
        llvm::CodeExtractor CE(LoopBody, &DT);
        if (!CE.isEligible())
        {
            // llvm::outs() << "NOT ELIGIBLE";
            continue;
        }

        if (llvm::Function *Outlined = CE.extractCodeRegion(CEAC))
        {
            // make the outlined function externally visible and give it a friendly name
            Outlined->setLinkage(llvm::GlobalValue::ExternalLinkage);
            Outlined->setName("outlined_main_loopbody");

            // llvm::outs() << "\n\n\n\n\ntriggering";
            // llvm::outs().flush();

            // llvm::errs() << "\n\n\n\n\ntriggering\n";
            // llvm::errs().flush();
            // fprintf(stderr, "triggering\n");
            // fflush(stderr);

            llvm::Instruction *Term = Preheader->getTerminator();
            llvm::IRBuilder<> B(Term);
            llvm::Module *M = F.getParent();

            // Types
            llvm::Type *VoidTy = llvm::Type::getVoidTy(F.getContext());
            llvm::Type *Int64Ty = llvm::Type::getInt64Ty(F.getContext());
            llvm::Type *Int8PtrTy = llvm::Type::getInt8Ty(F.getContext())->getPointerTo();
            llvm::Type *LoopBodyFnTy = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false)->getPointerTo();

            // Declare parallel_for_runtime
            llvm::FunctionCallee ParallelForFunc = M->getOrInsertFunction(
                "parallel_for_runtime",
                llvm::FunctionType::get(
                    VoidTy,
                    {Int64Ty, Int64Ty, Int64Ty, LoopBodyFnTy, Int8PtrTy}, false));

            // Build env struct
            // --- SAFE: do not attempt to store Outlined->getArg(0) here ---
            // For now pass a NULL env pointer to runtime (populate env later properly)
            llvm::Value *EnvPtrCast = nullptr;
            llvm::Type *ArgTy = nullptr;
            if (Outlined->getFunctionType()->getNumParams() > 0)
                ArgTy = Outlined->getFunctionType()->getParamType(0);

            // Look in the entry block for an Alloca that likely corresponds to the captured 'a'
            llvm::BasicBlock &EntryBB = F.getEntryBlock();
            llvm::AllocaInst *FoundAlloca = nullptr;
            for (llvm::Instruction &I : EntryBB)
            {
                if (auto *AI = llvm::dyn_cast<llvm::AllocaInst>(&I))
                {
                    // If ArgTy is an actual pointer-to-some-type, compare allocated type.
                    if (ArgTy)
                    {
                        if (AI->getAllocatedType() == ArgTy || AI->getAllocatedType()->isArrayTy())
                        {
                            FoundAlloca = AI;
                            break;
                        }
                    }
                    else
                    {
                        // fallback: take the first alloca
                        FoundAlloca = AI;
                        break;
                    }
                }
            }

            if (FoundAlloca)
            {
                // cast the alloca pointer to i8* (env)
                llvm::PointerType *Int8PtrTyPtr = llvm::cast<llvm::PointerType>(Int8PtrTy);
                EnvPtrCast = B.CreateBitCast(FoundAlloca, Int8PtrTyPtr, "envptr");
                // llvm::errs() << "Using alloca as env: " << *FoundAlloca << "\n";
            }
            else
            {
                // fallback to null (should not happen for simple stack-alloc cases)
                llvm::PointerType *Int8PtrTyPtr = llvm::cast<llvm::PointerType>(Int8PtrTy);
                EnvPtrCast = llvm::ConstantPointerNull::get(Int8PtrTyPtr);
                // llvm::errs() << "WARNING: no alloca found, passing null env\n";
            }

            // Ensure wrapper symbol is declared (so CreateBitCast will not get nullptr)
            llvm::Function *WrapperFn = M->getFunction("wrapper");
            if (!WrapperFn)
            {
                llvm::FunctionType *WFT = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false);
                WrapperFn = llvm::Function::Create(WFT, llvm::Function::ExternalLinkage, "wrapper", M);
            }
            // Now it's safe to bitcast the wrapper to the expected function-pointer type
            llvm::Value *CastedWrapper = B.CreateBitCast(WrapperFn, LoopBodyFnTy);

            // Insert the parallel call
            llvm::Value *StartArg = StartV;
            llvm::Value *EndArg = EndV;
            llvm::Value *StepArg = StepV;

            // If any are not i64, sign-extend them to i64 (safe for small positive indices)
            if (StartArg->getType() != Int64Ty && StartArg->getType()->isIntegerTy())
                StartArg = B.CreateSExt(StartArg, Int64Ty, "start64");
            if (EndArg->getType() != Int64Ty && EndArg->getType()->isIntegerTy())
                EndArg = B.CreateSExt(EndArg, Int64Ty, "end64");
            if (StepArg->getType() != Int64Ty && StepArg->getType()->isIntegerTy())
                StepArg = B.CreateSExt(StepArg, Int64Ty, "step64");

            // Now create the call with proper types
            if (!Preheader || !ExitBlock || !Term)
            {
                // llvm::errs() << "PDG_OUTLINE: missing Preheader/ExitBlock/Term; skipping outlining for this loop\n";
                // Leave IR untouched; return nullptr to indicate we didn't outline.
                return nullptr;
            }

            // Insert the parallel call (at Preheader's terminator)
            llvm::SmallVector<llvm::Value *, 5> CallArgs;
            CallArgs.push_back(StartArg);
            CallArgs.push_back(EndArg);
            CallArgs.push_back(StepArg);
            CallArgs.push_back(CastedWrapper);
            CallArgs.push_back(EnvPtrCast);

            // Create the call and a branch to the original exit block.
            // We do this using the IRBuilder positioned at the Preheader terminator.
            llvm::CallInst *CI = llvm::cast<llvm::CallInst>(B.CreateCall(ParallelForFunc, CallArgs));
            (void)CI; // silence unused-var in release builds

            // Create the new branch to the loop exit.
            B.CreateBr(ExitBlock);

            // Erase the old terminator *only* after we inserted new terminating instructions.
            // This avoids leaving the block without a terminator temporarily.
            Term->eraseFromParent();

            // IMPORTANT: do NOT call removeUnreachableBlocks(F) here.
            // That can aggressively delete IR if something is slightly off.
            // Keep analyses invalidated and return; caller (opt) or later code can run
            // passes that update analyses. For now, making a valid transformation
            // that leaves the CFG consistent is sufficient.

            // llvm::errs() << "PDG_OUTLINE: inserted parallel call into function " << F.getName() << "\n";

            // Return the outlined function so the caller knows we succeeded.
            return Outlined;
        }
    }
    return nullptr;
}

using namespace llvm;

struct LoopOutlinerPass : public PassInfoMixin<LoopOutlinerPass>
{
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
    {
        // Skip functions that we (or CodeExtractor) already generated.
        // This prevents re-processing newly created outlined functions.
        if (F.getName().contains(".loopcond") || F.hasFnAttribute("outlined-loop"))
            return PreservedAnalyses::all();

        LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
        DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
        ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

        llvm::Value *Start = nullptr;
        llvm::Value *Step = nullptr;
        llvm::Value *End = nullptr;

        Function *outlined = outlineLoop(F, LI, DT, SE, Start, End, Step);

        if (outlined)
        {
            // mark it so we never process it again
            outlined->addFnAttr("outlined-loop");
            // errs() << "Outlined function: " << outlined->getName() << "\n";
            // llvm::outs() << "Start: ";
            // Start->print(llvm::outs());
            // llvm::outs() << "\n";
            // llvm::outs() << "Step: ";
            // Step->print(llvm::outs());
            // llvm::outs() << "\n";
            // llvm::outs() << "End: ";
            // End->print(llvm::outs());
            // llvm::outs() << "\n";
            // we changed the IR, analyses are invalidated
            return PreservedAnalyses::none();
        }

        return PreservedAnalyses::all();
    }
};

void registerLoopOutlinerPass(llvm::FunctionPassManager &FPM)
{
    // Now the pass type is visible, so we can add it to the FPM.
    FPM.addPass(LoopOutlinerPass());
}

// Register the pass as a plugin
extern "C" PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION, "LoopOutliner", "v0.1",
        [](PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                    if (Name == "loop-outliner")
                    {
                        FPM.addPass(LoopOutlinerPass());
                        return true;
                    }
                    return false;
                });
        }};
}
