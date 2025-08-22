#include "llvm/IR/Metadata.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DataLayout.h"
#include "DependencyGraph.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Operator.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

struct LoopBounds
{
    bool success = false;

    // Materialized values in i64
    llvm::Value *startV = nullptr;
    llvm::Value *stepV = nullptr;
    llvm::Value *tripV = nullptr; // iteration count
    llvm::Value *endV = nullptr;  // start + step*trip

    // constant facts
    bool constStart = false;
    int64_t startConst = 0;
    bool constStep = false;
    int64_t stepConst = 0;
    bool constTrip = false;
    uint64_t tripConst = 0;

    std::string msg;
};

struct LiveIn
{
    llvm::Value *original = nullptr;
    llvm::Type *type = nullptr;
    bool isPointer = false;
    bool isEmbedded = false; // marking decision for using memcpy
    unsigned index = 0;
};

struct Env
{
    llvm::Value *envAsI8 = nullptr;
    llvm::StructType *envTy = nullptr;
    llvm::SmallVector<LiveIn, 16> liveIn;
    bool success = false;
    std::string msg;
};

enum Correctness
{
    SAFE,
    COPY,
    REJECT
};

static bool isMallocLike(llvm::Value *V)
{
    if (auto *CI = llvm::dyn_cast<llvm::CallInst>(V))
    {
        llvm::Function *F = CI->getCalledFunction();
        if (!F)
            return false;
        llvm::StringRef name = F->getName();
        return name == "malloc" || name == "calloc" || name == "realloc" || name == "__libc_malloc";
    }
    return false;
}

static uint64_t getStoreSizeSafe(llvm::Type *Ty, const llvm::DataLayout &DL)
{
    using namespace llvm;
    if (!Ty)
        return 0;

    // Pointers are sized regardless of pointee under opaque ptrs
    if (auto *PT = dyn_cast<PointerType>(Ty))
        return DL.getPointerSize(PT->getAddressSpace());

    // Only sized types may query DL for size
    if (Ty->isSized())
        return DL.getTypeStoreSize(Ty);

    // Function, void, label, token, metadata, opaque structs… treat as unknown size
    return 0;
}

// Try to infer the pointee/element type for a pointer Value *Ptr in LLVM20+ (opaque pointers).
// Returns nullptr if not found.
static llvm::Type *inferPointeeType(llvm::Value *Ptr)
{
    using namespace llvm;
    if (!Ptr)
        return nullptr;

    // 1) If Ptr is a GEP-like operator, ask it for its result element type.
    if (auto *GEPOp = dyn_cast<GEPOperator>(Ptr))
    {
        // getResultElementType() exists for GEPOperator (returns the element type of the pointer result)
        if (Type *T = GEPOp->getResultElementType())
            return T;
    }
    // 2) If Ptr is an actual GetElementPtrInst (rare because GEPOperator covers it),
    if (auto *GEPInst = dyn_cast<GetElementPtrInst>(Ptr))
    {
        if (Type *T = GEPInst->getResultElementType())
            return T;
    }

    // 3) Look at users of the pointer: if someone loads from it, the load's type is the pointee
    for (User *U : Ptr->users())
    {
        if (auto *LI = dyn_cast<LoadInst>(U))
        {
            // LoadInst->getType() is the loaded element type
            llvm::errs() << "DEBUG: inferPointeeType LoadInst " << *LI << " Ptr: " << *LI->getPointerOperand() << "\n";
            if (Type *t = LI->getType())
                return t;
        }
        if (auto *SI = dyn_cast<StoreInst>(U))
        {
            llvm::errs() << "DEBUG: inferPointeeType StoreInst " << *SI << " Ptr: " << *SI->getPointerOperand() << "\n";
            // If the pointer is the pointer operand of a store, the value being stored
            // gives us a candidate element type
            if (SI->getPointerOperand() == Ptr)
            {
                if (Type *t = SI->getValueOperand()->getType())
                    return t;
            }
        }
        // Optional: inspect CallBase uses — if Ptr is passed as an argument and the callee's
        // declared param types exist (not always the case with opaque pointers), you may use them.
    }

    // 4) Nothing found — return nullptr (caller can decide fallback).
    return nullptr;
}

static bool typeContainsPointerRec(llvm::Type *Ty, llvm::SmallPtrSetImpl<llvm::Type *> &Visited)
{
    if (!Ty)
        return true;

    if (Ty->isPointerTy())
        return true;
    if (Ty->isIntegerTy() || Ty->isFloatingPointTy())
        return false;

    if (!Visited.insert(Ty).second)
        return false;

    if (auto *ST = llvm::dyn_cast<llvm::StructType>(Ty))
    {
        if (ST->isOpaque())
        {
            return true;
        }
        for (llvm::Type *Elt : ST->elements())
        {
            if (typeContainsPointerRec(Elt, Visited))
                return true;
        }
        return false;
    }

    if (auto *Arr = llvm::dyn_cast<llvm::ArrayType>(Ty))
    {
        return typeContainsPointerRec(Arr->getElementType(), Visited);
    }

    if (auto *Vec = llvm::dyn_cast<llvm::VectorType>(Ty))
    {
        return typeContainsPointerRec(Vec->getElementType(), Visited);
    }

    return true;
}

static bool typeContainsPointer(llvm::Type *Ty)
{
    llvm::SmallPtrSet<llvm::Type *, 8> Vis;
    return typeContainsPointerRec(Ty, Vis);
}

static llvm::Type *getPointedObjectTypeFromMemcpySrc(llvm::Value *Src)
{
    if (!Src)
        return nullptr;

    // follow casts/geps to underlying object
    llvm::Value *UO = llvm::getUnderlyingObject(Src);
    if (!UO)
        return nullptr;

    if (llvm::AllocaInst *AI = llvm::dyn_cast<llvm::AllocaInst>(UO))
    {
        // allocated type (e.g., [10 x i32] or i32)
        return AI->getAllocatedType();
    }

    if (llvm::GlobalVariable *GV = llvm::dyn_cast<llvm::GlobalVariable>(UO))
    {
        // global's element type
        return GV->getValueType();
    }

    if (llvm::Argument *Arg = llvm::dyn_cast<llvm::Argument>(UO))
    {
        // if argument is a pointer, return pointed-to type
        if (Arg->getType()->isPointerTy())
            return Arg->getType();
        return nullptr;
    }

    // If UO is a bitcast or constant expr of pointer, try to recover element type
    if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(UO))
    {
        llvm::errs() << "DEBUG: getPointedObjectTypeFromMemcpySrc Instruction: " << I << "\n";
        if (I->getType()->isPointerTy())
            return I->getType();
    }
    if (llvm::Constant *C = llvm::dyn_cast<llvm::Constant>(UO))
    {
        if (C->getType()->isPointerTy())
            return C->getType();
    }

    // fallback unknown
    return nullptr;
}

static bool isMemcpyIntrinsicOrLib(const llvm::CallBase *CB)
{
    if (!CB)
        return false;
    if (const llvm::IntrinsicInst *II = llvm::dyn_cast<llvm::IntrinsicInst>(CB))
    {
        llvm::Intrinsic::ID id = II->getIntrinsicID();
        return id == llvm::Intrinsic::memcpy || id == llvm::Intrinsic::memmove;
    }
    if (const llvm::Function *CF = CB->getCalledFunction())
    {
        llvm::StringRef name = CF->getName();
        return name == "memcpy" || name == "memmove" ||
               name == "__builtin_memcpy" || name == "__builtin_memmove";
    }
    return false;
}

