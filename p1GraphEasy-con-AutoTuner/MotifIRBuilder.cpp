#include "MotifIRBuilder.h"

#include <llvm/IR/Verifier.h>

using namespace llvm;

namespace
{
constexpr const char *kEdgeKindFnName = "__ge_motif_edge_kind";
}

// ---------------------------------------------------------------------------
// graph_directed_edge_kind_runtime, emitted as IR
//
//   scan row_ptr[u] .. row_ptr[u+1] for col_idx == v; the FIRST hit wins.
//     no hit            -> 0
//     weights == NULL   -> 1   (unweighted: every edge is "positive")
//     weights[idx] > 0  -> 1
//     weights[idx] < 0  -> -1
//     weights[idx] == 0 -> 2   (matches no `required` value, so it poisons the
//                               pair for both present- and absent-requirements)
// ---------------------------------------------------------------------------
Function *MotifIRBuilder::getOrCreateEdgeKindFn()
{
    if (Function *existing = Module.getFunction(kEdgeKindFnName))
        return existing;

    Type *i64Ty = Type::getInt64Ty(Context);
    Type *i32Ty = Type::getInt32Ty(Context);
    Type *ptrTy = PointerType::get(Context, 0);

    FunctionType *fnTy =
        FunctionType::get(i32Ty, {ptrTy, ptrTy, ptrTy, i64Ty, i32Ty, i32Ty}, false);
    Function *fn = Function::Create(fnTy, Function::InternalLinkage, kEdgeKindFnName,
                                    &Module);
    fn->addFnAttr(Attribute::AlwaysInline);
    fn->addFnAttr(Attribute::NoUnwind);

    auto args = fn->arg_begin();
    Value *rowPtr = &*args++;
    rowPtr->setName("row_ptr");
    Value *colIdx = &*args++;
    colIdx->setName("col_idx");
    Value *weights = &*args++;
    weights->setName("weights");
    Value *n = &*args++;
    n->setName("n");
    Value *u = &*args++;
    u->setName("u");
    Value *v = &*args++;
    v->setName("v");

    BasicBlock *entry = BasicBlock::Create(Context, "entry", fn);
    BasicBlock *setup = BasicBlock::Create(Context, "setup", fn);
    BasicBlock *cond = BasicBlock::Create(Context, "scan.cond", fn);
    BasicBlock *body = BasicBlock::Create(Context, "scan.body", fn);
    BasicBlock *inc = BasicBlock::Create(Context, "scan.inc", fn);
    BasicBlock *found = BasicBlock::Create(Context, "found", fn);
    BasicBlock *retZero = BasicBlock::Create(Context, "ret.zero", fn);

    IRBuilder<> b(entry);
    // Guard exactly as the C version: null pointers or out-of-range endpoints
    // report "no edge" rather than trapping.
    Value *uI64 = b.CreateSExt(u, i64Ty, "u64");
    Value *vI64 = b.CreateSExt(v, i64Ty, "v64");
    Value *bad = b.CreateICmpEQ(rowPtr, ConstantPointerNull::get(cast<PointerType>(ptrTy)));
    bad = b.CreateOr(bad, b.CreateICmpEQ(
                              colIdx, ConstantPointerNull::get(cast<PointerType>(ptrTy))));
    bad = b.CreateOr(bad, b.CreateICmpSLT(uI64, ConstantInt::get(i64Ty, 0)));
    bad = b.CreateOr(bad, b.CreateICmpSLT(vI64, ConstantInt::get(i64Ty, 0)));
    bad = b.CreateOr(bad, b.CreateICmpSGE(uI64, n));
    bad = b.CreateOr(bad, b.CreateICmpSGE(vI64, n));
    b.CreateCondBr(bad, retZero, setup);

    b.SetInsertPoint(setup);
    Value *startPtr = b.CreateGEP(i64Ty, rowPtr, uI64, "row.start.ptr");
    Value *start = b.CreateLoad(i64Ty, startPtr, "row.start");
    Value *stopPtr = b.CreateGEP(i64Ty, rowPtr,
                                 b.CreateAdd(uI64, ConstantInt::get(i64Ty, 1)),
                                 "row.stop.ptr");
    Value *stop = b.CreateLoad(i64Ty, stopPtr, "row.stop");
    b.CreateBr(cond);

    b.SetInsertPoint(cond);
    PHINode *idx = b.CreatePHI(i64Ty, 2, "scan.idx");
    idx->addIncoming(start, setup);
    b.CreateCondBr(b.CreateICmpSLT(idx, stop, "scan.more"), body, retZero);

    b.SetInsertPoint(body);
    Value *colVal = b.CreateLoad(i32Ty, b.CreateGEP(i32Ty, colIdx, idx), "col");
    b.CreateCondBr(b.CreateICmpEQ(colVal, v, "col.hit"), found, inc);

    b.SetInsertPoint(inc);
    Value *next = b.CreateAdd(idx, ConstantInt::get(i64Ty, 1), "scan.next");
    idx->addIncoming(next, inc);
    b.CreateBr(cond);

    // The weights load must be guarded by the null test, not selected after it:
    // on an unweighted graph `weights` is a null pointer and dereferencing it
    // would fault. So branch, and merge the two kinds with a phi.
    BasicBlock *weighted = BasicBlock::Create(Context, "weighted", fn);
    BasicBlock *classify = BasicBlock::Create(Context, "classify", fn);

    b.SetInsertPoint(found);
    Value *unweighted =
        b.CreateICmpEQ(weights, ConstantPointerNull::get(cast<PointerType>(ptrTy)),
                       "unweighted");
    b.CreateCondBr(unweighted, classify, weighted);

    b.SetInsertPoint(weighted);
    Value *wv = b.CreateLoad(i32Ty, b.CreateGEP(i32Ty, weights, idx), "w");
    Value *pos = b.CreateICmpSGT(wv, ConstantInt::get(i32Ty, 0), "w.pos");
    Value *neg = b.CreateICmpSLT(wv, ConstantInt::get(i32Ty, 0), "w.neg");
    Value *kindWeighted = b.CreateSelect(
        pos, ConstantInt::get(i32Ty, 1),
        b.CreateSelect(neg, ConstantInt::getSigned(i32Ty, -1),
                       ConstantInt::get(i32Ty, 2)),
        "kind.weighted");
    b.CreateBr(classify);

    b.SetInsertPoint(classify);
    PHINode *kind = b.CreatePHI(i32Ty, 2, "kind");
    kind->addIncoming(ConstantInt::get(i32Ty, 1), found);
    kind->addIncoming(kindWeighted, weighted);
    b.CreateRet(kind);

    b.SetInsertPoint(retZero);
    b.CreateRet(ConstantInt::get(i32Ty, 0));

    return fn;
}

