#ifndef MOTIFIRBUILDER_H
#define MOTIFIRBUILDER_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <string>

#include "MotifPattern.h"

// Build-time stub, see MotifPattern.h.
class MotifIRBuilder
{
public:
    struct GraphInputs
    {
        llvm::Value *n = nullptr;
        llvm::Value *rowPtr = nullptr;
        llvm::Value *colIdx = nullptr;
        llvm::Value *weights = nullptr;
    };

    MotifIRBuilder(llvm::LLVMContext &, llvm::Module &, llvm::IRBuilder<> &,
                   const MotifPattern &, const std::string &)
    {
    }

    void setAdjacencyDriven(bool)
    {
    }

    llvm::Value *emitCountOnly(const GraphInputs &, bool)
    {
        return nullptr;
    }
};

#endif // MOTIFIRBUILDER_H