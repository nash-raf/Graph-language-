#include "pdg.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream> // for DOT output
#include <iostream>
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include <algorithm>
#include <limits>
#include <utility>
#include <functional>
#include "llvm/ADT/SCCIterator.h"
#include <string>
#include "parallel_runtime.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_os_ostream.h"
#include <cstdlib>

#define SGPL_QUIET_LOGS 1
#if SGPL_QUIET_LOGS
#define errs llvm::nulls
#define outs llvm::nulls
#endif

struct TaskArgumentInfo
{
    llvm::SmallVector<llvm::Value *> originalValues; // Values from main_original
    llvm::SmallVector<llvm::Type *> types;           // Argument types
    llvm::StructType *argStructType;                 // Struct to hold arguments

    TaskArgumentInfo() : argStructType(nullptr) {}
};

// Helper function to map extracted function arguments back to original values
static void analyzeTaskArguments(llvm::Function *extractedFunc,
                                 llvm::Function *originalMain,
                                 TaskArgumentInfo &info,
                                 llvm::LLVMContext &Ctx)
{
    if (!extractedFunc || !originalMain)
        return;

    // Find the call site where the extracted function is called
    // CodeExtractor inserts this call in the original function
    llvm::CallInst *callSite = nullptr;
    for (llvm::BasicBlock &BB : *originalMain)
    {
        for (llvm::Instruction &I : BB)
        {
            if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&I))
            {
                if (CI->getCalledFunction() == extractedFunc)
                {
                    callSite = CI;
                    break;
                }
            }
        }
        if (callSite)
            break;
    }

    if (!callSite)
    {
        llvm::nulls() << "Warning: Could not find call site for " << extractedFunc->getName() << "\n";
        return;
    }

    // Extract actual argument values from the call site
    for (unsigned i = 0; i < callSite->arg_size(); ++i)
    {
        llvm::Value *arg = callSite->getArgOperand(i);
        info.originalValues.push_back(arg);
        info.types.push_back(arg->getType());
    }

    // Create struct type to hold all arguments
    if (!info.types.empty())
    {
        info.argStructType = llvm::StructType::create(
            Ctx, info.types,
            extractedFunc->getName().str() + "_args");
    }
}

static bool tdgDebugEnabled()
{
    /* Debug logging disabled.
    static int cached = -1;
    if (cached != -1)
        return cached != 0;

    const char *raw = std::getenv("SGPL_TDG_DEBUG");
    if (!raw || raw[0] == '\0')
    {
        cached = 0;
        return false;
    }

    std::string value(raw);
    cached = (value != "0" && value != "false" && value != "FALSE") ? 1 : 0;
    return cached != 0;
    */
    return false;
}

// global collector (keeps a single combined graph you can print later)
static llvm::dependencyGraph gCollected;

static unsigned nextID = 0;

void printInstruction(llvm::Instruction *I)
{
    // I->print(llvm::nulls()); // prints to stderr
}

std::pair<llvm::Instruction *, llvm::Instruction *> getKeyForValue(llvm::depEdge e, llvm::dependencyGraph &G)
{
    llvm::Instruction *src = nullptr;
    llvm::Instruction *dst = nullptr;

    for (auto &KV : G.nodes)
    {
        if (KV.second == e.source)
            src = llvm::dyn_cast<llvm::Instruction>(KV.first);
        if (KV.second == e.target)
            dst = llvm::dyn_cast<llvm::Instruction>(KV.first);
    }

    if (!src || !dst)
    {
        return {nullptr, nullptr};
    }

    return std::make_pair(src, dst);
}

static llvm::Instruction *getInstructionFromVertex(unsigned vertexId, const llvm::dependencyGraph &G)
{
    for (auto &entry : G.nodes)
    {
        if (entry.second == vertexId)
        {
            return entry.first;
        }
    }
    return nullptr;
}

namespace
{
    enum class DistanceProofKind
    {
        ProvenZero,
        ProvenPositiveConstant,
        ProvenPositiveUnknown,
        ProvenNonPositiveConstant,
        ProvenNonZeroUnknownSign,
        Unknown
    };

    enum class CarrierKind
    {
        IntraIteration,
        ProvenLevel,
        Unknown
    };

    struct DistanceProof
    {
        DistanceProofKind kind = DistanceProofKind::Unknown;
        int64_t constantValue = 0;
    };

    struct CarrierInfo
    {
        CarrierKind kind = CarrierKind::Unknown;
        unsigned level = 0; // 1-based if ProvenLevel, earliest unknown prefix if Unknown
        DistanceProof carriedDistance;
    };

    struct LoopCarrierSummary
    {
        bool hasProvenCarriedDep = false;
        bool hasUnschedulableCarriedDep = false;
        bool hasUnknownAttributedDep = false;
        bool hasProofOfNoCarriedDeps = true;
        llvm::SmallVector<int64_t, 4> constantDistances;
    };

    static bool isSchedulableAtCarrier(const DistanceProof &Proof)
    {
        return Proof.kind == DistanceProofKind::ProvenPositiveConstant;
    }

    static std::string distanceProofToString(const DistanceProof &Proof)
    {
        switch (Proof.kind)
        {
        case DistanceProofKind::ProvenZero:
            return "zero";
        case DistanceProofKind::ProvenPositiveConstant:
            return ("positive constant distance=" + std::to_string(Proof.constantValue));
        case DistanceProofKind::ProvenPositiveUnknown:
            return "positive but unknown distance";
        case DistanceProofKind::ProvenNonPositiveConstant:
            return ("non-positive constant distance=" + std::to_string(Proof.constantValue));
        case DistanceProofKind::ProvenNonZeroUnknownSign:
            return "non-zero but unknown sign/distance";
        case DistanceProofKind::Unknown:
            return "unknown";
        }
        return "unknown";
    }

    static DistanceProof classifyDistanceComponent(const llvm::Dependence &Dep,
                                                   unsigned Level)
    {
        using namespace llvm;
        DistanceProof Proof;
        const SCEV *Dist = Dep.getDistance(Level);
        unsigned Dir = Dep.getDirection(Level);

        if (Dist)
        {
            if (const auto *C = dyn_cast<SCEVConstant>(Dist))
            {
                if (const auto *CI = dyn_cast<ConstantInt>(C->getValue()))
                {
                    Proof.constantValue = CI->getSExtValue();
                    if (Proof.constantValue == 0)
                    {
                        Proof.kind = DistanceProofKind::ProvenZero;
                    }
                    else if (Proof.constantValue > 0)
                    {
                        Proof.kind = DistanceProofKind::ProvenPositiveConstant;
                    }
                    else
                    {
                        Proof.kind = DistanceProofKind::ProvenNonPositiveConstant;
                    }
                    return Proof;
                }
            }
        }

        switch (Dir)
        {
        case Dependence::DVEntry::EQ:
            Proof.kind = DistanceProofKind::ProvenZero;
            return Proof;
        case Dependence::DVEntry::LT:
            Proof.kind = DistanceProofKind::ProvenPositiveUnknown;
            return Proof;
        case Dependence::DVEntry::GT:
            Proof.kind = DistanceProofKind::ProvenNonPositiveConstant;
            Proof.constantValue = -1;
            return Proof;
        case Dependence::DVEntry::NE:
            Proof.kind = DistanceProofKind::ProvenNonZeroUnknownSign;
            return Proof;
        case Dependence::DVEntry::LE:
        case Dependence::DVEntry::GE:
        case Dependence::DVEntry::ALL:
        case Dependence::DVEntry::NONE:
        default:
            Proof.kind = DistanceProofKind::Unknown;
            return Proof;
        }
    }

    // Bug to avoid:
    // It is incorrect to conclude "loop k is not DOALL if dk > 0" without
    // first proving d1..d(k-1) are all zero.
    static CarrierInfo proveCarrierForDependence(const llvm::Dependence &Dep,
                                                 llvm::Loop *L)
    {
        CarrierInfo Info;
        unsigned Depth = L->getLoopDepth();
        unsigned Levels = std::min<unsigned>(Depth, Dep.getLevels());

        // Only the component at THIS loop's own depth decides whether this loop
        // carries the dependence; components at shallower levels belong to
        // enclosing loops (they may be unknown/non-zero there without making
        // this loop sequential).
        if (Depth <= Levels)
        {
            DistanceProof Proof = classifyDistanceComponent(Dep, Depth);
            if (Proof.kind == DistanceProofKind::Unknown)
            {
                Info.kind = CarrierKind::Unknown;
                Info.level = Depth;
                return Info;
            }
            if (Proof.kind != DistanceProofKind::ProvenZero)
            {
                Info.kind = CarrierKind::ProvenLevel;
                Info.level = Depth;
                Info.carriedDistance = Proof;
                return Info;
            }
        }

        // Loop-independent w.r.t. this loop.
        Info.kind = CarrierKind::IntraIteration;
        return Info;
    }
}

void createEdge(llvm::Instruction *I1, llvm::Instruction *I2, llvm::dependencyGraph &G, std::string type)
{
    llvm::depEdge edge;
    // printInstruction(I1);
    // llvm::nulls() << "\n";

    unsigned source = G.nodes[I1];
    unsigned target = G.nodes[I2];

    edge.source = source;
    edge.target = target;

    for (auto &e : G.edges)
    {
        if (e.source == edge.source && e.target == edge.target)
        {
            if (e.type == type)
            {
                return;
            }
            else
            {
                e.type = e.type + "_" + type;
                return;
            }
        }
    }
    edge.type = type;
    G.edges.push_back(edge);

    // if (gCollected.nodes.find(I1) == gCollected.nodes.end())
    //     gCollected.nodes[I1] = edge.source;
    // if (gCollected.nodes.find(I2) == gCollected.nodes.end())
    //     gCollected.nodes[I2] = edge.target;
    // gCollected.edges.push_back(edge);
    // llvm::nulls()() << "EDGE: ";
    // I1->print(llvm::nulls());
    // llvm::nulls()() << " ----> ";
    // I2->print(llvm::nulls());

    // llvm::nulls() << " (" << edge.type << ")\n";
}

void populateNodes(llvm::Instruction *I1, llvm::dependencyGraph &G)
{
    auto it = G.nodes.find(I1);
    if (it == G.nodes.end()) // if I1 is not in the nodeset
    {
        G.nodes[I1] = nextID; // map node to int and insert into nodeset
        nextID++;
    }
}

void addMemoryBarrier(llvm::dependencyGraph &G, llvm::Module &M)
{
    if (G.memoryBarrier != nullptr)
    {
        return; // Already created
    }

    // Create a dummy function to hold the barrier instruction
    llvm::LLVMContext &Ctx = M.getContext();
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx), false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::GlobalValue::PrivateLinkage,
                                               "__pdg_memory_barrier", &M);
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(Ctx, "entry", F);

    // Create an unreachable instruction as the barrier marker
    G.memoryBarrier = new llvm::UnreachableInst(Ctx, BB);

    // Add to nodes map
    populateNodes(G.memoryBarrier, G);
}

void addControlBarrier(llvm::dependencyGraph &G, llvm::Module &M, std::string id)
{

    // Create a dummy function to hold the barrier instruction
    llvm::LLVMContext &Ctx = M.getContext();
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx), false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::GlobalValue::PrivateLinkage,
                                               "_control_barrier_" + id, &M);
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(Ctx, "entry", F);

    // Create an unreachable instruction as the barrier marker
    llvm::Instruction *controlBarrier = new llvm::UnreachableInst(Ctx, BB);
    G.controlBarriers.push_back(controlBarrier);

    // Add to nodes map
    populateNodes(controlBarrier, G);
}

static std::string escapeForDot(const std::string &s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        if (c == '"')
            out.push_back('\'');
        else if (c == '\n' || c == '\r')
            out += "\\n";
        else
            out.push_back(c);
    }
    return out;
}

// void printCollectedGraphDot(llvm::dependencyGraph &G,
//                             const std::string &filename = "pdg_global.dot")
// {
//     std::ofstream ofs(filename);
//     if (!ofs.is_open())
//     {
//         llvm::nulls() << "Failed to open " << filename << " for writing\n";
//         return;
//     }
//     ofs << "digraph PDG {\n";

//     // helper for stable node name
//     auto nodeName = [](int id)
//     {
//         return std::string("n") + std::to_string(id);
//     };

//     // nodes: emit stable names as identifiers and instruction text as label
//     for (auto &entry : G.nodes)
//     {
//         llvm::Instruction *I = entry.first;
//         int id = entry.second;
//         std::string s;

//         // Check if this is a barrier node
//         if (I == G.memoryBarrier)
//         {
//             s = "MEMORY_BARRIER";
//             ofs << "  \"" << nodeName(id) << "\" [label=\"" << s
//                 << "\", shape=box, style=filled, fillcolor=lightblue];\n";
//         }
//         else
//         {
//             llvm::raw_string_ostream rso(s);
//             I->print(rso);
//             rso.flush();
//             ofs << "  \"" << nodeName(id) << "\" [label=\"" << escapeForDot(s) << "\"];\n";
//         }
//     }

//     // edges: reference the same stable names
//     for (auto &e : G.edges)
//     {
//         ofs << "  \"" << nodeName(e.source) << "\" -> \"" << nodeName(e.target)
//             << "\" [label=\"" << escapeForDot(e.type) << "\"];\n";
//     }

//     ofs << "}\n";
//     ofs.close();
//     llvm::nulls() << "Wrote " << filename << "\n";
// }

static bool loopHasTerminatorMetadata(const llvm::Loop *L, llvm::StringRef Name)
{
    return L && L->getHeader() && L->getHeader()->getTerminator() &&
           L->getHeader()->getTerminator()->getMetadata(Name);
}

static void markNestedLoopsSequential(llvm::Loop *L)
{
    if (!L)
        return;
    llvm::LLVMContext &Ctx = L->getHeader()->getContext();
    for (llvm::Loop *SubLoop : L->getSubLoops())
    {
        if (SubLoop && SubLoop->getHeader() && SubLoop->getHeader()->getTerminator())
        {
            SubLoop->getHeader()->getTerminator()->setMetadata(
                "sgpl.frontier.nested.sequential",
                llvm::MDNode::get(Ctx, llvm::MDString::get(Ctx, "required")));
        }
        markNestedLoopsSequential(SubLoop);
    }
}

