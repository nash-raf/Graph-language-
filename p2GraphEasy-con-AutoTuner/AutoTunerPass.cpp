#include "AutoTunerPass.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;

namespace {

enum Layout : int {
  LAYOUT_CSR = 0,
  LAYOUT_PCSR = 1,
  LAYOUT_BCSR = 2,
  LAYOUT_SET = 3,
  LAYOUT_COUNT = 4
};

enum class RegionType {
  Traverse,
  Insert,
  Query,
  Compute
};

struct GraphMeta {
  Value *graphPtr = nullptr;
  Value *n = nullptr;
  Value *m = nullptr;
  Value *nodesBmp = nullptr;
  Value *edgesBmp = nullptr;
  Value *edgePairs = nullptr;
};

struct OpEvent {
  RegionType type;
  CallBase *call = nullptr;
  Value *graphPtr = nullptr;
  double execMultiplier = 1.0;
};

struct Region {
  RegionType dominant = RegionType::Query;
  CallBase *anchor = nullptr;
  Value *graphPtr = nullptr;
  std::array<double, 4> freq = {0.0, 0.0, 0.0, 0.0}; // T,I,Q,C
  uint64_t totalOps = 0;
  double execCount = 1.0;
};

struct LayoutSchedule {
  std::vector<int> chosen;                     // per region, layout id [0..3]
  std::vector<std::vector<double>> suffixCost; // [region][layout]
};

constexpr double kInf = 1.0e100;
constexpr double kSafetyMargin = 1.05;
constexpr uint64_t kMinRegionOpsToSwitch = 2;
// Only switch when model predicts at least 1.2x benefit over staying baseline.
constexpr double kMinBenefitRatio = 1.2;

int opIndex(RegionType t) {
  switch (t) {
  case RegionType::Traverse:
    return 0;
  case RegionType::Insert:
    return 1;
  case RegionType::Query:
    return 2;
  case RegionType::Compute:
    return 3;
  }
  return 2;
}

bool layoutFeasible(RegionType type, int layout) {
  if (layout < 0 || layout >= LAYOUT_COUNT)
    return false;
  switch (type) {
  case RegionType::Traverse:
  case RegionType::Compute:
    return layout == LAYOUT_CSR;
  case RegionType::Insert:
    return layout == LAYOUT_CSR || layout == LAYOUT_PCSR || layout == LAYOUT_SET;
  case RegionType::Query:
    return layout == LAYOUT_CSR || layout == LAYOUT_SET;
  }
  return false;
}

bool hasPrefix(StringRef s, StringRef pref) { return s.starts_with(pref); }

bool isTraverseCall(StringRef fn) {
  return fn == "bfs_runtime" || fn == "bfs_runtime_src" || fn == "dfs_runtime" ||
         fn == "dfs_runtime_src";
}

bool isComputeCall(StringRef fn) {
  return fn == "floyd_runtime" || fn == "bk_runtime" || fn == "dijkstra_runtime_src" ||
         fn == "chromacity_runtime" || fn == "karger_runtime";
}

bool isInsertCall(StringRef fn) {
  return fn == "graph_add_node" || fn == "graph_add_edge" ||
         fn == "graph_remove_node" || fn == "graph_remove_edge" ||
         hasPrefix(fn, "roaring_bitmap_add") || hasPrefix(fn, "roaring_bitmap_remove");
}

bool isQueryCall(StringRef fn) {
  return fn == "roaring_bitmap_contains" || fn == "roaring_bitmap_get_cardinality" ||
         fn == "roaring_bitmap_union" || fn == "roaring_bitmap_intersect" ||
         fn == "numVertices_runtime" ||
         fn == "numEdges_runtime";
}

double unitCostCoeff(int opIdx, int layout) {
  //                  CSR    PCSR   BCSR    SET
  // Traverse:        1.0    1.8    1.5     80.0   (CSR = sequential scan)
  // Insert:         50.0    1.2    80.0     0.8   (CSR = O(E) realloc, PCSR = O(deg) gap, SET = O(1) bitmap)
  // Query:           1.2    1.5    2.0      0.5   (SET = direct bitmap lookup)
  // Compute:         1.0    2.0    1.5     80.0   (CSR = dense arrays)
  static constexpr double C[4][4] = {
      {1.0,  1.8,  1.5,  80.0},
      {50.0, 1.2,  80.0,  0.8},
      {1.2,  1.5,  2.0,   0.5},
      {1.0,  2.0,  1.5,  80.0}
  };
  return C[opIdx][layout];
}

double alphaFromCSR(int to) {
  // Cost of converting FROM CSR TO the target layout.
  // CSR→SET:  ~O(1) — just free CSR arrays, bitmaps already canonical
  // CSR→PCSR: O(E) — allocate padded arrays, copy with gap slots
  // CSR→BCSR: O(E) — block-structured copy
  switch (to) {
  case LAYOUT_CSR:
    return 0.0;
  case LAYOUT_PCSR:
    return 1.5;
  case LAYOUT_BCSR:
    return 2.0;
  case LAYOUT_SET:
    return 0.1;
  default:
    return 1.0;
  }
}

double alphaToCSR(int from) {
  // Cost of converting TO CSR FROM the source layout.
  // SET→CSR:  O(E·log) — full rebuild from bitmaps (hash map + sort + edge iteration)
  // PCSR→CSR: O(E) — simple gap compaction (linear scan, no hashing)
  // BCSR→CSR: O(E) — unblock and reconstruct
  switch (from) {
  case LAYOUT_CSR:
    return 0.0;
  case LAYOUT_PCSR:
    return 0.5;
  case LAYOUT_BCSR:
    return 1.5;
  case LAYOUT_SET:
    return 3.0;
  default:
    return 1.0;
  }
}

double conversionCost(int from, int to, double n, double m) {
  if (from == to)
    return 0.0;
  const double scale = n + m;
  if (from == LAYOUT_CSR)
    return alphaFromCSR(to) * scale;
  if (to == LAYOUT_CSR)
    return alphaToCSR(from) * scale;
  // Route non-CSR transitions through CSR hub.
  return (alphaToCSR(from) + alphaFromCSR(to)) * scale;
}

double operationCost(const Region &r, int layout, double n, double m) {
  const double fT = r.freq[0];
  const double fI = r.freq[1];
  const double fQ = r.freq[2];
  const double fS = r.freq[3];
  const double H = std::max(1.0, r.execCount);
  const double safeM = std::max(2.0, m);
  // Im models per-mutation cost:
  //   CSR: O(sqrt(m)) amortized (realloc + memmove of partial array + row_ptr update)
  //   PCSR/SET: O(1) (gap-fill or bitmap add)
  const double Im = (layout == LAYOUT_CSR || layout == LAYOUT_BCSR) ? std::sqrt(m) : 1.0;
  const double Ct = unitCostCoeff(0, layout);
  const double Ci = unitCostCoeff(1, layout);
  const double Cq = unitCostCoeff(2, layout);
  const double Cs = unitCostCoeff(3, layout);
  (void)n;
  const double totalOps = std::max(1.0, static_cast<double>(r.totalOps));
  return H * totalOps * (fT * Ct * m + fI * Ci * Im + fQ * Cq * std::log2(safeM) + fS * Cs * m);
}

double estimateAllCSRPathCost(const std::vector<Region> &regions, double estN, double estM) {
  if (regions.empty())
    return 0.0;
  int current = LAYOUT_CSR;
  double total = 0.0;
  for (const Region &R : regions) {
    if (!layoutFeasible(R.dominant, LAYOUT_CSR))
      return kInf;
    total += conversionCost(current, LAYOUT_CSR, estN, estM);
    total += operationCost(R, LAYOUT_CSR, estN, estM);
    current = LAYOUT_CSR;
  }
  return total;
}

Function *resolveCallee(CallBase *CB) {
  if (!CB)
    return nullptr;
  if (Function *F = CB->getCalledFunction())
    return F;
  Value *called = CB->getCalledOperand();
  if (!called)
    return nullptr;
  called = called->stripPointerCasts();
  return dyn_cast<Function>(called);
}

double estimateChosenScheduleCost(const std::vector<Region> &regions,
                                  const std::vector<int> &chosen, double estN,
                                  double estM) {
  if (regions.empty() || chosen.size() != regions.size())
    return kInf;
  int current = LAYOUT_CSR;
  double total = 0.0;
  for (size_t i = 0; i < regions.size(); ++i) {
    const int layout = chosen[i];
    if (layout < 0 || layout >= LAYOUT_COUNT)
      return kInf;
    if (!layoutFeasible(regions[i].dominant, layout))
      return kInf;
    total += conversionCost(current, layout, estN, estM);
    total += operationCost(regions[i], layout, estN, estM);
    current = layout;
  }
  return total;
}

std::pair<double, double> estimateGraphSize(const GraphMeta &meta) {
  double estN = 50000.0;
  double estM = 500000.0;
  if (auto *cn = dyn_cast_or_null<ConstantInt>(meta.n))
    estN = std::max(1.0, static_cast<double>(cn->getSExtValue()));
  if (auto *cm = dyn_cast_or_null<ConstantInt>(meta.m))
    estM = std::max(1.0, static_cast<double>(cm->getSExtValue()));
  return {estN, estM};
}

LayoutSchedule solveDP(const std::vector<Region> &regions, double estN, double estM) {
  LayoutSchedule S;
  const int R = static_cast<int>(regions.size());
  if (R == 0)
    return S;

  std::vector<std::array<double, LAYOUT_COUNT>> dp(R);
  std::vector<std::array<int, LAYOUT_COUNT>> parent(R);
  for (int i = 0; i < R; ++i) {
    for (int l = 0; l < LAYOUT_COUNT; ++l) {
      dp[i][l] = kInf;
      parent[i][l] = -1;
    }
  }

  // DP base: graph starts in CSR (loaded by graph_loader_runtime).
  // The autotuner may convert away from CSR for the first region.
  for (int l = 0; l < LAYOUT_COUNT; ++l) {
    if (!layoutFeasible(regions[0].dominant, l))
      continue;
    dp[0][l] = conversionCost(LAYOUT_CSR, l, estN, estM) +
               operationCost(regions[0], l, estN, estM);
  }

  for (int i = 1; i < R; ++i) {
    for (int cur = 0; cur < LAYOUT_COUNT; ++cur) {
      if (!layoutFeasible(regions[i].dominant, cur))
        continue;
      const double runCost = operationCost(regions[i], cur, estN, estM);
      for (int prev = 0; prev < LAYOUT_COUNT; ++prev) {
        if (dp[i - 1][prev] >= kInf / 2.0)
          continue;
        const double cand =
            dp[i - 1][prev] + conversionCost(prev, cur, estN, estM) + runCost;
        if (cand < dp[i][cur]) {
          dp[i][cur] = cand;
          parent[i][cur] = prev;
        }
      }
    }
  }

  int bestLast = LAYOUT_SET;
  double bestVal = kInf;
  for (int l = 0; l < LAYOUT_COUNT; ++l) {
    if (dp[R - 1][l] < bestVal) {
      bestVal = dp[R - 1][l];
      bestLast = l;
    }
  }

  S.chosen.assign(R, LAYOUT_SET);
  int cur = bestLast;
  for (int i = R - 1; i >= 0; --i) {
    S.chosen[i] = cur;
    cur = parent[i][cur];
    if (i > 0 && cur < 0)
      cur = LAYOUT_SET;
  }

  // Reverse horizon DP for guarded conversion decisions.
  S.suffixCost.assign(R, std::vector<double>(LAYOUT_COUNT, kInf));
  for (int l = 0; l < LAYOUT_COUNT; ++l) {
    if (!layoutFeasible(regions[R - 1].dominant, l))
      continue;
    S.suffixCost[R - 1][l] = operationCost(regions[R - 1], l, estN, estM);
  }
  for (int i = R - 2; i >= 0; --i) {
    for (int l = 0; l < LAYOUT_COUNT; ++l) {
      if (!layoutFeasible(regions[i].dominant, l))
        continue;
      const double here = operationCost(regions[i], l, estN, estM);
      double tail = kInf;
      for (int nxt = 0; nxt < LAYOUT_COUNT; ++nxt) {
        if (S.suffixCost[i + 1][nxt] >= kInf / 2.0)
          continue;
        tail = std::min(tail, conversionCost(l, nxt, estN, estM) + S.suffixCost[i + 1][nxt]);
      }
      S.suffixCost[i][l] = (tail >= kInf / 2.0) ? here : here + tail;
    }
  }

  return S;
}

bool collectGraphMeta(Function &mainFn, std::map<Value *, GraphMeta> &metaByGraphPtr) {
  bool found = false;
  for (BasicBlock &BB : mainFn) {
    for (Instruction &I : BB) {
      auto *CB = dyn_cast<CallBase>(&I);
      if (!CB)
        continue;
      Function *Callee = resolveCallee(CB);
      if (!Callee)
        continue;
      if (Callee->getName() != "autograph_init" || CB->arg_size() < 6)
        continue;

      GraphMeta G;
      G.graphPtr = CB->getArgOperand(0);
      G.n = CB->getArgOperand(1);
      G.m = CB->getArgOperand(2);
      G.nodesBmp = CB->getArgOperand(3);
      G.edgesBmp = CB->getArgOperand(4);
      G.edgePairs = CB->getArgOperand(5);
      metaByGraphPtr[G.graphPtr] = G;
      if (Value *stripped = G.graphPtr->stripPointerCasts())
        metaByGraphPtr[stripped] = G;
      found = true;
    }
  }
  return found;
}

bool classifyCall(StringRef name, RegionType &outType, bool &usesGraphArg0) {
  if (isTraverseCall(name)) {
    outType = RegionType::Traverse;
    usesGraphArg0 = true;
    return true;
  }
  if (isComputeCall(name)) {
    outType = RegionType::Compute;
    usesGraphArg0 = true;
    return true;
  }
  if (isInsertCall(name)) {
    outType = RegionType::Insert;
    usesGraphArg0 = name == "graph_add_node" || name == "graph_add_edge" ||
                    name == "graph_remove_node" || name == "graph_remove_edge";
    return true;
  }
  if (isQueryCall(name)) {
    outType = RegionType::Query;
    usesGraphArg0 = false;
    return true;
  }
  return false;
}

double estimateExecMultiplier(BasicBlock *BB) {
  constexpr double kDefaultTripCount = 8.0;
  double mult = 1.0;
  SmallPtrSet<BasicBlock *, 8> visited;
  BasicBlock *cur = BB;
  while (cur && visited.insert(cur).second) {
    bool isLoopBody = false;
    for (BasicBlock *succ : successors(cur)) {
      for (BasicBlock *pred : predecessors(cur)) {
        if (pred == succ) {
          isLoopBody = true;
          break;
        }
      }
      if (isLoopBody) break;
    }
    for (BasicBlock *pred : predecessors(cur)) {
      auto *TI = pred->getTerminator();
      if (auto *BI = dyn_cast<BranchInst>(TI)) {
        if (BI->isConditional()) {
          for (BasicBlock *s : successors(pred)) {
            if (s == cur) {
              for (BasicBlock *pp : predecessors(pred)) {
                if (pp == cur || pp == BB) {
                  isLoopBody = true;
                  break;
                }
              }
            }
            if (isLoopBody) break;
          }
        }
      }
      if (isLoopBody) break;
    }
    if (isLoopBody)
      mult *= kDefaultTripCount;
    if (cur->hasNPredecessorsOrMore(1))
      cur = *pred_begin(cur);
    else
      break;
  }
  return std::max(1.0, mult);
}

void collectOpEvents(Function &mainFn, const std::map<Value *, GraphMeta> &metaByGraphPtr,
                     std::vector<OpEvent> &events) {
  Value *lastGraph = nullptr;

  std::map<Value *, Value *> storageToGraph;
  for (BasicBlock &BB : mainFn) {
    for (Instruction &I : BB) {
      auto *SI = dyn_cast<StoreInst>(&I);
      if (!SI)
        continue;
      Value *storedV = SI->getValueOperand();
      if (!storedV)
        continue;
      if (metaByGraphPtr.count(storedV)) {
        Value *slot = SI->getPointerOperand()->stripPointerCasts();
        storageToGraph[slot] = storedV;
      }
    }
  }

  for (BasicBlock &BB : mainFn) {
    for (Instruction &I : BB) {
      auto *CB = dyn_cast<CallBase>(&I);
      if (!CB)
        continue;
      Function *Callee = resolveCallee(CB);
      if (!Callee)
        continue;
      StringRef name = Callee->getName();
      RegionType ty;
      bool usesGraphArg0 = false;
      if (!classifyCall(name, ty, usesGraphArg0))
        continue;

      Value *graphPtr = nullptr;
      if (usesGraphArg0 && CB->arg_size() > 0)
        graphPtr = CB->getArgOperand(0);

      // Normalize graph pointer SSA values to improve matching against autograph_init keys.
      if (graphPtr && !metaByGraphPtr.count(graphPtr)) {
        if (Value *stripped = graphPtr->stripPointerCasts())
          graphPtr = stripped;
      }

      if (graphPtr && !metaByGraphPtr.count(graphPtr)) {
        Value *stripped = graphPtr->stripPointerCasts();
        if (auto *LI = dyn_cast<LoadInst>(stripped)) {
          Value *slot = LI->getPointerOperand()->stripPointerCasts();
          auto it = storageToGraph.find(slot);
          if (it != storageToGraph.end())
            graphPtr = it->second;
        }
      }

      if (graphPtr && metaByGraphPtr.count(graphPtr)) {
        lastGraph = graphPtr;
      } else if (graphPtr && !metaByGraphPtr.count(graphPtr) && metaByGraphPtr.size() == 1) {
        // Some pipelines carry graph through aliases that do not map 1:1 in IR values.
        graphPtr = metaByGraphPtr.begin()->first;
        lastGraph = graphPtr;
      } else if (!graphPtr && lastGraph && metaByGraphPtr.count(lastGraph)) {
        graphPtr = lastGraph;
      } else if (!graphPtr && metaByGraphPtr.size() == 1) {
        // Single-graph programs often have bitmap-only ops with no explicit graph arg.
        graphPtr = metaByGraphPtr.begin()->first;
        lastGraph = graphPtr;
      } else {
        continue; // strict provenance: skip unresolved/unknown graph pointers
      }

      if (!graphPtr || !metaByGraphPtr.count(graphPtr))
        continue;
      double mult = estimateExecMultiplier(CB->getParent());
      events.push_back({ty, CB, graphPtr, mult});
    }
  }
}

std::vector<Region> buildRegions(const std::vector<OpEvent> &events) {
  std::vector<Region> regions;
  if (events.empty())
    return regions;

  Region cur;
  cur.dominant = events[0].type;
  cur.anchor = events[0].call;
  cur.graphPtr = events[0].graphPtr;
  std::array<uint64_t, 4> counts = {0, 0, 0, 0};

  auto flushRegion = [&]() {
    if (cur.totalOps == 0)
      return;
    for (int i = 0; i < 4; ++i)
      cur.freq[i] = static_cast<double>(counts[i]) / static_cast<double>(cur.totalOps);
    regions.push_back(cur);
  };

  for (const OpEvent &ev : events) {
    if (ev.type != cur.dominant) {
      flushRegion();
      cur = Region{};
      cur.dominant = ev.type;
      cur.anchor = ev.call;
      cur.graphPtr = ev.graphPtr;
      counts = {0, 0, 0, 0};
    }
    const int idx = opIndex(ev.type);
    counts[idx]++;
    cur.totalOps++;
    cur.execCount = std::max(cur.execCount, ev.execMultiplier);
  }
  flushRegion();
  return regions;
}

bool isCSROnlyRegion(RegionType t) {
  return t == RegionType::Traverse || t == RegionType::Compute;
}

std::vector<Region> mergeSmallRegions(std::vector<Region> &&raw) {
  if (raw.size() <= 1)
    return std::move(raw);

  constexpr uint64_t kMergeThreshold = 4;
  std::vector<Region> merged;
  merged.push_back(raw[0]);

  for (size_t i = 1; i < raw.size(); ++i) {
    Region &prev = merged.back();
    const Region &cur = raw[i];

    bool prevCSROnly = isCSROnlyRegion(prev.dominant);
    bool curCSROnly = isCSROnlyRegion(cur.dominant);
    if (prevCSROnly != curCSROnly) {
      merged.push_back(cur);
      continue;
    }

    bool canMerge = false;
    if (cur.dominant == prev.dominant)
      canMerge = true;
    else if (cur.totalOps <= 1 && prev.totalOps >= cur.totalOps)
      canMerge = true;
    else if (prev.totalOps <= 1 && cur.totalOps >= prev.totalOps) {
      prev.dominant = cur.dominant;
      prev.anchor = cur.anchor;
      canMerge = true;
    }
    else if (prev.totalOps < kMergeThreshold && cur.totalOps < kMergeThreshold) {
      prev.dominant = (prev.totalOps >= cur.totalOps) ? prev.dominant : cur.dominant;
      canMerge = true;
    }

    if (canMerge) {
      uint64_t newTotal = prev.totalOps + cur.totalOps;
      if (newTotal > 0) {
        for (int k = 0; k < 4; ++k)
          prev.freq[k] = (prev.freq[k] * prev.totalOps + cur.freq[k] * cur.totalOps) /
                          static_cast<double>(newTotal);
      }
      prev.totalOps = newTotal;
      prev.execCount = std::max(prev.execCount, cur.execCount);
    } else {
      merged.push_back(cur);
    }
  }
  return merged;
}

int injectConversions(Module &M, const std::vector<Region> &regions, const LayoutSchedule &S,
                      const std::map<Value *, GraphMeta> &metaByGraphPtr, double estN,
                      double estM) {
  if (regions.empty() || S.chosen.size() != regions.size() || S.suffixCost.empty())
    return 0;

  LLVMContext &Ctx = M.getContext();
  Type *i64Ty = Type::getInt64Ty(Ctx);
  Type *i32Ty = Type::getInt32Ty(Ctx);
  Type *ptrTy = PointerType::getUnqual(Ctx);

  auto ensureTy = FunctionType::get(Type::getVoidTy(Ctx),
                                    {ptrTy, i64Ty, i64Ty, ptrTy, ptrTy, ptrTy, ptrTy, ptrTy,
                                     i32Ty},
                                    false);
  FunctionCallee ensureFn = M.getOrInsertFunction("autograph_ensure_layout", ensureTy);

  int injected = 0;
  std::map<Value *, int> effectiveLayoutByGraph;
  for (const auto &KV : metaByGraphPtr)
    effectiveLayoutByGraph[KV.first] = LAYOUT_CSR;

  for (size_t i = 0; i < regions.size(); ++i) {
    const Region &R = regions[i];
    auto itMeta = metaByGraphPtr.find(R.graphPtr);
    if (itMeta == metaByGraphPtr.end())
      continue;

    const GraphMeta &G = itMeta->second;
    const int target = S.chosen[i];
    const int current = effectiveLayoutByGraph[R.graphPtr];

    bool mustGuard = isCSROnlyRegion(R.dominant) && target == LAYOUT_CSR;
    if (!mustGuard && target == current)
      continue;
    if (!mustGuard && R.totalOps < kMinRegionOpsToSwitch)
      continue;

    if (!mustGuard) {
      const double stay = (current >= 0 && current < LAYOUT_COUNT)
                              ? S.suffixCost[i][current]
                              : kInf;
      double conv = conversionCost(current, target, estN, estM);
      if (current != LAYOUT_CSR || target != LAYOUT_CSR) {
        const double uncertainty =
            (estM < 100000.0) ? 1.3 : ((estM < 500000.0) ? 1.15 : 1.05);
        conv *= uncertainty;
      }
      const double sw = kSafetyMargin * conv + S.suffixCost[i][target];
      const double required = stay / kMinBenefitRatio;
      if (!(sw < required))
        continue;
    }

    IRBuilder<> B(R.anchor);
    Value *nullPtr = ConstantPointerNull::get(cast<PointerType>(ptrTy));
    Value *layoutV = ConstantInt::get(i32Ty, target);
    B.CreateCall(ensureFn, {G.graphPtr, G.n, G.m, nullPtr, nullPtr, G.nodesBmp, G.edgesBmp,
                            G.edgePairs, layoutV});
    effectiveLayoutByGraph[R.graphPtr] = target;
    ++injected;
  }
  return injected;
}

} // namespace

