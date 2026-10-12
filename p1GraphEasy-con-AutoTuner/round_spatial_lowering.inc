/* Runtime composition only. Consume the immutable original proof results.
 * Original access/contract identities stay in Info. No interference analysis
 * runs on these callbacks. */
static bool emitRoundSpatial(NeighborLoopInfo &Info, const RoundDomain &R) {
    auto &Spatial = Info.SpatialResult;
    if (Spatial.Status != SpatialProofStatus::IndependentFamily || !Spatial.RuntimeRealizable)
        return false;
    Function *Original = Info.DriverLoop->getHeader()->getParent();
    Module *M = Original->getParent();
    LLVMContext &C = M->getContext();
    Type *Ptr = PointerType::get(C, 0), *I32 = Type::getInt32Ty(C), *I64 = Type::getInt64Ty(C);
    BasicBlock *SourceHeader = Info.DriverLoop->getHeader();
    BasicBlock *SourceExit = Info.DriverLoop->getUniqueExitBlock();
    auto Refuse = [&](StringRef Why) {
        Info.RoundSpatialRuntimeReason = Why.str();
        if (getenv("GRAPH_FRONTIER_STATS") || getenv("SGPL_WITNESS_DUMP"))
            errs() << "[round-spatial-runtime] unavailable: " << Why << "\n";
        return false;
    };
    if (!SourceExit || !temporalGraphAvailable(Info, R.Body))
        return Refuse("round traversal has no supported continuation");
    SmallPtrSet<BasicBlock *, 32> Prefix, Suffix;
    auto Collect = [&](BasicBlock *Start, SmallPtrSetImpl<BasicBlock *> &Set, bool PrefixPart) {
        SmallVector<BasicBlock *, 32> Work{Start};
        while (!Work.empty()) {
            BasicBlock *B = Work.pop_back_val();
            if (B == R.Body.Unit->getHeader()) { if (PrefixPart) return false; else continue; }
            if (PrefixPart && (B == SourceHeader || B == SourceExit)) continue;
            if (!R.Body.Unit->contains(B) || Info.DriverLoop->contains(B)) return false;
            if (!Set.insert(B).second) continue;
            for (BasicBlock *S : successors(B)) Work.push_back(S);
        }
        return true;
    };
    if (!Collect(R.Body.Entry, Prefix, true) || !Collect(SourceExit, Suffix, false))
        return Refuse("round setup or finalization crosses the traversal boundary");
    for (BasicBlock *B : Prefix) if (Suffix.count(B)) return Refuse("round continuations overlap");

    SmallPtrSet<Function *, 32> Existing;
    for (Function &F : *M) Existing.insert(&F);
    auto Cleanup = [&]() {
        SmallVector<Function *, 16> Added;
        for (Function &F : *M) if (!Existing.count(&F) && !F.isDeclaration()) Added.push_back(&F);
        for (Function *F : Added) F->dropAllReferences();
        for (Function *F : Added) F->eraseFromParent();
    };
    DenseMap<const Value *, unsigned> Fields;
    SmallPtrSet<const Value *, 16> Private, IndirectPrivate;
    SmallVector<AllocaInst *, 8> DynamicAllocations;
    SmallVector<Type *, 32> Types;
    SmallVector<Value *, 16> Captures;
    auto Field = [&](const Value *V, Type *Ty) {
        if (Fields.count(V)) return;
        Fields[V] = Types.size(); Types.push_back(Ty);
    };
    for (const Value *V : R.Private) {
        Type *Ty = isa<GlobalVariable>(V) ? cast<GlobalVariable>(V)->getValueType()
            : cast<AllocaInst>(V)->getAllocatedType();
        if (auto *A = dyn_cast<AllocaInst>(V)) {
            auto *Count = dyn_cast<ConstantInt>(A->getArraySize());
            if (!Count) { IndirectPrivate.insert(A); DynamicAllocations.push_back(const_cast<AllocaInst *>(A)); Ty = Ptr; }
            else if (!Count->isOne()) Ty = ArrayType::get(Ty, Count->getZExtValue());
        }
        Private.insert(V); Field(V, Ty);
    }
    for (BasicBlock *B : R.Body.Blocks) for (Instruction &I : *B)
        if (auto *A = dyn_cast<AllocaInst>(&I)) {
            auto *Count = dyn_cast<ConstantInt>(A->getArraySize());
            if (Fields.count(A)) continue;
            Type *Ty = A->getAllocatedType();
            if (!Count) { IndirectPrivate.insert(A); DynamicAllocations.push_back(A); Ty = Ptr; }
            else if (!Count->isOne()) Ty = ArrayType::get(Ty, Count->getZExtValue());
            Private.insert(A); Field(A, Ty);
        }
    if (R.Phi) Field(R.Phi, I32);
    for (PHINode &P : SourceExit->phis()) Field(&P, P.getType());
    for (BasicBlock *B : Prefix) for (Instruction &I : *B)
        if (!I.getType()->isVoidTy() && !Private.count(&I) && !I.isTerminator()) Field(&I, I.getType());
    DominatorTree OriginalDT(*Original);
    for (BasicBlock *B : R.Body.Blocks) for (Instruction &I : *B) for (Value *V : I.operands()) {
        if (Private.count(V) || V == R.Phi || isa<Constant>(V) || isa<BasicBlock>(V)) continue;
        auto *IV = dyn_cast<Instruction>(V);
        if (IV && R.Body.Unit->contains(IV)) continue;
        if (!IV && !isa<Argument>(V)) continue;
        if (IV && !OriginalDT.dominates(IV, R.Body.Unit->getLoopPreheader()->getTerminator()))
            return Refuse("round capture is unavailable at dispatch");
        if (!Fields.count(V)) { Captures.push_back(V); Field(V, V->getType()); }
    }
    unsigned ContextField = Types.size(); Types.push_back(Ptr);
    unsigned ActiveField = Types.size(); Types.push_back(I32);
    // Reserve emission can append fields for its allocations before layout closes.
    StructType *Frame = StructType::create(C, "sgpl.round.spatial.frame");
    Frame->setBody(Types);
    auto WorkTy = FunctionType::get(I32, {Ptr, I64}, false);
    auto ReserveTy = FunctionType::get(I32, {Ptr, Ptr, I64}, false);
    Function *Reserve = Function::Create(ReserveTy, GlobalValue::InternalLinkage, "sgpl.round.spatial.reserve", M);
    BasicBlock *ReserveEntry = BasicBlock::Create(C, "entry", Reserve);
    IRBuilder<> RB(ReserveEntry); RB.CreateRet(RB.getInt32(0));
    SpatialStageParts Parts; Parts.Reserve = ReserveEntry;
    NeighborLoopInfo EmitInfo = Info;
    EmitInfo.CombinedStage = &Parts; EmitInfo.CombinedFields = &Fields;
    EmitInfo.CombinedPrivate = &Private; EmitInfo.CombinedIndirectPrivate = &IndirectPrivate; EmitInfo.CombinedFrame = Frame;
    EmitInfo.Schedule.Kind = ScheduleKind::SpatialDag;
    ExprFacts Facts; deriveExprFacts(Info, Facts);
    if (!emitSingleStage(EmitInfo, Facts.IsRed, Facts.IsSourceRed, Facts.IsPriv,
                         Spatial.CleanCutGroup == SpatialGroup::Destination)) {
        Cleanup(); return Refuse("spatial stage cannot expose its lifecycle");
    }
    SmallVector<AllocaInst *, 16> Allocations;
    for (BasicBlock &B : *Reserve) for (Instruction &I : B)
        if (auto *A = dyn_cast<AllocaInst>(&I)) Allocations.push_back(A);
    SmallVector<CallInst *, 8> Descriptors;
    for (BasicBlock &B : *Reserve) for (Instruction &I : B) if (auto *Call = dyn_cast<CallInst>(&I))
        if (Call->getCalledFunction() && Call->getCalledFunction()->getName() == "autograph_exec_op_create") Descriptors.push_back(Call);
    DenseMap<const Value *, unsigned> ReserveFields;
    for (AllocaInst *A : Allocations) { ReserveFields[A] = Types.size(); Types.push_back(Ptr); }
    for (CallInst *D : Descriptors) { ReserveFields[D] = Types.size(); Types.push_back(Ptr); }
    unsigned PrivateField = Types.size(); Types.push_back(Ptr);
    unsigned PrivateCountField = Types.size(); Types.push_back(I32);
    for (const auto &S : Parts.Snapshots) {
        ReserveFields[std::get<1>(S)] = Types.size(); Types.push_back(Ptr);
        ReserveFields[std::get<2>(S)] = Types.size(); Types.push_back(I64);
    }
    Frame->setBody(Types);
    StructType *Shared = StructType::create(C, "sgpl.round.spatial.captures");
    SmallVector<Type *, 16> CaptureTypes;
    for (Value *V : Captures) CaptureTypes.push_back(V->getType());
    Shared->setBody(CaptureTypes);
    auto Address = [&](IRBuilder<> &B, Value *State, unsigned F) {
        return B.CreateStructGEP(Frame, State, F);
    };
    auto Resolve = [&](IRBuilder<> &B, Value *State, const Value *V) -> Value * {
        auto It = Fields.find(V);
        if (It == Fields.end()) return nullptr;
        Value *A = Address(B, State, It->second);
        return (Private.count(V) && !IndirectPrivate.count(V)) ? A : B.CreateLoad(Frame->getElementType(It->second), A);
    };
    // Reserve pure setup expressions from the round index and immutable captures.
    // This is value mapping for emission, not a dependency proof.
    Instruction *ReserveAnchor = &*ReserveEntry->getFirstInsertionPt();
    RB.SetInsertPoint(ReserveAnchor);
    for (unsigned N = 0; N < Captures.size(); ++N) {
        Value *V = RB.CreateLoad(CaptureTypes[N], RB.CreateStructGEP(Shared, Reserve->getArg(0), N));
        RB.CreateStore(V, Address(RB, Reserve->getArg(1), Fields[Captures[N]]));
    }
    if (R.Counter) RB.CreateStore(RB.CreateTrunc(Reserve->getArg(2), I32), Address(RB, Reserve->getArg(1), Fields[R.Counter]));
    if (R.Phi) RB.CreateStore(RB.CreateTrunc(Reserve->getArg(2), I32), Address(RB, Reserve->getArg(1), Fields[R.Phi]));
    DenseMap<const Value *, Value *> ReservedValues;
    SmallPtrSet<const Value *, 16> Visiting;
    bool Valid = true;
    DenseMap<BasicBlock *, Value *> ReservedReach;
    SmallPtrSet<BasicBlock *, 16> VisitingReach;
    std::function<Value *(const Value *)> ReserveValue;
    std::function<Value *(BasicBlock *, BasicBlock *)> ReserveEdge;
    std::function<Value *(BasicBlock *)> ReserveReach = [&](BasicBlock *Block) -> Value * {
        if (Block == R.Body.Entry) return RB.getTrue();
        if (!Prefix.count(Block) || !VisitingReach.insert(Block).second) return nullptr;
        if (ReservedReach.count(Block)) { VisitingReach.erase(Block); return ReservedReach[Block]; }
        Value *Reach = RB.getFalse();
        for (BasicBlock *Pred : predecessors(Block)) {
            Value *Edge = ReserveEdge(Pred, Block);
            if (!Edge) { VisitingReach.erase(Block); return nullptr; }
            Reach = RB.CreateOr(Reach, Edge);
        }
        VisitingReach.erase(Block); return ReservedReach[Block] = Reach;
    };
    ReserveEdge = [&](BasicBlock *Pred, BasicBlock *Block) -> Value * {
        Value *Parent = ReserveReach(Pred);
        if (!Parent) return nullptr;
        Value *Edge = RB.getTrue();
        if (auto *Branch = dyn_cast<BranchInst>(Pred->getTerminator())) {
            if (Branch->isConditional() && Branch->getSuccessor(0) != Branch->getSuccessor(1)) {
                Value *Condition = ReserveValue(Branch->getCondition());
                if (!Condition) return nullptr;
                Edge = Branch->getSuccessor(0) == Block ? Condition : RB.CreateNot(Condition);
            }
        } else return nullptr;
        return RB.CreateSelect(Parent, Edge, RB.getFalse());
    };
    ReserveValue = [&](const Value *V) -> Value * {
        if (ReservedValues.count(V)) return ReservedValues[V];
        if (Private.count(V) || V == R.Phi || llvm::is_contained(Captures, V))
            return Resolve(RB, Reserve->getArg(1), V);
        if (isa<Constant>(V)) return const_cast<Value *>(V);
        const auto *I = dyn_cast<Instruction>(V);
        if (!I || !Visiting.insert(V).second) return nullptr;
        Value *Result = nullptr;
        if (const auto *P = dyn_cast<PHINode>(I)) {
            if (Prefix.count(P->getParent())) {
                Result = Constant::getNullValue(P->getType());
                for (unsigned N = 0; N < P->getNumIncomingValues(); ++N) {
                    Value *Reach = ReserveEdge(P->getIncomingBlock(N), const_cast<BasicBlock *>(P->getParent()));
                    Value *Incoming = ReserveValue(P->getIncomingValue(N));
                    if (!Reach || !Incoming) { Result = nullptr; break; }
                    Result = RB.CreateSelect(Reach, Incoming, Result);
                }
            }
        } else if (const auto *L = dyn_cast<LoadInst>(I)) {
            if (L->getPointerOperand() == R.Counter) {
                Value *Index = Reserve->getArg(2);
                if (R.Increment && OriginalDT.dominates(R.Increment, L)) Index = RB.CreateAdd(Index, RB.getInt64(1));
                if (!R.Increment || OriginalDT.dominates(R.Increment, L) || OriginalDT.dominates(L, R.Increment))
                    Result = RB.CreateTrunc(Index, L->getType());
            }
            else if (!L->isVolatile() && !L->isAtomic() && Private.count(L->getPointerOperand())) {
                const StoreInst *Seed = nullptr;
                for (const User *U : L->getPointerOperand()->users())
                    if (const auto *S = dyn_cast<StoreInst>(U))
                        if (S->getPointerOperand() == L->getPointerOperand() && OriginalDT.dominates(S, L))
                            if (!Seed || OriginalDT.dominates(Seed, S)) Seed = S;
                bool Unambiguous = Seed != nullptr;
                for (const User *U : L->getPointerOperand()->users()) {
                    if (!Seed) break;
                    if (const auto *S = dyn_cast<StoreInst>(U)) {
                        if (S->getPointerOperand() != L->getPointerOperand() ||
                            (S != Seed && !OriginalDT.dominates(S, Seed) && !OriginalDT.dominates(L, S))) Unambiguous = false;
                    } else if (!isa<LoadInst>(U)) {
                        const auto *I = dyn_cast<Instruction>(U);
                        if (!I || !temporalInstrumentation(*I)) Unambiguous = false;
                    }
                }
                if (Unambiguous) Result = ReserveValue(Seed->getValueOperand());
            }
        } else if (!I->mayReadOrWriteMemory() && !I->mayHaveSideEffects() && !I->isTerminator() &&
                   !isa<PHINode>(I) && !isa<AllocaInst>(I) && isSafeToSpeculativelyExecute(I)) {
            SmallVector<Value *, 8> Operands;
            for (const Value *O : I->operands()) {
                Value *Mapped = ReserveValue(O);
                if (!Mapped) { Operands.clear(); break; }
                Operands.push_back(Mapped);
            }
            if (Operands.size() == I->getNumOperands()) {
                Instruction *Copy = I->clone();
                for (unsigned N = 0; N < Operands.size(); ++N) Copy->setOperand(N, Operands[N]);
                RB.Insert(Copy); Result = Copy;
            }
        }
        Visiting.erase(V);
        if (Result) ReservedValues[V] = Result;
        return Result;
    };
    SmallVector<Value *, 8> DynamicStatuses;
    for (AllocaInst *A : DynamicAllocations) {
        Value *Count = ReserveValue(A->getArraySize());
        if (!Count) { Cleanup(); return Refuse("allocation extent requires executing user effects during reservation"); }
        Value *Enabled = RB.getTrue();
        for (BasicBlock *Block : Prefix) if (auto *Branch = dyn_cast<BranchInst>(Block->getTerminator()))
            if (Branch->isConditional() && OriginalDT.dominates(Block, A->getParent())) {
                bool TruePath = OriginalDT.dominates(Branch->getSuccessor(0), A->getParent());
                bool FalsePath = OriginalDT.dominates(Branch->getSuccessor(1), A->getParent());
                if (TruePath != FalsePath) {
                    Value *Condition = ReserveValue(Branch->getCondition());
                    if (!Condition) { Cleanup(); return Refuse("conditional allocation requires executing setup effects during reservation"); }
                    Enabled = RB.CreateAnd(Enabled, TruePath ? Condition : RB.CreateNot(Condition));
                }
            }
        Count = RB.CreateSelect(Enabled, Count, ConstantInt::get(Count->getType(), 0));
        Value *Memory = RB.CreateCall(M->getOrInsertFunction("sgpl_round_spatial_alloc",
            FunctionType::get(Ptr, {I64, I64}, false)), {RB.CreateZExtOrTrunc(Count, I64),
                RB.getInt64(M->getDataLayout().getTypeAllocSize(A->getAllocatedType()))});
        RB.CreateStore(Memory, Address(RB, Reserve->getArg(1), Fields[A]));
        DynamicStatuses.push_back(RB.CreateSelect(RB.CreateICmpEQ(Memory,
            ConstantPointerNull::get(cast<PointerType>(Ptr))), RB.getInt32(-4), RB.getInt32(0)));
    }
    // Helpers get the task's frame, never a module-global publication.
    SmallVector<Function *, 16> Helpers;
    for (Function &F : *M) if (!Existing.count(&F) && !F.isDeclaration()) Helpers.push_back(&F);
    for (Function *F : Helpers) {
        SmallVector<std::pair<Instruction *, unsigned>, 32> Uses;
        for (BasicBlock &B : *F) for (Instruction &I : B) for (unsigned O = 0; O < I.getNumOperands(); ++O) {
            Value *V = I.getOperand(O);
            if (Fields.count(V) && (Private.count(V) || isa<Instruction>(V) || isa<Argument>(V))) Uses.push_back({&I, O});
        }
        IRBuilder<> EB(F == Reserve ? ReserveAnchor : &*F->getEntryBlock().getFirstInsertionPt());
        Value *State = F == Reserve ? static_cast<Value *>(Reserve->getArg(1)) : static_cast<Value *>(EB.CreateCall(M->getOrInsertFunction(
            "sgpl_round_spatial_current_frame", FunctionType::get(Ptr, false))));
        for (auto U : Uses) {
            const Value *V = U.first->getOperand(U.second);
            Value *Replacement = nullptr;
            if (F == Reserve) {
                RB.SetInsertPoint(ReserveAnchor);
                Replacement = ReserveValue(V);
            } else Replacement = Resolve(EB, State, V);
            if (!Replacement) { Valid = false; continue; }
            U.first->setOperand(U.second, Replacement);
        }
        F->addFnAttr("sgpl.spatial.callback");
        F->addFnAttr("sgpl.temporal.scope", "Round");
    }
    if (!Valid) { Cleanup(); return Refuse("round setup value cannot be reserved without user effects"); }
    auto Check = [&](Value *Status) {
        auto *I = cast<Instruction>(Status);
        BasicBlock *B = I->getParent();
        BasicBlock *Continue = B->splitBasicBlock(std::next(I->getIterator()), "reserved");
        B->getTerminator()->eraseFromParent();
        BasicBlock *Fail = BasicBlock::Create(C, "reserve.failed", Reserve);
        IRBuilder<>(Fail).CreateRet(Status);
        IRBuilder<> CB(B); CB.CreateCondBr(CB.CreateICmpSGE(Status, CB.getInt32(0)), Continue, Fail);
    };
    for (Value *Status : DynamicStatuses) Check(Status);
    for (AllocaInst *A : Allocations) {
        IRBuilder<> B(A);
        Value *Count = B.CreateZExtOrTrunc(A->getArraySize(), I64);
        Value *Bytes = B.getInt64(M->getDataLayout().getTypeAllocSize(A->getAllocatedType()));
        // Checked allocator accepts count and stride, avoiding multiplication overflow.
        Value *Memory = B.CreateCall(M->getOrInsertFunction("sgpl_round_spatial_alloc",
            FunctionType::get(Ptr, {I64, I64}, false)), {Count, Bytes});
        B.CreateStore(Memory, Address(B, Reserve->getArg(1), ReserveFields[A]));
        A->replaceAllUsesWith(Memory);
        if (Parts.PrivateRecords == A) {
            Parts.PrivateRecords = Memory;
            B.CreateStore(Memory, Address(B, Reserve->getArg(1), PrivateField));
            B.CreateStore(Parts.PrivateParts, Address(B, Reserve->getArg(1), PrivateCountField));
        }
        Value *Status = B.CreateSelect(B.CreateICmpEQ(Memory, ConstantPointerNull::get(cast<PointerType>(Ptr))), B.getInt32(-4), B.getInt32(0));
        A->eraseFromParent();
        Check(Status);
    }
    for (CallInst *D : Descriptors) {
        IRBuilder<> Save(D->getNextNode());
        Save.CreateStore(D, Address(Save, Reserve->getArg(1), ReserveFields[D]));
    }
    SmallVector<CallInst *, 16> Requires;
    for (BasicBlock &B : *Reserve) for (Instruction &I : B) if (auto *Call = dyn_cast<CallInst>(&I))
        if (Call->getCalledFunction() && Call->getCalledFunction()->getName() == "sgpl_exec_require_status") Requires.push_back(Call);
    for (CallInst *Call : Requires) { Value *Status = Call->getArgOperand(0); Call->eraseFromParent(); Check(Status); }
    for (BasicBlock &B : *Reserve) if (isa<ReturnInst>(B.getTerminator()))
        if (cast<ReturnInst>(B.getTerminator())->getReturnValue() == ConstantInt::get(I32, 0)) {
            IRBuilder<> E(B.getTerminator());
            E.CreateStore(Parts.Context, Address(E, Reserve->getArg(1), ContextField));
            if (Parts.PrivateRecords) {
                E.CreateStore(Parts.PrivateRecords, Address(E, Reserve->getArg(1), PrivateField));
                E.CreateStore(Parts.PrivateParts, Address(E, Reserve->getArg(1), PrivateCountField));
            }
            for (auto S : Parts.Snapshots) {
                Value *Base = Fields.count(std::get<1>(S)) ? Resolve(E, Reserve->getArg(1), std::get<1>(S)) : std::get<1>(S);
                E.CreateStore(Base, Address(E, Reserve->getArg(1), ReserveFields[std::get<1>(S)]));
                E.CreateStore(std::get<2>(S), Address(E, Reserve->getArg(1), ReserveFields[std::get<2>(S)]));
            }
        }
    // Save the context as soon as it exists, including partial-reservation failure.
    IRBuilder<> ContextStore(cast<Instruction>(Parts.Context)->getNextNode());
    ContextStore.CreateStore(Parts.Context, Address(ContextStore, Reserve->getArg(1), ContextField));
    auto MakeContinuation = [&](StringRef Name, const SmallPtrSetImpl<BasicBlock *> &Blocks, BasicBlock *Start, bool Setup) -> Function * {
        Function *F = Function::Create(WorkTy, GlobalValue::InternalLinkage, Name, M);
        F->addFnAttr("sgpl.temporal.callback"); F->addFnAttr("sgpl.temporal.scope", "Round");
        BasicBlock *Entry = BasicBlock::Create(C, "frame", F);
        IRBuilder<> EB(Entry); ValueToValueMapTy Map;
        for (auto Field : Fields)
            if (!(isa<PHINode>(Field.first) && cast<PHINode>(Field.first)->getParent() == SourceExit))
                Map[Field.first] = Resolve(EB, F->getArg(0), Field.first);
        for (BasicBlock *B : R.Body.Blocks) if (Blocks.count(B)) Map[B] = CloneBasicBlock(B, Map, ".round", F);
        BasicBlock *Done = BasicBlock::Create(C, "continuation", F);
        BasicBlock *Skip = Setup ? BasicBlock::Create(C, "skip.traversal", F) : Done;
        EB.CreateBr(cast<BasicBlock>(Map[Start]));
        for (BasicBlock *B : R.Body.Blocks) if (Blocks.count(B)) {
            BasicBlock *Copy = cast<BasicBlock>(Map[B]);
            for (Instruction &I : *Copy) RemapInstruction(&I, Map, RF_IgnoreMissingLocals);
            if (Setup && B == Start) for (PHINode &P : Copy->phis())
                for (unsigned N = 0; N < P.getNumIncomingValues(); ++N)
                    if (P.getIncomingBlock(N) == R.Body.Unit->getHeader()) P.setIncomingBlock(N, Entry);
            auto *T = Copy->getTerminator();
            for (unsigned S = 0; S < T->getNumSuccessors(); ++S) {
                BasicBlock *Target = T->getSuccessor(S);
                if (Target == SourceHeader) T->setSuccessor(S, Done);
                else if (Target == SourceExit && Setup) {
                    IRBuilder<> PB(T);
                    for (PHINode &P : SourceExit->phis()) {
                        Value *OriginalValue = P.getIncomingValueForBlock(B);
                        Value *Mapped = Map.lookup(OriginalValue);
                        PB.CreateStore(Mapped ? Mapped : OriginalValue, Address(PB, F->getArg(0), Fields[&P]));
                    }
                    T->setSuccessor(S, Skip);
                }
                else if (Target == R.Body.Unit->getHeader()) T->setSuccessor(S, Done);
            }
            SmallVector<Instruction *, 8> Remove;
            for (Instruction &I : *B) {
                auto *CI = dyn_cast_or_null<Instruction>(Map.lookup(&I));
                if (!CI) continue;
                if (isa<PHINode>(&I) && B == SourceExit) {
                    IRBuilder<> PB(Copy, Copy->getFirstInsertionPt());
                    CI->replaceAllUsesWith(Resolve(PB, F->getArg(0), &I)); Remove.push_back(CI);
                } else if (isa<AllocaInst>(&I) && Private.count(&I)) {
                    IRBuilder<> AB(CI); CI->replaceAllUsesWith(Resolve(AB, F->getArg(0), &I)); Remove.push_back(CI);
                } else if (temporalInstrumentation(I)) Remove.push_back(CI);
                else if (Setup && Fields.count(&I) && !Private.count(&I) && !I.getType()->isVoidTy()) {
                    IRBuilder<> SB(isa<PHINode>(CI) ? &*Copy->getFirstInsertionPt() : CI->getNextNode());
                    SB.CreateStore(CI, Address(SB, F->getArg(0), Fields[&I]));
                }
            }
            for (Instruction *I : Remove) I->eraseFromParent();
        }
        if (Setup) {
            IRBuilder<> SB(Skip); SB.CreateStore(SB.getInt32(0), Address(SB, F->getArg(0), ActiveField)); SB.CreateRet(SB.getInt32(0));
            IRBuilder<> DB(Done); DB.CreateStore(DB.getInt32(1), Address(DB, F->getArg(0), ActiveField));
            Value *Context = DB.CreateLoad(Ptr, Address(DB, F->getArg(0), ContextField));
            auto FrameOperand = [&](Value *V) -> Value * {
                if (!V) return ConstantPointerNull::get(cast<PointerType>(Ptr));
                return Fields.count(V) ? Resolve(DB, F->getArg(0), V) : V;
            };
            auto FrameArray = [&](Value *V) -> Value * {
                Value *Storage = FrameOperand(V);
                if (auto *A = dyn_cast<AllocaInst>(V)) if (!A->getAllocatedType()->isPointerTy()) return Storage;
                if (auto *G = dyn_cast<GlobalVariable>(V)) if (G->getValueType()->isArrayTy()) return Storage;
                return DB.CreateLoad(Ptr, Storage);
            };
            auto SetupCheck = [&](Value *Status) {
                BasicBlock *Next = BasicBlock::Create(C, "frontier.ready", F), *Fail = BasicBlock::Create(C, "frontier.failed", F);
                DB.CreateCondBr(DB.CreateICmpSGE(Status, DB.getInt32(0)), Next, Fail);
                IRBuilder<>(Fail).CreateRet(Status); DB.SetInsertPoint(Next);
            };
            if (Info.MembershipGated || Info.HasFrontierAppend) {
                if (Info.FrontierSetPtr) {
                    Value *Bitmap = DB.CreateLoad(Ptr, FrameOperand(Info.FrontierSetPtr));
                    SetupCheck(DB.CreateCall(M->getOrInsertFunction("autograph_exec_ctx_materialize_frontier_bitmap",
                        FunctionType::get(I32, {Ptr, Ptr}, false)), {Context, Bitmap}));
                } else {
                    Value *Array = Info.FrontierArrayPtr ? FrameArray(Info.FrontierArrayPtr)
                        : ConstantPointerNull::get(cast<PointerType>(Ptr));
                    Value *Count = Info.FrontierSizePtr ? DB.CreateLoad(I32, FrameOperand(Info.FrontierSizePtr))
                        : Info.FrontierSizeVal ? FrameOperand(Info.FrontierSizeVal) : DB.getInt32(0);
                    SetupCheck(DB.CreateCall(M->getOrInsertFunction("autograph_exec_ctx_materialize_frontier_array",
                        FunctionType::get(I32, {Ptr, Ptr, I32}, false)), {Context, Array, Count}));
                }
            }
            for (auto S : Parts.Snapshots) {
                Value *Base = DB.CreateLoad(Ptr, Address(DB, F->getArg(0), ReserveFields[std::get<1>(S)]));
                Value *Count = DB.CreateLoad(I64, Address(DB, F->getArg(0), ReserveFields[std::get<2>(S)]));
                auto Fn = M->getOrInsertFunction("autograph_exec_ctx_materialize_snapshot",
                    FunctionType::get(I32, {Ptr, I32, Ptr, I64}, false));
                Value *SnapshotStatus = DB.CreateCall(Fn, {Context, DB.getInt32(std::get<0>(S)), Base, DB.CreateMul(Count, DB.getInt64(std::get<3>(S)))});
                BasicBlock *Next = BasicBlock::Create(C, "snapshot.ready", F), *SnapshotFail = BasicBlock::Create(C, "snapshot.failed", F);
                DB.CreateCondBr(DB.CreateICmpSGE(SnapshotStatus, DB.getInt32(0)), Next, SnapshotFail);
                IRBuilder<>(SnapshotFail).CreateRet(SnapshotStatus); DB.SetInsertPoint(Next);
            }
            auto Begin = M->getOrInsertFunction("sgpl_exec_spatial_begin", FunctionType::get(I32, {Ptr}, false));
            auto Publish = M->getOrInsertFunction("sgpl_exec_spatial_publish", FunctionType::get(I32, {Ptr}, false));
            Value *Status = DB.CreateCall(Begin, {Context});
            BasicBlock *Publication = BasicBlock::Create(C, "publish", F), *Fail = BasicBlock::Create(C, "failed", F);
            DB.CreateCondBr(DB.CreateICmpEQ(Status, DB.getInt32(0)), Publication, Fail);
            IRBuilder<>(Fail).CreateRet(Status); IRBuilder<> PB(Publication); PB.CreateRet(PB.CreateCall(Publish, {Context}));
        } else IRBuilder<>(Done).CreateRet(ConstantInt::get(I32, 0));
        removeUnreachableBlocks(*F);
        return F;
    };
    Function *Setup = MakeContinuation("sgpl.round.spatial.setup", Prefix, R.Body.Entry, true);
    Function *Finish = MakeContinuation("sgpl.round.spatial.finalize", Suffix, SourceExit, false);
    // Complete the spatial lifecycle before the original suffix.
    BasicBlock *FinishEntry = &Finish->getEntryBlock();
    Instruction *OldBranch = FinishEntry->getTerminator();
    BasicBlock *End = BasicBlock::Create(C, "spatial.join", Finish);
    IRBuilder<> FB(OldBranch);
    Value *Active = FB.CreateLoad(I32, Address(FB, Finish->getArg(0), ActiveField));
    BasicBlock *SuffixEntry = OldBranch->getSuccessor(0);
    OldBranch->eraseFromParent(); FB.SetInsertPoint(FinishEntry);
    FB.CreateCondBr(FB.CreateICmpNE(Active, FB.getInt32(0)), End, SuffixEntry);
    IRBuilder<> EndB(End); Value *Context = EndB.CreateLoad(Ptr, Address(EndB, Finish->getArg(0), ContextField));
    Value *Status = EndB.CreateCall(M->getOrInsertFunction("sgpl_exec_spatial_finish", FunctionType::get(I32, {Ptr}, false)), {Context});
    BasicBlock *EndFail = BasicBlock::Create(C, "failed", Finish);
    BasicBlock *Commit = (Info.HasFrontierAppend || !SourceExit->phis().empty()) ? BasicBlock::Create(C, "traversal.complete", Finish) : SuffixEntry;
    EndB.CreateCondBr(EndB.CreateICmpSLT(Status, EndB.getInt32(0)), EndFail, Commit);
    if (Commit != SuffixEntry) {
        IRBuilder<> CB(Commit);
        auto FrameOperand = [&](Value *V) -> Value * {
            return Fields.count(V) ? Resolve(CB, Finish->getArg(0), V) : V;
        };
        auto FrameArray = [&](Value *V) -> Value * {
            Value *Storage = FrameOperand(V);
            if (auto *A = dyn_cast<AllocaInst>(V)) if (!A->getAllocatedType()->isPointerTy()) return Storage;
            if (auto *G = dyn_cast<GlobalVariable>(V)) if (G->getValueType()->isArrayTy()) return Storage;
            return CB.CreateLoad(Ptr, Storage);
        };
        DenseMap<const Value *, Value *> ExitValues;
        Value *Induction = driverIndVar(Info.DriverLoop);
        auto *SourceBranch = dyn_cast<BranchInst>(SourceHeader->getTerminator());
        auto *SourceCompare = SourceBranch && SourceBranch->isConditional() ? dyn_cast<ICmpInst>(SourceBranch->getCondition()) : nullptr;
        std::function<Value *(Value *)> ExitValue = [&](Value *V) -> Value * {
            if (isa<Constant>(V)) return V;
            if (ExitValues.count(V)) return ExitValues[V];
            if (V == Induction && SourceCompare && SourceCompare->getOperand(0) == Induction &&
                (SourceCompare->getPredicate() == ICmpInst::ICMP_SLT || SourceCompare->getPredicate() == ICmpInst::ICMP_ULT))
                return ExitValue(SourceCompare->getOperand(1));
            if (Fields.count(V)) return Resolve(CB, Finish->getArg(0), V);
            auto *I = dyn_cast<Instruction>(V);
            if (!I || I->isTerminator() || isa<PHINode>(I) || I->mayHaveSideEffects() || I->mayReadOrWriteMemory()) return nullptr;
            SmallVector<Value *, 8> Operands;
            for (Value *O : I->operands()) { Value *Mapped = ExitValue(O); if (!Mapped) return nullptr; Operands.push_back(Mapped); }
            Instruction *Copy = I->clone();
            for (unsigned N = 0; N < Operands.size(); ++N) Copy->setOperand(N, Operands[N]);
            CB.Insert(Copy); return ExitValues[V] = Copy;
        };
        for (PHINode &P : SourceExit->phis()) {
            int Incoming = P.getBasicBlockIndex(SourceHeader);
            Value *Mapped = Incoming >= 0 ? ExitValue(P.getIncomingValue(Incoming)) : nullptr;
            if (!Mapped) { Cleanup(); return Refuse("traversal SSA output requires an unavailable completion value"); }
            CB.CreateStore(Mapped, Address(CB, Finish->getArg(0), Fields[&P]));
        }
        Value *CommitStatus = CB.getInt32(0);
        if (Info.HasFrontierAppend && Info.NextSetPtr) {
            Value *Bitmap = CB.CreateLoad(Ptr, FrameOperand(Info.NextSetPtr));
            CommitStatus = CB.CreateCall(M->getOrInsertFunction("autograph_exec_ctx_commit_frontier_bitmap",
                FunctionType::get(I32, {Ptr, Ptr, I32}, false)), {Context, Bitmap, Status});
        } else if (Info.HasFrontierAppend && Info.AppendArrayPtr) {
            Value *Array = FrameArray(Info.AppendArrayPtr);
            CommitStatus = CB.CreateCall(M->getOrInsertFunction("autograph_exec_ctx_commit_frontier_array",
                FunctionType::get(I32, {Ptr, Ptr, I32}, false)), {Context, Array, Status});
        }
        BasicBlock *Committed = BasicBlock::Create(C, "frontier.committed", Finish);
        BasicBlock *CommitFailed = BasicBlock::Create(C, "frontier.commit.failed", Finish);
        CB.CreateCondBr(CB.CreateICmpSLT(CommitStatus, CB.getInt32(0)), CommitFailed, Committed);
        IRBuilder<>(CommitFailed).CreateRet(CommitStatus); CB.SetInsertPoint(Committed);
        if (Info.HasFrontierAppend && Info.AppendCountPtr) CB.CreateStore(Status, FrameOperand(Info.AppendCountPtr));
        CB.CreateBr(SuffixEntry);
    }
    IRBuilder<>(EndFail).CreateRet(Status);
    Function *Batch = Function::Create(WorkTy, GlobalValue::InternalLinkage, "sgpl.round.spatial.batch", M);
    Batch->addFnAttr("sgpl.spatial.callback");
    IRBuilder<> BB(BasicBlock::Create(C, "entry", Batch));
    Active = BB.CreateLoad(I32, Address(BB, Batch->getArg(0), ActiveField));
    BasicBlock *Work = BasicBlock::Create(C, "work", Batch), *NoWork = BasicBlock::Create(C, "skip", Batch);
    BB.CreateCondBr(BB.CreateICmpNE(Active, BB.getInt32(0)), Work, NoWork);
    IRBuilder<>(NoWork).CreateRet(ConstantInt::get(I32, 0)); BB.SetInsertPoint(Work);
    Context = BB.CreateLoad(Ptr, Address(BB, Batch->getArg(0), ContextField));
    BB.CreateRet(BB.CreateCall(M->getOrInsertFunction("sgpl_exec_spatial_partition", FunctionType::get(I32, {Ptr, I32}, false)),
        {Context, BB.CreateTrunc(Batch->getArg(1), I32)}));
    Function *Release = Function::Create(FunctionType::get(Type::getVoidTy(C), {Ptr}, false), GlobalValue::InternalLinkage, "sgpl.round.spatial.release", M);
    IRBuilder<> Rel(BasicBlock::Create(C, "entry", Release));
    Context = Rel.CreateLoad(Ptr, Address(Rel, Release->getArg(0), ContextField));
    Rel.CreateCall(M->getOrInsertFunction("autograph_exec_ctx_destroy", FunctionType::get(Rel.getVoidTy(), {Ptr}, false)), {Context});
    if (Parts.PrivateRecords) {
        Value *Records = Rel.CreateLoad(Ptr, Address(Rel, Release->getArg(0), PrivateField));
        Value *PartsCount = Rel.CreateLoad(I32, Address(Rel, Release->getArg(0), PrivateCountField));
        for (unsigned N = 0; N < Info.PrivArrays.size(); ++N) Rel.CreateCall(M->getOrInsertFunction("autograph_priv_release_instance",
            FunctionType::get(Rel.getVoidTy(), {Ptr, I64, I32}, false)), {Records, Rel.getInt64(8 * (Info.Slots.size() + N)), PartsCount});
    }
    BasicBlock *FreeDescriptors = BasicBlock::Create(C, "unconsumed.descriptors", Release);
    BasicBlock *FreeBuffers = BasicBlock::Create(C, "free.buffers", Release);
    Rel.CreateCondBr(Rel.CreateICmpEQ(Context, ConstantPointerNull::get(cast<PointerType>(Ptr))), FreeDescriptors, FreeBuffers);
    Rel.SetInsertPoint(FreeDescriptors);
    for (CallInst *D : Descriptors) Rel.CreateCall(M->getOrInsertFunction("free", FunctionType::get(Rel.getVoidTy(), {Ptr}, false)),
        {Rel.CreateLoad(Ptr, Address(Rel, Release->getArg(0), ReserveFields[D]))});
    Rel.CreateBr(FreeBuffers); Rel.SetInsertPoint(FreeBuffers);
    for (AllocaInst *A : DynamicAllocations) Rel.CreateCall(M->getOrInsertFunction("free", FunctionType::get(Rel.getVoidTy(), {Ptr}, false)),
        {Rel.CreateLoad(Ptr, Address(Rel, Release->getArg(0), Fields[A]))});
    for (auto A : Allocations) Rel.CreateCall(M->getOrInsertFunction("free", FunctionType::get(Rel.getVoidTy(), {Ptr}, false)),
        {Rel.CreateLoad(Ptr, Address(Rel, Release->getArg(0), ReserveFields[A]))});
    Rel.CreateRetVoid();
    for (Function &F : *M) if (!Existing.count(&F) && !F.isDeclaration()) {
        if (verifyFunction(F, &errs())) { Cleanup(); return Refuse("outlined callback has an unsupported capture or CFG"); }
    }
    // Only now replace the original round loop.
    IRBuilder<> AB(&Original->getEntryBlock(), Original->getEntryBlock().getFirstInsertionPt());
    Value *SharedState = AB.CreateAlloca(Shared, nullptr, "round.spatial.captures");
    StructType *PlanTy = StructType::get(C, {Ptr, I64, I64, I64, I32, Ptr, Ptr, Ptr, Ptr, Ptr});
    Value *Plan = AB.CreateAlloca(PlanTy, nullptr, "round.spatial.plan");
    BasicBlock *Pre = R.Body.Unit->getLoopPreheader();
    BasicBlock *Exit = R.Body.Unit->getUniqueExitBlock();
    BasicBlock *Dispatch = BasicBlock::Create(C, "round.spatial.dispatch", Original);
    BasicBlock *Join = BasicBlock::Create(C, "round.spatial.join", Original);
    IRBuilder<> DB(Dispatch); Value *Graph = temporalPrepare(DB, Info);
    Value *RequestedParts = gpuBackendSelected(M) ? static_cast<Value *>(DB.CreateCall(M->getOrInsertFunction(
        "autograph_gpu_clean_cut_partitions", FunctionType::get(I32, false)))) : DB.getInt32(0);
    Value *PartCount = DB.CreateCall(M->getOrInsertFunction("autograph_build_clean_cut", FunctionType::get(I32, {Ptr, I32}, false)), {Graph, RequestedParts});
    Value *First32 = R.Phi ? R.Start : DB.CreateLoad(I32, R.Counter);
    Value *Bound32 = roundExternalValue(DB, R.Bound, R);
    Value *First = DB.CreateSExt(First32, I64), *Bound = DB.CreateSExt(Bound32, I64);
    Value *Count = DB.CreateSelect(DB.CreateICmpSLT(First, Bound), DB.CreateSub(Bound, First), DB.getInt64(0));
    for (unsigned N = 0; N < Captures.size(); ++N) DB.CreateStore(Captures[N], DB.CreateStructGEP(Shared, SharedState, N));
    SmallVector<Value *, 10> PlanValues{SharedState, First, Count, DB.getInt64(M->getDataLayout().getTypeAllocSize(Frame)), PartCount,
        Reserve, Setup, Batch, Finish, Release};
    for (unsigned N = 0; N < PlanValues.size(); ++N) DB.CreateStore(PlanValues[N], DB.CreateStructGEP(PlanTy, Plan, N));
    auto Fn = M->getOrInsertFunction("sgpl_exec_dag_round_spatial", FunctionType::get(I32, {Ptr, I32}, false));
    Value *Budget=temporalBudget(DB);CallInst *Call=nullptr;
    if(Parts.GpuPartition) {
        uint64_t FrameBytes=M->getDataLayout().getTypeAllocSize(Frame);
        const StructLayout *Layout=M->getDataLayout().getStructLayout(Frame);
        Function *DeviceRoot=gpuSpatialSpan(Parts.GpuPartition,Layout->getElementOffset(ContextField),Layout->getElementOffset(ActiveField));
        GpuCallback Device=buildGpuCallback(DeviceRoot,FrameBytes);
        if(Device.Kernel) {
            emitGpuStepPointeeRegs(M,C,DeviceRoot);
            auto *FieldTy=StructType::get(C,{I64,I64,I32});SmallVector<Constant *,32> GPUFields;
            auto Add=[&](uint64_t Offset,uint64_t Bytes,unsigned Kind){GPUFields.push_back(ConstantStruct::get(FieldTy,{DB.getInt64(Offset),DB.getInt64(Bytes),DB.getInt32(Kind)}));};
            Add(Layout->getElementOffset(ContextField),sizeof(sgpl_exec_ctx),3);
            for(Value *V:Captures) if(V->getType()->isPointerTy()) {
                uint64_t Bytes=0;unsigned Kind=0;
                if(V==Info.GraphPtr) Kind=1;
                else if(auto *G=dyn_cast<GlobalVariable>(V->stripPointerCasts())) {Bytes=M->getDataLayout().getTypeAllocSize(G->getValueType());Kind=G->getValueType()->isPointerTy()?2:0;}
                else if(auto *A=dyn_cast<AllocaInst>(V->stripPointerCasts())) {
                    if(auto *N=dyn_cast<ConstantInt>(A->getArraySize())) Bytes=M->getDataLayout().getTypeAllocSize(A->getAllocatedType())*N->getZExtValue();
                    if(A->getAllocatedType()->isPointerTy()) Kind=2;
                }
                Add(Layout->getElementOffset(Fields[V]),Bytes,Kind);
            }
            for(const Value *V:IndirectPrivate) Add(Layout->getElementOffset(Fields[V]),0,0);
            SmallVector<Constant *,16> Globals;
            for(unsigned N=0;N<Device.Globals.size();++N) {
                auto *G=Device.Globals[N];Globals.push_back(G);
                unsigned Kind=G->getValueType()->isPointerTy()?2:0;
                if(is_contained(Device.ReadOnlyGlobals,G)) Kind|=SGPL_GPU_FIELD_READ_ONLY;
                Add(FrameBytes+N*8,M->getDataLayout().getTypeAllocSize(G->getValueType()),Kind);
            }
            auto *FA=ArrayType::get(FieldTy,GPUFields.size()),*GA=ArrayType::get(Ptr,Globals.size());
            auto *Descriptors=new GlobalVariable(*M,FA,true,GlobalValue::PrivateLinkage,ConstantArray::get(FA,GPUFields),"sgpl.gpu.round.fields");
            auto *GlobalPointers=new GlobalVariable(*M,GA,true,GlobalValue::PrivateLinkage,ConstantArray::get(GA,Globals),"sgpl.gpu.round.globals");
            auto GpuFn=M->getOrInsertFunction("sgpl_exec_dag_round_spatial_gpu",FunctionType::get(I32,{Ptr,I32,Ptr,I64,Ptr,I32,Ptr,I32},false));
            Call=DB.CreateCall(GpuFn,{Plan,Budget,gpuCString(M,Device.Kernel->getName(),"gpu.round.spatial.name"),
                DB.getInt64(FrameBytes+Globals.size()*8),Descriptors,DB.getInt32(GPUFields.size()),GlobalPointers,DB.getInt32(Globals.size())});
            Call->setMetadata("sgpl.execution.backend",MDNode::get(C,MDString::get(C,"gpu-or-cpu")));
        } else if(getenv("SGPL_GPU_DEBUG")) errs()<<"[gpu-capability] RoundSpatial: "<<Device.Reason<<"; CPU\n";
    }
    if(!Call) Call=DB.CreateCall(Fn,{Plan,temporalCpuBudget(DB)});
    Call->setMetadata("sgpl.temporal.scope", MDNode::get(C, MDString::get(C, "Round")));
    Call->setMetadata("sgpl.spatial.group", MDNode::get(C, {MDString::get(C, spatialGroupName(Spatial.Group)),
        MDString::get(C, spatialGroupName(Spatial.CleanCutGroup)), MDString::get(C, "occupied-groups>=2")}));
    SmallVector<Metadata *, 8> Contracts;
    for (const auto &Contract : Spatial.Contracts) {
        SmallVector<Metadata *, 8> Sites{MDString::get(C, dischargeName(Contract.Kind))};
        for (unsigned Site : Contract.Sites) Sites.push_back(ConstantAsMetadata::get(DB.getInt32(Site)));
        Contracts.push_back(MDNode::get(C, Sites));
    }
    Call->setMetadata("sgpl.spatial.contracts", MDNode::get(C, Contracts));
    Call->setMetadata("sgpl.runtime.realization", MDNode::get(C, MDString::get(C, "realized")));
    Call->setMetadata("sgpl.schedule", MDNode::get(C, MDString::get(C, "round-spatial-dag")));
    temporalFailureBranch(DB, Call, Join);
    IRBuilder<> JB(Join); Value *Final = JB.CreateTrunc(JB.CreateAdd(First, Count), I32);
    if (R.Counter) JB.CreateStore(Final, R.Counter);
    JB.CreateBr(Exit);
    for (PHINode &P : Exit->phis()) {
        Value *Old = P.getIncomingValueForBlock(R.Body.Unit->getHeader());
        P.addIncoming(Old == R.Phi ? Final : Old, Join);
    }
    auto *Term = Pre->getTerminator();
    for (unsigned S = 0; S < Term->getNumSuccessors(); ++S) if (Term->getSuccessor(S) == R.Body.Unit->getHeader()) Term->setSuccessor(S, Dispatch);
    for (PHINode &P : R.Body.Unit->getHeader()->phis()) P.removeIncomingValue(Pre, false);
    Spatial.Realized = true;
    markDagOwned(Info.NeighborLoop, "spatial+temporal", "parallel", "parallel");
    markTemporalScope(Info.NeighborLoop, "Round");
    Info.Schedule.Kind = ScheduleKind::RoundSpatialDag;
    if (getenv("GRAPH_FRONTIER_STATS") || getenv("SGPL_WITNESS_DUMP"))
        errs() << "[round-spatial-runtime] realized group=" << spatialGroupName(Spatial.Group) << " cleancut=" << spatialGroupName(Spatial.CleanCutGroup) << "\n";
    return true;
}
