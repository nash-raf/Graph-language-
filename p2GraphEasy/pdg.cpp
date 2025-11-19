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
#include "llvm/Analysis/ScalarEvolutionExpressions.h" // <-- Add this include
#include <string>
#include <chrono> //added
using namespace std::chrono; //added
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"

// global collector (keeps a single combined graph you can print later)
static llvm::dependencyGraph gCollected;

static unsigned nextID = 0;

void printInstruction(llvm::Instruction *I)
{
    // I->print(llvm::errs()); // prints to stderr
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

void createEdge(llvm::Instruction *I1, llvm::Instruction *I2, llvm::dependencyGraph &G, std::string type)
{
    llvm::depEdge edge;
    // printInstruction(I1);
    // llvm::errs() << "\n";

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
    // llvm::outs()() << "EDGE: ";
    // I1->print(llvm::errs());
    // llvm::outs()() << " ----> ";
    // I2->print(llvm::errs());

    // llvm::errs() << " (" << edge.type << ")\n";
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

// replace printCollectedGraphDot with this
void printCollectedGraphDot(llvm::dependencyGraph &G,
                            const std::string &filename = "pdg_global.dot")
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
    {
        llvm::errs() << "Failed to open " << filename << " for writing\n";
        return;
    }
    ofs << "digraph PDG {\n";

    // helper for stable node name
    auto nodeName = [](int id)
    {
        return std::string("n") + std::to_string(id);
    };

    // nodes: emit stable names as identifiers and instruction text as label
    for (auto &entry : G.nodes)
    {
        llvm::Instruction *I = entry.first;
        int id = entry.second;
        std::string s;
        llvm::raw_string_ostream rso(s);
        I->print(rso);
        rso.flush();
        ofs << "  \"" << nodeName(id) << "\" [label=\"" << escapeForDot(s) << "\"];\n";
    }

    // edges: reference the same stable names
    for (auto &e : G.edges)
    {
        ofs << "  \"" << nodeName(e.source) << "\" -> \"" << nodeName(e.target)
            << "\" [label=\"" << escapeForDot(e.type) << "\"];\n";
    }

    ofs << "}\n";
    ofs.close();
    llvm::errs() << "Wrote " << filename << "\n";
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


    bool sawLoopCarried = false;
    bool allCarriedHaveConstantPositiveDistance = true; // constant and >= 1 required for DOACROSS
    bool anyUnknown = false;
    bool hasZeroDistance = false;
    bool hasNonPositiveDistance = false;
    bool phiCarry = false;

    // --- PHI handling: only mark phiCarry when the PHI is NOT an induction variable (SCEV AddRec) ---

    for (BasicBlock *BB : L->blocks())
    {
        for (Instruction &I : *BB)
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
                        break;
                    }
                }
                if (phiCarry)
                    break;
            }
        }
        if (phiCarry)
            break;
    }

    // 33

    for (auto E : G.edges)
    {
        std::pair<llvm::Instruction *, llvm::Instruction *> edge = getKeyForValue(E, G);

        // // llvm::outs()() << edge.first << " " << edge.second << "\n";

        if (L->contains(edge.first->getParent()) && L->contains(edge.second->getParent()))
        {

            std::unique_ptr<Dependence> Dep = DI.depends(edge.first, edge.second, /*AllowTrans=*/false);
            if (!Dep)
                continue;

            // edge.first->print(llvm::outs());
            // edge.second->print(llvm::outs());
            // Dep->dump(llvm::errs());

            // if (!Dep->isFlow())
            // {
            //     continue;
            // }
            if (Dep->isLoopIndependent())
            {
                continue;
            }

            // llvm::outs() << "trigger4\n";
            // llvm::outs() << G.nodes[edge.first] << " " << G.nodes[edge.second] << "\n";

            sawLoopCarried = true;

            const SCEV *Dist = Dep->getDistance(1); // innermost
            if (!Dist)
            {
                anyUnknown = true;
                allCarriedHaveConstantPositiveDistance = false;
                // llvm::outs() << "  -> distance unknown\n";
                continue;
            }

            // llvm::outs() << "  -> Distance SCEV: ";
            // Dist->print(llvm::outs());
            // llvm::outs() << "\n";

            if (const SCEVConstant *C = dyn_cast<SCEVConstant>(Dist))
            {
                if (const ConstantInt *CI = dyn_cast<ConstantInt>(C->getValue()))
                {
                    int64_t d = CI->getSExtValue();
                    // llvm::outs() << "  -> constant distance = " << d << "\n";
                    if (d == 0)
                    {
                        hasZeroDistance = true;
                        allCarriedHaveConstantPositiveDistance = false;
                        // llvm::outs() << "    -> zero distance => sequential\n";
                    }
                    else if (d < 1)
                    {
                        hasNonPositiveDistance = true;
                        allCarriedHaveConstantPositiveDistance = false;
                        // llvm::outs() << "    -> non-positive distance => sequential/unsupported for DOACROSS\n";
                    }
                }
                else
                {
                    anyUnknown = true;
                    allCarriedHaveConstantPositiveDistance = false;
                    // llvm::outs() << "  -> SCEVConstant but not ConstantInt -> unknown\n";
                }
            }
            else
            {
                allCarriedHaveConstantPositiveDistance = false;
                // llvm::outs() << "  -> distance not a SCEVConstant (non-constant) => treat as unknown/non-DOACROSS\n";
            }
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
    //         llvm::outs() << "triggering ";
    //         std::unique_ptr<Dependence> Dep = DI.depends(Src, Dst, /*AllowTrans=*/false);
    //         if (!Dep)
    //             continue;

    //         // debug
    //         llvm::outs() << "Dependence between: ";
    //         Src->print(llvm::outs());
    //         llvm::outs() << " -> ";
    //         Dst->print(llvm::outs());
    //         llvm::outs() << "\n";
    //         Dep->dump(llvm::errs());
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
    //             llvm::outs() << "  -> distance unknown\n";
    //             createEdge(Src, Dst, G, "DEP");
    //             continue;
    //         }

    //         // Print SCEV for debug
    //         llvm::outs() << "  -> Distance SCEV: ";
    //         Dist->print(llvm::outs());
    //         llvm::outs() << "\n";

    //         if (const SCEVConstant *C = dyn_cast<SCEVConstant>(Dist))
    //         {
    //             if (const ConstantInt *CI = dyn_cast<ConstantInt>(C->getValue()))
    //             {
    //                 int64_t d = CI->getSExtValue();
    //                 llvm::outs() << "  -> constant distance = " << d << "\n";
    //                 if (d == 0)
    //                 {
    //                     hasZeroDistance = true;
    //                     allCarriedHaveConstantPositiveDistance = false;
    //                     llvm::outs() << "    -> zero distance => sequential\n";
    //                 }
    //                 else if (d < 1)
    //                 {
    //                     hasNonPositiveDistance = true;
    //                     allCarriedHaveConstantPositiveDistance = false;
    //                     llvm::outs() << "    -> non-positive distance => sequential/unsupported for DOACROSS\n";
    //                 }
    //             }
    //             else
    //             {
    //                 anyUnknown = true;
    //                 allCarriedHaveConstantPositiveDistance = false;
    //                 llvm::outs() << "  -> SCEVConstant but not ConstantInt -> unknown\n";
    //             }
    //         }
    //         else
    //         {
    //             allCarriedHaveConstantPositiveDistance = false;
    //             llvm::outs() << "  -> distance not a SCEVConstant (non-constant) => treat as unknown/non-DOACROSS\n";
    //         }

    //         createEdge(Src, Dst, G, "DEP");
    //     }
    // }

    // final classification
    std::string classification;
    if (phiCarry)
    {
        classification = "SEQUENTIAL";
    }
    else if (anyUnknown || hasZeroDistance || hasNonPositiveDistance || !allCarriedHaveConstantPositiveDistance)
    {
        classification = "SEQUENTIAL";
    }
    else if (sawLoopCarried && allCarriedHaveConstantPositiveDistance)
    {
        classification = "DOACROSS";
    }
    else
    {
        classification = "DOALL";
    }
    // llvm::outs()() << "\n\n\nLoop header ";
    // llvm::outs()() << " classified as " << classification << "\n";

    // attach tiny custom metadata to loop latches so we can inspect IR
    LLVMContext &Ctx = F.getContext();
    MDNode *Node = MDNode::get(Ctx, MDString::get(Ctx, std::string("parallel.type=") + classification));

    llvm::BasicBlock *NN = L->getHeader();
    if (NN)
    {
        if (llvm::Instruction *MM = NN->getTerminator())
        {
            MM->setMetadata("my.loop.parallel", Node);
        }
    };

    // llvm::outs()() << "Loop header ";
    // L->getHeader()->printAsOperand(llvm::outs(), false);
    // llvm::outs() << " classified as " << classification << "\n";

    return classification;
}

static std::string analyzeAndAnnotateLoop(llvm::Loop *L, llvm::Function &F,
                                          llvm::DependenceInfo &DI,
                                          llvm::ScalarEvolution &SE,
                                          llvm::dependencyGraph &G);

void buildGraph(llvm::Function &F,
                llvm::DependenceInfo &DI,
                llvm::LoopInfo &LI,
                llvm::ScalarEvolution &SE,
                llvm::dependencyGraph &G,
                llvm::MemorySSA &MSSA)
{

    llvm::SmallVector<llvm::LoadInst *> loadCollector;
    llvm::SmallVector<llvm::StoreInst *> storeCollector;

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
            // llvm::errs() << " ";
            // if (I.use_empty())
            // {
            //     // llvm::outs()() << "NO EDGE\n";
            // }
            for (llvm::Value *U : I.users())
            {
                if (auto *UI = llvm::dyn_cast<llvm::Instruction>(U))
                {
                    // printInstruction(UI);
                    // llvm::errs() << "\n";
                    createEdge(&I, UI, G, "RAW");
                }
            }
        }
    }


    auto *Walker = MSSA.getWalker();

    //auto start = high_resolution_clock::now();

    for (auto LI : loadCollector)
    {
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
        }
    }

    //auto end = high_resolution_clock::now();
    //auto duration = duration_cast<milliseconds>(end - start).count();
    //llvm::outs() << "Loop analysis took " << duration << " ms\n";

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
}

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

            // Call the updated buildGraph that accepts DI, LI, SE by reference
            buildGraph(F, DAResult, LI, SE, G, MSSA);

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

            printCollectedGraphDot(G, "pdg_global.dot");

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