static std::string analyzeAndAnnotateLoop(llvm::Loop *L, llvm::Function &F,
                                          llvm::DependenceInfo &DI,
                                          llvm::ScalarEvolution &SE,
                                          llvm::dependencyGraph &G)
{
    using namespace llvm;
    SmallVector<Instruction *, 32> memInsts;

    // collect loads/stores inside the loop and ensure PDG nodes exist

    for (BasicBlock *BB : L->blocks())
    {
        for (Instruction &I : *BB)
        {
            if ((isa<LoadInst>(&I) || isa<StoreInst>(&I)) ||
                (isa<PHINode>(&I) && any_of(I.uses(), [](const Use &U)
                                            { return isa<StoreInst>(U.getUser()); })))
            {
                memInsts.push_back(&I);
                populateNodes(&I, G);
            }
        }
    }

    bool phiCarry = false;
    LoopCarrierSummary Summary;

    // Conservative memory pairing: accesses to the same underlying object may
    // alias regardless of what AA concludes. AA's SCEV reasoning can declare
    // GEP(phi) vs GEP(phi+1) NoAlias (hiding a distance-1 loop-carried
    // dependence), and MemorySSA clobbering then never links the store to the
    // load. Pair them explicitly and let DependenceInfo decide the distance.
    for (Instruction *A : memInsts)
    {
        auto *SI = dyn_cast<StoreInst>(A);
        if (!SI)
            continue;
        Value *ObjS = getUnderlyingObject(SI->getPointerOperand());
        for (Instruction *B : memInsts)
        {
            auto *LI = dyn_cast<LoadInst>(B);
            if (!LI)
                continue;
            if (getUnderlyingObject(LI->getPointerOperand()) == ObjS)
                createEdge(SI, LI, G, "RAW_MAY");
        }
    }
    for (Instruction *A : memInsts)
    {
        auto *SI = dyn_cast<StoreInst>(A);
        if (!SI)
            continue;
        Value *ObjS = getUnderlyingObject(SI->getPointerOperand());
        for (Instruction *B : memInsts)
        {
            auto *SI2 = dyn_cast<StoreInst>(B);
            if (!SI2 || SI2 == SI)
                continue;
            if (getUnderlyingObject(SI2->getPointerOperand()) == ObjS)
                createEdge(SI, SI2, G, "WAW_MAY");
        }
    }
    auto printInstToStderr = [](llvm::Instruction *Inst)
    {
        llvm::raw_os_ostream OS(std::cerr);
        Inst->print(OS);
        OS.flush();
    };
    auto logLoopClassify = [&](StringRef Msg)
    {
        (void)Msg;
        /* Debug logging disabled. Restore std::cerr logging here to re-enable loop-classify traces. */
    };

    // --- PHI handling: only mark phiCarry when the PHI is NOT an induction variable (SCEV AddRec) ---

    if (BasicBlock *Header = L->getHeader())
    {
        for (Instruction &I : *Header)
        {
            if (PHINode *PN = dyn_cast<PHINode>(&I))
            {
                // Use ScalarEvolution to decide if PN is a canonical induction (AddRec for this loop).
                const SCEV *PS = SE.getSCEV(PN);
                if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(PS))
                {
                    // If this AddRec is for this loop, it's an induction recurrence — ignore.
                    if (AR->getLoop() == L)
                    {
                        // canonical induction variable -> do not treat as carried scalar that forces sequential
                        continue;
                    }
                }

                for (unsigned k = 0, n = PN->getNumIncomingValues(); k < n; ++k)
                {
                    BasicBlock *IncBB = PN->getIncomingBlock(k);
                    if (IncBB && L->contains(IncBB) && IncBB != L->getLoopPreheader())
                    {
                        phiCarry = true;
                        logLoopClassify("phi-carry detected on non-induction phi");
                        /* Debug logging disabled: phi print */
                        break;
                    }
                }
                if (phiCarry)
                    break;
            }
            else
            {
                break;
            }
        }
    }

    // 33

    for (auto E : G.edges)
    {
        std::pair<llvm::Instruction *, llvm::Instruction *> edge = getKeyForValue(E, G);

        // // llvm::nulls()() << edge.first << " " << edge.second << "\n";

        if (L->contains(edge.first->getParent()) && L->contains(edge.second->getParent()))
        {

            std::unique_ptr<Dependence> Dep = DI.depends(edge.first, edge.second, /*AllowTrans=*/false);
            if (!Dep)
                continue;

            // edge.first->print(llvm::nulls());
            // edge.second->print(llvm::nulls());
            // Dep->dump(llvm::nulls());

            // if (!Dep->isFlow())
            // {
            //     continue;
            // }
            if (Dep->isLoopIndependent())
            {
                continue;
            }
            CarrierInfo Info = proveCarrierForDependence(*Dep, L);
            if (Info.kind == CarrierKind::IntraIteration)
            {
                logLoopClassify("ignored intra-iteration dependence");
                continue;
            }

            /* Debug logging disabled: dependence src/dst print */

            if (Info.kind == CarrierKind::Unknown)
            {
                if (Info.level == L->getLoopDepth())
                {
                    Summary.hasUnknownAttributedDep = true;
                    Summary.hasProofOfNoCarriedDeps = false;
                    logLoopClassify((Twine("carrier=UNKNOWN earliest_unknown_prefix=") + Twine(Info.level) +
                                     " attributed to current loop")
                                        .str());
                }
                else
                {
                    logLoopClassify((Twine("ignored for loop depth=") + Twine(L->getLoopDepth()) +
                                     " because carrier=UNKNOWN at prefix=" + Twine(Info.level))
                                        .str());
                }
                continue;
            }

            if (Info.level != L->getLoopDepth())
            {
                logLoopClassify((Twine("ignored for loop depth=") + Twine(L->getLoopDepth()) +
                                 " because carrier=" + Twine(Info.level))
                                    .str());
                continue;
            }

            Summary.hasProvenCarriedDep = true;
            Summary.hasProofOfNoCarriedDeps = false;
            logLoopClassify((Twine("carrier=") + Twine(Info.level) + " " +
                             distanceProofToString(Info.carriedDistance))
                                .str());

            if (!isSchedulableAtCarrier(Info.carriedDistance))
            {
                Summary.hasUnschedulableCarriedDep = true;
                continue;
            }

            Summary.constantDistances.push_back(Info.carriedDistance.constantValue);
            llvm::LLVMContext &Ctx = F.getContext();
            int srcID = G.nodes[edge.first];
            llvm::Metadata *IDVal = llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), srcID));
            llvm::MDNode *IDNode = llvm::MDNode::get(Ctx, IDVal);

            llvm::MDNode *postNode = llvm::MDNode::get(
                Ctx, llvm::MDString::get(Ctx, "doacross.post"));
            edge.first->setMetadata("doacross.post", postNode);
            edge.first->setMetadata("doacross.id", IDNode);

            llvm::MDNode *waitNode = llvm::MDNode::get(
                Ctx, llvm::MDString::get(Ctx, "doacross.wait"));
            Metadata *DistVal = ConstantAsMetadata::get(
                ConstantInt::get(Type::getInt64Ty(Ctx), Info.carriedDistance.constantValue));
            MDNode *DistNode = MDNode::get(Ctx, DistVal);
            edge.second->setMetadata("doacross.wait", waitNode);
            edge.second->setMetadata("doacross.dist", DistNode);
            edge.second->setMetadata("doacross.src", IDNode);
        }
    }

    // for (size_t i = 0; i < memInsts.size(); ++i)
    // {
    //     for (size_t j = 0; j < memInsts.size(); ++j)
    //     {
    //         if (i == j)
    //             continue;
    //         Instruction *Src = memInsts[i];
    //         Instruction *Dst = memInsts[j];
    //         llvm::nulls() << "triggering ";
    //         std::unique_ptr<Dependence> Dep = DI.depends(Src, Dst, /*AllowTrans=*/false);
    //         if (!Dep)
    //             continue;

    //         // debug
    //         llvm::nulls() << "Dependence between: ";
    //         Src->print(llvm::nulls());
    //         llvm::nulls() << " -> ";
    //         Dst->print(llvm::nulls());
    //         llvm::nulls() << "\n";
    //         Dep->dump(llvm::nulls());
    //         if (!Dep->isFlow())
    //             continue;
    //         if (Dep->isLoopIndependent())
    //         {
    //             // same-iteration dependence not relevant to across-iteration classification
    //             continue;
    //         }

    //         sawLoopCarried = true;

    //         // require distance info at innermost level (level 0)
    //         const SCEV *Dist = Dep->getDistance(1); // innermost
    //         if (!Dist)
    //         {
    //             anyUnknown = true;
    //             allCarriedHaveConstantPositiveDistance = false;
    //             llvm::nulls() << "  -> distance unknown\n";
    //             createEdge(Src, Dst, G, "DEP");
    //             continue;
    //         }

    //         // Print SCEV for debug
    //         llvm::nulls() << "  -> Distance SCEV: ";
    //         Dist->print(llvm::nulls());
    //         llvm::nulls() << "\n";

    //         if (const SCEVConstant *C = dyn_cast<SCEVConstant>(Dist))
    //         {
    //             if (const ConstantInt *CI = dyn_cast<ConstantInt>(C->getValue()))
    //             {
    //                 int64_t d = CI->getSExtValue();
    //                 llvm::nulls() << "  -> constant distance = " << d << "\n";
    //                 if (d == 0)
    //                 {
    //                     hasZeroDistance = true;
    //                     allCarriedHaveConstantPositiveDistance = false;
    //                     llvm::nulls() << "    -> zero distance => sequential\n";
    //                 }
    //                 else if (d < 1)
    //                 {
    //                     hasNonPositiveDistance = true;
    //                     allCarriedHaveConstantPositiveDistance = false;
    //                     llvm::nulls() << "    -> non-positive distance => sequential/unsupported for DOACROSS\n";
    //                 }
    //             }
    //             else
    //             {
    //                 anyUnknown = true;
    //                 allCarriedHaveConstantPositiveDistance = false;
    //                 llvm::nulls() << "  -> SCEVConstant but not ConstantInt -> unknown\n";
    //             }
    //         }
    //         else
    //         {
    //             allCarriedHaveConstantPositiveDistance = false;
    //             llvm::nulls() << "  -> distance not a SCEVConstant (non-constant) => treat as unknown/non-DOACROSS\n";
    //         }

    //         createEdge(Src, Dst, G, "DEP");
    //     }
    // }

    // final classification
    const bool IsVerifiedFrontier =
        loopHasTerminatorMetadata(L, "sgpl.frontier.first_wins.candidate");
    const bool IsNestedFrontierLoop =
        loopHasTerminatorMetadata(L, "sgpl.frontier.nested.sequential");

    std::string classification;
    if (IsNestedFrontierLoop)
    {
        classification = "SEQUENTIAL";
    }
    else if (IsVerifiedFrontier)
    {
        classification = "DOALL";
        markNestedLoopsSequential(L);
    }
    else if (phiCarry)
    {
        classification = "SEQUENTIAL";
    }
    else if (Summary.hasProofOfNoCarriedDeps)
    {
        classification = "DOALL";
    }
    else if (Summary.hasUnschedulableCarriedDep || Summary.hasUnknownAttributedDep)
    {
        classification = "SEQUENTIAL";
    }
    else
    {
        classification = "DOACROSS";
    }
    // llvm::nulls()() << "\n\n\nLoop header ";
    // llvm::nulls()() << " classified as " << classification << "\n";

    // attach tiny custom metadata to loop latches so we can inspect IR
    LLVMContext &Ctx = F.getContext();
    MDNode *Node = MDNode::get(Ctx, MDString::get(Ctx, std::string("parallel.type=") + classification));

    llvm::BasicBlock *NN = L->getHeader();
    if (NN)
    {
        if (llvm::Instruction *MM = NN->getTerminator())
        {
            MM->setMetadata("my.loop.parallel", Node);
            if (IsVerifiedFrontier)
            {
                MM->setMetadata(
                    "sgpl.frontier.first_wins.doall",
                    MDNode::get(Ctx, MDString::get(Ctx, "requires-int-append-priv")));
            }
        }
    };

    logLoopClassify((Twine("classification=") + classification +
                     " hasProvenCarriedDep=" + Twine(Summary.hasProvenCarriedDep ? 1 : 0) +
                     " hasUnschedulableCarriedDep=" + Twine(Summary.hasUnschedulableCarriedDep ? 1 : 0) +
                     " hasUnknownAttributedDep=" + Twine(Summary.hasUnknownAttributedDep ? 1 : 0) +
                     " hasProofOfNoCarriedDeps=" + Twine(Summary.hasProofOfNoCarriedDeps ? 1 : 0) +
                     " phiCarry=" + Twine(phiCarry ? 1 : 0))
                        .str());

    // llvm::nulls()() << "Loop header ";
    // L->getHeader()->printAsOperand(llvm::nulls(), false);
    // llvm::nulls() << " classified as " << classification << "\n";

    return classification;
}

static std::string analyzeAndAnnotateLoop(llvm::Loop *L, llvm::Function &F,
                                          llvm::DependenceInfo &DI,
                                          llvm::ScalarEvolution &SE,
                                          llvm::dependencyGraph &G);

static bool getLoopParallelClass(const llvm::Loop *L, std::string &ParallelClass)
{
    if (!L || !L->getHeader())
        return false;

    llvm::Instruction *Term = L->getHeader()->getTerminator();
    if (!Term)
        return false;

    llvm::MDNode *LoopMD = Term->getMetadata("my.loop.parallel");
    if (!LoopMD)
        return false;

    for (unsigned i = 0; i < LoopMD->getNumOperands(); ++i)
    {
        auto *MDS = llvm::dyn_cast<llvm::MDString>(LoopMD->getOperand(i));
        if (!MDS)
            continue;

        llvm::StringRef S = MDS->getString();
        if (!S.starts_with("parallel.type="))
            continue;

        ParallelClass = S.substr(strlen("parallel.type=")).str();
        return true;
    }

    return false;
}

static const llvm::dependencyGraph::LoopRegionInfo *findLoopRegionInfo(
    const llvm::dependencyGraph &G,
    unsigned LoopRegionId)
{
    for (const auto &Region : G.loopRegions)
    {
        if (Region.loopRegionId == LoopRegionId)
            return &Region;
    }
    return nullptr;
}

static unsigned countNestedSubloops(llvm::Loop *L)
{
    unsigned Count = 0;
    for (llvm::Loop *SubLoop : L->getSubLoops())
    {
        Count += 1;
        Count += countNestedSubloops(SubLoop);
    }
    return Count;
}

static void collectTopLevelLoopRegions(llvm::Function &F,
                                       llvm::LoopInfo &LI,
                                       llvm::dependencyGraph &G)
{
    unsigned NextLoopRegionId = G.loopRegions.size();
    for (llvm::Loop *TopLoop : LI)
    {
        llvm::dependencyGraph::LoopRegionInfo Region;
        Region.loopRegionId = NextLoopRegionId++;
        Region.loop = TopLoop;
        Region.header = TopLoop->getHeader();
        Region.preheader = TopLoop->getLoopPreheader();
        Region.latch = TopLoop->getLoopLatch();
        Region.exitBlock = TopLoop->getExitBlock();
        getLoopParallelClass(TopLoop, Region.parallelClass);
        Region.isExtractionRegion = true;
        Region.nestedSubloopCount = countNestedSubloops(TopLoop);
        Region.blocks.append(TopLoop->block_begin(), TopLoop->block_end());

        bool OverlapDetected = false;
        for (llvm::BasicBlock *BB : Region.blocks)
        {
            if (G.blockToLoopRegion.count(BB))
            {
                OverlapDetected = true;
                /* Debug logging disabled: overlapping loop region */
                break;
            }
        }
        if (OverlapDetected)
            continue;

        G.loopRegions.push_back(Region);

        /* Debug logging disabled: created top-level loop region */

        for (llvm::BasicBlock *BB : Region.blocks)
        {
            G.blockToLoopRegion[BB] = Region.loopRegionId;
            for (llvm::Instruction &I : *BB)
            {
                unsigned VertexId = G.nodes.lookup(&I);
                if (!VertexId)
                    continue;
                if (G.vertexToLoopRegion.count(VertexId))
                {
                    /* Debug logging disabled: overlapping loop vertex */
                    continue;
                }
                G.vertexToLoopRegion[VertexId] = Region.loopRegionId;
            }
        }
    }
}

