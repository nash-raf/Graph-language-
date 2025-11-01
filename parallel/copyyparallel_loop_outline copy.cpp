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

        llvm::SetVector<llvm::Value *> Inputs, Outputs;
        llvm::SetVector<llvm::Value *> SinkCands; // usually empty for loops

        CE.findInputsOutputs(Inputs, Outputs, SinkCands, /*CollectGlobalInputs = */ true);

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

        // llvm::SmallVector<llvm::Type *, 8> EnvFieldTys;
        // for (llvm::Value *V : Inputs)
        // {
        //     EnvFieldTys.push_back(V->getType()); // e.g. [5 x i32]* etc.
        // }

        // llvm::StructType *EnvStructTy =
        //     llvm::StructType::create(F.getContext(), EnvFieldTys, "env.struct");

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
            for (auto &Arg : Outlined->args())
            {
                llvm::Value *matched = nullptr;
                llvm::Type *Pty = Arg.getType();

                // Prefer mapping the induction var if types match and it isn't used yet
                if (IndVar && IndVar->getType() == Pty && !UsedCandidates.count(IndVar))
                {
                    matched = IndVar;
                    UsedCandidates.insert(matched);
                    ArgOriginVals.push_back(matched);
                    continue;
                }

                // Otherwise pick the first compatible candidate not yet used
                for (llvm::Value *C : Candidates)
                {
                    if (UsedCandidates.count(C))
                        continue;
                    if (C->getType() == Pty)
                    {
                        matched = C;
                        break;
                    }
                    if (C->getType()->isPointerTy() && Pty->isPointerTy())
                    {
                        matched = C;
                        break;
                    }
                    if (C->getType()->isIntegerTy() && Pty->isIntegerTy())
                    {
                        matched = C;
                        break;
                    }
                }
                if (matched)
                    UsedCandidates.insert(matched);
                ArgOriginVals.push_back(matched); // may be nullptr if nothing found
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
            // llvm::AllocaInst *EnvAlloca = B.CreateAlloca(EnvStructTy, nullptr, "env");

            // unsigned idx = 0;
            // for (llvm::Value *V : Inputs)
            // {
            //     // Get pointer to the struct field
            //     llvm::Value *GEP = B.CreateStructGEP(EnvStructTy, EnvAlloca, idx, "env_gep");
            //     // If the value type doesn't match the struct field type exactly, bitcast
            //     llvm::Value *ValToStore = V;
            //     if (V->getType() != EnvFieldTys[idx])
            //     {
            //         if (V->getType()->isPointerTy() && EnvFieldTys[idx]->isPointerTy())
            //         {
            //             ValToStore = B.CreateBitCast(V, EnvFieldTys[idx]);
            //         }
            //         else
            //         {
            //             llvm::errs() << "Type mismatch for env field; consider extending this.\n";
            //         }
            //     }
            //     B.CreateStore(ValToStore, GEP);
            //     ++idx;
            // }
            // INSERT this block instead of the removed code.
            // Allocate env on the heap using malloc, populate it, and set EnvPtrCast to i8*.

            // get DataLayout and size for the env struct
            // uint64_t EnvSize = M->getDataLayout().getTypeAllocSize(EnvStructTy);
            // llvm::Value *SizeConst = llvm::ConstantInt::get(Int64Ty, EnvSize);

            // // declare malloc (i8* malloc(i64))
            // llvm::FunctionCallee MallocFn = M->getOrInsertFunction(
            //     "malloc",
            //     llvm::FunctionType::get(Int8PtrTy, {Int64Ty}, false));

            // // call malloc to allocate raw i8*
            // llvm::Value *RawPtr = B.CreateCall(MallocFn, {SizeConst}, "env_raw"); // i8*

            // // cast raw pointer to (env.struct)*
            // llvm::Value *EnvPtr = B.CreateBitCast(RawPtr, EnvStructTy->getPointerTo(), "envptr_struct");

            // // populate struct fields (same order as EnvFieldTys / Inputs)
            // unsigned idx = 0;
            // for (llvm::Value *V : Inputs)
            // {
            //     llvm::Value *GEP = B.CreateStructGEP(EnvStructTy, EnvPtr, idx, "env_gep");
            //     llvm::Value *ValToStore = V;

            //     // basic type adjust/cast handling (pointer-to-pointer, integer widen/trunc)
            //     if (V->getType() != EnvFieldTys[idx])
            //     {
            //         if (V->getType()->isPointerTy() && EnvFieldTys[idx]->isPointerTy())
            //         {
            //             ValToStore = B.CreateBitCast(V, EnvFieldTys[idx]);
            //         }
            //         else if (V->getType()->isIntegerTy() && EnvFieldTys[idx]->isIntegerTy())
            //         {
            //             unsigned srcBits = V->getType()->getIntegerBitWidth();
            //             unsigned dstBits = EnvFieldTys[idx]->getIntegerBitWidth();
            //             if (srcBits < dstBits)
            //                 ValToStore = B.CreateSExt(V, EnvFieldTys[idx]);
            //             else if (srcBits > dstBits)
            //                 ValToStore = B.CreateTrunc(V, EnvFieldTys[idx]);
            //             else
            //                 ValToStore = V;
            //         }
            //         else
            //         {
            //             // fallback: try bitcast to field type if pointer-compatible, else store null
            //             if (V->getType()->isPointerTy() && EnvFieldTys[idx]->isPointerTy())
            //                 ValToStore = B.CreateBitCast(V, EnvFieldTys[idx]);
            //             else
            //                 ValToStore = llvm::Constant::getNullValue(EnvFieldTys[idx]);
            //         }
            //     }

            //     B.CreateStore(ValToStore, GEP);
            //     ++idx;
            // }

            // // set EnvPtrCast to the i8* returned by malloc (RawPtr already i8*)

            // llvm::Value *EnvPtrCast = RawPtr;
            //  Declare parallel_for_runtime
            llvm::FunctionCallee ParallelForFunc = M->getOrInsertFunction(
                "parallel_for_runtime",
                llvm::FunctionType::get(
                    VoidTy,
                    {Int64Ty, Int64Ty, Int64Ty, LoopBodyFnTy, Int8PtrTy}, false));

            // Build env struct
            // --- SAFE: do not attempt to store Outlined->getArg(0) here ---
            // For now pass a NULL env pointer to runtime (populate env later properly)
            // llvm::Value *EnvPtrCast = B.CreateBitCast(EnvAlloca, Int8PtrTy, "envptr");
            llvm::Type *ArgTy = nullptr;
            if (Outlined->getFunctionType()->getNumParams() > 0)
                ArgTy = Outlined->getFunctionType()->getParamType(0);
            // Look in the entry block for an Alloca that likely corresponds to the captured 'a'
            // llvm::BasicBlock &EntryBB = F.getEntryBlock();
            // llvm::AllocaInst *FoundAlloca = nullptr;
            // for (llvm::Instruction &I : EntryBB)
            // {
            //     if (auto *AI = llvm::dyn_cast<llvm::AllocaInst>(&I))
            //     {
            //         // If ArgTy is an actual pointer-to-some-type, compare allocated type.
            //         if (ArgTy)
            //         {
            //             if (AI->getAllocatedType() == ArgTy || AI->getAllocatedType()->isArrayTy())
            //             {
            //                 FoundAlloca = AI;
            //                 break;
            //             }
            //         }
            //         else
            //         {
            //             // fallback: take the first alloca
            //             FoundAlloca = AI;
            //             break;
            //         }
            //     }
            // }

            // if (FoundAlloca)
            // {
            //     // cast the alloca pointer to i8* (env)
            //     llvm::PointerType *Int8PtrTyPtr = llvm::cast<llvm::PointerType>(Int8PtrTy);
            //     EnvPtrCast = B.CreateBitCast(EnvAlloca, Int8PtrTyPtr, "envptr");
            //     // llvm::errs() << "Using alloca as env: " << *FoundAlloca << "\n";
            // }
            // else
            // {
            //     // fallback to null (should not happen for simple stack-alloc cases)
            //     llvm::PointerType *Int8PtrTyPtr = llvm::cast<llvm::PointerType>(Int8PtrTy);
            //     EnvPtrCast = llvm::ConstantPointerNull::get(Int8PtrTyPtr);
            //     // llvm::errs() << "WARNING: no alloca found, passing null env\n";
            // }

            // Ensure wrapper symbol is declared (so CreateBitCast will not get nullptr)
            // llvm::Function *WrapperFn = M->getFunction("wrapper");
            // if (!WrapperFn)
            // {
            //     llvm::FunctionType *WFT = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false);
            //     WrapperFn = llvm::Function::Create(WFT, llvm::Function::ExternalLinkage, "wrapper", M);
            // }
            // // Now it's safe to bitcast the wrapper to the expected function-pointer type
            // llvm::Value *CastedWrapper = B.CreateBitCast(WrapperFn, LoopBodyFnTy);

            // ----- BEGIN REPLACEMENT: define wrapper(i64, i8*) that unpacks env and calls Outlined -----

            // Create/obtain wrapper function (void wrapper(i64, i8*))
            // llvm::FunctionType *WrapperFT = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false);
            // llvm::Function *WrapperFn = M->getFunction("wrapper");
            // if (!WrapperFn)
            // {
            //     WrapperFn = llvm::Function::Create(WrapperFT, llvm::Function::ExternalLinkage, "wrapper", M);
            // }

            // // If the function has no body yet, define it now.
            // if (WrapperFn->empty())
            // {
            //     // name arguments
            //     auto argIt = WrapperFn->arg_begin();
            //     llvm::Argument *IdxArg = argIt++;
            //     IdxArg->setName("idx");
            //     llvm::Argument *EnvArg = argIt++;
            //     EnvArg->setName("env");

            //     // build entry block for wrapper
            //     llvm::BasicBlock *WEntry = llvm::BasicBlock::Create(F.getContext(), "entry", WrapperFn);
            //     llvm::IRBuilder<> Wb(WEntry);

            //     // cast env i8* -> env.struct*
            //     llvm::Value *EnvStructPtr = Wb.CreateBitCast(EnvArg, NewEnvStructTy->getPointerTo(), "envstruct");

            //     // load captured fields in the same order as EnvFieldTys (which was built from Inputs)
            //     llvm::SmallVector<llvm::Value *, 8> LoadedFields;
            //     LoadedFields.reserve(NewEnvFieldTys.size());
            //     for (unsigned i = 0; i < NewEnvFieldTys.size(); ++i)
            //     {
            //         llvm::Value *Fgep = Wb.CreateStructGEP(NewEnvStructTy, EnvStructPtr, i, "fgep");
            //         llvm::Value *Fload = Wb.CreateLoad(NewEnvFieldTys[i], Fgep, "fload");
            //         LoadedFields.push_back(Fload);
            //     }

            //     // Prepare call args for the outlined function.
            //     // Heuristic: if Outlined's first param is an integer, map it to the index;
            //     // otherwise, map Outlined params to captured fields in Inputs order.
            //     llvm::SmallVector<llvm::Value *, 8> CallArgs;
            //     unsigned nextField = 0;
            //     for (auto &Param : Outlined->args())
            //     {
            //         llvm::Type *PTy = Param.getType();
            //         if (&Param == &*Outlined->arg_begin() && PTy->isIntegerTy())
            //         {
            //             // convert idx to the expected integer width if needed:
            //             if (IdxArg->getType() != PTy)
            //             {
            //                 // use int cast (sign-extend) — adjust if you need zero-extend semantics
            //                 llvm::Value *IdxCast = Wb.CreateIntCast(IdxArg, PTy, /*isSigned=*/true, "idxcast");
            //                 CallArgs.push_back(IdxCast);
            //             }
            //             else
            //             {
            //                 CallArgs.push_back(IdxArg);
            //             }
            //             continue;
            //         }

            //         // otherwise pick the next captured field (Inputs order -> EnvFieldTys order)
            //         llvm::Value *FieldVal = nullptr;
            //         if (nextField < LoadedFields.size())
            //         {
            //             FieldVal = LoadedFields[nextField++];
            //             // if types mismatch, try bitcast or int cast
            //             if (FieldVal->getType() != PTy)
            //             {
            //                 if (FieldVal->getType()->isPointerTy() && PTy->isPointerTy())
            //                 {
            //                     FieldVal = Wb.CreateBitCast(FieldVal, PTy, "field_cast");
            //                 }
            //                 else if (FieldVal->getType()->isIntegerTy() && PTy->isIntegerTy())
            //                 {
            //                     FieldVal = Wb.CreateIntCast(FieldVal, PTy, /*isSigned=*/true, "field_intcast");
            //                 }
            //                 else
            //                 {
            //                     // fallback: provide null of required type to avoid crash (you may prefer an assert)
            //                     FieldVal = llvm::Constant::getNullValue(PTy);
            //                 }
            //             }
            //         }
            //         else
            //         {
            //             // no more fields -> pass null as fallback
            //             FieldVal = llvm::Constant::getNullValue(PTy);
            //         }
            //         CallArgs.push_back(FieldVal);
            //     }

            //     // Call the outlined per-iteration function
            //     Wb.CreateCall(Outlined, CallArgs);
            //     Wb.CreateRetVoid();
            // }

            // Build wrapper function (void wrapper(i64, i8*))
            // llvm::FunctionType *WrapperFT = llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false);
            // llvm::Function *WrapperFn = M->getFunction("wrapper");
            // if (!WrapperFn)
            // {
            //     WrapperFn = llvm::Function::Create(WrapperFT, llvm::Function::ExternalLinkage, "wrapper", M);
            // }
            // if (WrapperFn->empty())
            // {
            //     llvm::Argument *IdxArg = &*WrapperFn->arg_begin();
            //     IdxArg->setName("idx");
            //     llvm::Argument *EnvArg = &*std::next(WrapperFn->arg_begin());
            //     EnvArg->setName("env");

            //     llvm::BasicBlock *WEntry = llvm::BasicBlock::Create(F.getContext(), "entry", WrapperFn);
            //     llvm::IRBuilder<> Wb(WEntry);

            //     // cast env i8* -> EnvStructTy*
            //     llvm::Value *EnvStructPtr = Wb.CreateBitCast(EnvArg, NewEnvStructTy->getPointerTo());

            //     // load all fields in same order as EnvFieldTys
            //     llvm::SmallVector<llvm::Value *, 8> LoadedFields;
            //     for (unsigned k = 0; k < NewEnvFieldTys.size(); ++k)
            //     {
            //         llvm::Value *Fgep = Wb.CreateStructGEP(NewEnvStructTy, EnvStructPtr, k);
            //         llvm::Value *Fload = Wb.CreateLoad(NewEnvFieldTys[k], Fgep);
            //         LoadedFields.push_back(Fload);
            //     }

            //     // Now assemble call args for Outlined in order:
            //     llvm::SmallVector<llvm::Value *, 8> CallArgs;
            //     unsigned envIndexCursor = 0;
            //     for (unsigned pi = 0; pi < ArgOriginVals.size(); ++pi)
            //     {
            //         if ((int)pi == InductionParamIndex)
            //         {
            //             // use the incoming idx (cast to expected type)
            //             llvm::Type *PTy = Outlined->getFunctionType()->getParamType(pi);
            //             llvm::Value *IdxVal = Wb.CreateIntCast(IdxArg, PTy, /*isSigned=*/true);
            //             CallArgs.push_back(IdxVal);
            //         }
            //         else
            //         {
            //             // take next field from LoadedFields
            //             llvm::Value *fv = LoadedFields[envIndexCursor++];
            //             // cast if necessary to param type:
            //             llvm::Type *PTy = Outlined->getFunctionType()->getParamType(pi);
            //             if (fv->getType() != PTy)
            //             {
            //                 if (fv->getType()->isPointerTy() && PTy->isPointerTy())
            //                     fv = Wb.CreateBitCast(fv, PTy);
            //                 else if (fv->getType()->isIntegerTy() && PTy->isIntegerTy())
            //                     fv = Wb.CreateIntCast(fv, PTy, /*isSigned=*/true);
            //                 else
            //                     fv = llvm::Constant::getNullValue(PTy);
            //             }
            //             CallArgs.push_back(fv);
            //         }
            //     }

            //     Wb.CreateCall(Outlined, CallArgs);
            //     Wb.CreateRetVoid();
            // }

            // // Now bitcast the defined wrapper to the runtime expected function-pointer type
            // CastedWrapper = B.CreateBitCast(WrapperFn, LoopBodyFnTy);

            // ----- END REPLACEMENT -----

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

// Register the pass as a plugin
// extern "C" PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo()
// {
//     return {
//         LLVM_PLUGIN_API_VERSION, "LoopOutliner", "v0.1",
//         [](PassBuilder &PB)
//         {
//             PB.registerPipelineParsingCallback(
//                 [](StringRef Name, FunctionPassManager &FPM,
//                    ArrayRef<PassBuilder::PipelineElement>)
//                 {
//                     if (Name == "loop-outliner")
//                     {
//                         FPM.addPass(LoopOutlinerPass());
//                         return true;
//                     }
//                     return false;
//                 });
//         }};
// }
// make this visible to your executable

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

// // ensure canonical exported entrypoint
// extern "C" __attribute__((visibility("default"))) ::llvm::PassPluginLibraryInfo LLVMGetPassPluginInfo()
// {
//     return llvmGetPassPluginInfo(); // calls your existing function
// }