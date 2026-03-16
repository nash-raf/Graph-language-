// main.cpp
#include <fstream>
#include <iostream>

#include "ASTBuilder.h"
#include "ASTNode.h"
#include "BaseLexer.h"
#include "BaseParser.h"
#include "IRGenVisitor.h"
#include "antlr4-runtime.h"

#include "AutoTunerPass.h"
#include "parallel_loop_outline.h"
#include "pdg.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

#include <llvm/TargetParser/Host.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/LoopSimplify.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h> // CodeGenFileType, CodeGenOptLevel
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>

#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/SourceMgr.h>

#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/DCE.h"

using namespace antlr4;
using namespace llvm;

static cl::opt<std::string>
    InputFilename(cl::Positional, cl::desc("<input-graph-file>"), cl::Required);

static void writeBitcodeToFile(Module &M, const std::string &path) {
  std::error_code EC;
  raw_fd_ostream Out(path, EC, sys::fs::OF_None);
  if (EC) {
    return;
  }
  WriteBitcodeToFile(M, Out);
  Out.flush();
}

int main(int argc, char **argv) {
  InitLLVM initLLVM(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  std::ifstream in(InputFilename);
  if (!in.good()) {
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
    (void)pdg;

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

  // ── AutoTuner Pass: DISABLED for ablation study baseline ──
  // {
  //   LoopAnalysisManager LAM;
  //   FunctionAnalysisManager FAM;
  //   CGSCCAnalysisManager CGAM;
  //   ModuleAnalysisManager MAM;
  //   PassBuilder PB;
  //   PB.registerModuleAnalyses(MAM);
  //   PB.registerCGSCCAnalyses(CGAM);
  //   PB.registerFunctionAnalyses(FAM);
  //   PB.registerLoopAnalyses(LAM);
  //   PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
  //   ModulePassManager MPM;
  //   MPM.addPass(AutoTunerModulePass());
  //   MPM.run(*M, MAM);
  // }

  {
    llvm::SMDiagnostic Err;
    // Parse textual IR file into a new Module
    std::unique_ptr<llvm::Module> BfsMod =
        llvm::parseIRFile("bfs_runtime.ll", Err, Ctx);
    if (!BfsMod) {
      return 1;
    }

    const std::string M_DL = M->getDataLayout().getStringRepresentation();
    const std::string Bfs_DL =
        BfsMod->getDataLayout().getStringRepresentation();
    if (M_DL.empty() && !Bfs_DL.empty())
      M->setDataLayout(BfsMod->getDataLayout());

    // TargetTriple check is fine as-is (getTargetTriple().empty()).
    if (M->getTargetTriple().empty() && !BfsMod->getTargetTriple().empty())
      M->setTargetTriple(BfsMod->getTargetTriple());

    // Link BfsMod into M (M is the destination)
    llvm::Linker TheLinker(*M);
    // linkInModule returns true on error (older/newer APIs may differ — treat
    // non-zero/true as failure)
    if (TheLinker.linkInModule(std::move(BfsMod))) {
      return 1;
    }

    // llvm::outs() << "Successfully linked bfs_runtime.ll into module\n";
  }

  {
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> DfsMod =
        llvm::parseIRFile("dfs_runtime.ll", Err, Ctx);
    if (!DfsMod) {
      return 1;
    }

    const std::string M_DL = M->getDataLayout().getStringRepresentation();
    const std::string Dfs_DL =
        DfsMod->getDataLayout().getStringRepresentation();
    if (M_DL.empty() && !Dfs_DL.empty())
      M->setDataLayout(DfsMod->getDataLayout());

    if (M->getTargetTriple().empty() && !DfsMod->getTargetTriple().empty())
      M->setTargetTriple(DfsMod->getTargetTriple());

    llvm::Linker L(*M);
    if (L.linkInModule(std::move(DfsMod))) {
      return 1;
    }

    // llvm::outs() << "Successfully linked dfs_runtime.ll into module\n";
  }

  {
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> BkMod =
        llvm::parseIRFile("bk_runtime.ll", Err, Ctx);
    if (!BkMod) {
      return 1;
    }

    const std::string M_DL = M->getDataLayout().getStringRepresentation();
    const std::string Bk_DL = BkMod->getDataLayout().getStringRepresentation();
    if (M_DL.empty() && !Bk_DL.empty())
      M->setDataLayout(BkMod->getDataLayout());

    if (M->getTargetTriple().empty() && !BkMod->getTargetTriple().empty())
      M->setTargetTriple(BkMod->getTargetTriple());

    llvm::Linker L(*M);
    if (L.linkInModule(std::move(BkMod))) {
      return 1;
    }

    // llvm::outs() << "Successfully linked bk_runtime.ll into module\n";
  }

  {
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> FWMod =
        llvm::parseIRFile("floyd_runtime.ll", Err, Ctx);
    if (!FWMod) {
      return 1;
    }

    const std::string M_DL = M->getDataLayout().getStringRepresentation();
    const std::string Fw_DL = FWMod->getDataLayout().getStringRepresentation();
    if (M_DL.empty() && !Fw_DL.empty())
      M->setDataLayout(FWMod->getDataLayout());

    if (M->getTargetTriple().empty() && !FWMod->getTargetTriple().empty())
      M->setTargetTriple(FWMod->getTargetTriple());

    llvm::Linker L(*M);
    if (L.linkInModule(std::move(FWMod))) {
      return 1;
    }

    // llvm::outs() << "Successfully linked floyd_runtime.ll into module\n";
  }

  {
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> CmMOD =
        llvm::parseIRFile("chromacity_runtime.ll", Err, Ctx);
    if (!CmMOD) {
      return 1;
    }

    const std::string M_DL = M->getDataLayout().getStringRepresentation();
    const std::string Cm_DL = CmMOD->getDataLayout().getStringRepresentation();
    if (M_DL.empty() && !Cm_DL.empty())
      M->setDataLayout(CmMOD->getDataLayout());

    if (M->getTargetTriple().empty() && !CmMOD->getTargetTriple().empty())
      M->setTargetTriple(CmMOD->getTargetTriple());

    llvm::Linker L(*M);
    if (L.linkInModule(std::move(CmMOD))) {
      return 1;
    }

    // llvm::outs() << "Successfully linked floyd_runtime.ll into module\n";
  }

  // ── Link graph_mutation_runtime.ll ──
  {
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> MutMod =
        llvm::parseIRFile("graph_mutation_runtime.ll", Err, Ctx);
    if (!MutMod) {
      return 1;
    }

    const std::string M_DL = M->getDataLayout().getStringRepresentation();
    const std::string Mut_DL = MutMod->getDataLayout().getStringRepresentation();
    if (M_DL.empty() && !Mut_DL.empty())
      M->setDataLayout(MutMod->getDataLayout());

    if (M->getTargetTriple().empty() && !MutMod->getTargetTriple().empty())
      M->setTargetTriple(MutMod->getTargetTriple());

    llvm::Linker L(*M);
    if (L.linkInModule(std::move(MutMod))) {
      return 1;
    }
  }

  // ── Link autotuner_runtime.ll ──
  {
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> ATMod =
        llvm::parseIRFile("autotuner_runtime.ll", Err, Ctx);
    if (!ATMod) {
      return 1;
    }

    const std::string M_DL = M->getDataLayout().getStringRepresentation();
    const std::string AT_DL = ATMod->getDataLayout().getStringRepresentation();
    if (M_DL.empty() && !AT_DL.empty())
      M->setDataLayout(ATMod->getDataLayout());

    if (M->getTargetTriple().empty() && !ATMod->getTargetTriple().empty())
      M->setTargetTriple(ATMod->getTargetTriple());

    llvm::Linker L(*M);
    if (L.linkInModule(std::move(ATMod))) {
      return 1;
    }
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
  if (!Target) {
    return 1;
  }

  TargetOptions Opts;
  std::optional<llvm::Reloc::Model> RM = std::nullopt;
  std::optional<llvm::CodeModel::Model> CM = std::nullopt;

  // LLVM 20: use CodeGenOptLevel
  auto OptLevel = llvm::CodeGenOptLevel::Default;

  // Use the full modern signature for createTargetMachine (LLVM 18+ / 20)
  auto TM =
      Target->createTargetMachine(TargetTriple, "generic", /*Features=*/"",
                                  Opts, RM, CM, OptLevel, /*JIT=*/false);

  if (!TM) {
    return 1;
  }

  M->setDataLayout(TM->createDataLayout());

  std::error_code EC;
  raw_fd_ostream dest("program.o", EC, sys::fs::OF_None);
  if (EC) {
    return 1;
  }

  legacy::PassManager codeGenPass;

  // LLVM 20: use CodeGenFileType::ObjectFile
  if (TM->addPassesToEmitFile(codeGenPass, dest, nullptr,
                              llvm::CodeGenFileType::ObjectFile)) {
    return 1;
  }

  codeGenPass.run(*M);
  dest.flush();

  return 0;
}
