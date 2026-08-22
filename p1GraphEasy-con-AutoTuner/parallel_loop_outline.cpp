#include "parallel_loop_outline.h"
#include "parallel_runtime.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsNVPTX.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

#include <cctype>

#include <functional>
#include <optional>
#include <string>

using namespace llvm;

namespace
{
    enum class ParallelMode
    {
        None,
        DoAll,
        DoAcross,
    };

    struct LoopCandidateAnalysis
    {
        Loop *LoopObj = nullptr;
        ParallelMode Mode = ParallelMode::None;
        BasicBlock *Header = nullptr;
        BasicBlock *Preheader = nullptr;
        BasicBlock *Latch = nullptr;
        BasicBlock *ExitingBlock = nullptr;
        BasicBlock *ExitBlock = nullptr;
        PHINode *IndVar = nullptr;
        SmallVector<BasicBlock *, 8> LoopBlocks;
        SmallVector<BasicBlock *, 8> LoopBody;
        const SCEV *BackedgeCount = nullptr;
        const SCEV *Start = nullptr;
        const SCEV *Step = nullptr;
        const SCEV *End = nullptr;
        std::optional<uint64_t> ConstantTripCount;
        unsigned EffectiveBodyInstCount = 0;
        unsigned Depth = 0;
        unsigned DoAcrossWaitsPerIter = 0;
        unsigned DoAcrossPostsPerIter = 0;
        SmallPtrSet<BasicBlock *, 8> DependentBlocks;
        SmallPtrSet<BasicBlock *, 8> IndependentBlocks;
        bool RequiresIntAppendPriv = false;
    };

    struct LoopTransformTarget
    {
        ParallelMode Mode = ParallelMode::None;
        BasicBlock *Header = nullptr;
        BasicBlock *Preheader = nullptr;
        BasicBlock *Latch = nullptr;
        BasicBlock *ExitingBlock = nullptr;
        BasicBlock *ExitBlock = nullptr;
        PHINode *IndVar = nullptr;
        SmallVector<BasicBlock *, 8> LoopBlocks;
        SmallVector<BasicBlock *, 8> LoopBody;
        unsigned Depth = 0;
        bool RequiresIntAppendPriv = false;
    };

    struct LoopVersioningInfo
    {
        ParallelMode Mode = ParallelMode::None;
        LoopTransformTarget Serial;
        LoopTransformTarget Parallel;
        BasicBlock *DispatchBlock = nullptr;
        BasicBlock *SerialStartBlock = nullptr;
        BasicBlock *SerialExitBridge = nullptr;
        BasicBlock *ParallelExitBridge = nullptr;
        BasicBlock *SharedExitBlock = nullptr;
        Value *StartArg = nullptr;
        Value *EndArg = nullptr;
        Value *StepArg = nullptr;
        AllocaInst *SerialStartTimeSlot = nullptr;
        AllocaInst *DoAcrossDepTotalSlot = nullptr;
        AllocaInst *DoAcrossIndTotalSlot = nullptr;
        unsigned LoopId = 0;
    };

    struct ParallelTransformResult
    {
        Function *Outlined = nullptr;
        bool Changed = false;
        bool ParallelReady = false;
        bool Privatized = false;
        uint64_t EnvSize = 0;
        GlobalVariable *PrivOffsetsGV = nullptr;
        GlobalVariable *PrivKindsGV = nullptr;
        GlobalVariable *PrivAuxGV = nullptr;
        unsigned NumPrivTargets = 0;
        unsigned NumDoAcrossSyncIds = 0;
        Function *WrapperFn = nullptr;
    };

    struct OutlineLoopResult
    {
        Function *Outlined = nullptr;
        bool Changed = false;
    };

    static unsigned NextLoopProfileId = 1;

    static const char *modeName(ParallelMode Mode)
    {
        switch (Mode)
        {
        case ParallelMode::DoAll:
            return "DOALL";
        case ParallelMode::DoAcross:
            return "DOACROSS";
        case ParallelMode::None:
            break;
        }
        return "NONE";
    }

    static void logLoopState(Function &F, BasicBlock *Header, unsigned Depth, StringRef Phase)
    {
        (void)F;
        (void)Header;
        (void)Depth;
        (void)Phase;
        if (getenv("SGPL_OUTLINER_DEBUG"))
            errs() << "[outliner] " << Phase << " (fn=" << F.getName() << ")\n";
    }

    static ParallelMode parseParallelMode(Loop *L)
    {
        if (!L || !L->getHeader() || !L->getHeader()->getTerminator())
            return ParallelMode::None;

        MDNode *LoopMD = L->getHeader()->getTerminator()->getMetadata("my.loop.parallel");
        if (!LoopMD)
            return ParallelMode::None;

        for (unsigned I = 0; I < LoopMD->getNumOperands(); ++I)
        {
            auto *MDS = dyn_cast_or_null<MDString>(LoopMD->getOperand(I));
            if (!MDS)
                continue;
            StringRef S = MDS->getString();
            if (!S.starts_with("parallel.type="))
                continue;
            StringRef Val = S.substr(strlen("parallel.type="));
            if (Val.equals_insensitive("DOALL"))
                return ParallelMode::DoAll;
            if (Val.equals_insensitive("DOACROSS"))
                return ParallelMode::DoAcross;
        }

        return ParallelMode::None;
    }

    static AllocaInst *createEntryAlloca(Function &F, Type *Ty, StringRef Name)
    {
        IRBuilder<> B(&*F.getEntryBlock().getFirstInsertionPt());
        return B.CreateAlloca(Ty, nullptr, Name);
    }

    static Value *castIntegerToI64(IRBuilder<> &B, Value *V, StringRef Name)
    {
        Type *Int64Ty = Type::getInt64Ty(B.getContext());
        if (V->getType() == Int64Ty)
            return V;
        if (V->getType()->isIntegerTy())
            return B.CreateSExtOrTrunc(V, Int64Ty, Name);
        return ConstantInt::get(Int64Ty, 0);
    }

    static PHINode *findCanonicalIndVar(Loop *L, BasicBlock *Preheader, BasicBlock *Latch)
    {
        if (!L || !Preheader || !Latch)
            return nullptr;

        for (Instruction &I : *L->getHeader())
        {
            auto *Phi = dyn_cast<PHINode>(&I);
            if (!Phi || Phi->getNumIncomingValues() != 2)
                continue;

            int PreheaderIndex = -1;
            int LatchIndex = -1;
            for (unsigned Incoming = 0; Incoming < 2; ++Incoming)
            {
                if (Phi->getIncomingBlock(Incoming) == Preheader)
                    PreheaderIndex = (int)Incoming;
                if (Phi->getIncomingBlock(Incoming) == Latch)
                    LatchIndex = (int)Incoming;
            }

            if (PreheaderIndex >= 0 && LatchIndex >= 0)
                return Phi;
        }

        return nullptr;
    }

    static StructType *getLoopProfileDescTy(LLVMContext &Ctx);

    static bool isSkippableTimingInst(Instruction &I)
    {
        if (isa<PHINode>(&I) || I.isTerminator())
            return true;
        if (isa<DbgInfoIntrinsic>(&I))
            return true;
        if (auto *II = dyn_cast<IntrinsicInst>(&I))
        {
            switch (II->getIntrinsicID())
            {
            case Intrinsic::lifetime_start:
            case Intrinsic::lifetime_end:
            case Intrinsic::dbg_declare:
            case Intrinsic::dbg_value:
            case Intrinsic::dbg_label:
                return true;
            default:
                break;
            }
        }
        return false;
    }

    static void collectDoAcrossProfileData(LoopCandidateAnalysis &Candidate)
    {
        if (Candidate.Mode != ParallelMode::DoAcross)
            return;

        for (BasicBlock *BB : Candidate.LoopBody)
        {
            bool IsDependent = false;
            for (Instruction &I : *BB)
            {
                if (I.getMetadata("doacross.wait"))
                {
                    Candidate.DoAcrossWaitsPerIter += 1;
                    IsDependent = true;
                }
                if (I.getMetadata("doacross.post"))
                {
                    Candidate.DoAcrossPostsPerIter += 1;
                    IsDependent = true;
                }
            }

            if (IsDependent)
                Candidate.DependentBlocks.insert(BB);
            else
                Candidate.IndependentBlocks.insert(BB);
        }
    }

    static void instrumentDoAcrossSerialBlocks(Function &F,
                                               const LoopVersioningInfo &Versioning,
                                               const LoopCandidateAnalysis &Candidate)
    {
        if (Candidate.Mode != ParallelMode::DoAcross ||
            !Versioning.DoAcrossDepTotalSlot ||
            !Versioning.DoAcrossIndTotalSlot)
            return;

        Module *M = F.getParent();
        LLVMContext &Ctx = F.getContext();
        Type *Int64Ty = Type::getInt64Ty(Ctx);
        FunctionCallee NowFn = M->getOrInsertFunction(
            "sgpl_now_ns",
            FunctionType::get(Int64Ty, {}, false));

        IRBuilder<> InitBuilder(Versioning.SerialStartBlock->getTerminator());
        InitBuilder.CreateStore(ConstantInt::get(Int64Ty, 0), Versioning.DoAcrossDepTotalSlot);
        InitBuilder.CreateStore(ConstantInt::get(Int64Ty, 0), Versioning.DoAcrossIndTotalSlot);

        for (BasicBlock *BB : Versioning.Serial.LoopBody)
        {
            Instruction *FirstReal = nullptr;
            for (Instruction &I : *BB)
            {
                if (!isSkippableTimingInst(I))
                {
                    FirstReal = &I;
                    break;
                }
            }

            if (!FirstReal)
                continue;

            bool IsDependent = Candidate.DependentBlocks.contains(BB);
            AllocaInst *TotalSlot = IsDependent ? Versioning.DoAcrossDepTotalSlot : Versioning.DoAcrossIndTotalSlot;

            IRBuilder<> StartB(FirstReal);
            Value *StartNs = StartB.CreateCall(NowFn, {}, IsDependent ? "sgpl.dep.start" : "sgpl.ind.start");

            IRBuilder<> EndB(BB->getTerminator());
            Value *EndNs = EndB.CreateCall(NowFn, {}, IsDependent ? "sgpl.dep.end" : "sgpl.ind.end");
            Value *Elapsed = EndB.CreateSub(EndNs, StartNs, IsDependent ? "sgpl.dep.elapsed" : "sgpl.ind.elapsed");
            Value *PrevTotal = EndB.CreateLoad(Int64Ty, TotalSlot, IsDependent ? "sgpl.dep.total" : "sgpl.ind.total");
            EndB.CreateStore(EndB.CreateAdd(PrevTotal, Elapsed), TotalSlot);
        }
    }

    static void instrumentDoAcrossWrapper(Function &WrapperFn, GlobalVariable *Descriptor)
    {
        if (!Descriptor || WrapperFn.empty())
            return;

        Module *M = WrapperFn.getParent();
        LLVMContext &Ctx = WrapperFn.getContext();
        StructType *DescTy = getLoopProfileDescTy(Ctx);
        PointerType *DescPtrTy = DescTy->getPointerTo();
        FunctionCallee EnterFn = M->getOrInsertFunction(
            "sgpl_doacross_profile_enter",
            FunctionType::get(Type::getVoidTy(Ctx), {DescPtrTy}, false));
        FunctionCallee ExitFn = M->getOrInsertFunction(
            "sgpl_doacross_profile_exit",
            FunctionType::get(Type::getVoidTy(Ctx), {DescPtrTy}, false));

        BasicBlock &Entry = WrapperFn.getEntryBlock();
        IRBuilder<> EntryBuilder(&*Entry.getFirstInsertionPt());
        EntryBuilder.CreateCall(EnterFn, {Descriptor});

        for (BasicBlock &BB : WrapperFn)
        {
            auto *Ret = dyn_cast<ReturnInst>(BB.getTerminator());
            if (!Ret)
                continue;
            IRBuilder<> ExitBuilder(Ret);
            ExitBuilder.CreateCall(ExitFn, {Descriptor});
        }
    }

    static std::optional<std::string> precheckSharedSetPattern(const LoopCandidateAnalysis &Candidate)
    {
        struct SharedSetRef
        {
            enum class Kind
            {
                Unsupported,
                Parameter,
                DirectGlobal,
            };

            Kind kind = Kind::Unsupported;
            unsigned param_index = 0;
            GlobalVariable *global = nullptr;
            Type *value_ty = nullptr;
        };

        auto classifySharedSetRef = [&](Value *SetArg) -> SharedSetRef {
            SharedSetRef Ref;
            if (!SetArg)
                return Ref;

            SetArg = SetArg->stripPointerCasts();
            if (auto *Arg = dyn_cast<Argument>(SetArg))
            {
                Ref.kind = SharedSetRef::Kind::Parameter;
                Ref.param_index = Arg->getArgNo();
                Ref.value_ty = Arg->getType();
                return Ref;
            }

            if (auto *LI = dyn_cast<LoadInst>(SetArg))
            {
                Value *Storage = getUnderlyingObject(LI->getPointerOperand(), 64);
                if (auto *GV = dyn_cast<GlobalVariable>(Storage))
                {
                    Ref.kind = SharedSetRef::Kind::DirectGlobal;
                    Ref.global = GV;
                    Ref.value_ty = LI->getType();
                    return Ref;
                }
            }

            return Ref;
        };

        auto sameSharedSetRef = [](const SharedSetRef &A, const SharedSetRef &B) -> bool {
            if (A.kind != B.kind)
                return false;
            switch (A.kind)
            {
            case SharedSetRef::Kind::Parameter:
                return A.param_index == B.param_index;
            case SharedSetRef::Kind::DirectGlobal:
                return A.global == B.global;
            case SharedSetRef::Kind::Unsupported:
                break;
            }
            return false;
        };

        SmallVector<SharedSetRef, 8> Reads;
        SmallVector<SharedSetRef, 8> Writes;

        for (BasicBlock *BB : Candidate.LoopBlocks)
        {
            for (Instruction &I : *BB)
            {
                auto *CI = dyn_cast<CallInst>(&I);
                if (!CI)
                    continue;
                Function *Callee = CI->getCalledFunction();
                if (!Callee)
                    continue;

                StringRef Name = Callee->getName();
                if (Name != "roaring_bitmap_add" && Name != "roaring_bitmap_contains")
                    continue;

                SharedSetRef Ref = classifySharedSetRef(CI->getArgOperand(0));
                if (Name == "roaring_bitmap_add")
                {
                    if (Ref.kind == SharedSetRef::Kind::Unsupported)
                        return std::string("shared-set-unsupported-storage");
                    if (Candidate.Mode != ParallelMode::DoAll)
                        return std::string("shared-set-privatization-requires-doall");
                    Writes.push_back(Ref);
                }
                else
                {
                    if (Ref.kind == SharedSetRef::Kind::Unsupported)
                        return std::string("shared-set-unsupported-storage");
                    Reads.push_back(Ref);
                }
            }
        }

        for (const SharedSetRef &WriteRef : Writes)
        {
            for (const SharedSetRef &ReadRef : Reads)
            {
                if (sameSharedSetRef(WriteRef, ReadRef))
                    return std::string("shared-set-read-write-same-object");
            }
        }

        return std::nullopt;
    }

    struct EnvPrivTarget
    {
        uint64_t Offset = 0;
        int32_t Kind = SGPL_PRIV_ROARING;
        int64_t AuxSizeOffset = -1;
        int64_t AuxCapOffset = -1;
    };

    static Value *stripToNamedPointer(Value *V)
    {
        if (!V)
            return nullptr;

        for (unsigned Depth = 0; Depth < 8; ++Depth)
        {
            if (auto *LI = dyn_cast<LoadInst>(V))
            {
                V = LI->getPointerOperand();
                continue;
            }
            if (auto *Cast = dyn_cast<CastInst>(V))
            {
                V = Cast->getOperand(0);
                continue;
            }
            break;
        }

        return V;
    }

    static bool valuesEquivalent(Value *A, Value *B)
    {
        if (!A || !B)
            return false;
        if (A == B)
            return true;
        return stripToNamedPointer(A) == stripToNamedPointer(B);
    }

