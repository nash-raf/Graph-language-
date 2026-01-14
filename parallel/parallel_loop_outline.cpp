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
        bool doacross_check = false;

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
                                doacross_check = false;
                            }
                            else if (Val.equals_insensitive("DOACROSS"))
                            {
                                llvm::errs() << "  -> Detected DOACROSS parallel loop\n";
                                doall_check = false;
                                doacross_check = true;
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

        if (!doall_check && !doacross_check)
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

        if (doacross_check)
        {
            llvm::Module *M = F.getParent();
            llvm::LLVMContext &Ctx = M->getContext();
            llvm::Type *VoidTy = llvm::Type::getVoidTy(Ctx);
            llvm::Type *Int64Ty = llvm::Type::getInt64Ty(Ctx);
            llvm::Type *Int32Ty = llvm::Type::getInt32Ty(Ctx);

            llvm::FunctionCallee WaitFn = M->getOrInsertFunction("doacross_wait",
                                                                 llvm::FunctionType::get(VoidTy, {Int64Ty, Int64Ty, Int32Ty}, false));

            // void doacross_post(int64_t iter, int32_t id);
            llvm::FunctionCallee PostFn = M->getOrInsertFunction("doacross_post",
                                                                 llvm::FunctionType::get(VoidTy, {Int64Ty, Int32Ty}, false));

            // 2. Iterate over instructions in the loop body to find tags
            // We collect them first to avoid modifying the block while iterating
            struct InstrAction
            {
                llvm::Instruction *I;
                bool isWait; // true = wait, false = post
                int64_t dist;
                int32_t id;
            };

            llvm::SmallVector<InstrAction, 8> actions;

            for (llvm::BasicBlock *BB : LoopBody)
            {
                for (llvm::Instruction &I : *BB)
                {

                    // --- Check for WAIT (tagged on Store usually, via doacross.type) ---
                    if (llvm::MDNode *TypeMD = I.getMetadata("doacross.wait"))
                    {
                        int64_t distVal = 0;
                        int32_t idVal = 0;

                        if (llvm::MDNode *DistMD = I.getMetadata("doacross.dist"))
                            if (llvm::ConstantAsMetadata *CAM = llvm::dyn_cast<llvm::ConstantAsMetadata>(DistMD->getOperand(0)))
                                if (llvm::ConstantInt *CI = llvm::dyn_cast<llvm::ConstantInt>(CAM->getValue()))
                                    distVal = CI->getSExtValue();

                        if (llvm::MDNode *SrcMD = I.getMetadata("doacross.src"))
                            if (llvm::ConstantAsMetadata *CAM = llvm::dyn_cast<llvm::ConstantAsMetadata>(SrcMD->getOperand(0)))
                                if (llvm::ConstantInt *CI = llvm::dyn_cast<llvm::ConstantInt>(CAM->getValue()))
                                    idVal = (int32_t)CI->getSExtValue();

                        actions.push_back({&I, true, distVal, idVal});
                    }

                    // --- Check for POST (tagged on Load usually, or explicit instruction) ---
                    // Your IR shows: !doacross.post !1
                    // Note: You might have simply tagged it with "doacross.post" metadata directly
                    if (I.getMetadata("doacross.post"))
                    {
                        int32_t idVal = 0;
                        // The ID is usually in doacross.id
                        if (llvm::MDNode *IDMD = I.getMetadata("doacross.id"))
                            if (llvm::ConstantAsMetadata *CAM = llvm::dyn_cast<llvm::ConstantAsMetadata>(IDMD->getOperand(0)))
                                if (llvm::ConstantInt *CI = llvm::dyn_cast<llvm::ConstantInt>(CAM->getValue()))
                                    idVal = (int32_t)CI->getSExtValue();

                        actions.push_back({&I, false, 0, idVal});
                    }
                }
            }

            for (auto &act : actions)
            {
                llvm::IRBuilder<> B(act.I);

                // Ensure indvar is i64 for the runtime call
                llvm::Value *IndVar64 = indvar;
                if (indvar->getType() != Int64Ty)
                {
                    // Cast indvar to i64.
                    // Note: We should ideally cache this cast, but creating multiple casts is safe (optimization passes will clean it up)
                    IndVar64 = B.CreateIntCast(indvar, Int64Ty, true, "indvar.i64");
                }

                if (act.isWait)
                {
                    // Insert WAIT *before* the instruction
                    B.SetInsertPoint(act.I);
                    llvm::Value *DistArg = llvm::ConstantInt::get(Int64Ty, act.dist);
                    llvm::Value *IDArg = llvm::ConstantInt::get(Int32Ty, act.id);
                    B.CreateCall(WaitFn, {IndVar64, DistArg, IDArg});
                }
                else
                {
                    // Insert POST *after* the instruction
                    B.SetInsertPoint(act.I->getNextNode());
                    llvm::Value *IDArg = llvm::ConstantInt::get(Int32Ty, act.id);
                    B.CreateCall(PostFn, {IndVar64, IDArg});
                }
            }
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

        llvm::SetVector<llvm::Value *> Inputs, Outputs;
        llvm::SetVector<llvm::Value *> SinkCands; // usually empty for loops

        CE.findInputsOutputs(Inputs, Outputs, SinkCands, /*CollectGlobalInputs = */ true);

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

// outlineLoop - manual outlining replacement for CodeExtractor
// - clones loop body into a new function void(i64, i8*)
// - builds env struct for captured values
// - replaces the induction PHI with the idx parameter
// - rewrites exits to return
// - inserts runtime parallel_for_runtime(start,end,step,fn,env)
// Header comment: prefer snake_case style for internal helpers (user preference).
// llvm::Function *outlineLoop(llvm::Function &F,
//                             llvm::LoopInfo &LI,
//                             llvm::DominatorTree &DT,
//                             llvm::ScalarEvolution &SE,
//                             llvm::Value *&StartV,
//                             llvm::Value *&EndV,
//                             llvm::Value *&StepV)
// {
//     llvm::errs() << "[outlineLoop] Entering for function: " << F.getName() << "\n";

//     llvm::SmallVector<llvm::Loop *, 8> innermost;
//     for (llvm::Loop *Top : LI)
//         collectInnermostLoops(Top, innermost);

//     if (innermost.empty())
//     {
//         llvm::errs() << "[outlineLoop] ERROR: No innermost loops found\n";
//         return nullptr;
//     }

//     llvm::errs() << "[outlineLoop] Found " << innermost.size() << " innermost loops\n";

//     for (llvm::Loop *L : innermost)
//     {
//         llvm::errs() << "\n[outlineLoop] --- Inspecting loop header: "
//                      << L->getHeader()->getName() << " ---\n";

//         bool doall_check = false;
//         bool doacross_check = false;

//         if (llvm::Instruction *Term = L->getHeader()->getTerminator())
//         {
//             if (llvm::MDNode *LoopMD = Term->getMetadata("my.loop.parallel"))
//             {
//                 llvm::errs() << "[outlineLoop] Found my.loop.parallel metadata\n";
//                 for (unsigned i = 0; i < LoopMD->getNumOperands(); ++i)
//                 {
//                     if (auto *MDS = llvm::dyn_cast<llvm::MDString>(LoopMD->getOperand(i)))
//                     {
//                         llvm::StringRef S = MDS->getString();
//                         llvm::errs() << "  MDString operand: " << S << "\n";
//                         if (S.starts_with("parallel.type="))
//                         {
//                             llvm::StringRef Val = S.substr(strlen("parallel.type="));
//                             if (Val.equals_insensitive("DOALL"))
//                             {
//                                 doall_check = true;
//                                 doacross_check = false;
//                             }
//                             else if (Val.equals_insensitive("DOACROSS"))
//                             {
//                                 doall_check = false;
//                                 doacross_check = true;
//                             }
//                         }
//                     }
//                 }
//             }
//         }

//         if (!doall_check && !doacross_check)
//         {
//             llvm::errs() << "[outlineLoop] Skipping loop: no DOALL/DOACROSS tag\n";
//             continue;
//         }

//         llvm::errs() << "[outlineLoop] Parallel loop detected ("
//                      << (doall_check ? "DOALL" : "DOACROSS") << ")\n";

//         llvm::SmallVector<llvm::BasicBlock *, 8> loop_body;
//         for (llvm::BasicBlock *BB : L->blocks())
//             if (BB != L->getHeader())
//                 loop_body.push_back(BB);

//         if (loop_body.empty())
//         {
//             llvm::errs() << "[outlineLoop] ERROR: LoopBody empty after header exclusion\n";
//             continue;
//         }

//         llvm::BasicBlock *preheader = L->getLoopPreheader();
//         llvm::BasicBlock *exit_block = L->getExitBlock();

//         if (!preheader || !exit_block)
//         {
//             llvm::errs() << "[outlineLoop] ERROR: Missing preheader or single exit\n";
//             return nullptr;
//         }

//         llvm::PHINode *indvar = nullptr;
//         for (auto &I : *L->getHeader())
//         {
//             if (auto *phi = llvm::dyn_cast<llvm::PHINode>(&I))
//             {
//                 if (phi->getNumIncomingValues() >= 2 &&
//                     phi->getIncomingBlock(0) == preheader &&
//                     L->contains(phi->getIncomingBlock(1)))
//                 {
//                     indvar = phi;
//                     break;
//                 }
//             }
//         }

//         if (!indvar)
//         {
//             llvm::errs() << "[outlineLoop] ERROR: No canonical induction variable\n";
//             return nullptr;
//         }

//         llvm::errs() << "[outlineLoop] Found induction variable: ";
//         indvar->print(llvm::errs());
//         llvm::errs() << "\n";

//         const llvm::SCEV *backedge_count = SE.getBackedgeTakenCount(L);
//         const llvm::SCEV *Start = nullptr, *Step = nullptr, *End = nullptr;

//         if (const llvm::SCEVAddRecExpr *AR = llvm::dyn_cast<llvm::SCEVAddRecExpr>(SE.getSCEV(indvar)))
//         {
//             Start = AR->getStart();
//             Step = AR->getStepRecurrence(SE);
//             End = SE.getAddExpr(Start, SE.getMulExpr(Step, backedge_count));
//         }
//         else
//         {
//             llvm::errs() << "[outlineLoop] ERROR: Induction variable is not SCEVAddRec\n";
//             return nullptr;
//         }

//         llvm::SCEVExpander Exp(SE, F.getParent()->getDataLayout(), "scevexp");
//         StartV = Exp.expandCodeFor(Start, indvar->getType(), preheader->getTerminator());
//         StepV = Exp.expandCodeFor(Step, indvar->getType(), preheader->getTerminator());
//         EndV = Exp.expandCodeFor(End, indvar->getType(), preheader->getTerminator());

//         llvm::errs() << "[outlineLoop] Expanded loop bounds successfully\n";

//         // === Manual capture analysis ===
//         llvm::SmallPtrSet<llvm::Value *, 16> inputs_set;
//         llvm::SmallPtrSet<llvm::Value *, 16> outputs_set;

//         auto is_defined_in_loop = [&](llvm::Value *V) -> bool
//         {
//             if (auto *I = llvm::dyn_cast<llvm::Instruction>(V))
//                 return L->contains(I->getParent());
//             return false;
//         };

//         for (llvm::BasicBlock *BB : loop_body)
//         {
//             for (llvm::Instruction &I : *BB)
//             {
//                 for (llvm::Use &U : I.operands())
//                 {
//                     llvm::Value *Op = U.get();
//                     if (!Op || llvm::isa<llvm::Constant>(Op) || is_defined_in_loop(Op))
//                         continue;
//                     inputs_set.insert(Op);
//                 }

//                 for (llvm::Use &U : I.uses())
//                 {
//                     llvm::Instruction *UserI = llvm::dyn_cast<llvm::Instruction>(U.getUser());
//                     if (!UserI || is_defined_in_loop(UserI))
//                         continue;
//                     outputs_set.insert(&I);
//                     break;
//                 }

//                 if (auto *SI = llvm::dyn_cast<llvm::StoreInst>(&I))
//                 {
//                     llvm::Value *Ptr = SI->getPointerOperand();
//                     llvm::Value *Base = llvm::getUnderlyingObject(Ptr, 64);
//                     if (auto *BI = llvm::dyn_cast<llvm::Instruction>(Base))
//                         if (L->contains(BI->getParent()))
//                             continue;
//                     outputs_set.insert(Base);
//                 }
//             }
//         }

//         llvm::SmallVector<llvm::Value *, 16> candidates;
//         for (llvm::Value *V : outputs_set)
//             candidates.push_back(V);
//         for (llvm::Value *V : inputs_set)
//             candidates.push_back(V);

//         llvm::Module *M = F.getParent();
//         llvm::LLVMContext &Ctx = M->getContext();
//         llvm::Type *VoidTy = llvm::Type::getVoidTy(Ctx);
//         llvm::Type *Int64Ty = llvm::Type::getInt64Ty(Ctx);
//         llvm::Type *Int8PtrTy = llvm::Type::getInt8Ty(Ctx)->getPointerTo();

//         // Ensure all types are sized before adding to struct
//         llvm::SmallVector<llvm::Type *, 16> env_field_tys;
//         for (llvm::Value *V : candidates)
//         {
//             llvm::Type *ty = V ? V->getType() : Int8PtrTy;

//             // Replace unsized types with i8*
//             if (!ty->isSized())
//             {
//                 llvm::errs() << "[outlineLoop] WARNING: Replacing unsized type with i8*\n";
//                 ty = Int8PtrTy;
//             }

//             env_field_tys.push_back(ty);
//         }

//         llvm::StructType *env_struct_ty = llvm::StructType::create(Ctx, env_field_tys, "env.struct");

//         // Verify struct is sized before getting its size
//         if (!env_struct_ty->isSized())
//         {
//             llvm::errs() << "[outlineLoop] ERROR: Environment struct is not sized!\n";
//             return nullptr;
//         }

//         llvm::IRBuilder<> B(preheader->getTerminator());
//         uint64_t env_size = M->getDataLayout().getTypeAllocSize(env_struct_ty);
//         llvm::Value *size_const = llvm::ConstantInt::get(Int64Ty, env_size);
//         llvm::FunctionCallee malloc_fn = M->getOrInsertFunction("malloc", llvm::FunctionType::get(Int8PtrTy, {Int64Ty}, false));
//         llvm::Value *raw_ptr = B.CreateCall(malloc_fn, {size_const}, "env_raw");
//         llvm::Value *env_ptr = B.CreateBitCast(raw_ptr, env_struct_ty->getPointerTo(), "envptr_struct");

//         for (unsigned k = 0; k < candidates.size(); ++k)
//         {
//             llvm::Value *orig = candidates[k];
//             llvm::Value *gep = B.CreateStructGEP(env_struct_ty, env_ptr, k, "env_gep");
//             llvm::Type *fty = env_field_tys[k];

//             llvm::Value *store_val = nullptr;
//             if (!orig)
//             {
//                 store_val = llvm::Constant::getNullValue(fty);
//             }
//             else
//             {
//                 if (orig->getType() != fty)
//                 {
//                     if (orig->getType()->isPointerTy() && fty->isPointerTy())
//                         store_val = B.CreateBitCast(orig, fty);
//                     else if (orig->getType()->isIntegerTy() && fty->isIntegerTy())
//                         store_val = B.CreateIntCast(orig, fty, true);
//                     else
//                         store_val = B.CreateBitCast(orig, fty);
//                 }
//                 else
//                     store_val = orig;
//             }
//             B.CreateStore(store_val, gep);
//         }

//         llvm::Value *env_ptr_cast = raw_ptr;

//         // === Create outlined function ===
//         llvm::FunctionType *outlined_fty = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false);
//         llvm::Function *outlined = llvm::Function::Create(outlined_fty, llvm::GlobalValue::InternalLinkage, "outlined_loop_body", M);
//         auto arg_it = outlined->arg_begin();
//         llvm::Argument *idx_arg = &*arg_it++;
//         idx_arg->setName("idx");
//         llvm::Argument *env_arg = &*arg_it++;
//         env_arg->setName("env");

//         llvm::BasicBlock *entry_bb = llvm::BasicBlock::Create(Ctx, "entry", outlined);
//         llvm::IRBuilder<> Eb(entry_bb);
//         llvm::Value *env_struct_ptr = Eb.CreateBitCast(env_arg, env_struct_ty->getPointerTo(), "envstruct");

//         llvm::SmallVector<llvm::Value *, 16> loaded_fields;
//         for (unsigned k = 0; k < env_field_tys.size(); ++k)
//         {
//             llvm::Value *fgep = Eb.CreateStructGEP(env_struct_ty, env_struct_ptr, k, "fgep");
//             llvm::Value *fload = Eb.CreateLoad(env_field_tys[k], fgep, "fload");
//             loaded_fields.push_back(fload);
//         }

//         llvm::Value *idx_cast_to_indvar = (idx_arg->getType() != indvar->getType()) ? Eb.CreateIntCast(idx_arg, indvar->getType(), true, "idxcast") : idx_arg;

//         llvm::ValueToValueMapTy vmap;
//         vmap[indvar] = idx_cast_to_indvar;
//         for (unsigned k = 0; k < candidates.size(); ++k)
//             if (candidates[k])
//                 vmap[candidates[k]] = loaded_fields[k];

//         llvm::SmallVector<llvm::BasicBlock *, 8> cloned_blocks;
//         cloned_blocks.reserve(loop_body.size());
//         for (llvm::BasicBlock *OrigBB : loop_body)
//         {
//             llvm::BasicBlock *ClonedBB = llvm::BasicBlock::Create(Ctx, OrigBB->getName() + ".cloned", outlined);
//             llvm::IRBuilder<> Builder(ClonedBB);
//             for (llvm::Instruction &OrigI : *OrigBB)
//             {
//                 llvm::Instruction *ClonedI = OrigI.clone();
//                 Builder.Insert(ClonedI);
//                 vmap[&OrigI] = ClonedI;
//             }
//             vmap[OrigBB] = ClonedBB;
//             cloned_blocks.push_back(ClonedBB);
//         }

//         // Create return block BEFORE remapping
//         llvm::BasicBlock *ret_bb = llvm::BasicBlock::Create(Ctx, "ret", outlined);

//         // Map all external blocks (preheader, header, exit) to ret_bb
//         // This ensures RemapInstruction doesn't crash when it encounters references to them
//         vmap[exit_block] = ret_bb;
//         vmap[preheader] = ret_bb;
//         vmap[L->getHeader()] = ret_bb;

//         for (llvm::BasicBlock *ClonedBB : cloned_blocks)
//             for (llvm::Instruction &I : *ClonedBB)
//                 llvm::RemapInstruction(&I, vmap, llvm::RF_NoModuleLevelChanges | llvm::RF_IgnoreMissingLocals);

//         // FIX: Now fix up terminators - check if terminator exists before accessing
//         for (llvm::BasicBlock *ClonedBB : cloned_blocks)
//         {
//             // Check if block has instructions and verify it's still valid
//             if (ClonedBB->empty())
//             {
//                 llvm::errs() << "[outlineLoop] WARNING: Empty cloned block, adding branch to ret\n";
//                 llvm::BranchInst::Create(ret_bb, ClonedBB);
//                 continue;
//             }

//             // Get the last instruction safely
//             llvm::Instruction *LastInst = &ClonedBB->back();

//             // Verify the instruction is valid before checking if it's a terminator
//             if (!LastInst)
//             {
//                 llvm::errs() << "[outlineLoop] WARNING: Null last instruction, adding branch to ret\n";
//                 llvm::BranchInst::Create(ret_bb, ClonedBB);
//                 continue;
//             }

//             // If it's already a terminator, fix it up
//             if (LastInst->isTerminator())
//             {
//                 if (auto *BI = llvm::dyn_cast<llvm::BranchInst>(LastInst))
//                 {
//                     for (unsigned si = 0; si < BI->getNumSuccessors(); ++si)
//                     {
//                         llvm::BasicBlock *Succ = BI->getSuccessor(si);
//                         if (!Succ || Succ->getParent() != outlined)
//                             BI->setSuccessor(si, ret_bb);
//                     }
//                 }
//                 else if (auto *SI = llvm::dyn_cast<llvm::SwitchInst>(LastInst))
//                 {
//                     for (auto &Case : SI->cases())
//                     {
//                         llvm::BasicBlock *CaseSucc = Case.getCaseSuccessor();
//                         if (!CaseSucc || CaseSucc->getParent() != outlined)
//                             Case.setSuccessor(ret_bb);
//                     }
//                     llvm::BasicBlock *DefaultDest = SI->getDefaultDest();
//                     if (!DefaultDest || DefaultDest->getParent() != outlined)
//                         SI->setDefaultDest(ret_bb);
//                 }
//                 else
//                 {
//                     // Other terminator (ret, unreachable, etc.) - replace with br
//                     LastInst->eraseFromParent();
//                     llvm::BranchInst::Create(ret_bb, ClonedBB);
//                 }
//             }
//             else
//             {
//                 // No terminator - add one
//                 llvm::errs() << "[outlineLoop] WARNING: Block has no terminator, adding branch to ret\n";
//                 llvm::BranchInst::Create(ret_bb, ClonedBB);
//             }
//         }

//         // Connect entry to first cloned block
//         llvm::BasicBlock *first_clone_bb = llvm::cast<llvm::BasicBlock>(vmap[loop_body.front()]);
//         Eb.CreateBr(first_clone_bb);

//         // Add return instruction to ret_bb
//         llvm::ReturnInst::Create(Ctx, nullptr, ret_bb);

//         // IMPORTANT: Verify the outlined function is well-formed
//         llvm::errs() << "[outlineLoop] Verifying outlined function structure...\n";
//         std::string err_str;
//         llvm::raw_string_ostream err_stream(err_str);
//         // if (llvm::verifyFunction(*outlined, &err_stream))
//         // {
//         //     llvm::errs() << "[outlineLoop] ERROR: Outlined function verification failed:\n";
//         //     llvm::errs() << err_stream.str() << "\n";
//         //     outlined->eraseFromParent();
//         //     return nullptr;
//         // }
//         llvm::errs() << "[outlineLoop] Outlined function verified successfully\n";

//         // Call parallel runtime
//         llvm::Type *loop_body_fn_ty = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false)->getPointerTo();
//         llvm::FunctionCallee parallel_for_func = M->getOrInsertFunction(
//             "parallel_for_runtime",
//             llvm::FunctionType::get(VoidTy, {Int64Ty, Int64Ty, Int64Ty, loop_body_fn_ty, Int8PtrTy}, false));

//         llvm::Value *start_arg = (StartV->getType() != Int64Ty) ? B.CreateSExt(StartV, Int64Ty) : StartV;
//         llvm::Value *end_arg = (EndV->getType() != Int64Ty) ? B.CreateSExt(EndV, Int64Ty) : EndV;
//         llvm::Value *step_arg = (StepV->getType() != Int64Ty) ? B.CreateSExt(StepV, Int64Ty) : StepV;
//         llvm::Value *casted_outlined = B.CreateBitCast(outlined, loop_body_fn_ty);

//         B.CreateCall(parallel_for_func, {start_arg, end_arg, step_arg, casted_outlined, env_ptr_cast});
//         B.CreateBr(exit_block);
//         preheader->getTerminator()->eraseFromParent();

//         // CRITICAL: Remove the original loop blocks to avoid invalid CFG
//         llvm::errs() << "[outlineLoop] Removing original loop blocks...\n";

//         // First, replace all uses of PHI nodes in the header with undef
//         for (llvm::Instruction &I : *L->getHeader())
//         {
//             if (llvm::PHINode *PHI = llvm::dyn_cast<llvm::PHINode>(&I))
//             {
//                 PHI->replaceAllUsesWith(llvm::UndefValue::get(PHI->getType()));
//             }
//         }

//         // Remove all blocks in the loop (including header)
//         llvm::SmallVector<llvm::BasicBlock *, 8> to_delete;
//         for (llvm::BasicBlock *BB : L->blocks())
//         {
//             to_delete.push_back(BB);
//         }

//         for (llvm::BasicBlock *BB : to_delete)
//         {
//             // Remove all instructions first
//             while (!BB->empty())
//             {
//                 llvm::Instruction &I = BB->back();
//                 I.replaceAllUsesWith(llvm::UndefValue::get(I.getType()));
//                 I.eraseFromParent();
//             }
//             BB->eraseFromParent();
//         }

//         llvm::errs() << "[outlineLoop] Original loop removed\n";

//         outlined->setLinkage(llvm::GlobalValue::ExternalLinkage);
//         outlined->setName("outlined_main_loopbody");

//         llvm::errs() << "[outlineLoop] Outlined function created: " << outlined->getName() << "\n";
//         return outlined;
//     }

//     llvm::errs() << "[outlineLoop] Finished loop scan: nothing outlined\n";
//     return nullptr;
// }

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
