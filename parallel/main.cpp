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

// New Pass Manager headers
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

// Pass implementations
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/LoopSimplify.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

// Host/target headers — try common locations
#if defined(__has_include)
#if __has_include(<llvm/Support/Host.h>)
#include <llvm/Support/Host.h>
#elif __has_include(<llvm/TargetParser/Host.h>)
#include <llvm/TargetParser/Host.h>
#else
#include <llvm/Support/Host.h>
#endif
#else
#include <llvm/Support/Host.h>
#endif

#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/CodeGen.h> // CGFT_ObjectFile
#include <llvm/IR/LegacyPassManager.h>

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

#if defined(LLVM_VERSION_MAJOR) && LLVM_VERSION_MAJOR >= 18
    auto OptLevel = llvm::CodeGenOptLevel::Default;
#else
    auto OptLevel = llvm::CodeGenOpt::Default;
#endif

#if defined(LLVM_VERSION_MAJOR) && LLVM_VERSION_MAJOR >= 18
    auto TM = Target->createTargetMachine(TargetTriple, "generic", /*Features=*/"", Opts, RM, CM, OptLevel, /*JIT=*/false);
#else
    auto TM = Target->createTargetMachine(TargetTriple, "generic", /*Features=*/"", Opts);
#endif

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

#if defined(LLVM_VERSION_MAJOR) && LLVM_VERSION_MAJOR >= 18
    if (TM->addPassesToEmitFile(codeGenPass, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
    {
#else
    if (TM->addPassesToEmitFile(codeGenPass, dest, nullptr, llvm::CGFT_ObjectFile))
    {
#endif
        errs() << "TargetMachine can't emit a file of this type\n";
        return 1;
    }

    codeGenPass.run(*M);
    dest.flush();
}
