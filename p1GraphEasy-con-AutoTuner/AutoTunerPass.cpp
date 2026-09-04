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
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;

namespace
{

  enum Layout : int
  {
    LAYOUT_CSR = 0,
    LAYOUT_PCSR = 1,
    LAYOUT_BCSR = 2,
    LAYOUT_SET = 3,
    LAYOUT_COUNT = 4
  };

  // Cache-model selection (exactly one active; AUTOTUNER_CACHE_MODEL):
  //   Hybrid    -> legacy structural equations + class-tier weighted
  //                per-line rates on the memory-penalty terms (DEFAULT,
  //                production cost model)
  //   Legacy    -> footprint-only memPenalty (no RD correction)
  //   Aware     -> legacy + cache-aware penalty F(W,h2,h3) (diagnostics)
  //   ClassTier -> previous experimental class x residency rate model
  //                (diagnostics only)
  enum class CacheModel
  {
    Hybrid,
    Legacy,
    Aware,
    ClassTier
  };

  static CacheModel g_cacheModel = CacheModel::Hybrid;

  static CacheModel cacheModelFromEnv()
  {
    const char *raw = std::getenv("AUTOTUNER_CACHE_MODEL");
    if (raw && *raw)
    {
      std::string value(raw);
      std::transform(value.begin(), value.end(), value.begin(),
                     [](unsigned char c)
                     { return static_cast<char>(std::tolower(c)); });
      if (value == "legacy")
        return CacheModel::Legacy;
      if (value == "aware")
        return CacheModel::Aware;
      if (value == "class_tier")
        return CacheModel::ClassTier;
    }
    return CacheModel::Hybrid;
  }

  // Class-tier weighted per-line rate: h2·r_L2 + (h3−h2)·r_L3 + (1−h3)·r_DRAM
  // (the RD/MRC residency fractions applied to a measured per-tier rate).
  double weightedRate(double h2, double h3, const double rate[3])
  {
    return h2 * rate[0] + (h3 - h2) * rate[1] + (1.0 - h3) * rate[2];
  }

  enum class RegionType
  {
    Traverse,
    Insert,
    SetQuery,
    CSRQuery
  };

  struct GraphMeta
  {
    Value *graphPtr = nullptr;
    Value *n = nullptr;
    Value *m = nullptr;
    Value *nodesBmp = nullptr;
    Value *edgesBmp = nullptr;
    Value *edgePairs = nullptr;
    // Exact expected CSR/BCSR memmove-tail fractions (from the graph's real
    // degree vector, computed at compile time by IRGenVisitor and delivered
    // via the autotuner.shift_fracs metadata).  1.0 = blanket half-array
    // heuristic.  Mirrors shift_fractions() in cost_model.py.
    double csrFrac = 1.0;
    double bcsrFrac = 1.0;
    // Analytic per-class RD tiers (Phase 3): per directed insert
    // (N_s, h2_s, h3_s) for {scan, move, brow, struct}, delivered via the
    // autotuner.class_tiers metadata (IRGenVisitor::estimateClassTiers,
    // mirror of analytic_rd.py).  Replaces the BCSR insert line-traffic
    // terms (T/2T/Tm/P) with class × residency rates.
    std::array<double, 12> classTiers = {};
    bool hasClassTiers = false;
    // CSR analytic per-class RD tiers (Phase 3 extension): same payload for
    // the CSR structure ({scan,move,brow,struct} with scan unused), via the
    // autotuner.class_tiers_csr metadata (estimateCsrClassTiers, mirror of
    // AnalyticCSR).  Replaces the CSR insert line-traffic terms.
    std::array<double, 12> csrClassTiers = {};
    bool hasCsrClassTiers = false;
    // (degree stats removed — uniform freshProb only needs n and m)
  };

  struct OpEvent
  {
    RegionType type;
    Instruction *call = nullptr;
    Value *graphPtr = nullptr;
    double execMultiplier = 1.0;
  };

  struct Region
  {
    RegionType dominant = RegionType::SetQuery;
    Instruction *anchor = nullptr;
    Value *graphPtr = nullptr;
    std::array<double, 4> freq = {0.0, 0.0, 0.0, 0.0}; // T,I,SetQ,CSRQ
    uint64_t totalOps = 0;
    double execCount = 1.0;
  };

  struct LayoutSchedule
  {
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

  struct HwCalib
  {
    double L = 64.0;    // cache-line size in bytes
    double t = 1.0;     // random-access (redirection) latency, in abstract time units
    double T = 1.0;     // cache-line transfer cost, in abstract time units
    double R = 10000.0; // per-direction realloc cap (ns); heap copy vs O(1) page remap threshold
    // Measured by hw_calib_bench (with conservative fallbacks):
    double LLC = 8.0 * 1024.0 * 1024.0; // last-level-cache capacity in bytes
    // L2 capacity in bytes (1.25 MiB; matches rd_hist.L2_LINES * 64 and the
    // cost_model.py L2_BYTES used by the cache-aware penalty F).
    double l2Bytes = 1310720.0;
    double Tm = 2.5;                    // per-line cost of a cache-resident overlapping memmove
                                        // (read + write per line; distinct from read-only T)
    double P = 4.0;                     // DRAM-bound memmove penalty vs cache-resident (per line)
    // Measured ramp band [ramp_lo, ramp_hi] (bytes) within which the memmove
    // penalty transitions from cache-resident (1.0) to DRAM-saturated (P).
    // Measured by hw_calib_bench's working-set sweep.  Negative = absent from
    // JSON, in which case memPenalty falls back to the LLC/2 .. 2*LLC band.
    double ramp_lo = -1.0;
    double ramp_hi = -1.0;
    double h = 40.0;                    // ns per edge_hash_insert during a bulk hash build
                                        // (legacy scalar; used as h_dram fallback when h_cache
                                        //  and h_dram are absent from the JSON)
    double h_cache = -1.0;              // ns per edge_hash_insert when the hash table fits in LLC
                                        // (measured by hw_calib_bench at ~LLC/4 table bytes)
    double h_dram = -1.0;               // ns per edge_hash_insert when the table is DRAM-resident
                                        // (measured by hw_calib_bench at ~4*LLC table bytes)
    double c = 15.0;                    // ns per sequential-id bitmap contains check
    // Measured memmove per-line cost curve (cost_model.py _CURVE): a
    // log-spaced working-set sweep [ws_bytes, per_line_ns] emitted by
    // hw_calib_bench.  When present (>= 2 points), memPenalty interpolates
    // this instead of the [ramp_lo, ramp_hi] two-point band.
    std::vector<std::pair<double, double>> memmoveCurve;
    // Per-class per-tier ns/line rates (class_calib.c sweep): the analytic
    // BCSR insert cost (autotuner.class_tiers metadata) charges each access
    // class's line traffic at the rate of its RD tier.  scan = sequential
    // read stream, move/brow = overlapping R-M-W shift, struct = random
    // chase.  Defaults are this machine's measured class_calib.json values;
    // hw_calib.json "class_rates": {"seq": [L2,L3,DRAM], "rmw": [...],
    // "rand": [...], "dep": [...]} overrides them.
    double seqRate[3] = {0.5235, 1.4058, 4.1822};
    double rmwRate[3] = {1.6780, 3.1595, 8.5411};
    double randRate[3] = {7.5485, 16.1414, 118.0597};
    // Dependent sequential R-M-W (scalar addq $1, mem — the CSR row_ptr
    // prefix-loop pattern; class_calib.c "dep").  Unlike the streamed
    // memmove rmw, the load->modify->store serializes per line, so the
    // L2-resident rate is ~1.6x rmw (L3/DRAM are line-transfer bound and
    // similar).  Measured on this machine (scalar, no-tree-vectorize).
    double depRate[3] = {1.6220, 2.1931, 8.6007};
    // CSR col_idx memmove tail (class_calib.c "csr_move"): the ACTUAL kernel
    // pattern — libc memmove(dst = src + 4, src, len), int32 elements, 4-byte
    // overlapping forward shift (backward-copied by glibc).  Measured over
    // 12 calibration runs; the median ratio vs the BCSR rmw rate is
    // ~1.00/1.03/0.99 (L2/L3/DRAM) — the same libc memmove moves the same
    // byte volume, so the rates are indistinguishable within noise.  Baked
    // as frozen-rmw x median ratio.
    double csrMoveRate[3] = {1.6756, 3.2460, 8.4735};
  };

  // Log-linear ramp multiplier between lo (penalty 1.0) and hi (penalty P).
  // Below lo: 1.0.  At/above hi: P.  Guarantees hi > lo (caller ensures).
  double rampPenalty(double ws, double lo, double hi, double P)
  {
    if (ws <= lo)
      return 1.0;
    if (ws >= hi)
      return P;
    const double denom = std::log(hi / lo);
    if (denom <= 0.0)
      return P; // degenerate band; treat as saturated
    return 1.0 + (P - 1.0) * std::log(ws / lo) / denom;
  }

  // Resolve the ramp band from calibration, falling back to the original
  // LLC/2 .. 2*LLC guess when the measured keys are absent.
  static void rampBand(const HwCalib &hw, double &lo, double &hi)
  {
    lo = (hw.ramp_lo > 0.0) ? hw.ramp_lo : hw.LLC * 0.5;
    hi = (hw.ramp_hi > 0.0) ? hw.ramp_hi : hw.LLC * 2.0;
    if (hi <= lo)
      hi = lo + hw.L; // guard against a degenerate zero-width band
  }

  // Streaming-cost multiplier for memmove/shift terms, keyed on the size of the
  // backing array being shifted.  Below ramp_lo the working set is fully cache
  // resident (multiplier 1.0); above ramp_hi every shifted line misses to DRAM
  // (multiplier hw.P).  Between ramp_lo and ramp_hi the penalty ramps
  // log-linearly, modelling the gradual cache-pressure transition.  The band
  // edges are MEASURED by hw_calib_bench's working-set sweep (keys ramp_lo /
  // ramp_hi); absent those, it falls back to LLC/2 .. 2*LLC.  Mirrors
  // mem_penalty() in cost_model.py.
  // ns per shifted cache line at working set `ws` — log-log linear between
  // the measured memmove_curve anchors; falls back to Tm·(log-linear ramp)
  // without the curve.  Mirrors _curve_per_line() in cost_model.py.
  double curvePerLine(double ws, const HwCalib &hw)
  {
    if (hw.memmoveCurve.size() < 2)
    {
      double lo, hi;
      rampBand(hw, lo, hi);
      return hw.Tm * rampPenalty(ws, lo, hi, hw.P);
    }
    const auto &C = hw.memmoveCurve;
    if (ws <= C.front().first)
      return C.front().second;
    if (ws >= C.back().first)
      return C.back().second;
    const double lws = std::log(ws);
    for (size_t i = 1; i < C.size(); ++i)
    {
      const double w0 = C[i - 1].first, p0 = C[i - 1].second;
      const double w1 = C[i].first, p1 = C[i].second;
      if (ws <= w1)
      {
        const double denom = std::log(w1 / w0);
        if (denom <= 0.0)
          return p1;
        const double f = (lws - std::log(w0)) / denom;
        return p0 + (p1 - p0) * f;
      }
    }
    return C.back().second;
  }