void buildGraph(llvm::Function &F,
                llvm::DependenceInfo &DI,
                llvm::LoopInfo &LI,
                llvm::ScalarEvolution &SE,
                llvm::dependencyGraph &G,
                llvm::MemorySSA &MSSA,
                llvm::AAResults &AA,
                llvm::DominatorTree &DT,
                llvm::PostDominatorTree &PDT)
{

    llvm::SmallVector<llvm::LoadInst *> loadCollector;
    llvm::SmallVector<llvm::StoreInst *> storeCollector;
    llvm::SmallVector<llvm::Instruction *> barrierInsts;

    // data dependency
    if (F.isDeclaration())
        return;
    for (llvm::BasicBlock &BB : F)
    {
        for (llvm::Instruction &I : BB)
        {
            populateNodes(&I, G);
        }
    }
    for (llvm::BasicBlock &BB : F)
    {
        for (llvm::Instruction &I : BB)
        {

            if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(&I))
                loadCollector.push_back(LI);
            if (llvm::StoreInst *SI = llvm::dyn_cast<llvm::StoreInst>(&I))
                storeCollector.push_back(SI);
            // printInstruction(&I);
            // llvm::nulls() << " ";
            // if (I.use_empty())
            // {
            //     // llvm::nulls()() << "NO EDGE\n";
            // }
            for (llvm::Value *U : I.users())
            {
                if (auto *UI = llvm::dyn_cast<llvm::Instruction>(U))
                {
                    // printInstruction(UI);
                    // llvm::nulls() << "\n";
                    createEdge(&I, UI, G, "RAW");
                }
            }

            // Handle PHI node incoming values (critical for loop-carried dependencies)
            if (llvm::PHINode *Phi = llvm::dyn_cast<llvm::PHINode>(&I))
            {
                for (unsigned i = 0; i < Phi->getNumIncomingValues(); ++i)
                {
                    if (llvm::Instruction *Inc = llvm::dyn_cast<llvm::Instruction>(Phi->getIncomingValue(i)))
                    {
                        createEdge(Inc, Phi, G, "PHI_INCOMING");
                    }
                }
            }

            // Handle call instructions - add them to barriers and ensure argument dependencies
            if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&I))
            {
                // Exempt argmemonly calls from barrier treatment. Such calls only
                // access memory through pointer arguments, so they don't create
                // function-wide memory barriers. This lets neighbor iteration
                // calls (init/next) coexist with loop parallelization.
                // Argument dependency edges are still created.
                bool isArgMemOnly = false;
                if (llvm::Function *Callee = CI->getCalledFunction())
                    isArgMemOnly = Callee->onlyAccessesArgMemory();

                if (!isArgMemOnly)
                    barrierInsts.push_back(CI);

                // Ensure all arguments are connected (though def-use should catch this)
                for (llvm::Use &Arg : CI->args())
                {
                    if (llvm::Instruction *ArgI = llvm::dyn_cast<llvm::Instruction>(Arg.get()))
                    {
                        createEdge(ArgI, CI, G, "CALL_ARG");
                    }
                }
            }
        }
    }

    auto *Walker = MSSA.getWalker();

    // Memory dependencies
    for (auto LI : loadCollector)
    {

        if (llvm::isa<llvm::CallInst>(LI) || llvm::isa<llvm::FenceInst>(LI) || llvm::isa<llvm::AtomicRMWInst>(LI) || llvm::isa<llvm::AtomicCmpXchgInst>(LI) || LI->isVolatile() || LI->isAtomic())
        {
            barrierInsts.push_back(LI);
        }

        if (llvm::MemoryAccess *MA = MSSA.getMemoryAccess(LI))
        {
            if (auto *Clobber = Walker->getClobberingMemoryAccess(MA))
            {
                if (auto *MD = llvm::dyn_cast<llvm::MemoryDef>(Clobber))
                {
                    if (auto *SI = llvm::dyn_cast_or_null<llvm::StoreInst>(MD->getMemoryInst()))
                    {
                        createEdge(SI, LI, G, "RAW");
                    }
                }
            }

            for (auto SI : storeCollector)
            {
                if (llvm::isa<llvm::CallInst>(SI) || llvm::isa<llvm::FenceInst>(SI) || llvm::isa<llvm::AtomicRMWInst>(SI) || llvm::isa<llvm::AtomicCmpXchgInst>(SI) || SI->isVolatile() || SI->isAtomic())
                {
                    barrierInsts.push_back(SI);
                }
                llvm::MemoryAccess *StoreMA = MSSA.getMemoryAccess(SI);
                if (!StoreMA)
                    continue;

                for (auto SI2 : storeCollector)
                {
                    if (SI == SI2)
                        continue;
                    llvm::MemoryAccess *StoreMA2 = MSSA.getMemoryAccess(SI2);
                    if (!StoreMA2)
                        continue;

                    // if (AA.alias(SI, SI->getValueOperand()->getType()->getScalarSizeInBits() / 8, SI2, SI2->getValueOperand()->getType()->getScalarSizeInBits() / 8) != llvm::AliasResult::NoAlias)
                    // {
                    //     createEdge(SI, SI2, G, "WAW-may");
                    // }
                    if (AA.alias(SI, SI->getValueOperand()->getType()->getScalarSizeInBits() / 8, SI2, SI2->getValueOperand()->getType()->getScalarSizeInBits() / 8) != llvm::AliasResult::NoAlias)
                    {
                        if (auto *MA1 = MSSA.getMemoryAccess(SI))
                            if (auto *MA2 = MSSA.getMemoryAccess(SI2))
                            {
                                if (MSSA.dominates(MA1, MA2))
                                    createEdge(SI, SI2, G, "WAW");
                                else if (MSSA.dominates(MA2, MA1))
                                    createEdge(SI2, SI, G, "WAW");
                            }
                    }
                }
            }

            // WAR (Write-After-Read) dependencies
            for (auto SI : storeCollector)
            {
                llvm::MemoryAccess *StoreMA = MSSA.getMemoryAccess(SI);
                if (!StoreMA)
                    continue;

                // Check if this store may alias with the load
                if (AA.alias(LI, LI->getType()->getScalarSizeInBits() / 8,
                             SI, SI->getValueOperand()->getType()->getScalarSizeInBits() / 8) != llvm::AliasResult::NoAlias)
                {
                    // Check ordering: Load should happen before Store
                    if (auto *MA1 = MSSA.getMemoryAccess(LI))
                        if (auto *MA2 = MSSA.getMemoryAccess(SI))
                        {
                            if (MSSA.dominates(MA1, MA2))
                                createEdge(LI, SI, G, "WAR");
                            else if (MSSA.dominates(MA2, MA1))
                                createEdge(SI, LI, G, "WAR");
                        }
                }
            }
        }
    }

    llvm::SmallVector<llvm::BranchInst *> brInsts;

    // Control dependencies
    for (llvm::BasicBlock &BB : F)
    {

        if (auto *BI = llvm::dyn_cast<llvm::BranchInst>(BB.getTerminator()))
        {
            if (BI->isConditional())
            {
                brInsts.push_back(BI);
            }
        }
    }

    for (auto *bi : brInsts)
    {
        if (!bi->isConditional())
            continue;

        // Create control nodes ONCE per branch
        addControlBarrier(G, *F.getParent(), "ctrl_then_" + std::to_string(nextID));
        auto *CtrlThen = G.controlBarriers.back();
        addControlBarrier(G, *F.getParent(), "ctrl_else_" + std::to_string(nextID));
        auto *CtrlElse = G.controlBarriers.back();

        // Connect branch to control barriers
        createEdge(bi, CtrlThen, G, "CTRL_BRANCH");
        createEdge(bi, CtrlElse, G, "CTRL_BRANCH");

        llvm::BasicBlock *ThenBB = bi->getSuccessor(0);
        llvm::BasicBlock *ElseBB = bi->getSuccessor(1);

        for (auto &BB2 : F)
        {
            for (auto &I : BB2)
            {
                // Control dependence condition
                if (!DT.dominates(bi, &I))
                    continue;
                if (PDT.dominates(&I, bi))
                    continue;

                // Attach to the correct side
                if (DT.dominates(ThenBB, I.getParent()))
                {
                    createEdge(CtrlThen, &I, G, "CTRL_DEP");
                }
                else if (DT.dominates(ElseBB, I.getParent()))
                {
                    createEdge(CtrlElse, &I, G, "CTRL_DEP");
                }
            }
        }
    }

    // Memory barrier handling
    // for (auto bar : barrierInsts)
    // {
    //     createEdge(bar, G.memoryBarrier, G, "MEM_BARRIER");
    //     createEdge(G.memoryBarrier, bar, G, "MEM_BARRIER");
    // }

    for (auto *I : barrierInsts)
    {
        for (auto &BB : F)
        {
            for (auto &J : BB)
            {
                if (&J == I)
                    continue;

                if (DT.dominates(&J, I))
                {
                    // J happens before barrier
                    createEdge(&J, I, G, "MEM_BARRIER_BEFORE");
                }
                else if (DT.dominates(I, &J))
                {
                    // J happens after barrier
                    createEdge(I, &J, G, "MEM_BARRIER_AFTER");
                }
            }
        }
    }

    for (auto LIIt = LI.begin(), LIE = LI.end(); LIIt != LIE; ++LIIt)
    {
        llvm::Loop *TopL = *LIIt;
        analyzeAndAnnotateLoop(TopL, F, DI, SE, G);
        // recurse into subloops
        llvm::SmallVector<llvm::Loop *, 8> worklist;
        for (llvm::Loop *SL : TopL->getSubLoops())
            worklist.push_back(SL);
        while (!worklist.empty())
        {
            llvm::Loop *L = worklist.pop_back_val();
            analyzeAndAnnotateLoop(L, F, DI, SE, G);
            for (llvm::Loop *SL : L->getSubLoops())
                worklist.push_back(SL);
        }
    }

    collectTopLevelLoopRegions(F, LI, G);
}

// void buildGraph(llvm::Function &F,
//                 llvm::DependenceInfo &DI,
//                 llvm::LoopInfo &LI,
//                 llvm::ScalarEvolution &SE,
//                 llvm::dependencyGraph &G,
//                 llvm::MemorySSA &MSSA,
//                 llvm::AAResults &AA,
//                 llvm::DominatorTree &DT,
//                 llvm::PostDominatorTree &PDT)
// {

//     llvm::SmallVector<llvm::LoadInst *> loadCollector;
//     llvm::SmallVector<llvm::StoreInst *> storeCollector;

//     // data dependency
//     if (F.isDeclaration())
//         return;
//     for (llvm::BasicBlock &BB : F)
//     {
//         for (llvm::Instruction &I : BB)
//         {
//             populateNodes(&I, G);
//         }
//     }
//     for (llvm::BasicBlock &BB : F)
//     {
//         for (llvm::Instruction &I : BB)
//         {

//             if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(&I))
//                 loadCollector.push_back(LI);
//             if (llvm::StoreInst *SI = llvm::dyn_cast<llvm::StoreInst>(&I))
//                 storeCollector.push_back(SI);
//             // printInstruction(&I);
//             // llvm::nulls() << " ";
//             // if (I.use_empty())
//             // {
//             //     // llvm::nulls()() << "NO EDGE\n";
//             // }
//             for (llvm::Value *U : I.users())
//             {
//                 if (auto *UI = llvm::dyn_cast<llvm::Instruction>(U))
//                 {
//                     // printInstruction(UI);
//                     // llvm::nulls() << "\n";
//                     createEdge(&I, UI, G, "RAW");
//                 }
//             }
//         }
//     }

//     auto *Walker = MSSA.getWalker();
//     llvm::SmallVector<llvm::Instruction *> barrierInsts;

//     // Memory dependencies
//     for (auto LI : loadCollector)
//     {

//         if (llvm::isa<llvm::CallInst>(LI) || llvm::isa<llvm::FenceInst>(LI) || llvm::isa<llvm::AtomicRMWInst>(LI) || llvm::isa<llvm::AtomicCmpXchgInst>(LI) || LI->isVolatile() || LI->isAtomic())
//         {
//             barrierInsts.push_back(LI);
//         }

//         if (llvm::MemoryAccess *MA = MSSA.getMemoryAccess(LI))
//         {
//             if (auto *Clobber = Walker->getClobberingMemoryAccess(MA))
//             {
//                 if (auto *MD = llvm::dyn_cast<llvm::MemoryDef>(Clobber))
//                 {
//                     if (auto *SI = llvm::dyn_cast_or_null<llvm::StoreInst>(MD->getMemoryInst()))
//                     {
//                         createEdge(SI, LI, G, "RAW");
//                     }
//                 }
//             }

//             for (auto SI : storeCollector)
//             {
//                 if (llvm::isa<llvm::CallInst>(SI) || llvm::isa<llvm::FenceInst>(SI) || llvm::isa<llvm::AtomicRMWInst>(SI) || llvm::isa<llvm::AtomicCmpXchgInst>(SI) || SI->isVolatile() || SI->isAtomic())
//                 {
//                     barrierInsts.push_back(SI);
//                 }
//                 llvm::MemoryAccess *StoreMA = MSSA.getMemoryAccess(SI);
//                 if (!StoreMA)
//                     continue;

//                 for (auto SI2 : storeCollector)
//                 {
//                     if (SI == SI2)
//                         continue;
//                     llvm::MemoryAccess *StoreMA2 = MSSA.getMemoryAccess(SI2);
//                     if (!StoreMA2)
//                         continue;

//                     // if (AA.alias(SI, SI->getValueOperand()->getType()->getScalarSizeInBits() / 8, SI2, SI2->getValueOperand()->getType()->getScalarSizeInBits() / 8) != llvm::AliasResult::NoAlias)
//                     // {
//                     //     createEdge(SI, SI2, G, "WAW-may");
//                     // }
//                     if (AA.alias(SI, SI->getValueOperand()->getType()->getScalarSizeInBits() / 8, SI2, SI2->getValueOperand()->getType()->getScalarSizeInBits() / 8) != llvm::AliasResult::NoAlias)
//                     {
//                         if (auto *MA1 = MSSA.getMemoryAccess(SI))
//                             if (auto *MA2 = MSSA.getMemoryAccess(SI2))
//                             {
//                                 if (MSSA.dominates(MA1, MA2))
//                                     createEdge(SI, SI2, G, "WAW");
//                                 else if (MSSA.dominates(MA2, MA1))
//                                     createEdge(SI2, SI, G, "WAW");
//                             }
//                     }
//                 }
//             }
//         }
//     }

//     llvm::SmallVector<llvm::BranchInst *> brInsts;

//     // Control dependencies
//     for (llvm::BasicBlock &BB : F)
//     {

//         if (auto *BI = dyn_cast<BranchInst>(BB.getTerminator()))
//         {
//             if (BI->isConditional())
//             {
//                 brInsts.push_back(BI);
//             }
//         }
//     }

//     for (auto *bi : brInsts)
//     {
//         if (!bi->isConditional())
//             continue;

//         // Create control nodes ONCE per branch
//         addControlBarrier(G, *F.getParent(), "ctrl_then_" + nextID);
//         auto *CtrlThen = G.controlBarriers.back();
//         addControlBarrier(G, *F.getParent(), "ctrl_else_" + nextID);
//         auto *CtrlElse = G.controlBarriers.back();

//         BasicBlock *ThenBB = bi->getSuccessor(0);
//         BasicBlock *ElseBB = bi->getSuccessor(1);

//         for (auto &BB2 : F)
//         {
//             for (auto &I : BB2)
//             {
//                 // Control dependence condition
//                 if (!DT.dominates(bi, &I))
//                     continue;
//                 if (PDT.dominates(&I, bi))
//                     continue;

//                 // Attach to the correct side
//                 if (DT.dominates(ThenBB, I.getParent()))
//                 {
//                     createEdge(CtrlThen, &I, G, "CTRL_DEP");
//                 }
//                 else if (DT.dominates(ElseBB, I.getParent()))
//                 {
//                     createEdge(CtrlElse, &I, G, "CTRL_DEP");
//                 }
//             }
//         }
//     }

//     // Memory barrier handling
//     // for (auto bar : barrierInsts)
//     // {
//     //     createEdge(bar, G.memoryBarrier, G, "MEM_BARRIER");
//     //     createEdge(G.memoryBarrier, bar, G, "MEM_BARRIER");
//     // }

//     for (auto *I : barrierInsts)
//     {
//         for (auto &BB : F)
//         {
//             for (auto &J : BB)
//             {
//                 if (&J == I)
//                     continue;

//                 if (DT.dominates(&J, I))
//                 {
//                     // J happens before barrier
//                     createEdge(&J, I, G, "MEM_BARRIER_BEFORE");
//                 }
//                 else if (DT.dominates(I, &J))
//                 {
//                     // J happens after barrier
//                     createEdge(I, &J, G, "MEM_BARRIER_AFTER");
//                 }
//             }
//         }
//     }

