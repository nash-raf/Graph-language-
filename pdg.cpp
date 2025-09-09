#include "pdg.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream> // for DOT output
#include <iostream>

// global collector (keeps a single combined graph you can print later)
static llvm::dependencyGraph gCollected;

static unsigned nextID = 0;

void printInstruction(llvm::Instruction *I)
{
    I->print(llvm::errs()); // prints to stderr
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
    llvm::outs() << "EDGE: ";
    I1->print(llvm::errs());
    llvm::outs() << " ----> ";
    I2->print(llvm::errs());

    llvm::errs() << " (" << edge.type << ")\n";
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

std::string getKeyForValue(int V, llvm::dependencyGraph &G)
{
    std::string s;
    llvm::raw_string_ostream rso(s);
    for (auto &KV : G.nodes)
    {
        if (KV.second == V)
        {
            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(KV.first);
            inst->print(rso);
            rso.flush();
            return s;
        }
    }
    // handle not found
    return "null";
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

void buildGraph(llvm::Function &F, llvm::DependenceInfo DI, llvm::dependencyGraph &G, llvm::MemorySSA &MSSA)
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
            //     llvm::outs() << "NO EDGE\n";
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

    for (auto LI : loadCollector)
    {
        if (llvm::MemoryAccess *MA = MSSA.getMemoryAccess(LI))
        {
            if (auto *Clobber = Walker->getClobberingMemoryAccess(MA))
            {
                if (auto *MD = llvm::dyn_cast<llvm::MemoryDef>(Clobber))
                {
                    if (auto *SI = llvm::dyn_cast<llvm::StoreInst>(MD->getMemoryInst()))
                    {
                        createEdge(SI, LI, G, "RAW");
                    }
                }
            }
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
            buildGraph(F, DAResult, G, MSSA);
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

            // Example: print it
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
