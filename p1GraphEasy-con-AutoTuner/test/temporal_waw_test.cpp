#include "../graph_frontier_lowering.cpp"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include <iostream>

static unsigned failures;
static void check(bool condition, const char *message) {
    if (!condition) { ++failures; std::cerr << "FAIL " << message << '\n'; }
}

static void labelsAndProjection() {
    for (DomainKind domain : {DomainKind::Frontier, DomainKind::AllVertices})
        for (Region a : {Region::U, Region::V, Region::D, Region::G, Region::Top})
            for (Region b : {Region::U, Region::V, Region::D, Region::G, Region::Top})
                for (PhaseSegment sa : {PhaseSegment::Preamble, PhaseSegment::Pair, PhaseSegment::Epilogue})
                    for (PhaseSegment sb : {PhaseSegment::Preamble, PhaseSegment::Pair, PhaseSegment::Epilogue}) {
                        Footprint source, sink;
                        source.Reg = a; source.Seg = sa; source.Modes = FP_WRITE;
                        sink.Reg = b; sink.Seg = sb; sink.Modes = FP_WRITE;
                        AccessRelation r;
                        r.Kind = AccessRelKind::WAW;
                        r.SourceRegion = a; r.SinkRegion = b;
                        r.SourceSegment = sa; r.SinkSegment = sb;
                        r.SourceFootprint = &source; r.SinkFootprint = &sink;
                        check(relateAccessTemporal(r, domain, true) == Temporal::Unknown,
                              "repeated writes require a round proof for every region and phase");
                        check(relateAccessTemporal(r, domain, false) ==
                              (sa == sb ? Temporal::SameRoundWrite : Temporal::SameRoundOrdered),
                              "one traversal retains write conflicts and phase order");
                        NeighborLoopInfo info;
                        r.TemporalClass = Temporal::Unknown;
                        r.Spatial = OwnerClass::CrossOwner;
                        info.AccessRelations.push_back(r);
                        buildProofGraphs(info);
                        check(!graphAxisVerdict(info.Graphs).NIT,
                              "unknown offset cannot make the round projection clean");
                        check(info.Graphs.Round.Edges[0].Survives &&
                              info.Graphs.Round.Edges[0].From != info.Graphs.Round.Edges[0].To,
                              "unknown offsets retain distinct symbolic rounds");
                        check(info.Graphs.Spatial.Edges[0].Survives,
                              "unknown round separation cannot erase spatial conflicts");
                        info.AccessRelations[0].DischargedBy = DischargeKind::Privatization;
                        buildProofGraphs(info);
                        check(info.Graphs.Round.Edges[0].Survives &&
                              !graphAxisVerdict(info.Graphs).NIT &&
                              info.Graphs.Round.Edges[0].DischargedBy == DischargeKind::None,
                              "a spatial discharge cannot prove separation across rounds");
                        check(!info.Graphs.Spatial.Edges[0].Survives,
                              "the same discharge still applies within its spatial scope");
                    }

    Footprint write, read;
    write.Reg = Region::V; write.Seg = PhaseSegment::Pair;
    read.Reg = Region::U; read.Seg = PhaseSegment::Pair;
    AccessRelation r;
    r.Kind = AccessRelKind::RAW; r.SourceRegion = Region::V; r.SinkRegion = Region::U;
    r.SourceFootprint = &write; r.SinkFootprint = &read;
    check(relateAccessTemporal(r, DomainKind::Frontier, true) == Temporal::PreviousRoundRead,
          "legacy RAW read projection remains available");
    r.Kind = AccessRelKind::WAR; r.SourceFootprint = &read; r.SinkFootprint = &write;
    check(relateAccessTemporal(r, DomainKind::AllVertices, true) == Temporal::Carried,
          "legacy WAR read projection remains available");
}

