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
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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
  Query
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
  Instruction *call = nullptr;
  Value *graphPtr = nullptr;
  double execMultiplier = 1.0;
};

struct Region {
  RegionType dominant = RegionType::Query;
  Instruction *anchor = nullptr;
  Value *graphPtr = nullptr;
  std::array<double, 3> freq = {0.0, 0.0, 0.0}; // T,I,Q
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

// Runtime-measured PCSR expansion factor: gap_slots = deg, row_cap = 2*deg
// (autotuner_runtime.c convert_csr_to_pcsr). g = row_cap / deg = 2.
constexpr double kPcsrExpansionFactor = 2.0;
// Runtime-measured BCSR block size (autotuner_runtime.c: bcsr_block_size = 64).
constexpr double kBcsrBlockSize = 64.0;

struct HwCalib {
  double L = 64.0; // cache-line size in bytes
  double t = 1.0;  // random-access (redirection) latency, in abstract time units
  double T = 1.0;  // cache-line transfer cost, in abstract time units
};

HwCalib loadHwCalib() {
  HwCalib hw;
  const char *path = std::getenv("AUTOTUNER_HW_CALIB");
  std::string fpath = path ? path : "";
  if (fpath.empty()) {
    // Default: ~/.config/sgpl/hw_calib.json
    const char *home = std::getenv("HOME");
    if (home)
      fpath = std::string(home) + "/.config/sgpl/hw_calib.json";
  }
  if (fpath.empty())
    return hw;
  FILE *f = std::fopen(fpath.c_str(), "r");
  if (!f)
    return hw;
  char buf[512];
  size_t n = std::fread(buf, 1, sizeof(buf) - 1, f);
  buf[n] = '\0';
  std::fclose(f);
  // Minimal JSON parse: look for "L", "t", "T" keys.
  auto extract = [&](const char *key) -> double {
    const char *p = std::strstr(buf, key);
    if (!p)
      return -1.0;
    p = std::strchr(p, ':');
    if (!p)
      return -1.0;
    return std::strtod(p + 1, nullptr);
  };
  double vL = extract("\"L\"");
  double vt = extract("\"t\"");
  double vT = extract("\"T\"");
  if (vL > 0) hw.L = vL;
  if (vt > 0) hw.t = vt;
  if (vT > 0) hw.T = vT;
  return hw;
}

int opIndex(RegionType t) {
  switch (t) {
  case RegionType::Traverse:
    return 0;
  case RegionType::Insert:
    return 1;
  case RegionType::Query:
    return 2;
  }
  return 2;
}

const char *regionTypeName(RegionType t) {
  switch (t) {
  case RegionType::Traverse:
    return "Traverse";
  case RegionType::Insert:
    return "Insert";
  case RegionType::Query:
    return "Query";
  }
  return "Query";
}

const char *layoutName(int layout) {
  switch (layout) {
  case LAYOUT_CSR:
    return "CSR";
  case LAYOUT_PCSR:
    return "PCSR";
  case LAYOUT_BCSR:
    return "BCSR";
  case LAYOUT_SET:
    return "SET";
  default:
    return "UNKNOWN";
  }
}

int forcedLayoutFromEnv() {
  const char *raw = std::getenv("AUTOTUNER_FORCE_LAYOUT");
  if (!raw || !*raw)
    return -1;
  std::string value(raw);
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
  if (value == "CSR")
    return LAYOUT_CSR;
  if (value == "PCSR")
    return LAYOUT_PCSR;
  if (value == "BCSR")
    return LAYOUT_BCSR;
  if (value == "SET")
    return LAYOUT_SET;
  return -1;
}

bool layoutFeasible(RegionType type, int layout) {
  if (layout < 0 || layout >= LAYOUT_COUNT)
    return false;
  switch (type) {
  case RegionType::Traverse:
    return true; // CSR, PCSR, BCSR, SET all feasible
  case RegionType::Insert:
    return true; // CSR, PCSR, BCSR, SET all feasible
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

// Query unit-cost coefficients (retained from old model for the fixed query term).
//                   CSR   PCSR  BCSR  SET
static constexpr double queryCostCoeff[LAYOUT_COUNT] = {1.2, 1.5, 2.0, 0.5};

// Traversal cost: |Active Set| x (C_nav + ceil(B_useful / L) x T)
// |Active Set| = n (every vertex becomes active once across a traversal region).
// d = 2m/n (undirected average degree).
// g = kPcsrExpansionFactor (measured: 2), b = kBcsrBlockSize (measured: 64).
double traversalCost(int layout, double n, double m, const HwCalib &hw) {
  const double d = (n > 0) ? (2.0 * m / n) : 1.0;
  const double g = kPcsrExpansionFactor;
  const double b = kBcsrBlockSize;
  const double t = hw.t;
  const double T = hw.T;
  const double L = hw.L;

  double cNav = 0.0;
  double bUseful = 0.0;

  switch (layout) {
  case LAYOUT_CSR:
    // row_ptr[u] + row_ptr[u+1] = 16 bytes; d col_idx entries x 4 bytes
    cNav = 2.0 * t;
    bUseful = 16.0 + 4.0 * d;
    break;
  case LAYOUT_PCSR:
    // Same nav as CSR; span = g*d cells scanned (gaps included)
    cNav = 2.0 * t;
    bUseful = 16.0 + 4.0 * d * g;
    break;
  case LAYOUT_BCSR:
    // brow[u/b] + brow[u/b+1] = 8 bytes (i32 ptrs); slice = 2*b*d ints (runtime:
    // (local_row,col) pairs, 2 ints per edge in the block row)
    cNav = 2.0 * t;
    bUseful = 8.0 + 8.0 * b * d;
    break;
  case LAYOUT_SET:
    // No source index → scan all m edge_pairs (16 bytes each). Punitive.
    cNav = 1.0 * t;
    bUseful = 16.0 * m;
    break;
  default:
    return kInf;
  }

  const double cTraffic = std::ceil(bUseful / L) * T;
  return n * (cNav + cTraffic);
}

// Insert/update cost: C_locate + C_write + C_move
double insertCost(int layout, double n, double m, const HwCalib &hw) {
  const double d = (n > 0) ? (2.0 * m / n) : 1.0;
  const double g = kPcsrExpansionFactor;
  const double t = hw.t;
  const double T = hw.T;
  const double L = hw.L;
  // Derived averages for the cost model
  const double u = n / 2.0;       // average source row index
  const double p = m / 2.0;       // average insertion position
  const double gU = g * d;        // g(u) = physical span of vertex u

  switch (layout) {
  case LAYOUT_CSR: {
    // C_locate = (2 + d(u)) * t
    const double cLocate = (2.0 + d) * t;
    // C_write = 1 + ceil((n-u)*8 / L) * t  (1 cacheline for the edge +
    //           prefix-sum update of row_ptr after u)
    const double cWrite = 1.0 + std::ceil((n - u) * 8.0 / L) * t;
    // C_move = ceil(4*(|E|-p) / L) * t  (shift col_idx after insertion point)
    const double cMove = std::ceil(4.0 * (m - p) / L) * t;
    return cLocate + cWrite + cMove;
  }
  case LAYOUT_PCSR: {
    // In-place gap fill — no memmove (cMove removed).
    // C_locate = (2 + g(u)) * t  (row_ptr reads + linear scan for gap)
    // C_write = 1 * t            (col_idx[j] = to)
    const double cLocate = (2.0 + gU) * t;
    const double cWrite = 1.0 * t;
    return cLocate + cWrite;
  }
  case LAYOUT_BCSR: {
    // Native BCSR mutation: insert (local_row, col) pair into bcol + update brow.
    //
    // C_locate = (2 + d_BR) * t
    //   2: read brow[blk], brow[blk+1] to find the block row's span
    //   d_BR = b*d: scan the block row's (local_row, col) pairs for duplicate
    //
    // C_write = 1 + ceil((nb - blk) * 4 / L) * t
    //   1: write the new pair (2 ints = 8 bytes, 1 cache line)
    //   ceil((nb - blk) * 4 / L) * t: update brow prefix sums for all
    //   subsequent block rows (nb - blk entries, each int32 = 4 bytes)
    //
    // C_move = ceil(4 * (2m - p_B) / L) * t
    //   Shift all bcol entries after the insertion point.
    //   (2m - p_B) ints to shift, each 4 bytes.
    //   p_B ≈ m (mid-range position in bcol, which has 2m ints)
    const double b = kBcsrBlockSize;
    const double nb = std::ceil(n / b);       // number of block rows
    const double d_BR = b * d;                // avg edges per block row
    const double blkIdx = n / (2.0 * b);      // mid-range block row index
    const double p_B = m;                     // mid-range insertion position in bcol

    const double cLocate = (2.0 + d_BR) * t;
    const double cWrite = 1.0 + std::ceil((nb - blkIdx) * 4.0 / L) * t;
    const double cMove = std::ceil(4.0 * (2.0 * m - p_B) / L) * t;
    return cLocate + cWrite + cMove;
  }
  case LAYOUT_SET: {
    // autograph_canonical_add_edge is significantly more expensive than the
    // PCSR/BCSR incremental paths.  The dominant cost comes from:
    //
    //   2× roaring_bitmap_add FFI calls on the nodes bitmap (container lookup
    //      + bit set — each is an external C→C++ call with internal binary
    //      search over containers and O(1) set_bit for bitmap containers).
    //
    //   3× hash-table probes (canonical_pair_find_static, find_extra, plus
    //      edge_hash_insert).  For large m the static hash table (~34·m bytes)
    //      exceeds cache and every probe is a DRAM miss.
    //
    //   extra_edge_pairs append + live-flag write (~1 cache line).
    //
    //   refresh_graph_counts_from_canonical (O(1): cached cardinality +
    //      select on nodes bitmap + two 8-byte header writes).
    //
    // Empirical fit on the synthetic graphs (n=5000) shows cost ≈ 1.3·m
    // ns per insert, consistent with ~40 bytes of random DRAM access per
    // edge in the static hash table (each probe touches ~17 bytes, 2–3
    // probes per insert).  Hence:
    //
    //   c_hash_traffic = ceil(m · 40 / L) · T
    //
    // plus a small constant floor for the bitmap operations.
    const double cHashTraffic = std::ceil(m * 40.0 / L) * T;
    const double cBase = 5.0 * t;
    // Array‑container regime (n ≤ 4096): each roaring_bitmap_add does a
    // binary‑search + memmove of ~n/2 uint16 entries.  Add that cost here.
    constexpr double kThreshold = 4096.0;
    double cArrayCost = 0.0;
    if (n <= kThreshold) {
      const double cShift = std::ceil((n / 2.0) * 2.0 / L) * T;
      // two adds × (search + shift)
      cArrayCost = 2.0 * (std::ceil(std::log2(n) * 2.0 / L) * T + cShift);
    }
    return cHashTraffic + cBase + cArrayCost;
  }
  default:
    return kInf;
  }
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

double operationCost(const Region &r, int layout, double n, double m,
                     const HwCalib &hw) {
  const double fT = r.freq[0];
  const double fI = r.freq[1];
  const double fQ = r.freq[2];
  const double H = std::max(1.0, r.execCount);
  const double safeM = std::max(2.0, m);
  const double totalOps = std::max(1.0, static_cast<double>(r.totalOps));

  // New per-op unit costs from the cache-line-aware cost model.
  const double uTrav = traversalCost(layout, n, m, hw);
  const double uIns = insertCost(layout, n, m, hw);
  // Query term: retained from old model (fixed behavior).
  const double Cq = queryCostCoeff[layout];

  return H * totalOps *
         (fT * uTrav + fI * uIns + fQ * Cq * std::log2(safeM));
}

double estimateAllCSRPathCost(const std::vector<Region> &regions, double estN,
                              double estM, const HwCalib &hw) {
  if (regions.empty())
    return 0.0;
  int current = LAYOUT_CSR;
  double total = 0.0;
  for (const Region &R : regions) {
    if (!layoutFeasible(R.dominant, LAYOUT_CSR))
      return kInf;
    total += conversionCost(current, LAYOUT_CSR, estN, estM);
    total += operationCost(R, LAYOUT_CSR, estN, estM, hw);
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
                                   double estM, const HwCalib &hw) {
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
    total += operationCost(regions[i], layout, estN, estM, hw);
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

LayoutSchedule solveDP(const std::vector<Region> &regions, double estN,
                       double estM, const HwCalib &hw) {
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
               operationCost(regions[0], l, estN, estM, hw);
  }

  for (int i = 1; i < R; ++i) {
    for (int cur = 0; cur < LAYOUT_COUNT; ++cur) {
      if (!layoutFeasible(regions[i].dominant, cur))
        continue;
      const double runCost = operationCost(regions[i], cur, estN, estM, hw);
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
    S.suffixCost[R - 1][l] = operationCost(regions[R - 1], l, estN, estM, hw);
  }
  for (int i = R - 2; i >= 0; --i) {
    for (int l = 0; l < LAYOUT_COUNT; ++l) {
      if (!layoutFeasible(regions[i].dominant, l))
        continue;
      const double here = operationCost(regions[i], l, estN, estM, hw);
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
  std::vector<Value *> initGraphPtrs;

  auto registerGraphValue = [&](Value *graphPtr, const GraphMeta &G) {
    metaByGraphPtr[graphPtr] = G;
    if (Value *stripped = graphPtr->stripPointerCasts()) {
      metaByGraphPtr[stripped] = G;
      if (auto *LI = dyn_cast<LoadInst>(stripped))
        metaByGraphPtr[LI->getPointerOperand()] = G;
    }
  };

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
      if (MDNode *sizeMD = CB->getMetadata("autotuner.graph_size")) {
        if (sizeMD->getNumOperands() >= 2) {
          auto getSizeConstant = [](Metadata *M) -> ConstantInt * {
            if (auto *CAM = dyn_cast_or_null<ConstantAsMetadata>(M))
              return dyn_cast<ConstantInt>(CAM->getValue());
            return nullptr;
          };
          if (auto *modelN = getSizeConstant(sizeMD->getOperand(0).get()))
            G.n = modelN;
          if (auto *modelM = getSizeConstant(sizeMD->getOperand(1).get()))
            G.m = modelM;
        }
      }
      G.nodesBmp = CB->getArgOperand(3);
      G.edgesBmp = CB->getArgOperand(4);
      G.edgePairs = CB->getArgOperand(5);
      registerGraphValue(G.graphPtr, G);
      initGraphPtrs.push_back(G.graphPtr->stripPointerCasts());
      found = true;
    }
  }

  if (!found)
    return false;

  // File-loaded graphs are stored in a global (e.g. @G); neighbor loops reload it.
  for (BasicBlock &BB : mainFn) {
    for (Instruction &I : BB) {
      auto *SI = dyn_cast<StoreInst>(&I);
      if (!SI)
        continue;
      Value *stored = SI->getValueOperand()->stripPointerCasts();
      for (Value *initPtr : initGraphPtrs) {
        if (stored != initPtr)
          continue;
        Value *slot = SI->getPointerOperand()->stripPointerCasts();
        const GraphMeta &G = metaByGraphPtr.at(initPtr);
        metaByGraphPtr[slot] = G;
      }
    }
  }

  return true;
}

bool classifyCall(StringRef name, RegionType &outType, bool &usesGraphArg0) {
  if (isTraverseCall(name)) {
    outType = RegionType::Traverse;
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

Value *resolveGraphRoot(Value *V, const std::map<Value *, GraphMeta> &metaByGraphPtr);

void collectOpEvents(Function &F, const std::map<Value *, GraphMeta> &metaByGraphPtr,
                     std::vector<OpEvent> &events) {
  Value *lastGraph = nullptr;

  std::map<Value *, Value *> storageToGraph;
  for (BasicBlock &BB : F) {
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

  // Helper to resolve a graph pointer from an instruction's operands.
  auto resolveGraphPtr = [&](Value *graphPtr) -> Value * {
    if (!graphPtr)
      return nullptr;
    if (metaByGraphPtr.count(graphPtr))
      return graphPtr;
    if (Value *stripped = graphPtr->stripPointerCasts())
      graphPtr = stripped;
    if (metaByGraphPtr.count(graphPtr))
      return graphPtr;
    if (Value *stripped = graphPtr->stripPointerCasts()) {
      if (auto *LI = dyn_cast<LoadInst>(stripped)) {
        Value *slot = LI->getPointerOperand()->stripPointerCasts();
        auto it = storageToGraph.find(slot);
        if (it != storageToGraph.end())
          return it->second;
      }
    }
    return nullptr;
  };

  // Helper to finalize graph pointer with fallbacks.
  auto finalizeGraphPtr = [&](Value *graphPtr) -> Value * {
    if (graphPtr && metaByGraphPtr.count(graphPtr)) {
      lastGraph = graphPtr;
      return graphPtr;
    }
    if (graphPtr && metaByGraphPtr.size() == 1) {
      graphPtr = metaByGraphPtr.begin()->first;
      lastGraph = graphPtr;
      return graphPtr;
    }
    if (!graphPtr && lastGraph && metaByGraphPtr.count(lastGraph))
      return lastGraph;
    if (!graphPtr && metaByGraphPtr.size() == 1) {
      graphPtr = metaByGraphPtr.begin()->first;
      lastGraph = graphPtr;
      return graphPtr;
    }
    // Multi-graph fallback: use the first graph registered from autograph_init
    if (!graphPtr && !metaByGraphPtr.empty()) {
      graphPtr = metaByGraphPtr.begin()->first;
      lastGraph = graphPtr;
      return graphPtr;
    }
    return nullptr;
  };

  for (BasicBlock &BB : F) {
    // Check for autotuner.traverse metadata on the BB terminator (foreach loops).
    if (Instruction *Term = BB.getTerminator()) {
      if (MDNode *MD = Term->getMetadata("autotuner.traverse")) {
        if (MD->getNumOperands() >= 1) {
          if (auto *MDS = dyn_cast<MDString>(MD->getOperand(0))) {
            // This BB is a foreach loop header — classify as Traverse.
            // Resolve the graph pointer by scanning the loop body for
            // a GetElementPtrInst into struct.Graph.
            Value *graphPtr = nullptr;
            for (BasicBlock *Succ : successors(&BB)) {
              for (Instruction &SI : *Succ) {
                if (auto *GEP = dyn_cast<GetElementPtrInst>(&SI)) {
                  if (Value *root = resolveGraphRoot(GEP->getPointerOperand(),
                                                      metaByGraphPtr)) {
                    graphPtr = root;
                    break;
                  }
                }
                // Check the graph pointer used in autograph_get_neighbors calls
                if (auto *CB = dyn_cast<CallBase>(&SI)) {
                  if (Function *Callee = resolveCallee(CB)) {
                    if (Callee->getName() == "autograph_get_neighbors" &&
                        CB->arg_size() > 0) {
                      graphPtr = resolveGraphRoot(CB->getArgOperand(0), metaByGraphPtr);
                      if (graphPtr)
                        break;
                    }
                  }
                }
                // Check LoadInsts from graph storage (e.g., load ptr from @G)
                if (auto *LI = dyn_cast<LoadInst>(&SI)) {
                  if (Value *root = resolveGraphRoot(LI, metaByGraphPtr)) {
                    graphPtr = root;
                    break;
                  }
                }
              }
              if (graphPtr)
                break;
            }
            // Also scan the header BB itself (for vertex loops, the GEP
            // to field 0 is in the header, not a successor)
            if (!graphPtr) {
              for (Instruction &SI : BB) {
                if (auto *GEP = dyn_cast<GetElementPtrInst>(&SI)) {
                  if (Value *root = resolveGraphRoot(GEP->getPointerOperand(),
                                                      metaByGraphPtr)) {
                    graphPtr = root;
                    break;
                  }
                }
              }
            }
            // Fallback: if still no graph ptr, try lastGraph or single-graph heuristic
            if (!graphPtr) {
              graphPtr = finalizeGraphPtr(nullptr);
            }

            graphPtr = finalizeGraphPtr(graphPtr);
            if (graphPtr && metaByGraphPtr.count(graphPtr)) {
              double mult = estimateExecMultiplier(&BB);
              events.push_back({RegionType::Traverse, Term, graphPtr, mult});
            }
            (void)MDS; // metadata value ("neighbor"/"edge"/"vertex") — all are Traverse
          }
        }
      }
    }

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
        graphPtr = resolveGraphPtr(CB->getArgOperand(0));
      else
        graphPtr = lastGraph;

      graphPtr = finalizeGraphPtr(graphPtr);
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
  std::array<uint64_t, 3> counts = {0, 0, 0};

  auto flushRegion = [&]() {
    if (cur.totalOps == 0)
      return;
    for (int i = 0; i < 3; ++i)
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
      counts = {0, 0, 0};
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
  // No region type is CSR-only anymore: traversal runtimes are layout-aware
  // and the DP may choose PCSR/BCSR/SET for any Traverse or Insert region.
  (void)t;
  return false;
}

std::vector<Region> mergeSmallRegions(std::vector<Region> &&raw) {
  const char *strictRegions = std::getenv("AUTOTUNER_PROFILE_STRICT_REGIONS");
  if (strictRegions && std::string(strictRegions) == "1")
    return std::move(raw);
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
        for (int k = 0; k < 3; ++k)
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
                      double estM, const HwCalib &hw) {
  if (regions.empty() || S.chosen.size() != regions.size() || S.suffixCost.empty())
    return 0;

  LLVMContext &Ctx = M.getContext();
  Type *i64Ty = Type::getInt64Ty(Ctx);
  Type *i32Ty = Type::getInt32Ty(Ctx);
  Type *doubleTy = Type::getDoubleTy(Ctx);
  Type *ptrTy = PointerType::getUnqual(Ctx);

  auto ensureTy = FunctionType::get(Type::getVoidTy(Ctx),
                                    {ptrTy, i64Ty, i64Ty, ptrTy, ptrTy, ptrTy, ptrTy, ptrTy,
                                     i32Ty},
                                    false);
  FunctionCallee ensureFn = M.getOrInsertFunction("autograph_ensure_layout", ensureTy);
  auto profileEnterTy = FunctionType::get(Type::getVoidTy(Ctx),
                                          {i32Ty, i32Ty, i32Ty, doubleTy},
                                          false);
  auto profileExitTy =
      FunctionType::get(Type::getVoidTy(Ctx), {i32Ty}, false);
  FunctionCallee profileEnterFn =
      M.getOrInsertFunction("autograph_profile_region_enter", profileEnterTy);
  FunctionCallee profileExitFn =
      M.getOrInsertFunction("autograph_profile_region_exit", profileExitTy);

  int injected = 0;
  const int forcedLayout = forcedLayoutFromEnv();
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
    const int regionId = static_cast<int>(i);

    IRBuilder<> B(R.anchor);
    if (i > 0 && regions[i - 1].anchor &&
        regions[i - 1].anchor->getFunction() == R.anchor->getFunction()) {
      B.CreateCall(profileExitFn, {ConstantInt::get(i32Ty, static_cast<int>(i - 1))});
    }

    bool mustGuard = isCSROnlyRegion(R.dominant) && target == LAYOUT_CSR;
    bool shouldSwitch = true;
    if (!mustGuard && target == current)
      shouldSwitch = false;
    if (!mustGuard && forcedLayout < 0 && R.totalOps < kMinRegionOpsToSwitch)
      shouldSwitch = false;
    if (!mustGuard && forcedLayout < 0) {
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
        shouldSwitch = false;
    }

    const int actualLayout = shouldSwitch ? target : current;
    const double predictedNs = operationCost(R, actualLayout, estN, estM, hw);

    if (shouldSwitch) {
      Value *nullPtr = ConstantPointerNull::get(cast<PointerType>(ptrTy));
      Value *layoutV = ConstantInt::get(i32Ty, target);
      B.CreateCall(ensureFn, {G.graphPtr, G.n, G.m, nullPtr, nullPtr, G.nodesBmp, G.edgesBmp,
                              G.edgePairs, layoutV});
      effectiveLayoutByGraph[R.graphPtr] = target;
      ++injected;
    }

    B.CreateCall(profileEnterFn,
                 {ConstantInt::get(i32Ty, regionId),
                  ConstantInt::get(i32Ty, opIndex(R.dominant)),
                  ConstantInt::get(i32Ty, actualLayout),
                  ConstantFP::get(doubleTy, predictedNs)});
  }

  std::map<Function *, int> lastRegionByFunction;
  for (size_t i = 0; i < regions.size(); ++i)
    if (regions[i].anchor)
      lastRegionByFunction[regions[i].anchor->getFunction()] = static_cast<int>(i);

  for (const auto &KV : lastRegionByFunction) {
    Function *F = KV.first;
    const int regionId = KV.second;
    for (BasicBlock &BB : *F) {
      if (auto *RI = dyn_cast<ReturnInst>(BB.getTerminator())) {
        IRBuilder<> B(RI);
        B.CreateCall(profileExitFn, {ConstantInt::get(i32Ty, regionId)});
      }
    }
  }
  return injected;
}

Value *resolveGraphRoot(Value *V, const std::map<Value *, GraphMeta> &metaByGraphPtr) {
  for (int depth = 0; depth < 12 && V; ++depth) {
    V = V->stripPointerCasts();
    if (metaByGraphPtr.count(V))
      return V;
    if (auto *LI = dyn_cast<LoadInst>(V)) {
      V = LI->getPointerOperand();
      continue;
    }
    if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
      V = GEP->getPointerOperand();
      continue;
    }
    if (auto *AI = dyn_cast<AllocaInst>(V)) {
      if (metaByGraphPtr.count(AI))
        return AI;
      break;
    }
    break;
  }
  return nullptr;
}

bool moduleRequiresCSRLayout(Module &M,
                             const std::map<Value *, GraphMeta> &metaByGraphPtr) {
  auto csrRuntimeCall = [](StringRef fn) {
    return fn == "graph_get_edge_weight" || fn == "graph_get_edge_weight_by_id" ||
           fn == "graph_get_edge_src_by_id" || fn == "graph_get_edge_dst_by_id";
  };

  for (Function &F : M) {
    if (F.isDeclaration())
      continue;
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (auto *CB = dyn_cast<CallBase>(&I)) {
          if (Function *Callee = resolveCallee(CB)) {
            if (csrRuntimeCall(Callee->getName()))
              return true;
          }
        }

        auto *GEP = dyn_cast<GetElementPtrInst>(&I);
        if (!GEP)
          continue;
        if (!resolveGraphRoot(GEP->getPointerOperand(), metaByGraphPtr))
          continue;

        // Opaque-pointer IRGen: row_ptr +16, col_idx +24, weights +32 bytes.
        if (GEP->getNumIndices() == 1) {
          if (auto *byteOff = dyn_cast<ConstantInt>(GEP->getOperand(2))) {
            const int64_t off = byteOff->getSExtValue();
            if (off == 16 || off == 24 || off == 32)
              return true;
          }
          continue;
        }

        if (GEP->getNumIndices() < 2)
          continue;
        auto idxIt = GEP->idx_begin();
        ++idxIt;
        auto *fieldIdx = dyn_cast<ConstantInt>(idxIt);
        if (!fieldIdx)
          continue;
        const int field = static_cast<int>(fieldIdx->getSExtValue());
        if (field == 2 || field == 3 || field == 4)
          return true;
      }
    }
  }
  return false;
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

  const bool pinCSR = moduleRequiresCSRLayout(M, metaByGraphPtr);

  // Load hardware calibration (cache-line size, access/transfer costs).
  const HwCalib hw = loadHwCalib();
  const int forcedLayout = forcedLayoutFromEnv();

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

    LayoutSchedule S = solveDP(regions, estN, estM, hw);
    if (forcedLayout >= 0) {
      for (size_t i = 0; i < S.chosen.size(); ++i) {
        if (layoutFeasible(regions[i].dominant, forcedLayout))
          S.chosen[i] = forcedLayout;
      }
      errs() << "[AutoTuner] Forced layout override: "
             << layoutName(forcedLayout) << "\n";
    }
    const double chosenCost = estimateChosenScheduleCost(regions, S.chosen, estN, estM, hw);
    const double allCSR = estimateAllCSRPathCost(regions, estN, estM, hw);

    bool shouldSkip = false;
    if (forcedLayout < 0 && chosenCost < kInf / 2.0 && allCSR < kInf / 2.0) {
      const double required = allCSR / kMinBenefitRatio;
      if (!(chosenCost < required)) {
        shouldSkip = true;
      }
    }
    bool allCSRSchedule = true;
    for (int c : S.chosen) {
      if (c != LAYOUT_CSR) { allCSRSchedule = false; break; }
    }
    if (forcedLayout < 0 && allCSRSchedule) {
      shouldSkip = true;
    }
    // Neighbor loops read row_ptr/col_idx directly from the Graph struct; keep CSR.
    if (forcedLayout < 0 && pinCSR) {
      shouldSkip = true;
    }
    if (shouldSkip)
      std::fill(S.chosen.begin(), S.chosen.end(), LAYOUT_CSR);
    errs() << "[AutoTuner] Cost-model profile for graph " << graphKey
           << " (estN=" << estN << ", estM=" << estM << ")\n";
    for (size_t i = 0; i < regions.size(); ++i) {
      const double predictedNs =
          operationCost(regions[i], S.chosen[i], estN, estM, hw);
      errs() << "[AutoTuner]   region=" << i
             << " kind=" << regionTypeName(regions[i].dominant)
             << " layout=" << layoutName(S.chosen[i])
             << " totalOps=" << regions[i].totalOps
             << " execCount=" << regions[i].execCount
             << " predicted_ns=" << predictedNs
             << " predicted_ms=" << (predictedNs / 1.0e6) << "\n";
    }
    totalInjected += injectConversions(M, regions, S, metaByGraphPtr, estN, estM, hw);
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
