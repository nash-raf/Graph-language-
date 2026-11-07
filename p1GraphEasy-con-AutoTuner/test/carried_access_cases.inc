static void carriedFixture(StringRef mode, bool separated, bool disjoint = false) {
    LLVMContext context;
    bool perSource = mode == "per_source";
    bool round = mode == "round";
    std::string ir = R"(
target datalayout = "e-p:64:64-i64:64-n8:16:32:64-S128"
declare void @autograph_neighbor_iter_init(ptr, i64, ptr) memory(argmem: readwrite)
declare i32 @autograph_neighbor_iter_next(ptr, ptr) memory(argmem: readwrite)
define void @work(ptr %a, ptr %b, ptr %frontier, ptr noalias %graph) {
entry:
 %iter = alloca [128 x i8]
 %vslot = alloca i32
)";
    if (mode == "stable_slot" || mode == "mutated_slot")
        ir += " %slot = alloca ptr\n store ptr %a, ptr %slot\n";
    if (round) ir += " br label %round\nround:\n %t = phi i32 [0, %entry], [%tn, %end]\n br label %prefix\nprefix:\n";
    ir += " br label %driver\ndriver:\n %u = phi i32 [0, %" + std::string(round ? "prefix" : "entry") +
          "], [%un, %latch]\n";
    ir += " %more = icmp slt i32 %u, 8\n br i1 %more, label %source, label %" + std::string(round ? "end" : "exit") + "\nsource:\n";
    if (mode == "frontier")
        ir += " %fp = getelementptr i32, ptr %frontier, i32 %u\n %vertex = load i32, ptr %fp\n %u64 = zext i32 %vertex to i64\n";
    else ir += " %u64 = zext i32 %u to i64\n";
    if (mode == "stable_slot" || mode == "mutated_slot") ir += " %base = load ptr, ptr %slot\n";
    auto accesses = [&]() {
        std::string s;
        std::string ix = mode == "dest" || mode == "mixed" ? "%v" : "%u";
        if (mode == "frontier") ix = "%vertex";
        if (mode == "shift") s += " %shift = add i32 %u, 1\n";
        if (mode == "modulo") { s += " %mod = and i32 %u, 3\n"; ix = "%mod"; }
        if (mode == "truncate") { s += " %small = trunc i32 %u to i8\n %wide = zext i8 %small to i32\n"; ix = "%wide"; }
        if (mode == "scalar") ix = "0";
        s += " %rp = getelementptr i32, ptr %a, i32 " + std::string(mode == "mixed" ? "%u" : ix) + "\n";
        s += " %left = load i32, ptr %rp\n";
        s += " %wp = getelementptr i32, ptr " + std::string(mode == "alias" || mode == "noalias" ? "%b" : "%a") +
             ", i32 " + std::string(mode == "shift" ? "%shift" : ix) + "\n";
        s += " %desired = add i32 %left, 5\n store i32 %desired, ptr %wp\n";
        return s;
    };
    if (perSource) ir += accesses();
    ir += R"(
 call void @autograph_neighbor_iter_init(ptr %graph, i64 %u64, ptr %iter)
 br label %neighbor
neighbor:
 %next = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %vslot)
 %has = icmp ne i32 %next, 0
 br i1 %has, label %pair, label %latch