static bool valueEscapes(llvm::Value *V, llvm::AAResults &AA, const llvm::DataLayout &DL, llvm::DenseMap<const llvm::Value *, bool> &Memo)
{
    if (!V)
        return false;

    auto MI = Memo.find(V);
    if (MI != Memo.end())
        return MI->second;

    Memo[V] = true; // assuming it escapes until proven otherwise

    for (llvm::Use &use : V->uses())
    {
        llvm::User *U = use.getUser();
        llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(U);
        llvm::errs() << "DEBUG: valueEscapes Instruction: " << I << "\n";
        if (!I)
        {
            Memo[V] = true;
            return true; // unhandled or weird use
        }

        if (llvm::StoreInst *SI = llvm::dyn_cast<llvm::StoreInst>(I))
        {
            llvm::errs() << "DEBUG: valueEscapes StoreInst " << *SI << " Ptr: " << *SI->getPointerOperand() << "\n";
            if (SI->getValueOperand() == V || llvm::getUnderlyingObject(SI->getPointerOperand()) == V)
            {
                Memo[V] = true;
                return true;
            }
            continue;
        }

        if (llvm::isa<llvm::ReturnInst>(I))
        {
            Memo[V] = true;
            return true; // escaping through return
        }

        if (llvm::CallBase *CB = llvm::dyn_cast<llvm::CallBase>(I))
        {

            if (CB->isInlineAsm())
            {
                Memo[V] = true;
                return true; // inline asm can escape
            }

            if (isMemcpyIntrinsicOrLib(CB))
            {
                llvm::Value *Dest = nullptr;
                llvm::Value *Src = nullptr;

                if (llvm::IntrinsicInst *II = llvm::dyn_cast<llvm::IntrinsicInst>(CB))
                {
                    if (II->getIntrinsicID() == llvm::Intrinsic::memcpy ||
                        II->getIntrinsicID() == llvm::Intrinsic::memmove)
                    {
                        if (II->arg_size() >= 2)
                        {
                            Dest = II->getArgOperand(0);
                            Src = II->getArgOperand(1);
                        }
                    }
                }
                else
                {
                    unsigned nargs = CB->arg_size();
                    if (nargs >= 2)
                    {
                        Dest = CB->getArgOperand(0);
                        Src = CB->getArgOperand(1);
                    }
                }

                if (!Src || !Dest)
                {
                    Memo[V] = true;
                    return true;
                }

                if (llvm::getUnderlyingObject(Src) == V || Src == V)
                {
                    if (valueEscapes(Dest, AA, DL, Memo))
                    {
                        llvm::Type *ObjTy = getPointedObjectTypeFromMemcpySrc(Src);
                        if (!ObjTy)
                        {
                            Memo[V] = true;
                            return true; // can't determine pointed type
                        }

                        if (typeContainsPointer(ObjTy))
                        {
                            Memo[V] = true;
                            return true; // memcpy src contains pointers
                        }

                        continue; // memcpy src is safe
                    }

                    else
                    {
                        continue;
                    }
                }

                else
                {
                    continue; // memcpy src is not V
                }
            }

            int argIdx = -1;
            for (unsigned i = 0, e = CB->arg_size(); i < e; ++i)
            {
                if (CB->getArgOperand(i) == V)
                {
                    argIdx = (int)i;
                    break;
                }
            }

            if (argIdx == -1)
            {
                Memo[V] = true;
                return true; // V is not an argument of this call
            }

            if (!CB->doesNotCapture(argIdx))
            {
                Memo[V] = true;
                return true; // V is captured by this call
            }

            continue; // V is an argument but does not escape
        }

        if (llvm::GetElementPtrInst *GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(I))
        {
            if (valueEscapes(GEP, AA, DL, Memo))
            {
                Memo[V] = true;
                return true; // GEP escapes
            }
            continue; // GEP does not escape
        }

        if (llvm::BitCastInst *BC = llvm::dyn_cast<llvm::BitCastInst>(I))
        {
            if (valueEscapes(BC, AA, DL, Memo))
            {
                Memo[V] = true;
                return true;
            }
            continue;
        }
        if (llvm::AddrSpaceCastInst *ASC = llvm::dyn_cast<llvm::AddrSpaceCastInst>(I))
        {
            if (valueEscapes(ASC, AA, DL, Memo))
            {
                Memo[V] = true;
                return true;
            }
            continue;
        }

        if (llvm::isa<llvm::MemCpyInst>(I))
        {
            llvm::MemCpyInst *MCI = llvm::cast<llvm::MemCpyInst>(I);
            if (MCI->getRawSource() == V)
            {
                if (valueEscapes(MCI->getRawDest(), AA, DL, Memo))
                {
                    llvm::Type *ObjTy = getPointedObjectTypeFromMemcpySrc(V);
                    if (!ObjTy)
                    {
                        Memo[V] = true;
                        return true;
                    }
                    if (typeContainsPointer(ObjTy))
                    {
                        Memo[V] = true;
                        return true;
                    }
                    continue;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                // dest or other use -> safe
                continue;
            }
        }

        if (llvm::isa<llvm::LoadInst>(I))
        {
            llvm::errs() << "DEBUG: valueEscapes LoadInst " << I << "\n";
            continue;
        }

        Memo[V] = true;
        return true;
    }

    Memo[V] = false;
    return false;
}

static bool allocaEscapes(llvm::AllocaInst *AI, llvm::AAResults &AA, const llvm::DataLayout &DL)
{
    llvm::DenseMap<const llvm::Value *, bool> memo;
    return valueEscapes(AI, AA, DL, memo);
}

Correctness safety(llvm::Value *ptr, const llvm::DataLayout &DL, llvm::AAResults &AA)
{
    if (!ptr)
        return REJECT;
    llvm::Value *UO = llvm::getUnderlyingObject(ptr);
    if (!UO)
        return REJECT;
    llvm::MemoryLocation Loc(ptr, llvm::LocationSize::precise(getStoreSizeSafe(ptr->getType(), DL)));

    if (llvm::isa<llvm::GlobalVariable>(UO))
        return SAFE;

    if (auto *Call = llvm::dyn_cast<llvm::CallBase>(UO))
    {
        if (isMallocLike(Call))
            return SAFE;
    }

    if (auto *AI = llvm::dyn_cast<llvm::AllocaInst>(UO))
    {
        if (allocaEscapes(AI, AA, DL))
        {
            llvm::errs() << "[safety] alloca escapes: " << *AI << "\n";
            return REJECT;
        }
        else
        {
            return COPY;
        }
    }

    else if (llvm::isa<llvm::Argument>(UO) || llvm::isa<llvm::PHINode>(UO))
    {
        return REJECT; // alloca is safe to copy
    }

    return REJECT;
}

static void collectLiveInsForLoop(llvm::Loop *L, llvm::SmallVectorImpl<llvm::Value *> &liveInsOut)
{
    llvm::SmallPtrSet<llvm::Value *, 16> seen;
    llvm::SmallPtrSet<llvm::BasicBlock *, 16> loopBlocks;
    // gather loop blocks
    for (llvm::BasicBlock *BB : L->getBlocks())
        loopBlocks.insert(BB);

    llvm::BasicBlock *header = L->getHeader();
    llvm::BasicBlock *preheader = L->getLoopPreheader();

    // First: handle header PHIs specially - these often represent canonical live-ins.
    if (header)
    {
        for (llvm::PHINode &PN : header->phis())
        {
            // If there's a preheader incoming, that incoming VALUE is a live-in,
            // not the incoming BLOCK.
            if (preheader)
            {
                llvm::Value *inc = PN.getIncomingValueForBlock(preheader);
                if (inc && !llvm::isa<llvm::BasicBlock>(inc) && !llvm::isa<llvm::BlockAddress>(inc))
                {
                    bool definedInsideLoop = false;
                    if (llvm::Instruction *incInst = llvm::dyn_cast<llvm::Instruction>(inc))
                    {
                        llvm::BasicBlock *defBB = incInst->getParent();
                        if (loopBlocks.count(defBB))
                            definedInsideLoop = true;
                    }
                    if (!definedInsideLoop && seen.insert(inc).second)
                        liveInsOut.push_back(inc);
                }
            }
        }
    }

    // Then: for each instruction in the loop, inspect operands and collect those
    // that are defined outside the loop.
    for (llvm::BasicBlock *BB : L->getBlocks())
    {
        for (llvm::Instruction &I : *BB)
        {
            for (llvm::Use &U : I.operands())
            {
                llvm::Value *op = U.get();

                // Filter out non-data values (BasicBlock, BlockAddress, metadata, token types)
                if (!op || llvm::isa<llvm::BasicBlock>(op) || llvm::isa<llvm::BlockAddress>(op))
                    continue;

                // If operand is an instruction, check its parent block: if inside loop -> skip
                if (llvm::Instruction *opI = llvm::dyn_cast<llvm::Instruction>(op))
                {
                    llvm::BasicBlock *defBB = opI->getParent();
                    if (loopBlocks.count(defBB))
                        continue;
                }

                // Arguments and constants are OK; values defined outside the loop are OK
                if (seen.insert(op).second)
                {
                    liveInsOut.push_back(op);
                }
            }
        }
    }
}

struct Env createEnv(llvm::Loop *L, llvm::Function &F, llvm::FunctionAnalysisManager &FAM, llvm::AAResults &A, llvm::DependencyGraph &G, llvm::Instruction *insertPt, uint64_t maxEmbedSize = 128)
{
    Env E;
    if (!L || !insertPt)
    {
        E.msg = "No Loop or Insert Point";
        return E;
    }

    llvm::Module *M = F.getParent();
    const llvm::DataLayout &DL = M->getDataLayout();
    llvm::LLVMContext &ctx = F.getContext();
    llvm::IRBuilder<> Builder(insertPt);

    llvm::SmallVector<llvm::Value *, 16> liveIn;
    collectLiveInsForLoop(L, liveIn);

    if (liveIn.empty())
    {
        E.success = true;
        E.msg = "No live-in values";
        E.envAsI8 = llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0));
        E.envTy = nullptr;
        return E;
    }

    llvm::SmallVector<llvm::Type *, 16> fieldTypes;
    llvm::SmallVector<LiveIn, 16> fields;
    fields.reserve(liveIn.size());

    for (unsigned i = 0; i < liveIn.size(); ++i)
    {
        llvm::Value *V = liveIn[i];
        LiveIn LI;
        LI.original = V;
        LI.index = i;

        if (llvm::isa<llvm::Constant>(V) || llvm::isa<llvm::GlobalValue>(V))
            continue;

        Correctness c = safety(V, DL, A);

        if (c == REJECT)
        {
            llvm::errs() << "[env] rejecting live-in value: " << V->getName().str() << "\n";
            return E;
        }

        llvm::Type *fieldTy = nullptr;
        if (c == SAFE)
        {
            if (V->getType()->isPointerTy())
            {
                fieldTy = llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
                LI.isPointer = true;
                LI.isEmbedded = false;
            }

            else if (V->getType()->isAggregateType())
            {
                uint64_t size = V->getType()->isSized() ? getStoreSizeSafe(V->getType(), DL) : 0;
                if (size > maxEmbedSize)
                {
                    fieldTy = llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
                    LI.isPointer = true;
                    LI.isEmbedded = false;
                }
                else
                {
                    fieldTy = V->getType();
                    LI.isPointer = false;
                    LI.isEmbedded = true;
                }
            }

            else
            {
                // scalar types (int, float, etc.). Storing by value
                fieldTy = V->getType();
                LI.isPointer = false;
                LI.isEmbedded = true;
            }
        }

        else
        {
            llvm::Value *UO = llvm::getUnderlyingObject(V);

            if (auto *AI = llvm::dyn_cast<llvm::AllocaInst>(UO))
            {
                llvm::Type *allocTy = AI->getAllocatedType();
                fieldTy = allocTy;
                LI.isEmbedded = true;
                LI.isPointer = false;
            }

            else if (V->getType()->isAggregateType())
            {
                fieldTy = V->getType();
                LI.isEmbedded = true;
                LI.isPointer = false;
            }

            else
            {
                // fallback to storing by value
                fieldTy = V->getType();
                LI.isEmbedded = false;
                LI.isPointer = false;
            }
        }

        LI.type = fieldTy;
        fieldTypes.push_back(fieldTy);
        fields.push_back(LI);
    }

    if (fields.empty())
    {
        E.success = true;
        E.msg = "No valid live-in values";
        E.envAsI8 = llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0));
        E.envTy = nullptr;
        return E;
    }

    std::string envName = (F.getName() + ".loopenv").str();
    llvm::StructType *envTy = llvm::StructType::create(ctx, fieldTypes, envName, false);

    llvm::Type *I8PtrTy = llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
    llvm::Type *SizeTy = DL.getIntPtrType(ctx);

    uint64_t envBytes = DL.getTypeAllocSize(envTy);
    llvm::Constant *envSize = llvm::ConstantInt::get(SizeTy, envBytes);

    llvm::FunctionCallee mallocFunc = M->getOrInsertFunction("malloc", llvm::FunctionType::get(I8PtrTy, {SizeTy}, false));

    llvm::Value *raw = Builder.CreateCall(mallocFunc, {envSize}, envName + ".raw");
    llvm::Value *envPtr = Builder.CreateBitCast(raw, envTy->getPointerTo(), envName + ".ptr");

    for (unsigned i = 0; i < fields.size(); ++i)
    {
        LiveIn &LI = fields[i];
        llvm::Value *destGEP = Builder.CreateStructGEP(envTy, envPtr, i, envName + ".gep." + std::to_string(i));

        if (LI.isPointer)
        {
            if (LI.original->getType()->isPointerTy())
            {
                llvm::Value *store = (LI.original->getType() == LI.type) ? LI.original : Builder.CreateBitCast(LI.original, LI.type);
                Builder.CreateStore(store, destGEP);
            }

            else
            {
                llvm::Value *UO = llvm::getUnderlyingObject(LI.original);

                if (!UO)
                {
                    llvm::Value *null = llvm::ConstantPointerNull::get(llvm::PointerType::get(LI.type, 0));
                    Builder.CreateStore(null, destGEP);
                }

                else
                {
                    llvm::Value *addr = nullptr;

                    if (UO->getType()->isPointerTy())
                        addr = UO;

                    else
                        addr = llvm::ConstantPointerNull::get(llvm::PointerType::get(LI.type, 0));

                    if (addr->getType() != LI.type)
                        addr = Builder.CreateBitCast(addr, LI.type);

                    Builder.CreateStore(addr, destGEP);
                }
            }
        }

        else if (LI.isEmbedded)
        {
            llvm::Value *srcPtr = nullptr;

            llvm::Value *UO = llvm::getUnderlyingObject(LI.original);

            if (auto *AI = llvm::dyn_cast<llvm::AllocaInst>(UO))
            {
                srcPtr = AI;
            }

            else if (LI.original->getType()->isPointerTy())
            {
                srcPtr = LI.original;
            }

            else
            {
                // if an aggregate lives in ssa register instead of the memory
                llvm::Type *aggType = LI.type;
                llvm::AllocaInst *tmp = new llvm::AllocaInst(aggType, 0, envName + ".tmp", insertPt);
                Builder.Insert(tmp);
                Builder.CreateStore(LI.original, tmp);
                srcPtr = tmp;
            }

            llvm::Value *destPtr = Builder.CreateBitCast(destGEP, llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0));
            llvm::Value *srcPtrCast = Builder.CreateBitCast(srcPtr, llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0));
            uint64_t size = getStoreSizeSafe(LI.type, DL);
            llvm::Value *bytesC = llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), size);
            Builder.CreateMemCpy(destPtr, llvm::MaybeAlign(1), srcPtrCast, llvm::MaybeAlign(1), bytesC);
        }
        // store by value case
        else
        {
            llvm::Value *tostore = LI.original;
            if (tostore->getType() != LI.type)
            {
                if (tostore->getType()->isIntegerTy() && LI.type->isIntegerTy())
                {
                    unsigned from = tostore->getType()->getIntegerBitWidth();
                    unsigned to = LI.type->getIntegerBitWidth();
                    if (from < to)
                        tostore = Builder.CreateSExt(tostore, LI.type);
                    else if (from > to)
                        tostore = Builder.CreateTrunc(tostore, LI.type);
                }
                else if (tostore->getType()->isPointerTy() && LI.type->isPointerTy())
                {
                    tostore = Builder.CreateBitCast(tostore, LI.type);
                }
                else
                    tostore = llvm::UndefValue::get(LI.type);
            }
            Builder.CreateStore(tostore, destGEP);
        }
    }

    E.success = true;
    E.envAsI8 = Builder.CreateBitCast(envPtr, llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0), envName + ".env_as_i8");
    E.envTy = envTy;
    E.liveIn = std::move(fields);
    E.msg = "OK";
    return E;
}