    static AllocaInst *traceToAlloca(Value *V)
    {
        V = stripToNamedPointer(V);
        if (auto *GEP = dyn_cast<GetElementPtrInst>(V))
            return traceToAlloca(GEP->getPointerOperand());
        return dyn_cast<AllocaInst>(V);
    }

    static bool loopIncrementsAlloca(const LoopTransformTarget &Target, AllocaInst *AI);
    static bool loopIncrementsGlobal(const LoopTransformTarget &Target, GlobalVariable *GV);

    static bool loopIncrementsAlloca(const LoopTransformTarget &Target, AllocaInst *AI)
    {
        if (!AI)
            return false;

        for (BasicBlock *BB : Target.LoopBlocks)
        {
            for (Instruction &I : *BB)
            {
                auto *SI = dyn_cast<StoreInst>(&I);
                if (!SI)
                    continue;
                if (SI->getPointerOperand() != AI)
                    continue;

                Value *Stored = SI->getValueOperand();
                auto *BO = dyn_cast<BinaryOperator>(Stored);
                if (!BO || BO->getOpcode() != Instruction::Add)
                    continue;

                auto *Loaded = dyn_cast<LoadInst>(BO->getOperand(0));
                if (!Loaded || Loaded->getPointerOperand() != AI)
                    Loaded = dyn_cast<LoadInst>(BO->getOperand(1));
                if (!Loaded || Loaded->getPointerOperand() != AI)
                    continue;

                Value *IncrementOperand =
                    Loaded == BO->getOperand(0) ? BO->getOperand(1) : BO->getOperand(0);
                auto *Increment = dyn_cast<ConstantInt>(IncrementOperand);
                if (Increment && Increment->equalsInt(1))
                    return true;
            }
        }

        return false;
    }

    static bool loopReadsArrayExceptAppendIndex(const LoopTransformTarget &Target,
                                                Value *ArrayRoot,
                                                Value *SizeIndexLoad)
    {
        for (BasicBlock *BB : Target.LoopBlocks)
        {
            for (Instruction &I : *BB)
            {
                auto *LI = dyn_cast<LoadInst>(&I);
                if (!LI)
                    continue;

                Value *Ptr = LI->getPointerOperand();
                auto *GEP = dyn_cast<GetElementPtrInst>(Ptr);
                if (!GEP)
                    continue;

                if (!valuesEquivalent(GEP->getPointerOperand(), ArrayRoot))
                    continue;

                if (SizeIndexLoad && GEP->getNumIndices() == 1 && GEP->getOperand(1) == SizeIndexLoad)
                    continue;

                return true;
            }
        }

        return false;
    }

    static std::optional<std::pair<Value *, Value *>>
    detectIntAppendPattern(const LoopTransformTarget &Target)
    {
        std::optional<std::pair<Value *, Value *>> Match;
        for (BasicBlock *BB : Target.LoopBlocks)
        {
            for (Instruction &I : *BB)
            {
                auto *SI = dyn_cast<StoreInst>(&I);
                if (!SI)
                    continue;

                auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
                if (!GEP || GEP->getNumIndices() < 1)
                    continue;

                auto *IdxLoad = dyn_cast<LoadInst>(GEP->getOperand(GEP->getNumOperands() - 1));
                if (!IdxLoad)
                    continue;

                Value *SizeRoot = stripToNamedPointer(IdxLoad->getPointerOperand());
                if (!SizeRoot)
                    continue;

                if (auto *SizeAI = dyn_cast<AllocaInst>(SizeRoot))
                {
                    if (!loopIncrementsAlloca(Target, SizeAI))
                        continue;
                }
                else if (auto *SizeGV = dyn_cast<GlobalVariable>(SizeRoot))
                {
                    if (!loopIncrementsGlobal(Target, SizeGV))
                        continue;
                }
                else
                {
                    continue;
                }

                Value *ArrayRoot = stripToNamedPointer(GEP->getPointerOperand());
                if (!ArrayRoot || loopReadsArrayExceptAppendIndex(Target, ArrayRoot, IdxLoad))
                    continue;

                std::pair<Value *, Value *> Candidate = std::make_pair(ArrayRoot, SizeRoot);
                if (Match && (!valuesEquivalent(Match->first, Candidate.first) ||
                              !valuesEquivalent(Match->second, Candidate.second)))
                    return std::nullopt;
                Match = Candidate;
            }
        }

        return Match;
    }

    static bool loopIncrementsGlobal(const LoopTransformTarget &Target, GlobalVariable *GV)
    {
        if (!GV)
            return false;

        for (BasicBlock *BB : Target.LoopBlocks)
        {
            for (Instruction &I : *BB)
            {
                auto *SI = dyn_cast<StoreInst>(&I);
                if (!SI || SI->getPointerOperand() != GV)
                    continue;

                Value *Stored = SI->getValueOperand();
                auto *BO = dyn_cast<BinaryOperator>(Stored);
                if (!BO || BO->getOpcode() != Instruction::Add)
                    continue;

                auto *Loaded = dyn_cast<LoadInst>(BO->getOperand(0));
                if (!Loaded || Loaded->getPointerOperand() != GV)
                    Loaded = dyn_cast<LoadInst>(BO->getOperand(1));
                if (!Loaded || Loaded->getPointerOperand() != GV)
                    continue;

                Value *IncrementOperand =
                    Loaded == BO->getOperand(0) ? BO->getOperand(1) : BO->getOperand(0);
                auto *Increment = dyn_cast<ConstantInt>(IncrementOperand);
                if (Increment && Increment->equalsInt(1))
                    return true;
            }
        }

        return false;
    }

    static GlobalVariable *findVertexCountGlobal(Function &F)
    {
        for (GlobalVariable &GV : F.getParent()->globals())
        {
            if (!GV.getValueType()->isIntegerTy(32))
                continue;
            if (GV.getName() == "n")
                return &GV;
        }

        for (GlobalVariable &GV : F.getParent()->globals())
        {
            if (!GV.getValueType()->isIntegerTy(32))
                continue;
            for (User *U : GV.users())
            {
                auto *SI = dyn_cast<StoreInst>(U);
                if (!SI || SI->getPointerOperand() != &GV)
                    continue;
                if (isa<CallInst>(SI->getValueOperand()))
                    return &GV;
            }
        }

        return nullptr;
    }

    static void remapAppendGlobalsInOutlinedFunction(Function *Outlined,
                                                     GlobalVariable *FrontierGV,
                                                     GlobalVariable *SizeGV,
                                                     Argument *FrontierArg,
                                                     Argument *SizeArg)
    {
        if (!Outlined || !FrontierGV || !SizeGV || !FrontierArg || !SizeArg)
            return;

        SmallVector<Instruction *, 32> ToErase;
        for (BasicBlock &BB : *Outlined)
        {
            for (Instruction &I : BB)
            {
                auto *LI = dyn_cast<LoadInst>(&I);
                if (!LI)
                    continue;
                if (LI->getPointerOperand() == FrontierGV)
                {
                    LI->replaceAllUsesWith(FrontierArg);
                    ToErase.push_back(LI);
                    continue;
                }
                if (LI->getPointerOperand() == SizeGV)
                {
                    IRBuilder<> RB(LI);
                    Value *Loaded = RB.CreateLoad(Type::getInt32Ty(Outlined->getContext()), SizeArg, "next_size.val");
                    LI->replaceAllUsesWith(Loaded);
                    ToErase.push_back(LI);
                }
            }
        }

        for (Instruction *I : ToErase)
            I->eraseFromParent();
        ToErase.clear();

        for (BasicBlock &BB : *Outlined)
        {
            for (Instruction &I : BB)
            {
                auto *SI = dyn_cast<StoreInst>(&I);
                if (!SI || SI->getPointerOperand() != SizeGV)
                    continue;
                IRBuilder<> RB(SI);
                RB.CreateStore(SI->getValueOperand(), SizeArg);
                ToErase.push_back(SI);
            }
        }

        for (Instruction *I : ToErase)
            I->eraseFromParent();
    }

    static std::optional<uint64_t>
    envOffsetForOrigin(Value *Origin,
                       ArrayRef<Value *> ArgOriginVals,
                       ArrayRef<int> ParamToEnvFieldIndex,
                       const StructLayout *EnvLayout)
    {
        if (!Origin || !EnvLayout)
            return std::nullopt;

        for (unsigned ParamIndex = 0; ParamIndex < ArgOriginVals.size(); ++ParamIndex)
        {
            if (!valuesEquivalent(ArgOriginVals[ParamIndex], Origin))
                continue;
            if (ParamIndex >= ParamToEnvFieldIndex.size())
                return std::nullopt;
            int EnvFieldIndex = ParamToEnvFieldIndex[ParamIndex];
            if (EnvFieldIndex < 0)
                return std::nullopt;
            return EnvLayout->getElementOffset((unsigned)EnvFieldIndex);
        }

        return std::nullopt;
    }

    static int64_t findVertexCapEnvOffset(Function &F,
                                          ArrayRef<Value *> NewEnvOriginVals,
                                          ArrayRef<Type *> NewEnvFieldTys,
                                          const StructLayout *EnvLayout)
    {
        int64_t fallback = -1;

        for (unsigned FieldIndex = 0; FieldIndex < NewEnvOriginVals.size(); ++FieldIndex)
        {
            Value *Orig = NewEnvOriginVals[FieldIndex];
            if (!Orig || !NewEnvFieldTys[FieldIndex]->isIntegerTy(32))
                continue;

            if (fallback < 0)
                fallback = (int64_t)EnvLayout->getElementOffset(FieldIndex);

            if (auto *AI = dyn_cast<AllocaInst>(stripToNamedPointer(Orig)))
            {
                for (User *U : AI->users())
                {
                    auto *SI = dyn_cast<StoreInst>(U);
                    if (!SI || SI->getPointerOperand() != AI)
                        continue;
                    if (auto *CI = dyn_cast<CallInst>(SI->getValueOperand()))
                    {
                        Function *Callee = CI->getCalledFunction();
                        if (Callee && Callee->getName() == "numVertices")
                            return (int64_t)EnvLayout->getElementOffset(FieldIndex);
                    }
                }
            }
        }

        (void)F;
        return fallback;
    }

    static bool isTrivialLoopBodyInst(Instruction &I, Loop *L)
    {
        if (isa<PHINode>(&I) || I.isTerminator())
            return true;
        if (isa<DbgInfoIntrinsic>(&I))
            return true;
        if (auto *II = dyn_cast<IntrinsicInst>(&I))
        {
            switch (II->getIntrinsicID())
            {
            case Intrinsic::lifetime_start:
            case Intrinsic::lifetime_end:
            case Intrinsic::dbg_declare:
            case Intrinsic::dbg_value:
            case Intrinsic::dbg_label:
                return true;
            default:
                break;
            }
        }

        // Treat the canonical latch increment/compare scaffolding as trivial when obvious.
        if (auto *BO = dyn_cast<BinaryOperator>(&I))
        {
            if ((BO->getOpcode() == Instruction::Add || BO->getOpcode() == Instruction::Sub) &&
                L && L->isLoopInvariant(BO->getOperand(1)))
            {
                for (User *U : BO->users())
                {
                    if (auto *PN = dyn_cast<PHINode>(U))
                    {
                        if (PN->getParent() == L->getHeader())
                            return true;
                    }
                }
            }
        }
        if (auto *Cmp = dyn_cast<ICmpInst>(&I))
        {
            if (L)
            {
                bool HasVariantOperand = false;
                for (Value *Op : Cmp->operands())
                {
                    if (!L->isLoopInvariant(Op))
                        HasVariantOperand = true;
                }
                if (HasVariantOperand)
                    return true;
            }
        }
        if (isa<CastInst>(&I))
        {
            bool FeedsOnlyTrivialUses = true;
            for (User *U : I.users())
            {
                if (!isa<ICmpInst>(U) && !isa<BranchInst>(U) && !isa<PHINode>(U) && !isa<CastInst>(U))
                {
                    FeedsOnlyTrivialUses = false;
                    break;
                }
            }
            if (FeedsOnlyTrivialUses)
                return true;
        }

        return false;
    }

    static unsigned countEffectiveLoopBodyInstructions(const LoopCandidateAnalysis &Candidate)
    {
        unsigned Count = 0;
        for (BasicBlock *BB : Candidate.LoopBody)
        {
            for (Instruction &I : *BB)
            {
                if (isTrivialLoopBodyInst(I, Candidate.LoopObj))
                    continue;
                ++Count;
            }
        }
        return Count;
    }

    static std::optional<uint64_t> getConstantTripCount(const LoopCandidateAnalysis &Candidate, ScalarEvolution &SE)
    {
        if (!Candidate.BackedgeCount)
            return std::nullopt;

        const SCEV *TripCount = SE.getAddExpr(
            Candidate.BackedgeCount,
            SE.getOne(Candidate.BackedgeCount->getType()));
        if (auto *C = dyn_cast<SCEVConstant>(TripCount))
        {
            const APInt &V = C->getAPInt();
            if (V.isNegative())
                return std::nullopt;
            return V.getZExtValue();
        }
        return std::nullopt;
    }

    static std::optional<const SCEV *> deriveHalfOpenLoopEndFromExitICmp(const LoopCandidateAnalysis &Candidate,
                                                                         ScalarEvolution &SE)
    {
        if (!Candidate.ExitingBlock || !Candidate.IndVar)
            return std::nullopt;

        auto *BI = dyn_cast<BranchInst>(Candidate.ExitingBlock->getTerminator());
        if (!BI || !BI->isConditional())
            return std::nullopt;

        auto *Cmp = dyn_cast<ICmpInst>(BI->getCondition());
        if (!Cmp)
            return std::nullopt;

        const SCEV *IndVarSCEV = SE.getSCEV(Candidate.IndVar);
        auto matchIndVarOperand = [&](Value *V) -> bool {
            Value *Base = V->stripPointerCasts();
            if (Base == Candidate.IndVar)
                return true;
            if (auto *Cast = dyn_cast<CastInst>(Base))
                return Cast->getOperand(0)->stripPointerCasts() == Candidate.IndVar;
            return SE.getSCEV(V) == IndVarSCEV;
        };

        auto getBoundSCEV = [&](Value *V) -> const SCEV * {
            if (!Candidate.LoopObj->isLoopInvariant(V))
                return nullptr;
            return SE.getSCEV(V);
        };

        Value *LHS = Cmp->getOperand(0);
        Value *RHS = Cmp->getOperand(1);
        bool LHSIsIndVar = matchIndVarOperand(LHS);
        bool RHSIsIndVar = matchIndVarOperand(RHS);
        Type *CmpTy = Cmp->getOperand(0)->getType();

        switch (Cmp->getPredicate())
        {
        case CmpInst::ICMP_SLT:
        case CmpInst::ICMP_ULT:
            if (LHSIsIndVar)
                if (const SCEV *Bound = getBoundSCEV(RHS))
                    return Bound;
            break;
        case CmpInst::ICMP_SLE:
        case CmpInst::ICMP_ULE:
            if (LHSIsIndVar)
                if (const SCEV *Bound = getBoundSCEV(RHS))
                    return SE.getAddExpr(Bound, SE.getOne(CmpTy));
            break;
        case CmpInst::ICMP_SGT:
        case CmpInst::ICMP_UGT:
            if (RHSIsIndVar)
                if (const SCEV *Bound = getBoundSCEV(LHS))
                    return Bound;
            break;
        case CmpInst::ICMP_SGE:
        case CmpInst::ICMP_UGE:
            if (RHSIsIndVar)
                if (const SCEV *Bound = getBoundSCEV(LHS))
                    return SE.getAddExpr(Bound, SE.getOne(CmpTy));
            break;
        default:
            break;
        }

        return std::nullopt;
    }

    static std::optional<std::string> precheckObviousUnprofitableLoop(const LoopCandidateAnalysis &Candidate,
                                                                      ScalarEvolution &SE)
    {
        if (std::optional<uint64_t> TripCount = getConstantTripCount(Candidate, SE))
        {
            if (*TripCount <= 1)
                return std::string("constant-trip-count-leq-1");
            if (*TripCount < 32)
                return std::string("constant-trip-count-small");
        }

        if (countEffectiveLoopBodyInstructions(Candidate) < 8)
            return std::string("trivial-loop-body");

        return std::nullopt;
    }