PreservedAnalyses AutoTunerModulePass::run(Module &M, ModuleAnalysisManager &MAM) {
  (void)MAM;

  Function *mainFn = M.getFunction("main");
  if (!mainFn || mainFn->isDeclaration())
    return PreservedAnalyses::all();

  std::map<Value *, GraphMeta> metaByGraphPtr;
  if (!collectGraphMeta(*mainFn, metaByGraphPtr)) {
    return PreservedAnalyses::all();
  }

  std::vector<OpEvent> allEvents;
  for (Function &F : M) {
    if (F.isDeclaration())
      continue;
    collectOpEvents(F, metaByGraphPtr, allEvents);
  }
  if (allEvents.empty()) {
    return PreservedAnalyses::all();
  }

  // Build per-graph event sequences while preserving order.
  std::map<Value *, std::vector<OpEvent>> eventsByGraph;
  for (const OpEvent &E : allEvents) {
    if (metaByGraphPtr.count(E.graphPtr))
      eventsByGraph[E.graphPtr].push_back(E);
  }

  int totalInjected = 0;
  for (auto &KV : eventsByGraph) {
    const Value *graphKey = KV.first;
    std::vector<OpEvent> &events = KV.second;
    if (events.empty())
      continue;

    const auto metaIt = metaByGraphPtr.find(const_cast<Value *>(graphKey));
    if (metaIt == metaByGraphPtr.end())
      continue;
    auto [estN, estM] = estimateGraphSize(metaIt->second);
    std::vector<Region> regions = mergeSmallRegions(buildRegions(events));
    if (regions.empty())
      continue;

    LayoutSchedule S = solveDP(regions, estN, estM);
    const double chosenCost = estimateChosenScheduleCost(regions, S.chosen, estN, estM);
    const double allCSR = estimateAllCSRPathCost(regions, estN, estM);

    bool shouldSkip = false;
    if (chosenCost < kInf / 2.0 && allCSR < kInf / 2.0) {
      const double required = allCSR / kMinBenefitRatio;
      if (!(chosenCost < required)) {
        shouldSkip = true;
      }
    }
    bool allCSRSchedule = true;
    for (int c : S.chosen) {
      if (c != LAYOUT_CSR) { allCSRSchedule = false; break; }
    }
    if (allCSRSchedule) {
      shouldSkip = true;
    }
    if (!shouldSkip)
      totalInjected += injectConversions(M, regions, S, metaByGraphPtr, estN, estM);
  }

  (void)totalInjected;
  return PreservedAnalyses::none();
}

void runAutoTunerOnModule(Module &M) {
  ModuleAnalysisManager MAM;
  AutoTunerModulePass P;
  (void)P.run(M, MAM);
}

void registerAutoTunerPass(PassBuilder &PB) {
  PB.registerPipelineStartEPCallback(
      [](ModulePassManager &MPM, OptimizationLevel) { MPM.addPass(AutoTunerModulePass()); });
}