//     for (auto LIIt = LI.begin(), LIE = LI.end(); LIIt != LIE; ++LIIt)
//     {
//         llvm::Loop *TopL = *LIIt;
//         analyzeAndAnnotateLoop(TopL, F, DI, SE, G);
//         // recurse into subloops
//         llvm::SmallVector<llvm::Loop *, 8> worklist;
//         for (llvm::Loop *SL : TopL->getSubLoops())
//             worklist.push_back(SL);
//         while (!worklist.empty())
//         {
//             llvm::Loop *L = worklist.pop_back_val();
//             analyzeAndAnnotateLoop(L, F, DI, SE, G);
//             for (llvm::Loop *SL : L->getSubLoops())
//                 worklist.push_back(SL);
//         }
//     }
// }

struct PDGAnalysis : public llvm::AnalysisInfoMixin<PDGAnalysis>
{
    using Result = llvm::dependencyGraph;

    // Required for identification
    static llvm::AnalysisKey Key;

    Result run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM)
    {
        auto &FAM = MAM.getResult<llvm::FunctionAnalysisManagerModuleProxy>(M).getManager();

        llvm::dependencyGraph G;
        for (llvm::Function &F : M)
        {
            if (F.isDeclaration())
                continue;

            auto &DAResult = FAM.getResult<llvm::DependenceAnalysis>(F); // DependenceInfo
            auto &MSSAW = FAM.getResult<llvm::MemorySSAAnalysis>(F);
            llvm::MemorySSA &MSSA = MSSAW.getMSSA();

            // Request LoopInfo and ScalarEvolution from the FAM
            auto &LI = FAM.getResult<llvm::LoopAnalysis>(F);
            auto &SE = FAM.getResult<llvm::ScalarEvolutionAnalysis>(F);
            auto &DT = FAM.getResult<llvm::DominatorTreeAnalysis>(F);
            auto &PDT = FAM.getResult<llvm::PostDominatorTreeAnalysis>(F);
            auto &AA = FAM.getResult<llvm::AAManager>(F);

            // Call the updated buildGraph that accepts DI, LI, SE by reference
            buildGraph(F, DAResult, LI, SE, G, MSSA, AA, DT, PDT);

            // buildGraph(F, DAResult, G, MSSA);
        }

        return G; // <-- RETURN the graph instead of printing
    }
};
llvm::AnalysisKey PDGAnalysis::Key;

using namespace llvm;

namespace
{

    struct PDGBuilderPass : public PassInfoMixin<PDGBuilderPass>
    {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM)
        {
            auto &G = MAM.getResult<PDGAnalysis>(M); // Get the dependency graph

            // printCollectedGraphDot(G, "pdg_global.dot");

            return PreservedAnalyses::all();
        }
    };

} // namespace

static PassPluginLibraryInfo getPDGBuilderPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "PDGBuilderPass", LLVM_VERSION_STRING,
            [](PassBuilder &PB)
            {
                PB.registerAnalysisRegistrationCallback(
                    [](ModuleAnalysisManager &MAM)
                    {
                        MAM.registerPass([&]
                                         { return PDGAnalysis(); });
                    });

                PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                       ArrayRef<PassBuilder::PipelineElement>)
                    {
                        if (Name == "pdg-builder")
                        {
                            MPM.addPass(PDGBuilderPass());
                            return true;
                        }
                        return false;
                    });
            }};
}

namespace llvm
{

    dependencyGraph runPDGOnModule(Module &M)
    {
        // Create analysis managers
        LoopAnalysisManager LAM;
        FunctionAnalysisManager FAM;
        CGSCCAnalysisManager CGAM;
        ModuleAnalysisManager MAM;

        // Build a PassBuilder and register analyses & proxies (same pattern as opt)
        PassBuilder PB;
        PB.registerModuleAnalyses(MAM);
        PB.registerCGSCCAnalyses(CGAM);
        PB.registerFunctionAnalyses(FAM);
        PB.registerLoopAnalyses(LAM);
        PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

        // Run the PDGAnalysis defined in this TU (reuses your code)
        PDGAnalysis pdgAnalysis;
        dependencyGraph G = pdgAnalysis.run(M, MAM);
        return G;
    }

    dependencyGraph runPDGOnModuleAndWriteDot(Module &M)
    {
        dependencyGraph G = runPDGOnModule(M);
        // printCollectedGraphDot(G, dotFilename);
        return G;
    }

    SmallVector<SmallVector<unsigned>> topologicalSortByLevels(const dependencyGraph &G)
    {
        SmallVector<SmallVector<unsigned>> levels;

        // Build adjacency list and compute in-degrees
        DenseMap<unsigned, SmallVector<unsigned>> adjList;
        DenseMap<unsigned, unsigned> inDegree;

        // Initialize in-degrees for all nodes
        for (const auto &nodeEntry : G.nodes)
        {
            unsigned nodeId = nodeEntry.second;
            inDegree[nodeId] = 0;
        }

        // Build adjacency list and count in-degrees
        for (const auto &edge : G.edges)
        {
            adjList[edge.source].push_back(edge.target);
            inDegree[edge.target]++;
        }

        // Find all nodes with in-degree 0 (starting nodes)
        SmallVector<unsigned> currentLevel;
        for (const auto &nodeEntry : G.nodes)
        {
            unsigned nodeId = nodeEntry.second;
            if (inDegree[nodeId] == 0)
            {
                currentLevel.push_back(nodeId);
            }
        }

        // Process nodes level by level
        while (!currentLevel.empty())
        {
            levels.push_back(currentLevel);
            SmallVector<unsigned> nextLevel;

            // Process all nodes in current level
            for (unsigned nodeId : currentLevel)
            {
                // For each neighbor, reduce in-degree
                if (adjList.find(nodeId) != adjList.end())
                {
                    for (unsigned neighbor : adjList[nodeId])
                    {
                        inDegree[neighbor]--;
                        // If in-degree becomes 0, add to next level
                        if (inDegree[neighbor] == 0)
                        {
                            nextLevel.push_back(neighbor);
                        }
                    }
                }
            }

            currentLevel = std::move(nextLevel);
        }

        // Check for cycles (if any node still has non-zero in-degree)
        for (const auto &entry : inDegree)
        {
            if (entry.second > 0)
            {
                llvm::nulls() << "Warning: Cycle detected in PDG! Node " << entry.first
                              << " has remaining in-degree " << entry.second << "\n";
            }
        }

        return levels;
    }

    // Compute vertex degrees (in-degree + out-degree)
    static DenseMap<unsigned, unsigned> computeVertexDegrees(const dependencyGraph &G)
    {
        DenseMap<unsigned, unsigned> degrees;

        // Initialize all vertices with degree 0
        for (const auto &nodeEntry : G.nodes)
        {
            degrees[nodeEntry.second] = 0;
        }

        // Count degrees from edges
        for (const auto &edge : G.edges)
        {
            degrees[edge.source]++; // out-degree
            degrees[edge.target]++; // in-degree
        }

        return degrees;
    }

    // Find strongly connected components using Tarjan's algorithm
    static void tarjanSCC(unsigned v,
                          DenseMap<unsigned, SmallVector<unsigned>> &adjList,
                          DenseMap<unsigned, int> &disc,
                          DenseMap<unsigned, int> &low,
                          DenseMap<unsigned, bool> &onStack,
                          SmallVector<unsigned> &stack,
                          SmallVector<SmallVector<unsigned>> &sccs,
                          int &time)
    {
        disc[v] = low[v] = ++time;
        stack.push_back(v);
        onStack[v] = true;

        if (adjList.find(v) != adjList.end())
        {
            for (unsigned w : adjList[v])
            {
                if (disc[w] == -1)
                {
                    tarjanSCC(w, adjList, disc, low, onStack, stack, sccs, time);
                    low[v] = std::min(low[v], low[w]);
                }
                else if (onStack[w])
                {
                    low[v] = std::min(low[v], disc[w]);
                }
            }
        }

        // Found an SCC
        if (low[v] == disc[v])
        {
            SmallVector<unsigned> scc;
            unsigned w;
            do
            {
                w = stack.pop_back_val();
                onStack[w] = false;
                scc.push_back(w);
            } while (w != v);
            sccs.push_back(scc);
        }
    }

    // Helper: Check if instruction is a PHI node or loop control
    static bool isLoopControlInstruction(Instruction *I)
    {
        return isa<PHINode>(I) || isa<BranchInst>(I) ||
               (I->isTerminator() && I->getParent()->getName().contains("loop"));
    }

    // Helper: Detect loop regions using back-edges
    static DenseMap<BasicBlock *, unsigned> detectLoopRegions(const dependencyGraph &G)
    {
        DenseMap<BasicBlock *, unsigned> blockToLoop;
        StringMap<unsigned> loopNameToId;
        unsigned nextLoopId = 0;

        // Simple approach: group blocks by loop name prefix
        // e.g., "loopcond", "loopbody", "loopcond3", "loopbody4" etc.
        for (const auto &nodeEntry : G.nodes)
        {
            Instruction *I = nodeEntry.first;
            BasicBlock *BB = I->getParent();
            std::string bbName = BB->getName().str();

            // Check if block name starts with "loop"
            if (bbName.rfind("loop", 0) == 0) // starts_with
            {
                // Extract loop base name (e.g., "loopcond" from "loopcond" or "loopcond3")
                std::string loopBaseName;
                size_t i = 4; // skip "loop"
                while (i < bbName.size() && !isdigit(bbName[i]))
                {
                    i++;
                }
                loopBaseName = bbName.substr(0, i); // e.g., "loopcond"

                // Find number suffix if any
                std::string loopId = loopBaseName;
                if (i < bbName.size())
                {
                    // Has digit suffix, use base+digit as identifier
                    while (i < bbName.size() && isdigit(bbName[i]))
                    {
                        i++;
                    }
                    loopId = bbName.substr(0, i); // e.g., "loopcond3"
                }

                // Assign loop ID
                if (loopNameToId.find(loopId) == loopNameToId.end())
                {
                    loopNameToId[loopId] = nextLoopId++;
                }

                blockToLoop[BB] = loopNameToId[loopId];
            }
        }

        return blockToLoop;
    }

    static SmallVector<SmallVector<unsigned>> findStronglyConnectedComponents(const dependencyGraph &G)
    {
        SmallVector<SmallVector<unsigned>> sccs;

        // Build adjacency list
        DenseMap<unsigned, SmallVector<unsigned>> adj;
        DenseSet<unsigned> allVertices;

        for (const auto &edge : G.edges)
        {
            adj[edge.source].push_back(edge.target);
            allVertices.insert(edge.source);
            allVertices.insert(edge.target);
        }

        // Add vertices with no edges
        for (const auto &node : G.nodes)
        {
            allVertices.insert(node.second);
        }

        // Tarjan's algorithm state
        DenseMap<unsigned, int> index;
        DenseMap<unsigned, int> lowlink;
        DenseMap<unsigned, bool> onStack;
        SmallVector<unsigned> stack;
        int currentIndex = 0;

        std::function<void(unsigned)> strongconnect = [&](unsigned v)
        {
            index[v] = lowlink[v] = currentIndex++;
            stack.push_back(v);
            onStack[v] = true;

            if (adj.find(v) != adj.end())
            {
                for (unsigned w : adj[v])
                {
                    if (index.find(w) == index.end())
                    {
                        // Successor w has not yet been visited; recurse on it
                        strongconnect(w);
                        lowlink[v] = std::min(lowlink[v], lowlink[w]);
                    }
                    else if (onStack[w])
                    {
                        // Successor w is in stack S and hence in the current SCC
                        lowlink[v] = std::min(lowlink[v], index[w]);
                    }
                }
            }

            // If v is a root node, pop the stack and create an SCC
            if (lowlink[v] == index[v])
            {
                SmallVector<unsigned> scc;
                unsigned w;
                do
                {
                    w = stack.pop_back_val();
                    onStack[w] = false;
                    scc.push_back(w);
                } while (w != v);

                // Only add as SCC if it has more than one vertex OR has self-loop
                if (scc.size() > 1 || (adj.find(v) != adj.end() &&
                                       std::find(adj[v].begin(), adj[v].end(), v) != adj[v].end()))
                {
                    sccs.push_back(scc);
                }
            }
        };

        // Run Tarjan's algorithm on all vertices
        for (unsigned v : allVertices)
        {
            if (index.find(v) == index.end())
            {
                strongconnect(v);
            }
        }

        return sccs;
    }

    // Helper: Check if an instruction is part of a loop structure
    static bool isLoopStructureInstruction(Instruction *I)
    {
        if (isa<PHINode>(I))
            return true;
        if (isa<BranchInst>(I))
        {
            BasicBlock *BB = I->getParent();
            StringRef BBName = BB->getName();
            return BBName.contains("loop");
        }
        return false;
    }

    // Helper: Get all basic blocks for an SCC
    static SmallPtrSet<BasicBlock *, 16> getBasicBlocksForSCC(
        const SmallVector<unsigned> &scc,
        const dependencyGraph &G)
    {

        SmallPtrSet<BasicBlock *, 16> blocks;
        for (unsigned vertexId : scc)
        {
            if (Instruction *I = getInstructionFromVertex(vertexId, G))
            {
                blocks.insert(I->getParent());
            }
        }
        return blocks;
    }

