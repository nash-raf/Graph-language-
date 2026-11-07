// Original LLVM instructions drive the proof, independently of endpoint labels.
#include "../graph_frontier_lowering.cpp"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include <iostream>

static unsigned failures;
static void check(bool condition, const std::string &message) {
    if (!condition) { ++failures; std::cerr << "FAIL " << message << '\n'; }
}

static void fixture(StringRef mode, int distance, bool expected) {
    LLVMContext context;
    std::string ir = R"(
target datalayout = "e-p:64:64-i64:64-n8:16:32:64-S128"
@data = global [64 x i32] zeroinitializer
@other = global [64 x i32] zeroinitializer
@index = global i32 4
declare void @autograph_neighbor_iter_init(ptr, i64, ptr) memory(argmem: readwrite)
declare i32 @autograph_neighbor_iter_next(ptr, ptr) memory(argmem: readwrite)
declare void @opaque()
declare void @readonly_call() memory(read)
declare void @argwrite(ptr) memory(argmem: write)
define void @work(ptr noalias %graph, ptr %alias, i1 %gate) {
entry:
 %iter = alloca [128 x i8]
 %vslot = alloca i32
)";
    if (mode == "outer") ir += R"(
 br label %outer
outer:
 %q = phi i32 [0, %entry], [%qn, %outer_latch]
 br label %round
)";
    else ir += " br label %round\n";
    ir += "round:\n %r = phi i32 [4, %" + std::string(mode == "outer" ? "outer" : "entry") +
          "], [%rn, %suffix]\n %more = icmp slt i32 %r, 10\n"
          " br i1 %more, label %prefix, label %" +
          std::string(mode == "outer" ? "outer_latch" : "exit") + "\nprefix:\n";
    ir += " %prev = sub i32 %r, " + std::to_string(distance) + "\n";
    if (mode == "indirect") ir += " %idx = load i32, ptr @index\n";
    std::string index = mode == "indirect" ? "%idx" : "%prev";
    ir += " %rp = getelementptr [64 x i32], ptr @data, i32 0, i32 " + index + "\n";
    if (mode == "preamble") ir += " store i32 7, ptr %rp\n";
    ir += R"(
 br label %driver
driver:
 %u = phi i64 [0, %prefix], [%un, %latch]
 %has = icmp slt i64 %u, 4
 br i1 %has, label %source, label %suffix
source:
 call void @autograph_neighbor_iter_init(ptr %graph, i64 %u, ptr %iter)
 br label %neighbor
neighbor:
 %next = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %vslot)
 %hasv = icmp ne i32 %next, 0
 br i1 %hasv, label %pair, label %latch
pair:
 %v = load i32, ptr %vslot
 %read = load i32, ptr %rp
 %wp = getelementptr [64 x i32], ptr @data, i32 0, i32 %r
 store i32 %read, ptr %wp
)";
    if (mode == "same_round") ir += " store i32 9, ptr %rp\n";
    if (mode == "opaque") ir += " call void @opaque()\n";
    if (mode == "readonly") ir += " call void @readonly_call()\n";
    if (mode == "argwrite") ir += " call void @argwrite(ptr %wp)\n";
    if (mode == "alias") ir += " store i32 9, ptr %alias\n";
    if (mode == "disjoint") ir += " store i32 9, ptr @other\n";
    if (mode == "atomic") ir += " store atomic i32 9, ptr @other seq_cst, align 4\n";
    if (mode == "conditional") ir += R"(
 br i1 %gate, label %conditional_write, label %resume
conditional_write:
 store i32 9, ptr %rp
 br label %resume
resume:
)";
    ir += R"(
 br label %neighbor
latch:
 %un = add i64 %u, 1
 br label %driver
suffix:
)";
    if (mode == "epilogue") ir += " store i32 11, ptr %rp\n";
    ir += " %rn = add i32 %r, 1\n br label %round\n";
    if (mode == "outer") ir += R"(
