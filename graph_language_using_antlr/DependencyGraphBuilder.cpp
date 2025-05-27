#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include <fstream>

namespace {
    struct depEdge {
        unsigned source;
        unsigned target;
        std::string type;
    };

    struct DependencyGraphBuilder : public llvm::PassInfoMixin<DependencyGraphBuilder> {
        llvm::DenseMap<const llvm::Instruction*, unsigned> IDMap;
        llvm::SmallVector<depEdge, 256> Edges;
        unsigned NextID = 0;

        DependencyGraphBuilder() = default;

        llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &AM) {
            llvm::outs() << "Running DependencyGraphBuilder pass\n";
            
            auto &FAM = AM.getResult<llvm::FunctionAnalysisManagerModuleProxy>(M).getManager();
            
            for(llvm::Function &F : M) {
                if(F.isDeclaration()) continue;
                llvm::outs() << "Analyzing function: " << F.getName() << "\n";
                buildForFunction(F, FAM);
            }

            generateDOT(M.getName());
            return llvm::PreservedAnalyses::all();
        }

        void buildForFunction(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
            // Only proceed if function has basic blocks
            if (F.empty()) return;

            try {
                auto &LI = FAM.getResult<llvm::LoopAnalysis>(F);
                auto &AAR = FAM.getResult<llvm::AAManager>(F);
                auto &SE = FAM.getResult<llvm::ScalarEvolutionAnalysis>(F);
                
                llvm::DependenceInfo DI(&F, &AAR, &SE, &LI);
                
                // Process each loop separately for better loop-carried dependency detection
                for (auto *L : LI) {
                    analyzeLoopDependencies(L, DI, LI, SE);
                }
                
                // Analyze dependencies between instructions outside loops
                analyzeNonLoopDependencies(F, DI, LI, SE);
                
                // Add control dependencies for branches and calls
                addControlDependencies(F);
                
            } catch(const std::exception &e) {
                llvm::errs() << "Error in buildForFunction: " << e.what() << "\n";
            } catch(...) {
                llvm::errs() << "Unknown error in buildForFunction\n";
            }
        }

        void analyzeLoopDependencies(llvm::Loop *L, llvm::DependenceInfo &DI, llvm::LoopInfo &LI, 
                                     llvm::ScalarEvolution &SE) {
            std::vector<llvm::Instruction*> memInsts;
            llvm::Function *F1 = L->getHeader()->getParent();

            // Walk the IR in *textual* (program) order:
            for (llvm::BasicBlock &BB : *F1) {
            // 1) skip blocks not in this loop
                if (!L->contains(&BB))
                    continue;

                // 2) now collect each mem‐op in program order
                for (llvm::Instruction &I : BB) {
                    if (I.mayReadOrWriteMemory())
                        memInsts.push_back(&I);
                }
            }

            
            // dependencies between memory instructions in the loop
            for (size_t i = 0; i < memInsts.size(); ++i) {
                for (size_t j = i+1; j < memInsts.size(); ++j) {
                    
                    llvm::Instruction *I1 = memInsts[i];
                    llvm::Instruction *I2 = memInsts[j];
                    

                    if (auto dep = DI.depends(I1, I2, false)) {
                        analyzeDependency(I1, I2, dep.get(), L, LI, SE);
                    }
                }
            }
            
            // Recursively process nested loops
            for (auto *SubL : L->getSubLoops()) {
                analyzeLoopDependencies(SubL, DI, LI, SE);
            }
        }

        void analyzeNonLoopDependencies(llvm::Function &F, llvm::DependenceInfo &DI, llvm::LoopInfo &LI, llvm::ScalarEvolution &SE) {
            // Analyze dependencies between instructions not in loops
            for (auto &BB : F) {
                // Skip basic blocks that are inside loops (already processed)
                if (LI.getLoopFor(&BB)) continue;
                
                std::vector<llvm::Instruction*> memInsts;
                
                // Collect memory instructions
                for (auto &I : BB) {
                    if (I.mayReadOrWriteMemory()) {
                        memInsts.push_back(&I);
                    }
                }
                
                // Check dependencies between memory instructions
                for (size_t i = 0; i < memInsts.size(); ++i) {
                    for (size_t j = i + 1; j < memInsts.size(); ++j) {
                        llvm::Instruction *I1 = memInsts[i];
                        llvm::Instruction *I2 = memInsts[j];
                        
                        if (auto dep = DI.depends(I1, I2, false)) {
                            analyzeDependency(I1, I2, dep.get(), nullptr, LI, SE);
                        }
                    }
                }
            }
        }

