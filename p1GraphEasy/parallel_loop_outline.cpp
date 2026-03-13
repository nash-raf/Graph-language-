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
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

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
        /* Debug logging disabled. Restore errs() output here to re-enable loop-outliner state traces. */
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
        SmallPtrSet<Value *, 8> Reads;
        SmallPtrSet<Value *, 8> Writes;

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

                Value *SetArg = CI->getArgOperand(0)->stripPointerCasts();
                if (Name == "roaring_bitmap_add")
                {
                    if (Candidate.Mode != ParallelMode::DoAll)
                        return std::string("shared-set-privatization-requires-doall");
                    Writes.insert(SetArg);
                }
                else
                {
                    Reads.insert(SetArg);
                }
            }
        }

        for (Value *V : Writes)
        {
            if (Reads.contains(V))
                return std::string("shared-set-read-write-same-object");
        }

        return std::nullopt;
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

        Candidate.BackedgeCount = SE.getBackedgeTakenCount(L);
        if (isa<SCEVCouldNotCompute>(Candidate.BackedgeCount))
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

        const SCEV *TripCount = SE.getAddExpr(
            Candidate.BackedgeCount,
            SE.getOne(Candidate.BackedgeCount->getType()));
        Candidate.Start = AR->getStart();
        Candidate.Step = AR->getStepRecurrence(SE);
        Candidate.End = SE.getAddExpr(
            Candidate.Start,
            SE.getMulExpr(Candidate.Step, TripCount));
        Candidate.ConstantTripCount = getConstantTripCount(Candidate, SE);
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

        Info.Parallel.Mode = Candidate.Mode;
        Info.Parallel.Header = ClonedHeader;
        Info.Parallel.Preheader = ParallelPreheader;
        Info.Parallel.Latch = ClonedLatch;
        Info.Parallel.ExitingBlock = ClonedExitingBlock;
        Info.Parallel.ExitBlock = ParallelExitBridge;
        Info.Parallel.IndVar = ClonedIndVar;
        Info.Parallel.Depth = Candidate.Depth;
        Info.Parallel.LoopBlocks = ClonedLoopBlocks;
        for (BasicBlock *BB : ClonedLoopBlocks)
        {
            if (BB != ClonedHeader)
                Info.Parallel.LoopBody.push_back(BB);
        }

        /* Debug logging disabled: multiversion-created */
        return Info;
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

        if (!Target.Preheader || !Target.ExitBlock || !Target.Preheader->getTerminator())
        {
            logLoopState(F, Target.Header, Target.Depth, "skip:parallel-clone-invalid-structure");
            return Result;
        }

        bool IsDoAcross = Target.Mode == ParallelMode::DoAcross;
        bool IsDoAll = Target.Mode == ParallelMode::DoAll;

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
        int InductionParamIndex = -1;

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

                auto *Arg = dyn_cast<Argument>(CI->getArgOperand(0)->stripPointerCasts());
                if (!Arg)
                    continue;
                unsigned ParamIndex = Arg->getArgNo();
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
                if (AI->getFunction() == &F && !Inputs.count(AI) && !Outputs.count(AI))
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

        if (UnsupportedSharedSet)
            return Result;

        IRBuilder<> B(Target.Preheader->getTerminator());
        Type *VoidTy = Type::getVoidTy(Ctx);
        Type *Int32Ty = Type::getInt32Ty(Ctx);
        Type *Int64Ty = Type::getInt64Ty(Ctx);
        Type *Int8PtrTy = Type::getInt8Ty(Ctx)->getPointerTo();
        Type *LoopBodyFnTy = FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false)->getPointerTo();

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

            if (!Orig)
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

        SmallVector<uint64_t, 4> PrivatizedEnvFieldOffsets;
        const StructLayout *EnvLayout = M->getDataLayout().getStructLayout(NewEnvStructTy);
        for (unsigned ParamIndex = 0; ParamIndex < ParamUsesPrivatizedSet.size(); ++ParamIndex)
        {
            if (!ParamUsesPrivatizedSet[ParamIndex])
                continue;
            unsigned EnvFieldIndex = (unsigned)ParamToEnvFieldIndex[ParamIndex];
            PrivatizedEnvFieldOffsets.push_back(EnvLayout->getElementOffset(EnvFieldIndex));
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
        Result.WrapperFn = WrapperFn;

        Value *CastedWrapper = B.CreateBitCast(WrapperFn, LoopBodyFnTy);
        FunctionCallee ParallelForFunc = M->getOrInsertFunction(
            "parallel_for_runtime",
            FunctionType::get(VoidTy, {Int64Ty, Int64Ty, Int64Ty, LoopBodyFnTy, Int8PtrTy, Int32Ty, Int32Ty}, false));
        FunctionCallee ParallelForExFunc = M->getOrInsertFunction(
            "parallel_for_runtime_ex",
            FunctionType::get(VoidTy,
                              {Int64Ty, Int64Ty, Int64Ty, LoopBodyFnTy, Int8PtrTy, Int64Ty, Int64Ty->getPointerTo(), Int32Ty, Int32Ty, Int32Ty},
                              false));

        Value *StartArg = castIntegerToI64(B, StartV, "start64");
        Value *EndArg = castIntegerToI64(B, EndV, "end64");
        Value *StepArg = castIntegerToI64(B, StepV, "step64");
        Value *NeedsDoAcrossArg = ConstantInt::get(Int32Ty, IsDoAcross ? 1 : 0);
        Value *DoAcrossNumSyncIdsArg = ConstantInt::get(Int32Ty, Result.NumDoAcrossSyncIds);

        if (PrivatizedEnvFieldOffsets.empty())
        {
            B.CreateCall(ParallelForFunc,
                         {StartArg, EndArg, StepArg, CastedWrapper, RawPtr, NeedsDoAcrossArg, DoAcrossNumSyncIdsArg});
            Result.Privatized = false;
        }
        else
        {
            ArrayType *PrivOffsetsTy = ArrayType::get(Int64Ty, PrivatizedEnvFieldOffsets.size());
            SmallVector<Constant *, 4> PrivOffsetConsts;
            for (uint64_t Offset : PrivatizedEnvFieldOffsets)
                PrivOffsetConsts.push_back(ConstantInt::get(Int64Ty, Offset));

            Result.PrivOffsetsGV = new GlobalVariable(
                *M,
                PrivOffsetsTy,
                true,
                GlobalValue::PrivateLinkage,
                ConstantArray::get(PrivOffsetsTy, PrivOffsetConsts),
                "priv_offsets");

            Value *Zero32 = ConstantInt::get(Int32Ty, 0);
            Value *PrivOffsetsPtr = B.CreateInBoundsGEP(PrivOffsetsTy, Result.PrivOffsetsGV, {Zero32, Zero32}, "priv_offsets_ptr");
            B.CreateCall(ParallelForExFunc,
                         {StartArg,
                          EndArg,
                          StepArg,
                          CastedWrapper,
                          RawPtr,
                          ConstantInt::get(Int64Ty, NewEnvSize),
                          PrivOffsetsPtr,
                          ConstantInt::get(Int32Ty, (unsigned)PrivatizedEnvFieldOffsets.size()),
                          NeedsDoAcrossArg,
                          DoAcrossNumSyncIdsArg});
            Result.Privatized = true;
            Result.NumPrivTargets = PrivatizedEnvFieldOffsets.size();
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