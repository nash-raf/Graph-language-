#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Frontend/OpenMP/OMPIRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/BasicBlock.h"
#include <cassert>
#include "parallel_loop_outline.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

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
                    // Case 3: Constant-as-metadata (numbers attached as metadata)
                    else if (auto *CAM = llvm::dyn_cast<llvm::ConstantAsMetadata>(Op))
                    {
                        if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(CAM->getValue()))
                        {
                            bool Parallel = CI->getZExtValue() != 0;
                            // llvm::errs() << "  ConstantInt metadata -> parallel = " << Parallel << "\n";
                        }
                    }
                } // for operands
            }
        }

        if (!doall_check)
        {
            continue;
        }

        // llvm::SmallVector<llvm::BasicBlock *, 8> LoopBody(L->blocks().begin(), L->blocks().end());

        llvm::SmallVector<llvm::BasicBlock *, 8> LoopBody;
        for (llvm::BasicBlock *BB : L->blocks())
        {
            if (BB != L->getHeader()) // exclude header so the PHI becomes an input param
                LoopBody.push_back(BB);
        }

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

        // ...existing code...
        llvm::SetVector<llvm::Value *> Inputs, Outputs;
        llvm::SetVector<llvm::Value *> SinkCands; // usually empty for loops

        // LLVM-18 signature:
        CE.findInputsOutputs(Inputs, Outputs, SinkCands);

        // --- store-scan: find memory destinations the loop writes to and add their underlying object
        for (llvm::BasicBlock *BB : LoopBody)
        {
            for (llvm::Instruction &I : *BB)
            {
                if (auto *SI = llvm::dyn_cast<llvm::StoreInst>(&I))
                {
                    llvm::Value *Ptr = SI->getPointerOperand();
                    llvm::Value *Base = llvm::getUnderlyingObject(Ptr, /*MaxLookup=*/64);
                    if (!Base)
                        continue;

                    // if the base is an instruction, ensure it is defined outside the loop
                    if (auto *BI = llvm::dyn_cast<llvm::Instruction>(Base))
                    {
                        if (L->contains(BI->getParent()))
                            continue; // base defined inside loop -> skip
                    }

                    // consider Alloca/Global/Argument or any value defined outside loop
                    if (!Inputs.count(Base) && !Outputs.count(Base))
                    {
                        Outputs.insert(Base);
                        llvm::errs() << "store-scan: added base to Outputs: " << *Base << "\n";
                    }
                }
            }
        }

        llvm::outs() << "Captured Inputs:\n";
        for (llvm::Value *V : Inputs)
        {
            llvm::outs() << "  - " << V << "\n";
        }

        llvm::outs() << "Captured Inputs:\n";
        for (llvm::Value *V : Inputs)
        {
            llvm::outs() << "  - " << *V << "\n";
        }
        llvm::outs() << "Captured Outputs:\n";

        for (llvm::Value *V : Outputs)
        {
            llvm::outs() << "  - " << *V << "\n";
        }

        if (llvm::Function *Outlined = CE.extractCodeRegion(CEAC))
        {
            // make the outlined function externally visible and give it a friendly name
            Outlined->setLinkage(llvm::GlobalValue::ExternalLinkage);
            Outlined->setName("outlined_main_loopbody");

            llvm::SmallVector<llvm::Value *, 16> Candidates;
            // Prefer outputs first (so output allocas / results are matched to Outlined's output params)
            for (llvm::Value *V : Outputs)
                Candidates.push_back(V);
            // then inputs as fallback
            for (llvm::Value *V : Inputs)
                Candidates.push_back(V);

            llvm::Value *IndVar = indvar; // your earlier detected canonical PHI (may be nullptr)

            // Map each Outlined arg to a candidate original value (or nullptr)
            llvm::SmallVector<llvm::Value *, 8> ArgOriginVals;
            ArgOriginVals.reserve(Outlined->arg_size());

            llvm::SmallPtrSet<llvm::Value *, 8> UsedCandidates;

            // !!!!!!!!!!locate outlined function Call site and map args!!!!!!!!!!!!!!!!
            for (llvm::BasicBlock &BB : F)
            {
                llvm::StringRef BBName = BB.getName();
                if (BBName == "codeRepl")
                {
                    for (llvm::Instruction &I : BB)
                    {
                        if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&I))
                        {
                            if (CI->getCalledFunction() == Outlined)
                            {
                                // llvm::errs() << "Found outlined function call: " << *CI << "\n";
                                for (unsigned ai = 0; ai < CI->getNumOperands() - 1; ++ai)
                                {
                                    llvm::Value *ArgOp = CI->getArgOperand(ai);
                                    ArgOriginVals.push_back(ArgOp);
                                }
                            }
                        }
                    }
                }
            }

            // Debug
            for (llvm::Value *U : Candidates)
            {
                llvm::errs() << " candidate: " << *U << "\n";
            }
            // Debug
            llvm::errs() << "ArgOriginVals mapping:\n";
            for (unsigned i = 0; i < ArgOriginVals.size(); ++i)
            {
                llvm::errs() << "  param[" << i << "] -> ";
                if (ArgOriginVals[i])
                    ArgOriginVals[i]->print(llvm::errs());
                else
                    llvm::errs() << "NULL";
                llvm::errs() << "\n";
            }

            // Debug
            llvm::errs() << "Captured Outputs:\n";
            for (llvm::Value *V : Outputs)
            {
                if (V)
                    V->print(llvm::errs());
                llvm::errs() << "\n";
            }
            int InductionParamIndex = -1;
            llvm::SmallVector<llvm::Type *, 8> NewEnvFieldTys;
            llvm::SmallVector<llvm::Value *, 8> NewEnvOriginVals;

            for (unsigned pi = 0; pi < ArgOriginVals.size(); ++pi)
            {
                llvm::Value *orig = ArgOriginVals[pi];
                llvm::Type *ParamTy = Outlined->getFunctionType()->getParamType(pi);
                if (orig == IndVar)
                {
                    InductionParamIndex = (int)pi;
                    continue; // index param will be provided by runtime (idx)
                }
                NewEnvFieldTys.push_back(ParamTy);
                NewEnvOriginVals.push_back(orig); // may be nullptr => will store null
            }

            llvm::Instruction *Term = Preheader->getTerminator();
            llvm::IRBuilder<> B(Term);
            llvm::Module *M = F.getParent();

            // Types
            llvm::Type *VoidTy = llvm::Type::getVoidTy(F.getContext());
            llvm::Type *Int64Ty = llvm::Type::getInt64Ty(F.getContext());
            llvm::Type *Int8PtrTy = llvm::Type::getInt8Ty(F.getContext())->getPointerTo();
            llvm::Type *LoopBodyFnTy = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false)->getPointerTo();

            llvm::StructType *NewEnvStructTy =
                llvm::StructType::create(F.getContext(), NewEnvFieldTys, "env.struct");

            // Allocate env on heap via malloc
            uint64_t NewEnvSize = M->getDataLayout().getTypeAllocSize(NewEnvStructTy);
            llvm::Value *SizeConst = llvm::ConstantInt::get(Int64Ty, NewEnvSize);
            llvm::FunctionCallee MallocFn = M->getOrInsertFunction("malloc",
                                                                   llvm::FunctionType::get(Int8PtrTy, {Int64Ty}, false));
            llvm::Value *RawPtr = B.CreateCall(MallocFn, {SizeConst}, "env_raw"); // i8*
            llvm::Value *NewEnvPtr = B.CreateBitCast(RawPtr, NewEnvStructTy->getPointerTo(), "envptr_struct");

            // Populate env fields (same order)
            for (unsigned k = 0; k < NewEnvOriginVals.size(); ++k)
            {
                llvm::Value *orig = NewEnvOriginVals[k];
                llvm::Value *GEP = B.CreateStructGEP(NewEnvStructTy, NewEnvPtr, k, "env_gep");
                llvm::Value *storeVal = nullptr;
                llvm::Type *FTy = NewEnvFieldTys[k];

                if (!orig)
                {
                    storeVal = llvm::Constant::getNullValue(FTy);
                }
                else
                {
                    if (orig->getType() != FTy)
                    {
                        if (orig->getType()->isPointerTy() && FTy->isPointerTy())
                            storeVal = B.CreateBitCast(orig, FTy);
                        else if (orig->getType()->isIntegerTy() && FTy->isIntegerTy())
                            storeVal = B.CreateIntCast(orig, FTy, /*isSigned=*/true);
                        else
                            storeVal = llvm::Constant::getNullValue(FTy);
                    }
                    else
                    {
                        storeVal = orig;
                    }
                }

                B.CreateStore(storeVal, GEP);
            }

            llvm::Value *EnvPtrCast = RawPtr;

            // -------- define wrapper(i64, i8*) that unpacks env and calls Outlined --------

            llvm::FunctionType *WrapperFT = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false);
            llvm::Function *WrapperFn = M->getFunction("wrapper");
            if (!WrapperFn)
            {
                WrapperFn = llvm::Function::Create(WrapperFT, llvm::Function::ExternalLinkage, "wrapper", M);
            }

            if (WrapperFn->empty())
            {
                auto argIt = WrapperFn->arg_begin();
                llvm::Argument *IdxArg = argIt++;
                IdxArg->setName("idx");
                llvm::Argument *EnvArg = argIt++;
                EnvArg->setName("env");

                llvm::BasicBlock *WEntry = llvm::BasicBlock::Create(F.getContext(), "entry", WrapperFn);
                llvm::IRBuilder<> Wb(WEntry);

                // cast env -> NewEnvStructTy*
                llvm::Value *EnvStructPtr = Wb.CreateBitCast(EnvArg, NewEnvStructTy->getPointerTo(), "envstruct");

                // load fields in same order
                llvm::SmallVector<llvm::Value *, 8> LoadedFields;
                LoadedFields.reserve(NewEnvFieldTys.size());
                for (unsigned k = 0; k < NewEnvFieldTys.size(); ++k)
                {
                    llvm::Value *Fgep = Wb.CreateStructGEP(NewEnvStructTy, EnvStructPtr, k, "fgep");
                    llvm::Value *Fload = Wb.CreateLoad(NewEnvFieldTys[k], Fgep, "fload");
                    LoadedFields.push_back(Fload);
                }

                // assemble call args for Outlined in exact param order
                llvm::SmallVector<llvm::Value *, 8> CallArgsForOutlined;
                unsigned envCursor = 0;
                for (unsigned pi = 0; pi < ArgOriginVals.size(); ++pi)
                {
                    if ((int)pi == InductionParamIndex)
                    {
                        // use runtime idx (cast to param type)
                        llvm::Type *PTy = Outlined->getFunctionType()->getParamType(pi);
                        llvm::Value *IdxVal = Wb.CreateIntCast(IdxArg, PTy, /*isSigned=*/true, "idxcast");
                        CallArgsForOutlined.push_back(IdxVal);
                    }
                    else
                    {
                        llvm::Value *fv = LoadedFields[envCursor++];
                        llvm::Type *PTy = Outlined->getFunctionType()->getParamType(pi);
                        if (fv->getType() != PTy)
                        {
                            if (fv->getType()->isPointerTy() && PTy->isPointerTy())
                                fv = Wb.CreateBitCast(fv, PTy);
                            else if (fv->getType()->isIntegerTy() && PTy->isIntegerTy())
                                fv = Wb.CreateIntCast(fv, PTy, /*isSigned=*/true);
                            else
                                fv = llvm::Constant::getNullValue(PTy);
                        }
                        CallArgsForOutlined.push_back(fv);
                    }
                }

                Wb.CreateCall(Outlined, CallArgsForOutlined);
                Wb.CreateRetVoid();
            }

            // Bitcast wrapper to runtime expected function pointer type
            llvm::Value *CastedWrapper = B.CreateBitCast(WrapperFn, LoopBodyFnTy);

            //  Declare parallel_for_runtime
            llvm::FunctionCallee ParallelForFunc = M->getOrInsertFunction(
                "parallel_for_runtime",
                llvm::FunctionType::get(
                    VoidTy,
                    {Int64Ty, Int64Ty, Int64Ty, LoopBodyFnTy, Int8PtrTy}, false));

            llvm::Type *ArgTy = nullptr;
            if (Outlined->getFunctionType()->getNumParams() > 0)
                ArgTy = Outlined->getFunctionType()->getParamType(0);

            // Insert the parallel call
            llvm::Value *StartArg = StartV;
            llvm::Value *EndArg = EndV;
            llvm::Value *StepArg = StepV;

            // Improvement: Handle without sign extension or make dynamic to allow for larger types
            // If any are not i64, sign-extend them to i64 (safe for small positive indices)
            if (StartArg->getType() != Int64Ty && StartArg->getType()->isIntegerTy())
                StartArg = B.CreateSExt(StartArg, Int64Ty, "start64");
            if (EndArg->getType() != Int64Ty && EndArg->getType()->isIntegerTy())
                EndArg = B.CreateSExt(EndArg, Int64Ty, "end64");
            if (StepArg->getType() != Int64Ty && StepArg->getType()->isIntegerTy())
                StepArg = B.CreateSExt(StepArg, Int64Ty, "step64");

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
            llvm::CallInst *CI = llvm::cast<llvm::CallInst>(B.CreateCall(ParallelForFunc, CallArgs));
            (void)CI; // silence unused-var in release builds

            // Create new branch to the loop exit.
            B.CreateBr(ExitBlock);

            Term->eraseFromParent();

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

void runLoopOutlinerOnModule(Module &M)
{
    // analysis managers
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;

    // PassBuilder and register analyses/proxies
    PassBuilder PB;
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // Build FPM, add your function pass
    FunctionPassManager FPM;
    // Option A: add the pass type directly
    FPM.addPass(LoopOutlinerPass());
    // Option B: if you exposed a helper to register it, you can call that:
    // registerLoopOutlinerPass(FPM);

    // Wrap function-pass manager into a module pass and run
    ModulePassManager MPM;
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

    MPM.run(M, MAM);
}

void registerLoopOutlinerPluginWithPassBuilder(PassBuilder &PB)
{
    PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>)
        {
            if (Name == "loop-outliner")
            {
                FPM.addPass(LoopOutlinerPass());
                return true;
            }
            return false;
        });
}
