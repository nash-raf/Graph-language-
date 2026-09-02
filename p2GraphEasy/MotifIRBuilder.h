// Emits a specialized motif-matching loop nest directly as LLVM IR.
//
// Replaces the interpreted backtracker in runtime.c. The pattern is known at
// compile time, so the emitted nest carries the required-matrix entries as
// constants, unrolls to exactly k loop levels instead of recursing, and (once
// adjacency driving is enabled) enumerates candidates from CSR rows rather than
// scanning 0..n-1 at every level.
//
// All five DSL constructs that need motif matching share this one nest; they
// differ only in what they do at a complete assignment, which is supplied as a
// LeafEmitter.

#ifndef MOTIF_IR_BUILDER_H
#define MOTIF_IR_BUILDER_H

#include <functional>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "MotifPattern.h"

class MotifIRBuilder
{
public:
    // The CSR the nest walks. `weights` may be a null pointer at run time, which
    // means "unweighted"; the emitted probe handles that exactly as runtime.c
    // does (every edge reports kind 1, so a `-|` requirement can never match).
    struct GraphInputs
    {
        llvm::Value *n = nullptr;       // i64
        llvm::Value *rowPtr = nullptr;  // ptr to i64
        llvm::Value *colIdx = nullptr;  // ptr to i32
        llvm::Value *weights = nullptr; // ptr to i32, may be null at run time
    };

    // Invoked with the k bound vertex ids (each i32) at a complete, canonical
    // assignment. Must leave the builder in a block with no terminator; the nest
    // appends the branch back into the innermost increment.
    using LeafEmitter =
        std::function<void(llvm::IRBuilder<> &, llvm::ArrayRef<llvm::Value *>)>;

    MotifIRBuilder(llvm::LLVMContext &context, llvm::Module &module,
                   llvm::IRBuilder<> &builder, const MotifPattern &pattern,
                   std::string label)
        : Context(context), Module(module), Builder(builder), Pattern(pattern),
          Label(std::move(label))
    {
    }

    // Stage 5 switch. When false every level scans 0..n-1, which is what
    // runtime.c does and is the conservative baseline. When true a level whose
    // pattern forces an edge to an already-bound variable enumerates that
    // vertex's CSR row instead.
    void setAdjacencyDriven(bool enabled) { AdjacencyDriven = enabled; }

    // `applyCanonical` must be false for the graph-comprehension filter path,
    // which collects a vertex union and deliberately does not deduplicate under
    // the pattern's automorphisms.
    void emitNest(const GraphInputs &graph, bool applyCanonical, LeafEmitter leaf);

    // Counts complete assignments without materializing them. Returns an i64.
    llvm::Value *emitCountOnly(const GraphInputs &graph, bool applyCanonical);

private:
    llvm::LLVMContext &Context;
    llvm::Module &Module;
    llvm::IRBuilder<> &Builder;
    const MotifPattern &Pattern;
    std::string Label;
    bool AdjacencyDriven = false;

    // State valid only for the duration of one emitNest call.
    GraphInputs Graph;
    bool ApplyCanonical = false;
    LeafEmitter Leaf;

    void emitLevel(int level, llvm::SmallVectorImpl<llvm::Value *> &binding,
                   llvm::BasicBlock *exitTarget);

    // Candidate range for one level, as [base, end) indices.
    void computeRange(int level, llvm::ArrayRef<llvm::Value *> binding,
                      llvm::Value *&base, llvm::Value *&end);
    llvm::Value *loadCandidate(int level, llvm::Value *idx);

    // Emits every rejection test for `level`. On rejection control branches to
    // `rejectTarget`; on acceptance the builder is left in a fresh block.
    void emitFilters(int level, llvm::ArrayRef<llvm::Value *> binding,
                     llvm::Value *candidate, llvm::BasicBlock *rejectTarget);

    // Branch to `rejectTarget` unless `cond` holds, then continue in a new block.
    void emitGuard(llvm::Value *cond, llvm::BasicBlock *rejectTarget,
                   const std::string &name);

    // A faithful transcription of graph_directed_edge_kind_runtime, emitted once
    // per module. Returning the raw kind (rather than inlining a specialized
    // probe per call site) keeps the semantics obviously identical to the C
    // version; -O3 inlines it and folds the comparison against the constant.
    llvm::Function *getOrCreateEdgeKindFn();

    llvm::Value *emitEdgeKind(llvm::Value *u, llvm::Value *v);
};

#endif // MOTIF_IR_BUILDER_H
