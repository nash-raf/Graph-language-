// main.cpp
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include "antlr4-runtime.h"
#include "BaseLexer.h"
#include "BaseParser.h"
#include "ASTBuilder.h"
#include "ASTNode.h"
#include "IRGenVisitor.h"

#include "pdg.h"
#include "parallel_loop_outline.h"
#include "graph_frontier_lowering.h"
#include "AutoTunerPass.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/CommandLine.h>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/LoopSimplify.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/TargetParser/Host.h>

#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/CodeGen.h> // CodeGenFileType, CodeGenOptLevel
#include <llvm/IR/LegacyPassManager.h>

#include <polly/RegisterPasses.h>
#include <polly/ScopDetection.h>
#include <polly/LinkAllPasses.h>
#include <llvm/TargetParser/SubtargetFeature.h>

#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/SourceMgr.h>

#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "SemanticAnalyzer.h"

#include <llvm/Analysis/AssumptionCache.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/IR/Dominators.h>

using namespace antlr4;
using namespace llvm;

// Not cl::Required: the Polly flag parse deliberately passes only flags, so a
// required positional would abort before the filename is recovered from argv.
static cl::opt<std::string> InputFilename(
    cl::Positional,
    cl::desc("<input-graph-file>"),
    cl::init(""));

static cl::opt<std::string> IRBackendOption(
    "ir-backend",
    cl::desc("IR backend to select: auto|cpu|gpu"),
    cl::value_desc("auto|cpu|gpu"),
    cl::init("auto"));

static cl::opt<bool> GpuFlag(
    "gpu",
    cl::desc("Run DOALL loops on the GPU when a usable GPU is detected"),
    cl::init(false));

static cl::opt<bool> EmitIRBackendChoice(
    "print-ir-backend",
    cl::desc("Print selected IR backend path to stderr"),
    cl::init(false));

static cl::opt<std::string> EmitIRTo(
    "emit-ir-to",
    cl::desc("Optional path to additionally dump emitted LLVM IR"),
    cl::init(""));

// Compatibility alias used by existing scripts.
static cl::opt<bool> PrintIRAlias(
    "print-ir",
    cl::desc("Compatibility alias with existing run scripts."),
    cl::init(false),
    cl::Hidden);

static bool isTruthyEnv(const char *name)
{
    const char *raw = std::getenv(name);
    if (!raw)
        return false;

    const char *truthy[] = {"1", "true", "TRUE", "yes", "YES", "on", "ON"};
    for (const char *value : truthy)
    {
        if (std::strcmp(raw, value) == 0)
            return true;
    }
    return false;
}

static bool commandSucceeds(const std::string &command)
{
    const std::string probe = command + " >/dev/null 2>&1";
    return std::system(probe.c_str()) == 0;
}

static bool detectUsableGPU()
{
    return commandSucceeds("nvidia-smi -L") ||
           commandSucceeds("rocm-smi -i 0 --showproductname") ||
           commandSucceeds("clinfo");
}

static std::string resolveBackend(std::string &backendReason)
{
    const std::string chosen = IRBackendOption;
    const bool forceCPU = isTruthyEnv("FORCE_CPU");
    const bool forceGPU = isTruthyEnv("FORCE_GPU");
    const bool usableGpu = detectUsableGPU();

    bool requestGpu = false;
    if (forceCPU && forceGPU)
    {
        requestGpu = false;
        backendReason = "FORCE_CPU=1 (takes precedence over FORCE_GPU=1 and --ir-backend)";
    }
    else if (forceCPU)
    {
        requestGpu = false;
        backendReason = "FORCE_CPU=1";
    }
    else if (forceGPU)
    {
        requestGpu = true;
        backendReason = "FORCE_GPU=1";
    }
    else if (GpuFlag)
    {
        requestGpu = true;
        backendReason = "--gpu";
    }
    else if (chosen == "cpu")
    {
        requestGpu = false;
        backendReason = "--ir-backend=cpu (forced)";
    }
    else if (chosen == "gpu")
    {
        requestGpu = true;
        backendReason = "--ir-backend=gpu (forced)";
    }
    else
    {
        requestGpu = usableGpu;
        backendReason = requestGpu ? "--ir-backend=auto (GPU detected)" : "--ir-backend=auto (CPU default)";
    }

    if (requestGpu && !usableGpu)
    {
        backendReason += "; requested GPU backend but no usable GPU detected, falling back to CPU";
        return "cpu";
    }

    return requestGpu ? "gpu" : "cpu";
}

static void writeBitcodeToFile(Module &M, const std::string &path)
{
    std::error_code EC;
    raw_fd_ostream Out(path, EC, sys::fs::OF_None);
    if (EC)
    {
        errs() << "Error opening '" << path << "' for writing: " << EC.message() << "\n";
        return;
    }
    WriteBitcodeToFile(M, Out);
    Out.flush();
}