        void analyzeDependency(llvm::Instruction *I1, llvm::Instruction *I2, 
                              llvm::Dependence *dep, llvm::Loop *currentLoop, 
                              llvm::LoopInfo &LI, llvm::ScalarEvolution &SE) {
            
            std::string depType = "UNKNOWN";
            
            if (dep->isFlow()) {
                depType = "RAW";  
            } else if (dep->isAnti()) {
                depType = "WAR";  
            } else if (dep->isOutput()) {
                depType = "WAW";  
            } else if (dep->isInput()) {
                depType = "RAR";  
            }
            
            // Analyze loop-carried vs loop-independent dependencies
            bool isLoopCarried = false;
            std::string loopInfo = "";
            
            if (currentLoop) {
                // Check if this is a loop-carried dependency
                unsigned loopDepth = currentLoop->getLoopDepth();
                
                // Check distance vectors at different loop levels
                for (unsigned level = 1; level <= dep->getLevels(); ++level) {
                    if (auto distance = dep->getDistance(level)) {
                        if (!distance->isZero()) {
                            // Non-zero distance indicates loop-carried dependency
                            isLoopCarried = true;
                            
                            std::string distStr;
                            llvm::raw_string_ostream OS(distStr);
                            distance->print(OS);
                            
                            loopInfo = "_loop_carried[L" + std::to_string(level) + 
                                      ",dist=" + OS.str() + "]";
                            break;
                        }
                    } else {
                        // If we can't determine distance, check direction vectors
                        if (auto direction = dep->getDirection(level)) {
                            if (direction == llvm::Dependence::DVEntry::LT ||
                                direction == llvm::Dependence::DVEntry::GT) {
                                // Clear direction indicates loop-carried
                                isLoopCarried = true;
                                std::string dirStr = (direction == llvm::Dependence::DVEntry::LT) ? "forward" : "backward";
                                loopInfo = "_loop_carried[L" + std::to_string(level) + 
                                           ",dir=" + dirStr + "]";
                                break;
                            }
                        }
                    }
                }
                
                // Check if instructions access same memory location
                // with different loop induction variables
                if(dep->getLevels() == 0) {
                    if (!isLoopCarried) {
                        isLoopCarried = checkInductionVariableDependency(I1, I2, currentLoop, SE);
                        if (isLoopCarried) {
                            loopInfo = "_loop_carried[induction_var]";
                        }
                    }
                }
                
                if (!isLoopCarried) {
                    depType += "_loop_independent";
                } else {
                    depType += loopInfo;
                }
            } else {
                // Not in a loop context
                depType += "_sequential";
            }
            
            recordEdge(I1, I2, depType);
        }

        bool checkInductionVariableDependency(llvm::Instruction *I1, llvm::Instruction *I2, llvm::Loop *L, 
                                              llvm::ScalarEvolution &SE) {
            // if both instructions are loads/stores and involve
            // different iterations of the same array/memory location
            
            if (!I1 || !I2 || !L) return false;
            
            // Check if instructions are memory operations on potentially the same base address
            llvm::Value *ptr1 = nullptr, *ptr2 = nullptr;
            const llvm::SCEV *base1 = nullptr, *base2 = nullptr;
            
            

            if (auto *load1 = llvm::dyn_cast<llvm::LoadInst>(I1)) {
                ptr1 = load1->getPointerOperand();
            } else if (auto *store1 = llvm::dyn_cast<llvm::StoreInst>(I1)) {
                ptr1 = store1->getPointerOperand();
            }
            
            if (auto *load2 = llvm::dyn_cast<llvm::LoadInst>(I2)) {
                ptr2 = load2->getPointerOperand();
            } else if (auto *store2 = llvm::dyn_cast<llvm::StoreInst>(I2)) {
                ptr2 = store2->getPointerOperand();
            }
            
            if (!ptr1 || !ptr2) return false;

            auto* phi1 = llvm::dyn_cast<llvm::PHINode>(ptr1);
            auto* phi2 = llvm::dyn_cast<llvm::PHINode>(ptr2);
            
            if(!phi1 || phi1->getParent() != L->getHeader()) {
                if(ptr1) {
                    base1 = SE.getSCEV(ptr1);
                    if(auto *ar1 = llvm::dyn_cast<llvm::SCEVAddRecExpr>(base1)) {
                        if(ar1->getLoop() == L && llvm::isa<llvm::SCEVConstant>(ar1->getStepRecurrence(SE))) {
                            return false; 
                        }
                    }
                }
            }
            
            if(!phi2 || phi2->getParent() != L->getHeader()) {
                if(ptr2) {
                    base2 = SE.getSCEV(ptr2);
                    if(auto *ar2 = llvm::dyn_cast<llvm::SCEVAddRecExpr>(base2)) {
                        if ((ar2->getLoop() == L) && llvm::isa<llvm::SCEVConstant>(ar2->getStepRecurrence(SE))) {
                            return false; 
                        }
                    }
                }
            }
            
            // If both pointers involve GEP instructions with the loop induction variable,
            // this suggests a potential loop-carried dependency
            auto *gep1 = llvm::dyn_cast<llvm::GetElementPtrInst>(ptr1);
            auto *gep2 = llvm::dyn_cast<llvm::GetElementPtrInst>(ptr2);
            
            if (gep1 && gep2) {
                // Check if they access the same base but with different indices
                // that might involve the loop induction variable
                if (gep1->getPointerOperand() == gep2->getPointerOperand()) {
                    // Same base address - could be loop-carried if indices differ
                    return true;
                }
            }
            
            return false;
        }

