// Included by previous_round_read_test.cpp so both proofs use the same driver.
static void bufferFixture(StringRef mode, bool expected) {
    bool phi = mode.starts_with("phi");
    bool copy = mode.starts_with("copy") || mode == "memcpy";
    LLVMContext context;
    std::string ir = R"(
target datalayout = "e-p:64:64-i64:64-n8:16:32:64-S128"
declare void @autograph_neighbor_iter_init(ptr, i64, ptr) memory(argmem: readwrite)
declare i32 @autograph_neighbor_iter_next(ptr, ptr) memory(argmem: readwrite)
declare void @opaque(ptr)
declare void @readonly_call() memory(read)
declare void @argwrite(ptr) memory(argmem: write)
declare void @escape(ptr) memory(none)
declare noalias ptr @calloc(i64, i64)
declare void @llvm.memcpy.p0.p0.i64(ptr, ptr, i64, i1 immarg)
define void @work(ptr noalias %graph, ptr %alias, i1 %gate) {
entry:
 %a = alloca [64 x i32]
 %b = alloca [64 x i32]
 %input.slot = alloca ptr
 %output.slot = alloca ptr
 store ptr %a, ptr %input.slot
 store ptr %b, ptr %output.slot
 %iter = alloca [128 x i8]
 %vslot = alloca i32
 br label %round
round:
 %r = phi i32 [0, %entry], [%rn, %end]
)";
    if (phi) ir += " %cur = phi ptr [%a, %entry], [%out, %end]\n"
                   " %out = phi ptr [%b, %entry], [%cur, %end]\n";
    ir += R"(
 %more = icmp slt i32 %r, 10
 br i1 %more, label %prefix, label %exit
prefix:
)";
    if (mode == "copy_prefix_write") ir += " %early = load ptr, ptr %input.slot\n store i32 7, ptr %early\n";
    ir += R"(
 br label %driver
driver:
 %u = phi i64 [0, %prefix], [%un, %latch]
 %has = icmp slt i64 %u, 4
 br i1 %has, label %source, label %suffix
source:
)";
    if (!phi) ir += " %cur = load ptr, ptr %input.slot\n";
    ir += R"(
 %rp = getelementptr i32, ptr %cur, i64 %u
 %read = load i32, ptr %rp
 call void @autograph_neighbor_iter_init(ptr %graph, i64 %u, ptr %iter)
 br label %neighbor
neighbor:
 %next = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %vslot)
 %hasv = icmp ne i32 %next, 0
 br i1 %hasv, label %pair, label %latch
pair:
 %v = load i32, ptr %vslot
)";
    if (!phi) ir += " %out = load ptr, ptr %output.slot\n";
    ir += " %wp = getelementptr i32, ptr %out, i32 %v\n store i32 %read, ptr %wp\n";
    if (mode == "input_write" || mode == "phi_input_write") ir += " store i32 7, ptr %rp\n";
    if (mode == "alias_write") ir += " %aliasptr = select i1 %gate, ptr %cur, ptr %out\n store i32 7, ptr %aliasptr\n";
    if (mode == "slot_escape") ir += " call void @escape(ptr %input.slot)\n";
    if (mode == "seed_write") ir += " store i32 7, ptr %a\n";
    if (mode == "opaque" || mode == "phi_opaque" || mode == "copy_opaque") ir += " call void @opaque(ptr %cur)\n";
    if (mode == "readonly") ir += " call void @readonly_call()\n";
    if (mode == "argwrite") ir += " call void @argwrite(ptr %cur)\n";
    if (mode == "atomic") ir += " store atomic i32 7, ptr %rp seq_cst, align 4\n";
    if (mode == "copy_in_pair") ir += " %earlycopy = load i32, ptr %wp\n store i32 %earlycopy, ptr %rp\n";
    ir += R"(
 br label %neighbor
latch:
 %un = add i64 %u, 1
 br label %driver