// The DSL emits loops whose induction variable is carried through an alloca
// (load/store each iteration) and whose arrays are indexed through an i32
// trunc of that value. SCEV cannot analyze the load (no AddRec) and the
// trunc/sext-wrapped index makes DependenceInfo report an unknown direction,
// so such loops are classified SEQUENTIAL even when every iteration writes a
// distinct array element. Before the PDG runs, canonicalize the IR: promote
// the indvars to SSA phis and index GEPs with the induction phi directly.
static void canonicalizeLoopsForAnalysis(Module &M)
{
    {
        FunctionAnalysisManager FAM;
        PassBuilder PB;
        PB.registerFunctionAnalyses(FAM);
        FunctionPassManager FPM;
        FPM.addPass(llvm::PromotePass());
        for (Function &F : M)
            if (!F.isDeclaration())
                FPM.run(F, FAM);
    }

    for (Function &F : M)
    {
        if (F.isDeclaration())
            continue;
        TargetLibraryInfoImpl TLII;
        TargetLibraryInfo TLI(TLII);
        AssumptionCache AC(F);
        DominatorTree DT(F);
        LoopInfo LI(DT);
        ScalarEvolution SE(F, TLI, AC, DT, LI);

        SmallVector<Instruction *, 16> Orphans;
        for (Loop *L : LI.getLoopsInPreorder())
        {
            for (BasicBlock *BB : L->blocks())
            {
                for (Instruction &I : *BB)
                {
                    auto *GEP = dyn_cast<GetElementPtrInst>(&I);
                    if (!GEP)
                        continue;
                    for (Use &U : GEP->indices())
                    {
                        Value *Idx = U.get();
                        // Direct `trunc(phi)` index.
                        if (auto *Tr = dyn_cast<TruncInst>(Idx))
                        {
                            Value *Src = Tr->getOperand(0);
                            const SCEV *S = SE.getSCEV(Src);
                            if (auto *AR = dyn_cast<SCEVAddRecExpr>(S))
                            {
                                if (AR->getLoop() == L)
                                {
                                    U.set(Src);
                                    if (Tr->use_empty())
                                        Orphans.push_back(Tr);
                                }
                            }
                            continue;
                        }
                        // `add(trunc(phi), C)` index (e.g. a[v+1] = a[v] + 1):
                        // rewrite to `add(phi, sext(C))` so the carried
                        // dependence on the array becomes visible to
                        // DependenceInfo (otherwise the loop is misclassified).
                        auto *AddI = dyn_cast<BinaryOperator>(Idx);
                        if (!AddI || AddI->getOpcode() != Instruction::Add)
                            continue;
                        Value *A = AddI->getOperand(0);
                        Value *B = AddI->getOperand(1);
                        TruncInst *Tr = dyn_cast<TruncInst>(A);
                        ConstantInt *CI = dyn_cast<ConstantInt>(B);
                        if (!Tr || !CI)
                        {
                            Tr = dyn_cast<TruncInst>(B);
                            CI = dyn_cast<ConstantInt>(A);
                        }
                        if (!Tr || !CI)
                            continue;
                        Value *Src = Tr->getOperand(0);
                        const SCEV *S = SE.getSCEV(Src);
                        if (auto *AR = dyn_cast<SCEVAddRecExpr>(S))
                        {
                            if (AR->getLoop() == L)
                            {
                                IRBuilder<> Bld(AddI);
                                Value *CVal =
                                    ConstantInt::get(Src->getType(), CI->getSExtValue());
                                Value *NewAdd = Bld.CreateAdd(Src, CVal, AddI->getName());
                                U.set(NewAdd);
                                if (AddI->use_empty())
                                    Orphans.push_back(AddI);
                                if (Tr->use_empty())
                                    Orphans.push_back(Tr);
                            }
                        }
                    }
                }
            }
        }
        for (Instruction *Orphan : Orphans)
            Orphan->eraseFromParent();
    }
}


// Dump the module as bitcode; used by DUMP_LLVM_BC_PRE / DUMP_LLVM_BC_POST so a
// benchmark can count polly.* references and GOMP_parallel calls directly from
// the IR instead of re-deriving them from the object file.
static void dumpModuleBitcode(llvm::Module &M, const char *path)
{
    std::error_code EC;
    raw_fd_ostream out(path, EC, sys::fs::OF_None);
    if (EC)
    {
        errs() << "Error opening '" << path << "' for writing: " << EC.message() << "\n";
        return;
    }
    WriteBitcodeToFile(M, out);
    out.flush();
}

/* Build the Polly-enabled O3 pipeline.
 *
 * Four things have to be true for Polly to actually transform anything, and
 * p1-AT previously had none of them:
 *
 *  1. The PassBuilder must be constructed WITH a TargetMachine.  Without one,
 *     every function gets a no-op TargetTransformInfo -- no vector registers,
 *     unit cost per instruction -- so the vectorizers and Polly's own
 *     profitability and register-tiling heuristics optimize for an imaginary
 *     scalar machine.
 *  2. polly::registerPollyPasses(PB) must run BEFORE cl::ParseCommandLineOptions,
 *     or Polly's own cl::opt flags do not exist yet when the flags are parsed.
 *  3. The pipeline must be buildPerModuleDefaultPipeline, not
 *     buildModuleOptimizationPipeline: Polly hooks itself into
 *     ScalarOptimizerLateEP, which lives in the module SIMPLIFICATION half.
 *     With the optimization half alone its canonicalization runs but SCoP
 *     detection and codegen never do.  This one is decisive.
 *  4. -polly has to be on; it is off by default.
 *
 * Polly is ENABLED by default but not force-enabled: it is injected only
 * when the caller has not expressed an intent, and it can always be turned
 * off or tuned -- GRAPH_DISABLE_POLLY=1 / -polly=false disable it, and
 * explicit -polly-parallel / -polly-pattern-matching-based-opts or the
 * GRAPH_POLLY_* env vars override the defaults.  When Polly is off the
 * normal pipeline (O3 + PDG + loop outliner + autotuner) runs untouched.
 *
 * When Polly is on, flag defaults match p2GraphEasy, where they were measured:
 *   - the matmul pattern matcher is DISABLED (it accounted for ~97% of Polly
 *     compile time -- 30.2 s vs 0.73 s on a matmul-shaped kernel -- and also
 *     blocked parallelization);
 *   - -polly-parallel is ENABLED (mean speedup over 32 kernels 1.61x -> 2.79x).
 * GRAPH_POLLY_MATMUL_OPT / GRAPH_POLLY_NO_PARALLEL / GRAPH_DISABLE_POLLY and
 * GRAPH_POLLY_EXTRA_FLAGS override, and GRAPH_TARGET_CPU=generic restores the
 * untuned cost model.
 */
