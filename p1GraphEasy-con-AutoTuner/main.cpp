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
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/TargetParser/Host.h>

#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/CodeGen.h> // CodeGenFileType, CodeGenOptLevel
#include <llvm/IR/LegacyPassManager.h>

#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/SourceMgr.h>

#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "SemanticAnalyzer.h"

using namespace antlr4;
using namespace llvm;

static cl::opt<std::string> InputFilename(
    cl::Positional,
    cl::desc("<input-graph-file>"),
    cl::Required);

static cl::opt<std::string> IRBackendOption(
    "ir-backend",
    cl::desc("IR backend to select: auto|cpu|gpu"),
    cl::value_desc("auto|cpu|gpu"),
    cl::init("auto"));

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

int main(int argc, char **argv)
{
    InitLLVM initLLVM(argc, argv);
    cl::ParseCommandLineOptions(argc, argv);

    std::string backendSelectionReason;
    const std::string activeIRBackend = resolveBackend(backendSelectionReason);

    errs() << "IR backend selected: " << activeIRBackend << " (" << backendSelectionReason << ")\n";

    const bool usingGpuIR = activeIRBackend == "gpu";
    if (usingGpuIR)
    {
        errs() << "GPU backend IR path requested; placeholder path active -> emitting CPU IR for now\n";
    }

    std::ifstream in(InputFilename);
    if (!in.good())
    {
        std::cerr << "Failed to open input file: " << InputFilename << "\n";
        return 1;
    }

    ANTLRInputStream input(in);
    BaseLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    BaseParser parser(&tokens);
    auto tree = parser.program();
    if (parser.getNumberOfSyntaxErrors() > 0)
    {
        errs() << "Syntax error: failed to parse '" << InputFilename << "'\n";
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

    std::filesystem::path inputPath(InputFilename.getValue());
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

    // Run autotuner on user IR before PDG/outlining (which moves calls into
    // separate task functions) and before linking runtime IR modules.
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
        TuneMPM.run(*M, LocalMAM);
    }

    // {
    //     ModuleAnalysisManager MAM;
    //     dependencyGraph pdg = runPDGOnModule(*M);
    //     (void)pdg;
    //     runLoopOutlinerOnModule(*M);
    //     FunctionPassManager FPM;
    //     FPM.addPass(llvm::SimplifyCFGPass());
    //     FPM.addPass(llvm::ADCEPass()); // aggressive ctrl-flow aware DCE
    //     FPM.addPass(llvm::DCEPass());
    //     ModulePassManager MPM;
    //     MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

    //     MPM.run(*M, MAM);
    // }
    {
        // Create all analysis managers and register them with PassBuilder
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
        dependencyGraph pdg = runPDGOnModule(*M);
        // errs() << "✓ Built PDG with " << pdg.nodes.size() << " vertices and "

        //        << pdg.edges.size() << " edges\n\n";

        // // Step 2: Perform min-cut partitioning and create task graph

        // errs() << "Performing global min-cut partitioning...\n";

        TaskGraph TG = performMinCutAndCreateTaskGraph(pdg);

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

        // ====================================================================

        // PARALLEL IR RECONSTRUCTION

        // ====================================================================

        reconstructParallelIR(*M, pdg, TG, taskLevels);

        // optional: you can still call your helper which creates its own managers
        runLoopOutlinerOnModule(*M);

        // Build function-level cleanup pipeline
        FunctionPassManager FPM;
        FPM.addPass(llvm::SimplifyCFGPass());
        FPM.addPass(llvm::ADCEPass()); // aggressive ctrl-flow aware DCE
        FPM.addPass(llvm::DCEPass());

        ModulePassManager MPM;
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

        // Now this will succeed because MAM has been registered/cross-registered
        MPM.run(*M, MAM);
    }

    // M->print(outs(), nullptr);
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
        registerLoopOutlinerPass(FPM);

        ModulePassManager MPM;
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
        MPM.run(*M, MAM);
    }

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

    std::string TargetTriple = sys::getDefaultTargetTriple();
    M->setTargetTriple(TargetTriple);

    std::string Error;
    const Target *Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target)
    {
        errs() << "Failed to lookup target for triple '" << TargetTriple << "': " << Error << "\n";
        return 1;
    }

    TargetOptions Opts;
    std::optional<llvm::Reloc::Model> RM = std::nullopt;
    std::optional<llvm::CodeModel::Model> CM = std::nullopt;

    // LLVM 20: use CodeGenOptLevel
    auto OptLevel = llvm::CodeGenOptLevel::Default;

    // Use the full modern signature for createTargetMachine (LLVM 18+ / 20)
    auto TM = Target->createTargetMachine(TargetTriple, "generic", /*Features=*/"", Opts, RM, CM, OptLevel, /*JIT=*/false);

    if (!TM)
    {
        errs() << "Failed to create TargetMachine for triple '" << TargetTriple << "'\n";
        return 1;
    }

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