static bool loopParallelizable(llvm::Loop *L)
{
    if (!L)
        return false;

    // Try canonical loop-id first
    llvm::MDNode *LoopID = L->getLoopID();
    // Fallback: terminator metadata (older style)
    if (!LoopID && L->getHeader())
    {
        if (llvm::Instruction *T = L->getHeader()->getTerminator())
            LoopID = T->getMetadata("llvm.loop");
    }
    if (!LoopID)
    {
        llvm::errs() << "[bounds] no llvm.loop metadata on loop header '" << L->getHeader()->getName() << "'\n";
        return false;
    }

    bool found = false;
    for (unsigned i = 0, e = LoopID->getNumOperands(); i < e; ++i)
    {
        llvm::Metadata *Op = LoopID->getOperand(i);
        llvm::errs() << "DEBUG: loopParallelisable Operand retrieved" << Op << "\n";
        if (!Op)
            continue;

        // Case A: inner MDNode like !{!"llvm.loop.parallelize.enable", i1 true}
        if (auto *MDN = llvm::dyn_cast<llvm::MDNode>(Op))
        {
            if (MDN->getNumOperands() >= 1)
            {
                if (auto *S = llvm::dyn_cast<llvm::MDString>(MDN->getOperand(0)))
                {
                    llvm::StringRef Name = S->getString();
                    if (Name == "DOALL")
                        return true;
                    if (Name == "llvm.loop.parallelize.enable")
                    {
                        if (MDN->getNumOperands() >= 2)
                        {
                            if (auto *CAM = llvm::dyn_cast_or_null<llvm::ConstantAsMetadata>(MDN->getOperand(1)))
                                if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(CAM->getValue()))
                                    if (CI->isOne())
                                        return true;
                        }
                    }
                }
            }
        }

        // Case B: direct MDString operand !"DOALL"
        if (auto *S = llvm::dyn_cast<llvm::MDString>(Op))
        {
            if (S->getString() == "DOALL")
                return true;
        }
    }

    llvm::errs() << "[bounds] loop '" << L->getHeader()->getName() << "' not marked DOALL/parallelize\n";
    return false;
}