  double memPenalty(double workingSetBytes, const HwCalib &hw)
  {
    // With the measured curve present, the multiplier is the interpolated
    // per-line cost ÷ cache-resident per-line cost (hw.Tm).  Without it, the
    // log-linear ramp over the [ramp_lo, ramp_hi] band.
    return curvePerLine(workingSetBytes, hw) / hw.Tm;
  }

  // F(W, h2, h3) — cache-aware extension of the footprint penalty (mirror of
  // cost_model.py mem_penalty_cache_aware).  The RD/MRC (Sen/Wood) machinery
  // supplies the residency fractions h2 (L2), h3−h2 (L3), 1−h3 (DRAM); the
  // calibrated curve supplies the per-line cost proxy p(W)=memPenalty(W):
  //     F = h2·p(min(W, L2)) + (h3−h2)·p(min(W, LLC)) + (1−h3)·p(W).
  // Limits: h3=0 -> p(W) (exact legacy); h2=1 -> p(min(W,L2)) (cache-resident
  // floor); W<=L2 -> p(W).  This is a locality correction to the calibrated
  // penalty, NOT a replacement of the structural cost terms.
  double memPenaltyCacheAware(double ws, double h2, double h3,
                              const HwCalib &hw)
  {
    const double l2 = hw.l2Bytes;
    const double llc = hw.LLC;
    return h2 * memPenalty(std::min(ws, l2), hw) +
           (h3 - h2) * memPenalty(std::min(ws, llc), hw) +
           (1.0 - h3) * memPenalty(ws, hw);
  }

  // Size-aware per-insert cost for an EdgeHashMap bulk build.  The table is
  // next_pow2(2*pairs+1) entries of 24 bytes.  When it fits in the LLC the
  // per-insert cost is the cache-resident measurement (hw.h_cache); when it
  // spills to DRAM it is hw.h_dram.  In between, ramp log-linearly on the
  // table size vs the SAME measured [ramp_lo, ramp_hi] band the memmove term
  // uses (cache-residency is governed by the same LLC, whichever structure
  // occupies it).  Falls back to the legacy scalar hw.h (which was measured
  // DRAM-resident) when the two-size calibration keys are absent.
  double hashInsertCost(double pairs, const HwCalib &hw)
  {
    double tableEntries = 1.0;
    while (tableEntries < 2.0 * pairs + 1.0)
      tableEntries *= 2.0;
    const double tableBytes = tableEntries * 24.0;
    const double hDram = (hw.h_dram > 0.0) ? hw.h_dram : hw.h;
    if (hw.h_cache <= 0.0)
      return hDram; // legacy single-point calibration
    double lo, hi;
    rampBand(hw, lo, hi);
    if (tableBytes <= lo)
      return hw.h_cache;
    if (tableBytes >= hi)
      return hDram;
    const double denom = std::log(hi / lo);
    if (denom <= 0.0)
      return hDram;
    const double ratio = std::log(tableBytes / lo) / denom;
    return hw.h_cache + (hDram - hw.h_cache) * ratio;
  }

  // EdgeHashMap table size in bytes (next_pow2(2*pairs+1) entries × 24 B).
  double staticHashTableBytes(double pairs)
  {
    double tableEntries = 1.0;
    while (tableEntries < 2.0 * pairs + 1.0)
      tableEntries *= 2.0;
    return tableEntries * 24.0;
  }

  // Bulk-build cost for the lazy static EdgeHashMap over `pairs` undirected edges.
  // hashInsertCost() already drops to h_cache below ramp_lo, so this is
  // exactly the per-pair cost × pairs.  Mirrors static_hash_build_cost() in
  // cost_model.py.
  double staticHashBuildCost(double pairs, const HwCalib &hw)
  {
    return hashInsertCost(pairs, hw) * pairs;
  }