    static std::optional<LoopCandidateAnalysis> analyzeLoopCandidate(Function &F,
                                                                     Loop *L,
                                                                     DominatorTree &DT,
                                                                     ScalarEvolution &SE)
    {
        LoopCandidateAnalysis Candidate;
        Candidate.LoopObj = L;
        Candidate.Mode = parseParallelMode(L);
        Candidate.Header = L ? L->getHeader() : nullptr;
        Candidate.Depth = L ? L->getLoopDepth() : 0;
        Candidate.RequiresIntAppendPriv =
            Candidate.Header && Candidate.Header->getTerminator() &&
            Candidate.Header->getTerminator()->getMetadata(
                "sgpl.frontier.first_wins.doall");

        if (Candidate.Mode == ParallelMode::None)
            return std::nullopt;

        Candidate.Preheader = L->getLoopPreheader();
        Candidate.Latch = L->getLoopLatch();
        Candidate.ExitingBlock = L->getExitingBlock();
        Candidate.ExitBlock = L->getExitBlock();

        if (!Candidate.Preheader || !Candidate.Latch || !Candidate.ExitingBlock || !Candidate.ExitBlock)
        {
            logLoopState(F, Candidate.Header, Candidate.Depth, "skip:not-versionable reason=missing-structure");
            return std::nullopt;
        }

        for (Instruction &I : *Candidate.ExitBlock)
        {
            if (isa<PHINode>(&I))
            {
                logLoopState(F, Candidate.Header, Candidate.Depth, "skip:not-versionable reason=exit-phis");
                return std::nullopt;
            }
        }

        Candidate.IndVar = findCanonicalIndVar(L, Candidate.Preheader, Candidate.Latch);
        if (!Candidate.IndVar)
        {
            logLoopState(F, Candidate.Header, Candidate.Depth, "skip:not-versionable reason=no-canonical-indvar");
            return std::nullopt;
        }

        for (BasicBlock *BB : L->blocks())
            Candidate.LoopBlocks.push_back(BB);
        for (BasicBlock *BB : Candidate.LoopBlocks)
        {
            if (BB != Candidate.Header)
                Candidate.LoopBody.push_back(BB);
        }

        if (Candidate.LoopBody.empty())
        {
            logLoopState(F, Candidate.Header, Candidate.Depth, "skip:not-versionable reason=empty-loop-body");
            return std::nullopt;
        }

        if (Candidate.RequiresIntAppendPriv)
        {
            LoopTransformTarget AppendTarget;
            AppendTarget.Mode = Candidate.Mode;
            AppendTarget.Header = Candidate.Header;
            AppendTarget.LoopBlocks = Candidate.LoopBlocks;
            AppendTarget.LoopBody = Candidate.LoopBody;
            std::optional<std::pair<Value *, Value *>> AppendPattern =
                detectIntAppendPattern(AppendTarget);
            if (!AppendPattern ||
                !isa<GlobalVariable>(AppendPattern->first) ||
                !isa<GlobalVariable>(AppendPattern->second) ||
                !findVertexCountGlobal(F))
            {
                logLoopState(F, Candidate.Header, Candidate.Depth,
                             "skip:frontier-append-privatization-unavailable");
                return std::nullopt;
            }
        }

        Candidate.BackedgeCount = SE.getBackedgeTakenCount(L);
        bool HasComputedBackedge =
            !isa<SCEVCouldNotCompute>(Candidate.BackedgeCount);
        if (!HasComputedBackedge && !Candidate.RequiresIntAppendPriv)
        {
            logLoopState(F, Candidate.Header, Candidate.Depth, "skip:not-versionable reason=backedge-could-not-compute");
            return std::nullopt;
        }

        const SCEV *IndVarSCEV = SE.getSCEV(Candidate.IndVar);
        auto *AR = dyn_cast<SCEVAddRecExpr>(IndVarSCEV);
        if (!AR || AR->getLoop() != L)
        {
            logLoopState(F, Candidate.Header, Candidate.Depth, "skip:not-versionable reason=indvar-not-addrec");
            return std::nullopt;
        }

        Candidate.Start = AR->getStart();
        Candidate.Step = AR->getStepRecurrence(SE);
        if (HasComputedBackedge)
        {
            const SCEV *TripCount = SE.getAddExpr(
                Candidate.BackedgeCount,
                SE.getOne(Candidate.BackedgeCount->getType()));
            Candidate.End = SE.getAddExpr(
                Candidate.Start,
                SE.getMulExpr(Candidate.Step, TripCount));
            Candidate.ConstantTripCount = getConstantTripCount(Candidate, SE);
        }

        if (std::optional<const SCEV *> ExitBound =
                deriveHalfOpenLoopEndFromExitICmp(Candidate, SE))
            Candidate.End = *ExitBound;
        else if (!Candidate.End)
        {
            logLoopState(F, Candidate.Header, Candidate.Depth,
                         "skip:not-versionable reason=missing-loop-end");
            return std::nullopt;
        }
        Candidate.EffectiveBodyInstCount = countEffectiveLoopBodyInstructions(Candidate);

        CodeExtractor CE(Candidate.LoopBody, &DT);
        if (!CE.isEligible())
        {
            logLoopState(F, Candidate.Header, Candidate.Depth, "skip:not-versionable reason=code-extractor-ineligible");
            return std::nullopt;
        }

        if (std::optional<std::string> Reason = precheckSharedSetPattern(Candidate))
        {
            logLoopState(F, Candidate.Header, Candidate.Depth,
                         Twine("skip:not-versionable reason=" + *Reason).str());
            return std::nullopt;
        }

        collectDoAcrossProfileData(Candidate);
        if (Candidate.Mode == ParallelMode::DoAcross)
        {
            /* Debug logging disabled: doacross-profile */
        }

        if (std::optional<std::string> Reason = precheckObviousUnprofitableLoop(Candidate, SE))
        {
            logLoopState(F, Candidate.Header, Candidate.Depth,
                         Twine("skip:not-profitable reason=" + *Reason).str());
            return std::nullopt;
        }

        /* Debug logging disabled: candidate-selected */
        return Candidate;
    }

    static std::optional<LoopCandidateAnalysis> findOutermostParallelizableLoopRecursive(
        Function &F,
        Loop *L,
        DominatorTree &DT,
        ScalarEvolution &SE)
    {
        /* Debug logging disabled: inspect */

        if (std::optional<LoopCandidateAnalysis> Candidate = analyzeLoopCandidate(F, L, DT, SE))
            return Candidate;

        for (Loop *SubLoop : L->getSubLoops())
        {
            if (std::optional<LoopCandidateAnalysis> SubCandidate =
                    findOutermostParallelizableLoopRecursive(F, SubLoop, DT, SE))
                return SubCandidate;
        }

        return std::nullopt;
    }

    static std::optional<LoopCandidateAnalysis> findOutermostParallelizableLoop(Function &F,
                                                                                LoopInfo &LI,
                                                                                DominatorTree &DT,
                                                                                ScalarEvolution &SE)
    {
        for (Loop *TopLevel : LI)
        {
            if (std::optional<LoopCandidateAnalysis> Candidate =
                    findOutermostParallelizableLoopRecursive(F, TopLevel, DT, SE))
                return Candidate;
        }
        return std::nullopt;
    }

    static Function *findExtractedCallSite(Function &F, Function *Outlined, SmallVectorImpl<Value *> &ArgOrigins)
    {
        for (BasicBlock &BB : F)
        {
            for (Instruction &I : BB)
            {
                auto *CI = dyn_cast<CallInst>(&I);
                if (!CI || CI->getCalledFunction() != Outlined)
                    continue;

                ArgOrigins.clear();
                for (Value *Arg : CI->args())
                    ArgOrigins.push_back(Arg);
                return Outlined;
            }
        }

        return nullptr;
    }

    static LoopVersioningInfo createLoopMultiversion(Function &F,
                                                     const LoopCandidateAnalysis &Candidate,
                                                     ScalarEvolution &SE)
    {
        Module *M = F.getParent();
        LLVMContext &Ctx = F.getContext();
        BasicBlock *Header = Candidate.Header;
        BasicBlock *Preheader = Candidate.Preheader;
        BasicBlock *ExitBlock = Candidate.ExitBlock;
        BasicBlock *ExitingBlock = Candidate.ExitingBlock;
        BasicBlock *Latch = Candidate.Latch;
        Instruction *PreheaderTerm = Preheader->getTerminator();

        SCEVExpander Expander(SE, M->getDataLayout(), "mv.scev");
        Value *StartArg = Expander.expandCodeFor(Candidate.Start, Candidate.IndVar->getType(), PreheaderTerm);
        Value *StepArg = Expander.expandCodeFor(Candidate.Step, Candidate.IndVar->getType(), PreheaderTerm);
        Value *EndArg = Expander.expandCodeFor(Candidate.End, Candidate.IndVar->getType(), PreheaderTerm);

        std::string BaseName = Header->getName().str();
        BasicBlock *DispatchBlock = BasicBlock::Create(Ctx, "mv.dispatch." + BaseName, &F, Header);
        BasicBlock *SerialStartBlock = BasicBlock::Create(Ctx, "mv.serial.start." + BaseName, &F, Header);
        BasicBlock *ParallelPreheader = BasicBlock::Create(Ctx, "mv.parallel.preheader." + BaseName, &F, Header);
        BasicBlock *ParallelExitBridge = BasicBlock::Create(Ctx, "mv.parallel.exit." + BaseName, &F, ExitBlock);
        BasicBlock *SerialExitBridge = BasicBlock::Create(Ctx, "mv.serial.exit." + BaseName, &F, ExitBlock);

        ValueToValueMapTy VMap;
        VMap[Preheader] = ParallelPreheader;
        VMap[ExitBlock] = ParallelExitBridge;

        SmallVector<BasicBlock *, 8> ClonedLoopBlocks;
        ClonedLoopBlocks.reserve(Candidate.LoopBlocks.size());
        for (BasicBlock *BB : Candidate.LoopBlocks)
        {
            BasicBlock *Clone = CloneBasicBlock(BB, VMap, ".parallel", &F);
            VMap[BB] = Clone;
            ClonedLoopBlocks.push_back(Clone);
        }

        for (BasicBlock *BB : Candidate.LoopBlocks)
        {
            BasicBlock *Clone = cast<BasicBlock>(VMap[BB]);
            for (Instruction &I : *Clone)
                RemapInstruction(&I, VMap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);
        }

        auto *ClonedHeader = cast<BasicBlock>(VMap[Header]);
        auto *ClonedLatch = cast<BasicBlock>(VMap[Latch]);
        auto *ClonedExitingBlock = cast<BasicBlock>(VMap[ExitingBlock]);
        auto *ClonedIndVar = cast<PHINode>(VMap[Candidate.IndVar]);

        for (Instruction &I : *Header)
        {
            auto *OrigPhi = dyn_cast<PHINode>(&I);
            if (!OrigPhi)
                break;
            for (unsigned Index = 0; Index < OrigPhi->getNumIncomingValues(); ++Index)
            {
                if (OrigPhi->getIncomingBlock(Index) == Preheader)
                    OrigPhi->setIncomingBlock(Index, SerialStartBlock);
            }
        }

        for (Instruction &I : *ClonedHeader)
        {
            auto *Phi = dyn_cast<PHINode>(&I);
            if (!Phi)
                break;
            for (unsigned Index = 0; Index < Phi->getNumIncomingValues(); ++Index)
            {
                BasicBlock *IncomingBB = Phi->getIncomingBlock(Index);
                if (IncomingBB == Preheader)
                    Phi->setIncomingBlock(Index, ParallelPreheader);
                else if (Value *Mapped = VMap.lookup(IncomingBB))
                    Phi->setIncomingBlock(Index, cast<BasicBlock>(Mapped));
            }
        }

        IRBuilder<> DispatchBuilder(DispatchBlock);
        DispatchBuilder.CreateBr(SerialStartBlock);

        IRBuilder<> SerialStartBuilder(SerialStartBlock);
        SerialStartBuilder.CreateBr(Header);

        IRBuilder<> ParallelPreheaderBuilder(ParallelPreheader);
        ParallelPreheaderBuilder.CreateBr(ClonedHeader);

        ExitingBlock->getTerminator()->replaceSuccessorWith(ExitBlock, SerialExitBridge);

        IRBuilder<> ParallelExitBuilder(ParallelExitBridge);
        ParallelExitBuilder.CreateBr(ExitBlock);

        IRBuilder<> SerialExitBuilder(SerialExitBridge);
        SerialExitBuilder.CreateBr(ExitBlock);

        PreheaderTerm->replaceSuccessorWith(Header, DispatchBlock);

        LoopVersioningInfo Info;
        Info.Mode = Candidate.Mode;
        Info.DispatchBlock = DispatchBlock;
        Info.SerialStartBlock = SerialStartBlock;
        Info.SerialExitBridge = SerialExitBridge;
        Info.ParallelExitBridge = ParallelExitBridge;
        Info.SharedExitBlock = ExitBlock;
        Info.StartArg = StartArg;
        Info.StepArg = StepArg;
        Info.EndArg = EndArg;
        Info.LoopId = NextLoopProfileId++;
        if (Candidate.Mode == ParallelMode::DoAll)
            Info.SerialStartTimeSlot = createEntryAlloca(F, Type::getInt64Ty(Ctx), "sgpl.serial.start.ns");
        else if (Candidate.Mode == ParallelMode::DoAcross)
        {
            Info.DoAcrossDepTotalSlot = createEntryAlloca(F, Type::getInt64Ty(Ctx), "sgpl.doacross.dep.total.ns");
            Info.DoAcrossIndTotalSlot = createEntryAlloca(F, Type::getInt64Ty(Ctx), "sgpl.doacross.ind.total.ns");
        }

        Info.Serial.Mode = Candidate.Mode;
        Info.Serial.Header = Header;
        Info.Serial.Preheader = Preheader;
        Info.Serial.Latch = Latch;
        Info.Serial.ExitingBlock = ExitingBlock;
        Info.Serial.ExitBlock = SerialExitBridge;
        Info.Serial.IndVar = Candidate.IndVar;
        Info.Serial.LoopBlocks = Candidate.LoopBlocks;
        Info.Serial.LoopBody = Candidate.LoopBody;
        Info.Serial.Depth = Candidate.Depth;
        Info.Serial.RequiresIntAppendPriv = Candidate.RequiresIntAppendPriv;

        Info.Parallel.Mode = Candidate.Mode;
        Info.Parallel.Header = ClonedHeader;
        Info.Parallel.Preheader = ParallelPreheader;
        Info.Parallel.Latch = ClonedLatch;
        Info.Parallel.ExitingBlock = ClonedExitingBlock;
        Info.Parallel.ExitBlock = ParallelExitBridge;
        Info.Parallel.IndVar = ClonedIndVar;
        Info.Parallel.Depth = Candidate.Depth;
        Info.Parallel.RequiresIntAppendPriv = Candidate.RequiresIntAppendPriv;
        Info.Parallel.LoopBlocks = ClonedLoopBlocks;
        for (BasicBlock *BB : ClonedLoopBlocks)
        {
            if (BB != ClonedHeader)
                Info.Parallel.LoopBody.push_back(BB);
        }

        /* Debug logging disabled: multiversion-created */
        return Info;
    }

    static Constant *createCStringPtr(Module &M, StringRef Text, StringRef GlobalName);

    static bool isGpuBackendEnabled(Module &M)
    {
        if (NamedMDNode *NMD = M.getNamedMetadata("graph.ir.backend"))
        {
            for (const MDNode *Op : NMD->operands())
            {
                if (!Op)
                    continue;
                for (const MDOperand &MO : Op->operands())
                {
                    if (const MDString *MDS = dyn_cast_or_null<MDString>(MO.get()))
                    {
                        if (MDS->getString() == "gpu")
                            return true;
                    }
                }
            }
        }
        return false;
    }