    // COMPLETE REPLACEMENT for performMinCutAndCreateTaskGraph
    // COMPLETE REPLACEMENT for performMinCutAndCreateTaskGraph
    TaskGraph performMinCutAndCreateTaskGraph(const dependencyGraph &G)
    {
        TaskGraph TG;

        llvm::nulls() << "\nPerforming SCC-aware min-cut partitioning...\n";

        // Step 1: Find strongly connected components (these are loop-carried dependencies)
        SmallVector<SmallVector<unsigned>> sccs = findStronglyConnectedComponents(G);

        llvm::nulls() << "✓ Found " << sccs.size() << " strongly connected components (SCCs)\n";

        // Step 2: Create a vertex-to-SCC mapping
        DenseMap<unsigned, unsigned> vertexToSCC;
        for (unsigned sccId = 0; sccId < sccs.size(); ++sccId)
        {
            for (unsigned v : sccs[sccId])
            {
                vertexToSCC[v] = sccId;
            }
        }

        // Step 3: Collect all vertices (including those not in SCCs)
        DenseSet<unsigned> allVertices;
        for (const auto &node : G.nodes)
        {
            allVertices.insert(node.second);
        }

        // Step 5: Group vertices by task
        // - Each loop region becomes one task first
        // - Each SCC becomes one task for remaining vertices
        // - Vertices not in any SCC are grouped by basic block
        DenseMap<unsigned, unsigned> vertexToTask;
        SmallPtrSet<BasicBlock *, 16> loopTaskBlocks;
        unsigned nextTaskId = 0;

        for (const auto &Region : G.loopRegions)
        {
            if (!Region.isExtractionRegion)
                continue;

            TaskNode task;
            task.taskId = nextTaskId;
            task.kind = TaskKind::LoopRegion;
            task.loopRegionId = Region.loopRegionId;
            task.blocks = Region.blocks;

            DenseSet<unsigned> addedVertices;
            for (BasicBlock *BB : Region.blocks)
            {
                loopTaskBlocks.insert(BB);
                for (Instruction &I : *BB)
                {
                    unsigned VertexId = G.nodes.lookup(&I);
                    if (!VertexId)
                        continue;
                    if (!addedVertices.insert(VertexId).second)
                        continue;

                    task.vertices.push_back(VertexId);
                    vertexToTask[VertexId] = nextTaskId;
                }
            }

            if (!task.vertices.empty())
            {
                /* Debug logging disabled: created loop extraction task */
                TG.tasks.push_back(task);
                nextTaskId++;
            }
        }

        // Create tasks for SCCs
        for (unsigned sccId = 0; sccId < sccs.size(); ++sccId)
        {
            TaskNode task;
            task.taskId = nextTaskId;
            task.kind = TaskKind::Regular;

            for (unsigned VertexId : sccs[sccId])
            {
                if (vertexToTask.find(VertexId) != vertexToTask.end())
                    continue;
                task.vertices.push_back(VertexId);
            }

            // Map all vertices to this task
            for (unsigned v : task.vertices)
            {
                vertexToTask[v] = nextTaskId;
            }

            if (!task.vertices.empty())
            {
                TG.tasks.push_back(task);
                nextTaskId++;
            }
        }

        // Create tasks for non-SCC vertices (group by basic block)
        DenseMap<BasicBlock *, unsigned> blockToTask;

        for (unsigned v : allVertices)
        {
            if (vertexToTask.find(v) == vertexToTask.end())
            {
                // This vertex is not in any SCC
                Instruction *I = getInstructionFromVertex(v, G);
                if (!I)
                    continue;

                BasicBlock *BB = I->getParent();
                if (loopTaskBlocks.count(BB))
                    continue;

                // Check if we already have a task for this block
                if (blockToTask.find(BB) == blockToTask.end())
                {
                    // Create new task for this block
                    TaskNode task;
                    task.taskId = nextTaskId;
                    task.kind = TaskKind::Regular;
                    task.blocks.push_back(BB);

                    // Add all vertices from this block that aren't in SCCs
                    for (Instruction &Inst : *BB)
                    {
                        unsigned vId = G.nodes.lookup(&Inst);
                        if (vId && vertexToTask.find(vId) == vertexToTask.end())
                        {
                            task.vertices.push_back(vId);
                            vertexToTask[vId] = nextTaskId;
                        }
                    }

                    if (!task.vertices.empty())
                    {
                        blockToTask[BB] = nextTaskId;
                        TG.tasks.push_back(task);
                        nextTaskId++;
                    }
                }
                else
                {
                    // Add to existing task for this block
                    unsigned taskId = blockToTask[BB];
                    vertexToTask[v] = taskId;
                    TG.tasks[taskId].vertices.push_back(v);
                }
            }
        }

        // Copy vertexToTask mapping to TG
        TG.vertexToTask = vertexToTask;

        llvm::nulls() << "✓ Created task graph with " << TG.tasks.size() << " tasks\n";

        // Step 6: Build task-level edges
        DenseSet<std::pair<unsigned, unsigned>> seenTaskEdges;

        for (const auto &edge : G.edges)
        {
            unsigned srcVertex = edge.source;
            unsigned dstVertex = edge.target;

            auto srcIt = vertexToTask.find(srcVertex);
            auto dstIt = vertexToTask.find(dstVertex);

            if (srcIt == vertexToTask.end() || dstIt == vertexToTask.end())
            {
                continue;
            }

            unsigned srcTask = srcIt->second;
            unsigned dstTask = dstIt->second;

            // Skip self-edges (within same task)
            if (srcTask == dstTask)
                continue;

            auto taskEdgePair = std::make_pair(srcTask, dstTask);
            if (seenTaskEdges.insert(taskEdgePair).second)
            {
                depEdge taskEdge;
                taskEdge.source = srcTask;
                taskEdge.target = dstTask;
                taskEdge.type = edge.type;
                taskEdge.isLoop = edge.isLoop;
                TG.taskEdges.push_back(taskEdge);
            }
        }

        llvm::nulls() << "✓ Created " << TG.taskEdges.size() << " task-level edges\n";

        // Step 7: Identify cut vertices (optional, for analysis)
        DenseMap<unsigned, unsigned> taskInDegree;
        DenseMap<unsigned, unsigned> taskOutDegree;

        for (const auto &edge : TG.taskEdges)
        {
            taskOutDegree[edge.source]++;
            taskInDegree[edge.target]++;
        }

        for (unsigned taskId = 0; taskId < TG.tasks.size(); ++taskId)
        {
            unsigned inDeg = taskInDegree.lookup(taskId);
            unsigned outDeg = taskOutDegree.lookup(taskId);

            // High degree = potential bottleneck
            if (inDeg >= 2 && outDeg >= 2)
            {
                TG.cutVertices.push_back(taskId);
                TG.cutVertexOrder[taskId] = TG.cutVertices.size() - 1;
            }
        }

        llvm::nulls() << "✓ Identified " << TG.cutVertices.size() << " potential bottlenecks\n";

        return TG;
    }

    // Topological sort on task graph with level-by-level output
    // Preserves execution order of cut vertices
    SmallVector<SmallVector<unsigned>> topologicalSortTaskGraph(const TaskGraph &TG)
    {
        SmallVector<SmallVector<unsigned>> levels;

        // Build adjacency list and compute in-degrees for tasks
        DenseMap<unsigned, SmallVector<unsigned>> adjList;
        DenseMap<unsigned, unsigned> inDegree;

        // Initialize in-degrees for all tasks
        for (const auto &task : TG.tasks)
        {
            inDegree[task.taskId] = 0;
        }

        // Build adjacency list and count in-degrees
        for (const auto &edge : TG.taskEdges)
        {
            adjList[edge.source].push_back(edge.target);
            inDegree[edge.target]++;
        }

        // Find all tasks with in-degree 0 (starting tasks)
        SmallVector<unsigned> currentLevel;
        for (const auto &task : TG.tasks)
        {
            if (inDegree[task.taskId] == 0)
            {
                currentLevel.push_back(task.taskId);
            }
        }

        // Process tasks level by level
        while (!currentLevel.empty())
        {
            // Sort current level by cut vertex order if they are cut vertices
            std::sort(currentLevel.begin(), currentLevel.end(),
                      [&TG](unsigned a, unsigned b)
                      {
                          const TaskNode &taskA = TG.tasks[a];
                          const TaskNode &taskB = TG.tasks[b];

                          // Both are cut vertices - sort by their order
                          if (taskA.isCutVertex && taskB.isCutVertex)
                          {
                              return TG.cutVertexOrder.lookup(taskA.originalVertex) <
                                     TG.cutVertexOrder.lookup(taskB.originalVertex);
                          }
                          // Cut vertices come before non-cut vertices at same level
                          if (taskA.isCutVertex)
                              return true;
                          if (taskB.isCutVertex)
                              return false;
                          // Both are regular tasks - maintain stable order
                          return a < b;
                      });

            levels.push_back(currentLevel);
            SmallVector<unsigned> nextLevel;

            // Process all tasks in current level
            for (unsigned taskId : currentLevel)
            {
                // For each successor, reduce in-degree
                if (adjList.find(taskId) != adjList.end())
                {
                    for (unsigned successor : adjList[taskId])
                    {
                        inDegree[successor]--;
                        // If in-degree becomes 0, add to next level
                        if (inDegree[successor] == 0)
                        {
                            nextLevel.push_back(successor);
                        }
                    }
                }
            }

            currentLevel = std::move(nextLevel);
        }

        // Check for cycles
        for (const auto &entry : inDegree)
        {
            if (entry.second > 0)
            {
                llvm::nulls() << "Warning: Cycle detected in task graph! Task " << entry.first
                              << " has remaining in-degree " << entry.second << "\n";
            }
        }

        return levels;
    }

    // ========================================================================
    // PARALLEL IR RECONSTRUCTION
    // ========================================================================

    // Helper: Get instruction from vertex ID
    static Instruction *getInstructionFromVertex(unsigned vertexId, const dependencyGraph &PDG)
    {
        for (const auto &entry : PDG.nodes)
        {
            if (entry.second == vertexId)
                return entry.first;
        }
        return nullptr;
    }

    // Helper: Check if instruction is in component
    static bool isInComponent(Instruction *I, const TaskNode &task, const dependencyGraph &PDG)
    {
        for (unsigned vid : task.vertices)
        {
            if (getInstructionFromVertex(vid, PDG) == I)
                return true;
        }
        return false;
    }

    // Structure to hold component function information
    struct ComponentFunction
    {
        Function *func = nullptr;
        SmallVector<Value *> liveIns;
        SmallVector<Value *> liveOuts;
        StructType *argStructTy = nullptr;
        StructType *resultStructTy = nullptr;
        AllocaInst *argAlloca = nullptr;
        AllocaInst *resultAlloca = nullptr;
    };

    // Analyze live-in and live-out values for a component
    static void analyzeLiveValues(const TaskNode &task,
                                  const dependencyGraph &PDG,
                                  SmallVector<Value *> &liveIns,
                                  SmallVector<Value *> &liveOuts)
    {
        DenseSet<Value *> liveInSet, liveOutSet;

        for (unsigned vertexId : task.vertices)
        {
            Instruction *I = getInstructionFromVertex(vertexId, PDG);
            if (!I)
                continue;

            // Check operands for live-ins
            for (Use &U : I->operands())
            {
                if (Instruction *OpInst = dyn_cast<Instruction>(U.get()))
                {
                    if (!isInComponent(OpInst, task, PDG))
                    {
                        liveInSet.insert(OpInst);
                    }
                }
                else if (Argument *Arg = dyn_cast<Argument>(U.get()))
                {
                    liveInSet.insert(Arg);
                }
            }

            // Check uses for live-outs
            for (User *Usr : I->users())
            {
                if (Instruction *UseInst = dyn_cast<Instruction>(Usr))
                {
                    if (!isInComponent(UseInst, task, PDG))
                    {
                        liveOutSet.insert(I);
                        break;
                    }
                }
            }
        }

        liveIns.assign(liveInSet.begin(), liveInSet.end());
        liveOuts.assign(liveOutSet.begin(), liveOutSet.end());
    }

    // Outline a component into a separate function
    static Function *outlineComponent(const TaskNode &task,
                                      const dependencyGraph &PDG,
                                      const SmallVector<Value *> &liveIns,
                                      const SmallVector<Value *> &liveOuts,
                                      ComponentFunction &compFunc,
                                      Module &M)
    {
        LLVMContext &Ctx = M.getContext();

        // DEBUG: Print what we're trying to outline
        // llvm::nulls() << "\n[DEBUG] Outlining Task " << task.taskId << ":\n";
        // llvm::nulls() << "  Vertices: " << task.vertices.size() << "\n";
        // for (unsigned vertexId : task.vertices)
        // {
        //     Instruction *I = getInstructionFromVertex(vertexId, PDG);
        //     if (I)
        //     {
        //         llvm::nulls() << "    Vertex " << vertexId << ": ";
        //         I->print(llvm::nulls());
        //         llvm::nulls() << " (in block: " << I->getParent()->getName() << ")\n";
        //     }
        //     else
        //     {
        //         llvm::nulls() << "    Vertex " << vertexId << ": NULL instruction!\n";
        //     }
        // }
        // llvm::nulls() << "  Live-ins: " << liveIns.size() << "\n";
        // llvm::nulls() << "  Live-outs: " << liveOuts.size() << "\n";

        // Create struct types for arguments and results
        SmallVector<Type *> argTypes, resultTypes;
        for (Value *V : liveIns)
            argTypes.push_back(V->getType());
        for (Value *V : liveOuts)
            resultTypes.push_back(V->getType());

        compFunc.argStructTy = argTypes.empty() ? nullptr : StructType::create(argTypes, "component_args_" + std::to_string(task.taskId));
        compFunc.resultStructTy = resultTypes.empty() ? nullptr : StructType::create(resultTypes, "component_results_" + std::to_string(task.taskId));

        // Function signature: void component_N(argStruct*, resultStruct*)
        SmallVector<Type *> paramTypes;
        if (compFunc.argStructTy)
            paramTypes.push_back(compFunc.argStructTy->getPointerTo());
        if (compFunc.resultStructTy)
            paramTypes.push_back(compFunc.resultStructTy->getPointerTo());

        FunctionType *FT = FunctionType::get(Type::getVoidTy(Ctx), paramTypes, false);
        Function *F = Function::Create(FT, GlobalValue::InternalLinkage,
                                       "component_" + std::to_string(task.taskId), &M);

        // Group instructions by their parent basic block
        DenseMap<BasicBlock *, SmallVector<Instruction *>> blockInstructions;
        for (unsigned vertexId : task.vertices)
        {
            Instruction *I = getInstructionFromVertex(vertexId, PDG);
            if (!I)
                continue;
            blockInstructions[I->getParent()].push_back(I);
        }

        // If empty task, just return
        if (blockInstructions.empty())
        {
            BasicBlock *BB = BasicBlock::Create(Ctx, "entry", F);
            IRBuilder<> Builder(BB);
            Builder.CreateRetVoid();
            return F;
        }

        // Create value map for cloning
        ValueToValueMapTy VMap;
        DenseMap<BasicBlock *, BasicBlock *> BBMap;

        // Create entry block and load live-ins
        BasicBlock *EntryBB = BasicBlock::Create(Ctx, "entry", F);
        IRBuilder<> Builder(EntryBB);

        if (compFunc.argStructTy)
        {
            Value *ArgPtr = F->getArg(0);
            for (unsigned i = 0; i < liveIns.size(); ++i)
            {
                Value *FieldPtr = Builder.CreateStructGEP(compFunc.argStructTy, ArgPtr, i);
                Value *LoadedVal = Builder.CreateLoad(liveIns[i]->getType(), FieldPtr);
                VMap[liveIns[i]] = LoadedVal;
            }
        }

        // Clone all blocks (create empty blocks first)
        SmallVector<BasicBlock *> orderedBlocks;
        for (auto &KV : blockInstructions)
        {
            BasicBlock *OrigBB = KV.first;
            BasicBlock *NewBB = BasicBlock::Create(Ctx, OrigBB->getName(), F);
            BBMap[OrigBB] = NewBB;
            orderedBlocks.push_back(OrigBB);
        }

        // Clone instructions into their respective blocks
        for (BasicBlock *OrigBB : orderedBlocks)
        {
            BasicBlock *NewBB = BBMap[OrigBB];
            IRBuilder<> BBBuilder(NewBB);

            for (Instruction *I : blockInstructions[OrigBB])
            {
                Instruction *NewI = I->clone();
                BBBuilder.Insert(NewI);
                VMap[I] = NewI;
            }

            // Add terminator if block doesn't have one
            if (!NewBB->getTerminator())
            {
                // If this is the last block, branch to exit
                // Otherwise, add unreachable (will fix later)
                BBBuilder.CreateRetVoid();
            }
        }

        // Remap all operands and fix up PHI nodes
        for (BasicBlock *OrigBB : orderedBlocks)
        {
            BasicBlock *NewBB = BBMap[OrigBB];
            for (Instruction &I : *NewBB)
            {
                // Remap operands
                for (unsigned i = 0; i < I.getNumOperands(); ++i)
                {
                    Value *Op = I.getOperand(i);
                    if (VMap.count(Op))
                    {
                        I.setOperand(i, VMap[Op]);
                    }
                    else if (auto *BB = dyn_cast<BasicBlock>(Op))
                    {
                        if (BBMap.count(BB))
                            I.setOperand(i, BBMap[BB]);
                    }
                }

                // Fix PHI nodes - remove incoming edges from blocks we didn't clone
                if (auto *PHI = dyn_cast<PHINode>(&I))
                {
                    SmallVector<unsigned> toRemove;
                    for (unsigned i = 0; i < PHI->getNumIncomingValues(); ++i)
                    {
                        BasicBlock *IncomingBB = PHI->getIncomingBlock(i);
                        if (!BBMap.count(IncomingBB))
                        {
                            toRemove.push_back(i);
                        }
                    }
                    // Remove in reverse order to maintain indices
                    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it)
                    {
                        PHI->removeIncomingValue(*it, false);
                    }
                }
            }
        }

        // Branch from entry to first block
        if (!orderedBlocks.empty())
        {
            Builder.SetInsertPoint(EntryBB);
            Builder.CreateBr(BBMap[orderedBlocks[0]]);
        }