static std::unique_ptr<TargetMachine> setUpPollyPipeline(PassBuilder *&PBOut,
                                                         std::unique_ptr<PassBuilder> &PBStorage)
{
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    std::string triple = sys::getDefaultTargetTriple();
    std::string lookupError;
    const Target *target = TargetRegistry::lookupTarget(triple, lookupError);
    if (!target)
    {
        errs() << "Failed to lookup target for triple '" << triple << "': " << lookupError << "\n";
        return nullptr;
    }

    std::string cpu;
    std::string features;
    if (const char *cpuEnv = std::getenv("GRAPH_TARGET_CPU"))
        cpu = cpuEnv;
    else
        cpu = std::string(sys::getHostCPUName());
    if (cpu != "generic")
    {
        SubtargetFeatures featureList;
        for (const auto &feature : sys::getHostCPUFeatures())
            featureList.AddFeature(feature.first(), feature.second);
        features = featureList.getString();
    }

    TargetOptions opts;
    std::unique_ptr<TargetMachine> TM(target->createTargetMachine(
        triple, cpu, features, opts, /*RM=*/std::nullopt, /*CM=*/std::nullopt,
        llvm::CodeGenOptLevel::Default, /*JIT=*/false));
    if (!TM)
    {
        errs() << "Failed to create TargetMachine for triple '" << triple << "'\n";
        return nullptr;
    }

    PBStorage = std::make_unique<PassBuilder>(TM.get());
    PBOut = PBStorage.get();
    polly::registerPollyPasses(*PBOut);

    return TM;
}


// Parse the Polly flags.
//
// Deliberately called AFTER the source has been parsed, because the value of
// -polly-only-func depends on which functions the program declares (see the
// call site).  Polly's cl::opts have to exist before this runs, which
// registerPollyPasses in setUpPollyPipeline has already ensured.
static void parsePollyFlags(int argc, char **argv, const std::string &onlyFuncs)
{
    // Flags-only argv: the positional input filename must not reach the flag
    // parser, so it is recovered separately below.
    std::vector<std::string> flagArgs;
    auto hasArg = [&](const char *a) {
        for (int i = 1; i < argc; ++i)
            if (std::strcmp(argv[i], a) == 0)
                return true;
        return false;
    };

    // Polly is enabled by default but NOT force-enabled: it is injected only
    // when the caller has not expressed an intent, and every knob can turn it
    // off or tune it -- GRAPH_DISABLE_POLLY=1 / -polly=false disable it,
    // explicit -polly-parallel / -polly-pattern-matching-based-opts and the
    // GRAPH_POLLY_* env vars override the defaults.  When Polly is off the
    // normal pipeline (O3 + PDG + loop outliner + autotuner) runs untouched.
    if (isTruthyEnv("GRAPH_DISABLE_POLLY"))
    {
        flagArgs.emplace_back("-polly=false");
    }
    else if (!onlyFuncs.empty() && !hasArg("-polly-only-func"))
    {
        flagArgs.emplace_back("-polly");
        flagArgs.emplace_back("-polly-only-func=" + onlyFuncs);
        if (!isTruthyEnv("GRAPH_POLLY_MATMUL_OPT"))
            flagArgs.emplace_back("-polly-pattern-matching-based-opts=false");
        if (!isTruthyEnv("GRAPH_POLLY_NO_PARALLEL") && !hasArg("-polly-parallel"))
            flagArgs.emplace_back("-polly-parallel");
    }
    else if (!hasArg("-polly"))
    {
        flagArgs.emplace_back("-polly");
        if (!isTruthyEnv("GRAPH_POLLY_MATMUL_OPT") &&
            !hasArg("-polly-pattern-matching-based-opts"))
            flagArgs.emplace_back("-polly-pattern-matching-based-opts=false");
        if (!isTruthyEnv("GRAPH_POLLY_NO_PARALLEL") && !hasArg("-polly-parallel"))
            flagArgs.emplace_back("-polly-parallel");
    }

    if (const char *extra = std::getenv("GRAPH_POLLY_EXTRA_FLAGS"))
    {
        std::string token;
        for (const char *p = extra;; ++p)
        {
            if (*p == ' ' || *p == '\t' || *p == '\0')
            {
                if (!token.empty())
                {
                    flagArgs.push_back(token);
                    token.clear();
                }
                if (*p == '\0')
                    break;
            }
            else
                token.push_back(*p);
        }
    }

    for (int i = 1; i < argc; ++i)
    {
        const char *a = argv[i];
        if (a[0] != '-')
            continue;
        flagArgs.emplace_back(a);
        if (i + 1 < argc && argv[i + 1][0] != '-')
            flagArgs.emplace_back(argv[++i]);
    }

    // Collect every string first, THEN take pointers: growing the vector after
    // taking a pointer into it reallocates and leaves the pointer dangling,
    // which shows up as a garbled filename rather than as a crash.
    std::vector<char *> parseArgv;
    parseArgv.reserve(1 + flagArgs.size());
    parseArgv.push_back(argv[0]);
    for (auto &arg : flagArgs)
        parseArgv.push_back(const_cast<char *>(arg.c_str()));
    cl::ParseCommandLineOptions(static_cast<int>(parseArgv.size()), parseArgv.data());


}

