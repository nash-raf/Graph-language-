/* Static spatial proof over Pair(u,v,o), Preamble(u), and Epilogue(u).
 * There is no runtime conflict graph. A grouping witness contains every
 * surviving relation for all possible input topologies. */
namespace {
static const char *spatialGroupName(SpatialGroup G) {
    switch (G) {
    case SpatialGroup::PairOccurrence: return "PairOccurrence";
    case SpatialGroup::Destination: return "Destination";
    case SpatialGroup::Source: return "Source";
    default: return "None";
    }
}
static const char *spatialStatusName(SpatialProofStatus S) {
    switch (S) {
    case SpatialProofStatus::IndependentFamily: return "independent-family";
    case SpatialProofStatus::NoPartition: return "no-partition";
    case SpatialProofStatus::Inconclusive: return "inconclusive";
    default: return "not-applicable";
    }
}

/* Only identical injective affine maps imply equal endpoint bindings.
 * Odd coefficients are injective under integer wraparound. Truncations and
 * non-injective maps remain unknown. No provenance equality proves ownership. */
static SpatialIndex spatialIndex(Value *V, const NeighborLoopInfo &Info,
                                  unsigned Depth = 0) {
    SpatialIndex X;
    if (!V || Depth > 16 || !V->getType()->isIntegerTy()) return X;
    X.Bits = V->getType()->getIntegerBitWidth();
    if (X.Bits > 64) return X;
    for (Value *T = V; isa<CastInst>(T); T = cast<CastInst>(T)->getOperand(0))
        if (cast<CastInst>(T)->getOpcode() == Instruction::Trunc &&
            T->getType()->getIntegerBitWidth() < 32) return X;
    if (isOwnedEndpointIndex(V, Region::U, Info)) {
        X.Axis = Region::U; X.Scale = 1; X.Injective = true; return X;
    }
    if (isOwnedEndpointIndex(V, Region::V, Info)) {
        X.Axis = Region::V; X.Scale = 1; X.Injective = true; return X;
    }
    if (auto *C = dyn_cast<ConstantInt>(V)) {
        X.Axis = Region::Bottom; X.Offset = C->getSExtValue(); return X;
    }
    if (auto *L = dyn_cast<LoadInst>(V)) {
        if (L->getPointerOperand() == Info.UAlloca) X.Axis = Region::U;
        else if (L->getPointerOperand() == Info.VAlloca) X.Axis = Region::V;
        else return X;
        X.Scale = 1; X.Injective = true; return X;
    }
    if (auto *C = dyn_cast<CastInst>(V)) {
        if (C->getOpcode() != Instruction::SExt && C->getOpcode() != Instruction::ZExt)
            return X;
        return spatialIndex(C->getOperand(0), Info, Depth + 1);
    }
    auto *B = dyn_cast<BinaryOperator>(V);
    if (!B) return X;
    SpatialIndex L = spatialIndex(B->getOperand(0), Info, Depth + 1);
    SpatialIndex R = spatialIndex(B->getOperand(1), Info, Depth + 1);
    if (L.Axis == Region::Bottom && B->getOpcode() != Instruction::Sub)
        std::swap(L, R);
    if ((L.Axis != Region::U && L.Axis != Region::V) ||
        R.Axis != Region::Bottom || L.Bits != X.Bits) return X;
    /* Unsigned operations avoid host overflow. These coefficients model the
     * same modular arithmetic as the original integer instructions. */
    uint64_t A = uint64_t(L.Scale), O = uint64_t(L.Offset), C = uint64_t(R.Offset);
    switch (B->getOpcode()) {
    case Instruction::Add: O += C; break;
    case Instruction::Sub: O -= C; break;
    case Instruction::Mul: A *= C; O *= C; break;
    default: return X;
    }
    X.Axis = L.Axis; X.Scale = int64_t(A); X.Offset = int64_t(O);
    X.Injective = L.Injective && (A & 1); return X;
}

static bool spatialPrivateBase(const NeighborLoopInfo &Info, const Value *Base) {
    if (!Info.UsePrivLayout) return false;
    for (const auto &A : Info.PrivArrays) if (A.Base == Base) return true;
    for (const auto &S : Info.Slots) if (S.Ptr == Base) return true;
    return false;
}

/* Whole-domain fission must dispatch exactly the original ascending vertex
 * domain. An arbitrary loop bound or source list does not justify the existing
 * all-vertices owner runtime, even when its body stores commute. */
static bool spatialAllVerticesDomain(const NeighborLoopInfo &Info) {
    if (Info.FrontierArrayPtr || Info.MembershipGated) return false;
    auto *Phi = dyn_cast_or_null<PHINode>(driverIndVar(Info.DriverLoop));
    auto *Header = dyn_cast<BranchInst>(Info.DriverLoop->getHeader()->getTerminator());
    auto *Compare = Header && Header->isConditional() ? dyn_cast<ICmpInst>(Header->getCondition()) : nullptr;
    if (!Phi || Phi->getNumIncomingValues() != 2 || !Compare ||
        (Compare->getPredicate() != ICmpInst::ICMP_SLT && Compare->getPredicate() != ICmpInst::ICMP_ULT) ||
        Compare->getOperand(0) != Phi) return false;
    bool Zero = false, Increment = false;
    for (Value *V : Phi->incoming_values()) {
        if (auto *C = dyn_cast<ConstantInt>(V)) Zero |= C->isZero();
        if (auto *Add = dyn_cast<BinaryOperator>(V))
            if (Add->getOpcode() == Instruction::Add && Add->getOperand(0) == Phi)
                if (auto *C = dyn_cast<ConstantInt>(Add->getOperand(1))) Increment |= C->isOne();
    }
    auto *Load = dyn_cast<LoadInst>(Compare->getOperand(1));
    auto *G = Load ? dyn_cast<GetElementPtrInst>(Load->getPointerOperand()) : nullptr;
    Value *Endpoint = Info.UVal;
    while (auto *Cast = dyn_cast_or_null<CastInst>(Endpoint)) Endpoint = Cast->getOperand(0);
    return Zero && Increment && Endpoint == Phi && G &&
        llvm::all_of(G->indices(), [](const Use &U) {
            auto *C = dyn_cast<ConstantInt>(U.get()); return C && C->isZero();
        }) && sameAddressValue(G->getPointerOperand(), Info.GraphPtr, 0);
}

static void spatialPrint(const NeighborLoopInfo &Info) {
    if (!getenv("GRAPH_FRONTIER_STATS") && !getenv("SGPL_WITNESS_DUMP")) return;
    const auto &P = Info.SpatialResult;
    unsigned Remaining = 0;
    for (const auto &R : P.Relations) Remaining += R.Survives;
    errs() << "[spatial] nodes=Pair(u,v,o) lifecycle=Preamble(u),Epilogue(u) status="
           << spatialStatusName(P.Status) << " group=" << spatialGroupName(P.Group)
           << " cleancut=" << spatialGroupName(P.CleanCutGroup)
           << " accesses=" << P.Accesses.size() << " relations=" << P.Relations.size()
           << " remaining=" << Remaining << " removed=" << P.Relations.size() - Remaining
           << " nontrivial=occupied-groups>=2 runtime=" << P.RuntimeRealizable
           << " realized=" << P.Realized << " reason=" << P.Reason << "\n";
    if (!P.RuntimeReason.empty()) errs() << "[spatial-runtime] " << P.RuntimeReason << "\n";
    if (!getenv("SGPL_WITNESS_DUMP")) return;
    for (const auto &A : P.Accesses)
        errs() << "[spatial-access] site=" << A.Site << " family=" << A.Family
               << " mode=" << (A.Write ? "W" : "R") << " base=" << valueLabel(A.Base)
               << " axis=" << regionName(A.Index.Axis) << " scale=" << A.Index.Scale
               << " offset=" << A.Index.Offset << " snapshot=" << A.Snapshot
               << " private=" << A.Private << " resource=" << A.Resource
               << " guard=" << A.Guard << "\n";
    for (unsigned I = 0; I < P.Relations.size(); ++I) {
        const auto &R = P.Relations[I];
        errs() << "[spatial-relation] id=" << I << " sites=" << R.From << "," << R.To
               << " kind=" << (R.Kind == AccessRelKind::RAW ? "RAW" :
                                R.Kind == AccessRelKind::WAR ? "WAR" : "WAW")
               << " original=" << R.OriginalCondition << " removed-condition=" << R.RemovedCondition
               << " condition=" << R.Condition << " contained=" << regionName(R.ContainedBy)
               << " uncertain=" << R.Uncertain << " survives=" << R.Survives
               << " removed-by=" << dischargeName(R.RemovedBy)
               << " order=" << R.OriginalOrder << "\n";
    }
    for (const auto &C : P.Contracts) {
        errs() << "[spatial-contract] kind=" << dischargeName(C.Kind)
               << " premises=" << C.Premises << " replacement=" << C.Replacement << " removes=";
        for (unsigned R : C.Removes) errs() << R << ",";
        errs() << "\n";
    }
}

static void proveSpatialPartition(NeighborLoopInfo &Info, AAResults &AA,
                                   DominatorTree &DT) {
    auto &P = Info.SpatialResult;
    P = SpatialDecision();
    P.Status = SpatialProofStatus::Inconclusive;
    bool Complete = true, HavePair = false, GatesModelled = true;
    SmallVector<StoreInst *,4> ArraySites;
    Info.OrderedArrayFrontier = checkedArrayAppend(Info,&ArraySites) &&
        llvm::all_of(ArraySites,[](StoreInst *S){return S->getMetadata("sgpl.frontier.checked")!=nullptr;});
    Info.ArrayAppendSites = Info.OrderedArrayFrontier ? ArraySites.size() : 0;
    auto Refuse = [&](StringRef Why) {
        Complete = false;
        if (P.Reason.empty()) P.Reason = Why.str();
    };
    const DataLayout &DL = Info.NeighborLoop->getHeader()->getParent()->getParent()->getDataLayout();
    /* Build access sites from the original instructions, including guards and
     * source lifecycle. Private resource annotations never hide memory reads. */
    auto Add = [&](Instruction *I, Value *Ptr, bool Write, uint64_t Bytes,
                   uint32_t Resource = 0, Value *Index = nullptr) {
        SpatialAccess A;
        A.Origin = I; A.Pointer = Ptr; A.Write = Write; A.Bytes = Bytes;
        A.Resource = Resource; A.Phase = sourcePhaseFor(Info, I);
        A.Family = A.Phase == PhaseSegment::Pair ? 0 :
                   A.Phase == PhaseSegment::Preamble ? 1 : 2;
        A.Site = P.Accesses.size();
        A.Guard = I->getParent()->hasName() ? I->getParent()->getName().str() : "control-flow";
        if (Ptr) {
            auto *G = dyn_cast<GetElementPtrInst>(Ptr);
            if (!G) G = dyn_cast<GetElementPtrInst>(Ptr->stripPointerCasts());
            if (G) {
                A.Base = canonicalArrayBase(G);
                Index = primaryIndex(G);
                std::string Layout;
                raw_string_ostream OS(Layout);
                G->getSourceElementType()->print(OS);
                for (Value *V : G->indices()) {
                    if (V == Index) OS << ":index";
                    else if (auto *C = dyn_cast<ConstantInt>(V)) OS << ":" << C->getSExtValue();
                    else OS << ":unknown";
                }
                A.Layout = OS.str();
                APInt Offset(DL.getIndexTypeSizeInBits(G->getType()), 0);
                if (G->accumulateConstantOffset(DL, Offset) && Offset.getBitWidth() <= 64) {
                    A.FixedOffset = true; A.ByteOffset = Offset.getSExtValue();
                }
                if (A.Layout.find("unknown") != std::string::npos) A.Uncertain = true;
            } else A.Base = Ptr->stripPointerCasts();
        }
        A.Index = spatialIndex(Index, Info);
        if (!Index) A.Index.Axis = Region::Bottom;
        if (!Write)
            for (unsigned N = 0; N < Info.RoundSepBases.size(); ++N) {
                const auto &S = Info.RoundSepBases[N];
                if (S.Base == A.Base && llvm::is_contained(S.CrossReads, dyn_cast<LoadInst>(I)) &&
                    S.ElemTy && (S.ElemTy->isIntegerTy(32) || S.ElemTy->isDoubleTy()) &&
                    !roundSepEndpointConflict(Info)) A.Snapshot = N;
            }
        A.Private = A.Phase == PhaseSegment::Pair && !A.Layout.empty() && spatialPrivateBase(Info, A.Base);
        /* R3 redirects every proven scalar slot, not just ReducePtr. Scalars
         * have no GEP layout, so mark their accesses through the checked slot
         * list. Without the record layout only ReducePtr has private storage. */
        if (!Info.AccConsumeStore && A.Phase == PhaseSegment::Pair &&
            (Info.UsePrivLayout || A.Base == Info.ReducePtr))
            for (const auto &Slot : Info.Slots)
                if (Slot.Ptr == A.Base && foldSound(Slot.Op, Slot.ElemTy)) A.Private = true;
        /* A source accumulator has source-local lifetime, not a global fold.
         * Retain its dependencies and bind it to the source family. */
        if (Info.AccConsumeStore && A.Base == Info.ReducePtr) {
            A.SourceLocal = true;
        }
        P.Accesses.push_back(A);
        HavePair |= A.Family == 0;
    };
    auto Size = [&](Type *T) -> uint64_t {
        if (!T->isSized()) return 0;
        auto N = DL.getTypeStoreSize(T);
        return N.isScalable() ? 0 : N.getFixedValue();
    };
    auto Structural = [&](Value *V) {
        return V == Info.UAlloca || V == Info.VAlloca;
    };
    for (BasicBlock *BB : Info.DriverLoop->blocks()) for (Instruction &I : *BB) {
        if (temporalInstrumentation(I) || frontierInfrastructure(I)) continue;
        if (auto *Phi = dyn_cast<PHINode>(&I)) {
            if (Phi != driverIndVar(Info.DriverLoop)) Refuse("carried SSA value has no pair-occurrence frame");
        }
        if (auto *L = dyn_cast<LoadInst>(&I)) {
            if (Structural(L->getPointerOperand())) continue;
            if (L->isVolatile() || L->isAtomic()) Refuse("observable ordered load");
            Add(L, L->getPointerOperand(), false, Size(L->getType()),
                Info.OrderedArrayFrontier && L->getPointerOperand()==Info.AppendCountPtr ? SGPL_RES_NEXT_FRONTIER : 0);
        } else if (auto *S = dyn_cast<StoreInst>(&I)) {
            if (Structural(S->getPointerOperand())) {
                if (Info.NeighborLoop->contains(S)) Refuse("pair work changes an endpoint binding");
                else if (S->getPointerOperand() == Info.UAlloca) {
                    Value *Seed = S->getValueOperand();
                    while (auto *Cast = dyn_cast<CastInst>(Seed)) Seed = Cast->getOperand(0);
                    bool Known = Seed == driverIndVar(Info.DriverLoop);
                    if (auto *Call = dyn_cast<CallInst>(Seed)) {
                        Function *F = Call->getCalledFunction();
                        if (Info.FrontierSetPtr && F && Call->arg_size() == 2 &&
                            (F->getName() == "roaring_bitmap_get_at_index" ||
                             F->getName() == "roaring_bitmap_get_element_at_index")) {
                            auto *Set = dyn_cast<LoadInst>(Call->getArgOperand(0));
                            Value *Rank = Call->getArgOperand(1);
                            while (auto *Cast = dyn_cast<CastInst>(Rank)) Rank = Cast->getOperand(0);
                            Known |= Set && Set->getPointerOperand() == Info.FrontierSetPtr &&
                                     Rank == driverIndVar(Info.DriverLoop);
                        }
                    }
                    if (!Known) Refuse("source traversal changes an endpoint binding");
                }
                continue;
            }
            if (S->isVolatile() || S->isAtomic()) Refuse("observable ordered store");
            /* The array-frontier envelope rewrites append positions too, but
             * their order is not proved by the current set-frontier contract. */
            bool Append = Info.OrderedArrayFrontier &&
                (S->getPointerOperand()==Info.AppendCountPtr || llvm::is_contained(ArraySites,S));
            Add(S, S->getPointerOperand(), true, Size(S->getValueOperand()->getType()),
                Append ? SGPL_RES_NEXT_FRONTIER : 0);
        } else if (auto *C = dyn_cast<AtomicCmpXchgInst>(&I)) {
            Add(C, C->getPointerOperand(), false, Size(C->getCompareOperand()->getType()));
            Add(C, C->getPointerOperand(), true, Size(C->getCompareOperand()->getType()));
            bool Known = llvm::is_contained(Info.DriverUClaims, C);
            for (const auto &E : Info.Effects) Known |= E.Origin == C && E.Kind == EffectKind::Claim;
            if (!Known) Refuse("unmodeled atomic claim");
        } else if (auto *C = dyn_cast<CallBase>(&I)) {
            auto *Call = dyn_cast<CallInst>(C);
            Function *F = C->getCalledFunction();
            StringRef Name = F ? F->getName() : StringRef();
            if (isIteratorInit(Call) || isIteratorNext(Call) || Name == "graph_get_edge_weight") continue;
            auto BitmapSlot = [&](Value *V) -> Value * {
                if (auto *L = dyn_cast<LoadInst>(V->stripPointerCasts())) return L->getPointerOperand();
                return V->stripPointerCasts();
            };
            if (Name.starts_with("roaring_bitmap_get_") || Name == "roaring_bitmap_contains") {
                if (C->arg_size() && BitmapSlot(C->getArgOperand(0)) == Info.FrontierSetPtr) continue;
                Refuse("bitmap observer has no immutable membership contract");
                continue;
            }
            if (Name == "roaring_bitmap_add" && Info.HasFrontierAppend &&
                envelopeWired(Info) && C->arg_size() == 2 &&
                BitmapSlot(C->getArgOperand(0)) == Info.NextSetPtr && C->use_empty() &&
                spatialIndex(C->getArgOperand(1), Info).Axis == Region::V &&
                spatialIndex(C->getArgOperand(1), Info).Scale == 1 &&
                spatialIndex(C->getArgOperand(1), Info).Offset == 0 &&
                spatialIndex(C->getArgOperand(1), Info).Injective) {
                Add(&I, nullptr, true, 4, SGPL_RES_DEST_SEEN, C->getArgOperand(1));
                Add(&I, nullptr, true, 4, SGPL_RES_NEXT_FRONTIER);
                continue;
            }
            if (C->doesNotAccessMemory() && !C->mayHaveSideEffects()) continue;
            if (F && F->isIntrinsic() && (Name.starts_with("llvm.lifetime.") || Name.starts_with("llvm.assume"))) continue;
            Refuse(F ? ("unmodeled call or traversal mutation: " + Name).str() : "indirect call");
        } else if (isa<FenceInst>(I) || I.mayWriteToMemory()) {
            Refuse("unmodeled memory or ordering operation");
        }
    }
    for (User *Use : Info.NextCall->users()) {
        auto *Cmp = dyn_cast<ICmpInst>(Use);
        bool ControlOnly = Cmp && Cmp == cast<BranchInst>(Info.NeighborLoop->getHeader()->getTerminator())->getCondition();
        if (!ControlOnly) Refuse("observed iterator result has no pair callback value");
    }
    TemporalBody Shape;
    std::string ShapeReason;
    if (!temporalBody(Info.DriverLoop, Shape, ShapeReason)) Refuse(ShapeReason);
    auto ReachesPair = [&](BasicBlock *Start) {
        SmallVector<BasicBlock *, 16> Work{Start};
        SmallPtrSet<BasicBlock *, 32> Seen;
        while (!Work.empty()) {
            BasicBlock *B = Work.pop_back_val();
            if (B == Info.NeighborLoop->getHeader()) return true;
            if (B == Info.DriverLoop->getHeader() || !Info.DriverLoop->contains(B) ||
                !Seen.insert(B).second) continue;
            for (BasicBlock *Next : successors(B)) Work.push_back(Next);
        }
        return false;
    };
    SmallVector<std::pair<unsigned, std::pair<Value *, bool>>, 4> OrderedGates;
    for (BasicBlock *BB : Info.DriverLoop->blocks()) {
        if (Info.NeighborLoop->contains(BB) || BB == Info.DriverLoop->getHeader()) continue;
        auto *Branch = dyn_cast<BranchInst>(BB->getTerminator());
        if (!Branch || !Branch->isConditional()) continue;
        bool True = ReachesPair(Branch->getSuccessor(0)), False = ReachesPair(Branch->getSuccessor(1));
        if (True != False) {
            if (!DT.dominates(BB, Info.NeighborLoop->getHeader())) GatesModelled = false;
            else OrderedGates.push_back({DT.getNode(BB)->getLevel(), {Branch->getCondition(), True}});
        }
    }
    llvm::sort(OrderedGates, [](const auto &A, const auto &B) { return A.first < B.first; });
    for (const auto &Gate : OrderedGates) P.SourceGates.push_back(Gate.second);
    if (!HavePair) { P.Status = SpatialProofStatus::NotApplicable; P.Reason = "no pair effects"; return; }
    /* Source claims move into an ordered preparation envelope only when
     * their old value is used solely by the checked guard. Nested claims
     * require a control-flow frame which this realization does not provide. */
    auto Claims = perSourceClaims(Info);
    bool ClaimStaged = !Claims.empty() && Claims.size() == 1 &&
        (!Info.FrontierArrayPtr || Info.OrderedArrayFrontier);
    SmallVector<unsigned, 4> ClaimSites;
    if (ClaimStaged) {
        const Effect *Claim = Claims.front();
        auto *Store = dyn_cast_or_null<StoreInst>(Claim->Origin);
        auto *Guard = Store ? guardICmpForStore(Store) : nullptr;
        ClaimStaged &= Store && Guard && sourcePhaseFor(Info, Store) == PhaseSegment::Preamble;
        for (const auto &A : P.Accesses) if (A.Base == Claim->Base && !A.Layout.empty()) {
            if(!A.Write && A.Phase==PhaseSegment::Pair && A.Snapshot>=0) continue;
            /* A pair may repeat its source's successful constant claim. It
             * does not change the already-staged value, and source ownership
             * keeps these writes on the claiming source's worker. Retain its
             * spatial dependencies; it is not another claim-result site. */
            if (A.Write && A.Phase == PhaseSegment::Pair && Store &&
                A.Index.Axis == Region::U && A.Index.Injective &&
                A.Index.Scale == 1 && A.Index.Offset == 0)
                if (auto *Repeat = dyn_cast<StoreInst>(A.Origin))
                    if (!Repeat->isVolatile() && !Repeat->isAtomic() &&
                        isa<ConstantInt>(Store->getValueOperand()) &&
                        Repeat->getValueOperand() == Store->getValueOperand())
                        continue;
            if (A.Phase != PhaseSegment::Preamble || A.Index.Axis != Region::U ||
                !A.Index.Injective || A.Index.Scale != 1 || A.Index.Offset != 0) ClaimStaged = false;
            if (A.Write) ClaimStaged &= A.Origin == Store;
            else if (auto *Load = dyn_cast<LoadInst>(A.Origin)) {
                for (User *Use : Load->users()) ClaimStaged &= Use == Guard;
            } else ClaimStaged = false;
            ClaimSites.push_back(A.Site);
        }
        for (const auto &Gate : P.SourceGates)
            ClaimStaged &= Gate.first == Guard && Guard &&
                ((Guard->getPredicate() == ICmpInst::ICMP_EQ) == Gate.second);
        if (Guard) for (User *Use : Guard->users()) ClaimStaged &= isa<BranchInst>(Use);
    }
    auto AddContract = [&](DischargeKind Kind, unsigned Relation, unsigned A, unsigned B) {
        SpatialContract *Found = nullptr;
        for (auto &C : P.Contracts) if (C.Kind == Kind) Found = &C;
        if (!Found) {
            P.Contracts.emplace_back(); Found = &P.Contracts.back(); Found->Kind = Kind;
            if (Kind == DischargeKind::Snapshot) {
                Found->Premises = "designated CrossReads; supported element type; immutable distinct snapshot";
                Found->Replacement = "snapshot-publication -> pair-work -> join -> snapshot-release";
            } else if (Kind == DischargeKind::Privatization) {
                Found->Premises = "privLayout; matched private base; operator/type foldSound; no intermediate observer";
                Found->Replacement = "private-identity-init -> pair-work -> join -> deterministic-combine";
            } else if (Kind == DischargeKind::ClaimStaging) {
                Found->Premises = "one constant source claim; unique domain; exact u address; guard-only result; no live pair observer";
                Found->Replacement = "ordered-source-claim -> captured-source-gate -> pair-work -> join";
            } else if (Kind == DischargeKind::SourceFrame) {
                Found->Premises = "source reset seed; ordered complete row; once-only consume; last source output";
                Found->Replacement = "source-frame-init -> source-pairs-in-order -> source-consume -> join";
            } else {
                Found->Premises = Info.OrderedArrayFrontier
                    ? "checked array append sites; counter has no intermediate observer; private occurrence slots; duplicates and order retained"
                    : "set-frontier envelope; unobserved add result; private deduplication/append lanes; no live membership observer";
                Found->Replacement = Info.OrderedArrayFrontier
                    ? "symbolic-capacity-reserve -> private-occurrence-init -> pair-work -> join -> ordered-compaction -> frontier-progress"
                    : "membership/seen-init -> pair-work -> join -> lane-merge -> frontier-progress";
            }
        }
        Found->Removes.push_back(Relation);
        for (unsigned S : {A, B}) if (!llvm::is_contained(Found->Sites, S)) Found->Sites.push_back(S);
    };
    uint64_t Limit = 4096;
    if (const char *V = getenv("SGPL_REL_CAP")) {
        char *End = nullptr; unsigned long long N = strtoull(V, &End, 10);
        if (End != V && !*End) Limit = N;
    }
    auto Relate = [&](unsigned AID, unsigned BID, AccessRelKind Kind) {
        const auto &A = P.Accesses[AID], &B = P.Accesses[BID];
        if (A.Resource != B.Resource) return;
        bool Uncertain = A.Uncertain || B.Uncertain;
        if (!A.Resource) {
            TemporalAccess TA{A.Origin, A.Pointer, A.Write, 0};
            TemporalAccess TB{B.Origin, B.Pointer, B.Write, 0};
            bool SlotVsElement = A.Layout.empty() != B.Layout.empty();
            if (A.Base != B.Base || SlotVsElement) {
                /* Query whole base objects, not same-iteration indexed GEPs. */
                auto BasePointer = [](const SpatialAccess &X) {
                    auto *G = dyn_cast<GetElementPtrInst>(X.Pointer->stripPointerCasts());
                    return G ? G->getPointerOperand() : X.Pointer;
                };
                TA.Pointer = BasePointer(A); TB.Pointer = BasePointer(B);
                TA.Origin = nullptr; TB.Origin = nullptr;
                if (!temporalMayAlias(TA, TB, AA, Uncertain,Info.DriverLoop)) return;
            }
        }
        if (!A.Resource && A.Base == B.Base && A.FixedOffset && B.FixedOffset &&
            A.Layout.empty() == B.Layout.empty() && A.Bytes && B.Bytes) {
            if (A.ByteOffset <= B.ByteOffset && uint64_t(B.ByteOffset) - uint64_t(A.ByteOffset) >= A.Bytes) return;
            if (B.ByteOffset <= A.ByteOffset && uint64_t(A.ByteOffset) - uint64_t(B.ByteOffset) >= B.Bytes) return;
        }
        if (P.Relations.size() >= Limit) { P.Overflow = true; Refuse("spatial relation limit exceeded"); return; }
        SpatialRelation R;
        R.From = AID; R.To = BID; R.Kind = Kind; R.Uncertain = Uncertain;
        R.OriginalOrder = "source ordinal, neighbor ordinal, CFG site order";
        R.Condition = "may overlap for distinct (u,v,o),(u',v',o')";
        bool SameStorage = A.Resource || (A.Base == B.Base &&
                            A.Layout.empty() == B.Layout.empty());
        const auto &X = A.Index, &Y = B.Index;
        if (SameStorage && A.Bytes == B.Bytes && A.Layout == B.Layout &&
            !A.Uncertain && !B.Uncertain && X.Injective && Y.Injective &&
            X.Axis == Y.Axis && X.Scale == Y.Scale && X.Offset == Y.Offset && X.Bits == Y.Bits) {
            R.ContainedBy = X.Axis;
            R.Condition = X.Axis == Region::U ? "u=u'" : "v=v'";
        }
        R.OriginalCondition = R.Condition;
        if (A.SourceLocal && B.SourceLocal && SameStorage) {
            R.ContainedBy = Region::U;
            R.Condition = "u=u' after source-frame binding";
            R.RemovedBy = DischargeKind::SourceFrame;
            R.RemovedCondition = "u!=u'";
        }
        /* Version/storage changes remove only covered relations. Both sides
         * on the same immutable snapshot are reads, so they create no edge. */
        bool SnapshotRead = (!A.Write && A.Snapshot >= 0) || (!B.Write && B.Snapshot >= 0);
        bool BothPrivate = A.Private && B.Private && SameStorage;

        if (SnapshotRead && Kind != AccessRelKind::WAW) {
            R.Survives = false; R.RemovedBy = DischargeKind::Snapshot;
        } else if (BothPrivate) {
            R.Survives = false; R.RemovedBy = DischargeKind::Privatization;
        } else if (ClaimStaged && llvm::is_contained(ClaimSites, AID) &&
                   llvm::is_contained(ClaimSites, BID)) {
            R.Survives = false; R.RemovedBy = DischargeKind::ClaimStaging;
        } else if ((A.Resource == SGPL_RES_NEXT_FRONTIER || A.Resource == SGPL_RES_DEST_SEEN) &&
                   ((A.Write && B.Write) || (Info.OrderedArrayFrontier && A.Resource==SGPL_RES_NEXT_FRONTIER))) {
            R.Survives = false; R.RemovedBy = DischargeKind::FrontierPrivatization;
        }
        if (!R.Survives) R.RemovedCondition = "all covered collisions after storage/version replacement";
        unsigned ID = P.Relations.size(); P.Relations.push_back(R);
        if (R.RemovedBy != DischargeKind::None) AddContract(R.RemovedBy, ID, AID, BID);
    };
    /* Directed labels include both original-order orientations across arbitrary
     * iterations. Equality relations alone do not establish a direction. */
    for (unsigned A = 0; A < P.Accesses.size() && !P.Overflow; ++A)
        for (unsigned B = A; B < P.Accesses.size() && !P.Overflow; ++B) {
            bool W = P.Accesses[A].Write, V = P.Accesses[B].Write;
            if (!W && !V) continue;
            if (W && V) Relate(A, B, AccessRelKind::WAW);
            else {
                Relate(A, B, W ? AccessRelKind::RAW : AccessRelKind::WAR);
                Relate(B, A, W ? AccessRelKind::WAR : AccessRelKind::RAW);
            }
        }
    if (!Complete) return;
    P.CoverageComplete = true;
    bool Pair = true, Source = true, Destination = true;
    for (const auto &R : P.Relations) if (R.Survives) {
        Pair = false;
        Source &= R.ContainedBy == Region::U;
        Destination &= R.ContainedBy == Region::V;
    }
    /* Source-local reset/consume/claim values are replacement dependencies.
     * They require source grouping unless a checked staging contract exists. */
    if (Info.AccConsumeStore || (hasPerSourceClaim(Info) && !ClaimStaged)) {
        Pair = false; Destination = false;
    }
    P.SourceValid = Source; P.DestinationValid = Destination;
    if (!Pair && !Source && !Destination) {
        P.Reason = "surviving relations require topology-dependent or unresolved grouping";
        for (const auto &A : P.Accesses) {
            auto *G = dyn_cast_or_null<GetElementPtrInst>(A.Pointer);
            bool ConstantAddress = G && llvm::all_of(G->indices(), [](const Use &U) {
                return isa<ConstantInt>(U.get());
            });
            ConstantAddress |= A.Pointer && (isa<AllocaInst>(A.Pointer) || isa<GlobalVariable>(A.Pointer));
            bool SharedLifetime = A.Base && !isa<Instruction>(A.Base);
            if (auto *Allocation = dyn_cast_or_null<AllocaInst>(A.Base))
                SharedLifetime = !Info.DriverLoop->contains(Allocation);
            /* A pointer slot changed by the nest does not denote one fixed
             * address across all occurrences, even when the GEP is constant. */
            for (const auto &Other : P.Accesses)
                if (Other.Write && Other.Pointer && Other.Pointer->stripPointerCasts() == A.Base)
                    if (Other.Origin != A.Origin) SharedLifetime = false;
            if (SharedLifetime && A.Family == 0 && A.Write && !A.Private && !A.Resource && ConstantAddress &&
                Info.NeighborLoop->getLoopLatch() && DT.dominates(A.Origin, Info.NeighborLoop->getLoopLatch()->getTerminator())) {
                P.Status = SpatialProofStatus::NoPartition;
                P.Reason = "unconditional shared write connects every pair occurrence";
                break;
            }
        }
        return;
    }
    P.Status = SpatialProofStatus::IndependentFamily;
    P.Group = Pair ? SpatialGroup::PairOccurrence : Destination ? SpatialGroup::Destination : SpatialGroup::Source;
    P.CleanCutGroup = Destination ? SpatialGroup::Destination : SpatialGroup::Source;
    P.Reason = "every surviving relation stays within the symbolic group";
    P.RuntimeRealizable = legacySourceLifecycleAvailable(Info);
    if(Info.OrderedArrayFrontier) P.RuntimeRealizable=P.RuntimeRealizable && !Info.AccConsumeStore &&
        !Info.AccResetStore && !Info.ReducePtr && Info.DriverLoop->getLoopPreheader();
    auto *Phi = dyn_cast_or_null<PHINode>(driverIndVar(Info.DriverLoop));
    auto *Header = dyn_cast<BranchInst>(Info.DriverLoop->getHeader()->getTerminator());
    auto *Compare = Header ? dyn_cast<ICmpInst>(Header->getCondition()) : nullptr;
    bool Domain = Phi && Phi->getNumIncomingValues() == 2 && Compare &&
        (Compare->getPredicate() == ICmpInst::ICMP_SLT || Compare->getPredicate() == ICmpInst::ICMP_ULT) &&
        Compare->getOperand(0) == Phi;
    if (Domain) {
        bool Zero = false, Increment = false;
        for (unsigned I = 0; I < 2; ++I) {
            Value *V = Phi->getIncomingValue(I);
            if (auto *C = dyn_cast<ConstantInt>(V)) Zero |= C->isZero();
            if (auto *Add = dyn_cast<BinaryOperator>(V))
                if (Add->getOpcode() == Instruction::Add && Add->getOperand(0) == Phi)
                    if (auto *C = dyn_cast<ConstantInt>(Add->getOperand(1))) Increment |= C->isOne();
        }
        bool GraphBound = false, SetBound = false;
        Value *Bound = Compare->getOperand(1);
        if (auto *Load = dyn_cast<LoadInst>(Bound)) {
            auto *G = dyn_cast<GetElementPtrInst>(Load->getPointerOperand());
            GraphBound = G && llvm::all_of(G->indices(), [](const Use &U) {
                auto *C = dyn_cast<ConstantInt>(U.get()); return C && C->isZero();
            }) && sameAddressValue(G->getPointerOperand(), Info.GraphPtr, 0);
        }
        if (auto *Call = dyn_cast<CallInst>(Bound)) {
            Function *F = Call->getCalledFunction();
            auto *Set = Call->arg_size() ? dyn_cast<LoadInst>(Call->getArgOperand(0)) : nullptr;
            SetBound = F && F->getName() == "roaring_bitmap_get_cardinality" && Set &&
                       Set->getPointerOperand() == Info.FrontierSetPtr;
        }
        Value *Endpoint = Info.UVal;
        while (auto *Cast = dyn_cast<CastInst>(Endpoint)) Endpoint = Cast->getOperand(0);
        bool RankSource = false, EndpointIdentity = Endpoint == Phi;
        if (auto *Call = dyn_cast<CallInst>(Endpoint)) {
            Function *F = Call->getCalledFunction();
            if (F && F->getName() == "roaring_bitmap_get_at_index" && Call->arg_size() == 2) {
                auto *Set = dyn_cast<LoadInst>(Call->getArgOperand(0));
                Value *Rank = Call->getArgOperand(1);
                while (auto *Cast = dyn_cast<CastInst>(Rank)) Rank = Cast->getOperand(0);
                RankSource = Set && Set->getPointerOperand() == Info.FrontierSetPtr && Rank == Phi;
                EndpointIdentity = RankSource;
            }
        }
        if (auto *Load = dyn_cast<LoadInst>(Endpoint))
            EndpointIdentity = Load->getPointerOperand() == Info.UAlloca && Info.UAlloca;
        if (Info.UAlloca) for (User *U : Info.UAlloca->users()) if (auto *Store = dyn_cast<StoreInst>(U)) {
            Value *V = Store->getValueOperand();
            while (auto *Cast = dyn_cast<CastInst>(V)) V = Cast->getOperand(0);
            RankSource |= isa<CallInst>(V) && Info.DriverLoop->contains(Store);
        }
        Domain = Zero && Increment && EndpointIdentity && (RankSource ? SetBound : GraphBound);
    }
    if(Info.OrderedArrayFrontier) Domain=arraySourceDomain(Info);
    if (!Domain) {
        P.RuntimeRealizable = false;
        P.RuntimeReason = "source domain has no complete ascending CleanCut coverage proof";
    }
    if (!P.RuntimeRealizable && P.RuntimeReason.empty())
        P.RuntimeReason = "CleanCut callbacks cannot preserve the complete source lifecycle";
    if (!GatesModelled) {
        P.RuntimeRealizable = false;
        P.RuntimeReason = "source guards do not form a dominating short-circuit chain";
    }
    if (hasPerSourceClaim(Info)) {
        if (!ClaimStaged) {
            P.RuntimeRealizable = false;
            P.RuntimeReason = "source claim has no checked once-only result and guard frame";
        }
        auto Claims = perSourceClaims(Info);
        for (const auto &Gate : P.SourceGates) {
            bool Covered = false;
            for (const Effect *Claim : Claims) {
                if (auto *Store = dyn_cast<StoreInst>(Claim->Origin)) {
                    auto *Guard = guardICmpForStore(Store);
                    if (Guard == Gate.first)
                        Covered |= (Guard->getPredicate() == ICmpInst::ICMP_EQ) == Gate.second;
                } else if (auto *CAS = dyn_cast<AtomicCmpXchgInst>(Claim->Origin)) {
                    auto *Result = dyn_cast<ExtractValueInst>(Gate.first);
                    if (Result && Result->getAggregateOperand() == CAS &&
                        Result->getNumIndices() == 1 && Result->getIndices()[0] == 1)
                        Covered |= Gate.second;
                }
            }
            if (!Covered) {
                P.RuntimeRealizable = false;
                P.RuntimeReason = "compound source guard has no verified claim-result staging";
            }
        }
    }
    /* The existing cloner does not capture arbitrary function arguments. */
    for (const auto &A : P.Accesses) {
        SmallVector<Value *, 16> Work;
        if (A.Pointer) Work.push_back(A.Pointer);
        if (A.Origin) for (Value *Operand : A.Origin->operands()) Work.push_back(Operand);
        SmallPtrSet<Value *, 32> Seen;
        while (!Work.empty()) {
            Value *V = Work.pop_back_val();
            if (!Seen.insert(V).second) continue;
            if (isa<Argument>(V)) {
                P.RuntimeRealizable = false;
                P.RuntimeReason = "pair callback needs an external argument frame";
            } else if (auto *I = dyn_cast<Instruction>(V))
                for (Value *Operand : I->operands()) Work.push_back(Operand);
        }
    }
    for (const auto &Snapshot : Info.RoundSepBases) {
        Type *Element = nullptr;
        Value *Count = nullptr, *Storage = nullptr;
        bool Available = arrayBaseInfo(Snapshot.Base, Element, Count, Storage);
        if (auto *I = dyn_cast_or_null<Instruction>(Count))
            Available &= Info.DriverLoop->getLoopPreheader() &&
                DT.dominates(I, Info.DriverLoop->getLoopPreheader()->getTerminator());
        if (!Available) {
            P.RuntimeRealizable = false;
            P.RuntimeReason = "snapshot allocation extent is unavailable before dispatch";
        }
    }
    if (Info.FrontierArrayPtr && !Info.OrderedArrayFrontier) {
        P.RuntimeRealizable = false;
        P.RuntimeReason = "array source order and multiplicity have no CleanCut coverage proof";
    }
    for (const auto &R : P.Relations) if (R.Survives && R.Kind != AccessRelKind::WAW) {
        const auto &A = P.Accesses[R.From], &B = P.Accesses[R.To];
        const auto &Read = A.Write ? B : A;
        if (Read.Phase != PhaseSegment::Preamble || Read.Write) continue;
        if (Info.AccConsumeStore && Read.Base == Info.ReducePtr) continue;
        bool ClaimRead = false;
        for (const Effect *Claim : perSourceClaims(Info)) ClaimRead |= Read.Base == Claim->Base;
        if (!ClaimRead) {
            P.RuntimeRealizable = false;
            P.RuntimeReason = "mutable source guard requires a once-per-source frame value";
        }
    }
    if (Info.AccConsumeStore && Info.UsePrivLayout) {
        P.RuntimeRealizable = false;
        P.RuntimeReason = "private arrays with source accumulator require a separate lifecycle realization";
    }
    for (BasicBlock *BB : Info.NeighborLoop->blocks())
        if (!isa<BranchInst>(BB->getTerminator())) {
            P.RuntimeRealizable = false;
            P.RuntimeReason = "pair callback cannot preserve this terminator";
        }
    const Value *Iterator = getUnderlyingObject(Info.InitCall->getArgOperand(2));
    for (const auto &A : P.Accesses)
        if (A.Pointer && getUnderlyingObject(A.Pointer) == Iterator) {
            P.RuntimeRealizable = false;
            P.RuntimeReason = "explicit iterator state needs a traversal frame";
        }
    std::string ModelReason;
    if (!provesModelable(Info.NeighborLoop, ModelReason)) {
        P.RuntimeRealizable = false; P.RuntimeReason = "pair CFG realization: " + ModelReason;
    }
}
} // namespace

