// Exercise the original LLVM access sites without a runtime topology.
#include "../graph_frontier_lowering.cpp"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include <iostream>
static int failures;
static void check(bool value, const char *message) {
    if (!value) { std::cerr << "FAIL " << message << "\n"; ++failures; }
}
static SpatialDecision proof(StringRef body, bool noalias = false, bool snapshot = false) {
    LLVMContext C;
    std::string IR = "target datalayout = \"e-p:64:64-i64:64-n8:16:32:64-S128\"\n"
        "declare void @autograph_neighbor_iter_init(ptr, i64, ptr)\n"
        "declare i32 @autograph_neighbor_iter_next(ptr, ptr)\n"
        "define void @work(ptr %graph, ptr " + std::string(noalias ? "noalias " : "") +
        "%a, ptr " + std::string(noalias ? "noalias " : "") + "%b, i32 %n) {\n"
        "entry:\n %iter = alloca [64 x i8]\n %vslot = alloca i32\n br label %driver\n"
        "driver:\n %u = phi i32 [0, %entry], [%unext, %driver_latch]\n"
        " %test = icmp slt i32 %u, %n\n br i1 %test, label %source, label %exit\n"
        "source:\n %u64 = zext i32 %u to i64\n"
        " call void @autograph_neighbor_iter_init(ptr %graph, i64 %u64, ptr %iter)\n"
        " br label %neighbor\nneighbor:\n"
        " %next = call i32 @autograph_neighbor_iter_next(ptr %iter, ptr %vslot)\n"
        " %has = icmp ne i32 %next, 0\n br i1 %has, label %pair, label %driver_latch\n"
        "pair:\n %v = load i32, ptr %vslot\n" + body.str() +
        "\n br label %neighbor\ndriver_latch:\n %unext = add i32 %u, 1\n"
        " br label %driver\nexit:\n ret void\n}\n";
    SMDiagnostic Error;
    auto M = parseAssemblyString(IR, Error, C);
    if (!M) { Error.print("spatial_static_test", errs()); std::abort(); }
    Function &F = *M->getFunction("work");
    PassBuilder PB;
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
    LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
    AAResults &AA = FAM.getResult<AAManager>(F);
    DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
    NeighborLoopInfo Info;
    bool Found = false;
    for (Loop *L : LI.getLoopsInPreorder()) if (analyzeNeighborLoop(L, Info)) { Found = true; break; }
    check(Found, "canonical pair loop recognized");
    Info.RoundSepBases.clear(); // Test the original relation before choosing a snapshot contract.
    if (snapshot) {
        NeighborLoopInfo::RoundSepBase S;
        S.Base = F.getArg(1); S.ElemTy = Type::getInt32Ty(C);
        for (BasicBlock &B : F) for (Instruction &I : B)
            if (I.getName() == "left") S.CrossReads.insert(cast<LoadInst>(&I));
        Info.RoundSepBases.push_back(std::move(S));
    }
    proveSpatialPartition(Info, AA, DT);
    return Info.SpatialResult; // Only scalar fields and relation records are inspected.
}
int main() {
    LLVMContext Types;
    check(!foldSound(RedOp::FMinNum,Type::getDoubleTy(Types)) &&
          !foldSound(RedOp::FMaxNum,Type::getDoubleTy(Types)) &&
          !foldSound(RedOp::Min,Type::getDoubleTy(Types)),
          "float fold cannot discharge edges with an invalid NaN identity");
    auto source = proof("%p = getelementptr i32, ptr %a, i32 %u\n store i32 %v, ptr %p");
    check(source.Status == SpatialProofStatus::IndependentFamily && source.Group == SpatialGroup::Source,
          "one static source write still has dependencies across distinct occurrences");
    bool self = false;
    for (const auto &R : source.Relations) self |= R.From == R.To && R.Kind == AccessRelKind::WAW && R.Survives;
    check(self, "self-comparison retains repeated writes and duplicate-edge occurrences");
    auto destination = proof("%p = getelementptr i32, ptr %a, i32 %v\n store i32 %u, ptr %p");
    check(destination.Group == SpatialGroup::Destination, "destination equality contains every collision");
    auto disjoint = proof("%p = getelementptr i32, ptr %a, i32 %u\n %q = getelementptr i32, ptr %b, i32 %u\n store i32 %v, ptr %p\n store i32 %v, ptr %q", true);
    check(disjoint.Group == SpatialGroup::Source && disjoint.Accesses.size() >= 2, "same provenance does not merge access sites or disjoint bases");
    auto aliases = proof("%p = getelementptr i32, ptr %a, i32 %u\n %q = getelementptr i32, ptr %b, i32 %v\n store i32 %v, ptr %p\n store i32 %u, ptr %q");
    check(aliases.Status == SpatialProofStatus::Inconclusive, "different SSA pointers do not prove noalias");
    auto mixed = proof("%p = getelementptr i32, ptr %a, i32 %u\n %q = getelementptr i32, ptr %a, i32 %v\n store i32 %v, ptr %p\n store i32 %u, ptr %q");
    check(mixed.Status == SpatialProofStatus::Inconclusive, "unknown topology connectivity is not an exact no-partition proof");
    const char *example = "%p = getelementptr i32, ptr %a, i32 %u\n %q = getelementptr i32, ptr %a, i32 %v\n %left = load i32, ptr %p\n %right = load i32, ptr %q\n %sum = add i32 %left, %right\n store i32 %sum, ptr %q";
    auto live = proof(example);
    check(live.Status == SpatialProofStatus::Inconclusive, "x[u], x[v], write x[v] needs unknown topology connectivity when live");
    auto frozen = proof(example, false, true);
    check(frozen.Group == SpatialGroup::Destination, "covered snapshot removes cross-endpoint read collisions");
    bool keptWrite = false, removedRead = false;
    for (const auto &R : frozen.Relations) {
        keptWrite |= R.Kind == AccessRelKind::WAW && R.Survives;
        removedRead |= R.RemovedBy == DischargeKind::Snapshot && !R.Survives;
        check(!(R.Kind == AccessRelKind::WAW && R.RemovedBy == DischargeKind::Snapshot), "snapshot never discharges WAW");
    }
    check(keptWrite && removedRead, "snapshot contract is local to designated read sites");
    auto affine = proof("%i = add i32 %u, 1\n %p = getelementptr i32, ptr %a, i32 %i\n store i32 %v, ptr %p");
    check(affine.Group == SpatialGroup::Source, "injective affine index proves equal source binding");
    auto trunc = proof("%short = trunc i32 %u to i8\n %i = zext i8 %short to i32\n %p = getelementptr i32, ptr %a, i32 %i\n store i32 %v, ptr %p");
    check(trunc.Status == SpatialProofStatus::Inconclusive, "truncation does not prove injectivity");
    auto data = proof("%p = getelementptr i32, ptr %a, i32 %u\n %i = load i32, ptr %p\n %q = getelementptr i32, ptr %b, i32 %i\n store i32 %v, ptr %q", true);
    check(data.Status == SpatialProofStatus::Inconclusive, "data-derived indices retain possible collisions");
    auto constant = proof("%p = getelementptr i32, ptr %a, i32 0\n store i32 %v, ptr %p");
    check(constant.Status == SpatialProofStatus::NoPartition, "unconditional fixed-address write proves complete connection");
    std::cout << "static spatial proof: " << failures << " failures\n";
    return failures != 0;
}