static llvm::Value *materialise(const llvm::SCEV *S, llvm::ScalarEvolution &SE,
                                const llvm::DataLayout &DL, llvm::Instruction *InsertPt,
                                const llvm::Twine &Name = "scev_val")
{

    if (!S)
        return nullptr;
    llvm::LLVMContext &ctx = InsertPt->getContext();
    llvm::Type *I64 = llvm::Type::getInt64Ty(ctx);

    if (auto *C = llvm::dyn_cast<llvm::SCEVConstant>(S))
    {
        return llvm::ConstantInt::get(I64, C->getValue()->getSExtValue());
    }

    llvm::SCEVExpander Exp(SE, DL, "scev_expander");
    llvm::Value *V = Exp.expandCodeFor(S, I64, InsertPt);
    if (V->getType() != I64)
    {
        llvm::IRBuilder<> Builder(InsertPt);
        if (V->getType()->isIntegerTy())
        {
            V = Builder.CreateSExtOrTrunc(V, I64, Name);
        }
        else
        {
            V = Builder.CreatePtrToInt(V, I64, Name);
        }
    }

    return V;
}

static LoopBounds extractLoopBounds(llvm::Loop *L, llvm::Function &F,
                                    llvm::FunctionAnalysisManager &FAM, llvm::Instruction *InsertPt)
{

    LoopBounds R;
    if (!L)
    {
        R.msg = "No Loop";
        return R;
    }

    llvm::BasicBlock *preheader = L->getLoopPreheader();
    llvm::BasicBlock *latch = L->getLoopLatch();
    llvm::errs() << "[debug] header=" << L->getHeader()->getName();
    if (preheader)
        llvm::errs() << " preheader=" << preheader->getName();
    else
        llvm::errs() << " preheader=null";

    if (latch)
        llvm::errs() << " latch=" << latch->getName();
    else
        llvm::errs() << " latch=null";
    llvm::errs() << "\n";

    llvm::ScalarEvolution &SE = FAM.getResult<llvm::ScalarEvolutionAnalysis>(F);
    llvm::Module *M = F.getParent();
    const llvm::DataLayout &DL = M->getDataLayout();
    llvm::LLVMContext &ctx = F.getContext();
    llvm::Type *I64 = llvm::Type::getInt64Ty(ctx);

    llvm::PHINode *indPhi = nullptr;

    for (llvm::PHINode &phi : L->getHeader()->phis())
    {
        if (const llvm::SCEV *S = SE.getSCEV(&phi))
            if (const auto *AR = llvm::dyn_cast<llvm::SCEVAddRecExpr>(S))
                if (AR->getLoop() == L)
                {
                    indPhi = &phi;
                    break;
                }
    }

    if (!indPhi)
    {
        R.msg = "No canonical induction PHI found";
        return R;
    }

    const llvm::SCEV *PhiS = SE.getSCEV(indPhi);
    const auto *AR = llvm::dyn_cast<llvm::SCEVAddRecExpr>(PhiS);
    if (!AR)
    {
        R.msg = "Induction PHI is not AddRec";
        return R;
    }

    const llvm::SCEV *startS = AR->getStart();
    const llvm::SCEV *StepS = AR->getStepRecurrence(SE) ? AR->getStepRecurrence(SE) : (AR->getNumOperands() >= 2 ? AR->getOperand(1) : nullptr);

    if (!StepS)
    {
        R.msg = "Could not extract step SCEV";
        return R;
    }

    const llvm::SCEV *BEC = SE.getBackedgeTakenCount(L);

    llvm::Value *StartV = materialise(startS, SE, DL, InsertPt, "loop_start");
    llvm::Value *StepV = materialise(StepS, SE, DL, InsertPt, "loop_step");

    if (const auto *C = llvm::dyn_cast<llvm::SCEVConstant>(startS))
    {
        R.constStart = true;
        R.startConst = C->getAPInt().getSExtValue();
    }
    if (const auto *C = llvm::dyn_cast<llvm::SCEVConstant>(StepS))
    {
        R.constStep = true;
        R.stepConst = C->getAPInt().getSExtValue();
    }

    llvm::Value *TripV = nullptr;
    if (const auto *Cbec = llvm::dyn_cast<llvm::SCEVConstant>(BEC))
    {
        uint64_t bec = Cbec->getAPInt().getZExtValue();
        uint64_t trip = bec + 1;
        TripV = llvm::ConstantInt::get(I64, trip);
        R.constTrip = true;
        R.tripConst = trip;
    }
    else if (BEC && !llvm::isa<llvm::SCEVUnknown>(BEC))
    {
        // trip = bec + 1
        llvm::SCEVExpander Exp(SE, DL, "scev.expand");
        llvm::Value *BecV = Exp.expandCodeFor(BEC, I64, InsertPt);
        llvm::IRBuilder<> B(InsertPt);
        TripV = B.CreateAdd(BecV, llvm::ConstantInt::get(I64, 1), "trip");
    }
    else
    {
        // Fallback: single exit count
        llvm::SmallVector<llvm::BasicBlock *, 4> Exits;
        L->getExitBlocks(Exits);
        if (Exits.size() == 1)
        {
            const llvm::SCEV *ExitCount = SE.getExitCount(L, Exits[0]);
            if (const auto *Cc = llvm::dyn_cast_or_null<llvm::SCEVConstant>(ExitCount))
            {
                uint64_t trip = Cc->getAPInt().getZExtValue();
                TripV = llvm::ConstantInt::get(I64, trip);
                R.constTrip = true;
                R.tripConst = trip;
            }
            else if (ExitCount)
            {
                TripV = materialise(ExitCount, SE, DL, InsertPt, "trip");
            }
        }
    }

    if (!StartV || !StepV || !TripV)
    {
        R.msg = "Failed to materialize start/step/trip";
        return R;
    }

    // end = start + step * trip
    llvm::IRBuilder<> B(InsertPt);
    llvm::Value *Mul = B.CreateMul(StepV, TripV, "step_mul_trip");
    llvm::Value *EndV = B.CreateAdd(StartV, Mul, "loop_end");

    R.success = true;
    R.startV = StartV;
    R.stepV = StepV;
    R.tripV = TripV;
    R.endV = EndV;
    R.msg = "OK";
    return R;
}

