#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "BaseLexer.h"
#include "BaseParser.h"
#include "ASTBuilder.h"
#include "ASTNode.h"
#include "IRGenVisitor.h"
#include "SemanticAnalyzer.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/CommandLine.h>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/IR/LegacyPassManager.h>

// New Pass Manager headers
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

// Pass implementations
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/LoopSimplify.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

// Polly header (Fedora package provides this)
#include <polly/RegisterPasses.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/SourceMgr.h>

using namespace antlr4;
using namespace llvm;

static cl::opt<std::string> InputFilename(
    cl::Positional,
    cl::desc("<input-file>"),
    cl::init(""));

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

// struct DummyPollyCheckPass : public PassInfoMixin<DummyPollyCheckPass>
// {
//     PreservedAnalyses run(Function &F, FunctionAnalysisManager &)
//     {
//         errs() << ">>> DummyPollyCheckPass visiting function: " << F.getName() << "\n";

//         // Check function-level metadata
//         if (MDNode *MD = F.getMetadata("polly"))
//         {
//             errs() << "   -> Found Polly metadata on function!\n";
//             MD->print(errs());
//             errs() << "\n";
//         }

//         if (MDNode *MD = F.getMetadata("scop"))
//         {
//             errs() << "   -> Found Scop metadata on function!\n";
//             MD->print(errs());
//             errs() << "\n";
//         }

//         return PreservedAnalyses::all();
//     }
// };