/* Source guards are observable at their original once-per-source scope.
 * Frozen reads use the same invocation snapshot as the complete pair body. */
static Function *emitSpatialSourceGate(const NeighborLoopInfo &Info, Module *Mod,
                                        LLVMContext &Ctx) {
    Type *Ptr = PointerType::get(Ctx, 0), *I32 = Type::getInt32Ty(Ctx);
    Function *Fn = Function::Create(FunctionType::get(Type::getVoidTy(Ctx),
        {Ptr, Ptr, I32}, false), GlobalValue::InternalLinkage, "sgpl_source_gate", Mod);
    IRBuilder<> B(BasicBlock::Create(Ctx, "entry", Fn));
    SmallDenseMap<Value *, Value *> Map;
    Value *U = Fn->getArg(2), *Runtime = Fn->getArg(1);
    auto Seed = [&](Value *Original) {
        if (!Original || !Original->getType()->isIntegerTy()) return;
        Map[Original] = B.CreateZExtOrTrunc(U, Original->getType());
    };
    Seed(Info.UVal);
    Value *Ordinal=U;
    if(Info.OrderedArrayFrontier) {
        Ordinal=B.CreateTrunc(B.CreateCall(Mod->getOrInsertFunction("autograph_exec_ctx_occurrence",
            FunctionType::get(B.getInt64Ty(),{Ptr},false)),{Runtime}),I32);
        if(Value *IV=driverIndVar(Info.DriverLoop))Map[IV]=B.CreateZExtOrTrunc(Ordinal,IV->getType());
    } else Seed(driverIndVar(Info.DriverLoop));
    for (Value *V = Info.UVal; V && isa<CastInst>(V);) {
        V = cast<CastInst>(V)->getOperand(0); Seed(V);
    }
    for (const auto &A : Info.SpatialResult.Accesses)
        if (auto *L = dyn_cast<LoadInst>(A.Origin))
            if (L->getPointerOperand() == Info.UAlloca) Seed(L);
    /* Escaping u-slot loads are structural accesses, so scan them separately. */
    for (BasicBlock *BB : Info.DriverLoop->blocks())
        for (Instruction &I : *BB) if (auto *L = dyn_cast<LoadInst>(&I))
            if (L->getPointerOperand() == Info.UAlloca) Seed(L);
    if (Info.CombinedFields) {
        Value *Frame = B.CreateCall(Mod->getOrInsertFunction("sgpl_round_spatial_current_frame",
            FunctionType::get(Ptr, false)));
        for (const auto &Field : *Info.CombinedFields) {
            if (Map.count(const_cast<Value *>(Field.first))) continue;
            Value *Address = B.CreateStructGEP(Info.CombinedFrame, Frame, Field.second);
            Map[const_cast<Value *>(Field.first)] = (Info.CombinedPrivate->count(Field.first) && !Info.CombinedIndirectPrivate->count(Field.first)) ? Address
                : B.CreateLoad(Info.CombinedFrame->getElementType(Field.second), Address);
        }
    }
    bool Ok = true;
    FunctionCallee Snapshot = Mod->getOrInsertFunction("autograph_exec_ctx_snapshot",
        FunctionType::get(Ptr, {Ptr, I32}, false));
    std::function<Value *(Value *, unsigned)> Clone = [&](Value *V, unsigned Depth) -> Value * {
        if (!V || Depth > 32) { Ok = false; return nullptr; }
        if (Map.count(V)) return Map[V];
        if (isa<Constant>(V)) return V;
        auto *I = dyn_cast<Instruction>(V);
        if (!I) { Ok = false; return nullptr; }
        if (auto *Load = dyn_cast<LoadInst>(I)) {
            if (Info.AccConsumeStore && Load->getPointerOperand() == Info.ReducePtr) {
                FunctionCallee Partial = Mod->getOrInsertFunction("autograph_exec_partition_state",
                    FunctionType::get(Ptr, {Ptr}, false));
                return Map[V] = B.CreateLoad(Load->getType(), B.CreateCall(Partial, {Runtime}));
            }
            for (unsigned Slot = 0; Slot < Info.RoundSepBases.size(); ++Slot) {
                const auto &S = Info.RoundSepBases[Slot];
                if (!S.CrossReads.count(Load)) continue;
                auto *G = dyn_cast<GetElementPtrInst>(Load->getPointerOperand());
                if (!G) { Ok = false; return nullptr; }
                SmallVector<Value *, 4> Indices;
                for (const Use &Index : G->indices()) {
                    if (Indices.empty() && G->getNumIndices() > 1 &&
                        isa<ConstantInt>(Index.get()) && cast<ConstantInt>(Index.get())->isZero()) continue;
                    Value *M = Clone(Index.get(), Depth + 1);
                    if (!Ok || !M) return nullptr;
                    Indices.push_back(M);
                }
                Value *Old = B.CreateCall(Snapshot, {Runtime, B.getInt32(Slot)});
                return Map[V] = B.CreateLoad(S.ElemTy, B.CreateGEP(S.ElemTy, Old, Indices));
            }
        }
        if (auto *Call = dyn_cast<CallInst>(I)) {
            Function *F = Call->getCalledFunction();
            if (Info.MembershipGated && F && F->getName() == "roaring_bitmap_contains") {
                auto *L = dyn_cast<LoadInst>(Call->getArgOperand(0));
                if (L && L->getPointerOperand() == Info.FrontierSetPtr)
                    return Map[V] = ConstantInt::get(Call->getType(), 1);
            }
        }
        if (!isa<LoadInst>(I) && !isa<BinaryOperator>(I) && !isa<GetElementPtrInst>(I) &&
            !isa<CastInst>(I) && !isa<CmpInst>(I) && !isa<SelectInst>(I)) {
            Ok = false; return nullptr;
        }
        SmallVector<Value *, 8> Operands;
        for (Value *Operand : I->operands()) {
            Value *M = Clone(Operand, Depth + 1);
            if (!Ok || !M) return nullptr;
            Operands.push_back(M);
        }
        Instruction *Copy = I->clone();
        for (unsigned N = 0; N < Operands.size(); ++N) Copy->setOperand(N, Operands[N]);
        B.Insert(Copy);
        return Map[V] = Copy;
    };
    BasicBlock *Denied = BasicBlock::Create(Ctx, "denied", Fn);
    BasicBlock *Allowed = BasicBlock::Create(Ctx, "allowed", Fn);
    FunctionCallee Store = Mod->getOrInsertFunction("autograph_exec_ctx_set_source_gate",
        FunctionType::get(Type::getVoidTy(Ctx), {Ptr, I32, I32}, false));
    if (hasPerSourceClaim(Info)) {
        auto Claims = perSourceClaims(Info);
        FunctionCallee Get = Mod->getOrInsertFunction("autograph_exec_ctx_claims",
            FunctionType::get(Ptr, {Ptr}, false));
        Value *Array = B.CreateCall(Get, {Runtime});
        for (unsigned I = 0; I < Claims.size(); ++I) if (claimGatesLoop(Info, Claims[I])) {
            Value *Index = B.CreateAdd(B.CreateMul(Ordinal, B.getInt32(Claims.size())), B.getInt32(I));
            Value *Won = B.CreateLoad(I32, B.CreateGEP(I32, Array, Index));
            BasicBlock *Next = BasicBlock::Create(Ctx, "claim.passed", Fn);
            B.CreateCondBr(B.CreateICmpNE(Won, B.getInt32(0)), Next, Denied);
            B.SetInsertPoint(Next);
        }
    } else for (const auto &Gate : Info.SpatialResult.SourceGates) {
        Value *Condition = Clone(Gate.first, 0);
        if (!Ok || !Condition || !Condition->getType()->isIntegerTy(1)) {
            Fn->eraseFromParent(); return nullptr;
        }
        if (!Gate.second) Condition = B.CreateNot(Condition);
        BasicBlock *Next = BasicBlock::Create(Ctx, "guard.passed", Fn);
        B.CreateCondBr(Condition, Next, Denied);
        B.SetInsertPoint(Next);
    }
    B.CreateBr(Allowed);
    B.SetInsertPoint(Denied);
    B.CreateCall(Store, {Runtime, U, B.getInt32(0)});
    B.CreateRetVoid();
    B.SetInsertPoint(Allowed);
    B.CreateCall(Store, {Runtime, U, B.getInt32(1)});
    B.CreateRetVoid();
    return Fn;
}