// Outline a simple loop body into: void(i64 idx, i8* env)
// Returns nullptr on failure.
static llvm::Function *outlineSimpleLoop(llvm::Loop *L, llvm::Function &F, Env &E, llvm::Instruction *InsertPt)
{
    using namespace llvm;
    if (!L || !InsertPt || !E.success)
        return nullptr;
    LLVMContext &ctx = F.getContext();
    Module *M = F.getParent();

    Type *I64Ty = Type::getInt64Ty(ctx);
    Type *I8PtrTy = PointerType::getUnqual(Type::getInt8Ty(ctx));
    FunctionType *FT = FunctionType::get(Type::getVoidTy(ctx), {I64Ty, I8PtrTy}, false);
    Function *outlined = Function::Create(FT, Function::InternalLinkage, F.getName() + ".outlined", M);

    // name args
    auto ArgIt = outlined->arg_begin();
    Argument *idxArg = &*ArgIt++;
    idxArg->setName("idx");
    Argument *envArg = &*ArgIt++;
    envArg->setName("env");

    // --- entry for outlined function ---
    BasicBlock *entry = BasicBlock::Create(ctx, "entry", outlined);
    IRBuilder<> B(entry);

    // dedicated exit block for outlined function — used instead of branching to 'entry'
    BasicBlock *exitBB = BasicBlock::Create(ctx, "exit", outlined);
    IRBuilder<> exitBuilder(exitBB);
    exitBuilder.CreateRetVoid();

    // prepare mapping for replaced values inside outlined function
    ValueToValueMapTy VMap;

    // find canonical induction PHI (if any) in header (best-effort)
    PHINode *indPhi = nullptr;
    if (BasicBlock *Header = L->getHeader())
    {
        for (PHINode &phi : Header->phis())
        {
            // Heuristic: first PHI in header is often the induction phi
            if (phi.getNumIncomingValues() && phi.getParent() == Header)
            {
                indPhi = &phi;
                break;
            }
        }
    }

    // map induction phi -> converted idxArg (cast/trunc/sext to phi type)
    if (indPhi)
    {
        Type *phiTy = indPhi->getType();
        Value *phiValue = idxArg;
        if (phiTy != I64Ty)
        {
            if (phiTy->isIntegerTy())
            {
                unsigned fw = phiTy->getIntegerBitWidth();
                if (fw < 64)
                    phiValue = B.CreateTrunc(idxArg, phiTy, "idx_trunc");
                else
                    phiValue = B.CreateSExtOrTrunc(idxArg, phiTy, "idx_sext");
            }
            else
            {
                // non-integer phi: fallback to undef
                phiValue = UndefValue::get(phiTy);
            }
        }
        VMap[indPhi] = phiValue;
    }

    // --- Build env initializer in the preheader (we will allocate env there and store field values) ---
    BasicBlock *preheader = L->getLoopPreheader();
    if (!preheader)
    {
        // Try to fall back to the parent block of InsertPt if no proper preheader
        preheader = InsertPt->getParent();
    }

    Function *parentF = InsertPt->getFunction();
    IRBuilder<> PHB(preheader->getTerminator());                                     // builder at end of preheader
    IRBuilder<> entryB(&parentF->getEntryBlock(), parentF->getEntryBlock().begin()); // for entry allocas

    // We'll collect (fieldType, initValueOrNull, ptrInitFlag) for each live-in.
    // If initIsPtr==true, initVals[i] contains a pointer (Value*) - we will load from it at preheader
    // If initIsPtr==false, initVals[i] contains a scalar value available at preheader (or constant).
    SmallVector<Type *, 8> fieldTypes;
    SmallVector<Value *, 8> initVals;
    SmallVector<bool, 8> initIsPtr;
    SmallVector<Value *, 8> createdShadowAlloca; // track shadow allocas we added in entry (or nullptr)

    // Helper lambda to strip pointer casts
    auto stripPtr = [](Value *v) -> Value *
    {
        if (!v)
            return nullptr;
#if LLVM_VERSION_MAJOR >= 16
        return v->stripPointerCasts();
#else
        return v->stripPointerCasts();
#endif
    };

    // For each live-in from E.liveIn compute initialiser
    for (unsigned i = 0; i < E.liveIn.size(); ++i)
    {
        LiveIn &LI = E.liveIn[i];
        Value *orig = LI.original;
        Value *chosenInit = nullptr;
        bool chosenIsPtr = false;
        Type *chosenType = nullptr;
        createdShadowAlloca.push_back(nullptr);

        // 1) Constant -> capture directly
        if (isa<Constant>(orig))
        {
            chosenInit = orig;
            chosenIsPtr = false;
            chosenType = orig->getType();
        }
        else if (PHINode *PN = dyn_cast<PHINode>(orig))
        {
            // 2) PHI: prefer incoming value for preheader (if any)
            if (preheader)
            {
                Value *inc = PN->getIncomingValueForBlock(preheader);
                if (inc && inc != PN)
                {
                    chosenInit = inc;
                    chosenIsPtr = false;
                    chosenType = inc->getType();
                }
            }
        }

        // 3) If not chosen: if orig is an instruction defined outside the loop, reuse it
        if (!chosenInit)
        {
            if (Instruction *I = dyn_cast<Instruction>(orig))
            {
                BasicBlock *defBB = I->getParent();
                if (!L->contains(defBB))
                {
                    // conservative: reuse I as long as it is defined outside the loop
                    chosenInit = I;
                    chosenIsPtr = false;
                    chosenType = I->getType();
                }
            }
        }

        // 4) If orig is a load from an alloca in entry: capture the alloca pointer, load at preheader
        if (!chosenInit)
        {
            if (LoadInst *LIload = dyn_cast<LoadInst>(orig))
            {
                Value *ptr = stripPtr(LIload->getPointerOperand());
                if (AllocaInst *A = dyn_cast_or_null<AllocaInst>(ptr))
                {
                    // we will load from this alloca at preheader to obtain the init value
                    chosenInit = A;     // store pointer for later load
                    chosenIsPtr = true; // signal we need to load at preheader
                    chosenType = LIload->getType();
                }
            }
        }

        // 5) If orig itself is an alloca (caller used address-of): capture the pointer
        if (!chosenInit)
        {
            if (AllocaInst *A = dyn_cast<AllocaInst>(orig))
            {
                chosenInit = A;
                chosenIsPtr = true; // will load from it if field is scalar, or capture pointer if field expects pointer
                chosenType = A->getAllocatedType();
            }
        }

        // 6) Fallback: create a shadow alloca in the entry block so we have some addressable storage
        if (!chosenInit)
        {
            // Create shadow alloca of the original type in entry
            Type *fallbackTy = orig->getType();
            if (!fallbackTy || fallbackTy->isVoidTy())
            {
                // pick i32 as super-safe fallback
                fallbackTy = Type::getInt32Ty(ctx);
            }
            AllocaInst *shadow = entryB.CreateAlloca(fallbackTy, nullptr, "livein.shadow");
            // We'll store something into shadow in the preheader (best-effort); for now store undef
            chosenInit = shadow;
            chosenIsPtr = true;
            chosenType = fallbackTy;
            createdShadowAlloca.back() = shadow;
        }

        // record field types and initialisers
        // For env we will store the scalar value type (if pointer chosen, the field will be the pointee type)
        Type *fieldTy = nullptr;
        if (chosenIsPtr)
        {
            // chosenInit is a pointer to memory holding the value; field should be pointee type
            if (PointerType *PT = dyn_cast<PointerType>(chosenInit->getType()))
            {
                fieldTy = inferPointeeType(chosenInit);
            }
            else if (AllocaInst *A = dyn_cast<AllocaInst>(chosenInit))
            {
                fieldTy = A->getAllocatedType();
            }
            else
            {
                // fallback
                fieldTy = chosenType ? chosenType : Type::getInt32Ty(ctx);
            }
        }
        else
        {
            fieldTy = chosenType ? chosenType : Type::getInt32Ty(ctx);
        }

        fieldTypes.push_back(fieldTy);
        initVals.push_back(chosenInit);
        initIsPtr.push_back(chosenIsPtr);
    }

    // If there are no live-ins, we can keep simple path: but we still create outlined function and clone blocks below
    // Build an env struct type if not present in E (but the pass likely already filled E.envTy)
    StructType *envTy = E.envTy ? E.envTy : StructType::create(ctx, fieldTypes, F.getName().str() + ".env");

    // If the Env in E has different layout, we still use envTy computed here for initialization and mapping.
    // Create an env alloca in preheader and initialize its fields
    AllocaInst *envAlloca = nullptr;
    {
        // Place env alloca near the end of preheader (before terminator)
        envAlloca = PHB.CreateAlloca(envTy, nullptr, "loop.env");
        // Store each field
        for (unsigned i = 0; i < fieldTypes.size(); ++i)
        {
            Type *fTy = fieldTypes[i];
            Value *repr = initVals[i];
            bool isPtr = initIsPtr[i];

            // Compute the scalar value to store into env field
            Value *valToStore = nullptr;
            if (!isPtr)
            {
                // repr is a value available at preheader (hopefully). If it's an instruction defined elsewhere we can use it directly.
                valToStore = repr;
            }
            else
            {
                // repr is a pointer (Alloca or similar) - load from it at preheader
                Value *ptr = repr;
                // If repr is not actually a pointer type, bitcast it
                if (!ptr->getType()->isPointerTy())
                {
                    // try bitcast if possible (fallback)
                    ptr = PHB.CreateBitCast(ptr, PointerType::getUnqual(fTy), "env.ptr.cast");
                }
                // load the pointee into valToStore
                valToStore = PHB.CreateLoad(fTy, ptr, "env.field.load");
            }

            // Store into struct field
            Value *fieldPtr = PHB.CreateStructGEP(envTy, envAlloca, i, "env.gep");
            // Make sure types match: fieldPtr is a pointer to field type
            Type *wantPtrTy = PointerType::getUnqual(fTy);
            if (fieldPtr->getType() != wantPtrTy)
            {
                fieldPtr = PHB.CreateBitCast(fieldPtr, wantPtrTy, "env.gep.cast");
            }
            PHB.CreateStore(valToStore, fieldPtr);
        }
    }

    // Note: some other code in your pass previously created the Env layout (E.envTy). If you need E.envTy to match the one we built,
    // you should update E.envTy = envTy and E.liveIn -> field mapping accordingly. For safety, do:
    E.envTy = envTy;

    // --- Now inside the outlined function: map the original live-in values to loads from the envArg ---
    // bitcast envArg to envTy*
    Value *envPtrArg = B.CreateBitCast(envArg, PointerType::getUnqual(envTy), "env.ptr");

    for (unsigned i = 0; i < E.liveIn.size(); ++i)
    {
        LiveIn &LI = E.liveIn[i];
        Value *gep = B.CreateStructGEP(envTy, envPtrArg, i, "env.gep.out");
        Type *elemTy = fieldTypes[i];

        // pointer to the field value
        Value *wantPtr = gep;
        Type *wantPtrTy = PointerType::getUnqual(elemTy);
        if (wantPtr->getType() != wantPtrTy)
        {
            if (wantPtr->getType()->isPointerTy())
                wantPtr = B.CreatePointerCast(wantPtr, wantPtrTy, "cast.ptr.for.load");
            else
                wantPtr = B.CreateBitCast(wantPtr, wantPtrTy, "bitcast.ptr.for.load");
        }

        // load value
        LoadInst *loaded = B.CreateLoad(elemTy, wantPtr, "loaded_field");
        llvm::errs() << "DEBUG: outlineSimpleLoop LoadInst " << *loaded << " Ptr: " << *loaded->getPointerOperand() << "\n";

        // map original live-in value -> loaded value inside outlined function
        VMap[LI.original] = loaded;
    }

    // identify body-entry: the header's successor inside the loop
    BasicBlock *Header = L->getHeader();
    BasicBlock *bodyEntry = nullptr;
    for (succ_iterator SI = succ_begin(Header), SE = succ_end(Header); SI != SE; ++SI)
    {
        BasicBlock *s = *SI;
        if (L->contains(s))
        {
            if (bodyEntry)
            {
                // multiple in-loop successors -> give up in simple mode
                outlined->eraseFromParent();
                return nullptr;
            }
            bodyEntry = s;
        }
    }
    if (!bodyEntry)
    {
        outlined->eraseFromParent();
        return nullptr;
    }

    // collect blocks reachable from bodyEntry but staying inside loop (simple DFS)
    SmallPtrSet<BasicBlock *, 16> bodySet;
    SmallVector<BasicBlock *, 16> work;
    work.push_back(bodyEntry);
    while (!work.empty())
    {
        BasicBlock *bb = work.pop_back_val();
        if (!bodySet.insert(bb).second)
            continue;
        for (BasicBlock *s : successors(bb))
        {
            if (s == Header)
                continue; // backedge -> don't traverse into header
            if (L->contains(s))
                work.push_back(s);
        }
    }

    // create new blocks in outlined function corresponding to bodySet, keep deterministic order
    SmallVector<BasicBlock *, 16> origBlocks;
    for (BasicBlock *bb : bodySet)
        origBlocks.push_back(bb);
    llvm::sort(origBlocks, [](BasicBlock *A, BasicBlock *B)
               { return A->getName() < B->getName(); });

    // Clone each block into the outlined function.
    SmallVector<BasicBlock *, 16> clonedBlocks;
    clonedBlocks.reserve(origBlocks.size());

    // ---------- Manual clone of body blocks (no PHIs) ----------
    // ---------- Manual clone of body blocks (no PHIs) - FIXED ----------
    clonedBlocks.clear();
    VMap.insert(std::make_pair(&F.getEntryBlock(), entry)); // map original entry -> outlined entry

    // 1) Create empty cloned blocks (same order as origBlocks)
    for (BasicBlock *src : origBlocks)
    {
        BasicBlock *cloned = BasicBlock::Create(ctx, src->getName() + ".out", outlined);
        VMap[src] = cloned;
        clonedBlocks.push_back(cloned);
    }

    // 2) Clone non-PHI instructions into cloned blocks, remapping operands.
    //    Use IRBuilder::Insert to append cloned instructions safely.
    for (size_t i = 0; i < origBlocks.size(); ++i)
    {
        BasicBlock *src = origBlocks[i];
        BasicBlock *dst = cast<BasicBlock>(VMap[src]);

        // builder that inserts at end of dst
        IRBuilder<> insertBuilder(dst);

        for (Instruction &I : *src)
        {
            if (isa<PHINode>(&I))
            {
                // Don't clone PHIs. Ensure there's a mapping for the original PHI
                if (!VMap.lookup(&I))
                {
                    // If we couldn't map it earlier (induction phi / live-in mapping), try to match a live-in.
                    bool mapped = false;
                    for (LiveIn &LI : E.liveIn)
                    {
                        PHINode *p = dyn_cast<PHINode>(&I);
                        if (!p)
                            break;
                        // If any incoming value is the live-in original, map this PHI to the loaded env value.
                        for (unsigned k = 0, n = p->getNumIncomingValues(); k < n; ++k)
                        {
                            if (p->getIncomingValue(k) == LI.original)
                            {
                                Value *mv = VMap.lookup(LI.original);
                                if (!mv)
                                    mv = LI.original;
                                VMap[&I] = mv;
                                mapped = true;
                                break;
                            }
                        }
                        if (mapped)
                            break;
                    }
                    if (!mapped)
                    {
                        // Fallback: undef (should be rare)
                        VMap[&I] = UndefValue::get(I.getType());
                    }
                }
                continue; // skip emitting any PHI inside outlined function
            }

            // Clone instruction and insert into dst's end
            Instruction *NI = I.clone();
            insertBuilder.Insert(NI); // safe insertion, avoids getInstList access
            // Remap operands according to VMap (replace original operands with their mapped clones)
            RemapInstruction(NI, VMap, RF_IgnoreMissingLocals | RF_NoModuleLevelChanges);
            // Record mapping for future remaps
            VMap[&I] = NI;
        }
    }

    // 3) Recreate terminators for cloned blocks so successors are mapped to cloned blocks (or ret)
    for (BasicBlock *src : origBlocks)
    {
        BasicBlock *dst = cast<BasicBlock>(VMap[src]);
        Instruction *origTerm = src->getTerminator();

        // Remove any existing terminator in dst (we appended instructions earlier)
        if (dst->getTerminator())
            dst->getTerminator()->eraseFromParent();

        IRBuilder<> db(dst);

        if (BranchInst *BI = dyn_cast<BranchInst>(origTerm))
        {
            if (BI->isUnconditional())
            {
                BasicBlock *succ = BI->getSuccessor(0);
                auto it = VMap.find(succ);
                if (it != VMap.end())
                {
                    db.CreateBr(cast<BasicBlock>(it->second));
                }
                else
                {
                    db.CreateRetVoid();
                }
            }
            else
            {
                // Conditional branch: remap condition and successors
                Value *cond = BI->getCondition();
                Value *mappedCond = VMap.lookup(cond);
                if (!mappedCond)
                {
                    mappedCond = llvm::MapValue(cond, VMap, llvm::RF_IgnoreMissingLocals | llvm::RF_NoModuleLevelChanges);
                    if (!mappedCond)
                        mappedCond = cond;
                }

                // Ensure cond is an i1 (CreateCondBr requires i1). If it's an integer wider than 1,
                // turn it into an i1 by comparing with zero.
                if (!mappedCond->getType()->isIntegerTy(1))
                {
                    if (mappedCond->getType()->isIntegerTy())
                    {
                        // create (mappedCond != 0)
                        Constant *zero = ConstantInt::get(mappedCond->getType(), 0);
                        mappedCond = db.CreateICmpNE(mappedCond, zero, "cond.to.i1");
                    }
                    else
                    {
                        // Not an integer: try truncation to i1 as a last resort
                        mappedCond = db.CreateTrunc(mappedCond, db.getInt1Ty(), "cond.trunc.i1");
                    }
                }

                BasicBlock *s0 = BI->getSuccessor(0);
                BasicBlock *s1 = BI->getSuccessor(1);
                BasicBlock *m0 = (VMap.count(s0) ? cast<BasicBlock>(VMap[s0]) : nullptr);
                BasicBlock *m1 = (VMap.count(s1) ? cast<BasicBlock>(VMap[s1]) : nullptr);

                if (m0 && m1)
                {
                    db.CreateCondBr(mappedCond, m0, m1);
                }
                else if (m0 && !m1)
                {
                    db.CreateCondBr(mappedCond, m0, exitBB);
                }
                else if (!m0 && m1)
                {
                    db.CreateCondBr(mappedCond, exitBB, m1);
                }
                else
                {
                    db.CreateRetVoid();
                }
            }
        }
        else if (isa<SwitchInst>(origTerm))
        {
            // Conservative: replace switch with return for outlined
            db.CreateRetVoid();
        }
        else
        {
            // Other terminators: replace with ret void
            db.CreateRetVoid();
        }
    }

    // 4) Make outlined entry branch to first cloned block (or return)
    if (!clonedBlocks.empty())
    {
        IRBuilder<> entryBuilder(entry);
        entryBuilder.CreateBr(cast<BasicBlock>(VMap[origBlocks.front()]));
    }
    else
    {
        IRBuilder<> entryBuilder(entry);
        entryBuilder.CreateRetVoid();
    }

    // Set attributes: no unwind, nounwind etc.
    outlined->addFnAttr(Attribute::NoUnwind);
    errs() << "Outlined function IR (debug):\n";
    outlined->print(errs());

    return outlined;
}