        void addControlDependencies(llvm::Function &F) {
            for (auto &BB : F) {
                for (auto &I : BB) {
                    if (llvm::isa<llvm::BranchInst>(&I) || llvm::isa<llvm::CallInst>(&I)) {
                        // Find instructions that depend on this control instruction
                        for (auto *User : I.users()) {
                            if (auto *UserInst = llvm::dyn_cast<llvm::Instruction>(User)) {
                                recordEdge(&I, UserInst, "CONTROL");
                            }
                        }
                    }
                }
            }
        }

        unsigned getInstID(const llvm::Instruction *I) {
            if (!I) return 0; // Safety check
            
            auto It = IDMap.find(I);
            if (It != IDMap.end())
                return It->second;
            unsigned ID = NextID++;
            IDMap[I] = ID;
            return ID;
        }

        void recordEdge(const llvm::Instruction *Src, const llvm::Instruction *Dst, const std::string &Label) {
            if (!Src || !Dst) return; // Safety check
            
            unsigned S = getInstID(Src);
            unsigned D = getInstID(Dst);
            Edges.push_back({S, D, Label});
        }

        void generateDOT(llvm::StringRef ModuleName) {
            std::string dotGraph;
            llvm::raw_string_ostream OS(dotGraph);
            
            // Clean up module name
            std::string cleanName = ModuleName.str();
            if (cleanName.size() > 3 && cleanName.substr(cleanName.size()-3) == ".ll") {
                cleanName = cleanName.substr(0, cleanName.size()-3);
            }
            
            // Replace any problematic characters in the name
            for (char &c : cleanName) {
                if (!std::isalnum(c)) c = '_';
            }
            
            OS << "digraph " << cleanName << "_dep_graph {\n";
            OS << "  rankdir=TB;\n";
            OS << "  node [shape=box];\n";
            
            if (IDMap.empty()) {
                OS << "  node0 [label=\"No dependencies found\"];\n";
            } else {
                // Emit node labels
                for (auto &P : IDMap) {
                    const llvm::Instruction *I = P.first;
                    unsigned ID = P.second;
                    
                    std::string instStr;
                    llvm::raw_string_ostream InstOS(instStr);
                    I->print(InstOS);
                    
                    // Sanitize the label for DOT format
                    std::string escapedStr;
                    for (char c : InstOS.str()) {
                        switch(c) {
                            case '"': escapedStr += "\\\""; break;
                            case '\\': escapedStr += "\\\\"; break;
                            case '\n': escapedStr += "\\n"; break;
                            case '\r': escapedStr += "\\r"; break;
                            case '\t': escapedStr += "\\t"; break;
                            default: escapedStr += c; break;
                        }
                    }
                    
                    OS << "  node" << ID << " [label=\"" << escapedStr << "\"];\n";
                }
                
                // Emit edges with different colors for different dependency types
                for (auto &E : Edges) {
                    std::string color = "black";
                    std::string style = "solid";
                    
                    if (E.type.find("loop_carried") != std::string::npos) {
                        color = "red";
                        style = "bold";
                    } else if (E.type.find("loop_independent") != std::string::npos) {
                        color = "blue";
                    } else if (E.type.find("CONTROL") != std::string::npos) {
                        color = "green";
                        style = "dashed";
                    }
                    
                    OS << "  node" << E.source << " -> node" << E.target
                       << " [label=\"" << E.type << "\", color=\"" << color 
                       << "\", style=\"" << style << "\"];\n";
                }
            }
            
            OS << "}\n";
            
            // Output to stderr as expected by the script
            llvm::errs() << OS.str();
            llvm::errs().flush();
        }
    };
}

// Register the pass
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, 
        "DependencyGraphBuilder", 
        "0.1",
        [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
                  llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                    if (Name == "dep-graph-builder") {
                        MPM.addPass(DependencyGraphBuilder());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}