pair:
 %v = load i32, ptr %vslot
)";
    if (!perSource) ir += accesses();
    if (mode == "mutated_slot") ir += " store ptr %b, ptr %slot\n";
    ir += " br label %neighbor\nlatch:\n %un = add i32 %u, 1\n br label %driver\n";
    if (round) ir += "end:\n %tn = add i32 %t, 1\n %again = icmp slt i32 %tn, 3\n br i1 %again, label %round, label %exit\n";
    ir += "exit:\n ret void\n}\n";
    if (mode == "noalias") ir.replace(ir.find("ptr %a, ptr %b"), std::string("ptr %a, ptr %b").size(), "ptr noalias %a, ptr %b");
    auto replaceAll = [&](const std::string &from, const std::string &to) {
        size_t pos = 0;
        while ((pos = ir.find(from, pos)) != std::string::npos) { ir.replace(pos, from.size(), to); pos += to.size(); }
    };
    if (mode == "stable_slot" || mode == "mutated_slot")
        replaceAll("getelementptr i32, ptr %a", "getelementptr i32, ptr %base");
    if (mode == "wide_access") {
        replaceAll("%left = load i32", "%left = load i64");
        replaceAll("%desired = add i32", "%desired = add i64");
        replaceAll("store i32 %desired", "store i64 %desired");
    }
    SMDiagnostic error;
    auto module = parseAssemblyString(ir, error, context);
    if (!module) { error.print("carried_access", errs()); ++failures; return; }
    if (verifyModule(*module, &errs())) { ++failures; return; }
    PassBuilder pb;
    LoopAnalysisManager lam; FunctionAnalysisManager fam;
    CGSCCAnalysisManager cgam; ModuleAnalysisManager mam;
    pb.registerModuleAnalyses(mam); pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam); pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);
    Function &fn = *module->getFunction("work");
    auto &li = fam.getResult<LoopAnalysis>(fn);
    auto &aa = fam.getResult<AAManager>(fn);
    NeighborLoopInfo info;
    for (Loop *L : li.getLoopsInPreorder()) {
        if (L->getHeader()->getName() == "driver") info.DriverLoop = L;
        if (L->getHeader()->getName() == "neighbor") info.NeighborLoop = L;
    }
    LoadInst *read = nullptr; StoreInst *write = nullptr;
    for (BasicBlock &BB : fn) for (Instruction &I : BB) {
        if (I.getName() == "left") read = cast<LoadInst>(&I);
        if (auto *S = dyn_cast<StoreInst>(&I))
            if (S->getPointerOperand()->getName() == "wp") write = S;
        if (I.getName() == "u64") info.UVal = &I;
    }
    check(read && write, "carried fixture has both accesses");
    if (!read || !write) return;
    auto effect = [&](Instruction *I, EffectKind K) {
        Effect E; E.Kind = K; E.Origin = I;
        auto *G = cast<GetElementPtrInst>(carriedPointer(I));
        E.Base = G->getPointerOperand(); E.Index = G->getOperand(G->getNumOperands()-1);
        E.Reg = mode == "dest" ? Region::V : Region::U; // deliberately misleading for shifted/noninjective tests
        if (mode == "mixed" && K == EffectKind::W) E.Reg = Region::V;
        if (mode == "scalar") E.Reg = Region::G;
        E.Scope = perSource ? OccurrenceScope::PerSource : OccurrenceScope::PerPair;
        return E;
    };
    info.Effects.push_back(effect(read, EffectKind::R));
    info.Effects.push_back(effect(write, EffectKind::W));
    buildEffectExpr(info);
    proveCarriedAccesses(info, aa);
    check(carriedSourceSeparated(info.CarriedProof, read, write) == separated,
          "source address separation: " + mode.str());
    deriveAllTemporal(info);
    buildFootprints(info); deriveAccessRelations(info);
    bool pairFound = false, selfFound = false, rawFound = false;
    for (const AccessRelation &R : info.AccessRelations) {
        check(R.RoundOffset == 0 && R.Relation.Offset == 0 && !R.CrossRound,
              "carried never invents round distance: " + mode.str());
        check(R.RoundUncertain == round, "round uncertainty is separate: " + mode.str());
        if (R.Kind == AccessRelKind::WAW) selfFound = true;
        if (R.Kind == AccessRelKind::RAW) rawFound = true;
        if (R.Kind != AccessRelKind::WAR) continue;
        pairFound = true;
        check(R.MayCarrySource == !separated, "source carrying: " + mode.str());
        check(R.MayCarryNeighbor == !perSource, "neighbor carrying: " + mode.str());
        if (!separated || !perSource) check(R.TemporalClass == Temporal::Carried, "conservative carried label");
        if (mode == "alias") check(R.Spatial == OwnerClass::UnknownOwner && R.DischargedBy == DischargeKind::None,
                                    "unresolved aliases cannot inherit ownership/discharge");
    }
    check(pairFound != disjoint, "alias candidate coverage: " + mode.str());
    check(selfFound, "single write retains cross-iteration WAW: " + mode.str());
    check(rawFound == (!disjoint && (!perSource || round)), "reverse RAW recurrence coverage: " + mode.str());
    buildProofGraphs(info);
    for (const ProofEdge &E : info.Graphs.Round.Edges)
        check(E.CrossRound == round, "source carrying does not become round carrying");
    if (mode == "mixed" || mode == "alias")
        check(llvm::any_of(info.Graphs.Spatial.Edges, [](const ProofEdge &E) { return E.Survives && E.CrossPartition; }),
              "unresolved source conflict stays in spatial graph");
}

static void carriedCases() {
    carriedFixture("exact", true);
    carriedFixture("per_source", true);
    carriedFixture("round", true);
    carriedFixture("frontier", true); // uniqueness is supplied by language semantics
    carriedFixture("stable_slot", true);
    for (StringRef mode : {"dest", "mixed", "shift", "modulo", "truncate", "scalar", "alias", "mutated_slot", "wide_access"})
        carriedFixture(mode, false);
    carriedFixture("noalias", false, true);
}