// emits: parallel_for_runtime(start,end,step, outlined_fn, env)
// builder should be set with insertion point at the intended call site
static void emitParallelCallForOutlined(llvm::Module *M, llvm::IRBuilder<> &B,
                                        llvm::Function *outlinedFn,
                                        llvm::Value *startV, llvm::Value *endV, llvm::Value *stepV,
                                        llvm::Value *envAsI8)
{
    using namespace llvm;
    assert(M && outlinedFn && startV && endV && stepV);

    LLVMContext &ctx = M->getContext();
    Type *I64Ty = Type::getInt64Ty(ctx);
    Type *VoidTy = Type::getVoidTy(ctx);
    Type *I8PtrTy = PointerType::getUnqual(Type::getInt8Ty(ctx));

    // body function type
    FunctionType *BodyTy = FunctionType::get(VoidTy, {I64Ty, I8PtrTy}, false);
    Type *BodyPtrTy = PointerType::getUnqual(BodyTy);

    // runtime function type: void(i64,i64,i64, void(*)(i64,i8*), i8*)
    FunctionType *ParForTy = FunctionType::get(VoidTy, {I64Ty, I64Ty, I64Ty, BodyPtrTy, I8PtrTy}, false);
    FunctionCallee ParFor = M->getOrInsertFunction("parallel_for_runtime", ParForTy);

    // bitcast outlined to body ptr
    Value *bodyPtr = B.CreateBitCast(outlinedFn, BodyPtrTy, "body.ptr");

    // create call
    B.CreateCall(ParFor, {startV, endV, stepV, bodyPtr, envAsI8});

    // insert free(env) after call
    FunctionCallee FreeF = M->getOrInsertFunction("free", FunctionType::get(Type::getVoidTy(ctx), {I8PtrTy}, false));
    Value *envI8 = envAsI8;
    if (envI8->getType() != I8PtrTy)
        envI8 = B.CreateBitCast(envI8, I8PtrTy);
    B.CreateCall(FreeF, {envI8});
}