    static bool gpuGlobalIsDeviceSafe(const GlobalVariable *GV)
    {
        if (!GV)
            return false;
        if (GV->isConstant())
            return false; // constants are never copied at runtime
        if (GV->isThreadLocal())
            return false;
        if (GV->isDeclaration())
            return false;
        if (!GV->hasName())
            return false; // needed for cuModuleGetGlobal lookup by name
        if (!GV->getValueType()->isSized())
            return false;
        return true;
    }

    static bool gpuFunctionIsDeviceSafe(Function *Fn,
                                        SmallPtrSetImpl<GlobalVariable *> *ReferencedGlobals,
                                        SmallPtrSetImpl<Function *> &Visited)
    {
        if (!Fn)
            return false;
        if (Fn->isIntrinsic())
            return true;
        if (!Visited.insert(Fn).second)
            return true;

        for (BasicBlock &BB : *Fn)
        {
            for (Instruction &I : BB)
            {
                if (auto *CI = dyn_cast<CallInst>(&I))
                {
                    Function *Callee = CI->getCalledFunction();
                    if (!Callee)
                        return false;
                    if (!Callee->isIntrinsic())
                    {
                        // DOACROSS bodies carry doacross_wait/post/init calls;
                        // they are stripped from the device clones in
                        // emitGpuKernels (the wave barriers subsume them).
                        StringRef CalleeName = Callee->getName();
                        if (CalleeName == "doacross_wait" ||
                            CalleeName == "doacross_post" ||
                            CalleeName == "doacross_init" ||
                            CalleeName == "sgpl_doacross_profile_enter" ||
                            CalleeName == "sgpl_doacross_profile_exit")
                            continue;
                        if (Callee->isDeclaration())
                            return false;
                        if (!gpuFunctionIsDeviceSafe(Callee, ReferencedGlobals, Visited))
                            return false;
                    }
                }

                for (Value *Op : I.operands())
                {
                    if (auto *GV = dyn_cast<GlobalVariable>(Op->stripPointerCasts()))
                    {
                        if (!gpuGlobalIsDeviceSafe(GV))
                            return false;
                        if (ReferencedGlobals)
                            ReferencedGlobals->insert(GV);
                    }
                }
            }
        }
        return true;
    }

    static void stripDeviceIncompatibleIntrinsics(Function &Fn)
    {
        SmallVector<Instruction *, 16> ToErase;
        for (BasicBlock &BB : Fn)
        {
            for (Instruction &I : BB)
            {
                auto *CI = dyn_cast<CallInst>(&I);
                if (!CI)
                    continue;
                Function *Callee = CI->getCalledFunction();
                if (!Callee || !Callee->isIntrinsic())
                    continue;
                switch (Callee->getIntrinsicID())
                {
                case Intrinsic::lifetime_start:
                case Intrinsic::lifetime_end:
                case Intrinsic::dbg_declare:
                case Intrinsic::dbg_value:
                case Intrinsic::dbg_label:
                case Intrinsic::dbg_assign:
                    ToErase.push_back(CI);
                    break;
                default:
                    break;
                }
            }
        }
        for (Instruction *I : ToErase)
            I->eraseFromParent();
    }

    static ParallelTransformResult transformLoopCloneToParallel(Function &F,
                                                                const LoopTransformTarget &Target,
                                                                Value *StartV,
                                                                Value *EndV,
                                                                Value *StepV)
    {
        ParallelTransformResult Result;
        Module *M = F.getParent();
        LLVMContext &Ctx = F.getContext();

        // Minimum constant DOACROSS dependence distance (in iterations). The
        // GPU wave kernel uses it as the wave width: wave w owns iterations
        // [w*d, (w+1)*d), which only depend on strictly earlier waves.
        unsigned GpuDoAcrossMinDist = 0;

        if (!Target.Preheader || !Target.ExitBlock || !Target.Preheader->getTerminator())
        {
            logLoopState(F, Target.Header, Target.Depth, "skip:parallel-clone-invalid-structure");
            return Result;
        }

        bool IsDoAcross = Target.Mode == ParallelMode::DoAcross;
        bool IsDoAll = Target.Mode == ParallelMode::DoAll;

        if (Target.RequiresIntAppendPriv)
        {
            std::optional<std::pair<Value *, Value *>> AppendPattern =
                detectIntAppendPattern(Target);
            if (!AppendPattern ||
                !isa<GlobalVariable>(AppendPattern->first) ||
                !isa<GlobalVariable>(AppendPattern->second) ||
                !findVertexCountGlobal(F))
            {
                logLoopState(F, Target.Header, Target.Depth,
                             "skip:frontier-append-privatization-unavailable");
                return Result;
            }
        }

        if (IsDoAcross)
        {
            Type *VoidTy = Type::getVoidTy(Ctx);
            Type *Int64Ty = Type::getInt64Ty(Ctx);
            Type *Int32Ty = Type::getInt32Ty(Ctx);
            FunctionCallee WaitFn = M->getOrInsertFunction(
                "doacross_wait",
                FunctionType::get(VoidTy, {Int64Ty, Int64Ty, Int32Ty}, false));
            FunctionCallee PostFn = M->getOrInsertFunction(
                "doacross_post",
                FunctionType::get(VoidTy, {Int64Ty, Int32Ty}, false));

            struct InstrAction
            {
                Instruction *I = nullptr;
                bool IsWait = false;
                int64_t Dist = 0;
                int32_t Id = 0;
            };

            SmallVector<InstrAction, 8> Actions;
            DenseMap<int32_t, int32_t> SyncIdRemap;
            for (BasicBlock *BB : Target.LoopBody)
            {
                for (Instruction &I : *BB)
                {
                    if (MDNode *WaitMD = I.getMetadata("doacross.wait"))
                    {
                        (void)WaitMD;
                        int64_t Dist = 0;
                        int32_t Id = 0;
                        if (MDNode *DistMD = I.getMetadata("doacross.dist"))
                            if (auto *CAM = dyn_cast<ConstantAsMetadata>(DistMD->getOperand(0)))
                                if (auto *CI = dyn_cast<ConstantInt>(CAM->getValue()))
                                    Dist = CI->getSExtValue();
                        if (MDNode *SrcMD = I.getMetadata("doacross.src"))
                            if (auto *CAM = dyn_cast<ConstantAsMetadata>(SrcMD->getOperand(0)))
                                if (auto *CI = dyn_cast<ConstantInt>(CAM->getValue()))
                                    Id = (int32_t)CI->getSExtValue();
                        if (!SyncIdRemap.count(Id))
                        {
                            int32_t LocalId = (int32_t)SyncIdRemap.size();
                            SyncIdRemap[Id] = LocalId;
                            /* Debug logging disabled: doacross-remap */
                        }
                        if (Dist > 0 && (GpuDoAcrossMinDist == 0 || (unsigned)Dist < GpuDoAcrossMinDist))
                            GpuDoAcrossMinDist = (unsigned)Dist;
                        Actions.push_back({&I, true, Dist, Id});
                    }
                    if (I.getMetadata("doacross.post"))
                    {
                        int32_t Id = 0;
                        if (MDNode *IDMD = I.getMetadata("doacross.id"))
                            if (auto *CAM = dyn_cast<ConstantAsMetadata>(IDMD->getOperand(0)))
                                if (auto *CI = dyn_cast<ConstantInt>(CAM->getValue()))
                                    Id = (int32_t)CI->getSExtValue();
                        if (!SyncIdRemap.count(Id))
                        {
                            int32_t LocalId = (int32_t)SyncIdRemap.size();
                            SyncIdRemap[Id] = LocalId;
                            /* Debug logging disabled: doacross-remap */
                        }
                        Actions.push_back({&I, false, 0, Id});
                    }
                }
            }
            Result.NumDoAcrossSyncIds = SyncIdRemap.size();
            /* Debug logging disabled: doacross-num-sync-ids */

            for (const InstrAction &Action : Actions)
            {
                IRBuilder<> B(Action.I);
                Value *IndVar64 = Target.IndVar;
                if (Target.IndVar->getType() != Type::getInt64Ty(Ctx))
                    IndVar64 = B.CreateIntCast(Target.IndVar, Type::getInt64Ty(Ctx), true, "indvar.i64");

                if (Action.IsWait)
                {
                    B.CreateCall(WaitFn,
                                 {IndVar64,
                                  ConstantInt::get(Type::getInt64Ty(Ctx), Action.Dist),
                                  ConstantInt::get(Type::getInt32Ty(Ctx), SyncIdRemap.lookup(Action.Id))});
                }
                else
                {
                    Instruction *InsertBefore = Action.I->getNextNode();
                    IRBuilder<> PostBuilder(Ctx);
                    if (!InsertBefore)
                        PostBuilder.SetInsertPoint(Action.I->getParent());
                    else
                        PostBuilder.SetInsertPoint(InsertBefore);
                    PostBuilder.CreateCall(PostFn,
                                           {IndVar64, ConstantInt::get(Type::getInt32Ty(Ctx), SyncIdRemap.lookup(Action.Id))});
                }
            }
        }

        CodeExtractor CE(Target.LoopBody, nullptr);
        if (!CE.isEligible())
        {
            logLoopState(F, Target.Header, Target.Depth, "skip:parallel-clone-code-extractor-ineligible");
            return Result;
        }

        CodeExtractorAnalysisCache CEAC(F);
        SetVector<Value *> Inputs;
        SetVector<Value *> Outputs;
        SetVector<Value *> SinkCands;
        CE.findInputsOutputs(Inputs, Outputs, SinkCands, true);

        SmallPtrSet<BasicBlock *, 8> LoopBlockSet(Target.LoopBlocks.begin(), Target.LoopBlocks.end());
        for (BasicBlock *BB : Target.LoopBody)
        {
            for (Instruction &I : *BB)
            {
                auto *SI = dyn_cast<StoreInst>(&I);
                if (!SI)
                    continue;
                Value *Base = getUnderlyingObject(SI->getPointerOperand(), 64);
                if (!Base)
                    continue;
                if (auto *BI = dyn_cast<Instruction>(Base))
                {
                    if (LoopBlockSet.contains(BI->getParent()))
                        continue;
                }
                if (!Inputs.count(Base) && !Outputs.count(Base))
                    Outputs.insert(Base);
            }
        }

        Function *Outlined = CE.extractCodeRegion(CEAC);
        if (!Outlined)
        {
            logLoopState(F, Target.Header, Target.Depth, "skip:extract-code-region-returned-null");
            return Result;
        }

        Result.Changed = true;
        Result.Outlined = Outlined;
        Outlined->setLinkage(GlobalValue::ExternalLinkage);
        Outlined->setName("outlined_" + F.getName().str() + "_" + Target.Header->getName().str());

        SmallVector<Value *, 8> ArgOriginVals;
        if (!findExtractedCallSite(F, Outlined, ArgOriginVals))
        {
            logLoopState(F, Target.Header, Target.Depth, "skip:replacement-call-not-found");
            return Result;
        }

        if (ArgOriginVals.size() != Outlined->arg_size())
        {
            logLoopState(F, Target.Header, Target.Depth, "skip:replacement-call-arg-mismatch");
            return Result;
        }

        SmallVector<Type *, 8> NewEnvFieldTys;
        SmallVector<Value *, 8> NewEnvOriginVals;
        SmallVector<bool, 8> ParamUsesPrivateScratch(ArgOriginVals.size(), false);
        SmallVector<Type *, 8> ParamPrivateScratchTys(ArgOriginVals.size(), nullptr);
        SmallVector<int, 8> ParamToEnvFieldIndex(ArgOriginVals.size(), -1);
        SmallVector<bool, 8> ParamReadsSharedSet(ArgOriginVals.size(), false);
        SmallVector<bool, 8> ParamWritesSharedSet(ArgOriginVals.size(), false);
        SmallVector<bool, 8> ParamUsesPrivatizedSet(ArgOriginVals.size(), false);
        SmallVector<GlobalVariable *, 4> GlobalWriteSharedSets;
        SmallVector<GlobalVariable *, 4> GlobalReadSharedSets;
        SmallVector<Type *, 4> GlobalWriteSharedSetValueTys;
        DenseMap<GlobalVariable *, unsigned> GlobalWriteSetIndices;
        DenseMap<GlobalVariable *, unsigned> GlobalWriteSetEnvFieldIndices;
        int InductionParamIndex = -1;

        struct OutlinedSharedSetRef
        {
            enum class Kind
            {
                Unsupported,
                Parameter,
                DirectGlobal,
            };

            Kind kind = Kind::Unsupported;
            unsigned param_index = 0;
            GlobalVariable *global = nullptr;
            Type *value_ty = nullptr;
        };

        auto classifyOutlinedSharedSetRef = [&](Value *SetArg) -> OutlinedSharedSetRef {
            OutlinedSharedSetRef Ref;
            if (!SetArg)
                return Ref;

            SetArg = SetArg->stripPointerCasts();
            if (auto *Arg = dyn_cast<Argument>(SetArg))
            {
                Ref.kind = OutlinedSharedSetRef::Kind::Parameter;
                Ref.param_index = Arg->getArgNo();
                Ref.value_ty = Arg->getType();
                return Ref;
            }

            if (auto *LI = dyn_cast<LoadInst>(SetArg))
            {
                Value *Storage = getUnderlyingObject(LI->getPointerOperand(), 64);
                if (auto *GV = dyn_cast<GlobalVariable>(Storage))
                {
                    Ref.kind = OutlinedSharedSetRef::Kind::DirectGlobal;
                    Ref.global = GV;
                    Ref.value_ty = LI->getType();
                    return Ref;
                }
            }

            return Ref;
        };

        for (BasicBlock &OutlinedBB : *Outlined)
        {
            for (Instruction &OutlinedI : OutlinedBB)
            {
                auto *CI = dyn_cast<CallInst>(&OutlinedI);
                if (!CI)
                    continue;
                Function *Callee = CI->getCalledFunction();
                if (!Callee)
                    continue;
                StringRef CalleeName = Callee->getName();
                if (CalleeName != "roaring_bitmap_add" && CalleeName != "roaring_bitmap_contains")
                    continue;

                OutlinedSharedSetRef Ref = classifyOutlinedSharedSetRef(CI->getArgOperand(0));
                if (Ref.kind == OutlinedSharedSetRef::Kind::Unsupported)
                {
                    logLoopState(F, Target.Header, Target.Depth, "skip:shared-set-unsupported-storage");
                    return Result;
                }

                if (Ref.kind == OutlinedSharedSetRef::Kind::Parameter)
                {
                    unsigned ParamIndex = Ref.param_index;
                    if (ParamIndex >= ParamReadsSharedSet.size())
                        continue;

                    if (CalleeName == "roaring_bitmap_add")
                    {
                        ParamWritesSharedSet[ParamIndex] = true;
                        /* Debug logging disabled: set-write */
                    }
                    else
                    {
                        ParamReadsSharedSet[ParamIndex] = true;
                        /* Debug logging disabled: set-read */
                    }
                    continue;
                }

                if (CalleeName == "roaring_bitmap_add")
                {
                    if (!GlobalWriteSetIndices.count(Ref.global))
                    {
                        GlobalWriteSetIndices[Ref.global] = GlobalWriteSharedSets.size();
                        GlobalWriteSharedSets.push_back(Ref.global);
                        GlobalWriteSharedSetValueTys.push_back(Ref.value_ty);
                    }
                }
                else
                {
                    bool Seen = false;
                    for (GlobalVariable *GV : GlobalReadSharedSets)
                    {
                        if (GV == Ref.global)
                        {
                            Seen = true;
                            break;
                        }
                    }
                    if (!Seen)
                        GlobalReadSharedSets.push_back(Ref.global);
                }
            }
        }

        for (unsigned ParamIndex = 0; ParamIndex < ArgOriginVals.size(); ++ParamIndex)
        {
            Value *Orig = ArgOriginVals[ParamIndex];
            Type *ParamTy = Outlined->getFunctionType()->getParamType(ParamIndex);
            if (Orig == Target.IndVar)
            {
                InductionParamIndex = (int)ParamIndex;
                continue;
            }

            if (auto *AI = dyn_cast_or_null<AllocaInst>(Orig))
            {
                bool OnlyUsedInsideLoop = AI->getFunction() == &F;
                for (User *U : AI->users())
                {
                    auto *UseInst = dyn_cast<Instruction>(U);
                    if (!UseInst || !LoopBlockSet.contains(UseInst->getParent()))
                    {
                        OnlyUsedInsideLoop = false;
                        break;
                    }
                }
                if (OnlyUsedInsideLoop)
                {
                    ParamUsesPrivateScratch[ParamIndex] = true;
                    ParamPrivateScratchTys[ParamIndex] = AI->getAllocatedType();
                    /* Debug logging disabled: synthetic alloca privatization */
                    continue;
                }
            }

            ParamToEnvFieldIndex[ParamIndex] = (int)NewEnvFieldTys.size();
            NewEnvFieldTys.push_back(ParamTy);
            NewEnvOriginVals.push_back(Orig);
        }

        int AppendFrontierEnvField = -1;
        int AppendSizeEnvField = -1;
        int AppendCapEnvField = -1;
        GlobalVariable *AppendFrontierGV = nullptr;
        GlobalVariable *AppendSizeGV = nullptr;
        GlobalVariable *AppendCapGV = nullptr;

        if (IsDoAll)
        {
            if (auto AppendPattern = detectIntAppendPattern(Target))
            {
                Value *ArrayRoot = AppendPattern->first;
                Value *SizeRoot = AppendPattern->second;
                AppendFrontierGV = dyn_cast<GlobalVariable>(ArrayRoot);
                AppendSizeGV = dyn_cast<GlobalVariable>(SizeRoot);
                AppendCapGV = findVertexCountGlobal(F);

                if (AppendFrontierGV && AppendSizeGV && AppendCapGV)
                {
                    AppendFrontierEnvField = (int)NewEnvFieldTys.size();
                    NewEnvFieldTys.push_back(Type::getInt32Ty(Ctx)->getPointerTo());
                    NewEnvOriginVals.push_back(nullptr);

                    AppendSizeEnvField = (int)NewEnvFieldTys.size();
                    NewEnvFieldTys.push_back(Type::getInt32Ty(Ctx)->getPointerTo());
                    NewEnvOriginVals.push_back(AppendSizeGV);

                    AppendCapEnvField = (int)NewEnvFieldTys.size();
                    NewEnvFieldTys.push_back(Type::getInt32Ty(Ctx));
                    NewEnvOriginVals.push_back(AppendCapGV);
                }
            }
        }

        bool UnsupportedSharedSet = false;
        for (unsigned ParamIndex = 0; ParamIndex < ArgOriginVals.size(); ++ParamIndex)
        {
            if (!ParamWritesSharedSet[ParamIndex])
                continue;

            if (!IsDoAll)
            {
                UnsupportedSharedSet = true;
                logLoopState(F, Target.Header, Target.Depth, "skip:shared-set-privatization-requires-doall");
                break;
            }
            if (ParamReadsSharedSet[ParamIndex])
            {
                UnsupportedSharedSet = true;
                logLoopState(F, Target.Header, Target.Depth, "skip:shared-set-read-write-same-object");
                break;
            }
            if (ParamToEnvFieldIndex[ParamIndex] < 0)
            {
                UnsupportedSharedSet = true;
                logLoopState(F, Target.Header, Target.Depth, "skip:shared-set-write-not-captured-in-env");
                break;
            }

            ParamUsesPrivatizedSet[ParamIndex] = true;
            /* Debug logging disabled: privatize-set-output param */
        }

        if (!UnsupportedSharedSet)
        {
            for (GlobalVariable *GV : GlobalWriteSharedSets)
            {
                if (!IsDoAll)
                {
                    UnsupportedSharedSet = true;
                    logLoopState(F, Target.Header, Target.Depth, "skip:shared-set-privatization-requires-doall");
                    break;
                }

                bool ReadSameGlobal = false;
                for (GlobalVariable *ReadGV : GlobalReadSharedSets)
                {
                    if (ReadGV == GV)
                    {
                        ReadSameGlobal = true;
                        break;
                    }
                }
                if (ReadSameGlobal)
                {
                    UnsupportedSharedSet = true;
                    logLoopState(F, Target.Header, Target.Depth, "skip:shared-set-read-write-same-object");
                    break;
                }
            }
        }

        if (UnsupportedSharedSet)
            return Result;

        IRBuilder<> B(Target.Preheader->getTerminator());
        Type *VoidTy = Type::getVoidTy(Ctx);
        Type *Int32Ty = Type::getInt32Ty(Ctx);
        Type *Int64Ty = Type::getInt64Ty(Ctx);
        Type *Int8PtrTy = Type::getInt8Ty(Ctx)->getPointerTo();
        Type *LoopBodyFnTy = FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false)->getPointerTo();