  HwCalib loadHwCalib()
  {
    HwCalib hw;
    const char *path = std::getenv("AUTOTUNER_HW_CALIB");
    std::string fpath = path ? path : "";
    if (fpath.empty())
    {
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
    std::string text;
    char chunk[8192];
    size_t got;
    while ((got = std::fread(chunk, 1, sizeof(chunk), f)) > 0)
      text.append(chunk, got);
    std::fclose(f);
    // Minimal JSON parse: look for "L", "t", "T", ... keys.
    auto extract = [&](const char *key) -> double
    {
      const char *p = std::strstr(text.c_str(), key);
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
    double vR = extract("\"R\"");
    double vLLC = extract("\"LLC\"");
    double vTm = extract("\"Tm\"");
    double vP = extract("\"P\"");
    double vh = extract("\"h\"");
    double vc = extract("\"c\"");
    double vhCache = extract("\"h_cache\"");
    double vhDram = extract("\"h_dram\"");
    double vRampLo = extract("\"ramp_lo\"");
    double vRampHi = extract("\"ramp_hi\"");
    if (vL > 0)
      hw.L = vL;
    if (vt > 0)
      hw.t = vt;
    if (vT > 0)
      hw.T = vT;
    if (vR > 0)
      hw.R = vR;
    if (vLLC > 0)
      hw.LLC = vLLC;
    if (vTm > 0)
      hw.Tm = vTm;
    if (vP > 0)
      hw.P = vP;
    if (vh > 0)
      hw.h = vh;
    if (vc > 0)
      hw.c = vc;
    if (vhCache > 0)
      hw.h_cache = vhCache;
    if (vhDram > 0)
      hw.h_dram = vhDram;
    if (vRampLo > 0)
      hw.ramp_lo = vRampLo;
    if (vRampHi > 0)
      hw.ramp_hi = vRampHi;
    // If only the legacy scalar h is present, treat it as the DRAM value so
    // the size-aware ramp degrades to the old behaviour (table always charged
    // at the DRAM rate, which is what the legacy h measured).
    if (hw.h_dram <= 0.0 && hw.h > 0.0)
      hw.h_dram = hw.h;
    // Recalibrated SET-build bounds, mirroring cost_model.py: the measured
    // corpora plateau at ~34ns/pair (cache-resident) / ~60ns/pair (DRAM),
    // so hard-cap the calibration values.
    if (hw.h_cache > 0.0)
      hw.h_cache = (hw.h_cache > 34.0) ? std::min(hw.h_cache, 34.0) : std::max(hw.h_cache, 34.0);
    if (hw.h_dram > 0.0)
      hw.h_dram = std::min(hw.h_dram, 60.0);
    // memmove_curve: [[ws_bytes, per_line_ns], ...] log-spaced sweep.
    // Scan every "[ws, perLine]" pair (the first '[' is the outer array).
    const char *curveStart = std::strstr(text.c_str(), "\"memmove_curve\"");
    if (curveStart)
    {
      const char *p = std::strchr(curveStart, '[');
      bool first = true;
      while (p && *p)
      {
        double ws = std::strtod(p + 1, nullptr);
        char *p1 = nullptr;
        double perLine = std::strtod(p + 1, &p1);
        if (p1 && *p1 == ',')
          perLine = std::strtod(p1 + 1, nullptr);
        if (!first && ws > 0.0 && perLine > 0.0)
          hw.memmoveCurve.push_back({ws, perLine});
        first = false;
        if (!p1 || p1 == p)
          break;
        p = std::strchr(p1, '[');
      }
      std::sort(hw.memmoveCurve.begin(), hw.memmoveCurve.end());
    }
    // class_rates: {"seq": [L2,L3,DRAM], "rmw": [...], "rand": [...]}.
    // Mirrors class_rates_model() in cost_model.py.
    auto parseRateArray = [&](const char *key, double (&dst)[3])
    {
      const char *p = std::strstr(text.c_str(), key);
      if (!p)
        return;
      p = std::strchr(p, '[');
      if (!p)
        return;
      for (int i = 0; i < 3; ++i)
      {
        double val = std::strtod(p + 1, nullptr);
        if (val > 0.0)
          dst[i] = val;
        p = std::strchr(p + 1, ',');
        if (!p)
          break;
      }
    };
    parseRateArray("\"seq\"", hw.seqRate);
    parseRateArray("\"rmw\"", hw.rmwRate);
    parseRateArray("\"rand\"", hw.randRate);
    parseRateArray("\"dep\"", hw.depRate);
    parseRateArray("\"csr_move\"", hw.csrMoveRate);
    return hw;
  }

  int opIndex(RegionType t)
  {
    switch (t)
    {
    case RegionType::Traverse:
      return 0;
    case RegionType::Insert:
      return 1;
    case RegionType::SetQuery:
      return 2;
    case RegionType::CSRQuery:
      return 3;
    }
    return 2;
  }

  const char *regionTypeName(RegionType t)
  {
    switch (t)
    {
    case RegionType::Traverse:
      return "Traverse";
    case RegionType::Insert:
      return "Insert";
    case RegionType::SetQuery:
      return "SetQuery";
    case RegionType::CSRQuery:
      return "CSRQuery";
    }
    return "SetQuery";
  }

  const char *layoutName(int layout)
  {
    switch (layout)
    {
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

  int forcedLayoutFromEnv()
  {
    const char *raw = std::getenv("AUTOTUNER_FORCE_LAYOUT");
    if (!raw || !*raw)
      return -1;
    std::string value(raw);
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c)
                   { return static_cast<char>(std::toupper(c)); });
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

  bool layoutFeasible(RegionType type, int layout)
  {
    if (layout < 0 || layout >= LAYOUT_COUNT)
      return false;
    switch (type)
    {
    case RegionType::Traverse:
      return true; // CSR, PCSR, BCSR, SET all feasible
    case RegionType::Insert:
      return true; // CSR, PCSR, BCSR, SET all feasible
    case RegionType::SetQuery:
      return layout == LAYOUT_SET; // graph bitmap / set queries require SET
    case RegionType::CSRQuery:
      return layout == LAYOUT_CSR; // row_ptr/col_idx readers need CSR
    }
    return false;
  }

  bool hasPrefix(StringRef s, StringRef pref) { return s.starts_with(pref); }

  bool isTraverseCall(StringRef fn)
  {
    return fn == "bfs_runtime" || fn == "bfs_runtime_src" || fn == "dfs_runtime" ||
           fn == "dfs_runtime_src" || fn == "autograph_frontier_step";
  }

  bool isInsertCall(StringRef fn)
  {
    return fn == "graph_add_node" || fn == "graph_add_edge" ||
           fn == "graph_remove_node" || fn == "graph_remove_edge" ||
           hasPrefix(fn, "roaring_bitmap_add") || hasPrefix(fn, "roaring_bitmap_remove");
  }

  // Queries that operate directly on Roaring bitmaps — they cannot execute
  // unless the graph is in LAYOUT_SET, so they force SET via layoutFeasible.
  bool isSetSpecificQuery(StringRef fn)
  {
    return fn == "roaring_bitmap_contains" ||
           fn == "roaring_bitmap_get_cardinality" ||
           fn == "roaring_bitmap_union" ||
           fn == "roaring_bitmap_intersect";
  }

  // Queries that read CSR arrays (row_ptr / col_idx / weights) directly from
  // struct.Graph — they force LAYOUT_CSR.
  bool isCSRSpecificQuery(StringRef fn)
  {
    return fn == "graph_get_edge_weight" ||
           fn == "graph_get_edge_weight_by_id" ||
           fn == "graph_get_edge_src_by_id" ||
           fn == "graph_get_edge_dst_by_id";
  }

  // Traversal cost: |Active Set| x (C_nav + ceil(B_useful / L) x T)
  // |Active Set| = n (every vertex becomes active once across a traversal region).
  // d = 2m/n (undirected average degree).
  // g = kPcsrExpansionFactor (measured: 2), b = kBcsrBlockSize (measured: 64).
  double traversalCost(int layout, double n, double m, const HwCalib &hw)
  {
    const double d = (n > 0) ? (2.0 * m / n) : 1.0;
    const double g = kPcsrExpansionFactor;
    const double b = kBcsrBlockSize;
    const double t = hw.t;
    const double T = hw.T;
    const double L = hw.L;

    double cNav = 0.0;
    double bUseful = 0.0;

    switch (layout)
    {
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
    {
      // Per vertex: scan ALL m edge_pairs (16 bytes each) + per-pair compute.
      // Inner loop: roaring_bitmap_contains + int32 compare + branch.
      // The contains check is NOT free — it's a full function call with binary
      // search (~log(#containers) comps).  Approximate as t/6 ≈ 5.5ns/pair.
      bUseful = 8.0 * m;
      return n * (std::ceil(bUseful / L) * T);
    }
    default:
      return kInf;
    }

    const double cTraffic = std::ceil(bUseful / L) * T;
    return n * (cNav + cTraffic);
  }

  // Insert/update cost: C_locate + C_write + C_move
  // Per-undirected-edge insert cost.  m = undirected edge count.
  // One undirected add = 2 directed inserts (graph_add_edge does both).
  //
  // Memory traffic categories:
  //   T  = sequential read-only cost per cache line (also used for memmove,
  //        whose independent read+write streams are overlapped by hardware).
  //   2T = read-modify-write cost per cache line (prefix-sum increments:
  //        dependent load→store, cannot overlap).
  // Size-aware realloc.  realloc of a GROWING backing array is amortized
  // in-place for small arrays (growth bookkeeping ~Tm/line, no copy: the
  // runtime grows by a few entries per insert and almost never migrates at
  // these sizes), so the charge is the memmove-curve rate.  For arrays big
  // enough that the kernel's page-remap path beats memcpy, the flat
  // page-remap policy cost R applies instead:
  //     realloc.cost = min(R, lines·Tm·pen).  Mirrors realloc_cost() in
  //     cost_model.py; use it with objs = number of realloc events.
  double reallocCost(double arrayBytes, double objs, const HwCalib &hw)
  {
    if (arrayBytes <= 0.0)
      return 0.0;
    const double copy = std::ceil(arrayBytes / hw.L) * hw.Tm *
                        memPenalty(arrayBytes, hw);
    return std::min(copy, hw.R) * objs;
  }

  double insertCost(int layout, double n, double m, double csrFrac,
                    double bcsrFrac, const HwCalib &hw,
                    const double *classTiers = nullptr,
                    const double *csrClassTiers = nullptr)
  {
    const double d = (n > 0) ? (2.0 * m / n) : 1.0;
    const double g = kPcsrExpansionFactor;
    const double t = hw.t;
    const double T = hw.T;
    const double L = hw.L;
    const double R = hw.R;
    const double gU = g * d;  // g(u) = physical span of vertex u

    switch (layout)
    {
    case LAYOUT_CSR:
    {
      // PRODUCTION (Hybrid): legacy structural decomposition
      // (locate + write + move + realloc) with the memory-penalty terms
      // replaced by the class-tier weighted per-line rates:
      //   C_write = ⌈8n/L⌉ · wrate_brow      (was 2T·memPenalty)
      //   C_move  = ⌈4m·csrFrac/L⌉ · wrate_move  (was Tm·memPenalty)
      // wrate uses the autotuner.class_tiers_csr payload (brow/move classes)
      // at the rmw per-tier rates.  Absent the payload, the footprint-only
      // legacy equation applies.  Mirrors cost_model.py insert_cost_csr
      // under AUTOTUNER_CACHE_MODEL=hybrid.
      if (csrClassTiers && g_cacheModel == CacheModel::Hybrid)
      {
        const double cLocate = 2 * t + std::ceil(d * 4 / L) * T;
        const double cWrite =
            std::ceil(8.0 * n / L) *
            weightedRate(csrClassTiers[7], csrClassTiers[8], hw.rmwRate);
        const double moveBytes = 4.0 * m * std::max(0.0, csrFrac);
        const double cMove =
            std::ceil(moveBytes / L) *
            weightedRate(csrClassTiers[4], csrClassTiers[5], hw.rmwRate);
        const double cRealloc = R + std::ceil(4.0 * m / L) * T;
        return cLocate + cWrite + cMove + cRealloc;
      }
      // Class-tier diagnostics: the experimental class × residency rate
      // model replacing the whole memory terms.
      if (csrClassTiers && g_cacheModel == CacheModel::ClassTier)
      {
        const double *rates[4] = {hw.seqRate, hw.csrMoveRate, hw.depRate,
                                  hw.randRate};
        double perDir = 0.0;
        for (int cls = 0; cls < 4; ++cls)
        {
          const double N = csrClassTiers[cls * 3 + 0];
          const double h2 = csrClassTiers[cls * 3 + 1];
          const double h3 = csrClassTiers[cls * 3 + 2];
          perDir += N * (h2 * rates[cls][0] + (h3 - h2) * rates[cls][1] +
                         (1.0 - h3) * rates[cls][2]);
        }
        return 2.0 * t + 2.0 * perDir;
      }
      // One undirected edge = 2 directed inserts (from→to, to→from).
      // Per directed: realloc + memmove + row_ptr prefix-sum update.
      // C_locate  = 2t + ⌈d·4/L⌉·T       (row_ptr[from+1] random read + scan)
      // C_prefix  = t + ⌈(n−u)·8/L⌉·2T·pen (n/2 int64 R-M-W entries;
      //                                 dependent load→store, DRAM-bound past LLC)
      // C_move    = ⌈4m/L⌉·Tm·pen        (2 dirs × ~m/2 cols each ≈ 4m bytes
      //                                 read+written via Tm — same physics as
      //                                 cost_model.py insert_cost_csr)
      // C_realloc = R + ⌈4m/L⌉·T        (realloc cap + col_idx growth write)
      const double cLocate = 2 * t + std::ceil(d * 4 / L) * T;
      const double prefixBytes = n * 8.0;
      const bool awareCsr = (csrClassTiers && g_cacheModel == CacheModel::Aware);
      const double browH2 = awareCsr ? csrClassTiers[7] : 0.0;
      const double browH3 = awareCsr ? csrClassTiers[8] : 0.0;
      const double cWrite =
          1.0 * t + std::ceil(prefixBytes / L) * 2.0 * T *
                        (awareCsr ? memPenaltyCacheAware(prefixBytes, browH2, browH3, hw)
                                  : memPenalty(prefixBytes, hw));
      const double moveBytes = 4.0 * m * std::max(0.0, csrFrac);
      const double moveH2 = awareCsr ? csrClassTiers[4] : 0.0;
      const double moveH3 = awareCsr ? csrClassTiers[5] : 0.0;
      const double cMove =
          std::ceil(moveBytes / L) * hw.Tm *
          (awareCsr ? memPenaltyCacheAware(8.0 * m + moveBytes, moveH2, moveH3, hw)
                    : memPenalty(8.0 * m + moveBytes, hw));
      const double cRealloc = R + std::ceil(4.0 * m / L) * T;
      return cLocate + cWrite + cMove + cRealloc;
    }
    case LAYOUT_PCSR:
    {
      // In-place gap fill — no memmove, no realloc.
      // C_locate = 2·(2t + ⌈g·d·4/L⌉·T)  (2 dirs × random reads + read-only gap scan)
      // C_write  = 2·t                    (2 dirs × 1 random gap-slot write)
      const double cLocate = (2.0 * t + std::ceil(gU * 4.0 / L) * T);
      const double cWrite = T;
      return cLocate + cWrite;
    }
    case LAYOUT_BCSR:
    {
      // PRODUCTION (Hybrid): legacy structural decomposition with the
      // memory-penalty terms replaced by the class-tier weighted per-line
      // rates:
      //   C_move  = ⌈16m·bcsrFrac/L⌉ · wrate_move    (was Tm·memPenalty)
      //   C_realloc = min(⌈16m/L⌉·wrate_struct, R)·2  (was reallocCost)
      // C_write (brow prefix) has no memPenalty today and stays unchanged.
      // wrate uses the autotuner.class_tiers payload (move at rmw, struct
      // at rand).  Absent the payload, the footprint-only legacy equation
      // applies.  Mirrors cost_model.py insert_cost_bcsr under
      // AUTOTUNER_CACHE_MODEL=hybrid.
      if (classTiers && g_cacheModel == CacheModel::Hybrid)
      {
        const double b = kBcsrBlockSize;
        const double nb = std::ceil(n / b);
        constexpr double kDirs = 2.0;
        const double cLocate =
            4.0 * t + kDirs * std::ceil(8.0 * b * d / L) * T;
        const double cWrite = std::ceil(nb * 4.0 / L) * 2.0 * T;
        const double moveBytes = kDirs * 8.0 * std::max(0.0, bcsrFrac) * m;
        const double cMove =
            std::ceil(moveBytes / L) *
            weightedRate(classTiers[4], classTiers[5], hw.rmwRate);
        const double cRealloc =
            std::min(std::ceil(16.0 * m / L) *
                         weightedRate(classTiers[10], classTiers[11],
                                      hw.randRate),
                     R) *
            kDirs;
        return cLocate + cWrite + cMove + cRealloc;
      }
      // Class-tier diagnostics: the experimental class × residency rate
      // model replacing the whole memory terms.
      if (classTiers && g_cacheModel == CacheModel::ClassTier)
      {
        const double *rates[4] = {hw.seqRate, hw.rmwRate, hw.rmwRate,
                                  hw.randRate};
        double perDir = 0.0;
        for (int cls = 0; cls < 4; ++cls)
        {
          const double N = classTiers[cls * 3 + 0];
          const double h2 = classTiers[cls * 3 + 1];
          const double h3 = classTiers[cls * 3 + 2];
          perDir += N * (h2 * rates[cls][0] + (h3 - h2) * rates[cls][1] +
                         (1.0 - h3) * rates[cls][2]);
        }
        return 4.0 * t + 2.0 * perDir;
      }
      // graph_add_edge calls autograph_bcsr_add_edge twice (both directions).
      // Runtime (autograph_bcsr_add_edge): dup-scan the block row, realloc
      // bcol by +2 ints, memmove everything after the insertion point, bump
      // the brow prefix sums.  bcol backing array = 16m bytes.
      //
      // Both directed inserts always pay a memmove of the bcol tail after
      // the sorted insertion point; the expected tail volume is computed
      // exactly from the graph's degree vector (bcsrFrac) — deliberately NO
      // degree-based piecewise approximation (append shortcut / second-shift
      // probability / hard cutoff); the exact expected tail replaces all of
      // it.  Mirrors insert_cost_bcsr() in cost_model.py.
      const double b = kBcsrBlockSize;
      const double nb = std::ceil(n / b);
      constexpr double kDirs = 2.0;
      const double cLocate =
          4.0 * t + kDirs * std::ceil(8.0 * b * d / L) * T;
      // brow prefix-sum R-M-W: expected E[nb−blk] = nb/2 int32 entries per
      // directed insert × 2 dirs = nb int32 entries, 2T per line.
      const double cWrite = std::ceil(nb * 4.0 / L) * 2.0 * T;
      // Exact shift: bcsrFrac = expected tail / m (from the real degree
      // vector; 1.0 = whole array).  Two directed inserts × 8 bytes per
      // shifted edge-pair.  memPenalty keeps the working-set cache-pressure
      // physics of the whole backing array.
      const double moveBytes = kDirs * 8.0 * std::max(0.0, bcsrFrac) * m;
      const bool awareBcsr = (classTiers && g_cacheModel == CacheModel::Aware);
      const double moveH2 = awareBcsr ? classTiers[4] : 0.0;
      const double moveH3 = awareBcsr ? classTiers[5] : 0.0;
      const double cMove =
          std::ceil(moveBytes / L) * hw.Tm *
          (awareBcsr ? memPenaltyCacheAware(moveBytes, moveH2, moveH3, hw)
                     : memPenalty(moveBytes, hw));
      const double cRealloc = reallocCost(16.0 * m, kDirs, hw);
      return cLocate + cWrite + cMove + cRealloc;
    }
    case LAYOUT_SET:
    {
      // Steady-state cost of autograph_canonical_add_edge (one undirected add).
      // All O(1): live_edge_count is maintained incrementally, so the cached
      // counts refresh (canonical_edge_count_cached) does NOT rescan per insert,
      // and re-adding already-present nodes does not bump the bitmap mutation
      // epoch (no select-cache rebuild).
      //   C_locate = 5t       (2 roaring_bitmap_add on nodes_bitmap + static
      //                        edge-hash probe + extra edge-hash probe)
      //   C_write  = 2t + 5T  (extra_edge_pairs append + live flag + extra-hash
      //                        insert + O(1) cached counts refresh)
      // The expensive O(n+m) work (lazy static-hash build, first edge-count
      // scan, select-cache rebuild) happens ONCE after entering SET and is
      // charged per region in insertSetupCost(), not per operation.
      const double cLocate = 5.0 * t;
      const double cWrite = 2.0 * t + 5.0 * T;
      return cLocate + cWrite;
    }
    default:
      return kInf;
    }
  }

  // One-time lazy-initialization cost paid by the FIRST insert executed while
  // the graph is in `layout`.  For LAYOUT_SET (autograph_canonical_add_edge):
  //   get_static_edge_hash() builds a hash over all m static pairs.  The
  //   per-insert cost is hw.h (size-aware: hw.h_cache / hw.h_dram via
  //   hashInsertCost()), measured by hw_calib_bench against a replica of the
  //   open-addressing EdgeHashMap.
  // Plus one extra_edge_pairs realloc event: size-aware (reallocCost of the
  // static hash table's backing bytes — mirrors realloc_cost(staticHashTableBytes)
  // in cost_model.py insert_setup_cost_set).
  //
  // IMPORTANT: the other O(n+m) work the first refresh_graph_counts_from_canonical
  // performs (the canonical_edge_count roaring_bitmap_contains scan = hw.c*m,
  // and the canonical_node_span select-cache rebuild) is NOT paid by the first
  // timed insert.  It runs inside autograph_ensure_layout / _ensure_layout_set
  // BEFORE profile_region_enter, i.e. as part of the CSR->SET conversion, so
  // it is billed in conversionCost(CSR, SET) below, not here.  The conversion
  // also builds and destroys its own local edge hash (rebuild_sets_from_csr_meta),
  // so the only O(m) work the first timed insert actually pays is this lazy
  // static-hash build.
  //
  // Charged once per region (NOT scaled by H·totalOps): the built hash survives
  // subsequent inserts while the layout is SET.
  double insertSetupCost(int layout, double n, double m, const HwCalib &hw)
  {
    if (layout != LAYOUT_SET)
      return 0.0;
    const double cHashBuild = staticHashBuildCost(m, hw);
    return cHashBuild + reallocCost(staticHashTableBytes(m), 1.0, hw);
  }

  // C_read(A) — read the source layout's full representation.  The runtime
  // transition scans the source arrays end-to-end before writing the
  // destination, so the read is the full backing-store size (DRAM-bound past
  // the LLC, hence ceil-bytes/L cache lines at cost T per line).
  //   CSR  : row_ptr[0..n] + col_idx[0..m]    = 8(n+1) + 4m      bytes
  //   PCSR : row_ptr[0..n] + col_idx[0..g·m] = 8(n+1) + 4 g m   bytes
  //          (g = kPcsrExpansionFactor: gap-padded col_idx has g·m cells)
  //   BCSR : brow[0..n_b] + bcol[0..2m]×i32   = 4 n_b + 16m     bytes
  //          (n_b = ⌈n / kBcsrBlockSize⌉; bcol carries (local_row,col) int32
  //           pairs, 8 bytes per directed edge, 2m directed edges)
  //   SET  : edge_pairs[0..m]×(u,v)×i32       = 8m               bytes
  //          (bitmaps are canonical — no source scan; only the static edge
  //           pair table is read to seed the transition)
  double readCost(int from, double n, double m, const HwCalib &hw)
  {
    const double L = hw.L, T = hw.T;
    const double g = kPcsrExpansionFactor;
    const double nb = std::ceil(n / kBcsrBlockSize);
    switch (from)
    {
    case LAYOUT_CSR:  return std::ceil((8.0 * (n + 1.0) + 4.0 * m) / L) * T;
    case LAYOUT_PCSR: return std::ceil((8.0 * (n + 1.0) + 4.0 * g * m) / L) * T;
    case LAYOUT_BCSR: return std::ceil((4.0 * nb + 16.0 * m) / L) * T;
    case LAYOUT_SET:  return std::ceil(8.0 * m / L) * T;
    default:          return kInf;
    }
  }

  // C_meta-build(B) — write the destination's index/metadata structure.
  //   CSR  : row_ptr = 8(n+1) bytes (int64 prefix sums)
  //   PCSR : row_ptr = 8(n+1) bytes (int64 prefix sums, same shape as CSR)
  //   BCSR : brow    = 4 n_b bytes  (int32 prefix sums, one per block)
  //   SET  : three sub-phases mirrored from autograph_ensure_layout_set /
  //          refresh_graph_counts_from_canonical in autotuner_runtime.c:
  //            (a) rebuild_sets_from_csr_meta builds+destroys a local EdgeHashMap
  //                over the m static pairs — hashInsertCost(m) per pair.
  //            (b) canonical_edge_count_cached runs the full O(m)
  //                roaring_bitmap_contains scan (hw.c per check) because
  //                live_edge_count was just invalidated.
  //            (c) canonical_node_span triggers one O(n) select-cache rebuild
  //                of the nodes bitmap: 4n-byte write + n/8-byte read.
  double metaBuildCost(int to, double n, double m, const HwCalib &hw)
  {
    const double L = hw.L, T = hw.T;
    const double nb = std::ceil(n / kBcsrBlockSize);
    switch (to)
    {
    case LAYOUT_CSR:  return std::ceil(8.0 * (n + 1.0) / L) * T;
    case LAYOUT_PCSR: return std::ceil(8.0 * (n + 1.0) / L) * T;
    case LAYOUT_BCSR: return std::ceil(4.0 * nb / L) * T;
    case LAYOUT_SET:
      return hashInsertCost(m, hw) * m
           + hw.c * m
           + (std::ceil(4.0 * n / L) + std::ceil(n / 8.0 / L)) * T;
    default:          return kInf;
    }
  }

  // C_payload-build(B) — write the destination's adjacency payload.
  //   CSR  : col_idx = 4m bytes     (int32 per directed edge)
  //   PCSR : col_idx = 4 g m bytes  (int32 per directed edge, g = kPcsrExpansionFactor)
  //   BCSR : bcol    = 16m bytes    ((local_row,col) int32 pairs, 8 bytes
  //                                  per directed edge, 2m directed edges)
  //   SET  : edges_bitmap + extra_edge_pairs append = 8m bytes per undirected
  //          edge (the bitmap representation pays 1 bit per static pair plus
  //          an O(1) extra-pair slot; the model charges the same 8m bound
  //          the SET traversal/read equations use).
  double payloadBuildCost(int to, double n, double m, const HwCalib &hw)
  {
    const double L = hw.L, T = hw.T;
    const double g = kPcsrExpansionFactor;
    (void)n;
    switch (to)
    {
    case LAYOUT_CSR:  return std::ceil(4.0 * m / L) * T;
    case LAYOUT_PCSR: return std::ceil(4.0 * g * m / L) * T;
    case LAYOUT_BCSR: return std::ceil(16.0 * m / L) * T;
    case LAYOUT_SET:  return std::ceil(8.0 * m / L) * T;
    default:          return kInf;
    }
  }

  // General physics-based conversion cost:
  //   C_{A→B} = C_read(A) + C_meta-build(B) + C_payload-build(B)
  // Replaces the earlier alpha-scaled approximation (alphaFromCSR /
  // alphaToCSR multiplied by (n+m)) for all 11 transitions that were not
  // covered by conversionCostCSRToSET, and folds the CSR→SET leg into the
  // same decomposition.  The CSR→SET path keeps every physics term the old
  // conversionCostCSRToSET carried (m·hashInsertCost(m), c·m, the
  // ⌈4n/L⌉ + ⌈n/(8L)⌉ select-cache rebuild) under metaBuildCost(SET), and
  // replaces the incomplete cCsrScan = ⌈4m/L⌉·T read term with the full
  // C_read(CSR) = ⌈(8(n+1) + 4m)/L⌉·T — i.e. it now bills the row_ptr
  // scan it was missing — and additionally bills the SET payload write
  // (⌈8m/L⌉·T) the general formula requires via payloadBuildCost(SET).
  double conversionCost(int from, int to, double n, double m,
                        const HwCalib &hw)
  {
    if (from == to)
      return 0.0;
    return readCost(from, n, m, hw)
         + metaBuildCost(to, n, m, hw)
         + payloadBuildCost(to, n, m, hw);
  }

  double operationCost(const Region &r, int layout, double n, double m,
                       double csrFrac, double bcsrFrac, const HwCalib &hw,
                       const double *classTiers = nullptr,
                       const double *csrClassTiers = nullptr)
  {
    // Execution cost of a region under the chosen layout L.
    //   operationCost(R, L) = H · totalOps · ( f_T · uTrav(L) + f_I · uIns(L) )
    // H is the exact enclosing-loop trip count (autotuner.trip_count), so
    // 50 explicit adds (totalOps=50, H=1) match 1 looped add (totalOps=1, H=50).
    // Layout-forcing query regions (SetQuery → SET, CSRQuery → CSR) have
    // f_T = f_I = 0, so their operationCost is 0: the only variable cost is
    // the conversion to the forced layout, paid by the DP at the region
    // boundary via conversionCost(prev, forced_layout).  Query execution
    // time itself is layout-constant and drops out of the DP argmin.
    const double fT = r.freq[0];
    const double fI = r.freq[1];
    const double H = std::max(1.0, r.execCount);
    const double totalOps = std::max(1.0, static_cast<double>(r.totalOps));

    const double uTrav = traversalCost(layout, n, m, hw);
    const double uIns = insertCost(layout, n, m, csrFrac, bcsrFrac, hw,
                                   classTiers, csrClassTiers);

    // One-time lazy-init cost for the first insert under this layout (e.g.
    // SET's static-hash build).  Charged once per region, not per op: the
    // built structures persist across the region's inserts.
    const double setup = (fI > 0.0) ? insertSetupCost(layout, n, m, hw) : 0.0;

    return setup + H * totalOps * (fT * uTrav + fI * uIns);
  }

  double estimateAllCSRPathCost(const std::vector<Region> &regions, double estN,
                                double estM, double csrFrac, double bcsrFrac,
                                const HwCalib &hw,
                                const double *classTiers = nullptr,
                                const double *csrClassTiers = nullptr)
  {
    if (regions.empty())
      return 0.0;
    int current = LAYOUT_CSR;
    double total = 0.0;
    for (const Region &R : regions)
    {
      if (!layoutFeasible(R.dominant, LAYOUT_CSR))
        return kInf;
total += conversionCost(current, LAYOUT_CSR, estN, estM, hw);
      total += operationCost(R, LAYOUT_CSR, estN, estM, csrFrac, bcsrFrac, hw,
                             classTiers, csrClassTiers);
      current = LAYOUT_CSR;
    }
    return total;
  }

  Function *resolveCallee(CallBase *CB)
  {
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
                                    double estM, double csrFrac, double bcsrFrac,
                                    const HwCalib &hw,
                                    const double *classTiers = nullptr,
                                    const double *csrClassTiers = nullptr)
  {
    if (regions.empty() || chosen.size() != regions.size())
      return kInf;
    int current = LAYOUT_CSR;
    double total = 0.0;
    for (size_t i = 0; i < regions.size(); ++i)
    {
      const int layout = chosen[i];
      if (layout < 0 || layout >= LAYOUT_COUNT)
        return kInf;
      if (!layoutFeasible(regions[i].dominant, layout))
        return kInf;
      total += conversionCost(current, layout, estN, estM, hw);
      total += operationCost(regions[i], layout, estN, estM, csrFrac, bcsrFrac, hw,
                             classTiers, csrClassTiers);
      current = layout;
    }
    return total;
  }

  std::tuple<double, double> estimateGraphSize(const GraphMeta &meta)
  {
    double estN = 50000.0;
    double estM = 500000.0;
    if (auto *cn = dyn_cast_or_null<ConstantInt>(meta.n))
      estN = std::max(1.0, static_cast<double>(cn->getSExtValue()));
    if (auto *cm = dyn_cast_or_null<ConstantInt>(meta.m))
      estM = std::max(1.0, static_cast<double>(cm->getSExtValue()));
    return {estN, estM};
  }

  LayoutSchedule solveDP(const std::vector<Region> &regions, double estN,
                         double estM, double csrFrac, double bcsrFrac,
                         const HwCalib &hw, const double *classTiers = nullptr,
                         const double *csrClassTiers = nullptr)
  {
    LayoutSchedule S;
    const int R = static_cast<int>(regions.size());
    if (R == 0)
      return S;

    std::vector<std::array<double, LAYOUT_COUNT>> dp(R);
    std::vector<std::array<int, LAYOUT_COUNT>> parent(R);
    for (int i = 0; i < R; ++i)
    {
      for (int l = 0; l < LAYOUT_COUNT; ++l)
      {
        dp[i][l] = kInf;
        parent[i][l] = -1;
      }
    }

    // DP base: graph starts in CSR (loaded by graph_loader_runtime).
    // The autotuner may convert away from CSR for the first region.
    for (int l = 0; l < LAYOUT_COUNT; ++l)
    {
      if (!layoutFeasible(regions[0].dominant, l))
        continue;
      dp[0][l] = conversionCost(LAYOUT_CSR, l, estN, estM, hw) +
                 operationCost(regions[0], l, estN, estM, csrFrac, bcsrFrac, hw,
                               classTiers, csrClassTiers);
    }

    for (int i = 1; i < R; ++i)
    {
      for (int cur = 0; cur < LAYOUT_COUNT; ++cur)
      {
        if (!layoutFeasible(regions[i].dominant, cur))
          continue;
        const double runCost = operationCost(regions[i], cur, estN, estM, csrFrac, bcsrFrac, hw,
                                             classTiers, csrClassTiers);
        for (int prev = 0; prev < LAYOUT_COUNT; ++prev)
        {
          if (dp[i - 1][prev] >= kInf / 2.0)
            continue;
          const double cand =
              dp[i - 1][prev] + conversionCost(prev, cur, estN, estM, hw) + runCost;
          if (cand < dp[i][cur])
          {
            dp[i][cur] = cand;
            parent[i][cur] = prev;
          }
        }
      }
    }

    int bestLast = LAYOUT_SET;
    double bestVal = kInf;
    for (int l = 0; l < LAYOUT_COUNT; ++l)
    {
      if (dp[R - 1][l] < bestVal)
      {
        bestVal = dp[R - 1][l];
        bestLast = l;
      }
    }

    S.chosen.assign(R, LAYOUT_SET);
    int cur = bestLast;
    for (int i = R - 1; i >= 0; --i)
    {
      S.chosen[i] = cur;
      cur = parent[i][cur];
      if (i > 0 && cur < 0)
        cur = LAYOUT_SET;
    }

    // Reverse horizon DP for guarded conversion decisions.
    S.suffixCost.assign(R, std::vector<double>(LAYOUT_COUNT, kInf));
    for (int l = 0; l < LAYOUT_COUNT; ++l)
    {
      if (!layoutFeasible(regions[R - 1].dominant, l))
        continue;
      S.suffixCost[R - 1][l] = operationCost(regions[R - 1], l, estN, estM, csrFrac, bcsrFrac, hw,
                                             classTiers, csrClassTiers);
    }
    for (int i = R - 2; i >= 0; --i)
    {
      for (int l = 0; l < LAYOUT_COUNT; ++l)
      {
        if (!layoutFeasible(regions[i].dominant, l))
          continue;
        const double here = operationCost(regions[i], l, estN, estM, csrFrac, bcsrFrac, hw,
                                          classTiers, csrClassTiers);
        double tail = kInf;
        for (int nxt = 0; nxt < LAYOUT_COUNT; ++nxt)
        {
          if (S.suffixCost[i + 1][nxt] >= kInf / 2.0)
            continue;
          tail = std::min(tail, conversionCost(l, nxt, estN, estM, hw) + S.suffixCost[i + 1][nxt]);
        }
        S.suffixCost[i][l] = (tail >= kInf / 2.0) ? here : here + tail;
      }
    }

    return S;
  }

  bool collectGraphMeta(Function &mainFn, std::map<Value *, GraphMeta> &metaByGraphPtr)
  {
    bool found = false;
    std::vector<Value *> initGraphPtrs;

    auto registerGraphValue = [&](Value *graphPtr, const GraphMeta &G)
    {
      metaByGraphPtr[graphPtr] = G;
      if (Value *stripped = graphPtr->stripPointerCasts())
      {
        metaByGraphPtr[stripped] = G;
        if (auto *LI = dyn_cast<LoadInst>(stripped))
          metaByGraphPtr[LI->getPointerOperand()] = G;
      }
    };

    for (BasicBlock &BB : mainFn)
    {
      for (Instruction &I : BB)
      {
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
        if (MDNode *sizeMD = CB->getMetadata("autotuner.graph_size"))
        {
          if (sizeMD->getNumOperands() >= 2)
          {
            auto getSizeConstant = [](Metadata *M) -> ConstantInt *
            {
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
        if (MDNode *fracMD = CB->getMetadata("autotuner.shift_fracs"))
        {
          auto readFrac = [](Metadata *M) -> std::optional<double>
          {
            if (auto *CAM = dyn_cast_or_null<ConstantAsMetadata>(M))
              if (auto *CFP = dyn_cast_or_null<ConstantFP>(CAM->getValue()))
                return CFP->getValueAPF().convertToDouble();
            return std::nullopt;
          };
          if (fracMD->getNumOperands() >= 2)
          {
            if (auto cf = readFrac(fracMD->getOperand(0).get()))
              G.csrFrac = std::max(0.0, *cf);
            if (auto bf = readFrac(fracMD->getOperand(1).get()))
              G.bcsrFrac = std::max(0.0, *bf);
          }
        }
        if (MDNode *tierMD = CB->getMetadata("autotuner.class_tiers"))
        {
          auto readTier = [](Metadata *M) -> std::optional<double>
          {
            if (auto *CAM = dyn_cast_or_null<ConstantAsMetadata>(M))
              if (auto *CFP = dyn_cast_or_null<ConstantFP>(CAM->getValue()))
                return CFP->getValueAPF().convertToDouble();
            return std::nullopt;
          };
          const unsigned nOps = tierMD->getNumOperands();
          const unsigned want = 12;
          for (unsigned i = 0; i < nOps && i < want; ++i)
          {
            if (auto d = readTier(tierMD->getOperand(i).get()))
              G.classTiers[i] = *d;
          }
          G.hasClassTiers = (nOps >= want);
        }
        if (MDNode *csrTierMD = CB->getMetadata("autotuner.class_tiers_csr"))
        {
          auto readTier = [](Metadata *M) -> std::optional<double>
          {
            if (auto *CAM = dyn_cast_or_null<ConstantAsMetadata>(M))
              if (auto *CFP = dyn_cast_or_null<ConstantFP>(CAM->getValue()))
                return CFP->getValueAPF().convertToDouble();
            return std::nullopt;
          };
          const unsigned nOps = csrTierMD->getNumOperands();
          const unsigned want = 12;
          for (unsigned i = 0; i < nOps && i < want; ++i)
          {
            if (auto d = readTier(csrTierMD->getOperand(i).get()))
              G.csrClassTiers[i] = *d;
          }
          G.hasCsrClassTiers = (nOps >= want);
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
    for (BasicBlock &BB : mainFn)
    {
      for (Instruction &I : BB)
      {
        auto *SI = dyn_cast<StoreInst>(&I);
        if (!SI)
          continue;
        Value *stored = SI->getValueOperand()->stripPointerCasts();
        for (Value *initPtr : initGraphPtrs)
        {
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

  bool classifyCall(StringRef name, RegionType &outType, bool &usesGraphArg0)
  {
    if (isTraverseCall(name))
    {
      outType = RegionType::Traverse;
      usesGraphArg0 = true;
      return true;
    }
    if (isInsertCall(name))
    {
      outType = RegionType::Insert;
      usesGraphArg0 = name == "graph_add_node" || name == "graph_add_edge" ||
                      name == "graph_remove_node" || name == "graph_remove_edge";
      return true;
    }
    if (isSetSpecificQuery(name))
    {
      outType = RegionType::SetQuery;
      usesGraphArg0 = false;
      return true;
    }
    if (isCSRSpecificQuery(name))
    {
      outType = RegionType::CSRQuery;
      usesGraphArg0 = true;
      return true;
    }
    return false;
  }

  // Effective op count for merge / switch gates: static sites × exact trip count H.
  // 50 explicit adds (H=1) ≡ 1 looped add with trip count 50.
  double regionEffectiveOps(const Region &r)
  {
    const double sites = std::max(1.0, static_cast<double>(r.totalOps));
    const double H = std::max(1.0, r.execCount);
    return sites * H;
  }

  double estimateExecMultiplier(BasicBlock *BB)
  {
    // H = product of exact enclosing-loop trip counts (from IRGen metadata
    // autotuner.trip_count, or recovered from icmp vs constant). No heuristic ×8.
    double mult = 1.0;
    SmallPtrSet<BasicBlock *, 16> visited;
    BasicBlock *cur = BB;
    while (cur && visited.insert(cur).second)
    {
      if (Instruction *Term = cur->getTerminator())
      {
        if (MDNode *MD = Term->getMetadata("autotuner.trip_count"))
        {
          if (MD->getNumOperands() >= 1)
          {
            if (auto *CAM = dyn_cast<ConstantAsMetadata>(MD->getOperand(0)))
            {
              if (auto *CI = dyn_cast<ConstantInt>(CAM->getValue()))
              {
                const uint64_t trips = CI->getZExtValue();
                if (trips > 0)
                  mult *= static_cast<double>(trips);
              }
            }
          }
        }
        else if (auto *BI = dyn_cast<BranchInst>(Term))
        {
          // Fallback: loop header `br i1 (icmp slt/ult %iv, C), body, exit`
          // with a back-edge into this block — use constant C as trip count
          // when iv starts at 0 (matches SGPL while (i < N) lowering).
          if (BI->isConditional())
          {
            bool hasBackedge = false;
            for (BasicBlock *succ : successors(cur))
            {
              for (BasicBlock *pred : predecessors(cur))
              {
                if (pred == succ)
                {
                  hasBackedge = true;
                  break;
                }
              }
              if (hasBackedge)
                break;
            }
            if (hasBackedge)
            {
              if (auto *Cmp = dyn_cast<ICmpInst>(BI->getCondition()))
              {
                if (Cmp->getPredicate() == ICmpInst::ICMP_SLT ||
                    Cmp->getPredicate() == ICmpInst::ICMP_ULT ||
                    Cmp->getPredicate() == ICmpInst::ICMP_SLE ||
                    Cmp->getPredicate() == ICmpInst::ICMP_ULE)
                {
                  if (auto *C = dyn_cast<ConstantInt>(Cmp->getOperand(1)))
                  {
                    uint64_t trips = C->getZExtValue();
                    if (Cmp->getPredicate() == ICmpInst::ICMP_SLE ||
                        Cmp->getPredicate() == ICmpInst::ICMP_ULE)
                      trips += 1;
                    if (trips > 0)
                      mult *= static_cast<double>(trips);
                  }
                }
              }
            }
          }
        }
      }

      if (cur->hasNPredecessorsOrMore(1))
        cur = *pred_begin(cur);
      else
        break;
    }
    return std::max(1.0, mult);
  }

  static bool isCleanCutStepCall(const Instruction *I)
  {
    if (!I)
      return false;
    if (const auto *CB = dyn_cast<CallBase>(I))
      if (const Function *Callee = CB->getCalledFunction())
        if (Callee && Callee->getName().starts_with("autograph_frontier_step_owner"))
          return true;
    return false;
  }

  Value *resolveGraphRoot(Value *V, const std::map<Value *, GraphMeta> &metaByGraphPtr);

  bool isOutlinedTaskFunction(const Function *F)
  {
    if (!F)
      return false;
    const StringRef N = F->getName();
    return N.starts_with("task_") || N.starts_with("outlined_") ||
           N.starts_with("wrapper_");
  }

  void collectOpEventsFromFunction(Function &F,
                                   const std::map<Value *, GraphMeta> &metaByGraphPtr,
                                   std::vector<OpEvent> &events,
                                   std::set<Function *> &visited);

  void collectOpEventsInCallOrder(Function &F,
                                  const std::map<Value *, GraphMeta> &metaByGraphPtr,
                                  std::vector<OpEvent> &events)
  {
    std::set<Function *> visited;
    collectOpEventsFromFunction(F, metaByGraphPtr, events, visited);
  }

  void collectOpEventsFromFunction(Function &F,
                                   const std::map<Value *, GraphMeta> &metaByGraphPtr,
                                   std::vector<OpEvent> &events,
                                   std::set<Function *> &visited)
  {
    if (!visited.insert(&F).second)
      return;

    Value *lastGraph = nullptr;

    std::map<Value *, Value *> storageToGraph;
    for (BasicBlock &BB : F)
    {
      for (Instruction &I : BB)
      {
        auto *SI = dyn_cast<StoreInst>(&I);
        if (!SI)
          continue;
        Value *storedV = SI->getValueOperand();
        if (!storedV)
          continue;
        if (metaByGraphPtr.count(storedV))
        {
          Value *slot = SI->getPointerOperand()->stripPointerCasts();
          storageToGraph[slot] = storedV;
        }
      }
    }

    // Helper to resolve a graph pointer from an instruction's operands.
    auto resolveGraphPtr = [&](Value *graphPtr) -> Value *
    {
      if (!graphPtr)
        return nullptr;
      if (metaByGraphPtr.count(graphPtr))
        return graphPtr;
      if (Value *stripped = graphPtr->stripPointerCasts())
        graphPtr = stripped;
      if (metaByGraphPtr.count(graphPtr))
        return graphPtr;
      if (Value *stripped = graphPtr->stripPointerCasts())
      {
        if (auto *LI = dyn_cast<LoadInst>(stripped))
        {
          Value *slot = LI->getPointerOperand()->stripPointerCasts();
          auto it = storageToGraph.find(slot);
          if (it != storageToGraph.end())
            return it->second;
        }
      }
      if (Value *root = resolveGraphRoot(graphPtr, metaByGraphPtr))
        return root;
      return nullptr;
    };

    // Always return the canonical init pointer so Traverse/Insert on reloads
    // of the same file-backed graph share one DP schedule.
    auto canonicalize = [&](Value *graphPtr) -> Value *
    {
      if (!graphPtr || !metaByGraphPtr.count(graphPtr))
        return nullptr;
      Value *canon = metaByGraphPtr.at(graphPtr).graphPtr;
      if (Value *stripped = canon->stripPointerCasts())
        canon = stripped;
      return canon;
    };

    auto finalizeGraphPtr = [&](Value *graphPtr) -> Value *
    {
      if (Value *canon = canonicalize(graphPtr))
      {
        lastGraph = canon;
        return canon;
      }
      if (graphPtr && metaByGraphPtr.size() == 1)
      {
        graphPtr = canonicalize(metaByGraphPtr.begin()->first);
        lastGraph = graphPtr;
        return graphPtr;
      }
      if (!graphPtr && lastGraph && metaByGraphPtr.count(lastGraph))
        return lastGraph;
      if (!graphPtr && metaByGraphPtr.size() == 1)
      {
        graphPtr = canonicalize(metaByGraphPtr.begin()->first);
        lastGraph = graphPtr;
        return graphPtr;
      }
      if (!graphPtr && !metaByGraphPtr.empty())
      {
        graphPtr = canonicalize(metaByGraphPtr.begin()->first);
        lastGraph = graphPtr;
        return graphPtr;
      }
      return nullptr;
    };

    for (BasicBlock &BB : F)
    {
      if (Instruction *Term = BB.getTerminator())
      {
        if (MDNode *MD = Term->getMetadata("autotuner.traverse"))
        {
          if (MD->getNumOperands() >= 1)
          {
            if (auto *MDS = dyn_cast<MDString>(MD->getOperand(0)))
            {
              Value *graphPtr = nullptr;
              for (BasicBlock *Succ : successors(&BB))
              {
                for (Instruction &SI : *Succ)
                {
                  if (auto *GEP = dyn_cast<GetElementPtrInst>(&SI))
                  {
                    if (Value *root = resolveGraphRoot(GEP->getPointerOperand(),
                                                       metaByGraphPtr))
                    {
                      graphPtr = root;
                      break;
                    }
                  }
                  if (auto *CB = dyn_cast<CallBase>(&SI))
                  {
                    if (Function *Callee = resolveCallee(CB))
                    {
                      if (Callee->getName() == "autograph_neighbor_iter_init" &&
                          CB->arg_size() > 0)
                      {
                        graphPtr = resolveGraphRoot(CB->getArgOperand(0), metaByGraphPtr);
                        if (graphPtr)
                          break;
                      }
                    }
                  }
                  if (auto *LI = dyn_cast<LoadInst>(&SI))
                  {
                    if (Value *root = resolveGraphRoot(LI, metaByGraphPtr))
                    {
                      graphPtr = root;
                      break;
                    }
                  }
                }
                if (graphPtr)
                  break;
              }
              if (!graphPtr)
              {
                for (Instruction &SI : BB)
                {
                  if (auto *GEP = dyn_cast<GetElementPtrInst>(&SI))
                  {
                    if (Value *root = resolveGraphRoot(GEP->getPointerOperand(),
                                                       metaByGraphPtr))
                    {
                      graphPtr = root;
                      break;
                    }
                  }
                }
              }
              if (!graphPtr)
                graphPtr = finalizeGraphPtr(nullptr);

              graphPtr = finalizeGraphPtr(graphPtr);
              if (graphPtr && metaByGraphPtr.count(graphPtr))
              {
                double mult = estimateExecMultiplier(&BB);
                events.push_back({RegionType::Traverse, Term, graphPtr, mult});
              }
              (void)MDS;
            }
          }
        }
      }

      for (Instruction &I : BB)
      {
        auto *CB = dyn_cast<CallBase>(&I);
        if (!CB)
          continue;
        Function *Callee = resolveCallee(CB);
        if (Callee && isOutlinedTaskFunction(Callee) && !Callee->isDeclaration())
        {
          collectOpEventsFromFunction(*Callee, metaByGraphPtr, events, visited);
          continue;
        }
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

  std::vector<Region> buildRegions(const std::vector<OpEvent> &events)
  {
    std::vector<Region> regions;
    if (events.empty())
      return regions;

    Region cur;
    cur.dominant = events[0].type;
    cur.anchor = events[0].call;
    cur.graphPtr = events[0].graphPtr;
    std::array<uint64_t, 4> counts = {0, 0, 0, 0};

    auto flushRegion = [&]()
    {
      if (cur.totalOps == 0)
        return;
      for (int i = 0; i < 4; ++i)
        cur.freq[i] = static_cast<double>(counts[i]) / static_cast<double>(cur.totalOps);
      regions.push_back(cur);
    };

    for (const OpEvent &ev : events)
    {
      if (ev.type != cur.dominant)
      {
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

  // True for region types whose required layout is forced by the operations
  // they contain: SetQuery forces LAYOUT_SET (bitmap ops), CSRQuery forces
  // LAYOUT_CSR (row_ptr/col_idx readers). Surrounding regions may still pick
  // any feasible layout — buildRegions splits these into their own regions.
  bool isForcedLayoutRegion(RegionType t)
  {
    return t == RegionType::SetQuery || t == RegionType::CSRQuery;
  }

  std::vector<Region> mergeSmallRegions(std::vector<Region> &&raw)
  {
    const char *strictRegions = std::getenv("AUTOTUNER_PROFILE_STRICT_REGIONS");
    if (strictRegions && std::string(strictRegions) == "1")
      return std::move(raw);
    if (raw.size() <= 1)
      return std::move(raw);

    constexpr uint64_t kMergeThreshold = 4;
    std::vector<Region> merged;
    merged.push_back(raw[0]);

    for (size_t i = 1; i < raw.size(); ++i)
    {
      Region &prev = merged.back();
      const Region &cur = raw[i];

      // Forced-layout regions must not merge with non-forced or with each
      // other (conflicting forced layouts like SetQuery vs CSRQuery are
      // impossible to satisfy).  Only same-type forced regions may merge.
      if (isForcedLayoutRegion(prev.dominant) || isForcedLayoutRegion(cur.dominant))
      {
        if (prev.dominant != cur.dominant)
        {
          merged.push_back(cur);
          continue;
        }
      }

      bool canMerge = false;
      const double prevEff = regionEffectiveOps(prev);
      const double curEff = regionEffectiveOps(cur);
      if (cur.dominant == prev.dominant)
        canMerge = true;
      else if (curEff <= 1.0 && prevEff >= curEff)
        canMerge = true;
      else if (prevEff <= 1.0 && curEff >= prevEff)
      {
        prev.dominant = cur.dominant;
        prev.anchor = cur.anchor;
        canMerge = true;
      }
      else if (prevEff < static_cast<double>(kMergeThreshold) &&
               curEff < static_cast<double>(kMergeThreshold))
      {
        prev.dominant = (prevEff >= curEff) ? prev.dominant : cur.dominant;
        canMerge = true;
      }

      if (canMerge)
      {
        // Weight frequencies by effective ops (sites × trip count H).
        const double prevW = prevEff;
        const double curW = curEff;
        const double newW = prevW + curW;
        if (newW > 0)
        {
          for (int k = 0; k < 4; ++k)
            prev.freq[k] = (prev.freq[k] * prevW + cur.freq[k] * curW) / newW;
        }
        prev.totalOps = prev.totalOps + cur.totalOps;
        // Combined region executes with the larger enclosing trip factor;
        // keep totalOps as raw site count so cost = H · totalOps stays consistent
        // when both sides share the same H, and when merging different Hs the
        // weighted freqs already reflect relative work.
        prev.execCount = std::max(prev.execCount, cur.execCount);
        if (prevW + curW > 0 && prev.totalOps > 0)
        {
          // Recompute H so effOps(merged) ≈ prevEff + curEff.
          prev.execCount = (prevW + curW) / static_cast<double>(prev.totalOps);
        }
      }
      else
      {
        merged.push_back(cur);
      }
    }
    return merged;
  }

  int injectConversions(Module &M, const std::vector<Region> &regions, const LayoutSchedule &S,
                        const std::map<Value *, GraphMeta> &metaByGraphPtr, double estN,
                        double estM, const HwCalib &hw)
  {
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

    for (size_t i = 0; i < regions.size(); ++i)
    {
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
          regions[i - 1].anchor->getFunction() == R.anchor->getFunction())
      {
        B.CreateCall(profileExitFn, {ConstantInt::get(i32Ty, static_cast<int>(i - 1))});
      }

      bool mustGuard = isForcedLayoutRegion(R.dominant);
      bool shouldSwitch = true;
      if (target == current)
        shouldSwitch = false;
      if (!mustGuard && forcedLayout < 0 &&
          regionEffectiveOps(R) < static_cast<double>(kMinRegionOpsToSwitch))
        shouldSwitch = false;
      if (!mustGuard && forcedLayout < 0)
      {
        const double stay = (current >= 0 && current < LAYOUT_COUNT)
                                ? S.suffixCost[i][current]
                                : kInf;
        double conv = conversionCost(current, target, estN, estM, hw);
        if (current != LAYOUT_CSR || target != LAYOUT_CSR)
        {
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
      const double predictedNs = operationCost(R, actualLayout, estN, estM,
                                                 G.csrFrac, G.bcsrFrac, hw,
                                                 G.hasClassTiers ? G.classTiers.data() : nullptr,
                                                 G.hasCsrClassTiers ? G.csrClassTiers.data() : nullptr);

      if (shouldSwitch)
      {
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

      /* Step-anchored region: the exit must bracket the step call itself
       * (the step executes mid-function, not at a loop exit). */
      if (isCleanCutStepCall(R.anchor))
      {
        auto *StepCI = cast<CallInst>(R.anchor);
        IRBuilder<> BAfter(StepCI->getNextNode());
        BAfter.CreateCall(profileExitFn,
                          {ConstantInt::get(i32Ty, regionId)});
      }
    }

    std::map<Function *, int> lastRegionByFunction;
    for (size_t i = 0; i < regions.size(); ++i)
      if (regions[i].anchor)
        lastRegionByFunction[regions[i].anchor->getFunction()] = static_cast<int>(i);

    for (const auto &KV : lastRegionByFunction)
    {
      Function *F = KV.first;
      const int regionId = KV.second;
      for (BasicBlock &BB : *F)
      {
        if (auto *RI = dyn_cast<ReturnInst>(BB.getTerminator()))
        {
          IRBuilder<> B(RI);
          B.CreateCall(profileExitFn, {ConstantInt::get(i32Ty, regionId)});
        }
      }
    }
    return injected;
  }

  Value *resolveGraphRoot(Value *V, const std::map<Value *, GraphMeta> &metaByGraphPtr)
  {
    for (int depth = 0; depth < 12 && V; ++depth)
    {
      V = V->stripPointerCasts();
      if (metaByGraphPtr.count(V))
        return V;
      if (auto *LI = dyn_cast<LoadInst>(V))
      {
        V = LI->getPointerOperand();
        continue;
      }
      if (auto *GEP = dyn_cast<GetElementPtrInst>(V))
      {
        V = GEP->getPointerOperand();
        continue;
      }
      if (auto *AI = dyn_cast<AllocaInst>(V))
      {
        if (metaByGraphPtr.count(AI))
          return AI;
        break;
      }
      break;
    }
    return nullptr;
  }

  bool moduleRequiresCSRLayout(Module &M,
                               const std::map<Value *, GraphMeta> &metaByGraphPtr)
  {
    // CSR-specific runtime calls are now handled by CSRQuery regions, not
    // here.  Only direct GEP access to row_ptr / col_idx / weights in the
    // graph struct (e.g. neighbor loops) requires CSR pinning.
    //
    // row_ptr → field 2 (or byte offset +16), col_idx → field 3 (+24),
    // weights → field 4 (+32).
    //
    // Also pin when the module uses weight()/neighbor-iter APIs: those need
    // stable CSR for the whole loop.  Otherwise SetQuery regions from user
    // `set` roaring ops (frontier, setSize) get attributed to the graph, the
    // cost model inserts ensure_layout(SET), Polly loop-rotate peels the
    // first neighbor_iter_next before ensure_layout(CSR), and the iterator
    // aborts on epoch mismatch after one neighbor.
    for (Function &F : M)
    {
      if (F.isDeclaration())
        continue;
      for (BasicBlock &BB : F)
      {
        for (Instruction &I : BB)
        {
          if (auto *CB = dyn_cast<CallBase>(&I))
          {
            Function *Callee = CB->getCalledFunction();
            if (!Callee)
              continue;
            const StringRef name = Callee->getName();
            if (name == "graph_get_edge_weight" ||
                name == "graph_get_edge_weight_by_id" ||
                name == "autograph_neighbor_iter_init" ||
                name == "autograph_neighbor_iter_next")
              return true;
          }

          auto *GEP = dyn_cast<GetElementPtrInst>(&I);
          if (!GEP)
            continue;
          if (!resolveGraphRoot(GEP->getPointerOperand(), metaByGraphPtr))
            continue;

          // Opaque-pointer IRGen: row_ptr +16, col_idx +24, weights +32 bytes.
          // 1-index GEP operands are [ptr, index0] — the offset is operand 1, not 2.
          if (GEP->getNumIndices() == 1)
          {
            if (auto *byteOff = dyn_cast<ConstantInt>(GEP->getOperand(1)))
            {
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

PreservedAnalyses AutoTunerModulePass::run(Module &M, ModuleAnalysisManager &MAM)
{
  (void)MAM;

  Function *mainFn = M.getFunction("main");
  if (!mainFn || mainFn->isDeclaration())
    return PreservedAnalyses::all();

  std::map<Value *, GraphMeta> metaByGraphPtr;
  if (!collectGraphMeta(*mainFn, metaByGraphPtr))
  {
    return PreservedAnalyses::all();
  }

  std::vector<OpEvent> allEvents;
  collectOpEventsInCallOrder(*mainFn, metaByGraphPtr, allEvents);
  if (allEvents.empty())
  {
    return PreservedAnalyses::all();
  }

  const bool pinCSR = moduleRequiresCSRLayout(M, metaByGraphPtr);

  // Load hardware calibration (cache-line size, access/transfer costs).
  const HwCalib hw = loadHwCalib();
  const int forcedLayout = forcedLayoutFromEnv();
  g_cacheModel = cacheModelFromEnv();

  // Build per-graph event sequences while preserving order.
  // Canonicalize aliases (reloads of the same init graph) onto one key.
  std::map<Value *, std::vector<OpEvent>> eventsByGraph;
  for (const OpEvent &E : allEvents)
  {
    auto it = metaByGraphPtr.find(E.graphPtr);
    if (it == metaByGraphPtr.end())
      continue;
    Value *canon = it->second.graphPtr;
    if (Value *stripped = canon->stripPointerCasts())
      canon = stripped;
    OpEvent canonE = E;
    canonE.graphPtr = canon;
    eventsByGraph[canon].push_back(canonE);
  }

  int totalInjected = 0;
  for (auto &KV : eventsByGraph)
  {
    const Value *graphKey = KV.first;
    std::vector<OpEvent> &events = KV.second;
    if (events.empty())
      continue;

    const auto metaIt = metaByGraphPtr.find(const_cast<Value *>(graphKey));
    if (metaIt == metaByGraphPtr.end())
      continue;
    auto [estN, estM] = estimateGraphSize(metaIt->second);
    const double gCsrFrac = metaIt->second.csrFrac;
    const double gBcsrFrac = metaIt->second.bcsrFrac;
    const double *gClassTiers =
        metaIt->second.hasClassTiers ? metaIt->second.classTiers.data() : nullptr;
    const double *gCsrClassTiers =
        metaIt->second.hasCsrClassTiers ? metaIt->second.csrClassTiers.data() : nullptr;
    std::vector<Region> regions = mergeSmallRegions(buildRegions(events));
    if (regions.empty())
      continue;

    /* CleanCut dual annotation: graph-iterator loops were lowered by the
     * graph-frontier pass into owner-computes step calls.  Annotate each
     * step call as a Traverse region paired with the loop region of the same
     * graph (same totalOps estimate), so predicted-vs-measured and layout
     * decisions cover the executed kernel, not just the residual loops. */
    {
      /* Slot -> graph map: the generated main stores the loaded graph pointer
       * into the @G global; step calls re-load it.  Mirrors the collection
       * logic in collectOpEventsFromFunction. */
      std::map<Value *, Value *> storageToGraph;
      for (Function &Fn : M)
        for (BasicBlock &BB : Fn)
          for (Instruction &I : BB)
            if (auto *SI = dyn_cast<StoreInst>(&I))
              if (metaByGraphPtr.count(SI->getValueOperand()))
                storageToGraph[SI->getPointerOperand()->stripPointerCasts()] =
                    SI->getValueOperand();
      auto graphOfStepArg = [&](Value *V) -> Value * {
        if (!V)
          return nullptr;
        V = V->stripPointerCasts();
        if (metaByGraphPtr.count(V))
          return V;
        if (auto *LI = dyn_cast<LoadInst>(V))
        {
          Value *slot = LI->getPointerOperand()->stripPointerCasts();
          auto it = storageToGraph.find(slot);
          if (it != storageToGraph.end())
            return it->second;
        }
        return nullptr;
      };
      uint64_t loopOps = 0;
      for (const Region &R : regions)
        if (R.graphPtr == graphKey)
          loopOps = std::max<uint64_t>(loopOps, R.totalOps);
      if (loopOps == 0)
        loopOps = static_cast<uint64_t>(estM);
      for (Function &Fn : M)
        for (BasicBlock &BB : Fn)
          for (Instruction &I : BB)
          {
            auto *CB = dyn_cast<CallBase>(&I);
            if (!CB || !isCleanCutStepCall(&I) || CB->arg_size() == 0)
              continue;
            Value *g = graphOfStepArg(CB->getArgOperand(0));
            if (!g || g != const_cast<Value *>(graphKey))
              continue;
            Region SR;
            SR.dominant = RegionType::Traverse;
            SR.anchor = &I;
            SR.graphPtr = const_cast<Value *>(graphKey);
            SR.freq = {1.0, 0.0, 0.0, 0.0};
            SR.totalOps = loopOps;
            SR.execCount = 1.0;
            regions.push_back(SR);
          }
    }

    LayoutSchedule S = solveDP(regions, estN, estM, gCsrFrac, gBcsrFrac, hw,
                               gClassTiers, gCsrClassTiers);
    if (forcedLayout >= 0)
    {
      for (size_t i = 0; i < S.chosen.size(); ++i)
      {
        if (layoutFeasible(regions[i].dominant, forcedLayout))
          S.chosen[i] = forcedLayout;
      }
      // errs() << "[AutoTuner] Forced layout override: "
      //        << layoutName(forcedLayout) << "\n";
    }
    const double chosenCost = estimateChosenScheduleCost(
        regions, S.chosen, estN, estM, gCsrFrac, gBcsrFrac, hw, gClassTiers,
        gCsrClassTiers);
    const double allCSR = estimateAllCSRPathCost(
        regions, estN, estM, gCsrFrac, gBcsrFrac, hw, gClassTiers,
        gCsrClassTiers);

    bool shouldSkip = false;
    if (forcedLayout < 0 && chosenCost < kInf / 2.0 && allCSR < kInf / 2.0)
    {
      const double required = allCSR / kMinBenefitRatio;
      if (!(chosenCost < required))
      {
        shouldSkip = true;
      }
    }
    bool allCSRSchedule = true;
    for (int c : S.chosen)
    {
      if (c != LAYOUT_CSR)
      {
        allCSRSchedule = false;
        break;
      }
    }
    if (forcedLayout < 0 && allCSRSchedule)
    {
      shouldSkip = true;
    }
    // Neighbor loops read row_ptr/col_idx directly from the Graph struct; keep CSR.
    if (forcedLayout < 0 && pinCSR)
    {
      shouldSkip = true;
    }
    if (shouldSkip)
      std::fill(S.chosen.begin(), S.chosen.end(), LAYOUT_CSR);
#if 0
    errs() << "[AutoTuner] Cost-model profile for graph " << graphKey
           << " (estN=" << estN << ", estM=" << estM << ")\n";
    {
      double totalOpCost = 0;
      for (size_t i = 0; i < regions.size(); ++i)
      {
        const double predictedNs =
            operationCost(regions[i], S.chosen[i], estN, estM,
                          gCsrFrac, gBcsrFrac, hw, gClassTiers,
                          gCsrClassTiers);
        totalOpCost += predictedNs;
        errs() << "[AutoTuner]   region=" << i
               << " kind=" << regionTypeName(regions[i].dominant)
               << " layout=" << layoutName(S.chosen[i])
               << " totalOps=" << regions[i].totalOps
               << " execCount=" << regions[i].execCount
               << " predicted_ns=" << predictedNs
               << " predicted_ms=" << (predictedNs / 1.0e6) << "\n";
      }
      const double predictedConversion = chosenCost - totalOpCost;
      errs() << "[AutoTuner]   predicted_op_total_ns=" << totalOpCost
             << " predicted_total_ns=" << chosenCost
             << " predicted_conversion_ns=" << predictedConversion
             << " shouldSkip=" << shouldSkip << "\n";
    }
#endif
    totalInjected += injectConversions(M, regions, S, metaByGraphPtr, estN, estM, hw);
  }

  (void)totalInjected;
  return PreservedAnalyses::none();
}

void runAutoTunerOnModule(Module &M)
{
  ModuleAnalysisManager MAM;
  AutoTunerModulePass P;
  (void)P.run(M, MAM);
}

void registerAutoTunerPass(PassBuilder &PB)
{
  PB.registerPipelineStartEPCallback(
      [](ModulePassManager &MPM, OptimizationLevel)
      { MPM.addPass(AutoTunerModulePass()); });
}