outer_latch:
 %qn = add i32 %q, 1
 %again = icmp slt i32 %qn, 2
 br i1 %again, label %outer, label %exit
)";
    ir += "exit:\n ret void\n}\n";
    if (mode == "opaque_iterator") {
        const std::string attributes = " memory(argmem: readwrite)";
        size_t at;
        while ((at = ir.find(attributes)) != std::string::npos)
            ir.erase(at, attributes.size());
    }
    SMDiagnostic error;
    auto module = parseAssemblyString(ir, error, context);
    if (!module) { error.print("previous_round_read", errs()); ++failures; return; }
    check(!verifyModule(*module, &errs()), "fixture verifies: " + mode.str());
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
    check(found, "neighbor loop recognized: " + mode.str());
    if (!found) return;
    Effect *read = nullptr;
    for (Effect &e : info.Effects)
        if (e.Kind == EffectKind::R && e.Origin && e.Origin->getName() == "read") read = &e;
    check(read != nullptr, "round-indexed read has an effect atom: " + mode.str());
    if (!read) return;
    check(read->Temp != Temporal::PreviousRoundRead, "no proof before analysis");
    provePreviousRoundReads(info, di, li, aa, dt);
    deriveAllTemporal(info);
    check(info.EarlierRoundReads.count(read->Origin) == unsigned(expected),
          "direction proof: " + mode.str() + " distance=" + std::to_string(distance));
    check((read->Temp == Temporal::PreviousRoundRead) == expected,
          "atom projection: " + mode.str());
    if (!expected)
        check(read->Temp == Temporal::Unknown,
              "unproved non-endpoint read remains unknown: " + mode.str());

    // Membership has no role in the proof, and changing endpoint labels must
    // not manufacture an earlier-round proof.
    info.Expr.RoundDomain = DomainKind::Frontier;
    read->Reg = Region::U;
    for (Effect &e : info.Effects)
        if (e.Kind != EffectKind::R && e.Base == read->Base) e.Reg = Region::V;
    deriveAllTemporal(info);
    check(read->Temp == (expected ? Temporal::PreviousRoundRead : Temporal::Unknown),
          "frontier flag cannot replace evidence: " + mode.str());
    buildFootprints(info);
    deriveAccessRelations(info);
    bool earlier = false;
    for (const AccessRelation &r : info.AccessRelations) {
        if (r.TemporalClass != Temporal::PreviousRoundRead) continue;
        earlier = true;
        auto direction = r.Kind == AccessRelKind::WAR ? InstanceRelation::LaterInstance
                                                     : InstanceRelation::EarlierInstance;
        check(r.CrossRound && r.RoundOffset == 0 &&
              r.Relation.K == direction && r.Relation.Offset == 0,
              "earlier-round relation has direction but no invented distance");
    }
    check(earlier == expected, "relation projection: " + mode.str());
    if (expected) {
        buildProofGraphs(info);
        for (const ProofEdge &edge : info.Graphs.Spatial.Edges)
            if (edge.TemporalClass == Temporal::PreviousRoundRead)
                check(edge.CrossRound && !edge.CrossPartition && !edge.Survives,
                      "proven earlier relation is projected across rounds, not partitions");
    }
    // No enclosing round is never proof of an earlier round.
    info.DriverLoop = info.DriverLoop->getParentLoop();
    if (mode != "outer") {
        provePreviousRoundReads(info, di, li, aa, dt);
        check(info.EarlierRoundReads.empty(), "missing round produces no proof");
    }
}

#include "buffer_round_read_cases.inc"
#include "carried_access_cases.inc"

int main() {
    carriedCases();
    bufferCases();
    fixture("ordinary", 1, true);
    fixture("ordinary", 3, true);
    fixture("ordinary", 0, false);
    fixture("ordinary", -1, false);
    fixture("preamble", 1, false);
    fixture("epilogue", 1, false);
    fixture("same_round", 1, false);
    fixture("opaque", 1, false);
    fixture("readonly", 1, true);
    fixture("argwrite", 1, false);
    fixture("alias", 1, false);
    fixture("conditional", 1, false);
    fixture("opaque_iterator", 1, false);
    fixture("disjoint", 1, true);
    fixture("atomic", 1, false);
    fixture("indirect", 1, false);
    fixture("outer", 1, false);
    std::cout << "previous round read: " << failures << " failures\n";
    return failures != 0;
}