Value *MotifIRBuilder::emitEdgeKind(Value *u, Value *v)
{
    Function *fn = getOrCreateEdgeKindFn();
    return Builder.CreateCall(
        fn, {Graph.rowPtr, Graph.colIdx, Graph.weights, Graph.n, u, v}, "kind");
}

// ---------------------------------------------------------------------------

void MotifIRBuilder::emitGuard(Value *cond, BasicBlock *rejectTarget,
                               const std::string &name)
{
    Function *fn = Builder.GetInsertBlock()->getParent();
    BasicBlock *keep = BasicBlock::Create(Context, name, fn);
    Builder.CreateCondBr(cond, keep, rejectTarget);
    Builder.SetInsertPoint(keep);
}

void MotifIRBuilder::computeRange(int level, ArrayRef<Value *> binding, Value *&base,
                                  Value *&end)
{
    Type *i64Ty = Builder.getInt64Ty();
    const LevelPlan &plan = Pattern.level(level);

    bool driven = AdjacencyDriven && plan.source != LevelSource::FullScan;
    if (!driven)
    {
        base = ConstantInt::get(i64Ty, 0);
        end = Graph.n;
        return;
    }

    // Adjacency-driven: enumerate one CSR row. InRow needs the transpose, which
    // is not wired up until stage 5; guard so a premature flag flip is a loud
    // failure rather than silently reading the wrong array.
    assert(plan.source == LevelSource::OutRow &&
           "InRow enumeration requires the private transpose (stage 5)");

    Value *driver = Builder.CreateSExt(binding[plan.driverVar], i64Ty, "drv");
    Value *startPtr = Builder.CreateGEP(i64Ty, Graph.rowPtr, driver);
    base = Builder.CreateLoad(i64Ty, startPtr, "row.base");
    Value *stopPtr = Builder.CreateGEP(
        i64Ty, Graph.rowPtr, Builder.CreateAdd(driver, ConstantInt::get(i64Ty, 1)));
    end = Builder.CreateLoad(i64Ty, stopPtr, "row.end");
}

Value *MotifIRBuilder::loadCandidate(int level, Value *idx)
{
    const LevelPlan &plan = Pattern.level(level);
    bool driven = AdjacencyDriven && plan.source != LevelSource::FullScan;
    if (!driven)
        return Builder.CreateTrunc(idx, Builder.getInt32Ty(), "cand");
    return Builder.CreateLoad(Builder.getInt32Ty(),
                              Builder.CreateGEP(Builder.getInt32Ty(), Graph.colIdx, idx),
                              "cand");
}