struct LoopBoundExtractorPass : public llvm::PassInfoMixin<LoopBoundExtractorPass>
{
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM)
    {
        if (F.isDeclaration())
            return llvm::PreservedAnalyses::all();

        auto &LI = FAM.getResult<llvm::LoopAnalysis>(F);

        bool any = false;
        for (llvm::Loop *L : LI)
        {
            any |= processLoopRecursive(F, FAM, L);
        }

        // We only *read* analyses and insert code in preheaders;
        // conservatively say we modified IR if we succeeded anywhere.
        return any ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
    }

    // --- processLoopRecursive: choose insertion point with fallback ---
    bool processLoopRecursive(llvm::Function &F, llvm::FunctionAnalysisManager &FAM, llvm::Loop *L)
    {
        bool changed = false;

        if (loopParallelizable(L))
        {
            llvm::BasicBlock *Preheader = L->getLoopPreheader();
            llvm::Instruction *IP = nullptr;
            llvm::errs() << "DEBUG: outlineSimpleLoop Instruction: " << IP << "\n";

            if (Preheader)
            {
                // ideal: materialize in the preheader (run once before loop)
                IP = Preheader->getTerminator();
                llvm::errs() << "[bounds] using preheader '" << Preheader->getName()
                             << "' for loop '" << L->getHeader()->getName() << "'\n";
            }
            else
            {
                // fallback: insert at first non-PHI in header (will be executed inside the loop)
                llvm::BasicBlock *Header = L->getHeader();
                if (Header)
                {
                    IP = Header->getFirstNonPHI();
                    if (IP)
                    {
                        llvm::errs() << "[bounds] no preheader for loop '" << Header->getName()
                                     << "', falling back to header insertion at '" << IP->getName() << "'\n";
                    }
                    else
                    {
                        llvm::errs() << "[bounds] no insertion point (no non-PHI) for header '"
                                     << Header->getName() << "', skipping\n";
                    }
                }
            }

            if (!IP)
            {
                llvm::errs() << " [bounds] skip (no insertion point)\n";
            }
            else
            {
                LoopBounds LB = extractLoopBounds(L, F, FAM, IP);
                if (LB.success)
                {
                    changed = true; // we emitted IR to compute bounds
                    llvm::errs() << " [bounds] DOALL loop @ '" << L->getHeader()->getName() << "'\n";
                    llvm::errs() << "   start=";
                    if (LB.constStart)
                        llvm::errs() << LB.startConst;
                    else
                        llvm::errs() << *LB.startV;
                    llvm::errs() << "\n";
                    llvm::errs() << "   step =";
                    if (LB.constStep)
                        llvm::errs() << LB.stepConst;
                    else
                        llvm::errs() << *LB.stepV;
                    llvm::errs() << "\n";
                    llvm::errs() << "   trip =";
                    if (LB.constTrip)
                        llvm::errs() << LB.tripConst;
                    else
                        llvm::errs() << *LB.tripV;
                    llvm::errs() << "\n";
                    llvm::errs() << "   end  =" << *LB.endV << "\n";
                }
                else
                {
                    llvm::errs() << " [bounds] failed: " << LB.msg << "\n";
                }
            }
        }

        for (llvm::Loop *Sub : L->getSubLoops())
            changed |= processLoopRecursive(F, FAM, Sub);

        return changed;
    }
};

