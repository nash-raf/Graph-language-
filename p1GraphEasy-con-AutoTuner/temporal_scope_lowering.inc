/* CPU temporal realization. Keep the original CFG inside each component.
 * The proof covers instructions, including instructions outside the effect
 * algebra. No spatial ownership premise is needed: sources and component
 * interiors remain serial. Unknown dependencies fail the proof closed. */
namespace {
struct TemporalAccess {
    Instruction *Origin;
    Value *Pointer;
    bool Write;
    unsigned Phase;
};

static bool temporalInstrumentation(const Instruction &I) {
    const auto *C = dyn_cast<CallBase>(&I);
    const Function *F = C ? C->getCalledFunction() : nullptr;
    return F && (F->getName().starts_with("autograph_profile_") ||
                 F->getName().starts_with("llvm.lifetime."));
}

static bool temporalObservable(const Instruction &I) {
    if (temporalInstrumentation(I)) return false;
    if (const auto *C = dyn_cast<CallBase>(&I)) {
        if (isIteratorInit(dyn_cast<CallInst>(C)) ||
            isIteratorNext(dyn_cast<CallInst>(C))) return false;
    }
    return isa<AllocaInst>(I) || (I.mayHaveSideEffects() && !I.isTerminator());
}

/* Array slots refer to their allocation, not to an invented ownership region.
 * Resolve a slot only when every publication names the same allocation. */
static const Value *temporalObject(Value *P) {
    const Value *O = getUnderlyingObject(P);
    if (const auto *L = dyn_cast<LoadInst>(O)) {
        const Value *Slot = getUnderlyingObject(L->getPointerOperand());
        const Value *Allocation = nullptr;
        bool Valid = true;
        for (const User *U : Slot->users()) {
            if (const auto *S = dyn_cast<StoreInst>(U)) {
                if (S->getPointerOperand() != Slot) { Valid = false; break; }
                const Value *V = getUnderlyingObject(S->getValueOperand());
                const auto *Call = dyn_cast<CallBase>(V);
                const Function *Callee = Call ? Call->getCalledFunction() : nullptr;
                bool GraphAllocation = Callee && Callee->getName().starts_with("load_graph_from_file");
                if ((!isa<AllocaInst>(V) && !GraphAllocation) || (Allocation && Allocation != V)) {
                    Valid = false; break;
                }
                Allocation = V;
            }
        }
        if (Valid && Allocation) return Allocation;
        return O; /* unresolved loaded pointers can still alias */
    }
    return O;
}

static bool temporalMayAlias(const TemporalAccess &A, const TemporalAccess &B,
                             AAResults &AA, bool &Uncertain) {
    if (!A.Pointer || !B.Pointer) { Uncertain = true; return true; }
    const Value *OA = temporalObject(A.Pointer), *OB = temporalObject(B.Pointer);
    auto Identified = [](const Value *V) {
        if (isIdentifiedObject(V)) return true;
        const auto *Call = dyn_cast<CallBase>(V);
        const Function *F = Call ? Call->getCalledFunction() : nullptr;
        return F && F->getName().starts_with("load_graph_from_file");
    };
    if (OA != OB && Identified(OA) && Identified(OB))
        return false;
    auto Location = [](const TemporalAccess &X) {
        if (X.Origin)
            if (auto L = MemoryLocation::getOrNone(X.Origin)) return *L;
        return MemoryLocation::getBeforeOrAfter(X.Pointer);
    };
    if (AA.alias(Location(A), Location(B)) == AliasResult::NoAlias)
        return false;
    if (A.Pointer == B.Pointer || sameAddressValue(A.Pointer, B.Pointer, 0)) return true;
    Uncertain = true;
    return true;
}

static bool collectTemporalAccesses(ArrayRef<BasicBlock *> Blocks,
                                    const NeighborLoopInfo &Info,
                                    SmallVectorImpl<TemporalAccess> &Accesses,
                                    SmallVectorImpl<Instruction *> &Opaque) {
    for (BasicBlock *BB : Blocks) for (Instruction &I : *BB) {
        unsigned P = unsigned(sourcePhaseFor(Info, &I)) - 1;
        if (temporalInstrumentation(I)) continue;
        if (auto *L = dyn_cast<LoadInst>(&I)) {
            if (L->isVolatile() || L->isAtomic()) Opaque.push_back(&I);
            Accesses.push_back({&I, L->getPointerOperand(), false, P});
        } else if (auto *S = dyn_cast<StoreInst>(&I)) {
            if (S->isVolatile() || S->isAtomic()) Opaque.push_back(&I);
            Accesses.push_back({&I, S->getPointerOperand(), true, P});
        } else if (auto *C = dyn_cast<AtomicCmpXchgInst>(&I)) {
            Accesses.push_back({&I, C->getPointerOperand(), true, P});
        } else if (auto *R = dyn_cast<AtomicRMWInst>(&I)) {
            Accesses.push_back({&I, R->getPointerOperand(), true, P});
        } else if (auto *C = dyn_cast<CallInst>(&I)) {
            if (isIteratorInit(C)) {
                Accesses.push_back({&I, C->getArgOperand(2), true, P});
                Accesses.push_back({&I, C->getArgOperand(0), false, P});
            } else if (isIteratorNext(C)) {
                Accesses.push_back({&I, C->getArgOperand(0), true, P});
                Accesses.push_back({&I, C->getArgOperand(1), true, P});
            } else if (auto *M = dyn_cast<MemIntrinsic>(C)) {
                Accesses.push_back({&I, M->getRawDest(), true, P});
                if (auto *T = dyn_cast<MemTransferInst>(M))
                    Accesses.push_back({&I, T->getRawSource(), false, P});
            } else if (!C->doesNotAccessMemory() || C->mayHaveSideEffects()) Opaque.push_back(&I);
        } else if (I.mayReadOrWriteMemory()) Opaque.push_back(&I);
    }
    return Opaque.empty();
}

struct TemporalBody {
    Loop *Unit = nullptr;
    BasicBlock *Entry = nullptr, *Latch = nullptr;
    SmallVector<BasicBlock *, 32> Blocks;
};

static bool temporalBody(Loop *L, TemporalBody &B, std::string &Reason) {
    B.Unit = L;
    B.Latch = L->getLoopLatch();
    auto *Br = dyn_cast<BranchInst>(L->getHeader()->getTerminator());
    if (!B.Latch || !Br || !Br->isConditional() ||
        !L->contains(Br->getSuccessor(0)) || L->contains(Br->getSuccessor(1))) {
        Reason = "unit requires one header, one latch, and one exit"; return false;
    }
    B.Entry = Br->getSuccessor(0);
    for (BasicBlock *BB : L->blocks()) if (BB != L->getHeader()) {
        B.Blocks.push_back(BB);
        for (BasicBlock *S : successors(BB))
            if (S == L->getHeader()) {
                if (BB != B.Latch) { Reason = "multiple iteration exits"; return false; }
            } else if (!L->contains(S)) {
                Reason = "body has an early exit"; return false;
            }
        for (Instruction &I : *BB)
            for (User *U : I.users()) if (auto *Use = dyn_cast<Instruction>(U))
                if (!L->contains(Use)) {
                    Reason = "unit has an SSA output without a frame slot"; return false;
                }
    }
    return true;
}

static const char *temporalStatusName(TemporalProofStatus S) {
    switch (S) {
    case TemporalProofStatus::NotApplicable: return "not-applicable";
    case TemporalProofStatus::Skipped: return "skipped";
    case TemporalProofStatus::Independent: return "independent";
    case TemporalProofStatus::Interfering: return "interfering";
    case TemporalProofStatus::Inconclusive: return "inconclusive";
    }
    llvm_unreachable("temporal proof status");
}

static void temporalPrint(const NeighborLoopInfo &Info) {
    if (!getenv("GRAPH_FRONTIER_STATS") && !getenv("SGPL_WITNESS_DUMP")) return;
    const auto &T = Info.TemporalResult;
    bool KnownNoRounds = T.Rounds.Status == TemporalProofStatus::NotApplicable ||
                         T.Rounds.Status == TemporalProofStatus::Interfering;
    const char *Verdict = T.Realized ? "independent" :
        KnownNoRounds && T.Phases.Status == TemporalProofStatus::Interfering ? "interfering" : "inconclusive";
    errs() << "[temporal] scope=" << (T.Scope == TemporalScope::Round ? "Round" :
                                      T.Scope == TemporalScope::SourcePhase ? "SourcePhase" : "None")
           << " rounds=" << temporalStatusName(T.Rounds.Status)
           << " phases=" << temporalStatusName(T.Phases.Status)
           << " components=" << (T.Scope == TemporalScope::Round ? T.Rounds.Components : T.Phases.Components)
           << " realized=" << T.Realized << " verdict=" << Verdict << "\n";
    for (const auto *P : {&T.Rounds, &T.Phases}) {
        errs() << "[temporal] " << P->Reason << " members=";
        for (unsigned M : P->Members) errs() << M << ",";
        errs() << "\n";
        if (!P->RuntimeReason.empty()) errs() << "[temporal-runtime] " << P->RuntimeReason << "\n";
        if (getenv("SGPL_WITNESS_DUMP")) for (const auto &D : P->Relations)
            errs() << "[temporal-relation] " << D.From << "->" << D.To
                   << " basis=" << D.Basis << " uncertain=" << D.Uncertain
                   << " uniform=" << D.UniformAcrossRounds << "\n";
    }
}

static void proveSourcePhases(NeighborLoopInfo &Info, const TemporalBody &Body,
                             AAResults &AA) {
    auto &Proof = Info.TemporalResult.Phases;
    bool Nonempty[3] = {};
    unsigned Parent[3] = {0, 1, 2};
    SmallPtrSet<Instruction *, 32> Meaningful;
    auto Event = [&](Instruction *I, bool NeighborOutput = false) {
        unsigned P = unsigned(sourcePhaseFor(Info, I)) - 1;
        Nonempty[P] = true;
        if (Meaningful.insert(I).second) Proof.Meaningful.push_back({I, NeighborOutput});
    };
    auto Root = [&](unsigned A) { while (Parent[A] != A) A = Parent[A]; return A; };
    auto Edge = [&](unsigned A, unsigned B, StringRef Basis, bool Unknown = false) {
        if (A == B) return;
        Parent[Root(B)] = Root(A);
        Proof.Relations.push_back({A, B, Basis.str(), Unknown});
    };
    SmallVector<TemporalAccess, 64> Accesses;
    SmallVector<Instruction *, 8> Opaque;
    collectTemporalAccesses(Body.Blocks, Info, Accesses, Opaque);
    PostDominatorTree PDT(*Body.Entry->getParent());
    for (BasicBlock *BB : Body.Blocks) for (Instruction &I : *BB)
        if (temporalObservable(I)) Event(&I);
    for (const auto &A : Accesses) if (!A.Write && A.Phase != 1 &&
        (getUnderlyingObject(A.Pointer) == Info.VAlloca ||
         getUnderlyingObject(A.Pointer) == Info.InitCall->getArgOperand(2)))
        Nonempty[1] = true; /* traversal state is observed by a source phase */
    DominatorTree SourceDT(*Body.Entry->getParent());
    auto OutsideRead = [&](Value *Slot) {
        for (BasicBlock &BB : *Body.Entry->getParent()) for (Instruction &I : BB)
            if (auto *Load = dyn_cast<LoadInst>(&I))
                if (temporalObject(Load->getPointerOperand()) == getUnderlyingObject(Slot) &&
                    !Body.Unit->contains(Load) &&
                    (!Body.Unit->getLoopPreheader() ||
                     !SourceDT.dominates(Load, Body.Unit->getLoopPreheader()->getTerminator()))) return true;
        return false;
    };
    for (BasicBlock *BB : Body.Blocks) for (Instruction &I : *BB)
        if (auto *Call = dyn_cast<CallInst>(&I)) {
            if (isIteratorNext(Call) && OutsideRead(Call->getArgOperand(1))) Event(Call, true);
            if (isIteratorInit(Call) && OutsideRead(Call->getArgOperand(2))) Event(Call);
        }
    for (unsigned A = 0; A < Accesses.size(); ++A)
        for (unsigned B = A + 1; B < Accesses.size(); ++B) {
            const auto &X = Accesses[A], &Y = Accesses[B];
            if (X.Phase == Y.Phase || (!X.Write && !Y.Write)) continue;
            bool Unknown = false;
            if (temporalMayAlias(X, Y, AA, Unknown))
                Edge(X.Phase, Y.Phase, X.Write ? (Y.Write ? "WAW" : "RAW") : "WAR", Unknown);
        }
    /* SSA values and source guards bind their producing and consuming phases.
     * Address arithmetic and the source induction value are common inputs.
     * Propagate each non-address load or claim through its pure users. */
    for (BasicBlock *BB : Body.Blocks) for (Instruction &I : *BB) {
        if (!(isa<LoadInst>(I) && !I.getType()->isPointerTy()) &&
            !isa<AtomicCmpXchgInst>(I) && !isa<AtomicRMWInst>(I) && !isa<AllocaInst>(I) &&
            !(isa<CallInst>(I) && temporalObservable(I))) continue;
        if (isIteratorNext(dyn_cast<CallInst>(&I))) continue;
        unsigned Producer = unsigned(sourcePhaseFor(Info, &I)) - 1;
        SmallVector<User *, 16> Work(I.users());
        SmallPtrSet<User *, 32> Seen;
        while (!Work.empty()) {
            User *U = Work.pop_back_val();
            auto *Use = dyn_cast<Instruction>(U);
            if (!Use || !Body.Unit->contains(Use) || !Seen.insert(U).second) continue;
            if (auto *Br = dyn_cast<BranchInst>(Use)) {
                if (!Br->isConditional() || Br == Info.NeighborLoop->getHeader()->getTerminator()) continue;
                auto *Node = PDT.getNode(Br->getParent());
                BasicBlock *Join = Node && Node->getIDom() ? Node->getIDom()->getBlock() : nullptr;
                SmallVector<BasicBlock *, 16> Controlled(successors(Br->getParent()));
                SmallPtrSet<BasicBlock *, 32> Visited;
                while (!Controlled.empty()) {
                    BasicBlock *Block = Controlled.pop_back_val();
                    if (Block == Join || Block == Body.Unit->getHeader() ||
                        !Body.Unit->contains(Block) || !Visited.insert(Block).second) continue;
                    for (Instruction &Effect : *Block) if (temporalObservable(Effect)) {
                        Event(&I);
                        Edge(Producer, unsigned(sourcePhaseFor(Info, &Effect)) - 1, "guard");
                    }
                    for (BasicBlock *S : successors(Block)) Controlled.push_back(S);
                }
            } else if (isIteratorInit(dyn_cast<CallInst>(Use))) {
                Event(&I);
                Edge(Producer, 1, "traversal");
            } else if (temporalObservable(*Use)) {
                Event(&I);
                Edge(Producer, unsigned(sourcePhaseFor(Info, Use)) - 1, "value");
            } else for (User *Next : Use->users()) Work.push_back(Next);
        }
    }
    for (Instruction *I : Opaque) {
        unsigned P = unsigned(sourcePhaseFor(Info, I)) - 1;
        for (unsigned Q = 0; Q != 3; ++Q) if (Nonempty[Q]) Edge(P, Q, "shared-resource", true);
    }
    unsigned Label[3] = {~0u, ~0u, ~0u};
    for (unsigned P = 0; P != 3; ++P) {
        if (!Nonempty[P]) { Proof.Members.push_back(~0u); continue; }
        unsigned R = Root(P);
        if (Label[R] == ~0u) Label[R] = Proof.Components++;
        Proof.Members.push_back(Label[R]);
    }
    bool Unknown = !Opaque.empty();
    for (const auto &D : Proof.Relations) Unknown |= D.Uncertain;
    Proof.Status = Proof.Components >= 2 ? TemporalProofStatus::Independent :
        Unknown ? TemporalProofStatus::Inconclusive : TemporalProofStatus::Interfering;
    Proof.Reason = Proof.Components >= 2 ? "nonempty source phases have separate components" :
        Unknown ? "source phase dependencies are incomplete" : "source phases have no non-trivial partition";
}

struct TemporalClone {
    Function *Fn = nullptr;
    ValueToValueMapTy Map;
    BasicBlock *Entry = nullptr;
    SmallVector<std::pair<WeakTrackingVH, bool>, 32> Meaningful;
};

/* Clone one complete iteration. Phase callbacks retain the original control
 * flow, but bypass the neighbor loop when their component excludes Pair.
 * Remove only effects outside the component. Dependencies keep their value
 * producers in the same component. Dead pure instructions disappear later. */
static bool cloneTemporalBody(const TemporalBody &Body, NeighborLoopInfo &Info,
                              int Component, TemporalClone &Out,
                              ArrayRef<const Value *> Private = {},
                              const Value *Counter = nullptr) {
    Function *F = Body.Entry->getParent();
    auto &C = F->getContext();
    Module *M = F->getParent();
    Out.Fn = Function::Create(FunctionType::get(Type::getInt32Ty(C),
                               {PointerType::get(C, 0), Type::getInt64Ty(C)}, false),
                               GlobalValue::InternalLinkage, "sgpl.temporal.work", M);
    Out.Fn->addFnAttr("sgpl.temporal.callback");
    Out.Fn->addFnAttr("sgpl.temporal.scope", Component < 0 ? "Round" : "SourcePhase");
    Out.Entry = BasicBlock::Create(C, "frame", Out.Fn);
    IRBuilder<> EB(Out.Entry);
    for (const Value *V : Private) {
        if (const auto *A = dyn_cast<AllocaInst>(V)) {
            if (!isa<ConstantInt>(A->getArraySize())) return false;
            Value *Slot = EB.CreateAlloca(A->getAllocatedType(), const_cast<Value *>(A->getArraySize()), "round.scratch");
            Out.Map[V] = Slot;
            if (V == Counter) EB.CreateStore(EB.CreateIntCast(Out.Fn->getArg(1), A->getAllocatedType(), true), Slot);
        } else if (const auto *G = dyn_cast<GlobalVariable>(V)) {
            Value *Slot = EB.CreateAlloca(G->getValueType(), nullptr, "round.scalar");
            Out.Map[V] = Slot;
            if (V == Counter)
                EB.CreateStore(EB.CreateIntCast(Out.Fn->getArg(1), G->getValueType(), true), Slot);
        } else return false;
    }
    for (BasicBlock *BB : Body.Blocks)
        CloneBasicBlock(BB, Out.Map, ".temporal", Out.Fn);
    /* CloneBasicBlock does not publish its block mapping. */
    auto BI = std::next(Out.Fn->begin());
    for (BasicBlock *BB : Body.Blocks) Out.Map[BB] = &*BI++;
    if (Component >= 0)
        for (const auto &Event : Info.TemporalResult.Phases.Meaningful)
            if (Info.TemporalResult.Phases.Members[unsigned(sourcePhaseFor(Info, Event.first)) - 1] == unsigned(Component))
                Out.Meaningful.push_back({WeakTrackingVH(Out.Map.lookup(Event.first)), Event.second});
    EB.CreateBr(cast<BasicBlock>(Out.Map[Body.Entry]));
    for (auto &BB : *Out.Fn) if (&BB != Out.Entry)
        for (Instruction &I : BB)
            RemapInstruction(&I, Out.Map, RF_IgnoreMissingLocals);
    BasicBlock *Done = BasicBlock::Create(C, "iteration.done", Out.Fn);
    IRBuilder<>(Done).CreateRet(ConstantInt::get(Type::getInt32Ty(C), 0));
    /* The enclosing round latch can also be the source loop header. Preserve
     * its source branch and replace only the edge to the enclosing header. */
    for (BasicBlock *BB : Body.Blocks) {
        Instruction *T = cast<BasicBlock>(Out.Map[BB])->getTerminator();
        for (unsigned N = 0; N < T->getNumSuccessors(); ++N)
            if (T->getSuccessor(N) == Body.Unit->getHeader()) T->setSuccessor(N, Done);
    }
    if (Component >= 0 && Info.TemporalResult.Phases.Members[1] != unsigned(Component)) {
        BasicBlock *Exit = Info.NeighborLoop->getUniqueExitBlock();
        if (!Exit || !Out.Map.count(Exit)) return false;
        BasicBlock *Header = cast<BasicBlock>(Out.Map[Info.NeighborLoop->getHeader()]);
        for (BasicBlock &BB : *Out.Fn) if (auto *Br = dyn_cast<BranchInst>(BB.getTerminator()))
            for (unsigned S = 0; S < Br->getNumSuccessors(); ++S)
                if (Br->getSuccessor(S) == Header)
                    Br->setSuccessor(S, cast<BasicBlock>(Out.Map[Exit]));
        /* Exit PHIs are output dependencies. A phase-only clone cannot invent
         * a neighbor result. The shape check rejects such phase splitting. */
        if (!Exit->phis().empty()) return false;
    }
    if (Component >= 0) for (Loop *Sub : Body.Unit->getSubLoops()) {
        if (Sub == Info.NeighborLoop) continue;
        unsigned P = unsigned(sourcePhaseFor(Info, Sub->getHeader()->getTerminator())) - 1;
        if (Info.TemporalResult.Phases.Members[P] == unsigned(Component)) continue;
        BasicBlock *Exit = Sub->getUniqueExitBlock();
        if (!Exit || !Exit->phis().empty()) return false;
        BasicBlock *Header = cast<BasicBlock>(Out.Map[Sub->getHeader()]);
        for (BasicBlock *BB : Body.Blocks) if (!Sub->contains(BB)) {
            auto *T = cast<BasicBlock>(Out.Map[BB])->getTerminator();
            for (unsigned N = 0; N < T->getNumSuccessors(); ++N)
                if (T->getSuccessor(N) == Header) T->setSuccessor(N, cast<BasicBlock>(Out.Map[Exit]));
        }
    }
    for (BasicBlock *BB : Body.Blocks) for (Instruction &I : *BB) {
        auto *Copy = dyn_cast_or_null<Instruction>(Out.Map.lookup(&I));
        if (!Copy || Copy->isTerminator()) continue;
        bool Remove = temporalInstrumentation(I);
        if (Component >= 0 && temporalObservable(I))
            Remove |= Info.TemporalResult.Phases.Members[unsigned(sourcePhaseFor(Info, &I)) - 1] != unsigned(Component);
        if (Component >= 0 && isIteratorInit(dyn_cast<CallInst>(&I)))
            Remove |= Info.TemporalResult.Phases.Members[unsigned(sourcePhaseFor(Info, &I)) - 1] != unsigned(Component);
        if (Remove) {
            if (!Copy->getType()->isVoidTy()) Copy->replaceAllUsesWith(PoisonValue::get(Copy->getType()));
            Copy->eraseFromParent();
        }
    }
    removeUnreachableBlocks(*Out.Fn);
    bool Changed = true;
    TargetTransformInfo TTI(M->getDataLayout());
    while (Changed) {
        Changed = false;
        for (BasicBlock &BB : *Out.Fn)
            for (auto I = BB.begin(); I != BB.end();) {
                Instruction *V = &*I++;
                if (isInstructionTriviallyDead(V)) { V->eraseFromParent(); Changed = true; }
            }
        /* Remove empty guard branches. A component must not retain reads from
         * an excluded guard merely because its empty CFG still branches. */
        SmallVector<WeakTrackingVH, 32> Blocks;
        for (BasicBlock &BB : *Out.Fn) Blocks.emplace_back(&BB);
        for (auto &Handle : Blocks) {
            auto *Block = dyn_cast_or_null<BasicBlock>(Handle);
            if (Block && Block != Out.Entry) Changed |= simplifyCFG(Block, TTI);
        }
    }
    for (BasicBlock &BB : *Out.Fn) if (auto *T = BB.getTerminator()) {
        T->setMetadata("autotuner.traverse", nullptr);
        T->setMetadata("sgpl.frontier.nested.sequential", MDNode::get(C, MDString::get(C, "sequential")));
    }
    return true;
}

static bool temporalClonesDisjoint(ArrayRef<TemporalClone *> Clones,
                                   const NeighborLoopInfo &Info) {
    SmallVector<TemporalAccess, 64> Accesses;
    for (unsigned N = 0; N < Clones.size(); ++N) {
        SmallVector<BasicBlock *, 32> Blocks;
        for (BasicBlock &BB : *Clones[N]->Fn) Blocks.push_back(&BB);
        SmallVector<TemporalAccess, 32> Local;
        SmallVector<Instruction *, 8> Opaque;
        collectTemporalAccesses(Blocks, Info, Local, Opaque);
        if (!Opaque.empty()) return false;
        for (auto A : Local) { A.Phase = N; Accesses.push_back(A); }
    }
    /* Check the emitted accesses as well as the original proof. In particular,
     * guard loads must not become shared reads of another component's writes.
     * This conservative check does not depend on analysis of a different F. */
    for (unsigned A = 0; A < Accesses.size(); ++A)
        for (unsigned B = A + 1; B < Accesses.size(); ++B) {
            const auto &X = Accesses[A], &Y = Accesses[B];
            if (X.Phase == Y.Phase || (!X.Write && !Y.Write)) continue;
            const Value *OX = temporalObject(X.Pointer), *OY = temporalObject(Y.Pointer);
            if (OX != OY && isIdentifiedObject(OX) && isIdentifiedObject(OY)) continue;
            auto Graph = [](const Value *V) {
                const auto *Call = dyn_cast<CallBase>(V);
                const Function *F = Call ? Call->getCalledFunction() : nullptr;
                return F && F->getName().starts_with("load_graph_from_file");
            };
            if (OX != OY && ((Graph(OX) && isIdentifiedObject(OY)) ||
                             (Graph(OY) && isIdentifiedObject(OX)))) continue;
            return false;
        }
    return true;
}

static StructType *temporalFrame(ArrayRef<TemporalClone *> Clones,
                                 SmallVectorImpl<Value *> &Captures,
                                 StringRef Scope, ArrayRef<Value *> SourceSlots = {}) {
    Function *Original = nullptr;
    SmallPtrSet<Value *, 32> Seen;
    for (TemporalClone *Clone : Clones) for (BasicBlock &BB : *Clone->Fn)
        for (Instruction &I : BB) for (Value *Op : I.operands()) {
            if (auto *V = dyn_cast<Instruction>(Op)) {
                if (V->getFunction() == Clone->Fn) continue;
                Original = V->getFunction();
            } else if (auto *A = dyn_cast<Argument>(Op)) {
                if (A->getParent() == Clone->Fn) continue;
                Original = A->getParent();
            } else continue;
            if (Seen.insert(Op).second) Captures.push_back(Op);
        }
    (void)Original;
    LLVMContext &C = Clones.front()->Fn->getContext();
    SmallVector<Type *, 16> Types;
    for (Value *V : Captures) Types.push_back(V->getType());
    for (Value *Slot : SourceSlots) {
        if (auto *G = dyn_cast<GlobalVariable>(Slot)) Types.push_back(G->getValueType());
        else Types.push_back(cast<AllocaInst>(Slot)->getAllocatedType());
    }
    if (Scope == "SourcePhase") {
        Types.push_back(ArrayType::get(Type::getInt32Ty(C), Clones.size())); /* live component ids */
        Types.push_back(Type::getInt32Ty(C)); /* active sgpl_temporal_scope */
    }
    /* Round wrappers add start/total/count/scope after the common captures. */
    if (Scope == "Round") {
        Types.push_back(Type::getInt64Ty(C)); Types.push_back(Type::getInt64Ty(C));
        Types.push_back(Type::getInt64Ty(C));
        Types.push_back(Type::getInt32Ty(C));
    }
    StructType *Frame = StructType::create(C, Types, ("sgpl.temporal." + Scope + ".frame").str());
    for (TemporalClone *Clone : Clones) {
        IRBuilder<> EB(Clone->Entry, Clone->Entry->getFirstInsertionPt());
        for (unsigned N = 0; N < Captures.size(); ++N) {
            Value *L = EB.CreateLoad(Types[N], EB.CreateStructGEP(Frame, Clone->Fn->getArg(0), N), "capture");
            for (BasicBlock &BB : *Clone->Fn) for (Instruction &I : BB)
                for (Use &U : I.operands()) if (U.get() == Captures[N]) U.set(L);
        }
        for (unsigned N = 0; N < SourceSlots.size(); ++N) {
            Value *Address = EB.CreateStructGEP(Frame, Clone->Fn->getArg(0), Captures.size() + N, "source.accumulator");
            for (BasicBlock &BB : *Clone->Fn) for (Instruction &I : BB)
                for (Use &U : I.operands()) if (U.get() == SourceSlots[N]) U.set(Address);
        }
    }
    return Frame;
}

static CallInst *temporalDispatch(IRBuilder<> &B, Function *Callback,
                                  Value *Frame, Value *Count, Value *Budget,
                                  StringRef Scope) {
    Module *M = B.GetInsertBlock()->getModule();
    LLVMContext &C = M->getContext();
    Type *I32 = Type::getInt32Ty(C), *P = PointerType::get(C, 0);
    auto Fn = M->getOrInsertFunction("sgpl_exec_temporal_components",
                                    FunctionType::get(I32, {P, P, I32, I32}, false));
    auto *Call = B.CreateCall(Fn, {Callback, Frame, Count, Budget});
    Call->setMetadata("sgpl.temporal.scope", MDNode::get(C, MDString::get(C, Scope)));
    return Call;
}

static bool temporalGraphAvailable(const NeighborLoopInfo &Info, const TemporalBody &Body) {
    if (auto *Load = dyn_cast<LoadInst>(Info.GraphPtr))
        return !Load->isVolatile() && !Load->isAtomic() &&
               Body.Unit->isLoopInvariant(Load->getPointerOperand());
    return Body.Unit->isLoopInvariant(Info.GraphPtr);
}

static void temporalFailureBranch(IRBuilder<> &B, Value *Status, BasicBlock *Next);

static Value *temporalPrepare(IRBuilder<> &B, const NeighborLoopInfo &Info) {
    Value *Graph = Info.GraphPtr;
    if (auto *Load = dyn_cast<LoadInst>(Graph))
        Graph = B.CreateLoad(Load->getType(), Load->getPointerOperand(), "temporal.graph");
    Function *F = B.GetInsertBlock()->getParent();
    auto Fn = F->getParent()->getOrInsertFunction("sgpl_temporal_prepare_graph",
                              FunctionType::get(B.getInt32Ty(), {B.getPtrTy()}, false));
    Value *Status = B.CreateCall(Fn, {Graph});
    BasicBlock *Ready = BasicBlock::Create(F->getContext(), "temporal.ready", F);
    temporalFailureBranch(B, Status, Ready);
    B.SetInsertPoint(Ready);
    return Graph;
}

static Value *temporalCpuBudget(IRBuilder<> &B) {
    Module *M = B.GetInsertBlock()->getModule();
    return B.CreateCall(M->getOrInsertFunction("sgpl_temporal_worker_budget",
                        FunctionType::get(B.getInt32Ty(), false)));
}

static Value *temporalBudget(IRBuilder<> &B) {
    if (gpuBackendSelected(B.GetInsertBlock()->getModule())) return B.getInt32(0);
    return temporalCpuBudget(B);
}

static void temporalFailureBranch(IRBuilder<> &B, Value *Status, BasicBlock *Next) {
    Function *F = B.GetInsertBlock()->getParent();
    BasicBlock *Fail = BasicBlock::Create(F->getContext(), "temporal.failure", F);
    B.CreateCondBr(B.CreateICmpEQ(Status, B.getInt32(0)), Next, Fail);
    IRBuilder<> FB(Fail);
    FB.CreateCall(F->getParent()->getOrInsertFunction("sgpl_temporal_dispatch_failed",
                   FunctionType::get(FB.getVoidTy(), {FB.getInt32Ty()}, false)), {Status});
    FB.CreateUnreachable();
}

static void markTemporalScope(Loop *L, StringRef Scope) {
    while (Loop *Parent = L->getParentLoop()) L = Parent;
    SmallVector<Loop *, 16> Work{L};
    while (!Work.empty()) {
        Loop *Unit = Work.pop_back_val();
        Instruction *T = Unit->getHeader()->getTerminator();
        T->setMetadata("sgpl.temporal.scope", MDNode::get(T->getContext(), MDString::get(T->getContext(), Scope)));
        for (Loop *Sub : Unit->getSubLoops()) Work.push_back(Sub);
    }
}

/* Query the actual component CFG up to its first meaningful instruction.
 * User effects are never executed by the query. Iterator discovery uses
 * private storage, so zero-neighbor and guarded paths can return empty. */
static Function *temporalPresence(TemporalClone &Work) {
    ValueToValueMapTy Map;
    Function *Query = CloneFunction(Work.Fn, Map);
    Query->setName("sgpl.temporal.SourcePhase.present");
    Query->addFnAttr("sgpl.temporal.query");
    IRBuilder<> EB(&Query->getEntryBlock(), Query->getEntryBlock().getFirstInsertionPt());
    SmallDenseMap<Value *, Value *> Iterators, Outputs;
    for (BasicBlock &BB : *Query) for (Instruction &I : BB)
        if (auto *Call = dyn_cast<CallInst>(&I)) {
            if (isIteratorInit(Call) || isIteratorNext(Call)) {
                unsigned Arg = isIteratorInit(Call) ? 2 : 0;
                Value *Key = Call->getArgOperand(Arg)->stripPointerCasts();
                if (!Iterators.count(Key)) {
                    auto *Slot = EB.CreateAlloca(ArrayType::get(EB.getInt8Ty(), sizeof(AutoNeighborIter)), nullptr, "query.iterator");
                    Slot->setAlignment(Align(alignof(AutoNeighborIter)));
                    Iterators[Key] = Slot;
                }
                Call->setArgOperand(Arg, Iterators[Key]);
            }
            if (isIteratorNext(Call)) {
                Value *Key = Call->getArgOperand(1)->stripPointerCasts();
                if (!Outputs.count(Key)) Outputs[Key] = EB.CreateAlloca(EB.getInt32Ty(), nullptr, "query.neighbor");
                Call->setArgOperand(1, Outputs[Key]);
            }
        }
    BasicBlock *Present = BasicBlock::Create(Query->getContext(), "present", Query);
    IRBuilder<>(Present).CreateRet(EB.getInt32(1));
    for (const auto &Event : Work.Meaningful) {
        auto *Original = dyn_cast_or_null<Instruction>(Event.first);
        auto *I = Original ? dyn_cast_or_null<Instruction>(Map.lookup(Original)) : nullptr;
        if (!I) continue;
        BasicBlock *Before = I->getParent();
        BasicBlock *After = Before->splitBasicBlock(Event.second ? std::next(I->getIterator()) : I->getIterator(), "query.continue");
        Before->getTerminator()->eraseFromParent();
        IRBuilder<> B(Before);
        if (Event.second) B.CreateCondBr(B.CreateICmpNE(I, B.getInt32(0)), Present, After);
        else B.CreateBr(Present);
    }
    removeUnreachableBlocks(*Query);
    if (verifyFunction(*Query, &errs())) llvm_unreachable("invalid phase presence query");
    return Query;
}

static bool emitSourcePhases(NeighborLoopInfo &Info, const TemporalBody &Body) {
    Function *F = Body.Entry->getParent();
    auto &C = F->getContext();
    auto &Proof = Info.TemporalResult.Phases;
    if (!temporalGraphAvailable(Info, Body)) return false;
    /* The source binder has one canonical increment. Keep it outside dispatch. */
    PHINode *IV = dyn_cast_or_null<PHINode>(driverIndVar(Body.Unit));
    if (!IV || std::distance(Body.Unit->getHeader()->phis().begin(), Body.Unit->getHeader()->phis().end()) != 1)
        return false;
    auto *Step = dyn_cast<BinaryOperator>(IV->getIncomingValueForBlock(Body.Latch));
    if (!Step || Step->getOpcode() != Instruction::Add || Step->getOperand(0) != IV ||
        !isa<ConstantInt>(Step->getOperand(1))) return false;
    if (!Info.NeighborLoop->getUniqueExitBlock() ||
        !Info.NeighborLoop->getUniqueExitBlock()->phis().empty()) return false;
    std::vector<std::unique_ptr<TemporalClone>> Storage;
    SmallVector<TemporalClone *, 3> Clones;
    for (unsigned N = 0; N < Proof.Components; ++N) {
        Storage.emplace_back(new TemporalClone());
        auto *Clone = Storage.back().get();
        if (!cloneTemporalBody(Body, Info, int(N), *Clone)) {
            for (auto &S : Storage) if (S->Fn) S->Fn->eraseFromParent();
            return false;
        }
        Clones.push_back(Clone);
    }
    if (!temporalClonesDisjoint(Clones, Info)) {
        for (auto &S : Storage) S->Fn->eraseFromParent();
        return false;
    }
    SmallVector<Value *, 16> Captures;
    SmallVector<Value *, 2> SourceSlots;
    if (Value *Acc = Info.ReducePtr) {
        Type *Ty = nullptr;
        if (auto *G = dyn_cast<GlobalVariable>(Acc)) Ty = G->getValueType();
        if (auto *A = dyn_cast<AllocaInst>(Acc)) Ty = A->getAllocatedType();
        bool Plain = Ty && (Ty->isIntegerTy() || Ty->isFloatingPointTy());
        for (User *U : Acc->users())
            if (!isa<LoadInst>(U) && !isa<StoreInst>(U)) Plain = false;
        if (Plain) SourceSlots.push_back(Acc);
    }
    StructType *Frame = temporalFrame(Clones, Captures, "SourcePhase", SourceSlots);
    DominatorTree DT(*F);
    for (Value *V : Captures) if (auto *I = dyn_cast<Instruction>(V))
        if (!DT.dominates(I, Body.Unit->getHeader()->getTerminator())) {
            for (auto &S : Storage) S->Fn->eraseFromParent();
            return false;
        }
    SmallVector<Function *, 3> Presence;
    for (auto *Clone : Clones) Presence.push_back(temporalPresence(*Clone));
    Module *M = F->getParent();
    Function *Callback = Function::Create(Clones.front()->Fn->getFunctionType(),
        GlobalValue::InternalLinkage, "sgpl.temporal.SourcePhase.component", M);
    Callback->addFnAttr("sgpl.temporal.callback");
    Callback->addFnAttr("sgpl.temporal.scope", "SourcePhase");
    IRBuilder<> CB(BasicBlock::Create(C, "entry", Callback));
    BasicBlock *Bad = BasicBlock::Create(C, "invalid", Callback);
    unsigned IdField = Captures.size() + SourceSlots.size();
    Type *IdsTy = Frame->getElementType(IdField);
    Value *Ids = CB.CreateStructGEP(Frame, Callback->getArg(0), IdField);
    Value *Member = CB.CreateLoad(CB.getInt32Ty(), CB.CreateGEP(IdsTy, Ids, {CB.getInt32(0), Callback->getArg(1)}));
    auto *Switch = CB.CreateSwitch(CB.CreateSExt(Member, CB.getInt64Ty()), Bad, Proof.Components);
    for (unsigned N = 0; N < Proof.Components; ++N) {
        BasicBlock *BB = BasicBlock::Create(C, "component", Callback);
        Switch->addCase(CB.getInt64(N), BB);
        IRBuilder<> WB(BB);
        WB.CreateRet(WB.CreateCall(Clones[N]->Fn, {Callback->getArg(0), Callback->getArg(1)}));
    }
    IRBuilder<>(Bad).CreateRet(ConstantInt::getSigned(Type::getInt32Ty(C), SGPL_DAG_ERR_INVALID));
    for (auto &S : Storage) if (verifyFunction(*S->Fn, &errs())) llvm_unreachable("invalid phase callback");
    IRBuilder<> AB(&F->getEntryBlock(), F->getEntryBlock().getFirstInsertionPt());
    Value *State = AB.CreateAlloca(Frame, nullptr, "source.frame");
    BasicBlock *Dispatch = BasicBlock::Create(C, "temporal.source", F);
    BasicBlock *Join = BasicBlock::Create(C, "temporal.source.join", F);
    IRBuilder<> DB(Dispatch);
    temporalPrepare(DB, Info);
    for (unsigned N = 0; N < Captures.size(); ++N)
        DB.CreateStore(Captures[N], DB.CreateStructGEP(Frame, State, N));
    for (unsigned N = 0; N < SourceSlots.size(); ++N) {
        unsigned Field = Captures.size() + N;
        DB.CreateStore(DB.CreateLoad(Frame->getElementType(Field), SourceSlots[N]),
                       DB.CreateStructGEP(Frame, State, Field));
    }
    Value *LiveCount = DB.getInt32(0);
    for (unsigned N = 0; N < Proof.Components; ++N) {
        Value *Active = DB.CreateCall(Presence[N], {State, DB.getInt64(N)});
        DB.CreateStore(DB.getInt32(N), DB.CreateGEP(IdsTy, DB.CreateStructGEP(Frame, State, IdField), {DB.getInt32(0), LiveCount}));
        LiveCount = DB.CreateAdd(LiveCount, Active, "source.live.components");
    }
    DB.CreateStore(DB.CreateSelect(DB.CreateICmpSGT(LiveCount, DB.getInt32(1)),
                                   DB.getInt32(SGPL_TEMPORAL_SOURCE_PHASE), DB.getInt32(SGPL_TEMPORAL_NONE)),
                   DB.CreateStructGEP(Frame, State, IdField + 1));
    Value *Budget = temporalBudget(DB);
    Value *Status = gpuTemporalDispatch(DB, Callback, Frame, Captures, Info.GraphPtr, State, LiveCount, Budget, "SourcePhase");
    if (!Status) Status = temporalDispatch(DB, Callback, State, LiveCount, temporalCpuBudget(DB), "SourcePhase");
    temporalFailureBranch(DB, Status, Join);
    IRBuilder<> JB(Join);
    for (unsigned N = 0; N < SourceSlots.size(); ++N) {
        unsigned Field = Captures.size() + N;
        JB.CreateStore(JB.CreateLoad(Frame->getElementType(Field), JB.CreateStructGEP(Frame, State, Field)), SourceSlots[N]);
    }
    Value *Next = JB.CreateAdd(IV, Step->getOperand(1), "source.next");
    JB.CreateBr(Body.Unit->getHeader());
    IV->addIncoming(Next, Join);
    cast<BranchInst>(Body.Unit->getHeader()->getTerminator())->setSuccessor(0, Dispatch);
    markDagOwned(Info.NeighborLoop, "temporal", "ordered", "parallel");
    markTemporalScope(Info.NeighborLoop, "SourcePhase");
    return true;
}

struct RoundDomain {
    TemporalBody Body;
    Value *Counter = nullptr, *Bound = nullptr;
    PHINode *Phi = nullptr;
    StoreInst *Increment = nullptr;
    Value *Start = nullptr;
    SmallVector<const Value *, 16> Private;
};

static const ConstantInt *roundKnownValue(Value *V, Loop *L) {
    if (auto *K = dyn_cast<ConstantInt>(V)) return K;
    Value *Slot = V;
    if (auto *Load = dyn_cast<LoadInst>(V)) Slot = Load->getPointerOperand();
    const ConstantInt *Result = nullptr;
    for (User *U : Slot->users()) if (auto *S = dyn_cast<StoreInst>(U))
        if (S->getPointerOperand() == Slot && !L->contains(S)) {
            auto *K = dyn_cast<ConstantInt>(S->getValueOperand());
            if (!K || (Result && Result->getValue() != K->getValue())) return nullptr;
            Result = K;
        }
    return Result;
}

/* A finite ascending domain is independently enumerable. Memory induction is
 * common in the DSL. PHI induction is common after LLVM canonicalization.
 * Other loop-carried values and changing bounds remain inconclusive. */
static bool roundDomain(NeighborLoopInfo &Info, RoundDomain &R,
                        DominatorTree &DT, std::string &Reason) {
    Loop *L = Info.DriverLoop->getParentLoop();
    if (!L) { Reason = "there is no enclosing round loop"; return false; }
    if (!L->getLoopPreheader() || !temporalBody(L, R.Body, Reason)) return false;
    auto *Br = cast<BranchInst>(L->getHeader()->getTerminator());
    auto *Cmp = dyn_cast<ICmpInst>(Br->getCondition());
    if (!Cmp || Cmp->getPredicate() != ICmpInst::ICMP_SLT) {
        Reason = "round bound is not a finite ascending domain"; return false;
    }
    R.Bound = Cmp->getOperand(1);
    if (auto *Load = dyn_cast<LoadInst>(R.Bound)) {
        if (Load->isVolatile() || Load->isAtomic()) return false;
        if (!L->isLoopInvariant(Load->getPointerOperand())) {
            Reason = "round bound address changes during progression"; return false;
        }
        for (BasicBlock *BB : L->blocks()) for (Instruction &I : *BB)
            if (auto *S = dyn_cast<StoreInst>(&I))
                if (getUnderlyingObject(S->getPointerOperand()) == getUnderlyingObject(Load->getPointerOperand())) {
                    Reason = "round domain changes during progression"; return false;
                }
    } else if (!L->isLoopInvariant(R.Bound)) {
        Reason = "round bound depends on an earlier round"; return false;
    }
    Value *Induction = Cmp->getOperand(0);
    if (!Induction->getType()->isIntegerTy(32)) {
        /* i32 endpoints have an overflow-free i64 difference. Unknown i64
         * ranges need an overflow proof before enumeration can be accepted. */
        Reason = "round induction range requires an overflow proof"; return false;
    }
    if (auto *P = dyn_cast<PHINode>(Induction)) {
        if (std::distance(L->getHeader()->phis().begin(), L->getHeader()->phis().end()) != 1) {
            Reason = "round loop has another carried SSA value"; return false;
        }
        auto *Add = dyn_cast<BinaryOperator>(P->getIncomingValueForBlock(R.Body.Latch));
        if (!Add || Add->getOpcode() != Instruction::Add || Add->getOperand(0) != P ||
            !isa<ConstantInt>(Add->getOperand(1)) || !cast<ConstantInt>(Add->getOperand(1))->isOne()) {
            Reason = "round progression is not a unit increment"; return false;
        }
        R.Phi = P; R.Start = P->getIncomingValueForBlock(L->getLoopPreheader());
    } else if (auto *Load = dyn_cast<LoadInst>(Induction)) {
        R.Counter = Load->getPointerOperand()->stripPointerCasts();
        if (!isa<GlobalVariable>(R.Counter) && !isa<AllocaInst>(R.Counter)) return false;
        for (BasicBlock *BB : R.Body.Blocks) for (Instruction &I : *BB)
            if (auto *S = dyn_cast<StoreInst>(&I)) if (S->getPointerOperand() == R.Counter) {
                auto *Add = dyn_cast<BinaryOperator>(S->getValueOperand());
                auto *Old = Add ? dyn_cast<LoadInst>(Add->getOperand(0)) : nullptr;
                if (R.Increment || !Add || Add->getOpcode() != Instruction::Add || !Old ||
                    Old->getPointerOperand() != R.Counter || !isa<ConstantInt>(Add->getOperand(1)) ||
                    !cast<ConstantInt>(Add->getOperand(1))->isOne()) {
                    Reason = "round progression has another induction write"; return false;
                }
                R.Increment = S;
            }
        if (!R.Increment || !DT.dominates(R.Increment, R.Body.Latch->getTerminator())) {
            Reason = "round progression is conditional"; return false;
        }
        R.Private.push_back(R.Counter);
    } else { Reason = "round induction is unknown"; return false; }
    for (Instruction &I : *L->getHeader())
        if (&I != Br && temporalObservable(I)) {
            Reason = "round header has additional effects"; return false;
        }
    return true;
}

static bool roundIndex(Value *V, const RoundDomain &R, DominatorTree &DT,
                       int64_t &Offset) {
    if (auto *C = dyn_cast<CastInst>(V)) {
        if (C->getOpcode() == Instruction::SExt || C->getOpcode() == Instruction::ZExt ||
            C->getOpcode() == Instruction::Trunc) {
            if (!C->getType()->isIntegerTy() || C->getType()->getIntegerBitWidth() < 32) return false;
            return roundIndex(C->getOperand(0), R, DT, Offset);
        }
    }
    if (V == R.Phi && R.Phi) { Offset = 0; return true; }
    if (auto *L = dyn_cast<LoadInst>(V)) if (L->getPointerOperand() == R.Counter) {
        if (DT.dominates(R.Increment, L)) { Offset = 1; return true; }
        if (DT.dominates(L, R.Increment)) { Offset = 0; return true; }
        return false;
    }
    if (auto *B = dyn_cast<BinaryOperator>(V))
        if (auto *K = dyn_cast<ConstantInt>(B->getOperand(1)))
            if ((B->getOpcode() == Instruction::Add || B->getOpcode() == Instruction::Sub) &&
                roundIndex(B->getOperand(0), R, DT, Offset)) {
                Offset += B->getOpcode() == Instruction::Add ? K->getSExtValue() : -K->getSExtValue();
                return true;
            }
    return false;
}

static bool roundAddress(Value *Pointer, const RoundDomain &R, DominatorTree &DT,
                         int64_t &Offset, Type *&Element) {
    auto *G = dyn_cast<GetElementPtrInst>(Pointer->stripPointerCasts());
    if (!G) return false;
    if (G->getNumIndices() == 1) {
        Element = G->getSourceElementType();
        return roundIndex(G->getOperand(1), R, DT, Offset);
    }
    if (G->getNumIndices() == 2 && isa<ArrayType>(G->getSourceElementType()))
        if (auto *Zero = dyn_cast<ConstantInt>(G->getOperand(1))) if (Zero->isZero()) {
            Element = cast<ArrayType>(G->getSourceElementType())->getElementType();
            return roundIndex(G->getOperand(2), R, DT, Offset);
        }
    return false;
}

static bool roundScratch(const Value *Object, const RoundDomain &R,
                          ArrayRef<TemporalAccess> Accesses, DominatorTree &DT) {
    Type *Ty = nullptr;
    if (auto *G = dyn_cast<GlobalVariable>(Object)) {
        if (!G->hasLocalLinkage()) return false;
        Ty = G->getValueType();
    } else if (auto *A = dyn_cast<AllocaInst>(Object)) Ty = A->getAllocatedType();
    else return false;
    if (!Ty->isIntegerTy() && !Ty->isFloatingPointTy()) return false;
    StoreInst *Reset = nullptr;
    for (const auto &A : Accesses) if (temporalObject(A.Pointer) == Object)
        if (auto *S = dyn_cast<StoreInst>(A.Origin))
            if (R.Body.Unit->isLoopInvariant(S->getValueOperand())) {
                bool Dominates = true;
                for (const auto &B : Accesses) if (!B.Write && temporalObject(B.Pointer) == Object)
                    Dominates &= DT.dominates(S, B.Origin);
                if (Dominates) { Reset = S; break; }
            }
    if (!Reset) return false;
    for (const User *U : Object->users()) {
        const auto *I = dyn_cast<Instruction>(U);
        if (!I || (!isa<LoadInst>(I) && !isa<StoreInst>(I))) return false;
        if (isa<LoadInst>(I) && !R.Body.Unit->contains(I) &&
            !DT.dominates(I, R.Body.Unit->getLoopPreheader()->getTerminator())) return false;
    }
    return true;
}

static void proveRounds(NeighborLoopInfo &Info, RoundDomain &R,
                         AAResults &AA, DominatorTree &DT) {
    auto &P = Info.TemporalResult.Rounds;
    if (!Info.DriverLoop->getParentLoop()) {
        P.Status = TemporalProofStatus::NotApplicable;
        P.Reason = "one traversal cannot form a non-trivial round partition"; return;
    }
    P.Status = TemporalProofStatus::Inconclusive;
    if (!roundDomain(Info, R, DT, P.Reason)) return;
    const auto *Start = roundKnownValue(R.Phi ? R.Start : R.Counter, R.Body.Unit);
    const auto *Bound = roundKnownValue(R.Bound, R.Body.Unit);
    if (Start && Bound && Bound->getSExtValue() - Start->getSExtValue() < 2) {
        P.Status = TemporalProofStatus::NotApplicable;
        if (Bound->getSExtValue() > Start->getSExtValue()) {
            P.Components = 1; P.Members.push_back(0);
        }
        P.Reason = "round domain has fewer than two units";
        return;
    }
    SmallVector<TemporalAccess, 64> Accesses;
    SmallVector<Instruction *, 8> Opaque;
    SmallVector<BasicBlock *, 32> Complete(R.Body.Blocks);
    Complete.push_back(R.Body.Unit->getHeader());
    collectTemporalAccesses(Complete, Info, Accesses, Opaque);
    /* Iterator state is local to each outlined round. Every invocation starts
     * with iter_init. A slot observed after the nest cannot become private. */
    SmallVector<Value *, 8> IteratorSlots;
    for (BasicBlock *BB : R.Body.Blocks) for (Instruction &I : *BB)
        if (auto *Call = dyn_cast<CallInst>(&I)) {
            if (isIteratorInit(Call)) IteratorSlots.push_back(Call->getArgOperand(2));
            if (isIteratorNext(Call)) IteratorSlots.push_back(Call->getArgOperand(1));
        }
    SmallPtrSet<Value *, 16> Added;
    for (Value *Slot : IteratorSlots) {
        if (!Added.insert(Slot).second) continue;
        auto *A = dyn_cast<AllocaInst>(Slot);
        bool Local = A && isa<ConstantInt>(A->getArraySize());
        if (Local) for (User *U : A->users()) if (auto *I = dyn_cast<Instruction>(U))
            if (!R.Body.Unit->contains(I) && !isIteratorInit(dyn_cast<CallInst>(I)) &&
                !isIteratorNext(dyn_cast<CallInst>(I))) Local = false;
        if (Local) R.Private.push_back(Slot);
    }
    SmallPtrSet<const Value *, 16> Private(R.Private.begin(), R.Private.end());
    for (BasicBlock *BB : R.Body.Blocks) for (Instruction &I : *BB)
        if (auto *A = dyn_cast<AllocaInst>(&I))
            if (!PointerMayBeCaptured(A, true, true)) Private.insert(A);
    for (const auto &A : Accesses) if (A.Pointer && A.Write) {
        const Value *O = temporalObject(A.Pointer);
        if (!Private.count(O) && roundScratch(O, R, Accesses, DT)) {
            Private.insert(O); R.Private.push_back(O);
        }
    }
    bool Unknown = !Opaque.empty(), Conflict = false;
    for (const auto &A : Accesses) if (A.Write && !Private.count(temporalObject(A.Pointer)))
        for (const auto &B : Accesses) {
            if (Private.count(temporalObject(B.Pointer))) continue;
            bool May = false;
            if (!temporalMayAlias(A, B, AA, May)) continue;
            int64_t OA = 0, OB = 0; Type *TA = nullptr, *TB = nullptr;
            auto Fits = [](const TemporalAccess &Access, Type *Element) {
                const auto Loc = MemoryLocation::getOrNone(Access.Origin);
                if (!Loc || !Loc->Size.hasValue()) return false;
                const auto &DL = Access.Origin->getModule()->getDataLayout();
                return Loc->Size.getValue() <= DL.getTypeAllocSize(Element);
            };
            if (temporalObject(A.Pointer) == temporalObject(B.Pointer) &&
                roundAddress(A.Pointer, R, DT, OA, TA) && roundAddress(B.Pointer, R, DT, OB, TB) &&
                OA == OB && TA == TB && Fits(A, TA) && Fits(B, TB)) continue;
            bool Uniform = !isa<GetElementPtrInst>(A.Pointer) &&
                           isIdentifiedObject(getUnderlyingObject(A.Pointer)) &&
                           A.Pointer == B.Pointer && DT.dominates(A.Origin, R.Body.Latch->getTerminator());
            May |= !Uniform;
            /* Only an unconditional fixed-address relation has a folded self
             * edge. Other relations retain distinct symbolic round endpoints. */
            P.Relations.push_back({0, Uniform ? 0u : 1u, B.Write ? "round-WAW" : "round-RAW", May, Uniform});
            Unknown |= May; Conflict |= !May;
        }
    for (Instruction *I : Opaque) { (void)I; P.Relations.push_back({0, 1, "unknown-round-resource", true}); }
    if (Unknown || Conflict) {
        P.Components = 1; P.Members.push_back(0);
        if (llvm::any_of(P.Relations, [](const TemporalDependency &D) { return D.From != D.To; }))
            P.Members.push_back(0);
    }
    if (Conflict) { P.Status = TemporalProofStatus::Interfering; P.Reason = "complete rounds have a repeated dependency"; }
    else if (Unknown) { P.Status = TemporalProofStatus::Inconclusive; P.Reason = "complete round dependencies are incomplete"; }
    else {
        P.Status = TemporalProofStatus::Independent;
        P.Components = 2; /* symbolic independent domain, instantiated at runtime */
        P.Members.push_back(0); P.Members.push_back(1);
        P.Reason = "finite round domain has isolated scratch and disjoint outputs";
    }
}

static Value *roundExternalValue(IRBuilder<> &B, Value *V, const RoundDomain &R) {
    if (auto *L = dyn_cast<LoadInst>(V)) if (R.Body.Unit->contains(L))
        return B.CreateLoad(L->getType(), L->getPointerOperand(), "round.bound");
    return V;
}

static bool emitRounds(NeighborLoopInfo &Info, RoundDomain &R) {
    auto &Body = R.Body;
    Function *F = Body.Entry->getParent();
    LLVMContext &C = F->getContext();
    Module *M = F->getParent();
    if (!temporalGraphAvailable(Info, Body)) return false;
    TemporalClone Clone;
    if (!cloneTemporalBody(Body, Info, -1, Clone, R.Private, R.Counter)) {
        if (Clone.Fn) Clone.Fn->eraseFromParent(); return false;
    }
    if (R.Phi) {
        IRBuilder<> EB(Clone.Entry, Clone.Entry->getFirstInsertionPt());
        Value *IV = EB.CreateTrunc(Clone.Fn->getArg(1), R.Phi->getType(), "round.iv");
        for (BasicBlock &BB : *Clone.Fn) for (Instruction &I : BB)
            for (Use &U : I.operands()) if (U.get() == R.Phi) U.set(IV);
    }
    SmallVector<Value *, 16> Captures;
    StructType *Frame = temporalFrame({&Clone}, Captures, "Round");
    DominatorTree DT(*F);
    for (Value *V : Captures) if (auto *I = dyn_cast<Instruction>(V))
        if (!DT.dominates(I, Body.Unit->getLoopPreheader()->getTerminator())) {
            Clone.Fn->eraseFromParent(); return false;
        }
    if (verifyFunction(*Clone.Fn, &errs())) llvm_unreachable("invalid round callback");
    Function *Callback = Function::Create(Clone.Fn->getFunctionType(), GlobalValue::InternalLinkage,
                                          "sgpl.temporal.Round.component", M);
    Callback->addFnAttr("sgpl.temporal.callback");
    Callback->addFnAttr("sgpl.temporal.scope", "Round");
    BasicBlock *Entry = BasicBlock::Create(C, "entry", Callback);
    BasicBlock *Check = BasicBlock::Create(C, "check", Callback);
    BasicBlock *Work = BasicBlock::Create(C, "round", Callback);
    BasicBlock *Done = BasicBlock::Create(C, "done", Callback);
    BasicBlock *Fail = BasicBlock::Create(C, "failed", Callback);
    IRBuilder<> CB(Entry);
    unsigned Base = Captures.size();
    Value *Start = CB.CreateLoad(CB.getInt64Ty(), CB.CreateStructGEP(Frame, Callback->getArg(0), Base));
    Value *Total = CB.CreateLoad(CB.getInt64Ty(), CB.CreateStructGEP(Frame, Callback->getArg(0), Base + 1));
    Value *Count = CB.CreateLoad(CB.getInt64Ty(), CB.CreateStructGEP(Frame, Callback->getArg(0), Base + 2));
    Value *ID = Callback->getArg(1);
    /* Contiguous quotient/remainder groups avoid multiplication overflow. */
    Value *Q = CB.CreateSDiv(Total, Count), *Rem = CB.CreateSRem(Total, Count);
    Value *Extra = CB.CreateSelect(CB.CreateICmpSLT(ID, Rem), ID, Rem);
    Value *Begin = CB.CreateAdd(Start, CB.CreateAdd(CB.CreateMul(ID, Q), Extra));
    Value *Length = CB.CreateAdd(Q, CB.CreateZExt(CB.CreateICmpSLT(ID, Rem), CB.getInt64Ty()));
    Value *End = CB.CreateAdd(Begin, Length);
    CB.CreateBr(Check);
    IRBuilder<> HB(Check);
    PHINode *IV = HB.CreatePHI(HB.getInt64Ty(), 2, "member.round"); IV->addIncoming(Begin, Entry);
    HB.CreateCondBr(HB.CreateICmpSLT(IV, End), Work, Done);
    IRBuilder<> WB(Work);
    Value *Status = WB.CreateCall(Clone.Fn, {Callback->getArg(0), IV});
    Value *Next = WB.CreateAdd(IV, WB.getInt64(1)); IV->addIncoming(Next, Work);
    WB.CreateCondBr(WB.CreateICmpEQ(Status, WB.getInt32(0)), Check, Fail);
    IRBuilder<>(Done).CreateRet(ConstantInt::get(Type::getInt32Ty(C), 0));
    IRBuilder<>(Fail).CreateRet(Status);
    Check->getTerminator()->setMetadata("sgpl.frontier.nested.sequential", MDNode::get(C, MDString::get(C, "sequential")));
    IRBuilder<> AB(&F->getEntryBlock(), F->getEntryBlock().getFirstInsertionPt());
    Value *State = AB.CreateAlloca(Frame, nullptr, "round.frame");
    BasicBlock *Pre = Body.Unit->getLoopPreheader();
    BasicBlock *Exit = cast<BranchInst>(Body.Unit->getHeader()->getTerminator())->getSuccessor(1);
    BasicBlock *Dispatch = BasicBlock::Create(C, "temporal.rounds", F);
    BasicBlock *Join = BasicBlock::Create(C, "temporal.rounds.join", F);
    IRBuilder<> DB(Dispatch);
    temporalPrepare(DB, Info);
    Value *First32 = R.Phi ? R.Start : DB.CreateLoad(Type::getInt32Ty(C), R.Counter, "round.start");
    Value *Bound32 = roundExternalValue(DB, R.Bound, R);
    Value *First = DB.CreateSExt(First32, DB.getInt64Ty());
    Value *Bound = DB.CreateSExt(Bound32, DB.getInt64Ty());
    Value *TotalRounds = DB.CreateSelect(DB.CreateICmpSLT(First, Bound), DB.CreateSub(Bound, First), DB.getInt64(0));
    Value *Budget = temporalBudget(DB);
    Value *Limit = gpuBackendSelected(M) ? DB.getInt64(65536) : DB.CreateSExt(Budget, DB.getInt64Ty());
    Value *ComponentCount = DB.CreateSelect(DB.CreateICmpSLT(TotalRounds, Limit), TotalRounds, Limit);
    for (unsigned N = 0; N < Captures.size(); ++N)
        DB.CreateStore(Captures[N], DB.CreateStructGEP(Frame, State, N));
    DB.CreateStore(First, DB.CreateStructGEP(Frame, State, Base));
    DB.CreateStore(TotalRounds, DB.CreateStructGEP(Frame, State, Base + 1));
    DB.CreateStore(ComponentCount, DB.CreateStructGEP(Frame, State, Base + 2));
    DB.CreateStore(DB.CreateSelect(DB.CreateICmpSGT(TotalRounds, DB.getInt64(1)),
                                   DB.getInt32(SGPL_TEMPORAL_ROUND), DB.getInt32(SGPL_TEMPORAL_NONE)),
                   DB.CreateStructGEP(Frame, State, Base + 3));
    Value *DispatchCount = DB.CreateTrunc(ComponentCount, DB.getInt32Ty());
    Value *DispatchStatus = gpuTemporalDispatch(DB, Callback, Frame, Captures, Info.GraphPtr, State, DispatchCount, Budget, "Round");
    if (!DispatchStatus) {
        Value *CpuBudget = temporalCpuBudget(DB);
        Value *CpuLimit = DB.CreateSExt(CpuBudget, DB.getInt64Ty());
        Value *CpuCount = DB.CreateSelect(DB.CreateICmpSLT(TotalRounds, CpuLimit), TotalRounds, CpuLimit);
        DB.CreateStore(CpuCount, DB.CreateStructGEP(Frame, State, Base + 2));
        DispatchStatus = temporalDispatch(DB, Callback, State, DB.CreateTrunc(CpuCount, DB.getInt32Ty()), CpuBudget, "Round");
    }
    if (!Info.RoundSpatialRuntimeReason.empty())
        cast<CallInst>(DispatchStatus)->setMetadata("sgpl.round-spatial.unavailable",
            MDNode::get(C, MDString::get(C, Info.RoundSpatialRuntimeReason)));
    temporalFailureBranch(DB, DispatchStatus, Join);
    IRBuilder<> JB(Join);
    Value *Final = JB.CreateTrunc(JB.CreateAdd(First, TotalRounds), Type::getInt32Ty(C));
    if (R.Counter) JB.CreateStore(Final, R.Counter);
    JB.CreateBr(Exit);
    for (PHINode &P : Exit->phis()) {
        Value *Old = P.getIncomingValueForBlock(Body.Unit->getHeader());
        if (Old == R.Phi) P.addIncoming(Final, Join);
        else { /* temporalBody rejects ordinary SSA outputs */
            P.addIncoming(Old, Join);
        }
    }
    auto *Term = Pre->getTerminator();
    for (unsigned S = 0; S < Term->getNumSuccessors(); ++S)
        if (Term->getSuccessor(S) == Body.Unit->getHeader()) Term->setSuccessor(S, Dispatch);
    for (PHINode &P : Body.Unit->getHeader()->phis()) P.removeIncomingValue(Pre, false);
    markDagOwned(Info.NeighborLoop, "temporal", "ordered", "parallel");
    markTemporalScope(Info.NeighborLoop, "Round");
    Dispatch->getTerminator()->setMetadata("sgpl.temporal.scope", MDNode::get(C, MDString::get(C, "Round")));
    return true;
}

#include "round_spatial_lowering.inc"

static bool lowerTemporalScopes(NeighborLoopInfo &Info, AAResults &AA,
                                 DominatorTree &DT, bool Emit) {
    RoundDomain R;
    proveRounds(Info, R, AA, DT);
    auto &T = Info.TemporalResult;
    if (T.Rounds.Status == TemporalProofStatus::Independent) {
        if (Emit && emitRoundSpatial(Info, R)) {
            T.Scope = TemporalScope::Round; T.Realized = true;
            T.Rounds.RuntimeRealizable = true;
            T.Phases.Status = TemporalProofStatus::Skipped;
            T.Phases.Reason = "round concurrency has priority";
            temporalPrint(Info); return true;
        }
        if (Emit && emitRounds(Info, R)) {
            T.Scope = TemporalScope::Round; T.Realized = true;
            T.Rounds.RuntimeRealizable = true;
            T.Phases.Status = TemporalProofStatus::Skipped;
            T.Phases.Reason = "round concurrency has priority";
            temporalPrint(Info); return true;
        }
        T.Rounds.RuntimeReason = Emit ? "round proof has no supported CFG realization" : "temporal emission is disabled";
    }
    TemporalBody Body;
    if (!temporalBody(Info.DriverLoop, Body, T.Phases.Reason))
        T.Phases.Status = TemporalProofStatus::Inconclusive;
    else {
        proveSourcePhases(Info, Body, AA);
        if (T.Phases.Status == TemporalProofStatus::Independent) {
            if (Emit && emitSourcePhases(Info, Body)) {
                T.Scope = TemporalScope::SourcePhase; T.Realized = true;
                T.Phases.RuntimeRealizable = true;
                temporalPrint(Info); return true;
            }
            T.Phases.RuntimeReason = Emit ? "phase proof has no supported CFG realization" : "temporal emission is disabled";
        }
    }
    temporalPrint(Info); return false;
}

/* The spatial engine has specific source lifecycle callbacks. Reject effects
 * that those callbacks cannot preserve, instead of dropping source code. */
static bool legacySourceLifecycleAvailable(const NeighborLoopInfo &Info) {
    for (BasicBlock *BB : Info.DriverLoop->blocks()) {
        if (Info.NeighborLoop->contains(BB)) continue;
        for (Instruction &I : *BB) {
            if (temporalInstrumentation(I)) continue;
            if (auto *S = dyn_cast<StoreInst>(&I)) {
                if (S == Info.AccResetStore && Info.AccConsumeStore) continue;
                if (S == Info.AccConsumeStore) continue;
                if (S->getPointerOperand() == Info.UAlloca) continue;
                bool Known = false;
                if (sourcePhaseFor(Info, S) == PhaseSegment::Preamble)
                    for (const Effect &E : Info.Effects) if (E.Origin == S)
                        Known |= E.Kind == EffectKind::Claim || Info.UsePrivLayout;
                if (!Known) return false;
            } else if (auto *CAS = dyn_cast<AtomicCmpXchgInst>(&I)) {
                if (sourcePhaseFor(Info, CAS) != PhaseSegment::Preamble ||
                    !llvm::is_contained(Info.DriverUClaims, CAS)) return false;
            } else if (auto *Call = dyn_cast<CallInst>(&I)) {
                Function *F = Call->getCalledFunction();
                if (isIteratorInit(Call) || Call->doesNotAccessMemory()) continue;
                StringRef N = F ? F->getName() : StringRef();
                if (N.starts_with("roaring_bitmap_get_") || N == "roaring_bitmap_contains") continue;
                return false;
            } else if (I.mayWriteToMemory()) return false;
        }
    }
    return true;
}
} // namespace
