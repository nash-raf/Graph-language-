#include "llvm/IR/Metadata.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DataLayout.h"
#include "DependencyGraph.h"
#include "llvm/Analysis/AliasAnalysis.h"

struct LoopBounds
{
    bool success = false;

    // Materialized values in i64
    llvm::Value *startV = nullptr;
    llvm::Value *stepV = nullptr;
    llvm::Value *tripV = nullptr; // iteration count
    llvm::Value *endV = nullptr;  // start + step*trip

    // constant facts
    bool constStart = false;
    int64_t startConst = 0;
    bool constStep = false;
    int64_t stepConst = 0;
    bool constTrip = false;
    uint64_t tripConst = 0;

    std::string msg;
};

enum Correctness {
    SAFE,
    COPY,
    REJECT
};

static bool isMallocLike(llvm::Value *V) {
  if (auto *CI = llvm::dyn_cast<llvm::CallInst>(V)) {
    llvm::Function *F = CI->getCalledFunction();
    if (!F) return false;
    llvm::StringRef name = F->getName();
    return name == "malloc" || name == "calloc" || name == "realloc"
           || name == "__libc_malloc";
  }
  return false;
}

Correctness safety(llvm::Value *ptr, const llvm::DataLayout &DL, llvm::AAResults &AA) {
    llvm::Value *UO = llvm::getUnderlyingObject(ptr);
    llvm::MemoryLocation Loc(ptr, llvm::LocationSize::precise(DL.getTypeAllocSize(ptr->getType())));
    auto *Call = llvm::dyn_cast<llvm::CallInst>(UO);
    
    if(llvm::isa<llvm::GlobalVariable>(UO) || (Call && Call->getCalledFunction()->getName() == "malloc")) {
        return SAFE;
    }

    else if (llvm::isa<llvm::AllocaInst>(UO) || llvm::isa<llvm::Argument>(UO) || llvm::isa<llvm::PHINode>(UO)) {
        return COPY; // alloca is safe to copy
    }

    else
    {
        return REJECT;
    }
}