        // Create exit block and store live-outs
        BasicBlock *ExitBB = BasicBlock::Create(Ctx, "exit", F);
        IRBuilder<> ExitBuilder(ExitBB);

        if (compFunc.resultStructTy)
        {
            Value *ResultPtr = F->getArg(compFunc.argStructTy ? 1 : 0);
            for (unsigned i = 0; i < liveOuts.size(); ++i)
            {
                Value *ResultVal = liveOuts[i];
                auto It = VMap.find(liveOuts[i]);
                if (It != VMap.end())
                    ResultVal = static_cast<llvm::Value *>(It->second);

                Value *FieldPtr = ExitBuilder.CreateStructGEP(compFunc.resultStructTy, ResultPtr, i);
                ExitBuilder.CreateStore(ResultVal, FieldPtr);
            }
        }

        ExitBuilder.CreateRetVoid();

        // Fix all terminators to point to exit block if they would leave the function
        for (BasicBlock &BB : *F)
        {
            if (&BB == EntryBB || &BB == ExitBB)
                continue;

            Instruction *Term = BB.getTerminator();
            if (!Term || isa<ReturnInst>(Term))
            {
                if (Term)
                    Term->eraseFromParent();
                IRBuilder<>(&BB).CreateBr(ExitBB);
            }
            else if (auto *Br = dyn_cast<BranchInst>(Term))
            {
                for (unsigned i = 0; i < Br->getNumSuccessors(); ++i)
                {
                    BasicBlock *Succ = Br->getSuccessor(i);
                    if (Succ->getParent() != F || !BBMap.count(Succ))
                    {
                        Br->setSuccessor(i, ExitBB);
                    }
                }
            }
        }