// Counts Polly-profitable SCoPs by running Polly's ScopAnalysis (detection
// only, no codegen) on every function.  Detection already applies Polly's
// profitability heuristic, so a non-zero count means Polly would transform
// this program.
struct PollyProbePass : public llvm::PassInfoMixin<PollyProbePass>
{
    unsigned *Counter;

    PollyProbePass(unsigned *C) : Counter(C) {}

    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM)
    {
        auto &SD = FAM.getResult<polly::ScopAnalysis>(F);
        *Counter += std::distance(SD.begin(), SD.end());
        return llvm::PreservedAnalyses::all();
    }
};

// Runs the autotuner region-annotation pass, the PDG (dependency graph +
// SCC task graph + parallel-IR reconstruction) and the loop-outliner
// machinery that rewrites parallelizable loops into
// parallel_for_runtime / gpu_parallel_for_runtime callbacks.
//
// By default the ORDER is decided by a Polly profitability probe (see main):
// when Polly detects profitable SCoPs it runs FIRST (the outliner follows);
// otherwise the outliner runs first and the O3/Polly pipeline runs at the
// end of the optimization sequence.  GRAPH_OUTLINER_FIRST=1 forces the
// outliner first; GRAPH_DISABLE_POLLY=1 skips Polly entirely.
static void runPdgAndOutliner(llvm::Module &M, bool usingGpuIR)
{
    // GRAPH_DISABLE_PDG=1 skips PDG + loop outliner so benchmarks can measure
    // a true serial / Polly-only baseline against DOALL/DOACROSS outlining.
    if (isTruthyEnv("GRAPH_DISABLE_PDG"))
        return;

    // Run autotuner on user IR before PDG/outlining (which moves calls into
    // separate task functions) and before linking runtime IR modules.
    // Graph-loop race-freedom (Graptor CleanCut): any loop that used graph
    // iterators is lowered to the owner-computes frontier step BEFORE the
    // AutoTuner region pass / PDG / outliner see it, so the racy DOALL path
    // never fires on graph-iterator loops and the region pass never wraps the
    // rewritten round nest.
    {
        LoopAnalysisManager LAM;
        FunctionAnalysisManager FAM;
        CGSCCAnalysisManager CGAM;
        ModuleAnalysisManager LocalMAM;

        PassBuilder LocalPB;
        LocalPB.registerModuleAnalyses(LocalMAM);
        LocalPB.registerCGSCCAnalyses(CGAM);
        LocalPB.registerFunctionAnalyses(FAM);
        LocalPB.registerLoopAnalyses(LAM);
        LocalPB.crossRegisterProxies(LAM, FAM, CGAM, LocalMAM);

        FunctionPassManager FrontFPM;
        FrontFPM.addPass(GraphFrontierLoweringPass());
        ModulePassManager FrontMPM;
        FrontMPM.addPass(createModuleToFunctionPassAdaptor(std::move(FrontFPM)));
        FrontMPM.run(M, LocalMAM);
    }

    {
        LoopAnalysisManager LAM;
        FunctionAnalysisManager FAM;
        CGSCCAnalysisManager CGAM;
        ModuleAnalysisManager LocalMAM;

        PassBuilder LocalPB;
        LocalPB.registerModuleAnalyses(LocalMAM);
        LocalPB.registerCGSCCAnalyses(CGAM);
        LocalPB.registerFunctionAnalyses(FAM);
        LocalPB.registerLoopAnalyses(LAM);
        LocalPB.crossRegisterProxies(LAM, FAM, CGAM, LocalMAM);

        ModulePassManager TuneMPM;
        TuneMPM.addPass(AutoTunerModulePass());
        TuneMPM.run(M, LocalMAM);
    }

    // {
    //     ModuleAnalysisManager MAM;
    //     dependencyGraph pdg = runPDGOnModule(M);
    //     (void)pdg;
    //     runLoopOutlinerOnModule(M);
    //     FunctionPassManager FPM;
    //     FPM.addPass(llvm::SimplifyCFGPass());
    //     FPM.addPass(llvm::ADCEPass()); // aggressive ctrl-flow aware DCE
    //     FPM.addPass(llvm::DCEPass());
    //     ModulePassManager MPM;
    //     MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

    //     MPM.run(M, MAM);
    // }
    // Create all analysis managers and register them with PassBuilder
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

        // run PDG (you already do this)
        if (usingGpuIR)
            canonicalizeLoopsForAnalysis(M);
        dependencyGraph pdg = runPDGOnModule(M);
        // errs() << "✓ Built PDG with " << pdg.nodes.size() << " vertices and "

        //        << pdg.edges.size() << " edges\n\n";

        // // Step 2: Perform SCC-based task partitioning and create task graph

        // errs() << "Performing SCC-based task partitioning...\n";

        TaskGraph TG = buildSccTaskGraph(pdg);

        // errs() << "✓ Created task graph with " << TG.tasks.size() << " tasks\n";

        // errs() << "✓ Identified " << TG.cutVertices.size()

        //        << " cut vertices (serial bottlenecks)\n\n";

        // // Step 3: Perform topological sort on task graph

        // errs() << "Computing task schedule...\n";

        SmallVector<SmallVector<unsigned>> taskLevels = topologicalSortTaskGraph(TG);

        // errs() << "✓ Scheduled into " << taskLevels.size() << " levels\n\n";

        // // Step 4: Analyze parallelism potential

        // errs() << "Parallelism Analysis:\n";

        // errs() << "---------------------\n";

        unsigned totalVertices = pdg.nodes.size();

        unsigned numCutVertices = TG.cutVertices.size();

        unsigned numComponents = TG.tasks.size() - numCutVertices;

        // errs() << "  Total IR instructions: " << totalVertices << "\n";

        // errs() << "  Serial bottlenecks (cut vertices): " << numCutVertices

        //        << " (" << (100.0 * numCutVertices / totalVertices) << "%)\n";

        // errs() << "  Parallel components: " << numComponents << "\n";

        // errs() << "  Critical path length: " << taskLevels.size() << " levels\n\n";

        // // Step 5: Print task schedule with details

        // errs() << "Task Schedule (Level-by-Level):\n";

        // errs() << "--------------------------------\n";

#if 0
        for (unsigned i = 0; i < taskLevels.size(); ++i)

        {

            errs() << "Level " << i << " (" << taskLevels[i].size() << " tasks):\n";

            // Separate cut vertices and components for clarity

            SmallVector<unsigned> cutTasks, componentTasks;

            for (unsigned taskId : taskLevels[i])

            {

                if (TG.tasks[taskId].isCutVertex)

                    cutTasks.push_back(taskId);

                else

                    componentTasks.push_back(taskId);

            }

            // Print cut vertices first (these must execute serially)

            for (unsigned taskId : cutTasks)

            {

                const TaskNode &task = TG.tasks[taskId];

                errs() << "  [SERIAL] Task " << taskId << ": Cut Vertex (order="

                       << TG.cutVertexOrder.lookup(task.originalVertex)

                       << ", vertex=" << task.originalVertex << ")\n";

            }

            // Print components (these can potentially run in parallel)

            for (unsigned taskId : componentTasks)

            {

                const TaskNode &task = TG.tasks[taskId];

                errs() << "  [PARALLEL] Task " << taskId << ": Component with "

                       << task.vertices.size() << " instruction(s)\n";

            }

            errs() << "\n";

        }
#endif

        unsigned maxParallelTasks = 0;

        unsigned totalParallelOps = 0;

        unsigned totalSerialOps = 0;

        for (const auto &level : taskLevels)

        {

            unsigned parallelOpsInLevel = 0;

            unsigned serialOpsInLevel = 0;

            for (unsigned taskId : level)

            {

                const TaskNode &task = TG.tasks[taskId];

                if (task.isCutVertex)

                {

                    serialOpsInLevel += task.vertices.size();

                }

                else

                {

                    parallelOpsInLevel += task.vertices.size();

                }

            }

            totalParallelOps += parallelOpsInLevel;

            totalSerialOps += serialOpsInLevel;

            // Count parallel tasks (non-cut vertices)

            unsigned parallelTasksInLevel = 0;

            for (unsigned taskId : level)

            {

                if (!TG.tasks[taskId].isCutVertex)

                    parallelTasksInLevel++;

            }

            maxParallelTasks = std::max(maxParallelTasks, parallelTasksInLevel);

        }

#if 0
        errs() << "Detailed Metrics:\n";

        errs() << "-----------------\n";

        errs() << "  Instructions in parallel regions: " << totalParallelOps

               << " (" << (100.0 * totalParallelOps / totalVertices) << "%)\n";

        errs() << "  Instructions in serial regions: " << totalSerialOps

               << " (" << (100.0 * totalSerialOps / totalVertices) << "%)\n";

        errs() << "  Maximum parallel tasks per level: " << maxParallelTasks << "\n";

        errs() << "  Average tasks per level: "

               << (TG.tasks.size() / (float)taskLevels.size()) << "\n\n";

        // Step 7: Identify critical bottlenecks

        if (numCutVertices > 0)

        {

            errs() << "Serial Bottlenecks (in execution order):\n";

            errs() << "----------------------------------------\n";

            for (unsigned i = 0; i < TG.cutVertices.size(); ++i)

            {

                unsigned cv = TG.cutVertices[i];

                errs() << "  " << i << ". Vertex " << cv << " (must execute at specific point)\n";

            }

            errs() << "\n";

        }
#endif

        // ====================================================================

        // PARALLEL IR RECONSTRUCTION

        // ====================================================================

        reconstructParallelIR(M, pdg, TG, taskLevels);

        // After PDG annotation / parallel IR rewrite, before outlining.  Used by
        // the Polly-vs-PDG trigger matrix to count my.loop.parallel DOALL|DOACROSS
        // metadata that the outliner subsequently consumes.
        if (const char *dumpPath = std::getenv("DUMP_LLVM_BC_PDG"))
            dumpModuleBitcode(M, dumpPath);

        // optional: you can still call your helper which creates its own managers
        runLoopOutlinerOnModule(M);

        // Build function-level cleanup pipeline
        FunctionPassManager FPM;
        FPM.addPass(llvm::SimplifyCFGPass());
        FPM.addPass(llvm::ADCEPass()); // aggressive ctrl-flow aware DCE
        FPM.addPass(llvm::DCEPass());

        ModulePassManager MPM;
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

        // Now this will succeed because MAM has been registered/cross-registered
        MPM.run(M, MAM);
    }

    if (const char *dumpPath = std::getenv("DUMP_LLVM_BC_FINAL"))
        dumpModuleBitcode(M, dumpPath);

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
        if (!isTruthyEnv("GRAPH_NO_OUTLINER") && !polly_owns_user_fns)
            registerLoopOutlinerPass(FPM);

        ModulePassManager MPM;
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
        MPM.run(M, MAM);
    }
}

