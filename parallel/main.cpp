// main.cpp
#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "BaseLexer.h"
#include "BaseParser.h"
#include "ASTBuilder.h"
#include "ASTNode.h"
#include "IRGenVisitor.h"

#include "pdg.h"
#include "parallel_loop_outline.h"

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

using namespace antlr4;
using namespace llvm;

static cl::opt<std::string> InputFilename(
    cl::Positional,
    cl::desc("<input-graph-file>"),
    cl::Required);

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

    ASTBuilder astB;
    auto progAny = astB.visitProgram(tree);
    auto prog = std::any_cast<ProgramNodePtr>(progAny);

    LLVMContext Ctx;
    auto M = std::make_unique<Module>("my_module", Ctx);
    IRBuilder<> IRB(Ctx);

    IRGenVisitor irgen(Ctx, *M, IRB);
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

    {
        dependencyGraph pdg = runPDGOnModule(*M);
        (void)pdg;
        runLoopOutlinerOnModule(*M);
    }
    M->print(outs(), nullptr);

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