struct TrampolineOutliner : public llvm::PassInfoMixin<TrampolineOutliner>
{
    // Run per-function
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM)
    {
        using namespace llvm;

        if (F.isDeclaration())
            return PreservedAnalyses::all();

        // Get analyses we need
        auto &LI = FAM.getResult<LoopAnalysis>(F);
        auto &DG = FAM.getResult<DependencyGraphAnalysis>(F); // you register this earlier
        // ScalarEvolution and AAResults may be used by extract/createEnv
        // Acquire when needed per-loop below via FAM.getResult<...>(F)

        bool madeChange = false;

        Module *M = F.getParent();
        LLVMContext &Ctx = F.getContext();
        Type *I64Ty = Type::getInt64Ty(Ctx);
        Type *I8PtrTy = PointerType::getUnqual(Type::getInt8Ty(Ctx));

        // Ensure runtime prototype exists: void parallel_for_runtime(i64,i64,i64, void(i64,i8*)*, i8*)
        FunctionCallee ParFor = M->getOrInsertFunction(
            "parallel_for_runtime",
            FunctionType::get(Type::getVoidTy(Ctx), {I64Ty, I64Ty, I64Ty, PointerType::getUnqual(FunctionType::get(Type::getVoidTy(Ctx), {I64Ty, I8PtrTy}, false)), I8PtrTy}, false));

        for (Loop *L : LI)
        {
            if (!L || !L->getHeader())
                continue;

            // Check DOALL metadata
            MDNode *LoopID = L->getLoopID();
            if (!LoopID && L->getHeader())
            {
                if (Instruction *T = L->getHeader()->getTerminator())
                {
                    llvm::errs() << "DEBUG: Struct TrampolineOutliner Instruction: " << T << "\n";
                    LoopID = T->getMetadata("llvm.loop");
                }
            }
            bool isDOALL = false;
            if (LoopID)
            {
                for (unsigned i = 0, e = LoopID->getNumOperands(); i < e; ++i)
                {
                    Metadata *Op = LoopID->getOperand(i);
                    llvm::errs() << "DEBUG: Struct TrampolineOutliner Operand retrieved" << Op << "\n";
                    if (!Op)
                        continue;
                    if (auto *MDN = dyn_cast<MDNode>(Op))
                    {
                        if (MDN->getNumOperands() >= 1)
                        {
                            if (auto *S = dyn_cast_or_null<MDString>(MDN->getOperand(0)))
                            {
                                llvm::errs() << "DEBUG: Struct TrampolineOutliner Operand retrieved" << "\n";
                                if (S->getString() == "DOALL")
                                {
                                    isDOALL = true;
                                    break;
                                }
                            }
                        }
                    }
                    else if (auto *S = dyn_cast<MDString>(Op))
                    {
                        if (S->getString() == "DOALL")
                        {
                            isDOALL = true;
                            break;
                        }
                    }
                }
            }
            if (!isDOALL)
                continue;

            // Choose insertion point: preheader preferred, otherwise header first non-PHI.
            BasicBlock *Pre = L->getLoopPreheader();
            Instruction *InsertPt = nullptr;
            if (Pre)
            {
                InsertPt = Pre->getTerminator();
            }
            else
            {
                BasicBlock *H = L->getHeader();
                if (H)
                    InsertPt = H->getFirstNonPHI();
            }
            if (!InsertPt)
            {
                errs() << "[trampoline] no insertion point for loop @" << L->getHeader()->getName() << "\n";
                continue;
            }

            // Get AAResults needed by createEnv/escape analysis
            AAResults &AA = FAM.getResult<AAManager>(F);
            // Acquire dependency graph for env collection (you registered it earlier)
            DependencyGraph &depGraph = FAM.getResult<DependencyGraphAnalysis>(F);

            // 1) extract bounds
            LoopBounds LB = extractLoopBounds(L, F, FAM, InsertPt);
            if (!LB.success)
            {
                errs() << "[trampoline] failed to extract bounds for loop @" << L->getHeader()->getName() << " : " << LB.msg << "\n";
                continue;
            }

            // 2) build environment (uses your createEnv)
            Env E = createEnv(L, F, FAM, AA, depGraph, InsertPt);
            if (!E.success)
            {
                errs() << "[trampoline] createEnv failed for loop @" << L->getHeader()->getName() << " : " << E.msg << "\n";
                continue;
            }

            // 3) outline loop body
            Function *outlined = outlineSimpleLoop(L, F, E, InsertPt);
            if (!outlined)
            {
                errs() << "[trampoline] outlineSimpleLoop failed for loop @" << L->getHeader()->getName() << "\n";
                continue;
            }

            // Make outlined function pointer type (void(i64,i8*))
            FunctionType *BodyFT = FunctionType::get(Type::getVoidTy(Ctx), {I64Ty, I8PtrTy}, false);
            PointerType *BodyPtrTy = PointerType::getUnqual(BodyFT);

            // 4) Emit calls at InsertPt: start, end, step, funcptr, env
            IRBuilder<> B(InsertPt);

            Value *startV = LB.startV;
            Value *stepV = LB.stepV;
            Value *endV = LB.endV;

            // Ensure values are i64
            if (startV->getType() != I64Ty)
                startV = B.CreateSExtOrTrunc(startV, I64Ty, "start_i64");
            if (stepV->getType() != I64Ty)
                stepV = B.CreateSExtOrTrunc(stepV, I64Ty, "step_i64");
            if (endV->getType() != I64Ty)
                endV = B.CreateSExtOrTrunc(endV, I64Ty, "end_i64");

            // Convert outlined function to void(i64,i8*)* type
            Constant *FnPtr = ConstantExpr::getBitCast(outlined, BodyPtrTy);

            // env pointer: pass E.envAsI8 (already i8*)
            Value *envArg = E.envAsI8;
            if (!envArg)
                envArg = ConstantPointerNull::get(cast<PointerType>(I8PtrTy));

            // Create call: parallel_for_runtime(start,end,step, fnptr, env)
            CallInst *parCall = B.CreateCall(ParFor, {startV, endV, stepV, FnPtr, envArg});

            // Only name the call if it returns a non-void value
            if (!parCall->getType()->isVoidTy())
            {
                parCall->setName("parfor.call");
            }
            // Mark that a change happened in the function
            madeChange = true;

            errs() << "[trampoline] emitted parallel runtime call for loop @" << L->getHeader()->getName() << "\n";
        }

        return madeChange ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }
};

// -------- Plugin registration ----------
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION,
        "CombinedPluginWithTrampoline",
        LLVM_VERSION_STRING,
        [](llvm::PassBuilder &PB)
        {
            // Register analyses (you already register DependencyGraphAnalysis & LoopClassifier earlier in your file).
            PB.registerAnalysisRegistrationCallback(
                [](llvm::FunctionAnalysisManager &FAM)
                {
                    // Register AAManager and standard analyses required by this pass
                    FAM.registerPass([]
                                     { return llvm::AAManager(); });
                    FAM.registerPass([]
                                     { return llvm::ScalarEvolutionAnalysis(); });
                    FAM.registerPass([]
                                     { return llvm::LoopAnalysis(); });

                    // register AA/ScalarEvolution etc are standard; PassBuilder wires them automatically
                });

            // Register function-level passes parsing callbacks
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>)
                {
                    if (Name == "trampoline-outliner")
                    {
                        FPM.addPass(TrampolineOutliner());
                        return true;
                    }

                    // Keep existing names registered elsewhere in your file:
                    // - "loop-bound-extractor" should already be registered by your LoopBoundExtractor code.
                    // If not, add its registration here:
                    if (Name == "loop-bound-extractor")
                    {
                        FPM.addPass(LoopBoundExtractorPass()); // your existing pass class
                        return true;
                    }

                    return false;
                });
        }};
}