void MotifIRBuilder::emitFilters(int level, ArrayRef<Value *> binding, Value *candidate,
                                 BasicBlock *rejectTarget)
{
    Type *i32Ty = Builder.getInt32Ty();
    const LevelPlan &plan = Pattern.level(level);

    // 1. Canonicality, as lower bounds. Cheapest test and it prunes the subtree,
    //    so it goes first. See MotifPattern.h for why this is equivalent to the
    //    runtime's O(k!) automorphism search.
    if (ApplyCanonical)
    {
        for (int p : plan.lowerBounds)
        {
            Value *ok = Builder.CreateICmpSGT(candidate, binding[p], "canon");
            emitGuard(ok, rejectTarget, "motif.canon.ok");
        }
    }

    // 2. Injectivity. k <= 6, so this is at most five comparisons -- cheaper
    //    than the runtime's calloc(n) `used[]` byte array plus its zero fill.
    for (int p = 0; p < level; ++p)
    {
        Value *distinct = Builder.CreateICmpNE(candidate, binding[p], "distinct");
        emitGuard(distinct, rejectTarget, "motif.distinct.ok");
    }

    // 3. Induced edge constraints against every already-bound variable. Both
    //    directions are checked, including the ones required to be ABSENT --
    //    dropping those would turn induced matching into monomorphism and
    //    inflate every count.
    bool drivenByOut = AdjacencyDriven && plan.source == LevelSource::OutRow;
    for (const LevelCheck &check : plan.checks)
    {
        // required[level][prev]: kind(candidate, A[prev])
        {
            Value *actual = emitEdgeKind(candidate, binding[check.prev]);
            Value *want = ConstantInt::getSigned(i32Ty, static_cast<int>(check.forward));
            emitGuard(Builder.CreateICmpEQ(actual, want, "fwd.ok"), rejectTarget,
                      "motif.fwd.ok");
        }
        // required[prev][level]: kind(A[prev], candidate)
        {
            // When this level was enumerated from A[prev]'s out-row, membership
            // is already established -- but the *kind* still has to be verified
            // (the row tells us an edge exists, not whether its weight sign
            // matches), so the probe is not skippable. Only a future
            // unweighted-and-positive specialization could drop it.
            (void)drivenByOut;
            Value *actual = emitEdgeKind(binding[check.prev], candidate);
            Value *want = ConstantInt::getSigned(i32Ty, static_cast<int>(check.backward));
            emitGuard(Builder.CreateICmpEQ(actual, want, "bwd.ok"), rejectTarget,
                      "motif.bwd.ok");
        }
    }
}

void MotifIRBuilder::emitLevel(int level, SmallVectorImpl<Value *> &binding,
                               BasicBlock *exitTarget)
{
    Function *fn = Builder.GetInsertBlock()->getParent();

    if (level == Pattern.varCount())
    {
        Leaf(Builder, binding);
        Builder.CreateBr(exitTarget);
        return;
    }

    std::string tag = Label + ".l" + std::to_string(level);
    Type *i64Ty = Builder.getInt64Ty();

    BasicBlock *cond = BasicBlock::Create(Context, tag + ".cond", fn);
    BasicBlock *body = BasicBlock::Create(Context, tag + ".body", fn);
    BasicBlock *inc = BasicBlock::Create(Context, tag + ".inc", fn);
    BasicBlock *done = BasicBlock::Create(Context, tag + ".done", fn);

    Value *base = nullptr;
    Value *end = nullptr;
    computeRange(level, binding, base, end);
    BasicBlock *preheader = Builder.GetInsertBlock();
    Builder.CreateBr(cond);

    Builder.SetInsertPoint(cond);
    PHINode *idx = Builder.CreatePHI(i64Ty, 2, tag + ".idx");
    idx->addIncoming(base, preheader);
    Builder.CreateCondBr(Builder.CreateICmpSLT(idx, end, tag + ".more"), body, done);

    Builder.SetInsertPoint(body);
    Value *candidate = loadCandidate(level, idx);
    emitFilters(level, binding, candidate, inc);

    binding.push_back(candidate);
    emitLevel(level + 1, binding, inc);
    binding.pop_back();

    Builder.SetInsertPoint(inc);
    Value *next = Builder.CreateAdd(idx, ConstantInt::get(i64Ty, 1), tag + ".next");
    idx->addIncoming(next, inc);
    Builder.CreateBr(cond);

    Builder.SetInsertPoint(done);
    Builder.CreateBr(exitTarget);
    Builder.SetInsertPoint(exitTarget);
}

void MotifIRBuilder::emitNest(const GraphInputs &graph, bool applyCanonical,
                              LeafEmitter leaf)
{
    Graph = graph;
    ApplyCanonical = applyCanonical;
    Leaf = std::move(leaf);

    Function *fn = Builder.GetInsertBlock()->getParent();
    BasicBlock *merge = BasicBlock::Create(Context, Label + ".merge", fn);

    SmallVector<Value *, kMotifMaxVars> binding;
    emitLevel(0, binding, merge);
    // emitLevel leaves the builder positioned at `merge`.
}

Value *MotifIRBuilder::emitCountOnly(const GraphInputs &graph, bool applyCanonical)
{
    Function *fn = Builder.GetInsertBlock()->getParent();
    Type *i64Ty = Builder.getInt64Ty();

    IRBuilder<> entry(&fn->getEntryBlock(), fn->getEntryBlock().begin());
    AllocaInst *counter = entry.CreateAlloca(i64Ty, nullptr, Label + ".count");
    Builder.CreateStore(ConstantInt::get(i64Ty, 0), counter);

    emitNest(graph, applyCanonical, [&](IRBuilder<> &b, ArrayRef<Value *>) {
        Value *cur = b.CreateLoad(i64Ty, counter, "cnt");
        b.CreateStore(b.CreateAdd(cur, ConstantInt::get(i64Ty, 1), "cnt.next"), counter);
    });

    return Builder.CreateLoad(i64Ty, counter, Label + ".total");
}
