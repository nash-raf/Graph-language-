// Recognition tests use original LLVM CFGs, including incomplete lifecycles.
#include "../graph_frontier_lowering.cpp"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include <iostream>

static unsigned Failures = 0;
static std::string changeIR(std::string S, StringRef From, StringRef To) {
    size_t P = S.find(From.str());
    if (P == std::string::npos) { std::cerr << "missing fixture text\n"; std::abort(); }
    S.replace(P, From.size(), To.str());
    return S;
}

static const char *ArrayIR = R"(
declare void @autograph_neighbor_iter_init(ptr, i64, ptr)
declare i32 @autograph_neighbor_iter_next(ptr, ptr)
declare void @escape(ptr)
define void @work(ptr %graph, ptr %a, ptr %b, i1 %choose) {
entry:
 %current = alloca ptr
 %next = alloca ptr
 %size = alloca i32
 %count = alloca i32
 %iter = alloca [64 x i8]
 %vslot = alloca i32
 store ptr %a, ptr %current
 store ptr %b, ptr %next
 store i32 1, ptr %size
 store i32 0, ptr %count
 br label %round
round:
 %size0 = load i32, ptr %size
 %again = icmp sgt i32 %size0, 0
 br i1 %again, label %reset, label %exit
reset:
 store i32 0, ptr %count
 br label %driver
driver:
 %i = phi i32 [0, %reset], [%inext, %driver_latch]
 %limit = load i32, ptr %size
 %test = icmp slt i32 %i, %limit
 br i1 %test, label %source, label %publish
source:
 %cur = load ptr, ptr %current
 %srcp = getelementptr i32, ptr %cur, i32 %i
 %u = load i32, ptr %srcp
 %u64 = zext i32 %u to i64
 call void @autograph_neighbor_iter_init(ptr %graph, i64 %u64, ptr %iter)
 br label %neighbor
neighbor:
 %more = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %vslot)
 %has = icmp ne i32 %more, 0
 br i1 %has, label %pair, label %driver_latch
pair:
 %v = load i32, ptr %vslot
 %dst = load ptr, ptr %next
 %n = load i32, ptr %count
 %out = getelementptr i32, ptr %dst, i32 %n
 store i32 %v, ptr %out
 %n1 = add i32 %n, 1
 store i32 %n1, ptr %count
 br label %neighbor
driver_latch:
 %inext = add i32 %i, 1
 br label %driver
publish:
 %old = load ptr, ptr %current
 %new = load ptr, ptr %next
 store ptr %new, ptr %current
 store ptr %old, ptr %next
 %total = load i32, ptr %count
 store i32 %total, ptr %size
 br label %round_latch
round_latch:
 br label %round
exit:
 ret void
}
)";

static const char *SetIR = R"(
declare void @autograph_neighbor_iter_init(ptr, i64, ptr)
declare i32 @autograph_neighbor_iter_next(ptr, ptr)
declare ptr @roaring_bitmap_create(i64, i64)
declare i64 @roaring_bitmap_get_cardinality(ptr)
declare i32 @roaring_bitmap_get_at_index(ptr, i64)
declare void @roaring_bitmap_add(ptr, i32)
define void @work(ptr %graph, ptr %initial, i1 %choose) {
entry:
 %current = alloca ptr
 %next = alloca ptr
 %iter = alloca [64 x i8]
 %vslot = alloca i32
 store ptr %initial, ptr %current
 br label %round
round:
 %set = load ptr, ptr %current
 %size = call i64 @roaring_bitmap_get_cardinality(ptr %set)
 %again = icmp ugt i64 %size, 0
 br i1 %again, label %reset, label %exit
reset:
 %empty = call ptr @roaring_bitmap_create(i64 65536, i64 8)
 store ptr %empty, ptr %next
 %cur = load ptr, ptr %current
 %limit = call i64 @roaring_bitmap_get_cardinality(ptr %cur)
 br label %driver
driver:
 %i = phi i64 [0, %reset], [%inext, %driver_latch]
 %test = icmp ult i64 %i, %limit
 br i1 %test, label %source, label %publish
source:
 %u = call i32 @roaring_bitmap_get_at_index(ptr %cur, i64 %i)
 %u64 = zext i32 %u to i64
 call void @autograph_neighbor_iter_init(ptr %graph, i64 %u64, ptr %iter)
 br label %neighbor
neighbor:
 %more = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %vslot)
 %has = icmp ne i32 %more, 0
 br i1 %has, label %pair, label %driver_latch