        return F;
    }

    // Generate pthread-based parallel region for a level
    static void generateParallelRegion(IRBuilder<> &Builder,
                                       const SmallVector<unsigned> &taskIds,
                                       SmallVector<ComponentFunction> &components,
                                       Module *M)
    {
        LLVMContext &Ctx = M->getContext();

        // Declare pthread functions
        // pthread_create(pthread_t*, attr, void*(*)(void*), void*)
        Type *VoidTy = Type::getVoidTy(Ctx);
        PointerType *VoidPtrTy = VoidTy->getPointerTo();
        PointerType *PthreadTy = VoidTy->getPointerTo(); // Opaque handle

        FunctionType *StartRoutineTy = FunctionType::get(VoidPtrTy, {VoidPtrTy}, false);

        FunctionType *PthreadCreateTy = FunctionType::get(
            Type::getInt32Ty(Ctx),
            {PthreadTy->getPointerTo(), VoidPtrTy, StartRoutineTy->getPointerTo(), VoidPtrTy},
            false);

        FunctionType *PthreadJoinTy = FunctionType::get(
            Type::getInt32Ty(Ctx),
            {PthreadTy, VoidPtrTy->getPointerTo()},
            false);

        FunctionCallee PthreadCreate = M->getOrInsertFunction("pthread_create", PthreadCreateTy);
        FunctionCallee PthreadJoin = M->getOrInsertFunction("pthread_join", PthreadJoinTy);

        // Allocate thread handles
        Type *ThreadArrTy = ArrayType::get(PthreadTy, taskIds.size());
        AllocaInst *ThreadHandles = Builder.CreateAlloca(ThreadArrTy, nullptr, "thread_handles");

        // Create wrapper functions for pthread (since our components have different signatures)
        SmallVector<Function *> wrappers;
        for (unsigned tid : taskIds)
        {
            ComponentFunction &comp = components[tid];

            // Create wrapper: void* wrapper(void* arg)
            FunctionType *WrapperTy = FunctionType::get(VoidPtrTy, {VoidPtrTy}, false);
            Function *Wrapper = Function::Create(WrapperTy, GlobalValue::InternalLinkage,
                                                 "thread_wrapper_" + std::to_string(tid), M);
            BasicBlock *WrapperBB = BasicBlock::Create(Ctx, "entry", Wrapper);
            IRBuilder<> WrapperBuilder(WrapperBB);

            // Cast void* arg back to our struct types and call component function
            SmallVector<Value *> args;
            if (comp.argStructTy)
            {
                Value *ArgCast = WrapperBuilder.CreateBitCast(Wrapper->getArg(0),
                                                              comp.argStructTy->getPointerTo());
                args.push_back(ArgCast);
            }
            if (comp.resultStructTy)
            {
                // Result pointer is stored after arg pointer in the void* arg
                // For simplicity, we'll pass both through a struct
                args.push_back(WrapperBuilder.CreateBitCast(Wrapper->getArg(0),
                                                            comp.resultStructTy->getPointerTo()));
            }

            // WrapperBuilder.CreateCall(comp.func, args);
            // WrapperBuilder.CreateRet(ConstantPointerNull::get(VoidPtrTy));

            // Add printf for debugging
            FunctionType *PrintfTy = FunctionType::get(
                Type::getInt32Ty(Ctx),
                {PointerType::get(Type::getInt8Ty(Ctx), 0)},
                true);
            FunctionCallee PrintfFunc = M->getOrInsertFunction("printf", PrintfTy);

            // Create format string
            Value *FormatStr = WrapperBuilder.CreateGlobalStringPtr(
                "Thread executing component " + std::to_string(tid) + "\\n");
            WrapperBuilder.CreateCall(PrintfFunc, {FormatStr});

            WrapperBuilder.CreateCall(comp.func, args);

            FormatStr = WrapperBuilder.CreateGlobalStringPtr(
                "Thread finished component " + std::to_string(tid) + "\\n");
            WrapperBuilder.CreateCall(PrintfFunc, {FormatStr});

            WrapperBuilder.CreateRet(ConstantPointerNull::get(VoidPtrTy));

            wrappers.push_back(Wrapper);
        }

        // Spawn threads
        for (unsigned i = 0; i < taskIds.size(); ++i)
        {
            ComponentFunction &comp = components[taskIds[i]];

            Value *ThreadPtr = Builder.CreateGEP(ThreadArrTy, ThreadHandles,
                                                 {Builder.getInt32(0), Builder.getInt32(i)});

            // For simplicity, pass argAlloca as the thread argument
            Value *ThreadArg;

            if (comp.argAlloca)
                ThreadArg = Builder.CreateBitCast(comp.argAlloca, VoidPtrTy);
            else
                ThreadArg = ConstantPointerNull::get(VoidPtrTy);

            Builder.CreateCall(PthreadCreate, {ThreadPtr,
                                               ConstantPointerNull::get(VoidPtrTy), // default attributes
                                               wrappers[i],
                                               ThreadArg});
        }

        // Join threads (barrier)
        for (unsigned i = 0; i < taskIds.size(); ++i)
        {
            Value *ThreadPtr = Builder.CreateGEP(ThreadArrTy, ThreadHandles,
                                                 {Builder.getInt32(0), Builder.getInt32(i)});
            Value *Thread = Builder.CreateLoad(PthreadTy, ThreadPtr);
            Builder.CreateCall(PthreadJoin, {Thread, ConstantPointerNull::get(VoidPtrTy->getPointerTo())});
        }
    }

    static void orderLoopTaskBlocks(const TaskNode &task,
                                    const dependencyGraph &PDG,
                                    SmallVector<BasicBlock *> &blockVec)
    {
        if (task.kind != TaskKind::LoopRegion || !task.loopRegionId)
            return;

        const auto *Region = findLoopRegionInfo(PDG, *task.loopRegionId);
        if (!Region || !Region->header)
            return;

        SmallPtrSet<BasicBlock *, 16> blockSet(blockVec.begin(), blockVec.end());
        if (!blockSet.count(Region->header))
            return;

        SmallVector<BasicBlock *> ordered;
        SmallPtrSet<BasicBlock *, 16> visited;

        std::function<void(BasicBlock *)> visit = [&](BasicBlock *BB)
        {
            if (!BB || !blockSet.count(BB) || !visited.insert(BB).second)
                return;

            ordered.push_back(BB);

            SmallVector<BasicBlock *, 4> succs;
            for (BasicBlock *Succ : successors(BB))
            {
                if (blockSet.count(Succ))
                    succs.push_back(Succ);
            }

            llvm::sort(succs, [](BasicBlock *A, BasicBlock *B)
                       { return A->getName() < B->getName(); });
            for (BasicBlock *Succ : succs)
                visit(Succ);
        };

        visit(Region->header);

        SmallVector<BasicBlock *> remaining;
        for (BasicBlock *BB : blockVec)
        {
            if (!visited.count(BB))
                remaining.push_back(BB);
        }
        llvm::sort(remaining, [](BasicBlock *A, BasicBlock *B)
                   { return A->getName() < B->getName(); });
        ordered.append(remaining.begin(), remaining.end());
        blockVec = std::move(ordered);
    }

    static int64_t estimateInstructionWorkUnits(Instruction *I)
    {
        if (!I)
            return 1;

        if (auto *II = dyn_cast<IntrinsicInst>(I))
        {
            switch (II->getIntrinsicID())
            {
            case Intrinsic::memcpy:
            case Intrinsic::memmove:
            case Intrinsic::memset:
                return 14;
            case Intrinsic::lifetime_start:
            case Intrinsic::lifetime_end:
                return 0;
            default:
                return 6;
            }
        }

        if (auto *CB = dyn_cast<CallBase>(I))
        {
            Function *callee = CB->getCalledFunction();
            if (callee)
            {
                StringRef name = callee->getName();
                if (name.starts_with("graph_"))
                    return 18;
                if (name.contains("roaring") || name.contains("bitmap"))
                    return 16;
                if (name.starts_with("malloc") || name.starts_with("calloc") ||
                    name.starts_with("realloc") || name.starts_with("free"))
                    return 12;
                if (name.starts_with("sgpl_") || name.starts_with("parallel_"))
                    return 8;
            }
            return 10;
        }

        switch (I->getOpcode())
        {
        case Instruction::Load:
        case Instruction::Store:
            return 5;
        case Instruction::AtomicCmpXchg:
        case Instruction::AtomicRMW:
        case Instruction::Fence:
            return 18;
        case Instruction::Alloca:
            return 3;
        case Instruction::PHI:
        case Instruction::Br:
        case Instruction::Switch:
        case Instruction::IndirectBr:
        case Instruction::Select:
            return 2;
        case Instruction::ICmp:
        case Instruction::FCmp:
            return 2;
        case Instruction::GetElementPtr:
        case Instruction::Trunc:
        case Instruction::ZExt:
        case Instruction::SExt:
        case Instruction::FPToUI:
        case Instruction::FPToSI:
        case Instruction::UIToFP:
        case Instruction::SIToFP:
        case Instruction::FPTrunc:
        case Instruction::FPExt:
        case Instruction::PtrToInt:
        case Instruction::IntToPtr:
        case Instruction::BitCast:
        case Instruction::AddrSpaceCast:
            return 1;
        case Instruction::UDiv:
        case Instruction::SDiv:
        case Instruction::FDiv:
        case Instruction::URem:
        case Instruction::SRem:
        case Instruction::FRem:
            return 4;
        case Instruction::Mul:
        case Instruction::FMul:
        case Instruction::Add:
        case Instruction::FAdd:
        case Instruction::Sub:
        case Instruction::FSub:
        case Instruction::Shl:
        case Instruction::LShr:
        case Instruction::AShr:
        case Instruction::And:
        case Instruction::Or:
        case Instruction::Xor:
            return 2;
        default:
            return 3;
        }
    }

    static int64_t estimateTaskWorkUnits(const TaskNode &task, const dependencyGraph &PDG)
    {
        DenseSet<unsigned> vertexSet;
        int64_t work = 0;
        int64_t memOps = 0;
        int64_t callOps = 0;
        int64_t controlOps = 0;

        for (unsigned vertex : task.vertices)
            vertexSet.insert(vertex);

        for (const auto &entry : PDG.nodes)
        {
            Instruction *I = entry.first;
            unsigned vertexId = (unsigned)entry.second;

            if (!vertexSet.count(vertexId))
                continue;

            work += estimateInstructionWorkUnits(I);
            if (I->mayReadOrWriteMemory())
                ++memOps;
            if (isa<CallBase>(I))
                ++callOps;
            if (isa<PHINode>(I) || I->isTerminator())
                ++controlOps;
        }

        if (work == 0)
            work = task.vertices.empty() ? 1 : (int64_t)task.vertices.size() * 2;

        work += (int64_t)task.blocks.size() * 2;
        work += callOps * 2;
        work += controlOps;

        if (task.kind == TaskKind::LoopRegion)
        {
            work += 12;
            work += memOps * 2;
            if (task.loopRegionId)
            {
                for (const auto &region : PDG.loopRegions)
                {
                    if (region.loopRegionId != *task.loopRegionId)
                        continue;
                    work += (int64_t)region.blocks.size() * 2;
                    work += (int64_t)region.nestedSubloopCount * 8;
                    break;
                }
            }
        }

        if (task.kind == TaskKind::CutVertex || task.isCutVertex)
            work += 2;

        if (work < 1)
            work = 1;
        return work;
    }

    // COMPLETE REPLACEMENT for reconstructParallelIR
    // COMPLETE REPLACEMENT for reconstructParallelIR
    void reconstructParallelIR(Module &M,
                               const dependencyGraph &PDG,
                               const TaskGraph &TG,
                               const SmallVector<SmallVector<unsigned>> &levels)
    {

        // llvm::nulls() << "\n=======================================================\n";
        // llvm::nulls() << "       Parallel IR Reconstruction (SCC-Aware)\n";
        // llvm::nulls() << "=======================================================\n\n";

        Function *mainFunc = M.getFunction("main");
        if (!mainFunc)
        {
            llvm::nulls() << "✗ No main function found!\n";
            return;
        }

        llvm::nulls() << "✓ Found main function\n";

        // Create CodeExtractorAnalysisCache for the function
        CodeExtractorAnalysisCache CEAC(*mainFunc);

        // Step 1: For each task, collect ALL basic blocks needed
        SmallVector<SmallPtrSet<BasicBlock *, 16>> taskBlocks(TG.tasks.size());
        SmallVector<bool> isLoopTask(TG.tasks.size(), false);

        for (unsigned taskId = 0; taskId < TG.tasks.size(); ++taskId)
        {
            const TaskNode &task = TG.tasks[taskId];
            SmallPtrSet<BasicBlock *, 16> &blocks = taskBlocks[taskId];

            if (!task.blocks.empty())
            {
                for (BasicBlock *BB : task.blocks)
                {
                    blocks.insert(BB);
                }
            }
            else
            {
                // Collect all blocks containing vertices in this task
                for (unsigned vertexId : task.vertices)
                {
                    if (Instruction *I = getInstructionFromVertex(vertexId, PDG))
                    {
                        blocks.insert(I->getParent());
                    }
                }
            }

            isLoopTask[taskId] = task.kind == TaskKind::LoopRegion;

            // llvm::nulls() << "Task " << taskId << ": " << blocks.size() << " blocks"
            //<< (isLoopTask[taskId] ? " [LOOP]" : "") << "\n";
        }

        // Step 2: Extract tasks using CodeExtractor
        LLVMContext &Ctx = M.getContext();
        SmallVector<Function *> extractedFunctions(TG.tasks.size(), nullptr);

        // llvm::nulls() << "\nExtracting tasks using CodeExtractor...\n";

        for (unsigned taskId = 0; taskId < TG.tasks.size(); ++taskId)
        {
            const auto &blocks = taskBlocks[taskId];

            if (blocks.empty())
            {
                llvm::nulls() << "  Task " << taskId << ": No blocks - skipped\n";
                continue;
            }

            // Skip if blocks contain only barriers or are empty
            bool hasOnlyBarriers = true;
            for (BasicBlock *BB : blocks)
            {
                for (Instruction &I : *BB)
                {
                    if (!isa<UnreachableInst>(&I) && !isa<ReturnInst>(&I))
                    {
                        hasOnlyBarriers = false;
                        break;
                    }
                }
                if (!hasOnlyBarriers)
                    break;
            }

            if (hasOnlyBarriers)
            {
                llvm::nulls() << "  Task " << taskId << ": Only barriers/empty - skipped\n";
                continue;
            }

            // Convert to vector for CodeExtractor
            SmallVector<BasicBlock *> blockVec(blocks.begin(), blocks.end());

            if (TG.tasks[taskId].kind == TaskKind::LoopRegion)
            {
                orderLoopTaskBlocks(TG.tasks[taskId], PDG, blockVec);
                const auto *Region = findLoopRegionInfo(PDG, *TG.tasks[taskId].loopRegionId);
                (void)Region;
                /* Debug logging disabled: extracting loop nest task */
            }
            else
            {
                // Sort blocks to ensure a valid CFG (entry block first)
                BasicBlock *entryBlock = nullptr;
                for (BasicBlock *BB : blockVec)
                {
                    bool hasExternalPred = false;
                    for (BasicBlock *Pred : predecessors(BB))
                    {
                        if (!blocks.count(Pred))
                        {
                            hasExternalPred = true;
                            break;
                        }
                    }
                    if (hasExternalPred || BB == &mainFunc->getEntryBlock())
                    {
                        entryBlock = BB;
                        break;
                    }
                }

                if (!entryBlock && !blockVec.empty())
                {
                    entryBlock = blockVec[0];
                }

                // Reorder so entry is first
                if (entryBlock)
                {
                    auto it = std::find(blockVec.begin(), blockVec.end(), entryBlock);
                    if (it != blockVec.begin() && it != blockVec.end())
                    {
                        std::swap(*it, blockVec[0]);
                    }
                }
            }

            // Debug: print what we're trying to extract
            // llvm::nulls() << "  [DEBUG] Task " << taskId << " blocks to extract:\n";
            for (BasicBlock *BB : blockVec)
            {
                llvm::nulls() << "    - " << BB->getName() << "\n";
            }

            // Try to extract
            CodeExtractor CE(blockVec);

            if (Function *extracted = CE.extractCodeRegion(CEAC))
            {
                extracted->setName("task_" + Twine(taskId));
                extractedFunctions[taskId] = extracted;

                if (isLoopTask[taskId])
                {
                    llvm::nulls() << "  Task " << taskId << ": Extracted as " << extracted->getName()
                                  << " (LOOP with " << blockVec.size() << " blocks)\n";
                }
                else
                {
                    llvm::nulls() << "  Task " << taskId << ": Extracted as " << extracted->getName()
                                  << " (" << extracted->arg_size() << " args)\n";
                }
            }
            else
            {
                llvm::nulls() << "  Task " << taskId << ": Extraction failed - will run serially\n";
            }
        }

        llvm::nulls() << "✓ Code extraction complete\n\n";

        // Count successes
        unsigned numExtracted = 0;
        unsigned numLoops = 0;
        for (unsigned i = 0; i < extractedFunctions.size(); ++i)
        {
            if (extractedFunctions[i])
            {
                numExtracted++;
                if (isLoopTask[i])
                    numLoops++;
            }
        }

        // llvm::nulls() << "Successfully extracted " << numExtracted << " tasks\n";
        // llvm::nulls() << "  - " << numLoops << " loop tasks\n";
        // llvm::nulls() << "  - " << (numExtracted - numLoops) << " non-loop tasks\n\n";

        // // Step 3: Generate parallel wrapper with pthread support
        // llvm::nulls()
        //     << "Generating parallel wrapper with pthread support...\n";

        Type *Int32Ty = Type::getInt32Ty(Ctx);
        Type *Int64Ty = Type::getInt64Ty(Ctx);
        Type *VoidTy = Type::getVoidTy(Ctx);
        PointerType *VoidPtrTy = PointerType::get(Ctx, 0);

        // Step 3: Analyze arguments for each extracted task
        SmallVector<TaskArgumentInfo> taskArgInfo(TG.tasks.size());

        llvm::nulls() << "Analyzing task arguments...\n";
        for (unsigned taskId = 0; taskId < TG.tasks.size(); ++taskId)
        {
            Function *taskFunc = extractedFunctions[taskId];
            if (!taskFunc)
                continue;

            analyzeTaskArguments(taskFunc, mainFunc, taskArgInfo[taskId], Ctx);

            if (!taskArgInfo[taskId].originalValues.empty())
            {
                llvm::nulls() << "  Task " << taskId << ": "
                              << taskArgInfo[taskId].originalValues.size()
                              << " arguments captured\n";
            }
        }

        // Step 4: Create wrappers for extracted tasks
        SmallVector<Function *> wrapperFunctions(TG.tasks.size(), nullptr);

        llvm::nulls() << "Creating task wrappers...\n";
        for (unsigned taskId = 0; taskId < TG.tasks.size(); ++taskId)
        {
            Function *taskFunc = extractedFunctions[taskId];
            if (!taskFunc)
                continue;

            FunctionType *WrapperFT = FunctionType::get(VoidPtrTy, {VoidPtrTy}, false);
            Function *Wrapper = Function::Create(WrapperFT, Function::InternalLinkage,
                                                 "task_" + Twine(taskId) + "_wrapper", M);

            BasicBlock *WrapperBB = BasicBlock::Create(Ctx, "entry", Wrapper);
            IRBuilder<> Builder(WrapperBB);

            // Value *ThreadID = Builder.CreateCall(PthreadSelf);
            // Value *StartMsg = Builder.CreateGlobalStringPtr(
            //     "[THREAD START] Task " + std::to_string(taskId) + " on thread %ld\n");
            // Builder.CreateCall(Printf, {StartMsg, ThreadID});

            // CRITICAL: Extract arguments from the struct passed via pthread
            SmallVector<Value *> Args;

            TaskArgumentInfo &argInfo = taskArgInfo[taskId];
            if (argInfo.argStructType)
            {
                // We have arguments to extract from the struct
                Value *ArgVoidPtr = Wrapper->getArg(0);
                Value *ArgStructPtr = Builder.CreateBitCast(
                    ArgVoidPtr,
                    PointerType::get(argInfo.argStructType, 0),
                    "arg_struct_ptr");

                // Extract each field from the struct
                for (unsigned i = 0; i < argInfo.types.size(); ++i)
                {
                    Value *FieldPtr = Builder.CreateStructGEP(
                        argInfo.argStructType, ArgStructPtr, i,
                        "field_" + Twine(i) + "_ptr");
                    Value *FieldVal = Builder.CreateLoad(
                        argInfo.types[i], FieldPtr,
                        "field_" + Twine(i));
                    Args.push_back(FieldVal);
                }
            }
            else if (taskFunc->arg_size() > 0)
            {
                // No argument info but function expects args - use nulls as fallback
                llvm::nulls() << "Warning: Task " << taskId << " needs args but none captured\n";
                for (unsigned i = 0; i < taskFunc->arg_size(); ++i)
                {
                    Type *ArgTy = taskFunc->getFunctionType()->getParamType(i);
                    if (ArgTy->isPointerTy())
                    {
                        Args.push_back(ConstantPointerNull::get(cast<PointerType>(ArgTy)));
                    }
                    else if (ArgTy->isIntegerTy())
                    {
                        Args.push_back(ConstantInt::get(ArgTy, 0));
                    }
                    else
                    {
                        Args.push_back(UndefValue::get(ArgTy));
                    }
                }
            }

            Builder.CreateCall(taskFunc, Args);

            // Value *EndMsg = Builder.CreateGlobalStringPtr(
            //     "[THREAD END] Task " + std::to_string(taskId) + " completed\n");
            // Builder.CreateCall(Printf, {EndMsg});

            Builder.CreateRet(ConstantPointerNull::get(VoidPtrTy));

            wrapperFunctions[taskId] = Wrapper;
            llvm::nulls() << "  Created wrapper for task " << taskId << "\n";
        }

        // Region-aware, in-place scheduling (preserves original CFG/loop semantics).
        struct TaskRegionInfo
        {
            unsigned taskId = 0;
            const Loop *regionLoop = nullptr; // nullptr => ROOT
            SmallVector<CallInst *> anchorCallsites;
            bool ambiguousRegion = false;
        };
        struct CallGroup
        {
            unsigned levelIdx = 0;
            const Loop *regionLoop = nullptr;
            BasicBlock *bb = nullptr;
            int64_t workUnits = 0;
            int64_t spanUnits = 0;
            SmallVector<unsigned> taskIds;
            SmallVector<CallInst *> calls;
        };

        DenseMap<Function *, unsigned> extractedFuncToTaskId;
        for (unsigned taskId = 0; taskId < extractedFunctions.size(); ++taskId)
        {
            if (Function *F = extractedFunctions[taskId])
                extractedFuncToTaskId[F] = taskId;
        }

        DominatorTree DT(*mainFunc);
        LoopInfo LI(DT);
        DenseMap<Instruction *, unsigned> globalInstIndex;
        {
            unsigned globalIdx = 0;
            for (BasicBlock &BB : *mainFunc)
            {
                for (Instruction &I : BB)
                    globalInstIndex[&I] = globalIdx++;
            }
        }

        SmallVector<TaskRegionInfo> taskRegions(TG.tasks.size());
        for (unsigned taskId = 0; taskId < TG.tasks.size(); ++taskId)
            taskRegions[taskId].taskId = taskId;

        for (BasicBlock &BB : *mainFunc)
        {
            for (Instruction &I : BB)
            {
                auto *CI = dyn_cast<CallInst>(&I);
                if (!CI)
                    continue;
                Function *Callee = CI->getCalledFunction();
                if (!Callee)
                    continue;
                auto it = extractedFuncToTaskId.find(Callee);
                if (it == extractedFuncToTaskId.end())
                    continue;

                unsigned taskId = it->second;
                TaskRegionInfo &TR = taskRegions[taskId];
                TR.anchorCallsites.push_back(CI);

                const Loop *L = LI.getLoopFor(CI->getParent());
                if (TR.anchorCallsites.size() == 1)
                {
                    TR.regionLoop = L;
                }
                else if (TR.regionLoop != L)
                {
                    TR.ambiguousRegion = true;
                }
            }
        }

        SmallVector<CallGroup> groups;
        groups.reserve(levels.size());
        for (unsigned levelIdx = 0; levelIdx < levels.size(); ++levelIdx)
        {
            CallGroup G;
            G.levelIdx = levelIdx;
            for (unsigned taskId : levels[levelIdx])
            {
                if (taskId >= extractedFunctions.size() || !extractedFunctions[taskId])
                    continue;

                const TaskRegionInfo &TR = taskRegions[taskId];
                if (TR.anchorCallsites.size() != 1 || TR.ambiguousRegion)
                    continue;

                int64_t taskWork = estimateTaskWorkUnits(TG.tasks[taskId], PDG);
                G.taskIds.push_back(taskId);
                G.calls.push_back(TR.anchorCallsites[0]);
                G.workUnits += taskWork;
                if (taskWork > G.spanUnits)
                    G.spanUnits = taskWork;
            }
            if (!G.calls.empty())
                groups.push_back(std::move(G));
        }

        auto castValueForStore = [&](IRBuilder<> &B, Value *V, Type *Ty) -> Value *
        {
            if (V->getType() == Ty)
                return V;
            if (V->getType()->isPointerTy() && Ty->isPointerTy())
                return B.CreateBitCast(V, Ty);
            if (V->getType()->isIntegerTy() && Ty->isIntegerTy())
                return B.CreateIntCast(V, Ty, true);
            if (V->getType()->isFloatingPointTy() && Ty->isFloatingPointTy())
                return B.CreateFPCast(V, Ty);
            if (V->getType()->isIntegerTy() && Ty->isFloatingPointTy())
                return B.CreateSIToFP(V, Ty);
            if (V->getType()->isFloatingPointTy() && Ty->isIntegerTy())
                return B.CreateFPToSI(V, Ty);
            return nullptr;
        };

        auto isLocallyLaunchable = [&](unsigned taskId, CallInst *CI) -> bool
        {
            if (!CI || !wrapperFunctions[taskId])
                return false;
            if (!(CI->getType()->isVoidTy() || CI->use_empty()))
                return false;

            TaskArgumentInfo &argInfo = taskArgInfo[taskId];
            if (argInfo.argStructType)
                return argInfo.types.size() == CI->arg_size();
            return CI->arg_size() == 0;
        };

        auto touchesMutableGlobals = [&](Function *Root) -> bool
        {
            if (!Root || Root->isDeclaration())
                return false;

            DenseSet<Function *> visitedFunctions;
            SmallVector<Function *, 8> worklist;
            worklist.push_back(Root);

            auto referencesMutableGlobal = [](Value *V) -> bool
            {
                if (!V)
                    return false;
                Value *base = V->stripPointerCasts();
                auto *GV = dyn_cast<GlobalVariable>(base);
                return GV && !GV->isConstant();
            };

            while (!worklist.empty())
            {
                Function *F = worklist.pop_back_val();
                if (!F || F->isDeclaration() || !visitedFunctions.insert(F).second)
                    continue;

                for (BasicBlock &BB : *F)
                {
                    for (Instruction &I : BB)
                    {
                        if (auto *LI = dyn_cast<LoadInst>(&I))
                        {
                            if (referencesMutableGlobal(LI->getPointerOperand()))
                                return true;
                        }
                        else if (auto *SI = dyn_cast<StoreInst>(&I))
                        {
                            if (referencesMutableGlobal(SI->getPointerOperand()))
                                return true;
                        }
                        else if (auto *CB = dyn_cast<CallBase>(&I))
                        {
                            for (Value *Arg : CB->args())
                            {
                                if (referencesMutableGlobal(Arg))
                                    return true;
                            }
                            if (Function *Callee = CB->getCalledFunction())
                                worklist.push_back(Callee);
                        }
                    }
                }
            }

            return false;
        };

        DenseMap<unsigned, bool> taskTouchesMutableGlobals;

        auto isHoistSafe = [&](unsigned taskId, CallInst *CI, Instruction *launchPoint) -> bool
        {
            if (!isLocallyLaunchable(taskId, CI))
                return false;

            auto it = taskTouchesMutableGlobals.find(taskId);
            if (it == taskTouchesMutableGlobals.end())
                it = taskTouchesMutableGlobals.insert({taskId, touchesMutableGlobals(wrapperFunctions[taskId])}).first;
            bool touchesGlobals = it->second;
            if (launchPoint != CI && touchesGlobals)
                return false;

            TaskArgumentInfo &argInfo = taskArgInfo[taskId];
            if (!argInfo.argStructType)
                return true;

            for (unsigned a = 0; a < CI->arg_size(); ++a)
            {
                Value *op = CI->getArgOperand(a);
                if (auto *defI = dyn_cast<Instruction>(op))
                {
                    if (!DT.dominates(defI, launchPoint))
                        return false;
                }
            }
            return true;
        };

        auto getInstOrder = [&](Instruction *I) -> unsigned
        {
            auto it = globalInstIndex.find(I);
            if (it == globalInstIndex.end())
                return std::numeric_limits<unsigned>::max();
            return it->second;
        };

        struct SlotSetupInfo
        {
            AllocaInst *slotAlloca = nullptr;
            IntrinsicInst *lifetimeStart = nullptr;
            Instruction *firstConsumer = nullptr;
        };

        auto analyzePointerSlot = [&](Value *ptrValue,
                                      CallInst *producerCall,
                                      SlotSetupInfo &info) -> bool
        {
            Value *base = ptrValue ? ptrValue->stripPointerCasts() : nullptr;
            auto *AI = dyn_cast_or_null<AllocaInst>(base);
            if (!AI)
                return true;

            info.slotAlloca = AI;

            SmallVector<Instruction *, 4> starts;
            SmallVector<Instruction *, 4> ends;
            SmallVector<Instruction *, 8> consumers;
            SmallVector<Value *, 8> worklist;
            DenseSet<Value *> visited;
            worklist.push_back(AI);

            while (!worklist.empty())
            {
                Value *V = worklist.pop_back_val();
                if (!visited.insert(V).second)
                    continue;

                for (User *U : V->users())
                {
                    if (auto *II = dyn_cast<IntrinsicInst>(U))
                    {
                        if (II->getIntrinsicID() == Intrinsic::lifetime_start)
                            starts.push_back(II);
                        else if (II->getIntrinsicID() == Intrinsic::lifetime_end)
                            ends.push_back(II);
                        continue;
                    }

                    auto *I = dyn_cast<Instruction>(U);
                    if (!I)
                        continue;

                    if (isa<BitCastInst>(I) || isa<GetElementPtrInst>(I) ||
                        isa<AddrSpaceCastInst>(I) || isa<PHINode>(I) || isa<SelectInst>(I))
                    {
                        worklist.push_back(I);
                        continue;
                    }

                    if ((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
                        getInstOrder(I) > getInstOrder(producerCall))
                        consumers.push_back(I);
                }
            }

            bool hasManagedLifetime = !starts.empty() || !ends.empty() || !consumers.empty();
            if (!hasManagedLifetime)
                return true;

            llvm::sort(starts, [&](Instruction *A, Instruction *B)
                       { return getInstOrder(A) < getInstOrder(B); });
            llvm::sort(ends, [&](Instruction *A, Instruction *B)
                       { return getInstOrder(A) < getInstOrder(B); });
            llvm::sort(consumers, [&](Instruction *A, Instruction *B)
                       { return getInstOrder(A) < getInstOrder(B); });

            if (starts.size() != 1 || ends.empty())
                return false;

            unsigned firstEndOrder = getInstOrder(ends.front());
            for (Instruction *startI : starts)
            {
                if (startI != starts.front() && getInstOrder(startI) < firstEndOrder)
                    return false;
            }

            info.lifetimeStart = cast<IntrinsicInst>(starts.front());
            info.firstConsumer = ends.front();
            if (!consumers.empty() &&
                getInstOrder(consumers.front()) < getInstOrder(info.firstConsumer))
                info.firstConsumer = consumers.front();

            return getInstOrder(info.firstConsumer) > getInstOrder(info.lifetimeStart);
        };

        auto collectLaunchSetups = [&](unsigned taskId,
                                       CallInst *CI,
                                       Instruction *launchPoint,
                                       SmallVectorImpl<IntrinsicInst *> &requiredStarts) -> bool
        {
            if (!isHoistSafe(taskId, CI, launchPoint))
                return false;

            for (unsigned a = 0; a < CI->arg_size(); ++a)
            {
                Value *op = CI->getArgOperand(a);
                if (!op || !op->getType()->isPointerTy())
                    continue;

                SlotSetupInfo slotInfo;
                if (!analyzePointerSlot(op, CI, slotInfo))
                    return false;
                if (!slotInfo.lifetimeStart)
                    continue;
                if (slotInfo.firstConsumer &&
                    getInstOrder(slotInfo.firstConsumer) <= getInstOrder(launchPoint))
                    return false;

                if (std::find(requiredStarts.begin(), requiredStarts.end(), slotInfo.lifetimeStart) == requiredStarts.end())
                    requiredStarts.push_back(slotInfo.lifetimeStart);
            }
            return true;
        };

        auto collectTaskLoopIds = [&](llvm::Function *Root) {
            llvm::SmallVector<int32_t, 4> loopIds;
            llvm::DenseSet<int32_t> seenLoopIds;
            llvm::DenseSet<llvm::Function *> visitedFunctions;
            const llvm::StringRef LoopDescPrefix = "sgpl.loop.desc.";
            std::function<void(llvm::Value *)> inspectValue;
            std::function<void(llvm::Function *)> inspectFunction;

            inspectValue = [&](llvm::Value *V) {
                if (!V)
                    return;

                if (auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(V))
                {
                    llvm::StringRef Name = GV->getName();
                    if (Name.starts_with(LoopDescPrefix))
                    {
                        llvm::StringRef Suffix = Name.substr(LoopDescPrefix.size());
                        unsigned loopId = 0;
                        if (!Suffix.getAsInteger(10, loopId) && seenLoopIds.insert((int32_t)loopId).second)
                            loopIds.push_back((int32_t)loopId);
                    }
                    return;
                }

                if (auto *C = llvm::dyn_cast<llvm::Constant>(V))
                {
                    for (llvm::Value *Op : C->operands())
                        inspectValue(Op);
                }
            };

            inspectFunction = [&](llvm::Function *F) {
                if (!F || F->isDeclaration() || !visitedFunctions.insert(F).second)
                    return;

                for (llvm::BasicBlock &BB : *F)
                {
                    for (llvm::Instruction &I : BB)
                    {
                        for (llvm::Value *Op : I.operands())
                            inspectValue(Op);
                        if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I))
                            inspectFunction(CB->getCalledFunction());
                    }
                }
            };

            inspectFunction(Root);
            llvm::sort(loopIds);
            return loopIds;
        };

        llvm::DenseMap<unsigned, llvm::SmallVector<int32_t, 4>> taskLoopIdCache;
        PointerType *Int32PtrTy = Int32Ty->getPointerTo();
        StructType *TDGTaskDescTy = StructType::get(Ctx, {VoidPtrTy, VoidPtrTy, Int32Ty, Int32Ty, Int32Ty, Int32PtrTy});
        FunctionType *RunTDGLevelFT = FunctionType::get(
            VoidTy,
            {VoidPtrTy, Int32Ty, Int64Ty, Int64Ty},
            false);
        FunctionCallee RunTDGLevel = M.getOrInsertFunction("sgpl_run_tdg_level", RunTDGLevelFT);

        unsigned transformedGroups = 0;
        for (CallGroup &G : groups)
        {
            SmallVector<std::pair<unsigned, unsigned>> order;
            SmallVector<unsigned> includedTaskIds;
            SmallVector<CallInst *> includedCalls;
            SmallVector<int64_t> includedWorks;
            SmallVector<IntrinsicInst *> requiredStarts;
            Instruction *launchAnchor = nullptr;
            int64_t launchWorkUnits = 0;
            int64_t launchSpanUnits = 0;

            order.reserve(G.calls.size());
            for (unsigned i = 0; i < G.calls.size(); ++i)
            {
                auto it = globalInstIndex.find(G.calls[i]);
                if (it == globalInstIndex.end())
                    continue;
                order.push_back({it->second, i});
            }
            if (order.empty())
                continue;

            llvm::sort(order, [](const auto &a, const auto &b)
                       { return a.first < b.first; });

            for (const auto &ord : order)
            {
                unsigned localIdx = ord.second;
                unsigned taskId = G.taskIds[localIdx];
                CallInst *CI = G.calls[localIdx];
                if (isLocallyLaunchable(taskId, CI))
                {
                    launchAnchor = CI;
                    break;
                }
            }

            if (!launchAnchor)
                continue;

            for (const auto &ord : order)
            {
                unsigned localIdx = ord.second;
                unsigned taskId = G.taskIds[localIdx];
                CallInst *CI = G.calls[localIdx];
                int64_t taskWork = estimateTaskWorkUnits(TG.tasks[taskId], PDG);
                SmallVector<IntrinsicInst *, 4> taskStarts;

                if (!collectLaunchSetups(taskId, CI, launchAnchor, taskStarts))
                    continue;

                includedTaskIds.push_back(taskId);
                includedCalls.push_back(CI);
                includedWorks.push_back(taskWork);
                launchWorkUnits += taskWork;
                if (taskWork > launchSpanUnits)
                    launchSpanUnits = taskWork;

                for (IntrinsicInst *startI : taskStarts)
                {
                    if (std::find(requiredStarts.begin(), requiredStarts.end(), startI) == requiredStarts.end())
                        requiredStarts.push_back(startI);
                }
            }

            if (includedCalls.empty())
                continue;

            if (tdgDebugEnabled())
            {
                errs() << "[tdg.group] level=" << G.levelIdx
                       << " grouped_tasks=" << includedCalls.size()
                       << " total_calls=" << G.calls.size()
                       << " work=" << launchWorkUnits
                       << " span=" << launchSpanUnits
                       << " anchor=";
                launchAnchor->print(errs());
                errs() << "\n";

                for (unsigned idx = 0; idx < includedTaskIds.size(); ++idx)
                {
                    errs() << "[tdg.static] level=" << G.levelIdx
                           << " idx=" << idx
                           << " task_id=" << includedTaskIds[idx]
                           << " static_work=" << includedWorks[idx]
                           << " call=";
                    includedCalls[idx]->print(errs());
                    errs() << "\n";
                }
            }

            llvm::sort(requiredStarts, [&](Instruction *A, Instruction *B)
                       { return getInstOrder(A) < getInstOrder(B); });

            IRBuilder<> B(launchAnchor);
            SmallVector<Instruction *, 8> eraseStarts;
            for (IntrinsicInst *startI : requiredStarts)
            {
                if (!startI)
                    continue;
                if (getInstOrder(startI) <= getInstOrder(launchAnchor))
                    continue;
                Instruction *cloned = startI->clone();
                B.Insert(cloned);
                eraseStarts.push_back(startI);
            }

            ArrayType *TDGTaskArrayTy = ArrayType::get(TDGTaskDescTy, includedCalls.size());
            AllocaInst *TDGTaskArray = B.CreateAlloca(
                TDGTaskArrayTy,
                nullptr,
                "tdg_level_" + Twine(G.levelIdx) + "_tasks");

            for (unsigned idx = 0; idx < includedCalls.size(); ++idx)
            {
                unsigned taskId = includedTaskIds[idx];
                CallInst *CI = includedCalls[idx];
                TaskArgumentInfo &argInfo = taskArgInfo[taskId];

                Value *TaskDescPtr = B.CreateGEP(
                    TDGTaskArrayTy, TDGTaskArray,
                    {B.getInt32(0), B.getInt32((int)idx)});
                Value *FnFieldPtr = B.CreateStructGEP(TDGTaskDescTy, TaskDescPtr, 0);
                Value *ArgFieldPtr = B.CreateStructGEP(TDGTaskDescTy, TaskDescPtr, 1);
                Value *ProfileFieldPtr = B.CreateStructGEP(TDGTaskDescTy, TaskDescPtr, 2);
                Value *WorkFieldPtr = B.CreateStructGEP(TDGTaskDescTy, TaskDescPtr, 3);
                Value *LoopCountFieldPtr = B.CreateStructGEP(TDGTaskDescTy, TaskDescPtr, 4);
                Value *LoopIdsFieldPtr = B.CreateStructGEP(TDGTaskDescTy, TaskDescPtr, 5);
                Value *LoopIdsPtr = ConstantPointerNull::get(Int32PtrTy);
                int32_t LoopIdCount = 0;

                Value *TaskArg = ConstantPointerNull::get(VoidPtrTy);
                if (argInfo.argStructType)
                {
                    AllocaInst *ArgStruct = B.CreateAlloca(
                        argInfo.argStructType,
                        nullptr,
                        "task_" + Twine(taskId) + "_args");
                    bool argSafe = true;
                    for (unsigned a = 0; a < CI->arg_size(); ++a)
                    {
                        Value *FieldPtr = B.CreateStructGEP(argInfo.argStructType, ArgStruct, a);
                        Value *stored = castValueForStore(B, CI->getArgOperand(a), argInfo.types[a]);
                        if (!stored)
                        {
                            argSafe = false;
                            break;
                        }
                        B.CreateStore(stored, FieldPtr);
                    }
                    if (!argSafe)
                    {
                        includedCalls[idx] = nullptr;
                        continue;
                    }
                    TaskArg = B.CreateBitCast(ArgStruct, VoidPtrTy);
                }

                if (!taskLoopIdCache.count(taskId))
                    taskLoopIdCache[taskId] = collectTaskLoopIds(wrapperFunctions[taskId]);

                if (!taskLoopIdCache[taskId].empty())
                {
                    ArrayType *LoopIdsTy = ArrayType::get(Int32Ty, taskLoopIdCache[taskId].size());
                    AllocaInst *LoopIdsAlloca = B.CreateAlloca(LoopIdsTy, nullptr, "task_" + Twine(taskId) + "_loop_ids");
                    for (unsigned loopIdx = 0; loopIdx < taskLoopIdCache[taskId].size(); ++loopIdx)
                    {
                        Value *LoopIdPtr = B.CreateGEP(LoopIdsTy,
                                                       LoopIdsAlloca,
                                                       {B.getInt32(0), B.getInt32((int)loopIdx)});
                        B.CreateStore(B.getInt32(taskLoopIdCache[taskId][loopIdx]), LoopIdPtr);
                    }
                    LoopIdsPtr = B.CreateGEP(LoopIdsTy,
                                             LoopIdsAlloca,
                                             {B.getInt32(0), B.getInt32(0)});
                    LoopIdCount = (int32_t)taskLoopIdCache[taskId].size();
                }

                B.CreateStore(B.CreateBitCast(wrapperFunctions[taskId], VoidPtrTy), FnFieldPtr);
                B.CreateStore(TaskArg, ArgFieldPtr);
                B.CreateStore(B.getInt32((int32_t)taskId), ProfileFieldPtr);
                B.CreateStore(B.getInt32((int32_t)std::min<int64_t>(includedWorks[idx], (int64_t)std::numeric_limits<int32_t>::max())), WorkFieldPtr);
                B.CreateStore(B.getInt32(LoopIdCount), LoopCountFieldPtr);
                B.CreateStore(LoopIdsPtr, LoopIdsFieldPtr);
            }

            SmallVector<CallInst *> eraseCalls;
            eraseCalls.reserve(includedCalls.size());
            unsigned actualCount = 0;
            launchWorkUnits = 0;
            launchSpanUnits = 0;

            for (unsigned idx = 0; idx < includedCalls.size(); ++idx)
            {
                if (!includedCalls[idx])
                    continue;
                eraseCalls.push_back(includedCalls[idx]);
                actualCount++;
                launchWorkUnits += includedWorks[idx];
                if (includedWorks[idx] > launchSpanUnits)
                    launchSpanUnits = includedWorks[idx];
            }

            if (actualCount == 0)
                continue;

            Value *TDGTaskBase = B.CreateGEP(
                TDGTaskArrayTy, TDGTaskArray,
                {B.getInt32(0), B.getInt32(0)});
            B.CreateCall(
                RunTDGLevel,
                {B.CreateBitCast(TDGTaskBase, VoidPtrTy),
                 B.getInt32((int32_t)actualCount),
                 ConstantInt::get(Int64Ty, launchWorkUnits),
                 ConstantInt::get(Int64Ty, launchSpanUnits)});

            llvm::sort(eraseStarts, [&](Instruction *A, Instruction *B)
                       { return getInstOrder(A) > getInstOrder(B); });
            for (Instruction *I : eraseStarts)
                I->eraseFromParent();

            for (auto it = eraseCalls.rbegin(); it != eraseCalls.rend(); ++it)
                (*it)->eraseFromParent();

            transformedGroups++;
        }

        llvm::nulls() << "✓ CFG-preserving scheduler applied in-place: "
                      << transformedGroups << " runtime TDG regions transformed\n";
    }

} // namespace llvm

// canonical (uppercase) entry required by LLVM examples
extern "C" __attribute__((visibility("default"))) ::llvm::PassPluginLibraryInfo LLVMGetPassPluginInfo()
{
    return getPDGBuilderPassPluginInfo();
}

// older/alternate symbol some loaders look for (lowercase) — alias to the canonical one
extern "C" __attribute__((visibility("default"))) ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return LLVMGetPassPluginInfo();
}
