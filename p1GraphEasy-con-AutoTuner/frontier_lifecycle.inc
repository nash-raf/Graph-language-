/* Recognize a frontier by its round lifecycle, before constructing Activate.
 * This is an identity/ordering proof, not permission to discard duplicates or
 * reorder appends. Unrecognized lifecycles retain their original serial IR.
 * Supported publication forms are reciprocal array-pointer swaps with a count
 * assignment, and assignment of a freshly emptied bitmap to the current set. */
static Value *lifecycleValue(Value *V)
{
    while (auto *C = dyn_cast_or_null<CastInst>(V))
        V = C->getOperand(0);
    return V;
}

static LoadInst *lifecycleLoad(Value *V, const Value *Slot)
{
    auto *L = dyn_cast_or_null<LoadInst>(lifecycleValue(V));
    return L && L->getPointerOperand() == Slot ? L : nullptr;
}

static bool recognizeFrontierLifecycle(const NeighborLoopInfo &Info)
{
    Loop *Driver = Info.DriverLoop;
    Loop *Round = Driver ? Driver->getParentLoop() : nullptr;
    if (!Round || !Info.HasFrontierAppend)
        return false;
    BasicBlock *Latch = Round->getLoopLatch();
    if (!Latch)
        return false;
    Function &F = *Driver->getHeader()->getParent();
    DominatorTree DT(F);
    const bool Array = Info.AppendArrayPtr != nullptr;
    Value *Current = Array ? Info.FrontierArrayPtr : Info.FrontierSetPtr;
    Value *Next = Array ? Info.AppendArrayPtr : Info.NextSetPtr;
    if (!Current || !Next || Current == Next ||
        (Array && (Info.NextSetPtr || !Info.AppendCountPtr ||
                   !Info.FrontierSizePtr || Info.AppendCountPtr == Info.FrontierSizePtr)))
        return false;

    // Address-taken slots could be changed behind the syntactic proof's back.
    for (Value *Slot : {Current, Next,
                       Array ? Info.FrontierSizePtr : nullptr,
                       Array ? Info.AppendCountPtr : nullptr}) {
        if (!Slot) continue;
        for (User *U : Slot->users()) {
            if (auto *L = dyn_cast<LoadInst>(U)) {
                if (L->getPointerOperand() == Slot) continue;
            }
            if (auto *S = dyn_cast<StoreInst>(U)) {
                if (S->getPointerOperand() == Slot) continue;
            }
            // Capacity preparation receives the pointer slot by reference.
            if (auto *I = dyn_cast<Instruction>(U))
                if (I->getMetadata("sgpl.frontier.infrastructure")) continue;
            return false;
        }
    }

    auto BeforeDriver = [&](Instruction *I) {
        return !Driver->contains(I) &&
               DT.dominates(I, Driver->getHeader()->getTerminator());
    };
    auto AfterDriver = [&](Instruction *I) {
        return !Driver->contains(I) &&
               DT.dominates(Driver->getHeader(), I->getParent()) &&
               DT.dominates(I, Latch->getTerminator());
    };
    auto IsVertex = [&](Value *V) {
        V = lifecycleValue(V);
        return lifecycleLoad(V, Info.VAlloca) ||
               (Info.UAlloca && lifecycleLoad(V, Info.UAlloca)) ||
               V == lifecycleValue(Info.UVal);
    };

    StoreInst *Reset = nullptr, *Publish = nullptr, *Recycle = nullptr;
    StoreInst *PublishSize = nullptr;
    SmallVector<StoreInst *, 4> Appends, Increments;
    SmallVector<CallInst *, 4> Adds;
    auto Unique = [](StoreInst *&Site, StoreInst *S) {
        if (Site) return false;
        Site = S;
        return true;
    };
    for (BasicBlock *BB : Round->blocks())
        for (Instruction &I : *BB) {
            if (auto *S = dyn_cast<StoreInst>(&I)) {
                Value *Ptr = S->getPointerOperand();
                if (Ptr == Current) {
                    if (!Unique(Publish, S) || !AfterDriver(S) ||
                        !lifecycleLoad(S->getValueOperand(), Next)) return false;
                } else if (Ptr == Next) {
                    if (Array) {
                        if (!Unique(Recycle, S) || !AfterDriver(S) ||
                            !lifecycleLoad(S->getValueOperand(), Current)) return false;
                    } else {
                        auto *C = dyn_cast<CallInst>(S->getValueOperand());
                        if (!Unique(Reset, S) || !BeforeDriver(S) || !C ||
                            !C->hasOneUse() || !C->getCalledFunction() ||
                            C->getCalledFunction()->getName() != "roaring_bitmap_create" ||
                            !Round->contains(C) || !DT.dominates(C, S)) return false;
                    }
                } else if (Array && Ptr == Info.FrontierSizePtr) {
                    if (!Unique(PublishSize, S) || !AfterDriver(S) ||
                        !lifecycleLoad(S->getValueOperand(), Info.AppendCountPtr)) return false;
                } else if (Array && Ptr == Info.AppendCountPtr) {
                    auto *Zero = dyn_cast<ConstantInt>(S->getValueOperand());
                    if (Zero && Zero->isZero()) {
                        if (!Unique(Reset, S) || !BeforeDriver(S)) return false;
                    } else {
                        auto *Add = dyn_cast<BinaryOperator>(S->getValueOperand());
                        if (!Info.NeighborLoop->contains(S) || !Add ||
                            Add->getOpcode() != Instruction::Add ||
                            !lifecycleLoad(Add->getOperand(0), Info.AppendCountPtr)) return false;
                        auto *One = dyn_cast<ConstantInt>(Add->getOperand(1));
                        if (!One || !One->isOne()) return false;
                        Increments.push_back(S);
                    }
                } else if (Array) {
                    auto *G = dyn_cast<GetElementPtrInst>(Ptr);
                    if (G && lifecycleLoad(primaryIndex(G), Info.AppendCountPtr) &&
                        canonicalArrayBase(G) != Next) return false;
                    if (G && canonicalArrayBase(G) == Next) {
                        if (!Info.NeighborLoop->contains(S) ||
                            !lifecycleLoad(primaryIndex(G), Info.AppendCountPtr) ||
                            !IsVertex(S->getValueOperand())) return false;
                        Appends.push_back(S);
                    }
                }
            }
            if (auto *C = dyn_cast<CallInst>(&I))
                if (C->getCalledFunction() &&
                    C->getCalledFunction()->getName() == "roaring_bitmap_add" &&
                    (Info.NeighborLoop->contains(C) ||
                     (C->arg_size() && lifecycleLoad(C->getArgOperand(0), Next)))) {
                    if (Array || C->arg_size() < 2 ||
                        !Info.NeighborLoop->contains(C) ||
                        !lifecycleLoad(C->getArgOperand(0), Next) ||
                        !IsVertex(C->getArgOperand(1))) return false;
                    Adds.push_back(C);
                }
        }
    if (!Reset || !Publish) return false;
    LoadInst *NextValue = lifecycleLoad(Publish->getValueOperand(), Next);
    if (!AfterDriver(NextValue) || !DT.dominates(NextValue, Publish)) return false;

    // Both representations must enumerate the current collection from zero.
    auto *Ind = dyn_cast_or_null<PHINode>(driverIndVar(Driver));
    if (!Ind || Ind->getNumIncomingValues() != 2) return false;
    bool ZeroStart = false, UnitStep = false;
    for (unsigned N = 0; N < 2; ++N) {
        Value *V = Ind->getIncomingValue(N);
        if (auto *K = dyn_cast<ConstantInt>(V))
            ZeroStart |= K->isZero() && !Driver->contains(Ind->getIncomingBlock(N));
        if (auto *Add = dyn_cast<BinaryOperator>(V))
            if (Add->getOpcode() == Instruction::Add && Add->getOperand(0) == Ind)
                if (auto *K = dyn_cast<ConstantInt>(Add->getOperand(1)))
                    UnitStep |= K->isOne() && Driver->contains(Ind->getIncomingBlock(N));
    }
    if (!ZeroStart || !UnitStep) return false;

    if (Array) {
        if (!Recycle || !PublishSize || Appends.empty() ||
            Appends.size() != Increments.size()) return false;
        LoadInst *OldCurrent = lifecycleLoad(Recycle->getValueOperand(), Current);
        // Both loads must precede both swap stores; sequential assignment
        // current=next; next=current aliases the buffers and is not a swap.
        if (!AfterDriver(OldCurrent) || !DT.dominates(OldCurrent, Publish) ||
            !DT.dominates(OldCurrent, Recycle) || !DT.dominates(NextValue, Recycle)) return false;
        LoadInst *Size = lifecycleLoad(PublishSize->getValueOperand(), Info.AppendCountPtr);
        if (!AfterDriver(Size) || !DT.dominates(Size, PublishSize)) return false;
        for (StoreInst *Append : Appends) {
            auto *G = cast<GetElementPtrInst>(Append->getPointerOperand());
            auto *Buffer = lifecycleLoad(G->getPointerOperand(), Next);
            auto *Index = lifecycleLoad(primaryIndex(G), Info.AppendCountPtr);
            if (!Buffer || !Round->contains(Buffer) ||
                Index->getParent() != Append->getParent() ||
                !Index->comesBefore(Append)) return false;
            // Pair every append with its own following increment. No skipped
            // increment, shared increment, or counter change before the store.
            for (Instruction *I = Index->getNextNode(); I != Append; I = I->getNextNode())
                if (auto *S = dyn_cast<StoreInst>(I))
                    if (S->getPointerOperand() == Info.AppendCountPtr) return false;
            StoreInst *Increment = nullptr;
            for (Instruction *I = Append->getNextNode(); I; I = I->getNextNode()) {
                auto *S = dyn_cast<StoreInst>(I);
                if (!S) continue;
                if (auto *AG = dyn_cast<GetElementPtrInst>(S->getPointerOperand()))
                    if (canonicalArrayBase(AG) == Next) return false;
                if (S->getPointerOperand() == Info.AppendCountPtr) { Increment = S; break; }
            }
            if (!Increment || !llvm::is_contained(Increments, Increment)) return false;
            auto *Add = cast<BinaryOperator>(Increment->getValueOperand());
            auto *Count = lifecycleLoad(Add->getOperand(0), Info.AppendCountPtr);
            if (Count->getParent() != Increment->getParent() ||
                !DT.dominates(Reset, Count) || !DT.dominates(Count, Increment)) return false;
            for (Instruction *I = Count->getNextNode(); I != Increment; I = I->getNextNode())
                if (auto *S = dyn_cast<StoreInst>(I))
                    if (S->getPointerOperand() == Info.AppendCountPtr) return false;
        }
        // The current array's element, not its ordinal, supplies the source.
        Value *Source = lifecycleValue(Info.UVal);
        auto IsSource = [&](Value *V) {
            auto *L = dyn_cast_or_null<LoadInst>(lifecycleValue(V));
            auto *G = L ? dyn_cast<GetElementPtrInst>(L->getPointerOperand()) : nullptr;
            auto *Base = G ? lifecycleLoad(G->getPointerOperand(), Current) : nullptr;
            return G && Base && Round->contains(Base) && Driver->contains(L) &&
                   lifecycleValue(primaryIndex(G)) == driverIndVar(Driver);
        };
        bool SourceOK = IsSource(Source);
        if (Info.UAlloca)
            for (User *U : Info.UAlloca->users())
                if (auto *S = dyn_cast<StoreInst>(U))
                    if (Driver->contains(S)) {
                        if (!IsSource(S->getValueOperand())) return false;
                        SourceOK = true;
                    }
        auto *B = dyn_cast<BranchInst>(Driver->getHeader()->getTerminator());
        auto *Cmp = B && B->isConditional() ? dyn_cast<ICmpInst>(B->getCondition()) : nullptr;
        if (!SourceOK || !Cmp ||
            (Cmp->getPredicate() != ICmpInst::ICMP_SLT && Cmp->getPredicate() != ICmpInst::ICMP_ULT) ||
            lifecycleValue(Cmp->getOperand(0)) != driverIndVar(Driver) ||
            !lifecycleLoad(Cmp->getOperand(1), Info.FrontierSizePtr) ||
            !Round->contains(lifecycleLoad(Cmp->getOperand(1), Info.FrontierSizePtr))) return false;
    } else {
        if (Adds.empty()) return false;
        for (CallInst *Add : Adds)
            if (!DT.dominates(Reset, lifecycleLoad(Add->getArgOperand(0), Next))) return false;
        // Follow the source value through the frontend's optional scalar copy.
        auto IsSource = [&](Value *V) {
            auto *C = dyn_cast_or_null<CallInst>(lifecycleValue(V));
            auto *Base = C && C->arg_size() ? lifecycleLoad(C->getArgOperand(0), Current) : nullptr;
            return C && Driver->contains(C) && C->getCalledFunction() &&
                   C->getCalledFunction()->getName() == "roaring_bitmap_get_at_index" &&
                   C->arg_size() >= 2 && Base && Round->contains(Base) &&
                   lifecycleValue(C->getArgOperand(1)) == Ind;
        };
        bool SourceOK = IsSource(Info.UVal);
        if (Info.UAlloca)
            for (User *U : Info.UAlloca->users())
                if (auto *S = dyn_cast<StoreInst>(U))
                    if (Driver->contains(S)) {
                        if (!IsSource(S->getValueOperand())) return false;
                        SourceOK = true;
                    }
        if (!SourceOK) return false;
        auto *B = dyn_cast<BranchInst>(Driver->getHeader()->getTerminator());
        auto *Cmp = B && B->isConditional() ? dyn_cast<ICmpInst>(B->getCondition()) : nullptr;
        if (!Cmp || lifecycleValue(Cmp->getOperand(0)) != Ind ||
            (Cmp->getPredicate() != ICmpInst::ICMP_SLT && Cmp->getPredicate() != ICmpInst::ICMP_ULT)) return false;
        auto *Size = dyn_cast<CallInst>(lifecycleValue(Cmp->getOperand(1)));
        auto *Base = Size && Size->arg_size() ? lifecycleLoad(Size->getArgOperand(0), Current) : nullptr;
        if (!Size || !Size->getCalledFunction() ||
            Size->getCalledFunction()->getName() != "roaring_bitmap_get_cardinality" ||
            !Round->contains(Size) || !Base || !Round->contains(Base)) return false;
    }
    return true;
}