pair:
 %v = load i32, ptr %vslot
 %dst = load ptr, ptr %next
 call void @roaring_bitmap_add(ptr %dst, i32 %v)
 br label %neighbor
driver_latch:
 %inext = add i64 %i, 1
 br label %driver
publish:
 %new = load ptr, ptr %next
 store ptr %new, ptr %current
 br label %round_latch
round_latch:
 br label %round
exit:
 ret void
}
)";

static void check(StringRef Name, const std::string &IR, bool Expected) {
    LLVMContext C;
    SMDiagnostic Error;
    auto M = parseAssemblyString(IR, Error, C);
    if (!M || verifyModule(*M, &errs())) {
        Error.print("frontier_lifecycle_test", errs()); std::abort();
    }
    Function &F = *M->getFunction("work");
    PassBuilder PB;
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;
    PB.registerModuleAnalyses(MAM); PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM); PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
    auto &LI = FAM.getResult<LoopAnalysis>(F);
    bool Found = false, Recognized = false, Activate = false, Unproven = false;
    for (Loop *L : LI.getLoopsInPreorder()) {
        NeighborLoopInfo Info;
        if (!analyzeNeighborLoop(L, Info)) continue;
        Found = true;
        Recognized = Info.HasFrontierAppend;
        Unproven = Info.UnprovenFrontierLifecycle;
        for (const auto &E : Info.Effects) Activate |= E.Kind == EffectKind::Activate;
        break;
    }
    bool OK = Found && Recognized == Expected && Activate == Expected && Unproven != Expected;
    if (!Expected) {
        // A rejected append must not reach an emitter or acquire a partial
        // frontier-storage rewrite. The original stores/calls remain intact.
        unsigned Before = F.getInstructionCount();
        GraphFrontierLoweringPass Pass;
        Pass.run(F, FAM);
        OK &= !verifyModule(*M, &errs()) && F.getInstructionCount() == Before;
        for (Loop *L : LI.getLoopsInPreorder())
            if (L->getHeader()->getName() == "neighbor" || L->getHeader()->getName() == "driver")
                OK &= L->getHeader()->getTerminator()->getMetadata("sgpl.frontier.nested.sequential") != nullptr;
    }
    std::cout << (OK ? "PASS " : "FAIL ") << Name.str() << '\n';
    if (!OK) {
        std::cerr << "found=" << Found << " recognized=" << Recognized
                  << " activate=" << Activate << " unproven=" << Unproven << '\n';
        ++Failures;
    }
}

