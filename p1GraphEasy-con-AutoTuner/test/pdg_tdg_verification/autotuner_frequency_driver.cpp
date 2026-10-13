#include "AutoTunerPass.h"
#include "graph_frontier_lowering.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

int main(int argc, char **argv)
{
    if (argc < 2)
        return 2;
    llvm::LLVMContext context;
    llvm::SMDiagnostic diagnostic;
    auto module = llvm::parseIRFile(argv[1], diagnostic, context);
    if (!module)
    {
        diagnostic.print(argv[0], llvm::errs());
        return 1;
    }
    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;
    llvm::PassBuilder pb;
    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);
    llvm::ModulePassManager passes;
    if (argc > 2 && llvm::StringRef(argv[2]) == "--lower")
    {
        llvm::FunctionPassManager frontier;
        frontier.addPass(GraphFrontierLoweringPass());
        passes.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(frontier)));
    }
    passes.addPass(AutoTunerModulePass());
    passes.run(*module, mam);
    if (llvm::verifyModule(*module, &llvm::errs()))
        return 1;
    module->print(llvm::outs(), nullptr);
    return 0;
}