        for (unsigned GlobalIndex = 0; GlobalIndex < GlobalWriteSharedSets.size(); ++GlobalIndex)
        {
            GlobalVariable *GV = GlobalWriteSharedSets[GlobalIndex];
            Type *FieldTy = GlobalWriteSharedSetValueTys[GlobalIndex];
            unsigned EnvFieldIndex = NewEnvFieldTys.size();
            GlobalWriteSetEnvFieldIndices[GV] = EnvFieldIndex;
            NewEnvFieldTys.push_back(FieldTy);
            NewEnvOriginVals.push_back(B.CreateLoad(FieldTy, GV, GV->getName() + ".captured"));
        }

        StructType *NewEnvStructTy = StructType::create(Ctx, NewEnvFieldTys, "env.struct");
        uint64_t NewEnvSize = M->getDataLayout().getTypeAllocSize(NewEnvStructTy);
        Result.EnvSize = NewEnvSize;

        FunctionCallee MallocFn = M->getOrInsertFunction(
            "malloc",
            FunctionType::get(Int8PtrTy, {Int64Ty}, false));
        Value *RawPtr = B.CreateCall(MallocFn, {ConstantInt::get(Int64Ty, NewEnvSize)}, "env_raw");
        Value *NewEnvPtr = B.CreateBitCast(RawPtr, NewEnvStructTy->getPointerTo(), "envptr_struct");

        for (unsigned FieldIndex = 0; FieldIndex < NewEnvOriginVals.size(); ++FieldIndex)
        {
            Value *Orig = NewEnvOriginVals[FieldIndex];
            Value *GEP = B.CreateStructGEP(NewEnvStructTy, NewEnvPtr, FieldIndex, "env_gep");
            Type *FieldTy = NewEnvFieldTys[FieldIndex];
            Value *StoreVal = nullptr;

            if ((int)FieldIndex == AppendFrontierEnvField && AppendFrontierGV)
            {
                StoreVal = B.CreateLoad(FieldTy, AppendFrontierGV, AppendFrontierGV->getName() + ".captured");
            }
            else if ((int)FieldIndex == AppendSizeEnvField && AppendSizeGV)
            {
                StoreVal = AppendSizeGV;
                if (StoreVal->getType() != FieldTy)
                    StoreVal = B.CreateBitCast(StoreVal, FieldTy);
            }
            else if ((int)FieldIndex == AppendCapEnvField && AppendCapGV)
            {
                StoreVal = B.CreateLoad(FieldTy, AppendCapGV, AppendCapGV->getName() + ".cap");
            }
            else if (!Orig)
            {
                StoreVal = Constant::getNullValue(FieldTy);
            }
            else if (Orig->getType() == FieldTy)
            {
                StoreVal = Orig;
            }
            else if (Orig->getType()->isPointerTy() && FieldTy->isPointerTy())
            {
                StoreVal = B.CreateBitCast(Orig, FieldTy);
            }
            else if (Orig->getType()->isIntegerTy() && FieldTy->isIntegerTy())
            {
                StoreVal = B.CreateIntCast(Orig, FieldTy, true);
            }
            else
            {
                StoreVal = Constant::getNullValue(FieldTy);
            }

            B.CreateStore(StoreVal, GEP);
        }

        SmallVector<EnvPrivTarget, 4> PrivTargets;
        const StructLayout *EnvLayout = M->getDataLayout().getStructLayout(NewEnvStructTy);
        for (unsigned ParamIndex = 0; ParamIndex < ParamUsesPrivatizedSet.size(); ++ParamIndex)
        {
            if (!ParamUsesPrivatizedSet[ParamIndex])
                continue;
            unsigned EnvFieldIndex = (unsigned)ParamToEnvFieldIndex[ParamIndex];
            EnvPrivTarget TargetDesc;
            TargetDesc.Offset = EnvLayout->getElementOffset(EnvFieldIndex);
            TargetDesc.Kind = SGPL_PRIV_ROARING;
            PrivTargets.push_back(TargetDesc);
        }
        for (GlobalVariable *GV : GlobalWriteSharedSets)
        {
            unsigned EnvFieldIndex = GlobalWriteSetEnvFieldIndices.lookup(GV);
            EnvPrivTarget TargetDesc;
            TargetDesc.Offset = EnvLayout->getElementOffset(EnvFieldIndex);
            TargetDesc.Kind = SGPL_PRIV_ROARING;
            PrivTargets.push_back(TargetDesc);
        }

        if (AppendFrontierEnvField >= 0 && AppendSizeEnvField >= 0 && AppendCapEnvField >= 0)
        {
            EnvPrivTarget TargetDesc;
            TargetDesc.Offset = EnvLayout->getElementOffset((unsigned)AppendFrontierEnvField);
            TargetDesc.Kind = SGPL_PRIV_INT_APPEND;
            TargetDesc.AuxSizeOffset = (int64_t)EnvLayout->getElementOffset((unsigned)AppendSizeEnvField);
            TargetDesc.AuxCapOffset = (int64_t)EnvLayout->getElementOffset((unsigned)AppendCapEnvField);
            PrivTargets.push_back(TargetDesc);
        }
        /* Debug logging disabled: privatize-set-output count */