suffix:
)";
    if (mode == "bypass" || mode == "copy_bypass")
        ir += " br i1 %gate, label %publish, label %end\npublish:\n";
    if (mode == "memcpy") {
        ir += " %ca = load ptr, ptr %input.slot\n %cb = load ptr, ptr %output.slot\n"
              " call void @llvm.memcpy.p0.p0.i64(ptr %ca, ptr %cb, i64 256, i1 false)\n br label %end\n";
    } else if (copy) {
        std::string incoming = mode == "copy_bypass" ? "publish" : "suffix";
        ir += " br label %copy\ncopy:\n %j = phi i64 [0, %" + incoming + "], [%jn, %copy_body]\n"
              " %copy_more = icmp slt i64 %j, 64\n br i1 %copy_more, label %copy_body, label %end\ncopy_body:\n"
              " %ca = load ptr, ptr %input.slot\n %cb = load ptr, ptr %output.slot\n"
              " %cp = getelementptr i32, ptr %ca, i64 %j\n %cq = getelementptr i32, ptr %cb, i64 %j\n"
              " %cv = load i32, ptr %cq\n store i32 %cv, ptr %cp\n"
              " %jn = add i64 %j, 1\n br label %copy\n";
    } else {
        if (!phi) {
            ir += " %oldcur = load ptr, ptr %input.slot\n %oldout = load ptr, ptr %output.slot\n"
                  " store ptr %oldout, ptr %input.slot\n store ptr %oldcur, ptr %output.slot\n";
            if (mode == "post_swap_load") ir += " %late = load ptr, ptr %input.slot\n %lateval = load i32, ptr %late\n";
        }
        ir += " br label %end\n";
    }
    ir += "end:\n %rn = add i32 %r, 1\n br label %round\nexit:\n ret void\n}\n";
    auto replace = [&](StringRef from, StringRef to) {
        size_t pos = ir.find(from.str());
        check(pos != std::string::npos, "fixture replacement exists");
        if (pos != std::string::npos) ir.replace(pos, from.size(), to.str());
    };
    if (mode == "same_buffer" || mode == "copy_same_buffer")
        replace("store ptr %b, ptr %output.slot", "store ptr %a, ptr %output.slot");
    if (mode == "phi_same_buffer") replace("%out = phi ptr [%b", "%out = phi ptr [%a");
    if (mode == "one_way") replace("store ptr %oldcur, ptr %output.slot", "store ptr %oldout, ptr %output.slot");
    if (mode == "copy_not_copy") replace("store i32 %cv, ptr %cp", "store i32 7, ptr %cp");
    if (mode == "volatile") replace("%read = load i32", "%read = load volatile i32");
    if (mode == "global_swap" || mode == "copy_heap") {
        replace("define void @work", "@input.slot = internal global ptr null\n@output.slot = internal global ptr null\ndefine void @work");
        replace(" %input.slot = alloca ptr\n", "");
        replace(" %output.slot = alloca ptr\n", "");
        for (StringRef slot : {"input.slot", "output.slot"}) {
            size_t at;
            while ((at = ir.find("%" + slot.str())) != std::string::npos)
                ir.replace(at, 1, "@");
        }
    }
    if (mode == "copy_heap") {
        replace("alloca [64 x i32]", "call ptr @calloc(i64 64, i64 4)");
        replace("alloca [64 x i32]", "call ptr @calloc(i64 64, i64 4)");
    }
    if (mode == "copy_direct") {
        replace("%cur = load ptr, ptr %input.slot", "%cur = getelementptr i32, ptr %a, i64 0");
        replace("%out = load ptr, ptr %output.slot", "%out = getelementptr i32, ptr %b, i64 0");
        replace("%ca = load ptr, ptr %input.slot", "%ca = getelementptr i32, ptr %a, i64 0");
        replace("%cb = load ptr, ptr %output.slot", "%cb = getelementptr i32, ptr %b, i64 0");
    }

    SMDiagnostic error;
    auto module = parseAssemblyString(ir, error, context);
    if (!module) { error.print("buffer_round_read", errs()); ++failures; return; }
    if (verifyModule(*module, &errs())) { ++failures; return; }
    PassBuilder pb;
    LoopAnalysisManager lam; FunctionAnalysisManager fam;
    CGSCCAnalysisManager cgam; ModuleAnalysisManager mam;
    pb.registerModuleAnalyses(mam); pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam); pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);
    Function &function = *module->getFunction("work");
    auto &li = fam.getResult<LoopAnalysis>(function);
    auto &aa = fam.getResult<AAManager>(function);
    auto &dt = fam.getResult<DominatorTreeAnalysis>(function);
    auto &di = fam.getResult<DependenceAnalysis>(function);
    NeighborLoopInfo info;
    bool found = false;
    for (Loop *loop : li.getLoopsInPreorder())
        if (analyzeNeighborLoop(loop, info)) { found = true; break; }
    check(found, "buffer traversal recognized: " + mode.str());
    if (!found) return;
    Effect *read = nullptr;
    for (Effect &e : info.Effects)
        if (e.Kind == EffectKind::R && e.Origin && e.Origin->getName() == "read") read = &e;
    check(read != nullptr, "buffer read has an effect atom: " + mode.str());
    if (!read) return;
    auto valueSource = read->VSource;
    provePreviousRoundReads(info, di, li, aa, dt);
    deriveAllTemporal(info);
    check(read->VSource == valueSource, "buffer proof does not manufacture a snapshot");
    check(info.BufferRoundReads.count(read->Origin) == unsigned(expected), "buffer certificate: " + mode.str());
    if (expected && info.BufferRoundReads.count(read->Origin)) {
        check(read->Temp == Temporal::PreviousRoundRead, "buffer read projection: " + mode.str());
        check(info.EarlierRoundReads.empty(), "buffer proof does not invent dependence directions");
        check(info.BufferRoundReads.at(read->Origin).Kind ==
              (copy ? PDGBufferRoundProof::Copy : PDGBufferRoundProof::Swap), "publication kind");
        // A state certificate cannot move a read-before-publication WAR into
        // another round or manufacture a snapshot discharge.
        Effect write;
        write.Kind = EffectKind::W; write.Base = read->Base; write.Reg = read->Reg;
        AccessRelation war;
        war.Kind = AccessRelKind::WAR; war.Source = read; war.Sink = &write;
        war.SourceRegion = war.SinkRegion = read->Reg;
        check(relateAccessTemporal(war, DomainKind::AllVertices, true, &info) == Temporal::Carried,
              "buffer certificate preserves unproved WAR conflicts");
    }
    info.DriverLoop = info.DriverLoop->getParentLoop();
    provePreviousRoundReads(info, di, li, aa, dt);
    check(info.BufferRoundReads.empty(), "buffer proof requires an enclosing round");
}

static void bufferCases() {
    for (StringRef mode : {"swap", "readonly", "phi", "copy", "memcpy", "global_swap", "copy_heap", "copy_direct"})
        bufferFixture(mode, true);
    for (StringRef mode : {"same_buffer", "phi_same_buffer", "copy_same_buffer", "one_way", "bypass",
                          "copy_bypass", "input_write", "phi_input_write", "copy_prefix_write",
                          "copy_in_pair", "copy_not_copy", "opaque", "phi_opaque", "copy_opaque",
                          "alias_write", "seed_write", "argwrite", "atomic", "volatile", "post_swap_load", "slot_escape"})
        bufferFixture(mode, false);
}