int main(int argc, char **argv)
{
    InitLLVM initLLVM(argc, argv);

    PassBuilder *PollyPB = nullptr;
    std::unique_ptr<PassBuilder> PollyPBStorage;
    std::unique_ptr<TargetMachine> PollyTM =
        setUpPollyPipeline(argc, argv, PollyPB, PollyPBStorage);
    if (!PollyTM)
        return 1;

    std::string backendSelectionReason;
    const std::string activeIRBackend = resolveBackend(backendSelectionReason);

    // errs() << "IR backend selected: " << activeIRBackend << " (" << backendSelectionReason << ")\n";

    const bool usingGpuIR = activeIRBackend == "gpu";
    if (usingGpuIR)
    {
        // errs() << "GPU backend IR path requested; placeholder path active -> emitting CPU IR for now\n";
    }

    // InputFilename is positional and the flag-only parse above deliberately
    // skipped it, so fall back to the first non-flag argument.
    std::string infile = InputFilename;
    if (infile.empty())
        for (int i = 1; i < argc; ++i)
            if (argv[i][0] != '-')
            {
                infile = argv[i];
                break;
            }
    if (infile.empty())
    {
        std::cerr << "No input filename provided.\n";
        return 1;
    }

    std::ifstream in(infile);
    if (!in.good())
    {
        std::cerr << "Failed to open input file: " << infile << "\n";
        return 1;
    }

    ANTLRInputStream input(in);
    BaseLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    BaseParser parser(&tokens);
    auto tree = parser.program();
    if (parser.getNumberOfSyntaxErrors() > 0)
    {
        errs() << "Syntax error: failed to parse '" << infile << "'\n";
        return 1;
    }

    ASTBuilder astB;
    auto progAny = astB.visitProgram(tree);
    auto prog = std::any_cast<ProgramNodePtr>(progAny);

    LLVMContext Ctx;
    auto M = std::make_unique<Module>("my_module", Ctx);
    {
        auto *backendName = llvm::MDString::get(Ctx, activeIRBackend);
        auto *backendMD = llvm::MDNode::get(Ctx, backendName);
        M->getOrInsertNamedMetadata("graph.ir.backend")->addOperand(backendMD);
    }

    // exit(0);
    try
    {
        SemanticAnalyzer sema(prog);
        sema.analyze();
    }
    catch (const std::exception &ex)
    {
        errs() << ex.what() << "\n";
        return 1;
    }

    IRBuilder<> IRB(Ctx);

    std::filesystem::path inputPath(infile);
    std::string sourceDir = inputPath.has_parent_path()
                                ? inputPath.parent_path().string()
                                : std::string(".");
    IRGenVisitor irgen(Ctx, *M, IRB, activeIRBackend, sourceDir);
    irgen.visitProgram(prog);

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
        // after you construct PB and set up analysis managers
        registerLoopOutlinerPluginWithPassBuilder(PB);

        FunctionPassManager FPM;
        FPM.addPass(PromotePass());      // mem2reg
        FPM.addPass(LoopSimplifyPass()); // loop-simplify
        FPM.addPass(SimplifyCFGPass());  // simplifycfg

        ModulePassManager MPM;
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

        MPM.run(*M, MAM);
    }


    // Polly profitability probe: when Polly is enabled and the caller has not
    // forced an order, run Polly's SCoP DETECTION (analysis only, no codegen)
    // on the clean IRGen module.  If at least one profitable SCoP exists,
    // Polly/O3 runs first (its transformations feed the outliner); otherwise
    // the outliner runs first and the O3/Polly pipeline runs at the END of
    // the optimization sequence, where its loop restructuring can no longer
    // hide parallel loops from the PDG/outliner.
    bool pollyFirst = !isTruthyEnv("GRAPH_DISABLE_POLLY") && !isTruthyEnv("GRAPH_OUTLINER_FIRST");
    if (pollyFirst)
    {
        unsigned pollyScopCount = 0;

        LoopAnalysisManager PollyLAM;
        FunctionAnalysisManager PollyFAM;
        CGSCCAnalysisManager PollyCGAM;
        ModuleAnalysisManager PollyMAM;
        PollyPB->registerModuleAnalyses(PollyMAM);
        PollyPB->registerCGSCCAnalyses(PollyCGAM);
        PollyPB->registerFunctionAnalyses(PollyFAM);
        PollyPB->registerLoopAnalyses(PollyLAM);
        PollyPB->crossRegisterProxies(PollyLAM, PollyFAM, PollyCGAM, PollyMAM);

        FunctionPassManager ProbeFPM;
        ProbeFPM.addPass(PollyProbePass(&pollyScopCount));
        ModulePassManager ProbeMPM;
        ProbeMPM.addPass(createModuleToFunctionPassAdaptor(std::move(ProbeFPM)));
        ProbeMPM.run(*M, PollyMAM);

        pollyFirst = pollyScopCount > 0;
        if (isTruthyEnv("GRAPH_DEBUG_POLLY_PROBE"))
            errs() << "[polly-probe] profitable SCoPs: " << pollyScopCount << " -> "
                   << (pollyFirst ? "Polly/O3 first"
                                  : "outliner first, O3/Polly at end")
                   << "\n";
    }

    // Outliner runs BEFORE the Polly/O3 pipeline when Polly is off, when
    // GRAPH_OUTLINER_FIRST=1, or when the probe found nothing for Polly.
    if (!pollyFirst)
        runPdgAndOutliner(*M, usingGpuIR);

    // ---------------------------------------------------------------------
    // Polly.
    //
    // This has to run HERE, immediately after IRGen and BEFORE the PDG /
    // SCC / reconstructParallelIR / loop-outliner machinery below.  That
    // machinery rewrites every loop body into a callback invoked through
    // parallel_for_runtime, which leaves no loop nest in the caller and an
    // opaque callee in its place -- measured on a 512x512 matmul, stock `opt`
    // finds 218 polly.* references in the IR before those passes and exactly 0
    // after, so running Polly at the end (where the object file is emitted)
    // cannot ever fire.
    //
    // Consequence worth stating plainly: when Polly runs first, it -- not the
    // outliner -- parallelizes the loops it transforms.  A profitability probe
    // above decides the order per program: Polly-first when it finds SCoPs,
    // outliner-first (with O3/Polly moved to the end of the sequence)
    // otherwise.  GRAPH_DISABLE_POLLY=1 (or -polly=false) disables Polly, and
    // GRAPH_OUTLINER_FIRST=1 forces the outliner first.
    // ---------------------------------------------------------------------
    if (const char *dumpPath = std::getenv("DUMP_LLVM_BC_PRE"))
        dumpModuleBitcode(*M, dumpPath);

    if (!isTruthyEnv("GRAPH_DISABLE_POLLY"))
    {
        M->setTargetTriple(sys::getDefaultTargetTriple());
        M->setDataLayout(PollyTM->createDataLayout());

        LoopAnalysisManager PollyLAM;
        FunctionAnalysisManager PollyFAM;
        CGSCCAnalysisManager PollyCGAM;
        ModuleAnalysisManager PollyMAM;
        PollyPB->registerModuleAnalyses(PollyMAM);
        PollyPB->registerCGSCCAnalyses(PollyCGAM);
        PollyPB->registerFunctionAnalyses(PollyFAM);
        PollyPB->registerLoopAnalyses(PollyLAM);
        PollyPB->crossRegisterProxies(PollyLAM, PollyFAM, PollyCGAM, PollyMAM);

        // Canonicalization SCoP detection depends on: until mem2reg runs, the
        // loop bounds and array subscripts are still loads and stores and
        // nothing looks affine.
        FunctionPassManager CanonFPM;
        CanonFPM.addPass(PromotePass());
        CanonFPM.addPass(LoopSimplifyPass());
        CanonFPM.addPass(SimplifyCFGPass());
        ModulePassManager CanonMPM;
        CanonMPM.addPass(createModuleToFunctionPassAdaptor(std::move(CanonFPM)));
        CanonMPM.run(*M, PollyMAM);

        ModulePassManager OptMPM =
            PollyPB->buildPerModuleDefaultPipeline(OptimizationLevel::O3);
        OptMPM.run(*M, PollyMAM);
    }

    if (const char *dumpPath = std::getenv("DUMP_LLVM_BC_POST"))
        dumpModuleBitcode(*M, dumpPath);

    // Outliner runs AFTER Polly only when Polly went first (the probe found
    // profitable SCoPs); otherwise it already ran before the Polly/O3 block.
    if (pollyFirst)
        runPdgAndOutliner(*M, usingGpuIR);

    if (const char *dumpPath = std::getenv("DUMP_LLVM_BC_AFTER_OUTLINE"))
        dumpModuleBitcode(*M, dumpPath);

    {
        llvm::SMDiagnostic Err;
        // Parse textual IR file into a new Module
        std::unique_ptr<llvm::Module> BfsMod = llvm::parseIRFile("bfs_runtime.ll", Err, Ctx);
        if (!BfsMod)
        {
            Err.print("GraphProgram", llvm::errs());
            llvm::errs() << "Failed to parse bfs_runtime.ll\n";
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string Bfs_DL = BfsMod->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !Bfs_DL.empty())
            M->setDataLayout(BfsMod->getDataLayout());

        // TargetTriple check is fine as-is (getTargetTriple().empty()).
        if (M->getTargetTriple().empty() && !BfsMod->getTargetTriple().empty())
            M->setTargetTriple(BfsMod->getTargetTriple());

        // Link BfsMod into M (M is the destination)
        llvm::Linker TheLinker(*M);
        // linkInModule returns true on error (older/newer APIs may differ — treat non-zero/true as failure)
        if (TheLinker.linkInModule(std::move(BfsMod)))
        {
            llvm::errs() << "Linking bfs_runtime.ll into main module failed\n";
            return 1;
        }

        // llvm::outs() << "Successfully linked bfs_runtime.ll into module\n";
    }

    {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> DfsMod = llvm::parseIRFile("dfs_runtime.ll", Err, Ctx);
        if (!DfsMod)
        {
            Err.print("GraphProgram", llvm::errs());
            llvm::errs() << "Failed to parse dfs_runtime.ll\n";
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string Dfs_DL = DfsMod->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !Dfs_DL.empty())
            M->setDataLayout(DfsMod->getDataLayout());

        if (M->getTargetTriple().empty() && !DfsMod->getTargetTriple().empty())
            M->setTargetTriple(DfsMod->getTargetTriple());

        llvm::Linker L(*M);
        if (L.linkInModule(std::move(DfsMod)))
        {
            llvm::errs() << "Linking dfs_runtime.ll into main module failed\n";
            return 1;
        }

        // llvm::outs() << "Successfully linked dfs_runtime.ll into module\n";
    }

    {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> BkMod = llvm::parseIRFile("bk_runtime.ll", Err, Ctx);
        if (!BkMod)
        {
            Err.print("GraphProgram", llvm::errs());
            llvm::errs() << "Failed to parse bk_runtime.ll\n";
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string Bk_DL = BkMod->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !Bk_DL.empty())
            M->setDataLayout(BkMod->getDataLayout());

        if (M->getTargetTriple().empty() && !BkMod->getTargetTriple().empty())
            M->setTargetTriple(BkMod->getTargetTriple());

        llvm::Linker L(*M);
        if (L.linkInModule(std::move(BkMod)))
        {
            llvm::errs() << "Linking bk_runtime.ll into main module failed\n";
            return 1;
        }

        // llvm::outs() << "Successfully linked bk_runtime.ll into module\n";
    }

    {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> FWMod = llvm::parseIRFile("floyd_runtime.ll", Err, Ctx);
        if (!FWMod)
        {
            Err.print("GraphProgram", llvm::errs());
            llvm::errs() << "Failed to parse floyd_runtime.ll\n";
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string Fw_DL = FWMod->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !Fw_DL.empty())
            M->setDataLayout(FWMod->getDataLayout());

        if (M->getTargetTriple().empty() && !FWMod->getTargetTriple().empty())
            M->setTargetTriple(FWMod->getTargetTriple());

        llvm::Linker L(*M);
        if (L.linkInModule(std::move(FWMod)))
        {
            llvm::errs() << "Linking floyd_runtime.ll into main module failed\n";
            return 1;
        }

        // llvm::outs() << "Successfully linked floyd_runtime.ll into module\n";
    }

    {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> CmMOD = llvm::parseIRFile("chromacity_runtime.ll", Err, Ctx);
        if (!CmMOD)
        {
            Err.print("GraphProgram", llvm::errs());
            llvm::errs() << "Failed to parse floyd_runtime.ll\n";
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string Cm_DL = CmMOD->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !Cm_DL.empty())
            M->setDataLayout(CmMOD->getDataLayout());

        if (M->getTargetTriple().empty() && !CmMOD->getTargetTriple().empty())
            M->setTargetTriple(CmMOD->getTargetTriple());

        llvm::Linker L(*M);
        if (L.linkInModule(std::move(CmMOD)))
        {
            llvm::errs() << "Linking floyd_runtime.ll into main module failed\n";
            return 1;
        }

        // llvm::outs() << "Successfully linked floyd_runtime.ll into module\n";
    }

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    if (usingGpuIR)
        emitGpuKernels(*M, "kernels.ptx");

    // Dump the final module (after GPU kernel emission so embedded PTX payloads
    // are included) when an IR output path was requested.
    if (!EmitIRTo.empty())
    {
        std::error_code EC;
        raw_fd_ostream IROut(EmitIRTo, EC, sys::fs::OF_None);
        if (EC)
        {
            errs() << "Could not open IR output file '" << EmitIRTo << "': " << EC.message() << "\n";
        }
        else
        {
            M->print(IROut, nullptr);
            IROut.flush();
        }
    }

    std::string TargetTriple = sys::getDefaultTargetTriple();
    M->setTargetTriple(TargetTriple);

    // Reuse the TargetMachine the Polly pipeline was built with.  A second one
    // created here with "generic" and no features would describe a different
    // machine than the one the optimizer costed against, so the vector width
    // Polly assumed and the vector width codegen emits could disagree.
    TargetMachine *TM = PollyTM.get();
    M->setDataLayout(TM->createDataLayout());

    std::error_code EC;
    raw_fd_ostream dest("program.o", EC, sys::fs::OF_None);
    if (EC)
    {
        errs() << "Could not open output file 'program.o': " << EC.message() << "\n";
        return 1;
    }

    legacy::PassManager codeGenPass;

    // LLVM 20: use CodeGenFileType::ObjectFile
    if (TM->addPassesToEmitFile(codeGenPass, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
    {
        errs() << "TargetMachine can't emit a file of this type\n";
        return 1;
    }

    codeGenPass.run(*M);
    dest.flush();

    return 0;
}
