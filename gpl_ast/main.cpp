// // // main.cpp
// #include <iostream>
// #include <fstream>

// #include "antlr4-runtime.h"
// #include "BaseLexer.h"
// #include "BaseParser.h"
// #include "ASTBuilder.h"
// #include "ASTNode.h"
// #include "IRGenVisitor.h"

// #include <llvm/IR/LLVMContext.h>
// #include <llvm/IR/Module.h>
// #include <llvm/IR/IRBuilder.h>
// #include <llvm/Support/TargetSelect.h>
// #include <llvm/Support/InitLLVM.h>
// #include <llvm/Support/CommandLine.h>
// #include <llvm/ExecutionEngine/Orc/LLJIT.h>
// #include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>

// using namespace antlr4;
// using namespace llvm;
// using namespace llvm::orc;

// static cl::opt<bool> RunJIT("run", cl::desc("JIT-execute instead of just emit IR"));
// static cl::opt<std::string> InputFilename(
//     cl::Positional,
//     cl::desc("<input-file>"),
//     cl::Required);
// int main(int argc, char **argv)
// {
//     InitLLVM X(argc, argv);
//     cl::ParseCommandLineOptions(argc, argv);
//     std::ifstream in(InputFilename);

//     ANTLRInputStream input(in);
//     BaseLexer lexer(&input);
//     CommonTokenStream tokens(&lexer);
//     BaseParser parser(&tokens);
//     auto tree = parser.program();

//     // 2) Build AST
//     ASTBuilder astB;
//     auto progAny = astB.visitProgram(tree);
//     auto prog = std::any_cast<ProgramNodePtr>(progAny);

//     // 3) Build LLVM IR
//     LLVMContext Ctx;
//     auto M = std::make_unique<Module>("my_module", Ctx);
//     IRBuilder<> IRB(Ctx);
//     IRGenVisitor irgen(Ctx, *M, IRB);
//     irgen.visitProgram(prog);

//     if (!RunJIT)
//     {
//         // just dump IR
//         M->print(outs(), nullptr);
//         return 0;
//     }

//     // 4) Initialize targets & build the JIT
//     InitializeNativeTarget();
//     InitializeNativeTargetAsmPrinter();
//     InitializeNativeTargetAsmParser();

//     auto J = cantFail(LLJITBuilder().create());

//     // ThreadSafeModule wants its own context
//     auto ExecCtx = std::make_unique<LLVMContext>();
//     cantFail(J->addIRModule(
//         ThreadSafeModule(std::move(M), std::move(ExecCtx))));

//     // Look up main() in the JIT
//     auto Sym = cantFail(J->lookup("main"));
//     using MainFn = int (*)();
//     auto *MainPtr = (MainFn)(intptr_t)Sym.getValue();
//     int code = MainPtr();
//     std::cout.flush();
//     return code;
// }

// main.cpp

#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "BaseLexer.h"
#include "BaseParser.h"
#include "ASTBuilder.h"
#include "ASTNode.h"
#include "IRGenVisitor.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

using namespace antlr4;
using namespace llvm;
using namespace llvm::orc;

static void writeBitcodeToFile(llvm::Module &M, const std::string &path)
{
    std::error_code EC;
    llvm::raw_fd_ostream Out(path, EC, llvm::sys::fs::OF_None);
    if (EC)
    {
        llvm::errs() << "Error opening '" << path << "' for writing: " << EC.message() << "\n";
        return;
    }
    llvm::WriteBitcodeToFile(M, Out);
    Out.flush();
}

static void writeIRToFile(llvm::Module &M, const std::string &path)
{
    std::error_code EC;
    llvm::raw_fd_ostream Out(path, EC, llvm::sys::fs::OF_None);
    if (EC)
    {
        llvm::errs() << "Error opening '" << path << "' for writing: " << EC.message() << "\n";
        return;
    }
    M.print(Out, nullptr);
    Out.flush();
}

static cl::opt<bool> RunJIT("run", cl::desc("JIT-execute instead of just emit IR"));
static cl::opt<std::string> InputFilename(
    cl::Positional,
    cl::desc("<input-file>"),
    cl::Required);
int main(int argc, char **argv)
{
    InitLLVM X(argc, argv);
    cl::ParseCommandLineOptions(argc, argv);
    std::ifstream in(InputFilename);

    ANTLRInputStream input(in);
    BaseLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    BaseParser parser(&tokens);
    auto tree = parser.program();

    // 2) Build AST
    ASTBuilder astB;
    auto progAny = astB.visitProgram(tree);
    auto prog = std::any_cast<ProgramNodePtr>(progAny);

    // 3) Build LLVM IR
    LLVMContext Ctx;
    auto M = std::make_unique<Module>("my_module", Ctx);
    IRBuilder<> IRB(Ctx);
    IRGenVisitor irgen(Ctx, *M, IRB);
    // irgen.visitProgram(prog);

    irgen.visitProgram(prog);

    writeBitcodeToFile(*M, "myprog.bc"); 
    writeIRToFile(*M, "myprog.ll");     

    if (!RunJIT)
    {
        
        M->print(outs(), nullptr);
        return 0;
    }

   
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    auto J = cantFail(LLJITBuilder().create());

   
    auto ExecCtx = std::make_unique<LLVMContext>();
    cantFail(J->addIRModule(
        ThreadSafeModule(std::move(M), std::move(ExecCtx))));

    auto Sym = cantFail(J->lookup("main"));
    using MainFn = int (*)();
    auto *MainPtr = (MainFn)(intptr_t)Sym.getValue();
    int code = MainPtr();
    std::cout.flush();
    return code;
}