int main() {
    check("array reset/append/swap/count/consume", ArrayIR, true);
    check("bitmap reset/add/publish/consume", SetIR, true);
    check("one-shot append has no next-round lifecycle", changeIR(ArrayIR,
          "round_latch:\n br label %round", "round_latch:\n ret void"), false);
    check("missing per-round counter reset", changeIR(ArrayIR,
          "reset:\n store i32 0, ptr %count", "reset:"), false);
    check("wrong counter reset", changeIR(ArrayIR,
          "reset:\n store i32 0, ptr %count", "reset:\n store i32 1, ptr %count"), false);
    check("ordinary append without publication", changeIR(ArrayIR,
          "store ptr %new, ptr %current", "; no publication"), false);
    check("missing next-size publication", changeIR(ArrayIR,
          "store i32 %total, ptr %size", "; no size publication"), false);
    check("wrong next-size publication", changeIR(ArrayIR,
          "store i32 %total, ptr %size", "store i32 1, ptr %size"), false);
    check("missing reciprocal buffer swap", changeIR(ArrayIR,
          "store ptr %old, ptr %next", "; no recycling"), false);
    check("aliased sequential assignments are not swap", changeIR(ArrayIR,
          "store ptr %old, ptr %next", "%alias = load ptr, ptr %current\n store ptr %alias, ptr %next"), false);
    auto Conditional = changeIR(ArrayIR, "label %source, label %publish",
                               "label %source, label %dispatch");
    Conditional = changeIR(Conditional, "publish:\n",
        "dispatch:\n br i1 %choose, label %publish, label %round_latch\npublish:\n");
    check("conditional publication bypass", Conditional, false);
    auto ConditionalReset = changeIR(ArrayIR, "label %reset, label %exit",
                                    "label %reset_dispatch, label %exit");
    ConditionalReset = changeIR(ConditionalReset, "reset:\n",
        "reset_dispatch:\n br i1 %choose, label %reset, label %skip_reset\n"
        "skip_reset:\n br label %driver\nreset:\n");
    ConditionalReset = changeIR(ConditionalReset, "[0, %reset],", "[0, %reset], [0, %skip_reset],");
    check("conditional reset bypass", ConditionalReset, false);
    check("ordinary payload", changeIR(ArrayIR, "store i32 %v, ptr %out",
          "store i32 42, ptr %out"), false);
    check("non-unit increment", changeIR(ArrayIR, "%n1 = add i32 %n, 1",
          "%n1 = add i32 %n, 2"), false);
    check("missing increment", changeIR(ArrayIR, "store i32 %n1, ptr %count",
          "; no increment"), false);
    check("wrong consumed array", changeIR(ArrayIR, "%cur = load ptr, ptr %current",
          "%cur = load ptr, ptr %next"), false);
    check("escaping frontier slot", changeIR(ArrayIR, "reset:\n",
          "reset:\n call void @escape(ptr %next)\n"), false);
    auto Duplicate = changeIR(ArrayIR, "store i32 %n1, ptr %count\n",
        "store i32 %n1, ptr %count\n %n2 = load i32, ptr %count\n"
        " %out2 = getelementptr i32, ptr %dst, i32 %n2\n store i32 %v, ptr %out2\n"
        " %n3 = add i32 %n2, 1\n store i32 %n3, ptr %count\n");
    check("duplicate vertex occurrences remain frontiers", Duplicate, true);
    check("bitmap without fresh empty set", changeIR(SetIR,
          "store ptr %empty, ptr %next", "; no reset"), false);
    check("ordinary bitmap collection", changeIR(SetIR,
          "store ptr %new, ptr %current", "; no publication"), false);
    check("non-vertex bitmap insertion", changeIR(SetIR,
          "@roaring_bitmap_add(ptr %dst, i32 %v)", "@roaring_bitmap_add(ptr %dst, i32 42)"), false);
    auto ConditionalSet = changeIR(SetIR, "label %source, label %publish",
                                 "label %source, label %dispatch");
    ConditionalSet = changeIR(ConditionalSet, "publish:\n",
        "dispatch:\n br i1 %choose, label %publish, label %round_latch\npublish:\n");
    check("conditional bitmap publication", ConditionalSet, false);
    auto Stale = changeIR(ArrayIR, "store i32 0, ptr %count\n br label %round",
        "store i32 0, ptr %count\n %stale = load ptr, ptr %next\n br label %round");
    Stale = changeIR(Stale, "%out = getelementptr i32, ptr %dst, i32 %n",
        "%out = getelementptr i32, ptr %stale, i32 %n");
    check("append through buffer cached before rounds", Stale, false);
    auto StaleSource = changeIR(ArrayIR, "store i32 0, ptr %count\n br label %round",
        "store i32 0, ptr %count\n %stale = load ptr, ptr %current\n br label %round");
    StaleSource = changeIR(StaleSource, "%srcp = getelementptr i32, ptr %cur, i32 %i",
        "%srcp = getelementptr i32, ptr %stale, i32 %i");
    check("source buffer cached before rounds", StaleSource, false);
    return Failures ? 1 : 0;
}