        FunctionType *WrapperFT = FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false);
        std::string WrapperName = "wrapper_" + F.getName().str() + "_" + Target.Header->getName().str();
        Function *WrapperFn = M->getFunction(WrapperName);
        if (!WrapperFn)
            WrapperFn = Function::Create(WrapperFT, GlobalValue::ExternalLinkage, WrapperName, M);

        if (WrapperFn->empty())
        {
            auto ArgIt = WrapperFn->arg_begin();
            Argument *IdxArg = &*ArgIt++;
            Argument *EnvArg = &*ArgIt++;
            IdxArg->setName("idx");
            EnvArg->setName("env");

            BasicBlock *Entry = BasicBlock::Create(Ctx, "entry", WrapperFn);
            IRBuilder<> WB(Entry);
            Value *EnvStructPtr = WB.CreateBitCast(EnvArg, NewEnvStructTy->getPointerTo(), "envstruct");

            SmallVector<Value *, 8> LoadedFields;
            LoadedFields.reserve(NewEnvFieldTys.size());
            for (unsigned FieldIndex = 0; FieldIndex < NewEnvFieldTys.size(); ++FieldIndex)
            {
                Value *FieldGEP = WB.CreateStructGEP(NewEnvStructTy, EnvStructPtr, FieldIndex, "fgep");
                LoadedFields.push_back(WB.CreateLoad(NewEnvFieldTys[FieldIndex], FieldGEP, "fload"));
            }

            if (AppendFrontierGV && AppendSizeGV && AppendFrontierEnvField >= 0 && AppendSizeEnvField >= 0)
            {
                SmallVector<Type *, 8> NewParamTys;
                for (Argument &A : Outlined->args())
                    NewParamTys.push_back(A.getType());
                NewParamTys.push_back(Type::getInt32Ty(Ctx)->getPointerTo());
                NewParamTys.push_back(Type::getInt32Ty(Ctx)->getPointerTo());

                FunctionType *NewFT = FunctionType::get(Outlined->getReturnType(), NewParamTys, false);
                Function *NewOutlined = Function::Create(
                    NewFT, Outlined->getLinkage(), Outlined->getName() + ".append_priv", M);
                NewOutlined->copyAttributesFrom(Outlined);

                ValueToValueMapTy VMap;
                auto NewArgIt = NewOutlined->arg_begin();
                for (Argument &OldArg : Outlined->args())
                {
                    NewArgIt->setName(OldArg.getName());
                    VMap[&OldArg] = &*NewArgIt++;
                }
                Argument *FrontierArg = &*NewArgIt++;
                Argument *SizeArg = &*NewArgIt++;
                FrontierArg->setName("append_frontier");
                SizeArg->setName("append_size");

                SmallVector<ReturnInst *, 8> Returns;
                CloneFunctionInto(NewOutlined, Outlined, VMap, CloneFunctionChangeType::LocalChangesOnly, Returns);
                remapAppendGlobalsInOutlinedFunction(NewOutlined, AppendFrontierGV, AppendSizeGV, FrontierArg, SizeArg);

                Outlined->eraseFromParent();
                Outlined = NewOutlined;
                Result.Outlined = Outlined;

                SmallVector<Value *, 8> CallArgs;
                CallArgs.reserve(ArgOriginVals.size() + 2);
                unsigned EnvCursor = 0;
                for (unsigned ParamIndex = 0; ParamIndex < ArgOriginVals.size(); ++ParamIndex)
                {
                    Type *ParamTy = Outlined->getFunctionType()->getParamType(ParamIndex);
                    if ((int)ParamIndex == InductionParamIndex)
                    {
                        CallArgs.push_back(WB.CreateIntCast(IdxArg, ParamTy, true, "idxcast"));
                        continue;
                    }

                    if (ParamUsesPrivateScratch[ParamIndex])
                    {
                        Value *Scratch = WB.CreateAlloca(ParamPrivateScratchTys[ParamIndex], nullptr, "outlined.scratch");
                        if (Scratch->getType() != ParamTy)
                        {
                            if (Scratch->getType()->isPointerTy() && ParamTy->isPointerTy())
                                Scratch = WB.CreateBitCast(Scratch, ParamTy);
                            else
                                Scratch = Constant::getNullValue(ParamTy);
                        }
                        CallArgs.push_back(Scratch);
                        continue;
                    }

                    Value *FieldValue = LoadedFields[EnvCursor++];
                    if (FieldValue->getType() != ParamTy)
                    {
                        if (FieldValue->getType()->isPointerTy() && ParamTy->isPointerTy())
                            FieldValue = WB.CreateBitCast(FieldValue, ParamTy);
                        else if (FieldValue->getType()->isIntegerTy() && ParamTy->isIntegerTy())
                            FieldValue = WB.CreateIntCast(FieldValue, ParamTy, true);
                        else
                            FieldValue = Constant::getNullValue(ParamTy);
                    }
                    CallArgs.push_back(FieldValue);
                }

                CallArgs.push_back(LoadedFields[(unsigned)AppendFrontierEnvField]);
                CallArgs.push_back(LoadedFields[(unsigned)AppendSizeEnvField]);
                WB.CreateCall(Outlined, CallArgs);
                WB.CreateRetVoid();
            }
            else
            {
                SmallVector<Value *, 8> CallArgs;
                CallArgs.reserve(ArgOriginVals.size());
                unsigned EnvCursor = 0;
                for (unsigned ParamIndex = 0; ParamIndex < ArgOriginVals.size(); ++ParamIndex)
                {
                    Type *ParamTy = Outlined->getFunctionType()->getParamType(ParamIndex);
                    if ((int)ParamIndex == InductionParamIndex)
                    {
                        CallArgs.push_back(WB.CreateIntCast(IdxArg, ParamTy, true, "idxcast"));
                        continue;
                    }

                    if (ParamUsesPrivateScratch[ParamIndex])
                    {
                        Value *Scratch = WB.CreateAlloca(ParamPrivateScratchTys[ParamIndex], nullptr, "outlined.scratch");
                        if (Scratch->getType() != ParamTy)
                        {
                            if (Scratch->getType()->isPointerTy() && ParamTy->isPointerTy())
                                Scratch = WB.CreateBitCast(Scratch, ParamTy);
                            else
                                Scratch = Constant::getNullValue(ParamTy);
                        }
                        CallArgs.push_back(Scratch);
                        continue;
                    }

                    Value *FieldValue = LoadedFields[EnvCursor++];
                    if (FieldValue->getType() != ParamTy)
                    {
                        if (FieldValue->getType()->isPointerTy() && ParamTy->isPointerTy())
                            FieldValue = WB.CreateBitCast(FieldValue, ParamTy);
                        else if (FieldValue->getType()->isIntegerTy() && ParamTy->isIntegerTy())
                            FieldValue = WB.CreateIntCast(FieldValue, ParamTy, true);
                        else
                            FieldValue = Constant::getNullValue(ParamTy);
                    }
                    CallArgs.push_back(FieldValue);
                }

                WB.CreateCall(Outlined, CallArgs);
                WB.CreateRetVoid();
            }
        }
        Result.WrapperFn = WrapperFn;

        Value *CastedWrapper = B.CreateBitCast(WrapperFn, LoopBodyFnTy);
        FunctionCallee ParallelForFunc = M->getOrInsertFunction(
            "parallel_for_runtime",
            FunctionType::get(VoidTy, {Int64Ty, Int64Ty, Int64Ty, LoopBodyFnTy, Int8PtrTy, Int32Ty, Int32Ty}, false));
        FunctionCallee ParallelForExFunc = M->getOrInsertFunction(
            "parallel_for_runtime_ex",
            FunctionType::get(VoidTy,
                              {Int64Ty,
                               Int64Ty,
                               Int64Ty,
                               LoopBodyFnTy,
                               Int8PtrTy,
                               Int64Ty,
                               Int64Ty->getPointerTo(),
                               Int32Ty->getPointerTo(),
                               Int64Ty->getPointerTo(),
                               Int32Ty,
                               Int32Ty,
                               Int32Ty},
                              false));

        Value *StartArg = castIntegerToI64(B, StartV, "start64");
        Value *EndArg = castIntegerToI64(B, EndV, "end64");
        Value *StepArg = castIntegerToI64(B, StepV, "step64");
        Value *NeedsDoAcrossArg = ConstantInt::get(Int32Ty, IsDoAcross ? 1 : 0);
        Value *DoAcrossNumSyncIdsArg = ConstantInt::get(Int32Ty, Result.NumDoAcrossSyncIds);

        bool EmittedGpu = false;
        SmallPtrSet<GlobalVariable *, 8> GpuGlobals;
        SmallPtrSet<Function *, 8> GpuVisited;
        // DOACROSS loops offload with the cooperative wave kernel when a
        // constant dependence distance is known (the wave width). Branched
        // bodies are fine: the barrier stays outside the per-iteration body.
        bool GpuDoAcrossOk = IsDoAcross ? GpuDoAcrossMinDist > 0 : true;
        bool GpuEligible = isGpuBackendEnabled(*M) && (IsDoAll || IsDoAcross) &&
                           PrivTargets.empty() && GpuDoAcrossOk;
        if (GpuEligible &&
            gpuFunctionIsDeviceSafe(Outlined, &GpuGlobals, GpuVisited) &&
            gpuFunctionIsDeviceSafe(WrapperFn, &GpuGlobals, GpuVisited))
        {
            const StructLayout *GpuEnvLayout = M->getDataLayout().getStructLayout(NewEnvStructTy);
            SmallVector<int64_t, 8> GpuPtrOffsets;
            SmallVector<int64_t, 8> GpuPtrSizes;
            SmallVector<int64_t, 8> GpuPtrCapOffsets;
            SmallVector<int64_t, 8> GpuPtrElemSizes;
            bool GpuPtrLayoutOk = true;

            for (unsigned FieldIndex = 0; FieldIndex < NewEnvFieldTys.size(); ++FieldIndex)
            {
                Type *FieldTy = NewEnvFieldTys[FieldIndex];
                if (!FieldTy->isPointerTy())
                    continue;

                int64_t Offset = (int64_t)GpuEnvLayout->getElementOffset(FieldIndex);
                int64_t Size = 0;
                int64_t CapOffset = -1;
                int64_t ElemSize = 0;

                if ((int)FieldIndex == AppendFrontierEnvField && AppendCapEnvField >= 0)
                {
                    CapOffset = (int64_t)GpuEnvLayout->getElementOffset((unsigned)AppendCapEnvField);
                    ElemSize = 4;
                }
                else
                {
                    Value *Orig = stripToNamedPointer(NewEnvOriginVals[FieldIndex]);
                    if (auto *AI = dyn_cast_or_null<AllocaInst>(Orig))
                    {
                        // A variable-length array (e.g. `alloca i32, i64 %n`) has a
                        // runtime element count that we cannot size at compile time.
                        // Fall back to CPU rather than copy the wrong number of bytes.
                        if (AI->isArrayAllocation())
                        {
                            GpuPtrLayoutOk = false;
                            break;
                        }
                        Type *AllocTy = AI->getAllocatedType();
                        if (AllocTy->isSized())
                            Size = (int64_t)M->getDataLayout().getTypeAllocSize(AllocTy);
                        else
                        {
                            GpuPtrLayoutOk = false;
                            break;
                        }
                    }
                    else if (auto *GV = dyn_cast_or_null<GlobalVariable>(Orig))
                    {
                        Type *ValueTy = GV->getValueType();
                        if (ValueTy->isSized())
                            Size = (int64_t)M->getDataLayout().getTypeAllocSize(ValueTy);
                    }
                    else
                    {
                        GpuPtrLayoutOk = false;
                        break;
                    }
                }

                if (Size <= 0 && CapOffset < 0)
                {
                    GpuPtrLayoutOk = false;
                    break;
                }

                GpuPtrOffsets.push_back(Offset);
                GpuPtrSizes.push_back(Size);
                GpuPtrCapOffsets.push_back(CapOffset);
                GpuPtrElemSizes.push_back(ElemSize);
            }

            if (GpuPtrLayoutOk)
            {
                stripDeviceIncompatibleIntrinsics(*Outlined);
                stripDeviceIncompatibleIntrinsics(*WrapperFn);

                // The kernel name becomes the PTX .entry symbol; ptxas rejects
                // names containing '.' (e.g. a loop header "foreach.cond").
                // Sanitize it to [A-Za-z0-9_] for the device module.
                std::string KernelBase =
                    "gpu_kernel_" + F.getName().str() + "_" + Target.Header->getName().str();
                std::string KernelName;
                KernelName.reserve(KernelBase.size());
                for (char C : KernelBase)
                    KernelName += (isalnum((unsigned char)C) || C == '_') ? C : '_';
                FunctionType *KernelFT = FunctionType::get(VoidTy, {Int64Ty, Int64Ty, Int64Ty, Int8PtrTy}, false);
                Function *KernelFn = Function::Create(KernelFT, GlobalValue::ExternalLinkage, KernelName, M);
                KernelFn->setCallingConv(CallingConv::PTX_Kernel);

                auto KernelArgIt = KernelFn->arg_begin();
                Argument *KStart = &*KernelArgIt++;
                Argument *KEnd = &*KernelArgIt++;
                Argument *KStep = &*KernelArgIt++;
                Argument *KEnv = &*KernelArgIt++;
                KStart->setName("start");
                KEnd->setName("end");
                KStep->setName("step");
                KEnv->setName("env");

                BasicBlock *KEntry = BasicBlock::Create(Ctx, "entry", KernelFn);
                IRBuilder<> KB(KEntry);

                Function *GetBX = Intrinsic::getDeclaration(M, Intrinsic::nvvm_read_ptx_sreg_ctaid_x);
                Function *GetTX = Intrinsic::getDeclaration(M, Intrinsic::nvvm_read_ptx_sreg_tid_x);
                Function *GetBD = Intrinsic::getDeclaration(M, Intrinsic::nvvm_read_ptx_sreg_ntid_x);

                Value *BX = KB.CreateCall(GetBX, {}, "bx");
                Value *TX = KB.CreateCall(GetTX, {}, "tx");
                Value *BD = KB.CreateCall(GetBD, {}, "bd");

                Value *BX64 = KB.CreateZExt(BX, Int64Ty, "bx64");
                Value *TX64 = KB.CreateZExt(TX, Int64Ty, "tx64");
                Value *BD64 = KB.CreateZExt(BD, Int64Ty, "bd64");

                Value *LinIdx = KB.CreateAdd(KB.CreateMul(BX64, BD64, "blockoff"), TX64, "lin");

                if (IsDoAcross)
                {
                    // Cooperative wave/phase kernel. Wave w owns the iterations
                    // [start + w*d, start + (w+1)*d) where d is the minimum
                    // constant dependence distance; every producer of those
                    // iterations lives in a strictly earlier wave. The full
                    // original body (conditionals and SIMT divergence included)
                    // runs per assigned iteration, then an unconditional
                    // cooperative grid sync separates the waves.
                    //
                    // Grid sync is emitted as inline PTX (the NVVM
                    // griddepcontrol intrinsics are not selectable by NVPTX).
                    // On sm_70..sm_89 the cooperative-groups grid sync is
                    // simply `bar.sync 0` under a cooperative launch (all
                    // blocks resident); griddepcontrol is sm_90+ only.
                    Function *GetGD = Intrinsic::getDeclaration(M, Intrinsic::nvvm_read_ptx_sreg_nctaid_x);
                    InlineAsm *GridSync = InlineAsm::get(
                        FunctionType::get(VoidTy, {}, false),
                        "bar.sync 0;",
                        "~{memory}",
                        /*hasSideEffects=*/true);

                    Value *GD = KB.CreateCall(GetGD, {}, "gd");
                    Value *GD64 = KB.CreateZExt(GD, Int64Ty, "gd64");
                    Value *NThreads = KB.CreateMul(GD64, BD64, "nthreads");

                    Value *Trip = KB.CreateSDiv(KB.CreateSub(KEnd, KStart), KStep, "trip");
                    Value *D64 = ConstantInt::get(Int64Ty, GpuDoAcrossMinDist);
                    Value *NWaves = KB.CreateSDiv(KB.CreateAdd(Trip, D64), D64, "nwaves");

                    BasicBlock *KWloop = BasicBlock::Create(Ctx, "wloop", KernelFn);
                    BasicBlock *KWbody = BasicBlock::Create(Ctx, "wbody", KernelFn);
                    BasicBlock *KWcheck = BasicBlock::Create(Ctx, "wcheck", KernelFn);
                    BasicBlock *KWiter = BasicBlock::Create(Ctx, "witer", KernelFn);
                    BasicBlock *KWsync = BasicBlock::Create(Ctx, "wsync", KernelFn);
                    BasicBlock *KWdone = BasicBlock::Create(Ctx, "wdone", KernelFn);
                    KB.CreateBr(KWloop);

                    IRBuilder<> WB(KWloop);
                    PHINode *WavePhi = WB.CreatePHI(Int64Ty, 2, "wave");
                    WavePhi->addIncoming(ConstantInt::get(Int64Ty, 0), KEntry);
                    Value *WCond = WB.CreateICmpSLT(WavePhi, NWaves, "wave.in.range");
                    WB.CreateCondBr(WCond, KWbody, KWdone);

                    IRBuilder<> WBB(KWbody);
                    Value *WaveOff = WBB.CreateMul(WavePhi, D64, "wave.off");
                    Value *IBase =
                        WBB.CreateAdd(KStart, WBB.CreateMul(WaveOff, KStep, "wave.off.step"), "ibase");
                    Value *IEndRaw = WBB.CreateAdd(IBase, WBB.CreateMul(D64, KStep, "wave.width"), "iend.raw");
                    Value *IEndClamp = WBB.CreateICmpSLT(IEndRaw, KEnd, "iend.clamp");
                    Value *IEnd = WBB.CreateSelect(IEndClamp, IEndRaw, KEnd, "iend");
                    Value *IInit = WBB.CreateAdd(IBase, WBB.CreateMul(LinIdx, KStep, "i.init.step"), "iinit");
                    WBB.CreateBr(KWcheck);

                    IRBuilder<> WCB(KWcheck);
                    PHINode *IPhi = WCB.CreatePHI(Int64Ty, 2, "i");
                    IPhi->addIncoming(IInit, KWbody);
                    Value *ICond = WCB.CreateICmpSLT(IPhi, IEnd, "in.wave");
                    WCB.CreateCondBr(ICond, KWiter, KWsync);

                    IRBuilder<> WIB(KWiter);
                    WIB.CreateCall(WrapperFn, {IPhi, KEnv});
                    Value *INext = WIB.CreateAdd(IPhi, WIB.CreateMul(NThreads, KStep, "i.stride"), "inext");
                    WIB.CreateBr(KWcheck);

                    IRBuilder<> WSB(KWsync);
                    WSB.CreateCall(GridSync);
                    Value *WaveNext = WSB.CreateAdd(WavePhi, ConstantInt::get(Int64Ty, 1), "wave.next");
                    WSB.CreateBr(KWloop);

                    IRBuilder<> WDB(KWdone);
                    WDB.CreateRetVoid();

                    WavePhi->addIncoming(WaveNext, KWsync);
                    IPhi->addIncoming(INext, KWiter);
                }
                else
                {
                    Value *Iter = KB.CreateAdd(KB.CreateMul(LinIdx, KStep, "strided"), KStart, "i");
                    Value *InRange = KB.CreateICmpSLT(Iter, KEnd, "in.range");

                    BasicBlock *KBody = BasicBlock::Create(Ctx, "body", KernelFn);
                    BasicBlock *KDone = BasicBlock::Create(Ctx, "done", KernelFn);
                    KB.CreateCondBr(InRange, KBody, KDone);

                    KB.SetInsertPoint(KBody);
                    KB.CreateCall(WrapperFn, {Iter, KEnv});
                    KB.CreateBr(KDone);

                    KB.SetInsertPoint(KDone);
                    KB.CreateRetVoid();
                }

                NamedMDNode *GpuKernels = M->getOrInsertNamedMetadata("graph.gpu.kernels");
                ValueAsMetadata *KernelVAM = ValueAsMetadata::get(KernelFn);
                GpuKernels->addOperand(MDNode::get(Ctx, KernelVAM));

                unsigned NumGpuPtrFields = (unsigned)GpuPtrOffsets.size();
                PointerType *I64PtrTy = cast<PointerType>(Int64Ty->getPointerTo());
                Value *GpuPtrOffsetsArg = ConstantPointerNull::get(I64PtrTy);
                Value *GpuPtrSizesArg = ConstantPointerNull::get(I64PtrTy);
                Value *GpuPtrCapOffsetsArg = ConstantPointerNull::get(I64PtrTy);
                Value *GpuPtrElemSizesArg = ConstantPointerNull::get(I64PtrTy);

                if (NumGpuPtrFields > 0)
                {
                    ArrayType *OffsetsArrTy = ArrayType::get(Int64Ty, NumGpuPtrFields);
                    SmallVector<Constant *, 8> OffsetsC;
                    SmallVector<Constant *, 8> SizesC;
                    SmallVector<Constant *, 8> CapC;
                    SmallVector<Constant *, 8> ElemC;
                    for (unsigned P = 0; P < NumGpuPtrFields; ++P)
                    {
                        OffsetsC.push_back(ConstantInt::get(Int64Ty, GpuPtrOffsets[P]));
                        SizesC.push_back(ConstantInt::get(Int64Ty, GpuPtrSizes[P]));
                        CapC.push_back(ConstantInt::get(Int64Ty, GpuPtrCapOffsets[P]));
                        ElemC.push_back(ConstantInt::get(Int64Ty, GpuPtrElemSizes[P]));
                    }

                    auto *OffsetsGV = new GlobalVariable(*M, OffsetsArrTy, true, GlobalValue::PrivateLinkage,
                                                         ConstantArray::get(OffsetsArrTy, OffsetsC), "gpu_ptr_offsets");
                    auto *SizesGV = new GlobalVariable(*M, OffsetsArrTy, true, GlobalValue::PrivateLinkage,
                                                       ConstantArray::get(OffsetsArrTy, SizesC), "gpu_ptr_sizes");
                    auto *CapGV = new GlobalVariable(*M, OffsetsArrTy, true, GlobalValue::PrivateLinkage,
                                                     ConstantArray::get(OffsetsArrTy, CapC), "gpu_ptr_cap_offsets");
                    auto *ElemGV = new GlobalVariable(*M, OffsetsArrTy, true, GlobalValue::PrivateLinkage,
                                                      ConstantArray::get(OffsetsArrTy, ElemC), "gpu_ptr_elem_sizes");

                    Value *Zero32 = ConstantInt::get(Int32Ty, 0);
                    GpuPtrOffsetsArg = B.CreateInBoundsGEP(OffsetsArrTy, OffsetsGV, {Zero32, Zero32}, "gpu_offsets_ptr");
                    GpuPtrSizesArg = B.CreateInBoundsGEP(OffsetsArrTy, SizesGV, {Zero32, Zero32}, "gpu_sizes_ptr");
                    GpuPtrCapOffsetsArg = B.CreateInBoundsGEP(OffsetsArrTy, CapGV, {Zero32, Zero32}, "gpu_cap_ptr");
                    GpuPtrElemSizesArg = B.CreateInBoundsGEP(OffsetsArrTy, ElemGV, {Zero32, Zero32}, "gpu_elem_ptr");
                }

                // Global arrays referenced by the device functions are cloned into
                // the device module; the runtime copies their data in/out by name.
                SmallVector<GlobalVariable *, 8> GpuGlobalList(GpuGlobals.begin(), GpuGlobals.end());
                unsigned NumGpuGlobals = (unsigned)GpuGlobalList.size();
                PointerType *Int8PtrPtrTy = cast<PointerType>(Int8PtrTy->getPointerTo());
                Value *GpuGlobalNamesArg = ConstantPointerNull::get(Int8PtrPtrTy);
                Value *GpuGlobalPtrsArg = ConstantPointerNull::get(Int8PtrPtrTy);
                Value *GpuGlobalSizesArg = ConstantPointerNull::get(I64PtrTy);

                if (NumGpuGlobals > 0)
                {
                    ArrayType *NamesArrTy = ArrayType::get(Int8PtrTy, NumGpuGlobals);
                    ArrayType *PtrsArrTy = ArrayType::get(Int8PtrTy, NumGpuGlobals);
                    ArrayType *SizesArrTy = ArrayType::get(Int64Ty, NumGpuGlobals);
                    SmallVector<Constant *, 8> NamesC;
                    SmallVector<Constant *, 8> PtrsC;
                    SmallVector<Constant *, 8> SizesC;
                    for (GlobalVariable *GV : GpuGlobalList)
                    {
                        NamesC.push_back(createCStringPtr(
                            *M, GV->getName(), std::string("gpu.global.name.") + GV->getName().str()));
                        PtrsC.push_back(ConstantExpr::getPointerBitCastOrAddrSpaceCast(GV, Int8PtrTy));
                        SizesC.push_back(ConstantInt::get(
                            Int64Ty, (uint64_t)M->getDataLayout().getTypeAllocSize(GV->getValueType())));
                    }
                    auto *NamesGV = new GlobalVariable(*M, NamesArrTy, true, GlobalValue::PrivateLinkage,
                                                       ConstantArray::get(NamesArrTy, NamesC), "gpu_global_names");
                    auto *PtrsGV = new GlobalVariable(*M, PtrsArrTy, true, GlobalValue::PrivateLinkage,
                                                      ConstantArray::get(PtrsArrTy, PtrsC), "gpu_global_ptrs");
                    auto *SizesGV = new GlobalVariable(*M, SizesArrTy, true, GlobalValue::PrivateLinkage,
                                                       ConstantArray::get(SizesArrTy, SizesC), "gpu_global_sizes");

                    Value *Zero32 = ConstantInt::get(Int32Ty, 0);
                    GpuGlobalNamesArg = B.CreateInBoundsGEP(NamesArrTy, NamesGV, {Zero32, Zero32}, "gpu_global_names_ptr");
                    GpuGlobalPtrsArg = B.CreateInBoundsGEP(PtrsArrTy, PtrsGV, {Zero32, Zero32}, "gpu_global_ptrs_ptr");
                    GpuGlobalSizesArg = B.CreateInBoundsGEP(SizesArrTy, SizesGV, {Zero32, Zero32}, "gpu_global_sizes_ptr");
                }

                FunctionCallee GpuForFn = M->getOrInsertFunction(
                    "gpu_parallel_for_runtime",
                    FunctionType::get(VoidTy,
                                      {Int64Ty,
                                       Int64Ty,
                                       Int64Ty,
                                       Int8PtrTy,
                                       LoopBodyFnTy,
                                       Int8PtrTy,
                                       Int64Ty,
                                       I64PtrTy,
                                       I64PtrTy,
                                       I64PtrTy,
                                       I64PtrTy,
                                       Int32Ty,
                                       Int8PtrPtrTy,
                                       Int8PtrPtrTy,
                                       I64PtrTy,
                                       Int32Ty,
                                       Int32Ty,
                                       Int32Ty},
                                      false));

                Value *KernelNameStr =
                    createCStringPtr(*M, KernelName, std::string("gpu.kernel.name.") + KernelName);
                B.CreateCall(GpuForFn,
                             {StartArg,
                              EndArg,
                              StepArg,
                              KernelNameStr,
                              CastedWrapper,
                              RawPtr,
                              ConstantInt::get(Int64Ty, NewEnvSize),
                              GpuPtrOffsetsArg,
                              GpuPtrSizesArg,
                              GpuPtrCapOffsetsArg,
                              GpuPtrElemSizesArg,
                              ConstantInt::get(Int32Ty, NumGpuPtrFields),
                              GpuGlobalNamesArg,
                              GpuGlobalPtrsArg,
                              GpuGlobalSizesArg,
                              ConstantInt::get(Int32Ty, NumGpuGlobals),
                              ConstantInt::get(Int32Ty, IsDoAcross ? 1 : 0),
                              DoAcrossNumSyncIdsArg});
                EmittedGpu = true;
            }
        }

        if (!EmittedGpu && PrivTargets.empty())
        {
            B.CreateCall(ParallelForFunc,
                         {StartArg, EndArg, StepArg, CastedWrapper, RawPtr, NeedsDoAcrossArg, DoAcrossNumSyncIdsArg});
            Result.Privatized = false;
        }
        else if (!EmittedGpu)
        {
            ArrayType *PrivOffsetsTy = ArrayType::get(Int64Ty, PrivTargets.size());
            ArrayType *PrivKindsTy = ArrayType::get(Int32Ty, PrivTargets.size());
            ArrayType *PrivAuxTy = ArrayType::get(Int64Ty, PrivTargets.size() * 2);
            SmallVector<Constant *, 4> PrivOffsetConsts;
            SmallVector<Constant *, 4> PrivKindConsts;
            SmallVector<Constant *, 8> PrivAuxConsts;

            for (const EnvPrivTarget &TargetDesc : PrivTargets)
            {
                PrivOffsetConsts.push_back(ConstantInt::get(Int64Ty, TargetDesc.Offset));
                PrivKindConsts.push_back(ConstantInt::get(Int32Ty, TargetDesc.Kind));
                PrivAuxConsts.push_back(ConstantInt::get(Int64Ty, TargetDesc.AuxSizeOffset));
                PrivAuxConsts.push_back(ConstantInt::get(Int64Ty, TargetDesc.AuxCapOffset));
            }

            Result.PrivOffsetsGV = new GlobalVariable(
                *M,
                PrivOffsetsTy,
                true,
                GlobalValue::PrivateLinkage,
                ConstantArray::get(PrivOffsetsTy, PrivOffsetConsts),
                "priv_offsets");
            Result.PrivKindsGV = new GlobalVariable(
                *M,
                PrivKindsTy,
                true,
                GlobalValue::PrivateLinkage,
                ConstantArray::get(PrivKindsTy, PrivKindConsts),
                "priv_kinds");
            Result.PrivAuxGV = new GlobalVariable(
                *M,
                PrivAuxTy,
                true,
                GlobalValue::PrivateLinkage,
                ConstantArray::get(PrivAuxTy, PrivAuxConsts),
                "priv_aux");

            Value *Zero32 = ConstantInt::get(Int32Ty, 0);
            Value *PrivOffsetsPtr = B.CreateInBoundsGEP(PrivOffsetsTy, Result.PrivOffsetsGV, {Zero32, Zero32}, "priv_offsets_ptr");
            Value *PrivKindsPtr = B.CreateInBoundsGEP(PrivKindsTy, Result.PrivKindsGV, {Zero32, Zero32}, "priv_kinds_ptr");
            Value *PrivAuxPtr = B.CreateInBoundsGEP(PrivAuxTy, Result.PrivAuxGV, {Zero32, Zero32}, "priv_aux_ptr");
            B.CreateCall(ParallelForExFunc,
                         {StartArg,
                          EndArg,
                          StepArg,
                          CastedWrapper,
                          RawPtr,
                          ConstantInt::get(Int64Ty, NewEnvSize),
                          PrivOffsetsPtr,
                          PrivKindsPtr,
                          PrivAuxPtr,
                          ConstantInt::get(Int32Ty, (unsigned)PrivTargets.size()),
                          NeedsDoAcrossArg,
                          DoAcrossNumSyncIdsArg});
            Result.Privatized = true;
            Result.NumPrivTargets = PrivTargets.size();
        }

        B.CreateBr(Target.ExitBlock);
        Target.Preheader->getTerminator()->eraseFromParent();

        Result.ParallelReady = true;
        /* Debug logging disabled: parallel-clone-transformed */
        return Result;
    }

    static StructType *getLoopProfileDescTy(LLVMContext &Ctx)
    {
        Type *Int32Ty = Type::getInt32Ty(Ctx);
        Type *Int64Ty = Type::getInt64Ty(Ctx);
        Type *Int8PtrTy = Type::getInt8Ty(Ctx)->getPointerTo();
        return StructType::get(Ctx,
                               {Int32Ty,
                                Int32Ty,
                                Int32Ty,
                                Int32Ty,
                                Int64Ty,
                                Int64Ty->getPointerTo(),
                                Int32Ty,
                                Int32Ty,
                                Int8PtrTy,
                                Int32Ty,
                                Int32Ty,
                                Int32Ty,
                                Int32Ty,
                                Int32Ty,
                                Int32Ty,
                                Int64Ty});
    }

    static Constant *createCStringPtr(Module &M, StringRef Text, StringRef GlobalName)
    {
        LLVMContext &Ctx = M.getContext();
        auto *TextTy = ArrayType::get(Type::getInt8Ty(Ctx), Text.size() + 1);
        auto *TextGV = new GlobalVariable(
            M,
            TextTy,
            true,
            GlobalValue::PrivateLinkage,
            ConstantDataArray::getString(Ctx, Text, true),
            GlobalName);
        Constant *Zero32 = ConstantInt::get(Type::getInt32Ty(Ctx), 0);
        Constant *Indices[] = {Zero32, Zero32};
        return ConstantExpr::getInBoundsGetElementPtr(TextTy, TextGV, Indices);
    }

    static GlobalVariable *buildLoopProfileDescriptor(Module &M,
                                                      const LoopVersioningInfo &Versioning,
                                                      const LoopCandidateAnalysis &Candidate,
                                                      const ParallelTransformResult &Transform,
                                                      StringRef FunctionName,
                                                      StringRef HeaderName)
    {
        LLVMContext &Ctx = M.getContext();
        StructType *DescTy = getLoopProfileDescTy(Ctx);
        Type *Int32Ty = Type::getInt32Ty(Ctx);
        Type *Int64Ty = Type::getInt64Ty(Ctx);

        Constant *PrivOffsetsPtr = ConstantPointerNull::get(cast<PointerType>(Int64Ty->getPointerTo()));
        if (Transform.PrivOffsetsGV)
        {
            ArrayType *PrivOffsetsTy = cast<ArrayType>(Transform.PrivOffsetsGV->getValueType());
            Constant *Zero32 = ConstantInt::get(Int32Ty, 0);
            Constant *Indices[] = {Zero32, Zero32};
            PrivOffsetsPtr = ConstantExpr::getInBoundsGetElementPtr(
                PrivOffsetsTy,
                Transform.PrivOffsetsGV,
                Indices);
        }

        Constant *DebugName = createCStringPtr(
            M,
            (FunctionName + "::" + HeaderName).str(),
            (Twine("sgpl.loop.debug.") + Twine(Versioning.LoopId)).str());

        Constant *Fields[] = {
            ConstantInt::get(Int32Ty, Versioning.LoopId),
            ConstantInt::get(Int32Ty,
                             Versioning.Mode == ParallelMode::DoAcross ? SGPL_LOOP_DOACROSS : SGPL_LOOP_DOALL),
            ConstantInt::get(Int32Ty,
                             Transform.Privatized ? SGPL_RUNTIME_PRIVATIZED : SGPL_RUNTIME_PLAIN),
            ConstantInt::get(Int32Ty, 0),
            ConstantInt::get(Int64Ty, Transform.EnvSize),
            PrivOffsetsPtr,
            ConstantInt::get(Int32Ty, Transform.NumPrivTargets),
            ConstantInt::get(Int32Ty, 0),
            DebugName,
            ConstantInt::get(Int32Ty, Candidate.DoAcrossWaitsPerIter),
            ConstantInt::get(Int32Ty, Candidate.DoAcrossPostsPerIter),
            ConstantInt::get(Int32Ty, Candidate.Mode == ParallelMode::DoAcross ? 1 : 0),
            ConstantInt::get(Int32Ty, 0),
            ConstantInt::get(Int32Ty, Transform.NumDoAcrossSyncIds),
            ConstantInt::get(Int32Ty, 0),
            ConstantInt::get(Int64Ty, 0)};

        auto *Descriptor = new GlobalVariable(
            M,
            DescTy,
            true,
            GlobalValue::PrivateLinkage,
            ConstantStruct::get(DescTy, Fields),
            "sgpl.loop.desc." + Twine(Versioning.LoopId));
        return Descriptor;
    }

    static void finalizeLoopDispatch(Function &F,
                                     const LoopVersioningInfo &Versioning,
                                     const LoopCandidateAnalysis &Candidate,
                                     const ParallelTransformResult &Transform)
    {
        if (!Transform.ParallelReady)
            return;

        Module *M = F.getParent();
        LLVMContext &Ctx = F.getContext();
        Type *Int32Ty = Type::getInt32Ty(Ctx);
        Type *Int64Ty = Type::getInt64Ty(Ctx);
        StructType *DescTy = getLoopProfileDescTy(Ctx);
        PointerType *DescPtrTy = DescTy->getPointerTo();

        GlobalVariable *Descriptor = buildLoopProfileDescriptor(
            *M, Versioning, Candidate, Transform, F.getName(), Versioning.Serial.Header->getName());

        if (Candidate.Mode == ParallelMode::DoAcross && Transform.WrapperFn)
            instrumentDoAcrossWrapper(*Transform.WrapperFn, Descriptor);

        if (Instruction *OldTerm = Versioning.DispatchBlock->getTerminator())
            OldTerm->eraseFromParent();

        IRBuilder<> DispatchBuilder(Versioning.DispatchBlock);
        Value *StartArg = castIntegerToI64(DispatchBuilder, Versioning.StartArg, "start64.mv");
        Value *EndArg = castIntegerToI64(DispatchBuilder, Versioning.EndArg, "end64.mv");
        Value *StepArg = castIntegerToI64(DispatchBuilder, Versioning.StepArg, "step64.mv");

        FunctionCallee SelectorFn = M->getOrInsertFunction(
            Versioning.Mode == ParallelMode::DoAcross ? "sgpl_should_parallelize_doacross" : "sgpl_should_parallelize_doall",
            FunctionType::get(Int32Ty, {DescPtrTy, Int64Ty, Int64Ty, Int64Ty}, false));

        Value *ShouldParallel =
            DispatchBuilder.CreateCall(SelectorFn, {Descriptor, StartArg, EndArg, StepArg}, "sgpl.should.parallel");
        Value *TakeParallel =
            DispatchBuilder.CreateICmpNE(ShouldParallel, ConstantInt::get(Int32Ty, 0), "sgpl.take.parallel");
        DispatchBuilder.CreateCondBr(TakeParallel, Versioning.Parallel.Preheader, Versioning.SerialStartBlock);

        if (Versioning.Mode == ParallelMode::DoAll && Versioning.SerialStartTimeSlot)
        {
            FunctionCallee NowFn = M->getOrInsertFunction(
                "sgpl_now_ns",
                FunctionType::get(Int64Ty, {}, false));
            IRBuilder<> SerialStartBuilder(Versioning.SerialStartBlock->getTerminator());
            Value *StartNs = SerialStartBuilder.CreateCall(NowFn, {}, "sgpl.serial.start");
            SerialStartBuilder.CreateStore(StartNs, Versioning.SerialStartTimeSlot);

            if (Instruction *OldExitTerm = Versioning.SerialExitBridge->getTerminator())
                OldExitTerm->eraseFromParent();

            IRBuilder<> SerialExitBuilder(Versioning.SerialExitBridge);
            Value *EndNs = SerialExitBuilder.CreateCall(NowFn, {}, "sgpl.serial.end");
            Value *StartNsLoad = SerialExitBuilder.CreateLoad(Int64Ty, Versioning.SerialStartTimeSlot, "sgpl.serial.start.load");
            Value *Elapsed = SerialExitBuilder.CreateSub(EndNs, StartNsLoad, "sgpl.serial.elapsed");
            FunctionCallee RecordFn = M->getOrInsertFunction(
                "sgpl_record_doall_serial_sample",
                FunctionType::get(Type::getVoidTy(Ctx), {DescPtrTy, Int64Ty, Int64Ty, Int64Ty, Int64Ty}, false));
            SerialExitBuilder.CreateCall(RecordFn, {Descriptor, StartArg, EndArg, StepArg, Elapsed});
            SerialExitBuilder.CreateBr(Versioning.SharedExitBlock);
        }
        else if (Versioning.Mode == ParallelMode::DoAcross &&
                 Versioning.DoAcrossDepTotalSlot &&
                 Versioning.DoAcrossIndTotalSlot)
        {
            if (Instruction *OldExitTerm = Versioning.SerialExitBridge->getTerminator())
                OldExitTerm->eraseFromParent();

            IRBuilder<> SerialExitBuilder(Versioning.SerialExitBridge);
            Value *DepTotal = SerialExitBuilder.CreateLoad(Int64Ty, Versioning.DoAcrossDepTotalSlot, "sgpl.dep.total.load");
            Value *IndTotal = SerialExitBuilder.CreateLoad(Int64Ty, Versioning.DoAcrossIndTotalSlot, "sgpl.ind.total.load");
            FunctionCallee RecordFn = M->getOrInsertFunction(
                "sgpl_record_doacross_serial_sample",
                FunctionType::get(Type::getVoidTy(Ctx), {DescPtrTy, Int64Ty, Int64Ty, Int64Ty, Int64Ty, Int64Ty}, false));
            SerialExitBuilder.CreateCall(RecordFn, {Descriptor, StartArg, EndArg, StepArg, DepTotal, IndTotal});
            SerialExitBuilder.CreateBr(Versioning.SharedExitBlock);
        }
    }

    static OutlineLoopResult outlineLoop(Function &F,
                                         LoopInfo &LI,
                                         DominatorTree &DT,
                                         ScalarEvolution &SE)
    {
        OutlineLoopResult Result;
        /* Debug logging disabled: scanning */

        std::optional<LoopCandidateAnalysis> Candidate =
            findOutermostParallelizableLoop(F, LI, DT, SE);
        if (!Candidate)
        {
            /* Debug logging disabled: no loops outlined */
            return Result;
        }

        LoopVersioningInfo Versioning = createLoopMultiversion(F, *Candidate, SE);
        Result.Changed = true;
        instrumentDoAcrossSerialBlocks(F, Versioning, *Candidate);

        ParallelTransformResult Transform = transformLoopCloneToParallel(
            F,
            Versioning.Parallel,
            Versioning.StartArg,
            Versioning.EndArg,
            Versioning.StepArg);
        Result.Changed = Result.Changed || Transform.Changed;
        Result.Outlined = Transform.Outlined;

        // The original loop survives as the serial fallback path; strip the
        // parallel tag from it so a later outliner pass does not outline it
        // again (which would create a duplicate kernel/parallel dispatch).
        // Nested loops keep their own tags.
        if (Transform.Outlined && Candidate->Header && Candidate->Header->getTerminator())
            Candidate->Header->getTerminator()->setMetadata("my.loop.parallel", nullptr);

        finalizeLoopDispatch(F, Versioning, *Candidate, Transform);
        return Result;
    }

    struct LoopOutlinerPass : public PassInfoMixin<LoopOutlinerPass>
    {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
        {
            /* Debug logging disabled: pass run */
            if (F.getName().starts_with("wrapper_") ||
                F.getName().contains(".loopcond") ||
                F.hasFnAttribute("outlined-loop"))
            {
                /* Debug logging disabled: skip generated function */
                return PreservedAnalyses::all();
            }

            LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
            DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
            ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

            OutlineLoopResult Outline = outlineLoop(F, LI, DT, SE);
            if (Outline.Outlined)
                Outline.Outlined->addFnAttr("outlined-loop");

            if (Outline.Changed)
            {
                /* Debug logging disabled: changed function */
                return PreservedAnalyses::none();
            }

            /* Debug logging disabled: no outlining performed */
            return PreservedAnalyses::all();
        }
    };
} // namespace