static void completeRounds(StringRef mode) {
    LLVMContext context;
    std::string target = mode == "shared" ? "@data" : "%storage";
    std::string ir = R"(
target datalayout = "e-p:64:64-i64:64-n8:16:32:64-S128"
@data = global [4 x i32] zeroinitializer
declare ptr @load_graph_from_file_fixture()
declare void @autograph_neighbor_iter_init(ptr, i64, ptr)
declare i32 @autograph_neighbor_iter_next(ptr, ptr)
define void @work() {
entry:
 %g = call ptr @load_graph_from_file_fixture()
 br label %round
round:
 %r = phi i32 [0, %entry], [%rn, %suffix]
 %more = icmp slt i32 %r, 4
 br i1 %more, label %prefix, label %exit
prefix:
 %iter = alloca [128 x i8]
 %vslot = alloca i32
)";
    if (mode == "private") ir += " %storage = alloca [4 x i32]\n";
    else if (mode == "indexed") ir += " %storage = getelementptr [4 x i32], ptr @data, i32 0, i32 %r\n";
    ir += R"(
 br label %driver
driver:
 %u = phi i64 [0, %prefix], [%un, %latch]
 %has = icmp slt i64 %u, 4
 br i1 %has, label %source, label %suffix
source:
 call void @autograph_neighbor_iter_init(ptr %g, i64 %u, ptr %iter)
 br label %neighbor
neighbor:
 %next = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %vslot)
 %hasv = icmp ne i32 %next, 0
 br i1 %hasv, label %pair, label %latch
pair:
 %v = load i32, ptr %vslot
)";
    ir += " store i32 %r, ptr " + target + "\n store i32 %v, ptr " + target + "\n";
    ir += R"(
 br label %neighbor
latch:
 %un = add i64 %u, 1
 br label %driver
suffix:
 %rn = add i32 %r, 1
 br label %round
exit:
 ret void
})";
    SMDiagnostic error;
    auto module = parseAssemblyString(ir, error, context);
    if (!module) { error.print("temporal_waw", errs()); ++failures; return; }
    PassBuilder pb;
    LoopAnalysisManager lam; FunctionAnalysisManager fam;
    CGSCCAnalysisManager cgam; ModuleAnalysisManager mam;
    pb.registerModuleAnalyses(mam); pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam); pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);
    auto &function = *module->getFunction("work");
    auto &li = fam.getResult<LoopAnalysis>(function);
    auto &aa = fam.getResult<AAManager>(function);
    auto &dt = fam.getResult<DominatorTreeAnalysis>(function);
    NeighborLoopInfo info;
    bool found = false;
    for (Loop *loop : li.getLoopsInPreorder()) if (analyzeNeighborLoop(loop, info)) { found = true; break; }
    check(found, "complete round fixture is recognized");
    if (!found) return;
    RoundDomain rounds;
    proveRounds(info, rounds, aa, dt);
    if (mode == "shared") {
        check(info.TemporalResult.Rounds.Status != TemporalProofStatus::Independent,
              "shared writes cannot authorize round concurrency");
        check(llvm::any_of(info.TemporalResult.Rounds.Relations,
                          [](const TemporalDependency &d) { return d.Basis == "round-WAW"; }),
              "complete analysis records shared WAW");
    } else {
        check(info.TemporalResult.Rounds.Status == TemporalProofStatus::Independent,
              "complete proof accepts isolated storage or disjoint round indices");
    }
    // Check a lone write site. There is no second static write to pair with it.
    info.Effects.clear(); info.ResourceFootprints.clear();
    Effect effect; effect.Kind = EffectKind::W;
    Footprint fp; fp.Base = module->getGlobalVariable("data");
    fp.Modes = FP_WRITE; fp.Reg = Region::V;
    effect.Footprints.push_back(fp); info.Effects.push_back(effect);
    deriveAccessRelations(info);
    check(info.AccessRelations.size() == 1, "one repeated write generates a self-site WAW relation");
    if (!info.AccessRelations.empty()) {
        const auto &r = info.AccessRelations.front();
        check(r.Kind == AccessRelKind::WAW && r.TemporalClass == Temporal::Unknown &&
              r.Relation.K == InstanceRelation::Unknown && r.RoundOffset == 0 &&
              r.PhaseRel == PhaseRelation::Unknown && !r.SourcedFromOrder,
              "self-site WAW has no invented offset or instruction order");
    }
}

int main() {
    labelsAndProjection();
    for (StringRef mode : {"shared", "private", "indexed"}) completeRounds(mode);
    std::cout << "temporal WAW: " << failures << " failures\n";
    return failures != 0;
}
