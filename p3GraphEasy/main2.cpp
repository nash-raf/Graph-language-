// // main.cpp
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

// #include <llvm/Bitcode/BitcodeWriter.h>
// #include <llvm/Support/FileSystem.h>
// #include <llvm/Support/raw_ostream.h>

// // New Pass Manager headers
// #include <llvm/IR/PassManager.h>
// #include <llvm/Passes/PassBuilder.h>

// // Pass implementations
// #include <llvm/Transforms/Utils/Mem2Reg.h>
// #include <llvm/Transforms/Utils/LoopSimplify.h>
// #include <llvm/Transforms/Scalar/SimplifyCFG.h>

// // Polly header (Fedora package provides this)
// #include <polly/RegisterPasses.h>

// using namespace antlr4;
// using namespace llvm;

// static cl::opt<std::string> InputFilename(
//     cl::Positional,
//     cl::desc("<input-file>"),
//     cl::Required);

// static void writeBitcodeToFile(Module &M, const std::string &path)
// {
//     std::error_code EC;
//     raw_fd_ostream Out(path, EC, sys::fs::OF_None);
//     if (EC)
//     {
//         errs() << "Error opening '" << path << "' for writing: " << EC.message() << "\n";
//         return;
//     }
//     WriteBitcodeToFile(M, Out);
//     Out.flush();
// }

// int main(int argc, char **argv)
// {
//     // NOTE: we postpone cl::ParseCommandLineOptions until after Polly registers its flags,
//     // so we do not call it here with the original argv. We'll build a new argv
//     // (original args + our forced polly args) and parse that after registration.

//     InitLLVM initLLVM(argc, argv);

//     // --- Open input file ---
//     // We may postpone cl::ParseCommandLineOptions until after Polly registers flags,
//     // so InputFilename (cl::opt) can be empty here. Fall back to argv[1] when present.
//     std::string infile;
//     if (argc > 1 && strlen(argv[1]) > 0)
//     {
//         // If the user provided a positional argument, prefer it (this matches how
//         // your run.sh invokes the program: "./GraphProgram <input.gpl>").
//         infile = argv[1];
//     }
//     else
//     {
//         // Otherwise, try the cl::opt variable (may be set after ParseCommandLineOptions).
//         infile = InputFilename;
//     }

//     std::ifstream in(infile);
//     if (!in.good())
//     {
//         std::cerr << "Failed to open input file: " << infile << "\n";
//         return 1;
//     }

//     ANTLRInputStream input(in);
//     BaseLexer lexer(&input);
//     CommonTokenStream tokens(&lexer);
//     BaseParser parser(&tokens);
//     auto tree = parser.program();

//     ASTBuilder astB;
//     auto progAny = astB.visitProgram(tree);
//     auto prog = std::any_cast<ProgramNodePtr>(progAny);

//     LLVMContext Ctx;
//     auto M = std::make_unique<Module>("my_module", Ctx);
//     IRBuilder<> IRB(Ctx);

//     IRGenVisitor irgen(Ctx, *M, IRB);
//     irgen.visitProgram(prog);

//     // --- New Pass Manager setup ---
//     LoopAnalysisManager LAM;
//     FunctionAnalysisManager FAM;
//     CGSCCAnalysisManager CGAM;
//     ModuleAnalysisManager MAM;

//     PassBuilder PB;

//     // Register the analyses with the PassBuilder-managed analysis managers
//     PB.registerModuleAnalyses(MAM);
//     PB.registerCGSCCAnalyses(CGAM);
//     PB.registerFunctionAnalyses(FAM);
//     PB.registerLoopAnalyses(LAM);
//     PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

//     // Build a simple function-level pipeline: mem2reg, loop-simplify, simplifycfg
//     FunctionPassManager FPM;
//     FPM.addPass(PromotePass());      // mem2reg
//     FPM.addPass(LoopSimplifyPass()); // loop-simplify
//     FPM.addPass(SimplifyCFGPass());  // simplifycfg

//     ModulePassManager MPM;
//     MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

//     // --- Polly integration using PassBuilder (Fedora packaging API) ---
//     // Register Polly's passes into the pass registry / infrastructure.
//     // Fedora's polly package provides registerPollyPasses() (legacy or registration
//     // helper) rather than a RegisterPollyPasses class.
//     polly::registerPollyPasses(PB);

//     // Build a new argv: original args plus forced polly options
//     std::vector<const char *> newArgv;
//     std::vector<std::string> storedArgs; // hold std::string to keep storage alive

//     // Keep program name
//     newArgv.push_back(argv[0]);

//     // copy original args (if any)
//     for (int i = 1; i < argc; ++i)
//     {
//         storedArgs.emplace_back(argv[i]);
//         newArgv.push_back(storedArgs.back().c_str());
//     }

//     // Now parse command-line options (this sets up polly options as if passed on CLI)
//     cl::ParseCommandLineOptions(argc, argv);
//     bool pollyPipelineAdded = false;

//     // Parse textual pipeline "polly" and handle the returned llvm::Error correctly.
//     // PassBuilder::parsePassPipeline returns an llvm::Error (doesn't throw).
//     if (auto Err = PB.parsePassPipeline(MPM, "polly"))
//     {
//         // Print diagnostic and continue without Polly pipeline.
//         llvm::logAllUnhandledErrors(std::move(Err), llvm::errs(),
//                                     "Polly pipeline parse failed: ");
//         errs() << "Continuing with MPM without the 'polly' textual pipeline.\n";
//     }
//     else
//     {
//         pollyPipelineAdded = true;
//         errs() << "Polly textual pipeline 'polly' added to MPM.\n";
//     }

//     // Run the constructed ModulePassManager
//     MPM.run(*M, MAM);

//     // Write bitcode and print IR
//     writeBitcodeToFile(*M, "myprog.bc");
//     M->print(outs(), nullptr);

//     return 0;
// }