int main(int argc, char **argv)
{
    // NOTE: we postpone cl::ParseCommandLineOptions until after Polly registers its flags,
    // so we do not call it here with the original argv. We'll build a new argv
    // (original args + our forced polly args) and parse that after registration.

    InitLLVM initLLVM(argc, argv);

    // We must register Polly with the PassBuilder before parsing Polly flags
    // but we must avoid parsing positional arguments with the flag parser.

    // --- New Pass Manager setup (early, to register Polly) ---
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;

    PassBuilder PB;

    // --- Polly integration using PassBuilder (Fedora packaging API) ---
    // errs() << "DEBUG: about to call polly::registerPollyPasses(PB)\n";
    polly::registerPollyPasses(PB);
    // errs() << "DEBUG: returned from polly::registerPollyPasses(PB)\n";

    // Register the analyses with the PassBuilder-managed analysis managers
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // Build a simple function-level pipeline: mem2reg, loop-simplify, simplifycfg
    FunctionPassManager FPM;
    FPM.addPass(PromotePass());      // mem2reg
    FPM.addPass(LoopSimplifyPass()); // loop-simplify
    FPM.addPass(SimplifyCFGPass());  // simplifycfg

    // FPM.addPass(DummyPollyCheckPass());

    ModulePassManager MPM;
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    ModulePassManager OptMPM; // will be built after flag parsing

    // ------------------
    // FLAGS-ONLY PARSING
    // ------------------
    // Build a parse-only argv that contains program name + flags (no positional filename)
    std::vector<const char *> parseArgv;
    std::vector<std::string> storedArgs; // keep strings alive for c_str()

    // program name
    parseArgv.push_back(argv[0]);

    // helper to check existing args in original argv
    auto hasArg = [&](const char *a)
    {
        for (int i = 1; i < argc; ++i)
            if (strcmp(argv[i], a) == 0)
                return true;
        return false;
    };

    // Force-enable polly if caller didn't pass it
    if (!hasArg("-polly"))
    {
        storedArgs.emplace_back("-polly");
        parseArgv.push_back(storedArgs.back().c_str());
    }

    // Append any ORIGINAL *flag* arguments (those that start with '-') from argv.
    // If a flag has a value (next argv doesn't start with '-'), include that too.
    for (int i = 1; i < argc; ++i)
    {
        const char *a = argv[i];
        if (a[0] == '-')
        {
            // include flag
            storedArgs.emplace_back(a);
            parseArgv.push_back(storedArgs.back().c_str());
            // if next token exists and is not a flag, treat it as the flag's value
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                storedArgs.emplace_back(argv[i + 1]);
                parseArgv.push_back(storedArgs.back().c_str());
                ++i; // skip the value we consumed
            }
        }
    }

    // Parse only the flags so polly's flags are registered without confusing positional args.
    {
        int parseArgc = static_cast<int>(parseArgv.size());
        std::vector<char *> mutableArgv;
        mutableArgv.reserve(parseArgv.size());
        for (auto s : parseArgv)
            mutableArgv.push_back(const_cast<char *>(s));
        // errs() << "DEBUG: parsing flag-only command-line (count=" << parseArgc << ")\n";
        cl::ParseCommandLineOptions(parseArgc, mutableArgv.data());
    }
    // Build O3 pipeline *after* parsing flags so Polly's cl::opts (e.g. -polly) are respected.
    OptMPM = PB.buildModuleOptimizationPipeline(OptimizationLevel::O3, ThinOrFullLTOPhase::None);

    // ------------------
    // INPUT FILE (positional) NOW
    // ------------------
    std::string infile;
    if (!InputFilename.empty())
    {
        infile = InputFilename;
    }
    else if (argc > 1 && strlen(argv[1]) > 0)
    {
        infile = argv[1];
    }
    else
    {
        errs() << "No input filename provided.\n";
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

    ASTBuilder astB;
    auto progAny = astB.visitProgram(tree);
    auto prog = std::any_cast<ProgramNodePtr>(progAny);

    // Semantic analysis (name resolution, type checking, validations)
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

    // --- Now LLVM IR generation/linking and running passes ---
    LLVMContext Ctx;
    auto M = std::make_unique<Module>("my_module", Ctx);
    IRBuilder<> IRB(Ctx);

    IRGenVisitor irgen(Ctx, *M, IRB);
    irgen.visitProgram(prog);

    // M->print(outs(), nullptr);

    // --- Link in external IR (bfs_runtime.ll) into 'M' ---
    {
        llvm::SMDiagnostic Err;
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

        if (M->getTargetTriple().empty() && !BfsMod->getTargetTriple().empty())
            M->setTargetTriple(BfsMod->getTargetTriple());

        llvm::Linker TheLinker(*M);
        if (TheLinker.linkInModule(std::move(BfsMod)))
        {
            llvm::errs() << "Linking bfs_runtime.ll into main module failed\n";
            return 1;
        }

        // llvm::outs() << "Successfully linked bfs_runtime.ll into module\n";
    }
    {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> BfsModSrc = llvm::parseIRFile("bfs_runtime_src.ll", Err, Ctx);
        if (!BfsModSrc)
        {
            Err.print("GraphProgram", llvm::errs());
            llvm::errs() << "Failed to parse bfs_runtime.ll\n";
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string Bfs_DL = BfsModSrc->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !Bfs_DL.empty())
            M->setDataLayout(BfsModSrc->getDataLayout());

        if (M->getTargetTriple().empty() && !BfsModSrc->getTargetTriple().empty())
            M->setTargetTriple(BfsModSrc->getTargetTriple());

        llvm::Linker TheLinker(*M);
        if (TheLinker.linkInModule(std::move(BfsModSrc)))
        {
            llvm::errs() << "Linking bfs_runtime.ll into main module failed\n";
            return 1;
        }
    }
     {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> karMod = llvm::parseIRFile("karger_runtime.ll", Err, Ctx);
        if (!karMod)
        {
            Err.print("GraphProgram", llvm::errs());
            llvm::errs() << "Failed to parse karger_runtime.ll\n";
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string Bfs_DL = karMod->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !Bfs_DL.empty())
            M->setDataLayout(karMod->getDataLayout());

        if (M->getTargetTriple().empty() && !karMod->getTargetTriple().empty())
            M->setTargetTriple(karMod->getTargetTriple());

        llvm::Linker TheLinker(*M);
        if (TheLinker.linkInModule(std::move(karMod)))
        {
            llvm::errs() << "Linking bfs_runtime.ll into main module failed\n";
            return 1;
        }
    }
    {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> DfsMod = llvm::parseIRFile("dfs_runtime_src.ll", Err, Ctx);
        if (!DfsMod)
        {
            Err.print("GraphProgram", llvm::errs());
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string Dfs_DL = DfsMod->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !Dfs_DL.empty())
            M->setDataLayout(Dfs_DL);

        if (M->getTargetTriple().empty() && !DfsMod->getTargetTriple().empty())
            M->setTargetTriple(DfsMod->getTargetTriple());

        llvm::Linker L(*M);
        if (L.linkInModule(std::move(DfsMod)))
        {
            llvm::errs() << "Error linking dfs_runtime_src.ll\n";
            return 1;
        }
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
        std::unique_ptr<llvm::Module> DIMod = llvm::parseIRFile("dijkstra_runtime.ll", Err, Ctx);
        if (!DIMod)
        {
            Err.print("GraphProgram", llvm::errs());
            llvm::errs() << "Failed to parse dijkstra_runtime.ll\n";
            return 1;
        }

        const std::string M_DL = M->getDataLayout().getStringRepresentation();
        const std::string DI_DL = DIMod->getDataLayout().getStringRepresentation();
        if (M_DL.empty() && !DI_DL.empty())
            M->setDataLayout(DIMod->getDataLayout());

        if (M->getTargetTriple().empty() && !DIMod->getTargetTriple().empty())
            M->setTargetTriple(DIMod->getTargetTriple());

        llvm::Linker L(*M);
        if (L.linkInModule(std::move(DIMod)))
        {
            llvm::errs() << "Linking dijkstra_runtime.ll into main module failed\n";
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

    // Run the constructed ModulePassManager
    // Run the standard O3 optimization pipeline first (programmatic).
    OptMPM.run(*M, MAM);
    // after OptMPM.run(*M, MAM)  (or MPM.run)

    // Then run the constructed ModulePassManager (which may include other passes).
    MPM.run(*M, MAM);
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
    // errs() << "=== Named metadata in module ===\n";
    // for (auto &N : M->named_metadata())
    // {
    //     StringRef name = N.getName();
    //     errs() << "NamedMD: " << name << "  (operands=" << N.getNumOperands() << ")\n";

    //     if (name.contains("polly") || name.contains("scop"))
    //     {
    //         errs() << "  -> Looks like Polly/Scop metadata exists\n";

    //         // Print each operand (MDNode) in a supported way instead of using dump().
    //         for (unsigned i = 0, e = N.getNumOperands(); i != e; ++i)
    //         {
    //             if (auto *MD = N.getOperand(i))
    //             {
    //                 errs() << "   operand[" << i << "]: ";
    //                 MD->print(errs()); // public API
    //                 errs() << "\n";
    //             }
    //         }
    //     }
    // }

    // Write bitcode and print IR
    // writeBitcodeToFile(*M, "myprog.bc");
    // M->print(outs(), nullptr);

    return 0;
}