static llvm::SmallVector<llvm::Value *, 16>
collectLiveIn(llvm::Loop *L, llvm::Function &F, llvm::DependencyGraph &G)
{
    llvm::SmallVector<llvm::Value *, 16> liveIn;
    llvm::SmallPtrSet<const llvm::Value *, 16> liveInSet;

    if (!L)
        return liveIn;

    llvm::Module *M = F.getParent();
    const llvm::DataLayout &DL = M->getDataLayout();
    const uint64_t LARGE_AGG_BYTES = 128;

    llvm::SmallPtrSet<const llvm::Instruction *, 128> loopInsts;
    for (llvm::BasicBlock *BB : L->getBlocks())
    {
        for (llvm::Instruction &I : *BB)
        {
            loopInsts.insert(&I);
        }
    }

    auto addCandidate = [&](llvm::Value *V)
    {
        if (!V)
            return;

        if (llvm::isa<llvm::Constant>(V) || llvm::isa<llvm::GlobalValue>(V))
            return;

        //  - load of a large aggregate -> prefer the load's pointer operand
        if (auto *LI = llvm::dyn_cast<llvm::LoadInst>(V))
        {
            llvm::Type *Ty = LI->getType();
            if (Ty->isAggregateType())
            {
                uint64_t sz = Ty->isSized() ? DL.getTypeAllocSize(Ty) : 0;
                if (sz > LARGE_AGG_BYTES)
                {
                    llvm::Value *ptr = LI->getPointerOperand();
                    if (liveInSet.insert(ptr).second)
                        liveIn.push_back(ptr);
                    return;
                }
            }

            if (V->getType()->isPointerTy())
            {
                llvm::Type *Ty = V->getType();
                if (Ty && Ty->isAggregateType())
                {
                    uint64_t sz = Ty->isSized() ? DL.getTypeAllocSize(Ty) : 0;
                    if (sz > LARGE_AGG_BYTES)
                    {
                        llvm::Value *ptr = LI->getPointerOperand();
                        if (liveInSet.insert(ptr).second)
                            liveIn.push_back(ptr);
                        return;
                    }
                }
            }

            if (V->getType()->isAggregateType())
            {
                uint64_t sz = V->getType()->isSized() ? DL.getTypeAllocSize(V->getType()) : 0;
                if (sz > LARGE_AGG_BYTES)
                {
                    llvm::Value *UO = llvm::getUnderlyingObject(V);
                    if (UO && UO != V)
                    {
                        if (UO->getType()->isPointerTy())
                        {
                            llvm::Type *elem = UO->getType();
                            if (elem && elem->isAggregateType() && elem->isSized())
                            {
                                uint64_t usz = DL.getTypeAllocSize(elem);
                                if (usz > LARGE_AGG_BYTES)
                                {
                                    if (liveInSet.insert(UO).second)
                                        liveIn.push_back(UO);
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (liveInSet.insert(V).second)
            liveIn.push_back(V);
    };

    for (auto &E : G.Edges)
    {
        llvm::Instruction *src = G.getInstByID(E.source);
        llvm::Instruction *tgt = G.getInstByID(E.target);
        if (!src || !tgt)
            continue;
        bool tgtIn = loopInsts.count(tgt);
        bool srcIn = loopInsts.count(src);
        if (tgtIn && !srcIn)
        {
            addCandidate(src);
        }
    }

    for (llvm::BasicBlock *BB : L->getBlocks())
    {
        for (llvm::Instruction &I : *BB)
        {
            for (llvm::Use &U : I.operands())
            {
                llvm::Value *V = U.get();
                if (!V)
                    continue;
                // skip constants/globals
                if (llvm::isa<llvm::Constant>(V) || llvm::isa<llvm::GlobalValue>(V))
                    continue;

                if (llvm::Instruction *VI = llvm::dyn_cast<llvm::Instruction>(V))
                {
                    if (!loopInsts.count(VI))
                        addCandidate(V);
                }
                else if (llvm::Argument *Arg = llvm::dyn_cast<llvm::Argument>(V))
                {
                    addCandidate(Arg);
                }
                else
                {
                    addCandidate(V);
                }
            }
        }
    }

    return liveIn;
}

static bool loopParallelizable(llvm::Loop *L)
{
    if (!L)
        return false;

    // Try canonical loop-id first
    llvm::MDNode *LoopID = L->getLoopID();
    // Fallback: terminator metadata (older style)
    if (!LoopID && L->getHeader())
    {
        if (llvm::Instruction *T = L->getHeader()->getTerminator())
            LoopID = T->getMetadata("llvm.loop");
    }
    if (!LoopID)
    {
        llvm::errs() << "[bounds] no llvm.loop metadata on loop header '" << L->getHeader()->getName() << "'\n";
        return false;
    }

    bool found = false;
    for (unsigned i = 0, e = LoopID->getNumOperands(); i < e; ++i)
    {
        llvm::Metadata *Op = LoopID->getOperand(i);
        if (!Op)
            continue;

        // Case A: inner MDNode like !{!"llvm.loop.parallelize.enable", i1 true}
        if (auto *MDN = llvm::dyn_cast<llvm::MDNode>(Op))
        {
            if (MDN->getNumOperands() >= 1)
            {
                if (auto *S = llvm::dyn_cast<llvm::MDString>(MDN->getOperand(0)))
                {
                    llvm::StringRef Name = S->getString();
                    if (Name == "DOALL")
                        return true;
                    if (Name == "llvm.loop.parallelize.enable")
                    {
                        if (MDN->getNumOperands() >= 2)
                        {
                            if (auto *CAM = llvm::dyn_cast_or_null<llvm::ConstantAsMetadata>(MDN->getOperand(1)))
                                if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(CAM->getValue()))
                                    if (CI->isOne())
                                        return true;
                        }
                    }
                }
            }
        }

        // Case B: direct MDString operand !"DOALL"
        if (auto *S = llvm::dyn_cast<llvm::MDString>(Op))
        {
            if (S->getString() == "DOALL")
                return true;
        }
    }

    llvm::errs() << "[bounds] loop '" << L->getHeader()->getName() << "' not marked DOALL/parallelize\n";
    return false;
}

static llvm::Value *materialise(const llvm::SCEV *S, llvm::ScalarEvolution &SE,
                                const llvm::DataLayout &DL, llvm::Instruction *InsertPt,
                                const llvm::Twine &Name = "scev_val")
{

    if (!S)
        return nullptr;
    llvm::LLVMContext &ctx = InsertPt->getContext();
    llvm::Type *I64 = llvm::Type::getInt64Ty(ctx);

    if (auto *C = llvm::dyn_cast<llvm::SCEVConstant>(S))
    {
        return llvm::ConstantInt::get(I64, C->getValue()->getSExtValue());
    }

    llvm::SCEVExpander Exp(SE, DL, "scev_expander");
    llvm::Value *V = Exp.expandCodeFor(S, I64, InsertPt);
    if (V->getType() != I64)
    {
        llvm::IRBuilder<> Builder(InsertPt);
        if (V->getType()->isIntegerTy())
        {
            V = Builder.CreateSExtOrTrunc(V, I64, Name);
        }
        else
        {
            V = Builder.CreatePtrToInt(V, I64, Name);
        }
    }

    return V;
}

static LoopBounds extractLoopBounds(llvm::Loop *L, llvm::Function &F,
                                    llvm::FunctionAnalysisManager &FAM, llvm::Instruction *InsertPt)
{

    LoopBounds R;
    if (!L)
    {
        R.msg = "No Loop";
        return R;
    }

    llvm::BasicBlock *preheader = L->getLoopPreheader();
    llvm::BasicBlock *latch = L->getLoopLatch();
    llvm::errs() << "[debug] header=" << L->getHeader()->getName();
    if (preheader)
        llvm::errs() << " preheader=" << preheader->getName();
    else
        llvm::errs() << " preheader=null";

    if (latch)
        llvm::errs() << " latch=" << latch->getName();
    else
        llvm::errs() << " latch=null";
    llvm::errs() << "\n";

    // NOTE: we don't require a canonical preheader here — we will still attempt to find
    // an induction phi and use the provided InsertPt to materialize SCEV values.

    llvm::ScalarEvolution &SE = FAM.getResult<llvm::ScalarEvolutionAnalysis>(F);
    llvm::Module *M = F.getParent();
    const llvm::DataLayout &DL = M->getDataLayout();
    llvm::LLVMContext &ctx = F.getContext();
    llvm::Type *I64 = llvm::Type::getInt64Ty(ctx);

    llvm::PHINode *indPhi = nullptr;

    for (llvm::PHINode &phi : L->getHeader()->phis())
    {
        if (const llvm::SCEV *S = SE.getSCEV(&phi))
            if (const auto *AR = llvm::dyn_cast<llvm::SCEVAddRecExpr>(S))
                if (AR->getLoop() == L)
                {
                    indPhi = &phi;
                    break;
                }
    }

    if (!indPhi)
    {
        R.msg = "No canonical induction PHI found";
        return R;
    }

    const llvm::SCEV *PhiS = SE.getSCEV(indPhi);
    const auto *AR = llvm::dyn_cast<llvm::SCEVAddRecExpr>(PhiS);
    if (!AR)
    {
        R.msg = "Induction PHI is not AddRec";
        return R;
    }

    const llvm::SCEV *startS = AR->getStart();
    const llvm::SCEV *StepS = AR->getStepRecurrence(SE) ? AR->getStepRecurrence(SE) : (AR->getNumOperands() >= 2 ? AR->getOperand(1) : nullptr);

    if (!StepS)
    {
        R.msg = "Could not extract step SCEV";
        return R;
    }

    const llvm::SCEV *BEC = SE.getBackedgeTakenCount(L);

    llvm::Value *StartV = materialise(startS, SE, DL, InsertPt, "loop_start");
    llvm::Value *StepV = materialise(StepS, SE, DL, InsertPt, "loop_step");

    if (const auto *C = llvm::dyn_cast<llvm::SCEVConstant>(startS))
    {
        R.constStart = true;
        R.startConst = C->getAPInt().getSExtValue();
    }
    if (const auto *C = llvm::dyn_cast<llvm::SCEVConstant>(StepS))
    {
        R.constStep = true;
        R.stepConst = C->getAPInt().getSExtValue();
    }

    llvm::Value *TripV = nullptr;
    if (const auto *Cbec = llvm::dyn_cast<llvm::SCEVConstant>(BEC))
    {
        uint64_t bec = Cbec->getAPInt().getZExtValue();
        uint64_t trip = bec + 1;
        TripV = llvm::ConstantInt::get(I64, trip);
        R.constTrip = true;
        R.tripConst = trip;
    }
    else if (BEC && !llvm::isa<llvm::SCEVUnknown>(BEC))
    {
        // trip = bec + 1
        llvm::SCEVExpander Exp(SE, DL, "scev.expand");
        llvm::Value *BecV = Exp.expandCodeFor(BEC, I64, InsertPt);
        llvm::IRBuilder<> B(InsertPt);
        TripV = B.CreateAdd(BecV, llvm::ConstantInt::get(I64, 1), "trip");
    }
    else
    {
        // Fallback: single exit count
        llvm::SmallVector<llvm::BasicBlock *, 4> Exits;
        L->getExitBlocks(Exits);
        if (Exits.size() == 1)
        {
            const llvm::SCEV *ExitCount = SE.getExitCount(L, Exits[0]);
            if (const auto *Cc = llvm::dyn_cast_or_null<llvm::SCEVConstant>(ExitCount))
            {
                uint64_t trip = Cc->getAPInt().getZExtValue();
                TripV = llvm::ConstantInt::get(I64, trip);
                R.constTrip = true;
                R.tripConst = trip;
            }
            else if (ExitCount)
            {
                TripV = materialise(ExitCount, SE, DL, InsertPt, "trip");
            }
        }
    }

    if (!StartV || !StepV || !TripV)
    {
        R.msg = "Failed to materialize start/step/trip";
        return R;
    }

    // end = start + step * trip
    llvm::IRBuilder<> B(InsertPt);
    llvm::Value *Mul = B.CreateMul(StepV, TripV, "step_mul_trip");
    llvm::Value *EndV = B.CreateAdd(StartV, Mul, "loop_end");

    R.success = true;
    R.startV = StartV;
    R.stepV = StepV;
    R.tripV = TripV;
    R.endV = EndV;
    R.msg = "OK";
    return R;
}

struct LoopBoundExtractorPass : public llvm::PassInfoMixin<LoopBoundExtractorPass>
{
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM)
    {
        if (F.isDeclaration())
            return llvm::PreservedAnalyses::all();

        auto &LI = FAM.getResult<llvm::LoopAnalysis>(F);

        bool any = false;
        for (llvm::Loop *L : LI)
        {
            any |= processLoopRecursive(F, FAM, L);
        }

        // We only *read* analyses and insert code in preheaders;
        // conservatively say we modified IR if we succeeded anywhere.
        return any ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
    }

    // --- processLoopRecursive: choose insertion point with fallback ---
    bool processLoopRecursive(llvm::Function &F, llvm::FunctionAnalysisManager &FAM, llvm::Loop *L)
    {
        bool changed = false;

        if (loopParallelizable(L))
        {
            llvm::BasicBlock *Preheader = L->getLoopPreheader();
            llvm::Instruction *IP = nullptr;

            if (Preheader)
            {
                // ideal: materialize in the preheader (run once before loop)
                IP = Preheader->getTerminator();
                llvm::errs() << "[bounds] using preheader '" << Preheader->getName()
                             << "' for loop '" << L->getHeader()->getName() << "'\n";
            }
            else
            {
                // fallback: insert at first non-PHI in header (will be executed inside the loop)
                llvm::BasicBlock *Header = L->getHeader();
                if (Header)
                {
                    IP = Header->getFirstNonPHI();
                    if (IP)
                    {
                        llvm::errs() << "[bounds] no preheader for loop '" << Header->getName()
                                     << "', falling back to header insertion at '" << IP->getName() << "'\n";
                    }
                    else
                    {
                        llvm::errs() << "[bounds] no insertion point (no non-PHI) for header '"
                                     << Header->getName() << "', skipping\n";
                    }
                }
            }

            if (!IP)
            {
                llvm::errs() << " [bounds] skip (no insertion point)\n";
            }
            else
            {
                LoopBounds LB = extractLoopBounds(L, F, FAM, IP);
                if (LB.success)
                {
                    changed = true; // we emitted IR to compute bounds
                    llvm::errs() << " [bounds] DOALL loop @ '" << L->getHeader()->getName() << "'\n";
                    llvm::errs() << "   start=";
                    if (LB.constStart)
                        llvm::errs() << LB.startConst;
                    else
                        llvm::errs() << *LB.startV;
                    llvm::errs() << "\n";
                    llvm::errs() << "   step =";
                    if (LB.constStep)
                        llvm::errs() << LB.stepConst;
                    else
                        llvm::errs() << *LB.stepV;
                    llvm::errs() << "\n";
                    llvm::errs() << "   trip =";
                    if (LB.constTrip)
                        llvm::errs() << LB.tripConst;
                    else
                        llvm::errs() << *LB.tripV;
                    llvm::errs() << "\n";
                    llvm::errs() << "   end  =" << *LB.endV << "\n";
                }
                else
                {
                    llvm::errs() << " [bounds] failed: " << LB.msg << "\n";
                }
            }
        }

        for (llvm::Loop *Sub : L->getSubLoops())
            changed |= processLoopRecursive(F, FAM, Sub);

        return changed;
    }
};

// -------- Plugin registration ----------
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION,
        "LoopBoundExtractor",
        LLVM_VERSION_STRING,
        [](llvm::PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>)
                {
                    if (Name == "loop-bound-extractor")
                    {
                        FPM.addPass(LoopBoundExtractorPass());
                        return true;
                    }
                    return false;
                });
        }};
}