void registerLoopOutlinerPass(FunctionPassManager &FPM)
{
    FPM.addPass(LoopOutlinerPass());
}

void runLoopOutlinerOnModule(Module &M)
{
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;

    PassBuilder PB;
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    FunctionPassManager FPM;
    FPM.addPass(LoopOutlinerPass());

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

void emitGpuKernels(Module &M, StringRef PtxPath)
{
    NamedMDNode *GpuKernelsMD = M.getNamedMetadata("graph.gpu.kernels");
    if (!GpuKernelsMD || GpuKernelsMD->getNumOperands() == 0)
        return;

    SmallVector<Function *, 8> Kernels;
    for (const MDNode *Op : GpuKernelsMD->operands())
    {
        if (!Op || Op->getNumOperands() == 0)
            continue;
        if (auto *VAM = dyn_cast_or_null<ValueAsMetadata>(Op->getOperand(0)))
            if (auto *Kernel = dyn_cast<Function>(VAM->getValue()))
                Kernels.push_back(Kernel);
    }

    SmallPtrSet<Function *, 16> Seen;
    SmallVector<Function *, 16> Order;
    std::function<void(Function *)> Collect = [&](Function *Fn)
    {
        if (!Fn || Fn->isDeclaration() || Fn->isIntrinsic())
            return;
        if (!Seen.insert(Fn).second)
            return;
        Order.push_back(Fn);
        for (BasicBlock &BB : *Fn)
        {
            for (Instruction &I : BB)
            {
                if (auto *CI = dyn_cast<CallInst>(&I))
                    if (Function *Callee = CI->getCalledFunction())
                        Collect(Callee);
            }
        }
    };
    for (Function *K : Kernels)
        Collect(K);

    // Gather the globals referenced by the kernel graph so they can be cloned
    // into the device module. The runtime then copies their data in/out by name.
    SmallPtrSet<GlobalVariable *, 16> SeenGlobals;
    SmallVector<GlobalVariable *, 16> Globals;
    for (Function *Fn : Order)
    {
        for (BasicBlock &BB : *Fn)
        {
            for (Instruction &I : BB)
            {
                for (Value *Op : I.operands())
                {
                    if (auto *GV = dyn_cast<GlobalVariable>(Op->stripPointerCasts()))
                    {
                        // Host-only runtime structures (loop descriptors, ...)
                        // must never be cloned: their names contain '.' which
                        // is illegal in PTX identifiers, and the GPU path never
                        // uses them.
                        if (GV->getName().starts_with("sgpl."))
                            continue;
                        if (SeenGlobals.insert(GV).second)
                            Globals.push_back(GV);
                    }
                }
            }
        }
    }

    LLVMContext &Ctx = M.getContext();
    std::unique_ptr<Module> DeviceModule = std::make_unique<Module>("gpu_kernels", Ctx);

    ValueToValueMapTy VMap;
    for (GlobalVariable *OldGV : Globals)
    {
        GlobalVariable *NewGV = new GlobalVariable(
            *DeviceModule, OldGV->getValueType(), false, GlobalValue::ExternalLinkage,
            Constant::getNullValue(OldGV->getValueType()), OldGV->getName());
        NewGV->setAlignment(OldGV->getAlign());
        VMap[OldGV] = NewGV;
    }
    for (Function *Old : Order)
    {
        Function *New = Function::Create(Old->getFunctionType(), GlobalValue::ExternalLinkage,
                                         Old->getName(), DeviceModule.get());
        New->copyAttributesFrom(Old);
        if (Old->getCallingConv() == CallingConv::PTX_Kernel)
            New->setCallingConv(CallingConv::PTX_Kernel);
        VMap[Old] = New;
        auto NewArgIt = New->arg_begin();
        for (auto OldArgIt = Old->arg_begin(); OldArgIt != Old->arg_end(); ++OldArgIt, ++NewArgIt)
            VMap[&*OldArgIt] = &*NewArgIt;
    }
    for (Function *Old : Order)
    {
        Function *New = cast<Function>(VMap[Old]);
        SmallVector<ReturnInst *, 8> Returns;
        CloneFunctionInto(New, Old, VMap, CloneFunctionChangeType::DifferentModule, Returns);
    }

    // Cloned bodies may still reference the host module's intrinsic declarations
    // (e.g. the NVVM special-register reads). Re-home them into the device module.
    for (Function &F : *DeviceModule)
    {
        SmallVector<CallInst *, 16> IntrinsicCalls;
        for (BasicBlock &BB : F)
            for (Instruction &I : BB)
                if (auto *CI = dyn_cast<CallInst>(&I))
                    if (Function *Callee = CI->getCalledFunction())
                        if (Callee->isIntrinsic())
                            IntrinsicCalls.push_back(CI);
        for (CallInst *CI : IntrinsicCalls)
        {
            Function *OldCallee = CI->getCalledFunction();
            Function *Decl = Intrinsic::getDeclaration(DeviceModule.get(), OldCallee->getIntrinsicID());
            CI->setCalledFunction(Decl);
        }
    }

    // Drop all instruction metadata (debug, TBAA, ...) from the device module so
    // cloned instructions never reference DI nodes owned by the host module.
    for (Function &F : *DeviceModule)
    {
        for (BasicBlock &BB : F)
        {
            for (Instruction &I : BB)
            {
                SmallVector<std::pair<unsigned, MDNode *>, 8> MDs;
                I.getAllMetadata(MDs);
                for (auto &KV : MDs)
                    I.setMetadata(KV.first, nullptr);
            }
        }
    }

    // DOACROSS bodies carry host-side doacross_wait/post/init calls and the
// wrapper carries the profile enter/exit hooks (all used only by the CPU
// fallback path). On the device the cooperative wave barriers subsume them,
// so strip the calls before inlining.
    for (Function &F : *DeviceModule)
    {
        SmallVector<CallInst *, 16> DoAcrossCalls;
        for (BasicBlock &BB : F)
            for (Instruction &I : BB)
                if (auto *CI = dyn_cast<CallInst>(&I))
                    if (Function *Callee = CI->getCalledFunction())
                    {
                        StringRef N = Callee->getName();
                        if (N == "doacross_wait" || N == "doacross_post" || N == "doacross_init" ||
                            N == "sgpl_doacross_profile_enter" || N == "sgpl_doacross_profile_exit")
                            DoAcrossCalls.push_back(CI);
                    }
        for (CallInst *CI : DoAcrossCalls)
            CI->eraseFromParent();
    }

    // Inline the wrapper chain (kernel -> wrapper -> outlined body) into the
    // kernel. A self-contained entry keeps the emitted PTX free of extra
    // forward-declared .visible .func symbols, which the driver's JIT compiler
    // rejects when more than one is present.
    {
        bool Changed = true;
        while (Changed)
        {
            Changed = false;
            for (Function &Fn : *DeviceModule)
            {
                if (Fn.isDeclaration() || Fn.isIntrinsic())
                    continue;
                for (BasicBlock &BB : Fn)
                {
                    for (Instruction &I : BB)
                    {
                        auto *CI = dyn_cast<CallInst>(&I);
                        if (!CI)
                            continue;
                        Function *Callee = CI->getCalledFunction();
                        if (!Callee || Callee->isDeclaration() || Callee->isIntrinsic() || Callee == &Fn)
                            continue;
                        InlineFunctionInfo IFI;
                        if (InlineFunction(*CI, IFI).isSuccess())
                        {
                            Changed = true;
                            break;
                        }
                    }
                    if (Changed)
                        break;
                }
                if (Changed)
                    break;
            }
        }

        // Drop now-dead helper functions (wrapper/outlined) so they do not
        // linger in the PTX as unused declarations.
        SmallVector<Function *, 4> DeadFns;
        for (Function &Fn : *DeviceModule)
            if (!Fn.isDeclaration() && !Fn.isIntrinsic() && Fn.use_empty() &&
                Fn.getCallingConv() != CallingConv::PTX_Kernel)
                DeadFns.push_back(&Fn);
        for (Function *Fn : DeadFns)
            Fn->eraseFromParent();
    }

    // Remove the temporary kernel placeholders (and their registry) from the host
    // module now that their bodies have been cloned into the device module. This
    // keeps the NVPTX kernel calling convention out of host codegen regardless of
    // whether PTX emission succeeds below.
    GpuKernelsMD->eraseFromParent();
    for (Function *K : Kernels)
        K->eraseFromParent();

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    std::string Error;
    const Target *NvTarget = TargetRegistry::lookupTarget("nvptx64-nvidia-cuda", Error);
    if (!NvTarget)
    {
        errs() << "[gpu] NVPTX target unavailable (" << Error << "); GPU offload disabled\n";
        return;
    }

    TargetOptions Opts;
    std::optional<Reloc::Model> RM = std::nullopt;
    std::optional<CodeModel::Model> CM = std::nullopt;
    std::unique_ptr<TargetMachine> TM(
        NvTarget->createTargetMachine("nvptx64-nvidia-cuda", "sm_70", "", Opts, RM, CM,
                                      CodeGenOptLevel::Default, /*JIT=*/false));
    if (!TM)
    {
        errs() << "[gpu] Failed to create NVPTX TargetMachine; GPU offload disabled\n";
        return;
    }

    DeviceModule->setDataLayout(TM->createDataLayout());
    DeviceModule->setTargetTriple("nvptx64-nvidia-cuda");

    SmallString<0> PtxBufStr;
    raw_svector_ostream PtxBuf(PtxBufStr);
    legacy::PassManager PM;
    if (TM->addPassesToEmitFile(PM, PtxBuf, nullptr, CodeGenFileType::AssemblyFile))
    {
        errs() << "[gpu] NVPTX TargetMachine cannot emit PTX assembly\n";
        return;
    }
    PM.run(*DeviceModule);
    std::string PtxText(PtxBufStr.str());
    if (PtxText.empty())
    {
        errs() << "[gpu] NVPTX emitted empty PTX; GPU offload disabled\n";
        return;
    }

    std::error_code EC;
    raw_fd_ostream PtxOS(PtxPath, EC, sys::fs::OF_None);
    if (EC)
    {
        errs() << "[gpu] Could not open '" << PtxPath << "' for writing: " << EC.message() << "\n";
    }
    else
    {
        PtxOS << PtxText;
        PtxOS.flush();
    }

    // Embed the PTX text in the host module so the executable can load the
    // module without reading kernels.ptx from disk. External linkage and the
    // exact symbol name make it visible to gpu_runtime.c via a weak extern.
    Constant *PtxArr = ConstantDataArray::getString(Ctx, PtxText, /*AddNull=*/true);
    new GlobalVariable(M, PtxArr->getType(), /*isConstant=*/true,
                       GlobalValue::ExternalLinkage, PtxArr, "gpu_embedded_ptx");
}
