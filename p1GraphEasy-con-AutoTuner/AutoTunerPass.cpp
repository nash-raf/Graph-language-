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
  double memPenalty(double workingSetBytes, const HwCalib &hw)
  {
    double lo, hi;
    rampBand(hw, lo, hi);
    return rampPenalty(workingSetBytes, lo, hi, hw.P);
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
    char buf[512];
    size_t n = std::fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    std::fclose(f);
    // Minimal JSON parse: look for "L", "t", "T" keys.
    auto extract = [&](const char *key) -> double
    {
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
      return layout == LAYOUT_SET; // bitmap-specific ops force SET
    case RegionType::CSRQuery:
      return layout == LAYOUT_CSR; // row_ptr/col_idx readers force CSR
    }
    return false;
  }

  bool hasPrefix(StringRef s, StringRef pref) { return s.starts_with(pref); }

  bool isTraverseCall(StringRef fn)
  {
    return fn == "bfs_runtime" || fn == "bfs_runtime_src" || fn == "dfs_runtime" ||
           fn == "dfs_runtime_src";
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
  double insertCost(int layout, double n, double m, const HwCalib &hw)
  {
    const double d = (n > 0) ? (2.0 * m / n) : 1.0;
    const double g = kPcsrExpansionFactor;
    const double t = hw.t;
    const double T = hw.T;
    const double L = hw.L;
    const double R = hw.R;
    const double gU = g * d;  // g(u) = physical span of vertex u
    const double u = n / 2.0; // average vertex id for random row_ptr access
    const double p = m / 2.0; // average directed edge id for random col_idx access

    switch (layout)
    {
    case LAYOUT_CSR:
    {
      // One undirected edge = 2 directed inserts (from→to, to→from).
      // Per directed: realloc + memmove + row_ptr prefix-sum update.
      // C_locate  = 4t                    (2 dirs × 2t: row_ptr[from+1] random read)
      // C_prefix  = 2·⌈n/2·8/L⌉·2T·pen    (2 dirs × n/2 int64 R-M-W entries;
      //                                 dependent load→store, cannot overlap, and
      //                                 DRAM-bound once the prefix array outgrows LLC)
      // C_move    = 2·⌈4m/L⌉·Tm·pen      (2 dirs × ~4m bytes memmove, read+write
      //                                 per line via Tm, DRAM-bound past LLC —
      //                                 same physics as BCSR's cMove)
      // C_realloc = 2·R                   (2 dirs × mremap O(1))
      // C_write   = 2                     (2 dirs × write 1 int32)
      const double cLocate = 2 * t + std::ceil(d * 4 / L) * T;
      const double prefixBytes = (n - u) * 8.0;
      const double cWrite =
          1.0 * t + std::ceil(prefixBytes / L) * 2.0 * T *
                        memPenalty(prefixBytes, hw);
      const double moveBytes = 4.0 * (m - p);
      const double cMove =
          std::ceil(moveBytes / L) * hw.Tm * memPenalty(moveBytes, hw);
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
      // Runtime (autograph_bcsr_add_edge): dup-scan the block row, realloc
      // bcol by +2 ints, memmove everything after the insertion point, bump
      // the brow prefix sums.  bcol backing array = 4m int32 (2 ints per
      // directed edge, 2m directed edges) = 16m bytes.
      // C_locate  = 4t + ⌈8·b·d/L⌉·T   (2 dirs × brow reads + dup/insert-point
      //                                 scan of ~half the 8·b·d-byte block row)
      // C_write   = ⌈nb·4/L⌉·2T        (brow prefix R-M-W, both directions)
      // C_move    = ⌈(16m/2)/L⌉·Tm·pen (shift ~half the backing array; memmove
      //                                 pays read+write per line (Tm, measured),
      //                                 DRAM-bound once it outgrows the LLC)
      // C_realloc = R                  (realloc(+8B) extends in place /
      //                                 mremap: O(1) policy cap, no O(m) copy)
      const double b = kBcsrBlockSize;
      const double nb = std::ceil(n / b);
      const double arrBytes = 16.0 * m;
      const double cLocate = 4.0 * t + std::ceil(8.0 * b * d / L) * T;
      const double cWrite = std::ceil(nb * 4.0 / L) * 2.0 * T;
      const double cMove =
          std::ceil((arrBytes / 2.0) / L) * hw.Tm * memPenalty(arrBytes, hw);
      const double cRealloc = R;
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
  // Plus one extra_edge_pairs realloc event: R.
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
    const double cHashBuild = hashInsertCost(m, hw) * m;
    return cHashBuild + hw.R;
  }

  double alphaFromCSR(int to)
  {
    // Cost of converting FROM CSR TO the target layout.
    // CSR→SET:  ~O(1) — just free CSR arrays, bitmaps already canonical
    // CSR→PCSR: O(E) — allocate padded arrays, copy with gap slots
    // CSR→BCSR: O(E) — block-structured copy
    switch (to)
    {
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

  double alphaToCSR(int from)
  {
    // Cost of converting TO CSR FROM the source layout.
    // SET→CSR:  O(E·log) — full rebuild from bitmaps (hash map + sort + edge iteration)
    // PCSR→CSR: O(E) — simple gap compaction (linear scan, no hashing)
    // BCSR→CSR: O(E) — unblock and reconstruct
    switch (from)
    {
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

  // Physics-based conversion cost for the CSR -> SET path.
  // autograph_ensure_layout_set (autotuner_runtime.c) does, before
  // profile_region_enter fires:
  //   1) rebuild_sets_from_csr_meta: builds and destroys its own local
  //      EdgeHashMap over the m static pairs (hashInsertCost per pair),
  //      then scans the CSR arrays to repopulate the bitmaps.
  //   2) refresh_graph_counts_from_canonical: canonical_node_span triggers
  //      one O(n) select-cache rebuild of the nodes bitmap (write 4n bytes +
  //      read ~n/8 bytes), and canonical_edge_count_cached runs the full
  //      O(m) roaring_bitmap_contains scan (hw.c per check) because
  //      live_edge_count was just invalidated.
  // These replace the crude alpha=0.1*(n+m) charge that the old model used
  // for CSR->SET.  Other transitions keep the alpha-scaled approximation.
  double conversionCostCSRToSET(double n, double m, const HwCalib &hw)
  {
    const double cHashBuild = hashInsertCost(m, hw) * m;
    const double cEdgeCount = hw.c * m;
    const double cSelCache =
        (std::ceil(4.0 * n / hw.L) + std::ceil(n / 8.0 / hw.L)) * hw.T;
    // Plus the CSR array scan in rebuild_sets_from_csr_meta: ~4m bytes read.
    const double cCsrScan = std::ceil(4.0 * m / hw.L) * hw.T;
    return cHashBuild + cEdgeCount + cSelCache + cCsrScan;
  }

  double conversionCost(int from, int to, double n, double m,
                        const HwCalib &hw)
  {
    if (from == to)
      return 0.0;
    if (from == LAYOUT_CSR && to == LAYOUT_SET)
      return conversionCostCSRToSET(n, m, hw);
    const double scale = n + m;
    if (from == LAYOUT_CSR)
      return alphaFromCSR(to) * scale;
    if (to == LAYOUT_CSR)
      return alphaToCSR(from) * scale;
    // Route non-CSR transitions through CSR hub.  A non-CSR -> SET transition
    // routes X -> CSR -> SET, so it picks up the physics-based CSR->SET leg.
    if (to == LAYOUT_SET)
      return alphaToCSR(from) * scale + conversionCostCSRToSET(n, m, hw);
    return (alphaToCSR(from) + alphaFromCSR(to)) * scale;
  }

  double operationCost(const Region &r, int layout, double n, double m,
                       const HwCalib &hw)
  {
    // Execution cost of a region under the chosen layout L.
    //   operationCost(R, L) = H · totalOps · ( f_T · uTrav(L) + f_I · uIns(L) )
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
    const double uIns = insertCost(layout, n, m, hw);

    // One-time lazy-init cost for the first insert under this layout (e.g.
    // SET's static-hash build).  Charged once per region, not per op: the
    // built structures persist across the region's inserts.
    const double setup = (fI > 0.0) ? insertSetupCost(layout, n, m, hw) : 0.0;

    return setup + H * totalOps * (fT * uTrav + fI * uIns);
  }

  double estimateAllCSRPathCost(const std::vector<Region> &regions, double estN,
                                double estM, const HwCalib &hw)
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
      total += operationCost(R, LAYOUT_CSR, estN, estM, hw);
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
                                    double estM, const HwCalib &hw)
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
      total += operationCost(regions[i], layout, estN, estM, hw);
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
                         double estM, const HwCalib &hw)
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
                 operationCost(regions[0], l, estN, estM, hw);
    }

    for (int i = 1; i < R; ++i)
    {
      for (int cur = 0; cur < LAYOUT_COUNT; ++cur)
      {
        if (!layoutFeasible(regions[i].dominant, cur))
          continue;
        const double runCost = operationCost(regions[i], cur, estN, estM, hw);
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
      S.suffixCost[R - 1][l] = operationCost(regions[R - 1], l, estN, estM, hw);
    }
    for (int i = R - 2; i >= 0; --i)
    {
      for (int l = 0; l < LAYOUT_COUNT; ++l)
      {
        if (!layoutFeasible(regions[i].dominant, l))
          continue;
        const double here = operationCost(regions[i], l, estN, estM, hw);
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

  double estimateExecMultiplier(BasicBlock *BB)
  {
    constexpr double kDefaultTripCount = 8.0;
    double mult = 1.0;
    SmallPtrSet<BasicBlock *, 8> visited;
    BasicBlock *cur = BB;
    while (cur && visited.insert(cur).second)
    {
      bool isLoopBody = false;
      for (BasicBlock *succ : successors(cur))
      {
        for (BasicBlock *pred : predecessors(cur))
        {
          if (pred == succ)
          {
            isLoopBody = true;
            break;
          }
        }
        if (isLoopBody)
          break;
      }
      for (BasicBlock *pred : predecessors(cur))
      {
        auto *TI = pred->getTerminator();
        if (auto *BI = dyn_cast<BranchInst>(TI))
        {
          if (BI->isConditional())
          {
            for (BasicBlock *s : successors(pred))
            {
              if (s == cur)
              {
                for (BasicBlock *pp : predecessors(pred))
                {
                  if (pp == cur || pp == BB)
                  {
                    isLoopBody = true;
                    break;
                  }
                }
              }
              if (isLoopBody)
                break;
            }
          }
        }
        if (isLoopBody)
          break;
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
                       std::vector<OpEvent> &events)
  {
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
      return nullptr;
    };

    // Helper to finalize graph pointer with fallbacks.
    auto finalizeGraphPtr = [&](Value *graphPtr) -> Value *
    {
      if (graphPtr && metaByGraphPtr.count(graphPtr))
      {
        lastGraph = graphPtr;
        return graphPtr;
      }
      if (graphPtr && metaByGraphPtr.size() == 1)
      {
        graphPtr = metaByGraphPtr.begin()->first;
        lastGraph = graphPtr;
        return graphPtr;
      }
      if (!graphPtr && lastGraph && metaByGraphPtr.count(lastGraph))
        return lastGraph;
      if (!graphPtr && metaByGraphPtr.size() == 1)
      {
        graphPtr = metaByGraphPtr.begin()->first;
        lastGraph = graphPtr;
        return graphPtr;
      }
      // Multi-graph fallback: use the first graph registered from autograph_init
      if (!graphPtr && !metaByGraphPtr.empty())
      {
        graphPtr = metaByGraphPtr.begin()->first;
        lastGraph = graphPtr;
        return graphPtr;
      }
      return nullptr;
    };

    for (BasicBlock &BB : F)
    {
      // Check for autotuner.traverse metadata on the BB terminator (foreach loops).
      if (Instruction *Term = BB.getTerminator())
      {
        if (MDNode *MD = Term->getMetadata("autotuner.traverse"))
        {
          if (MD->getNumOperands() >= 1)
          {
            if (auto *MDS = dyn_cast<MDString>(MD->getOperand(0)))
            {
              // This BB is a foreach loop header — classify as Traverse.
              // Resolve the graph pointer by scanning the loop body for
              // a GetElementPtrInst into struct.Graph.
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
                  // Check the graph pointer used in autograph_get_neighbors calls
                  if (auto *CB = dyn_cast<CallBase>(&SI))
                  {
                    if (Function *Callee = resolveCallee(CB))
                    {
                      if (Callee->getName() == "autograph_get_neighbors" &&
                          CB->arg_size() > 0)
                      {
                        graphPtr = resolveGraphRoot(CB->getArgOperand(0), metaByGraphPtr);
                        if (graphPtr)
                          break;
                      }
                    }
                  }
                  // Check LoadInsts from graph storage (e.g., load ptr from @G)
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
              // Also scan the header BB itself (for vertex loops, the GEP
              // to field 0 is in the header, not a successor)
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
              // Fallback: if still no graph ptr, try lastGraph or single-graph heuristic
              if (!graphPtr)
              {
                graphPtr = finalizeGraphPtr(nullptr);
              }

              graphPtr = finalizeGraphPtr(graphPtr);
              if (graphPtr && metaByGraphPtr.count(graphPtr))
              {
                double mult = estimateExecMultiplier(&BB);
                events.push_back({RegionType::Traverse, Term, graphPtr, mult});
              }
              (void)MDS; // metadata value ("neighbor"/"edge"/"vertex") — all are Traverse
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
      if (cur.dominant == prev.dominant)
        canMerge = true;
      else if (cur.totalOps <= 1 && prev.totalOps >= cur.totalOps)
        canMerge = true;
      else if (prev.totalOps <= 1 && cur.totalOps >= prev.totalOps)
      {
        prev.dominant = cur.dominant;
        prev.anchor = cur.anchor;
        canMerge = true;
      }
      else if (prev.totalOps < kMergeThreshold && cur.totalOps < kMergeThreshold)
      {
        prev.dominant = (prev.totalOps >= cur.totalOps) ? prev.dominant : cur.dominant;
        canMerge = true;
      }

      if (canMerge)
      {
        uint64_t newTotal = prev.totalOps + cur.totalOps;
        if (newTotal > 0)
        {
          for (int k = 0; k < 4; ++k)
            prev.freq[k] = (prev.freq[k] * prev.totalOps + cur.freq[k] * cur.totalOps) /
                           static_cast<double>(newTotal);
        }
        prev.totalOps = newTotal;
        prev.execCount = std::max(prev.execCount, cur.execCount);
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
      if (!mustGuard && forcedLayout < 0 && R.totalOps < kMinRegionOpsToSwitch)
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
      const double predictedNs = operationCost(R, actualLayout, estN, estM, hw);

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
    for (Function &F : M)
    {
      if (F.isDeclaration())
        continue;
      for (BasicBlock &BB : F)
      {
        for (Instruction &I : BB)
        {
          auto *GEP = dyn_cast<GetElementPtrInst>(&I);
          if (!GEP)
            continue;
          if (!resolveGraphRoot(GEP->getPointerOperand(), metaByGraphPtr))
            continue;

          // Opaque-pointer IRGen: row_ptr +16, col_idx +24, weights +32 bytes.
          if (GEP->getNumIndices() == 1)
          {
            if (auto *byteOff = dyn_cast<ConstantInt>(GEP->getOperand(2)))
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
  for (Function &F : M)
  {
    if (F.isDeclaration())
      continue;
    collectOpEvents(F, metaByGraphPtr, allEvents);
  }
  if (allEvents.empty())
  {
    return PreservedAnalyses::all();
  }

  const bool pinCSR = moduleRequiresCSRLayout(M, metaByGraphPtr);

  // Load hardware calibration (cache-line size, access/transfer costs).
  const HwCalib hw = loadHwCalib();
  const int forcedLayout = forcedLayoutFromEnv();

  // Build per-graph event sequences while preserving order.
  std::map<Value *, std::vector<OpEvent>> eventsByGraph;
  for (const OpEvent &E : allEvents)
  {
    if (metaByGraphPtr.count(E.graphPtr))
      eventsByGraph[E.graphPtr].push_back(E);
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
    std::vector<Region> regions = mergeSmallRegions(buildRegions(events));
    if (regions.empty())
      continue;

    LayoutSchedule S = solveDP(regions, estN, estM, hw);
    if (forcedLayout >= 0)
    {
      for (size_t i = 0; i < S.chosen.size(); ++i)
      {
        if (layoutFeasible(regions[i].dominant, forcedLayout))
          S.chosen[i] = forcedLayout;
      }
      errs() << "[AutoTuner] Forced layout override: "
             << layoutName(forcedLayout) << "\n";
    }
    const double chosenCost = estimateChosenScheduleCost(regions, S.chosen, estN, estM, hw);
    const double allCSR = estimateAllCSRPathCost(regions, estN, estM, hw);

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
    errs() << "[AutoTuner] Cost-model profile for graph " << graphKey
           << " (estN=" << estN << ", estM=" << estM << ")\n";
    for (size_t i = 0; i < regions.size(); ++i)
    {
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
