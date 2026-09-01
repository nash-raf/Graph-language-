/*
 * autotuner_runtime.c — Phase-Aware Graph Layout AutoTuner Runtime
 *
 * Implements the Set-Based Architecture constraints.
 * LAYOUT_SET (Roaring bitamps) is the absolute base.
 * Transient adjacency layouts (CSR, PCSR, BCSR) are built on-demand
 * for specific execution phases and destroyed after use.
 */

#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "autotuner_runtime.h"
#include "parallel_runtime.h"

/* Forward declaring external Roaring C API we need */
#ifdef __cplusplus
extern "C" {
#endif
typedef struct roaring_bitmap_s RoaringBitmap;
RoaringBitmap *roaring_bitmap_create(uint64_t arena_size,
                                     uint64_t initial_capacity);
void roaring_bitmap_add(RoaringBitmap *r, uint32_t val);
void roaring_bitmap_remove(RoaringBitmap *r, uint32_t val);
bool roaring_bitmap_contains(const RoaringBitmap *r, uint32_t val);
void roaring_bitmap_clear(RoaringBitmap *r);
uint64_t roaring_bitmap_get_cardinality(const RoaringBitmap *r);
uint32_t roaring_bitmap_get_at_index(const RoaringBitmap *r, uint64_t idx);
#ifdef __cplusplus
}
#endif

/* Struct defined in IRGenVisitor.h for edge lists */
typedef struct {
  int32_t u;
  int32_t v;
} EdgePair;

/* ══════════════════════════════════════════════════════════════════
 *  Per-graph metadata registry
 * ══════════════════════════════════════════════════════════════════ */
#define MAX_GRAPHS 64
static AutoGraphMeta g_meta[MAX_GRAPHS];
static int g_meta_count = 0;

#define AUTOGRAPH_MAX_PROFILE_REGIONS 1024
#define AUTOGRAPH_PROFILE_ENABLED 1

typedef struct {
  atomic_int seen;
  int kind;
  int layout;
  double predicted_ns;
  atomic_uint_fast64_t measured_ns;
  atomic_uint_fast64_t visits;
} AutoProfileRegion;

static AutoProfileRegion g_profile_regions[AUTOGRAPH_MAX_PROFILE_REGIONS];
static pthread_once_t g_profile_atexit_once = PTHREAD_ONCE_INIT;
static __thread int g_active_region_id = -1;
static __thread uint64_t g_active_region_start_ns = 0;
static __thread uint64_t g_neighbor_scan_start_ns = 0;
static atomic_uint_fast64_t g_kernel_measured_ns[3];
static uint64_t g_conversion_ns = 0;
static int g_conversions_injected = 0;

static uint64_t now_monotonic_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static const char *profile_kind_name(int kind) {
  switch (kind) {
  case 0:
    return "Traverse";
  case 1:
    return "Insert";
  case 2:
    return "Query";
  default:
    return "Unknown";
  }
}

static const char *profile_layout_name(int layout) {
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

static void autograph_profile_flush_active(void) {
  if (g_active_region_id < 0 || g_active_region_id >= AUTOGRAPH_MAX_PROFILE_REGIONS)
    return;
  uint64_t end_ns = now_monotonic_ns();
  if (end_ns >= g_active_region_start_ns)
    atomic_fetch_add_explicit(
        &g_profile_regions[g_active_region_id].measured_ns,
        end_ns - g_active_region_start_ns, memory_order_relaxed);
  g_active_region_id = -1;
  g_active_region_start_ns = 0;
}

static void autograph_profile_report(void) {
#if AUTOGRAPH_PROFILE_ENABLED
  autograph_profile_flush_active();

  double predicted_totals[3] = {0.0, 0.0, 0.0};
  uint64_t measured_totals[3] = {0, 0, 0};

  fprintf(stderr, "[AutoTunerProfile] predicted_vs_measured_region_times\n");
  for (int i = 0; i < AUTOGRAPH_MAX_PROFILE_REGIONS; ++i) {
    AutoProfileRegion *region = &g_profile_regions[i];
    if (!region->seen)
      continue;
    int kind = region->kind;
    if (kind >= 0 && kind < 3) {
      predicted_totals[kind] += region->predicted_ns;
      measured_totals[kind] += region->measured_ns;
    }
    fprintf(stderr,
            "[AutoTunerProfile] region=%d kind=%s layout=%s visits=%llu "
            "predicted_ns=%.3f measured_ns=%llu predicted_ms=%.6f measured_ms=%.6f\n",
            i, profile_kind_name(region->kind), profile_layout_name(region->layout),
            (unsigned long long)region->visits, region->predicted_ns,
            (unsigned long long)region->measured_ns, region->predicted_ns / 1.0e6,
            (double)region->measured_ns / 1.0e6);
  }

  for (int kind = 0; kind < 3; ++kind) {
    fprintf(stderr,
            "[AutoTunerProfile] total kind=%s predicted_ns=%.3f measured_ns=%llu "
            "predicted_ms=%.6f measured_ms=%.6f pure_kernel_ns=%llu pure_kernel_ms=%.6f\n",
            profile_kind_name(kind), predicted_totals[kind],
            (unsigned long long)measured_totals[kind], predicted_totals[kind] / 1.0e6,
            (double)measured_totals[kind] / 1.0e6,
            (unsigned long long)atomic_load_explicit(
                &g_kernel_measured_ns[kind], memory_order_relaxed),
            (double)atomic_load_explicit(
                &g_kernel_measured_ns[kind], memory_order_relaxed) / 1.0e6);
  }

  fprintf(stderr, "[AutoTunerProfile] injected %d layout conversions total, conversion_ns=%llu\n",
          g_conversions_injected, (unsigned long long)g_conversion_ns);
#endif
}

/* Forward-declared: per-graph cached hash map for static edge lookup. */
typedef struct EdgeHashEntry_s EdgeHashEntry;
typedef struct EdgeHashMap_s EdgeHashMap;
static EdgeHashMap *g_static_edge_hash[MAX_GRAPHS];  /* indexed same as g_meta */
static EdgeHashMap *g_extra_edge_hash[MAX_GRAPHS];   /* indexed same as g_meta */

static AutoGraphMeta *find_meta(void *graph_ptr) {
  for (int i = 0; i < g_meta_count; i++) {
    if (g_meta[i].graph_ptr == graph_ptr)
      return &g_meta[i];
  }
  return NULL;
}

static AutoGraphMeta *find_or_create_meta(void *graph_ptr) {
  AutoGraphMeta *existing = find_meta(graph_ptr);
  if (existing)
    return existing;
  if (g_meta_count >= MAX_GRAPHS)
    return NULL;

  AutoGraphMeta *meta = &g_meta[g_meta_count++];
  memset(meta, 0, sizeof(*meta));
  meta->graph_ptr = graph_ptr;
  meta->current_layout = LAYOUT_SET; /* Absolute Base State */
  meta->layout_epoch = 1;
  return meta;
}

static void autograph_set_layout(AutoGraphMeta *meta, int32_t layout) {
  if (!meta)
    return;
  if (meta->current_layout != layout)
    meta->layout_epoch++;
  meta->current_layout = layout;
}

static void autograph_profile_install_atexit(void) {
  atexit(autograph_profile_report);
}

void autograph_profile_region_enter(int32_t region_id, int32_t kind,
                                    int32_t layout, double predicted_ns) {
#if !AUTOGRAPH_PROFILE_ENABLED
  (void)region_id;
  (void)kind;
  (void)layout;
  (void)predicted_ns;
  return;
#else
  if (region_id < 0 || region_id >= AUTOGRAPH_MAX_PROFILE_REGIONS)
    return;
  pthread_once(&g_profile_atexit_once, autograph_profile_install_atexit);

  AutoProfileRegion *region = &g_profile_regions[region_id];
  int expected = 0;
  if (atomic_compare_exchange_strong_explicit(
          &region->seen, &expected, -1,
          memory_order_acq_rel, memory_order_acquire)) {
    region->kind = kind;
    region->layout = layout;
    region->predicted_ns = predicted_ns;
    atomic_store_explicit(&region->seen, 1, memory_order_release);
  } else {
    while (atomic_load_explicit(&region->seen, memory_order_acquire) != 1) {
    }
  }
  atomic_fetch_add_explicit(&region->visits, 1, memory_order_relaxed);

  if (g_active_region_id == region_id)
    return;
  autograph_profile_flush_active();
  g_active_region_id = region_id;
  g_active_region_start_ns = now_monotonic_ns();
#endif
}

void autograph_profile_region_exit(int32_t region_id) {
#if !AUTOGRAPH_PROFILE_ENABLED
  (void)region_id;
  return;
#else
  if (region_id < 0 || region_id >= AUTOGRAPH_MAX_PROFILE_REGIONS)
    return;
  if (g_active_region_id != region_id)
    return;
  autograph_profile_flush_active();
#endif
}

void autograph_profile_record_kernel_ns(int32_t kind, uint64_t elapsed_ns) {
#if !AUTOGRAPH_PROFILE_ENABLED
  (void)kind;
  (void)elapsed_ns;
  return;
#else
  if (kind < 0 || kind >= 3)
    return;
  pthread_once(&g_profile_atexit_once, autograph_profile_install_atexit);
  atomic_fetch_add_explicit(&g_kernel_measured_ns[kind], elapsed_ns,
                            memory_order_relaxed);
#endif
}

typedef struct {
  int32_t key;
  int32_t value;
  uint8_t used;
} IdMapEntry;

static uint32_t hash_u32(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}

static int next_pow2_at_least(int need) {
  int cap = 1;
  while (cap < need)
    cap <<= 1;
  return cap;
}

static int cmp_i32(const void *a, const void *b) {
  int32_t av = *(const int32_t *)a;
  int32_t bv = *(const int32_t *)b;
  if (av < bv)
    return -1;
  if (av > bv)
    return 1;
  return 0;
}

/* Forward declarations — needed for hash resize in ensure_extra_capacity. */
static EdgeHashMap *edge_hash_create(int64_t expected);
static void edge_hash_insert(EdgeHashMap *m, int32_t u, int32_t v, int64_t value);
static void edge_hash_destroy(EdgeHashMap *m);
static int meta_index(AutoGraphMeta *meta);

static int ensure_extra_capacity(AutoGraphMeta *meta, int64_t need_count) {
  if (need_count <= meta->extra_edge_capacity)
    return 1;
  int64_t new_cap = meta->extra_edge_capacity > 0 ? meta->extra_edge_capacity : 16;
  while (new_cap < need_count)
    new_cap *= 2;
  int32_t *new_pairs =
      (int32_t *)realloc(meta->extra_edge_pairs, (size_t)(2 * new_cap) * sizeof(int32_t));
  uint8_t *new_live = (uint8_t *)realloc(meta->extra_edge_live, (size_t)new_cap * sizeof(uint8_t));
  if (!new_pairs || !new_live)
    return 0;
  if (new_cap > meta->extra_edge_capacity) {
    memset(new_live + meta->extra_edge_capacity, 0,
           (size_t)(new_cap - meta->extra_edge_capacity) * sizeof(uint8_t));
  }
  meta->extra_edge_pairs = new_pairs;
  meta->extra_edge_live = new_live;
  meta->extra_edge_capacity = new_cap;

  /* Resize the extras hash to match the new capacity. */
  int midx = meta_index(meta);
  if (midx >= 0 && g_extra_edge_hash[midx]) {
    edge_hash_destroy(g_extra_edge_hash[midx]);
    g_extra_edge_hash[midx] = edge_hash_create(new_cap * 2 + 16);
    for (int64_t i = 0; i < meta->extra_edge_count; i++)
      edge_hash_insert(g_extra_edge_hash[midx],
                       meta->extra_edge_pairs[2 * i],
                       meta->extra_edge_pairs[2 * i + 1], i);
  }
  return 1;
}

struct EdgeHashEntry_s {
  uint64_t key;
  int64_t value;
  uint8_t occupied;
};

struct EdgeHashMap_s {
  EdgeHashEntry *table;
  int64_t capacity;
};

static uint64_t edge_key(int32_t u, int32_t v) {
  int32_t lo = u < v ? u : v;
  int32_t hi = u < v ? v : u;
  return ((uint64_t)(uint32_t)lo << 32) | (uint64_t)(uint32_t)hi;
}

static uint64_t hash_u64(uint64_t x) {
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebULL;
  x ^= x >> 31;
  return x;
}

static EdgeHashMap *edge_hash_create(int64_t expected) {
  EdgeHashMap *m = (EdgeHashMap *)malloc(sizeof(EdgeHashMap));
  m->capacity = next_pow2_at_least((int)(expected * 2 + 1));
  m->table = (EdgeHashEntry *)calloc((size_t)m->capacity, sizeof(EdgeHashEntry));
  return m;
}

static void edge_hash_insert(EdgeHashMap *m, int32_t u, int32_t v, int64_t value) {
  uint64_t k = edge_key(u, v);
  uint64_t idx = hash_u64(k) & (uint64_t)(m->capacity - 1);
  while (m->table[idx].occupied) {
    if (m->table[idx].key == k) {
      m->table[idx].value = value;
      return;
    }
    idx = (idx + 1) & (uint64_t)(m->capacity - 1);
  }
  m->table[idx].key = k;
  m->table[idx].value = value;
  m->table[idx].occupied = 1;
}

static int64_t edge_hash_find(const EdgeHashMap *m, int32_t u, int32_t v) {
  if (!m || !m->table || m->capacity <= 0) return -1;
  uint64_t k = edge_key(u, v);
  uint64_t idx = hash_u64(k) & (uint64_t)(m->capacity - 1);
  while (m->table[idx].occupied) {
    if (m->table[idx].key == k)
      return m->table[idx].value;
    idx = (idx + 1) & (uint64_t)(m->capacity - 1);
  }
  return -1;
}

static void edge_hash_destroy(EdgeHashMap *m) {
  if (!m) return;
  free(m->table);
  free(m);
}

static int meta_index(AutoGraphMeta *meta) {
  for (int i = 0; i < g_meta_count; i++)
    if (&g_meta[i] == meta) return i;
  return -1;
}

static EdgeHashMap *get_static_edge_hash(AutoGraphMeta *meta) {
  int idx = meta_index(meta);
  if (idx < 0) return NULL;
  if (!g_static_edge_hash[idx] && meta->edge_pairs_table) {
    g_static_edge_hash[idx] = edge_hash_create(meta->static_pair_count + 16);
    EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;
    for (int64_t i = 0; i < meta->static_pair_count; i++)
      edge_hash_insert(g_static_edge_hash[idx], pairs[i].u, pairs[i].v, i);
  }
  return g_static_edge_hash[idx];
}

static EdgeHashMap *get_or_create_extra_edge_hash(AutoGraphMeta *meta) {
  int idx = meta_index(meta);
  if (idx < 0) return NULL;
  if (!g_extra_edge_hash[idx]) {
    int64_t cap = meta->extra_edge_capacity > 0 ? meta->extra_edge_capacity * 2 + 16 : 64;
    g_extra_edge_hash[idx] = edge_hash_create(cap);
  }
  return g_extra_edge_hash[idx];
}

static void reset_extra_edge_hash(AutoGraphMeta *meta) {
  int idx = meta_index(meta);
  if (idx >= 0 && g_extra_edge_hash[idx]) {
    edge_hash_destroy(g_extra_edge_hash[idx]);
    g_extra_edge_hash[idx] = NULL;
  }
}

static int canonical_pair_find_static(AutoGraphMeta *meta, int32_t u, int32_t v) {
  if (!meta || !meta->edge_pairs_table)
    return -1;
  EdgeHashMap *m = get_static_edge_hash(meta);
  return (int)edge_hash_find(m, u, v);
}

static int64_t canonical_pair_find_extra(AutoGraphMeta *meta, int32_t u, int32_t v) {
  if (!meta || meta->extra_edge_count <= 0)
    return -1;
  EdgeHashMap *m = get_or_create_extra_edge_hash(meta);
  if (!m) return -1;
  return edge_hash_find(m, u, v);
}

static int64_t canonical_edge_count(AutoGraphMeta *meta) {
  if (!meta || !meta->edges_bitmap)
    return 0;
  int64_t count = 0;
  RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
  for (int64_t e = 0; e < meta->static_pair_count; e++) {
    if (roaring_bitmap_contains(eb, (uint32_t)e))
      count++;
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    if (meta->extra_edge_live[i])
      count++;
  }
  return count;
}

static int64_t canonical_edge_count_cached(AutoGraphMeta *meta) {
  if (!meta)
    return 0;
  if (meta->live_edge_count < 0) {
    meta->live_edge_count = canonical_edge_count(meta);
  }
  return meta->live_edge_count;
}

static int64_t canonical_node_span(AutoGraphMeta *meta) {
  if (!meta || !meta->nodes_bitmap)
    return 0;
  RoaringBitmap *nb = (RoaringBitmap *)meta->nodes_bitmap;
  uint64_t card = roaring_bitmap_get_cardinality(nb);
  if (card == 0)
    return 0;
  return (int64_t)roaring_bitmap_get_at_index(nb, card - 1) + 1;
}

static void refresh_graph_counts_from_canonical(AutoGraphMeta *meta) {
  if (!meta || !meta->graph_ptr)
    return;
  char *base = (char *)meta->graph_ptr;
  int64_t nn = canonical_node_span(meta);
  int64_t undirected = canonical_edge_count_cached(meta);
  int64_t directed = undirected * 2;
  *((int64_t *)(base + 0)) = nn;
  *((int64_t *)(base + 8)) = directed;
  meta->csr_n = nn;
  meta->csr_m = directed;
}

/* ══════════════════════════════════════════════════════════════════
 *  Set Base Construction & Rebuilding
 * ══════════════════════════════════════════════════════════════════ */

/* Canonical builder: stable label->dense mapping by sorted node labels and
 * hash lookup, then deterministic CSR construction. */
static void build_csr_from_meta(AutoGraphMeta *meta, int64_t *out_n, int64_t **out_rp,
                                int32_t **out_ci, int64_t *out_m) {
  RoaringBitmap *nb = (RoaringBitmap *)meta->nodes_bitmap;
  RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
  EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;

  int64_t node_count = (int64_t)roaring_bitmap_get_cardinality(nb);
  if (node_count < 0)
    node_count = 0;

  int64_t dense_n = 0;
  if (node_count > 0) {
    int32_t max_label = (int32_t)roaring_bitmap_get_at_index(nb, (uint64_t)(node_count - 1));
    if (max_label >= 0)
      dense_n = (int64_t)max_label + 1;
  }

  int64_t *rp = (int64_t *)calloc((size_t)(dense_n + 1), sizeof(int64_t));
  int64_t edge_count = 0;

  for (int64_t e = 0; e < meta->static_pair_count; e++) {
    if (!roaring_bitmap_contains(eb, (uint32_t)e))
      continue;
    int32_t u = pairs[e].u;
    int32_t v = pairs[e].v;
    if (u >= 0 && v >= 0 && (int64_t)u < dense_n && (int64_t)v < dense_n) {
      rp[(int64_t)u + 1]++;
      rp[(int64_t)v + 1]++;
      edge_count += 2;
    }
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    if (!meta->extra_edge_live[i])
      continue;
    int32_t u = meta->extra_edge_pairs[2 * i];
    int32_t v = meta->extra_edge_pairs[2 * i + 1];
    if (u >= 0 && v >= 0 && (int64_t)u < dense_n && (int64_t)v < dense_n) {
      rp[(int64_t)u + 1]++;
      rp[(int64_t)v + 1]++;
      edge_count += 2;
    }
  }
  for (int64_t u = 1; u <= dense_n; u++)
    rp[u] += rp[u - 1];

  int32_t *ci = edge_count > 0 ? (int32_t *)malloc((size_t)edge_count * sizeof(int32_t)) : NULL;
  int64_t *next = (int64_t *)malloc((size_t)(dense_n + 1) * sizeof(int64_t));
  memcpy(next, rp, (size_t)(dense_n + 1) * sizeof(int64_t));

  for (int64_t e = 0; e < meta->static_pair_count; e++) {
    if (!roaring_bitmap_contains(eb, (uint32_t)e))
      continue;
    int32_t u = pairs[e].u;
    int32_t v = pairs[e].v;
    if (u >= 0 && v >= 0 && (int64_t)u < dense_n && (int64_t)v < dense_n) {
      ci[next[u]++] = v;
      ci[next[v]++] = u;
    }
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    if (!meta->extra_edge_live[i])
      continue;
    int32_t u = meta->extra_edge_pairs[2 * i];
    int32_t v = meta->extra_edge_pairs[2 * i + 1];
    if (u >= 0 && v >= 0 && (int64_t)u < dense_n && (int64_t)v < dense_n) {
      ci[next[u]++] = v;
      ci[next[v]++] = u;
    }
  }

  free(next);

  *out_n = dense_n;
  *out_rp = rp;
  *out_ci = ci;
  *out_m = edge_count;
  meta->csr_owned = 1;
  /* fprintf(stderr, "[AutoTuner] SET->CSR built (stable mapping): n=%ld m=%ld\n",
          (long)dense_n, (long)edge_count); */
}

void build_csr_from_set(int64_t n, int64_t pair_count, void *edges_bitmap, void *edge_pairs_raw,
                        int64_t **out_rp, int32_t **out_ci, int64_t *out_m) {
  (void)n;
  RoaringBitmap *eb = (RoaringBitmap *)edges_bitmap;
  EdgePair *pairs = (EdgePair *)edge_pairs_raw;
  int64_t max_label = -1;
  for (int64_t e = 0; e < pair_count; e++) {
    if (!roaring_bitmap_contains(eb, (uint32_t)e))
      continue;
    if (pairs[e].u > max_label)
      max_label = pairs[e].u;
    if (pairs[e].v > max_label)
      max_label = pairs[e].v;
  }
  int64_t dense_n = max_label + 1;
  int64_t *rp = (int64_t *)calloc((size_t)(dense_n + 1), sizeof(int64_t));
  int64_t m2 = 0;
  for (int64_t e = 0; e < pair_count; e++) {
    if (roaring_bitmap_contains(eb, (uint32_t)e) && pairs[e].u >= 0 &&
        pairs[e].u < dense_n) {
      rp[pairs[e].u + 1]++;
      m2++;
    }
  }
  for (int64_t u = 1; u <= dense_n; u++)
    rp[u] += rp[u - 1];
  int32_t *ci = m2 > 0 ? (int32_t *)malloc((size_t)m2 * sizeof(int32_t)) : NULL;
  int64_t *next = (int64_t *)malloc((size_t)(dense_n + 1) * sizeof(int64_t));
  memcpy(next, rp, (size_t)(dense_n + 1) * sizeof(int64_t));
  for (int64_t e = 0; e < pair_count; e++) {
    if (roaring_bitmap_contains(eb, (uint32_t)e) && pairs[e].u >= 0 &&
        pairs[e].u < dense_n)
      ci[next[pairs[e].u]++] = pairs[e].v;
  }
  free(next);
  *out_rp = rp;
  *out_ci = ci;
  *out_m = m2;
}

void rebuild_sets_from_csr(int64_t n, int64_t pair_count, int64_t m, const int64_t *csr_rp,
                           const int32_t *csr_ci, void *nodes_bitmap,
                           void *edges_bitmap, void *edge_pairs_raw) {
  RoaringBitmap *nb = (RoaringBitmap *)nodes_bitmap;
  RoaringBitmap *eb = (RoaringBitmap *)edges_bitmap;
  EdgePair *pairs = (EdgePair *)edge_pairs_raw;

  roaring_bitmap_clear(nb);
  roaring_bitmap_clear(eb);

  EdgeHashMap *ht = edge_hash_create(pair_count + 16);
  for (int64_t e = 0; e < pair_count; e++)
    edge_hash_insert(ht, pairs[e].u, pairs[e].v, e);

  for (int64_t u = 0; u < n; u++) {
    int64_t start = csr_rp[u];
    int64_t end = csr_rp[u + 1];
    if (end > start)
      roaring_bitmap_add(nb, (uint32_t)u);

    for (int64_t j = start; j < end; j++) {
      int32_t v = csr_ci[j];
      roaring_bitmap_add(nb, (uint32_t)v);
      int64_t eid = edge_hash_find(ht, (int32_t)u, v);
      if (eid >= 0)
        roaring_bitmap_add(eb, (uint32_t)eid);
    }
  }

  edge_hash_destroy(ht);
  /* fprintf(stderr, "[AutoTuner] CSR->SET rebuilt (hash): n=%ld m=%ld\n", (long)n, (long)m); */
}

/* Rebuild bitmaps + extra_edge_pairs from CSR. Used when transitioning to SET
 * from CSR/PCSR so we can skip canonical sync during mutations. */
static void rebuild_sets_from_csr_meta(AutoGraphMeta *meta, const int64_t *csr_rp,
                                       const int32_t *csr_ci, int64_t n, int64_t m) {
  (void)m;
  RoaringBitmap *nb = (RoaringBitmap *)meta->nodes_bitmap;
  RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
  EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;

  roaring_bitmap_clear(nb);
  roaring_bitmap_clear(eb);
  for (int64_t i = 0; i < meta->extra_edge_capacity; i++)
    meta->extra_edge_live[i] = 0;
  meta->extra_edge_count = 0;
  reset_extra_edge_hash(meta);

  EdgeHashMap *ht = edge_hash_create(meta->static_pair_count + 16);
  for (int64_t e = 0; e < meta->static_pair_count; e++)
    edge_hash_insert(ht, pairs[e].u, pairs[e].v, e);

  for (int64_t u = 0; u < n; u++) {
    int64_t start = csr_rp[u];
    int64_t end = csr_rp[u + 1];
    if (end > start)
      roaring_bitmap_add(nb, (uint32_t)u);

    for (int64_t j = start; j < end; j++) {
      int32_t v = csr_ci[j];
      roaring_bitmap_add(nb, (uint32_t)v);
      if ((int64_t)u > (int64_t)v)
        continue;
      int64_t eid = edge_hash_find(ht, (int32_t)u, v);
      if (eid >= 0) {
        roaring_bitmap_add(eb, (uint32_t)eid);
      } else {
        int64_t ex = canonical_pair_find_extra(meta, (int32_t)u, v);
        if (ex >= 0) {
          meta->extra_edge_live[ex] = 1;
        } else if (ensure_extra_capacity(meta, meta->extra_edge_count + 1)) {
          int64_t at = meta->extra_edge_count++;
          meta->extra_edge_pairs[2 * at] = (int32_t)u;
          meta->extra_edge_pairs[2 * at + 1] = v;
          meta->extra_edge_live[at] = 1;
          EdgeHashMap *eh = get_or_create_extra_edge_hash(meta);
          if (eh)
            edge_hash_insert(eh, (int32_t)u, v, at);
        }
      }
    }
  }
  edge_hash_destroy(ht);
  meta->canonical_dirty = 0;
  refresh_graph_counts_from_canonical(meta);
  /* fprintf(stderr, "[AutoTuner] CSR->SET rebuilt (meta): n=%ld m=%ld extra=%ld\n",
          (long)n, (long)m, (long)meta->extra_edge_count); */
}

void autograph_update_csr_pointers(void *graph_ptr, int64_t *row_ptr, int32_t *col_idx) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  /* Only CSR mode: struct.Graph's row_ptr/col_idx alias the CSR arrays here.
   * In PCSR/BCSR mode they alias the transient arrays (or are NULL), and
   * storing them into meta->csr_* would poison later conversions. */
  if (meta && meta->current_layout == LAYOUT_CSR) {
    meta->csr_row_ptr = row_ptr;
    meta->csr_col_idx = col_idx;
  }
}

void autograph_record_adjacency_state(void *graph_ptr, int64_t n, int64_t m,
                                      int64_t *row_ptr, int32_t *col_idx) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta)
    return;
  if (meta->current_layout != LAYOUT_CSR)
    return;
  meta->csr_n = n;
  meta->csr_m = m;
  meta->csr_row_ptr = row_ptr;
  meta->csr_col_idx = col_idx;
}

void autograph_mark_canonical_dirty(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta)
    return;
  meta->canonical_dirty = 1;
}

void autograph_sync_canonical_if_dirty(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->canonical_dirty)
    return;
  autograph_ensure_layout_set(graph_ptr);
}

/* Option B: Convert to SET from current layout. Rebuild bitmaps from adjacency. */
void autograph_ensure_layout_set(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->nodes_bitmap || !meta->edges_bitmap || !meta->edge_pairs_table)
    return;
  if (meta->current_layout == LAYOUT_SET && !meta->canonical_dirty)
    return;

  /* Diagnostic output disabled.
  fprintf(stderr, "[autograph_ensure_layout_set] graph=%p from=%s to=SET\n",
          graph_ptr,
          profile_layout_name(meta->current_layout));
  */

  int64_t *rp = NULL;
  int32_t *ci = NULL;
  int64_t nn = 0, mm = 0;
  int free_rp_ci = 0;

  if (meta->current_layout == LAYOUT_CSR) {
    rp = meta->csr_row_ptr;
    ci = meta->csr_col_idx;
    nn = meta->csr_n;
    mm = meta->csr_m;
    free_rp_ci = meta->csr_owned;
  } else if (meta->current_layout == LAYOUT_PCSR) {
    convert_pcsr_to_csr(meta->csr_n, meta->pcsr_capacity, meta->pcsr_row_ptr,
                        meta->pcsr_col_idx, &rp, &ci, &mm);
    nn = meta->csr_n;
    free_rp_ci = 1;
    free(meta->pcsr_row_ptr);
    meta->pcsr_row_ptr = NULL;
    free(meta->pcsr_col_idx);
    meta->pcsr_col_idx = NULL;
  } else if (meta->current_layout == LAYOUT_BCSR) {
    convert_bcsr_to_csr(meta->csr_n, meta->bcsr_nblocks, meta->bcsr_block_size,
                        meta->bcsr_brow_ptr, meta->bcsr_bcol_idx, &rp, &ci, &mm);
    nn = meta->csr_n;
    free_rp_ci = 1;
    free(meta->bcsr_brow_ptr);
    meta->bcsr_brow_ptr = NULL;
    free(meta->bcsr_bcol_idx);
    meta->bcsr_bcol_idx = NULL;
  }

  if (rp && ci) {
    meta->live_edge_count = -1;  /* invalidate; rebuild will rewrite bitmap */
    rebuild_sets_from_csr_meta(meta, rp, ci, nn, mm);
  }
  if (free_rp_ci && rp) free(rp);
  if (free_rp_ci && ci) free(ci);

  meta->csr_row_ptr = NULL;
  meta->csr_col_idx = NULL;
  meta->csr_owned = 0;

  char *base = (char *)graph_ptr;
  refresh_graph_counts_from_canonical(meta);  /* will recompute live_edge_count via cache */
  *((int64_t **)(base + 16)) = NULL;
  *((int32_t **)(base + 24)) = NULL;
  autograph_set_layout(meta, LAYOUT_SET);
  meta->canonical_dirty = 0;
}

/* ══════════════════════════════════════════════════════════════════
 *  CSR <-> PCSR <-> BCSR Transient Conversions
 * ══════════════════════════════════════════════════════════════════ */
void convert_csr_to_pcsr(int64_t n, int64_t m, const int64_t *csr_rp,
                         const int32_t *csr_ci, int64_t **out_rp,
                         int32_t **out_ci, int64_t *out_capacity) {
  int64_t cap = (m < 16) ? 32 : m * 2;
  int32_t *pcol = (int32_t *)malloc((size_t)cap * sizeof(int32_t));
  int64_t *prp = (int64_t *)malloc((size_t)(n + 1) * sizeof(int64_t));

  memset(pcol, 0xFF, (size_t)cap * sizeof(int32_t));
  int64_t write_pos = 0;
  for (int64_t u = 0; u < n; u++) {
    int64_t deg = csr_rp[u + 1] - csr_rp[u];
    int64_t gap_slots = (deg > 2) ? deg : 2;
    int64_t row_cap = deg + gap_slots;

    int64_t need = write_pos + row_cap;
    if (need > cap) {
      int64_t old_cap = cap;
      while (cap < need)
        cap *= 2;
      pcol = (int32_t *)realloc(pcol, (size_t)cap * sizeof(int32_t));
      memset(pcol + old_cap, 0xFF, (size_t)(cap - old_cap) * sizeof(int32_t));
    }

    prp[u] = write_pos;
    for (int64_t j = 0; j < deg; j++) {
      pcol[write_pos + j] = csr_ci[csr_rp[u] + j];
    }
    write_pos += row_cap;
  }
  prp[n] = write_pos;

  *out_rp = prp;
  *out_ci = pcol;
  *out_capacity = cap;
  /* fprintf(stderr, "[AutoTuner] Transient CSR->PCSR\n"); */
}

void convert_pcsr_to_csr(int64_t n, int64_t pcsr_cap __attribute__((unused)),
                         const int64_t *pcsr_rp, const int32_t *pcsr_ci,
                         int64_t **out_rp, int32_t **out_ci, int64_t *out_m) {
  const int32_t GAP = -1;
  int64_t *rp = (int64_t *)calloc((size_t)(n + 1), sizeof(int64_t));
  int64_t total = 0;

  for (int64_t u = 0; u < n; u++) {
    int64_t start = pcsr_rp[u];
    int64_t end = pcsr_rp[u + 1];
    int64_t count = 0;
    for (int64_t j = start; j < end; j++) {
      if (pcsr_ci[j] != GAP)
        count++;
    }
    rp[u + 1] = count;
    total += count;
  }
  for (int64_t u = 1; u <= n; u++)
    rp[u] += rp[u - 1];

  int32_t *ci = (int32_t *)malloc((size_t)total * sizeof(int32_t));
  int64_t *next = (int64_t *)malloc((size_t)(n + 1) * sizeof(int64_t));
  memcpy(next, rp, (size_t)(n + 1) * sizeof(int64_t));

  for (int64_t u = 0; u < n; u++) {
    int64_t start = pcsr_rp[u];
    int64_t end = pcsr_rp[u + 1];
    for (int64_t j = start; j < end; j++) {
      if (pcsr_ci[j] != GAP) {
        ci[next[u]++] = pcsr_ci[j];
      }
    }
  }
  free(next);
  *out_rp = rp;
  *out_ci = ci;
  *out_m = total;
  /* fprintf(stderr, "[AutoTuner] Transient PCSR->CSR\n"); */
}

/*
 * BCSR payload format used by this runtime:
 *   - brow[b]..brow[b+1)-1 is the slice for block b
 *   - bcol stores (local_row, col) pairs as consecutive i32 values
 *     [r0, c0, r1, c1, ...]
 * where local_row is in [0, block_size).
 */
void convert_csr_to_bcsr(int64_t n, int64_t m, const int64_t *csr_rp,
                         const int32_t *csr_ci, int32_t **out_brow,
                         int32_t **out_bcol, int32_t block_size,
                         int32_t *out_nblocks) {
  if (!out_brow || !out_bcol || !out_nblocks || block_size <= 0) {
    return;
  }

  int32_t nb = (int32_t)((n + block_size - 1) / block_size);
  int32_t *brow = (int32_t *)calloc((size_t)(nb + 1), sizeof(int32_t));
  if (!brow) {
    return;
  }

  /* First pass: count i32 payload per block (2 ints per edge). */
  int64_t total_ints64 = 0;
  for (int32_t b = 0; b < nb; b++) {
    int64_t start_u = (int64_t)b * block_size;
    int64_t end_u = start_u + block_size;
    if (end_u > n)
      end_u = n;

    int64_t block_edges = 0;
    for (int64_t u = start_u; u < end_u; u++) {
      int64_t deg = csr_rp[u + 1] - csr_rp[u];
      if (deg > 0)
        block_edges += deg;
    }

    int64_t block_ints = block_edges * 2;
    total_ints64 += block_ints;
    if (total_ints64 > INT32_MAX) {
      free(brow);
      return;
    }
    brow[b + 1] = (int32_t)total_ints64;
  }

  int32_t *bcol = NULL;
  if (total_ints64 > 0) {
    bcol = (int32_t *)malloc((size_t)total_ints64 * sizeof(int32_t));
    if (!bcol) {
      free(brow);
      return;
    }
  }

  /* Second pass: emit (local_row, col) pairs by block. */
  for (int32_t b = 0; b < nb; b++) {
    int64_t start_u = (int64_t)b * block_size;
    int64_t end_u = start_u + block_size;
    if (end_u > n)
      end_u = n;

    int32_t write = brow[b];
    for (int64_t u = start_u; u < end_u; u++) {
      int32_t local_row = (int32_t)(u - start_u);
      for (int64_t j = csr_rp[u]; j < csr_rp[u + 1]; j++) {
        bcol[write++] = local_row;
        bcol[write++] = csr_ci[j];
      }
    }
  }

  *out_brow = brow;
  *out_bcol = bcol;
  *out_nblocks = nb;
  (void)m; /* m is implicit in payload length; keep signature stable. */
  /* fprintf(stderr,
          "[AutoTuner] Transient CSR->BCSR (blocks=%d, payload_ints=%ld)\n", nb,
          (long)total_ints64); */
}

void convert_bcsr_to_csr(int64_t n, int32_t nblocks, int32_t block_size,
                         const int32_t *bcsr_brow, const int32_t *bcsr_bcol,
                         int64_t **out_rp, int32_t **out_ci, int64_t *out_m) {
  if (!out_rp || !out_ci || !out_m || !bcsr_brow || block_size <= 0 ||
      nblocks < 0) {
    return;
  }

  int64_t *rp = (int64_t *)calloc((size_t)(n + 1), sizeof(int64_t));
  if (!rp) {
    return;
  }

  /* First pass: count per-row degree from BCSR payload pairs. */
  for (int32_t b = 0; b < nblocks; b++) {
    int64_t start_u = (int64_t)b * block_size;
    int64_t end_u = start_u + block_size;
    if (end_u > n)
      end_u = n;

    int32_t start = bcsr_brow[b];
    int32_t end = bcsr_brow[b + 1];
    for (int32_t k = start; k + 1 < end; k += 2) {
      int32_t local_row = bcsr_bcol[k];
      int64_t u = start_u + (int64_t)local_row;
      if (u >= start_u && u < end_u) {
        rp[u + 1]++;
      }
    }
  }

  for (int64_t u = 1; u <= n; u++) {
    rp[u] += rp[u - 1];
  }

  int64_t total_edges = rp[n];
  int32_t *ci = NULL;
  if (total_edges > 0) {
    ci = (int32_t *)malloc((size_t)total_edges * sizeof(int32_t));
    if (!ci) {
      free(rp);
      return;
    }
  }

  int64_t *next = (int64_t *)malloc((size_t)(n + 1) * sizeof(int64_t));
  if (!next) {
    free(rp);
    free(ci);
    return;
  }
  memcpy(next, rp, (size_t)(n + 1) * sizeof(int64_t));

  /* Second pass: restore CSR col_idx stream. */
  for (int32_t b = 0; b < nblocks; b++) {
    int64_t start_u = (int64_t)b * block_size;
    int64_t end_u = start_u + block_size;
    if (end_u > n)
      end_u = n;

    int32_t start = bcsr_brow[b];
    int32_t end = bcsr_brow[b + 1];
    for (int32_t k = start; k + 1 < end; k += 2) {
      int32_t local_row = bcsr_bcol[k];
      int32_t col = bcsr_bcol[k + 1];
      int64_t u = start_u + (int64_t)local_row;
      if (u >= start_u && u < end_u) {
        ci[next[u]++] = col;
      }
    }
  }

  free(next);
  *out_rp = rp;
  *out_ci = ci;
  *out_m = total_edges;
  /* fprintf(stderr, "[AutoTuner] Transient BCSR->CSR (m=%ld)\n",
          (long)total_edges); */
}

/* ══════════════════════════════════════════════════════════════════
 *  Main Layout Transition Entry
 * ══════════════════════════════════════════════════════════════════ */
void autograph_ensure_layout(void *graph_ptr, int64_t n, int64_t m,
                             int64_t *struct_row_ptr, int32_t *struct_col_idx,
                             void *nodes_bmp, void *edges_bmp,
                             void *edge_pairs_table, int32_t target_layout) {
  (void)n;
  (void)m;
  (void)struct_row_ptr;
  (void)struct_col_idx;
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta) {
    /* fprintf(stderr,
            "[AutoTuner] WARN: ensure_layout on unregistered graph %p "
            "(target=%d). Ignoring.\n",
            graph_ptr, target_layout); */
    return;
  }
  if (nodes_bmp)
    meta->nodes_bitmap = nodes_bmp;
  if (edges_bmp)
    meta->edges_bitmap = edges_bmp;
  if (edge_pairs_table)
    meta->edge_pairs_table = edge_pairs_table;
  if (!meta->nodes_bitmap || !meta->edges_bitmap || !meta->edge_pairs_table)
    return;
  if (target_layout < LAYOUT_CSR || target_layout > LAYOUT_SET)
    return;

  if (meta->current_layout == target_layout) {
    return; // nothing to do
  }

  /* Diagnostic output disabled.
  fprintf(stderr, "[autograph_ensure_layout] graph=%p from=%s to=%s\n",
          graph_ptr,
          profile_layout_name(meta->current_layout),
          profile_layout_name(target_layout));
  */

  uint64_t conv_start = now_monotonic_ns();

  /*
   * CASE 1: Transitioning FROM a transient layout BACK TO the BASE SET
   * Action: Rebuild bitmaps from current layout (lazy sync), then DESTROY the
   * transient layout.
   */
  if (target_layout == LAYOUT_SET) {
    autograph_ensure_layout_set(graph_ptr);
    g_conversion_ns += now_monotonic_ns() - conv_start;
    g_conversions_injected++;
    return;
  }

  /*
   * CASE 2: Transitioning FROM BASE SET to a TRANSIENT LAYOUT
   * Action: Build the layout directly from the Sets.
   */
  if (meta->current_layout == LAYOUT_SET) {
    if (target_layout == LAYOUT_CSR) {
      build_csr_from_meta(meta, &meta->csr_n, &meta->csr_row_ptr, &meta->csr_col_idx, &meta->csr_m);
      if (meta->csr_row_ptr)
        autograph_set_layout(meta, LAYOUT_CSR);
    } else if (target_layout == LAYOUT_PCSR) {
      /* Build CSR first, then PCSR, then destroy CSR */
      int64_t tmp_m;
      int64_t tmp_n;
      int64_t *tmp_rp;
      int32_t *tmp_ci;
      build_csr_from_meta(meta, &tmp_n, &tmp_rp, &tmp_ci, &tmp_m);
      meta->csr_n = tmp_n;
      convert_csr_to_pcsr(tmp_n, tmp_m, tmp_rp, tmp_ci, &meta->pcsr_row_ptr,
                          &meta->pcsr_col_idx, &meta->pcsr_capacity);
      free(tmp_rp);
      free(tmp_ci);
      if (meta->pcsr_row_ptr)
        autograph_set_layout(meta, LAYOUT_PCSR);
    } else if (target_layout == LAYOUT_BCSR) {
      /* Build CSR first, then BCSR, then destroy CSR */
      int64_t tmp_m;
      int64_t tmp_n;
      int64_t *tmp_rp;
      int32_t *tmp_ci;
      build_csr_from_meta(meta, &tmp_n, &tmp_rp, &tmp_ci, &tmp_m);
      meta->csr_n = tmp_n;
      meta->bcsr_block_size = 64;
      convert_csr_to_bcsr(tmp_n, tmp_m, tmp_rp, tmp_ci, &meta->bcsr_brow_ptr,
                          &meta->bcsr_bcol_idx, 64, &meta->bcsr_nblocks);
      free(tmp_rp);
      free(tmp_ci);
      if (meta->bcsr_brow_ptr)
        autograph_set_layout(meta, LAYOUT_BCSR);
    }
  }

  /*
   * CASE 3: Transient to Transient
   * Action: Convert, then immediately destroy the old layout arrays.
   */
  else {
    int64_t tmp_m;
    int64_t *tmp_rp;
    int32_t *tmp_ci;
    int tmp_owned = 0;

    if (meta->current_layout == LAYOUT_PCSR) {
      convert_pcsr_to_csr(meta->csr_n, meta->pcsr_capacity, meta->pcsr_row_ptr,
                          meta->pcsr_col_idx, &tmp_rp, &tmp_ci, &tmp_m);
      free(meta->pcsr_row_ptr);
      meta->pcsr_row_ptr = NULL;
      free(meta->pcsr_col_idx);
      meta->pcsr_col_idx = NULL;
      tmp_owned = 1;
    } else if (meta->current_layout == LAYOUT_BCSR) {
      convert_bcsr_to_csr(meta->csr_n, meta->bcsr_nblocks, meta->bcsr_block_size,
                          meta->bcsr_brow_ptr, meta->bcsr_bcol_idx, &tmp_rp,
                          &tmp_ci, &tmp_m);
      free(meta->bcsr_brow_ptr);
      meta->bcsr_brow_ptr = NULL;
      free(meta->bcsr_bcol_idx);
      meta->bcsr_bcol_idx = NULL;
      tmp_owned = 1;
    } else {
      tmp_m = meta->csr_m;
      tmp_rp = meta->csr_row_ptr;
      tmp_ci = meta->csr_col_idx;
      tmp_owned = meta->csr_owned;
    }

    if (target_layout == LAYOUT_CSR) {
      meta->csr_m = tmp_m;
      meta->csr_row_ptr = tmp_rp;
      meta->csr_col_idx = tmp_ci;
      meta->csr_owned = tmp_owned;
    } else if (target_layout == LAYOUT_PCSR) {
      convert_csr_to_pcsr(meta->csr_n, tmp_m, tmp_rp, tmp_ci, &meta->pcsr_row_ptr,
                          &meta->pcsr_col_idx, &meta->pcsr_capacity);
      if (tmp_owned) { free(tmp_rp); free(tmp_ci); }
      if (meta->current_layout == LAYOUT_CSR) {
        meta->csr_row_ptr = NULL;
        meta->csr_col_idx = NULL;
      }
    } else if (target_layout == LAYOUT_BCSR) {
      meta->bcsr_block_size = 64;
      convert_csr_to_bcsr(meta->csr_n, tmp_m, tmp_rp, tmp_ci, &meta->bcsr_brow_ptr,
                          &meta->bcsr_bcol_idx, 64, &meta->bcsr_nblocks);
      if (tmp_owned) { free(tmp_rp); free(tmp_ci); }
      if (meta->current_layout == LAYOUT_CSR) {
        meta->csr_row_ptr = NULL;
        meta->csr_col_idx = NULL;
      }
    }
    autograph_set_layout(meta, target_layout);
  }

  /* Final Step: expose the ACTIVE layout's arrays in struct.Graph.
   * CSR  → row_ptr/col_idx point to CSR arrays
   * PCSR → row_ptr/col_idx point to PCSR arrays (gap-encoded)
   * SET  → row_ptr/col_idx are NULL (mutations use bitmaps directly)
   */
  char *base = (char *)graph_ptr;
  *((int64_t *)(base + 0)) = meta->csr_n;
  *((int64_t *)(base + 8)) = meta->csr_m;

  if (meta->current_layout == LAYOUT_CSR) {
    *((int64_t **)(base + 16)) = meta->csr_row_ptr;
    *((int32_t **)(base + 24)) = meta->csr_col_idx;
  } else if (meta->current_layout == LAYOUT_PCSR) {
    *((int64_t **)(base + 16)) = meta->pcsr_row_ptr;
    *((int32_t **)(base + 24)) = meta->pcsr_col_idx;
  } else {
    *((int64_t **)(base + 16)) = NULL;
    *((int32_t **)(base + 24)) = NULL;
  }
  g_conversion_ns += now_monotonic_ns() - conv_start;
  g_conversions_injected++;
}

int32_t autograph_get_layout(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (meta)
    return meta->current_layout;
  return LAYOUT_SET; /* assumed SET if not tracked yet */
}

void autograph_debug_print(void *graph_ptr) {
  AutoGraphMeta *meta = NULL;
  for (int i = 0; i < g_meta_count; i++) {
    if (g_meta[i].graph_ptr == graph_ptr) {
      meta = &g_meta[i];
      break;
    }
  }
  if (!meta) {
    /* fprintf(stderr, "[AutoTuner] debug: graph %p is not registered\n",
            graph_ptr); */
    return;
  }

  /* fprintf(stderr,
          "[AutoTuner] debug graph=%p layout=%d csr(n=%ld,m=%ld,rp=%p,ci=%p) "
          "pcsr(cap=%ld,rp=%p,ci=%p) bcsr(block=%d,nblocks=%d,brow=%p,bcol=%p)\n",
          graph_ptr, meta->current_layout, (long)meta->csr_n, (long)meta->csr_m,
          (void *)meta->csr_row_ptr, (void *)meta->csr_col_idx,
          (long)meta->pcsr_capacity, (void *)meta->pcsr_row_ptr,
          (void *)meta->pcsr_col_idx, meta->bcsr_block_size, meta->bcsr_nblocks,
          (void *)meta->bcsr_brow_ptr, (void *)meta->bcsr_bcol_idx); */
}

int autograph_get_meta_handles(void *graph_ptr, void **nodes_bmp, void **edges_bmp,
                               void **edge_pairs) {
  autograph_sync_canonical_if_dirty(graph_ptr);
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta)
    return 0;
  if (nodes_bmp)
    *nodes_bmp = meta->nodes_bitmap;
  if (edges_bmp)
    *edges_bmp = meta->edges_bitmap;
  if (edge_pairs)
    *edge_pairs = meta->edge_pairs_table;
  return 1;
}

int autograph_canonical_add_node(void *graph_ptr, int32_t node_label) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->nodes_bitmap)
    return 0;
  roaring_bitmap_add((RoaringBitmap *)meta->nodes_bitmap, (uint32_t)node_label);
  if (meta->current_layout == LAYOUT_SET)
    refresh_graph_counts_from_canonical(meta);
  meta->canonical_dirty = 0;
  return 1;
}

int autograph_canonical_remove_node(void *graph_ptr, int32_t node_label) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->nodes_bitmap || !meta->edges_bitmap)
    return 0;
  roaring_bitmap_remove((RoaringBitmap *)meta->nodes_bitmap, (uint32_t)node_label);

  EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;
  RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
  int64_t removed_edges = 0;
  for (int64_t i = 0; i < meta->static_pair_count; i++) {
    if (!roaring_bitmap_contains(eb, (uint32_t)i))
      continue;
    if (pairs[i].u == node_label || pairs[i].v == node_label) {
      roaring_bitmap_remove(eb, (uint32_t)i);
      removed_edges++;
    }
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    if (!meta->extra_edge_live[i])
      continue;
    int32_t u = meta->extra_edge_pairs[2 * i];
    int32_t v = meta->extra_edge_pairs[2 * i + 1];
    if (u == node_label || v == node_label) {
      meta->extra_edge_live[i] = 0;
      removed_edges++;
    }
  }
  if (meta->live_edge_count >= 0)
    meta->live_edge_count -= removed_edges;
  if (meta->current_layout == LAYOUT_SET)
    refresh_graph_counts_from_canonical(meta);
  meta->canonical_dirty = 0;
  return 1;
}

int autograph_canonical_add_edge(void *graph_ptr, int32_t u, int32_t v) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->nodes_bitmap || !meta->edges_bitmap)
    return 0;
  roaring_bitmap_add((RoaringBitmap *)meta->nodes_bitmap, (uint32_t)u);
  roaring_bitmap_add((RoaringBitmap *)meta->nodes_bitmap, (uint32_t)v);

  int sidx = canonical_pair_find_static(meta, u, v);
  if (sidx >= 0) {
    RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
    int already_live = roaring_bitmap_contains(eb, (uint32_t)sidx);
    roaring_bitmap_add(eb, (uint32_t)sidx);
    if (!already_live && meta->live_edge_count >= 0)
      meta->live_edge_count++;
    if (meta->current_layout == LAYOUT_SET)
      refresh_graph_counts_from_canonical(meta);
    meta->canonical_dirty = 0;
    return 1;
  }
  int64_t eidx = canonical_pair_find_extra(meta, u, v);
  if (eidx >= 0) {
    if (!meta->extra_edge_live[eidx] && meta->live_edge_count >= 0)
      meta->live_edge_count++;
    meta->extra_edge_live[eidx] = 1;
    if (meta->current_layout == LAYOUT_SET)
      refresh_graph_counts_from_canonical(meta);
    meta->canonical_dirty = 0;
    return 1;
  }
  if (!ensure_extra_capacity(meta, meta->extra_edge_count + 1))
    return 0;
  int64_t at = meta->extra_edge_count++;
  meta->extra_edge_pairs[2 * at] = u;
  meta->extra_edge_pairs[2 * at + 1] = v;
  meta->extra_edge_live[at] = 1;
  EdgeHashMap *eh = get_or_create_extra_edge_hash(meta);
  if (eh)
    edge_hash_insert(eh, u, v, at);
  if (meta->live_edge_count >= 0)
    meta->live_edge_count++;
  if (meta->current_layout == LAYOUT_SET)
    refresh_graph_counts_from_canonical(meta);
  meta->canonical_dirty = 0;
  return 1;
}

int autograph_canonical_remove_edge(void *graph_ptr, int32_t u, int32_t v) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->edges_bitmap)
    return 0;
  int sidx = canonical_pair_find_static(meta, u, v);
  if (sidx >= 0) {
    RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
    int was_live = roaring_bitmap_contains(eb, (uint32_t)sidx);
    roaring_bitmap_remove(eb, (uint32_t)sidx);
    if (was_live && meta->live_edge_count >= 0)
      meta->live_edge_count--;
    if (meta->current_layout == LAYOUT_SET)
      refresh_graph_counts_from_canonical(meta);
    meta->canonical_dirty = 0;
    return 1;
  }
  int64_t eidx = canonical_pair_find_extra(meta, u, v);
  if (eidx >= 0) {
    if (meta->extra_edge_live[eidx] && meta->live_edge_count >= 0)
      meta->live_edge_count--;
    meta->extra_edge_live[eidx] = 0;
    if (meta->current_layout == LAYOUT_SET)
      refresh_graph_counts_from_canonical(meta);
    meta->canonical_dirty = 0;
    return 1;
  }
  if (meta->current_layout == LAYOUT_SET)
    refresh_graph_counts_from_canonical(meta);
  meta->canonical_dirty = 0;
  return 1;
}

/* ══════════════════════════════════════════════════════════════════
 *  BCSR-native mutations
 * ══════════════════════════════════════════════════════════════════ */

/* ── In-situ BCSR shift profiling (env AUTOTUNER_SHIFT_PROFILE=<path>) ──
 * Accumulates wall-time spent in the autograph_bcsr_add_edge memmove and
 * realloc-migration counts; dumps one JSON line per process at exit.
 * Completely inert unless the env var is set. */
static int64_t shift_prof_ns = 0, shift_prof_cnt = 0, shift_prof_bytes = 0;
static int64_t shift_prof_reloc_cnt = 0, shift_prof_reloc_bytes = 0;
static int shift_prof_state = -1; /* -1 uninit, 0 off, 1 on */

static void shift_prof_dump(void) {
  const char *path = getenv("AUTOTUNER_SHIFT_PROFILE");
  if (!path || shift_prof_cnt <= 0) return;
  FILE *f = fopen(path, "a");
  if (!f) return;
  fprintf(f,
          "{\"shift_ns\": %lld, \"shift_cnt\": %lld, \"shift_bytes\": %lld, "
          "\"shift_per_line\": %.4f, \"reloc_cnt\": %lld, "
          "\"reloc_bytes\": %lld}\n",
          (long long)shift_prof_ns, (long long)shift_prof_cnt,
          (long long)shift_prof_bytes,
          shift_prof_bytes > 0
              ? (double)shift_prof_ns / ((double)shift_prof_bytes / 64.0)
              : 0.0,
          (long long)shift_prof_reloc_cnt,
          (long long)shift_prof_reloc_bytes);
  fclose(f);
}

static int shift_prof_on(void) {
  if (shift_prof_state < 0) {
    if (getenv("AUTOTUNER_SHIFT_PROFILE")) {
      shift_prof_state = 1;
      atexit(shift_prof_dump);
    } else {
      shift_prof_state = 0;
    }
  }
  return shift_prof_state;
}

int autograph_bcsr_add_edge(void *graph_ptr, int32_t from, int32_t to) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || meta->current_layout != LAYOUT_BCSR)
    return 0;
  if (!meta->bcsr_brow_ptr || !meta->bcsr_bcol_idx || meta->bcsr_block_size <= 0)
    return 0;
  if (from < 0 || to < 0)
    return 0;

  int32_t b = meta->bcsr_block_size;
  int32_t nb = meta->bcsr_nblocks;
  int32_t *brow = meta->bcsr_brow_ptr;
  int32_t *bcol = meta->bcsr_bcol_idx;

  int32_t blk = from / b;
  int32_t local_row = from % b;
  if (blk < 0 || blk >= nb)
    return 0;

  int32_t start = brow[blk];
  int32_t end = brow[blk + 1];

  /* Scan for duplicate edge in this block row. */
  for (int32_t k = start; k < end; k += 2) {
    if (bcol[k] == local_row && bcol[k + 1] == to)
      return 1; /* already exists */
  }

  /* Find insertion point to maintain ascending local_row order in bcol.
   * All pairs for row r are before pairs for row r+1, etc. */
  int32_t insert_pos = end;
  for (int32_t k = start; k < end; k += 2) {
    if (bcol[k] > local_row) {
      insert_pos = k;
      break;
    }
  }

  /* Insert the new (local_row, col) pair at insert_pos. */
  int32_t total_ints = brow[nb]; /* = 2 * m */
  int32_t *old_bcol = bcol;
  int32_t *new_bcol = (int32_t *)realloc(bcol,
      (size_t)(total_ints + 2) * sizeof(int32_t));
  if (!new_bcol)
    return 0;
  if (shift_prof_on() && new_bcol != old_bcol) {
    shift_prof_reloc_cnt++;
    shift_prof_reloc_bytes += (int64_t)total_ints * 4;
  }

  /* Shift everything after the insertion point. */
  if (total_ints > insert_pos) {
    size_t mv = (size_t)(total_ints - insert_pos) * sizeof(int32_t);
    if (shift_prof_on()) {
      struct timespec ts0, ts1;
      clock_gettime(CLOCK_MONOTONIC, &ts0);
      memmove(&new_bcol[insert_pos + 2], &new_bcol[insert_pos], mv);
      clock_gettime(CLOCK_MONOTONIC, &ts1);
      shift_prof_ns += (int64_t)(ts1.tv_sec - ts0.tv_sec) * 1000000000LL +
                       (int64_t)(ts1.tv_nsec - ts0.tv_nsec);
      shift_prof_cnt++;
      shift_prof_bytes += (int64_t)mv;
    } else {
      memmove(&new_bcol[insert_pos + 2], &new_bcol[insert_pos], mv);
    }
  }
  new_bcol[insert_pos] = local_row;
  new_bcol[insert_pos + 1] = to;

  /* Update brow prefix sums for all subsequent block rows. */
  for (int32_t i = blk + 1; i <= nb; i++)
    brow[i] += 2;

  /* Update meta and Graph struct. */
  meta->bcsr_bcol_idx = new_bcol;
  meta->csr_m += 1;
  char *base = (char *)graph_ptr;
  *((int64_t *)(base + 8)) = meta->csr_m; /* Graph.m */

  return 1;
}

int autograph_bcsr_remove_edge(void *graph_ptr, int32_t from, int32_t to) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || meta->current_layout != LAYOUT_BCSR)
    return 0;
  if (!meta->bcsr_brow_ptr || !meta->bcsr_bcol_idx || meta->bcsr_block_size <= 0)
    return 0;
  if (from < 0 || to < 0)
    return 0;

  int32_t b = meta->bcsr_block_size;
  int32_t nb = meta->bcsr_nblocks;
  int32_t *brow = meta->bcsr_brow_ptr;
  int32_t *bcol = meta->bcsr_bcol_idx;

  int32_t blk = from / b;
  int32_t local_row = from % b;
  if (blk < 0 || blk >= nb)
    return 0;

  int32_t start = brow[blk];
  int32_t end = brow[blk + 1];

  /* Find the edge in this block row. */
  int32_t pos = -1;
  for (int32_t k = start; k < end; k += 2) {
    if (bcol[k] == local_row && bcol[k + 1] == to) {
      pos = k;
      break;
    }
  }
  if (pos < 0)
    return 0; /* edge not found */

  int32_t total_ints = brow[nb];

  /* Shift everything after the removed pair to close the gap. */
  if (pos + 2 < total_ints) {
    memmove(&bcol[pos], &bcol[pos + 2],
            (size_t)(total_ints - pos - 2) * sizeof(int32_t));
  }

  /* Update brow prefix sums for all subsequent block rows. */
  for (int32_t i = blk + 1; i <= nb; i++)
    brow[i] -= 2;

  /* Update meta and Graph struct. */
  meta->csr_m -= 1;
  char *base = (char *)graph_ptr;
  *((int64_t *)(base + 8)) = meta->csr_m; /* Graph.m */

  return 1;
}

/* ══════════════════════════════════════════════════════════════════
 *  Representation-agnostic neighbor iterator
 * ══════════════════════════════════════════════════════════════════ */

void autograph_neighbor_iter_init(void *graph_ptr, int64_t u,
                                  AutoNeighborIter *iter) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  iter->meta = meta;
  iter->u = u;
  iter->layout = LAYOUT_SET;
  iter->epoch = 0;
  g_neighbor_scan_start_ns = now_monotonic_ns();
  if (!meta) {
    iter->pos = iter->end = 0;
    iter->state = 0;
    return;
  }
  iter->layout = meta->current_layout;
  iter->epoch = meta->layout_epoch;
  switch (meta->current_layout) {
  case LAYOUT_CSR:
    if (meta->csr_row_ptr && u >= 0 && u < meta->csr_n) {
      iter->pos = meta->csr_row_ptr[u];
      iter->end = meta->csr_row_ptr[u + 1];
    } else {
      iter->pos = iter->end = 0;
    }
    iter->state = 0;
    break;
  case LAYOUT_PCSR:
    if (meta->pcsr_row_ptr && u >= 0 && u < meta->csr_n) {
      iter->pos = meta->pcsr_row_ptr[u];
      iter->end = meta->pcsr_row_ptr[u + 1];
    } else {
      iter->pos = iter->end = 0;
    }
    iter->state = 0;
    break;
  case LAYOUT_BCSR:
    if (meta->bcsr_brow_ptr && meta->bcsr_bcol_idx && meta->bcsr_block_size > 0 &&
        u >= 0 && u < meta->csr_n) {
      int32_t blk = (int32_t)(u / meta->bcsr_block_size);
      if (blk >= 0 && blk < meta->bcsr_nblocks) {
        iter->pos = meta->bcsr_brow_ptr[blk];
        iter->end = meta->bcsr_brow_ptr[blk + 1];
        iter->state = (int32_t)(u % meta->bcsr_block_size);
      } else {
        iter->pos = iter->end = 0;
        iter->state = 0;
      }
    } else {
      iter->pos = iter->end = 0;
      iter->state = 0;
    }
    break;
  case LAYOUT_SET:
  default:
    iter->pos = 0;
    iter->end = meta->static_pair_count;
    iter->state = 0; /* phase 0 = static edges */
    iter->layout = LAYOUT_SET;
    break;
  }
}

int autograph_neighbor_iter_next(AutoNeighborIter *iter, int32_t *out_v) {
#define AUTOGRAPH_RECORD_SCAN_DONE() do {                                        \
    if (g_neighbor_scan_start_ns != 0) {                                          \
      autograph_profile_record_kernel_ns(0,                                        \
          now_monotonic_ns() - g_neighbor_scan_start_ns);                          \
      g_neighbor_scan_start_ns = 0;                                                \
    }                                                                             \
  } while (0)
  AutoGraphMeta *meta = (AutoGraphMeta *)iter->meta;
  if (!meta || !out_v)
    return 0;

  /* Layout was converted (and transient arrays possibly freed) under us.
   * Common with Polly loop-rotate: ensure_layout is injected at the rotated
   * header after the first peeled next().  Re-bind and continue from the
   * start of the new layout rather than silently truncating the scan. */
  if (iter->epoch != meta->layout_epoch || iter->layout != meta->current_layout) {
    if (!meta->graph_ptr)
      return 0;
    int64_t u = iter->u;
    autograph_neighbor_iter_init(meta->graph_ptr, u, iter);
    meta = (AutoGraphMeta *)iter->meta;
    if (!meta || iter->epoch != meta->layout_epoch ||
        iter->layout != meta->current_layout)
      return 0;
  }

  switch (iter->layout) {
  case LAYOUT_CSR: {
    if (!meta->csr_col_idx || iter->pos >= iter->end) {
      AUTOGRAPH_RECORD_SCAN_DONE();
      return 0;
    }
    *out_v = meta->csr_col_idx[iter->pos++];
    return 1;
  }
  case LAYOUT_PCSR: {
    if (!meta->pcsr_col_idx)
      return 0;
    while (iter->pos < iter->end) {
      int32_t v = meta->pcsr_col_idx[iter->pos++];
      if (v != -1) {
        *out_v = v;
        return 1;
      }
    }
    AUTOGRAPH_RECORD_SCAN_DONE();
    return 0;
  }
  case LAYOUT_BCSR: {
    if (!meta->bcsr_bcol_idx) {
      AUTOGRAPH_RECORD_SCAN_DONE();
      return 0;
    }
    int32_t local_row = iter->state;
    while (iter->pos < iter->end) {
      int32_t r = meta->bcsr_bcol_idx[iter->pos];
      if (r == local_row) {
        *out_v = meta->bcsr_bcol_idx[iter->pos + 1];
        iter->pos += 2;
        return 1;
      } else if (r > local_row) {
        break;
      }
      iter->pos += 2;
    }
    AUTOGRAPH_RECORD_SCAN_DONE();
    return 0;
  }
  case LAYOUT_SET:
  default: {
    /* Phase 0: static edge pairs */
    if (iter->state == 0) {
      RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
      EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;
      if (!eb || !pairs) {
        iter->state = 1;
        iter->pos = 0;
        iter->end = meta->extra_edge_count;
      } else {
        while (iter->pos < iter->end) {
          int64_t e = iter->pos++;
          if (!roaring_bitmap_contains(eb, (uint32_t)e))
            continue;
          if (pairs[e].u == (int32_t)iter->u) {
            *out_v = pairs[e].v;
            return 1;
          } else if (pairs[e].v == (int32_t)iter->u) {
            *out_v = pairs[e].u;
            return 1;
          }
        }
        /* Fall through to extra edges */
        iter->state = 1;
        iter->pos = 0;
        iter->end = meta->extra_edge_count;
      }
    }
    /* Phase 1: extra edges */
    if (!meta->extra_edge_pairs || !meta->extra_edge_live) {
      AUTOGRAPH_RECORD_SCAN_DONE();
      return 0;
    }
    while (iter->pos < iter->end && iter->pos < meta->extra_edge_count) {
      int64_t i = iter->pos++;
      if (!meta->extra_edge_live[i])
        continue;
      int32_t eu = meta->extra_edge_pairs[2 * i];
      int32_t ev = meta->extra_edge_pairs[2 * i + 1];
      if (eu == (int32_t)iter->u) {
        *out_v = ev;
        return 1;
      } else if (ev == (int32_t)iter->u) {
        *out_v = eu;
        return 1;
      }
    }
    AUTOGRAPH_RECORD_SCAN_DONE();
    return 0;
  }
  }
#undef AUTOGRAPH_RECORD_SCAN_DONE
}

typedef struct {
  int32_t *data;
  int32_t size;
  int32_t capacity;
} AutoFrontierLane;

static int autograph_scratch_ensure(AutoGraphMeta *meta, int32_t lane_count);
static void autograph_fill_frontier_membership(AutoGraphMeta *meta,
                                               const int32_t *frontier,
                                               int32_t frontier_size);

static int autograph_envelope_bufs_ensure(AutoGraphMeta *meta) {
  if (!meta || meta->csr_n <= 0)
    return 0;
  int64_t n = meta->csr_n;
  if (meta->scratch_n < n && !autograph_scratch_ensure(meta, 1))
    return 0;
  if (!meta->scratch_cur_frontier)
    meta->scratch_cur_frontier = (int32_t *)calloc((size_t)n, sizeof(int32_t));
  if (!meta->scratch_next_frontier)
    meta->scratch_next_frontier = (int32_t *)calloc((size_t)n, sizeof(int32_t));
  if (!meta->scratch_dest_seen)
    meta->scratch_dest_seen = (int32_t *)calloc((size_t)n, sizeof(int32_t));
  return meta->scratch_cur_frontier && meta->scratch_next_frontier &&
         meta->scratch_dest_seen;
}

int32_t *autograph_scratch_dest_seen(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !autograph_envelope_bufs_ensure(meta))
    return NULL;
  memset(meta->scratch_dest_seen, 0, (size_t)meta->csr_n * sizeof(int32_t));
  return meta->scratch_dest_seen;
}

int32_t *autograph_scratch_next_frontier(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !autograph_envelope_bufs_ensure(meta))
    return NULL;
  return meta->scratch_next_frontier;
}

uint8_t *autograph_scratch_membership(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !autograph_scratch_ensure(meta, 1))
    return NULL;
  return meta->scratch_membership;
}

int32_t autograph_prepare_frontier_array(void *graph_ptr,
                                         const int32_t *frontier,
                                         int32_t frontier_size) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !autograph_scratch_ensure(meta, 1))
    return 0;
  autograph_fill_frontier_membership(meta, frontier, frontier_size);
  return frontier_size;
}

int32_t autograph_prepare_frontier_bitmap(void *graph_ptr, void *frontier_bitmap) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !frontier_bitmap || !autograph_envelope_bufs_ensure(meta))
    return 0;
  RoaringBitmap *bm = (RoaringBitmap *)frontier_bitmap;
  uint64_t card = roaring_bitmap_get_cardinality(bm);
  if (card > (uint64_t)meta->csr_n)
    card = (uint64_t)meta->csr_n;
  for (uint64_t i = 0; i < card; ++i)
    meta->scratch_cur_frontier[i] =
        (int32_t)roaring_bitmap_get_at_index(bm, (uint32_t)i);
  autograph_fill_frontier_membership(meta, meta->scratch_cur_frontier,
                                     (int32_t)card);
  return (int32_t)card;
}

void autograph_commit_frontier_bitmap(void *graph_ptr, void *next_bitmap,
                                      int32_t new_size) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !next_bitmap || !meta->scratch_next_frontier)
    return;
  RoaringBitmap *bm = (RoaringBitmap *)next_bitmap;
  roaring_bitmap_clear(bm);
  if (new_size < 0)
    new_size = 0;
  if ((int64_t)new_size > meta->csr_n)
    new_size = (int32_t)meta->csr_n;
  for (int32_t i = 0; i < new_size; ++i)
    roaring_bitmap_add(bm, (uint32_t)meta->scratch_next_frontier[i]);
}

static int autograph_scratch_ensure(AutoGraphMeta *meta, int32_t lane_count) {
  if (!meta || meta->csr_n <= 0 || lane_count < 1)
    return 0;

  if (meta->scratch_n < meta->csr_n) {
    uint8_t *membership =
        (uint8_t *)realloc(meta->scratch_membership,
                           (size_t)meta->csr_n * sizeof(uint8_t));
    uint8_t *round_member =
        (uint8_t *)realloc(meta->scratch_round_member,
                           (size_t)meta->csr_n * sizeof(uint8_t));
    if (!membership || !round_member)
      abort();
    meta->scratch_membership = membership;
    meta->scratch_round_member = round_member;
    meta->scratch_n = meta->csr_n;
  }

  if (meta->scratch_lane_count < lane_count) {
    AutoFrontierLane *lanes = (AutoFrontierLane *)realloc(
        meta->scratch_lanes, (size_t)lane_count * sizeof(AutoFrontierLane));
    if (!lanes)
      abort();
    for (int32_t lane = meta->scratch_lane_count; lane < lane_count; ++lane) {
      lanes[lane].data = NULL;
      lanes[lane].size = 0;
      lanes[lane].capacity = 0;
    }
    meta->scratch_lanes = lanes;
    meta->scratch_lane_count = lane_count;
  }

  if (meta->scratch_offsets_cap < lane_count + 1) {
    int64_t *offsets = (int64_t *)realloc(
        meta->scratch_offsets, (size_t)(lane_count + 1) * sizeof(int64_t));
    if (!offsets)
      abort();
    meta->scratch_offsets = offsets;
    meta->scratch_offsets_cap = lane_count + 1;
  }

  AutoFrontierLane *lanes = (AutoFrontierLane *)meta->scratch_lanes;
  int64_t initial_lane_capacity =
      (meta->csr_n + lane_count - 1) / lane_count;
  if (initial_lane_capacity > 16384)
    initial_lane_capacity = 16384;
  if (initial_lane_capacity < 64)
    initial_lane_capacity = 64;
  for (int32_t lane = 0; lane < lane_count; ++lane) {
    if (lanes[lane].capacity < (int32_t)initial_lane_capacity) {
      int32_t *data = (int32_t *)realloc(
          lanes[lane].data,
          (size_t)initial_lane_capacity * sizeof(int32_t));
      if (!data)
        abort();
      lanes[lane].data = data;
      lanes[lane].capacity = (int32_t)initial_lane_capacity;
    }
    lanes[lane].size = 0;
  }
  return 1;
}

static void autograph_scratch_reset_lanes(AutoGraphMeta *meta,
                                          int32_t lane_count) {
  AutoFrontierLane *lanes = (AutoFrontierLane *)meta->scratch_lanes;
  for (int32_t lane = 0; lane < lane_count; ++lane)
    lanes[lane].size = 0;
}

static void autograph_scratch_clear_membership(AutoGraphMeta *meta) {
  if (meta->scratch_membership && meta->csr_n > 0)
    memset(meta->scratch_membership, 0, (size_t)meta->csr_n * sizeof(uint8_t));
}

static void autograph_scratch_clear_round_member(AutoGraphMeta *meta) {
  if (meta->scratch_round_member && meta->csr_n > 0)
    memset(meta->scratch_round_member, 0,
           (size_t)meta->csr_n * sizeof(uint8_t));
}

static void autograph_fill_frontier_membership(AutoGraphMeta *meta,
                                               const int32_t *frontier,
                                               int32_t frontier_size) {
  autograph_scratch_clear_membership(meta);
  for (int32_t i = 0; i < frontier_size; ++i) {
    int32_t vertex = frontier[i];
    if (vertex >= 0 && vertex < meta->csr_n)
      meta->scratch_membership[vertex] = 1;
  }
}

static int64_t autograph_frontier_row_work(const AutoGraphMeta *meta,
                                           int32_t vertex);

static int autograph_should_use_pull(const AutoGraphMeta *meta,
                                     const int32_t *frontier,
                                     int32_t frontier_size) {
  if (meta->current_layout == LAYOUT_SET)
    return 0;
  int64_t push_edge_work = 0;
  for (int32_t i = 0; i < frontier_size; ++i) {
    int64_t row_work = autograph_frontier_row_work(meta, frontier[i]);
    if (row_work > INT64_MAX - push_edge_work) {
      push_edge_work = INT64_MAX;
      break;
    }
    push_edge_work += row_work;
  }
  int use_pull = push_edge_work > meta->csr_n;
  const char *mode = getenv("SGPL_FRONTIER_MODE");
  if (mode && strcmp(mode, "push") == 0)
    use_pull = 0;
  else if (mode && strcmp(mode, "pull") == 0)
    use_pull = 1;
  return use_pull;
}

typedef struct {
  AutoGraphMeta *meta;
  const int32_t *frontier;
  AutoFrontierLane *lanes;
  int32_t lane_count;
  int32_t *claim;
  int32_t expected;
  int32_t desired;
  int32_t *parent;
  const uint8_t *frontier_membership;
} AutoFrontierStepEnv;

static void autograph_frontier_append_claimed(AutoFrontierStepEnv *env,
                                              int32_t source,
                                              int32_t destination) {
  int32_t lane_index = sgpl_current_worker_index();
  if (lane_index < 0 || lane_index >= env->lane_count)
    lane_index = 0;
  AutoFrontierLane *lane = &env->lanes[lane_index];
  if (lane->size == lane->capacity) {
    int64_t grown_capacity = lane->capacity ? (int64_t)lane->capacity * 2 : 64;
    if (grown_capacity > INT32_MAX ||
        (uint64_t)grown_capacity > SIZE_MAX / sizeof(int32_t))
      abort();
    int32_t new_capacity = (int32_t)grown_capacity;
    int32_t *new_data =
        (int32_t *)realloc(lane->data, (size_t)new_capacity * sizeof(int32_t));
    if (!new_data)
      abort();
    lane->data = new_data;
    lane->capacity = new_capacity;
  }
  lane->data[lane->size++] = destination;
  if (env->parent)
    env->parent[destination] = source;
}

static void autograph_frontier_publish(AutoFrontierStepEnv *env,
                                       int32_t source, int32_t destination) {
  if (destination < 0 || destination >= env->meta->csr_n)
    return;

  int32_t expected = env->expected;
  if (!atomic_compare_exchange_strong_explicit(
          (_Atomic int32_t *)&env->claim[destination], &expected, env->desired,
          memory_order_relaxed, memory_order_relaxed))
    return;

  autograph_frontier_append_claimed(env, source, destination);
}

static void autograph_frontier_publish_owned(AutoFrontierStepEnv *env,
                                             int32_t source,
                                             int32_t destination) {
  /*
   * Dense pull assigns every destination to exactly one partition.  The
   * parallel loop completes before the next BFS level, so no other worker can
   * read or write this destination's claim during this step.
   */
  env->claim[destination] = env->desired;
  autograph_frontier_append_claimed(env, source, destination);
}

static void autograph_frontier_push_vertex(AutoFrontierStepEnv *env,
                                           int32_t source) {
  AutoGraphMeta *meta = env->meta;
  if (source < 0 || source >= meta->csr_n)
    return;

  switch (meta->current_layout) {
  case LAYOUT_CSR:
    if (meta->csr_row_ptr && meta->csr_col_idx)
      for (int64_t j = meta->csr_row_ptr[source];
           j < meta->csr_row_ptr[source + 1]; ++j)
        autograph_frontier_publish(env, source, meta->csr_col_idx[j]);
    break;
  case LAYOUT_PCSR:
    if (meta->pcsr_row_ptr && meta->pcsr_col_idx)
      for (int64_t j = meta->pcsr_row_ptr[source];
           j < meta->pcsr_row_ptr[source + 1]; ++j) {
        int32_t destination = meta->pcsr_col_idx[j];
        if (destination != -1)
          autograph_frontier_publish(env, source, destination);
      }
    break;
  case LAYOUT_BCSR:
    if (meta->bcsr_brow_ptr && meta->bcsr_bcol_idx &&
        meta->bcsr_block_size > 0) {
      int32_t block_size = meta->bcsr_block_size;
      int32_t block = source / block_size;
      int32_t local_row = source % block_size;
      for (int32_t k = meta->bcsr_brow_ptr[block];
           k < meta->bcsr_brow_ptr[block + 1]; k += 2) {
        int32_t row = meta->bcsr_bcol_idx[k];
        if (row == local_row)
          autograph_frontier_publish(env, source,
                                     meta->bcsr_bcol_idx[k + 1]);
        else if (row > local_row)
          break;
      }
    }
    break;
  case LAYOUT_SET:
  default: {
    RoaringBitmap *edges = (RoaringBitmap *)meta->edges_bitmap;
    EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;
    if (edges && pairs)
      for (int64_t e = 0; e < meta->static_pair_count; ++e) {
        if (!roaring_bitmap_contains(edges, (uint32_t)e))
          continue;
        if (pairs[e].u == source)
          autograph_frontier_publish(env, source, pairs[e].v);
        else if (pairs[e].v == source)
          autograph_frontier_publish(env, source, pairs[e].u);
      }
    for (int64_t e = 0; e < meta->extra_edge_count; ++e) {
      if (!meta->extra_edge_live[e])
        continue;
      int32_t u = meta->extra_edge_pairs[2 * e];
      int32_t v = meta->extra_edge_pairs[2 * e + 1];
      if (u == source)
        autograph_frontier_publish(env, source, v);
      else if (v == source)
        autograph_frontier_publish(env, source, u);
    }
    break;
  }
  }
}

static int32_t autograph_frontier_find_source(const AutoFrontierStepEnv *env,
                                              int32_t destination) {
  AutoGraphMeta *meta = env->meta;
#define AUTOGRAPH_RETURN_IF_FRONTIER(candidate)                                  \
  do {                                                                            \
    int32_t autograph_source_ = (candidate);                                       \
    if (autograph_source_ >= 0 && autograph_source_ < meta->csr_n &&               \
        env->frontier_membership[autograph_source_])                               \
      return autograph_source_;                                                    \
  } while (0)

  switch (meta->current_layout) {
  case LAYOUT_CSR:
    if (meta->csr_row_ptr && meta->csr_col_idx)
      for (int64_t j = meta->csr_row_ptr[destination];
           j < meta->csr_row_ptr[destination + 1]; ++j)
        AUTOGRAPH_RETURN_IF_FRONTIER(meta->csr_col_idx[j]);
    break;
  case LAYOUT_PCSR:
    if (meta->pcsr_row_ptr && meta->pcsr_col_idx)
      for (int64_t j = meta->pcsr_row_ptr[destination];
           j < meta->pcsr_row_ptr[destination + 1]; ++j)
        if (meta->pcsr_col_idx[j] != -1)
          AUTOGRAPH_RETURN_IF_FRONTIER(meta->pcsr_col_idx[j]);
    break;
  case LAYOUT_BCSR:
    if (meta->bcsr_brow_ptr && meta->bcsr_bcol_idx &&
        meta->bcsr_block_size > 0) {
      int32_t block_size = meta->bcsr_block_size;
      int32_t block = destination / block_size;
      int32_t local_row = destination % block_size;
      for (int32_t k = meta->bcsr_brow_ptr[block];
           k < meta->bcsr_brow_ptr[block + 1]; k += 2) {
        int32_t row = meta->bcsr_bcol_idx[k];
        if (row == local_row)
          AUTOGRAPH_RETURN_IF_FRONTIER(meta->bcsr_bcol_idx[k + 1]);
        else if (row > local_row)
          break;
      }
    }
    break;
  default:
    break;
  }
#undef AUTOGRAPH_RETURN_IF_FRONTIER
  return -1;
}

static void autograph_frontier_push_body(int64_t index, void *opaque) {
  AutoFrontierStepEnv *env = (AutoFrontierStepEnv *)opaque;
  autograph_frontier_push_vertex(env, env->frontier[index]);
}

static void autograph_frontier_pull_partition_body(int64_t index,
                                                   void *opaque) {
  AutoFrontierStepEnv *env = (AutoFrontierStepEnv *)opaque;
  int64_t vertex_count = env->meta->csr_n;
  int64_t begin = vertex_count * index / env->lane_count;
  int64_t end = vertex_count * (index + 1) / env->lane_count;

  for (int64_t destination = begin; destination < end; ++destination) {
    if (env->claim[destination] != env->expected)
      continue;
    int32_t source =
        autograph_frontier_find_source(env, (int32_t)destination);
    if (source >= 0)
      autograph_frontier_publish_owned(env, source, (int32_t)destination);
  }
}

static int64_t autograph_frontier_row_work(const AutoGraphMeta *meta,
                                           int32_t vertex) {
  if (!meta || vertex < 0 || vertex >= meta->csr_n)
    return 0;
  switch (meta->current_layout) {
  case LAYOUT_CSR:
    return meta->csr_row_ptr
               ? meta->csr_row_ptr[vertex + 1] - meta->csr_row_ptr[vertex]
               : 0;
  case LAYOUT_PCSR:
    return meta->pcsr_row_ptr
               ? meta->pcsr_row_ptr[vertex + 1] - meta->pcsr_row_ptr[vertex]
               : 0;
  case LAYOUT_BCSR:
    if (meta->bcsr_brow_ptr && meta->bcsr_bcol_idx &&
        meta->bcsr_block_size > 0) {
      int32_t block = vertex / meta->bcsr_block_size;
      int32_t local_row = vertex % meta->bcsr_block_size;
      int64_t work = 0;
      for (int32_t k = meta->bcsr_brow_ptr[block];
           k < meta->bcsr_brow_ptr[block + 1]; k += 2) {
        int32_t row = meta->bcsr_bcol_idx[k];
        if (row == local_row)
          ++work;
        else if (row > local_row)
          break;
      }
      return work;
    }
    return 0;
  default:
    return 0;
  }
}

typedef struct {
  const AutoFrontierLane *lanes;
  const int64_t *offsets;
  int32_t *destination;
} AutoFrontierMergeEnv;

static void autograph_frontier_merge_lane(int64_t index, void *opaque) {
  AutoFrontierMergeEnv *merge = (AutoFrontierMergeEnv *)opaque;
  const AutoFrontierLane *lane = &merge->lanes[index];
  if (lane->size > 0)
    memcpy(merge->destination + merge->offsets[index], lane->data,
           (size_t)lane->size * sizeof(int32_t));
}

static int32_t autograph_scratch_merge_lanes(AutoGraphMeta *meta,
                                             int32_t lane_count,
                                             int32_t *next_frontier,
                                             int32_t initial_next_size) {
  AutoFrontierLane *lanes = (AutoFrontierLane *)meta->scratch_lanes;
  int64_t *offsets = meta->scratch_offsets;
  offsets[0] = initial_next_size;
  for (int32_t lane = 0; lane < lane_count; ++lane) {
    if (lanes[lane].size < 0 ||
        offsets[lane] > meta->csr_n - lanes[lane].size)
      abort();
    offsets[lane + 1] = offsets[lane] + lanes[lane].size;
  }

  AutoFrontierMergeEnv merge = {
      .lanes = lanes,
      .offsets = offsets,
      .destination = next_frontier,
  };
  int64_t appended = offsets[lane_count] - initial_next_size;
  if (appended >= 65536 && lane_count > 1)
    parallel_for_runtime(0, lane_count, 1, autograph_frontier_merge_lane, &merge,
                         0, 0);
  else
    for (int32_t lane = 0; lane < lane_count; ++lane)
      autograph_frontier_merge_lane(lane, &merge);

  return (int32_t)offsets[lane_count];
}

static int32_t autograph_edgemap_cas_first(void *graph_ptr,
                                           const int32_t *frontier,
                                           int32_t frontier_size,
                                           int32_t *next_frontier,
                                           int32_t initial_next_size,
                                           int32_t *claim, int32_t *parent,
                                           int32_t expected, int32_t desired) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !frontier || !next_frontier || !claim || frontier_size <= 0 ||
      meta->csr_n <= 0 || meta->csr_n > INT32_MAX ||
      initial_next_size < 0 || initial_next_size > meta->csr_n)
    return initial_next_size;

  int32_t lane_count = sgpl_configured_worker_count();
  if (lane_count < 1)
    lane_count = 1;
  if (!autograph_scratch_ensure(meta, lane_count))
    return initial_next_size;
  autograph_scratch_reset_lanes(meta, lane_count);

  AutoFrontierLane *lanes = (AutoFrontierLane *)meta->scratch_lanes;
  AutoFrontierStepEnv env = {
      .meta = meta,
      .frontier = frontier,
      .lanes = lanes,
      .lane_count = lane_count,
      .claim = claim,
      .expected = expected,
      .desired = desired,
      .parent = parent,
      .frontier_membership = NULL,
  };

  int use_pull = autograph_should_use_pull(meta, frontier, frontier_size);
  if (use_pull) {
    autograph_fill_frontier_membership(meta, frontier, frontier_size);
    env.frontier_membership = meta->scratch_membership;
    parallel_for_runtime(0, lane_count, 1,
                         autograph_frontier_pull_partition_body, &env, 0, 0);
  } else {
    parallel_for_runtime(0, frontier_size, 1, autograph_frontier_push_body, &env,
                         0, 0);
  }

  return autograph_scratch_merge_lanes(meta, lane_count, next_frontier,
                                       initial_next_size);
}

int32_t autograph_frontier_step(void *graph_ptr,
                                const int32_t *frontier,
                                int32_t frontier_size,
                                int32_t *next_frontier,
                                int32_t initial_next_size,
                                int32_t *claim,
                                int32_t expected,
                                int32_t desired,
                                int32_t *parent) {
  uint64_t start_ns = now_monotonic_ns();
  int32_t result =
      autograph_edgemap_cas_first(graph_ptr, frontier, frontier_size,
                                  next_frontier, initial_next_size, claim,
                                  parent, expected, desired);
  autograph_profile_record_kernel_ns(0, now_monotonic_ns() - start_ns);
  return result;
}

/* ── Generic owner-computes frontier step (Graptor CleanCut model) ──
 *
 * Home-partition assignment: destination d belongs to lane
 *   lane = (int64_t)d * lane_count / n
 * i.e. contiguous ranges of destinations per lane (owner-computes rule).
 * Each lane runs serially over its range, so exactly one worker writes any
 * given destination slot.  The per-pair work_fn is called with
 * (source, destination, local_index, work_env).
 *
 * Source iteration mirrors the existing motif walker: for every in-neighbor
 * `u` of destination d (CSR row d for undirected graphs), if membership is
 * NULL or u is in the frontier, call work_fn(u, d, ...).
 *
 * dest_seen: caller-owned byte array of size n; set to 1 on first visit of
 * each destination (settled destination) and appended to next_frontier.
 */
typedef struct {
  AutoGraphMeta *meta;
  int32_t lane_count;
  sgpl_frontier_pair_fn work_fn;
  void *work_env;
  const uint8_t *membership;
  int32_t *next_frontier;
  int32_t initial_next_size;
  int32_t *dest_seen;
  _Atomic int32_t appended; /* atomic head into next_frontier for push */
  int64_t n;
} AutoOwnerStepEnv;

static void autograph_owner_scan_vertex(AutoOwnerStepEnv *env, int64_t peer) {
  if (peer < 0 || peer >= env->n)
    return;
  AutoGraphMeta *meta = env->meta;
  /* inbound neighbours: transpose row of `peer` for directed graphs; forward
   * (symmetric) CSR row of `peer` for undirected. */
  if (meta->in_row_ptr && meta->in_col_idx) {
    for (int64_t j = meta->in_row_ptr[peer]; j < meta->in_row_ptr[peer + 1];
         ++j) {
      int32_t src = meta->in_col_idx[j];
      if (env->membership == NULL || env->membership[src])
        env->work_fn(src, (int32_t)peer, peer, env->work_env);
    }
    return;
  }
  switch (meta->current_layout) {
  case LAYOUT_CSR:
    if (meta->csr_row_ptr && meta->csr_col_idx)
      for (int64_t j = meta->csr_row_ptr[peer];
           j < meta->csr_row_ptr[peer + 1]; ++j)
        if (env->membership == NULL ||
            env->membership[meta->csr_col_idx[j]])
          env->work_fn(meta->csr_col_idx[j], (int32_t)peer, peer, env->work_env);
    break;
  case LAYOUT_PCSR:
    if (meta->pcsr_row_ptr && meta->pcsr_col_idx)
      for (int64_t j = meta->pcsr_row_ptr[peer];
           j < meta->pcsr_row_ptr[peer + 1]; ++j) {
        int32_t src = meta->pcsr_col_idx[j];
        if (src == -1)
          continue;
        if (env->membership == NULL || env->membership[src])
          env->work_fn(src, (int32_t)peer, peer, env->work_env);
      }
    break;
  case LAYOUT_BCSR:
    if (meta->bcsr_brow_ptr && meta->bcsr_bcol_idx &&
        meta->bcsr_block_size > 0) {
      int32_t block_size = meta->bcsr_block_size;
      int32_t block = (int32_t)(peer / block_size);
      int32_t local_row = (int32_t)(peer % block_size);
      for (int64_t k = meta->bcsr_brow_ptr[block];
           k < meta->bcsr_brow_ptr[block + 1]; k += 2) {
        int32_t row = meta->bcsr_bcol_idx[k];
        if (row == local_row) {
          int32_t src = meta->bcsr_bcol_idx[k + 1];
          if (env->membership == NULL || env->membership[src])
            env->work_fn(src, (int32_t)peer, peer, env->work_env);
        } else if (row > local_row) {
          break;
        }
      }
    }
    break;
  case LAYOUT_SET:
  default: {
    RoaringBitmap *edges = (RoaringBitmap *)meta->edges_bitmap;
    EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;
    if (edges && pairs)
      for (int64_t e = 0; e < meta->static_pair_count; ++e) {
        if (!roaring_bitmap_contains(edges, (uint32_t)e))
          continue;
        int32_t u = pairs[e].u;
        int32_t v = pairs[e].v;
        if ((int64_t)u == peer && (env->membership == NULL || env->membership[v]))
          env->work_fn(v, u, peer, env->work_env);
        else if ((int64_t)v == peer &&
                 (env->membership == NULL || env->membership[u]))
          env->work_fn(u, v, peer, env->work_env);
      }
    for (int64_t e = 0; e < meta->extra_edge_count; ++e) {
      if (!meta->extra_edge_live[e])
        continue;
      int32_t u = meta->extra_edge_pairs[2 * e];
      int32_t v = meta->extra_edge_pairs[2 * e + 1];
      if ((int64_t)u == peer && (env->membership == NULL || env->membership[v]))
        env->work_fn(v, u, peer, env->work_env);
      else if ((int64_t)v == peer &&
               (env->membership == NULL || env->membership[u]))
        env->work_fn(u, v, peer, env->work_env);
    }
    break;
  }
  }
}

static void autograph_owner_partition_body(int64_t index, void *opaque) {
  AutoOwnerStepEnv *env = (AutoOwnerStepEnv *)opaque;
  int64_t n = env->n;
  int64_t begin = n * index / env->lane_count;
  int64_t end = n * (index + 1) / env->lane_count;
  for (int64_t dest = begin; dest < end; ++dest) {
    if (env->dest_seen && env->dest_seen[dest])
      continue;
    autograph_owner_scan_vertex(env, dest);
    if (env->dest_seen) {
      env->dest_seen[dest] = 1;
      int32_t head =
          atomic_fetch_add_explicit(&env->appended, 1, memory_order_relaxed);
      env->next_frontier[env->initial_next_size + head] = (int32_t)dest;
    }
  }
}

int32_t autograph_frontier_step_owner(void *graph_ptr,
                                      const int32_t *frontier,
                                      int32_t frontier_size,
                                      sgpl_frontier_pair_fn work_fn,
                                      void *work_env,
                                      const uint8_t *membership,
                                      int32_t *next_frontier,
                                      int32_t initial_next_size,
                                      int32_t *dest_seen) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !work_fn || meta->csr_n <= 0 || meta->csr_n > INT32_MAX)
    return initial_next_size;

  int32_t lane_count = sgpl_configured_worker_count();
  if (lane_count < 1)
    lane_count = 1;

  /* Directed graphs need the reverse adjacency for in-edge scans; build it on
   * demand (no-op when the struct already carries it). */
  if (meta->in_row_ptr == NULL || meta->in_col_idx == NULL)
    autograph_ensure_transpose(graph_ptr);

  AutoOwnerStepEnv env = {
      .meta = meta,
      .lane_count = lane_count,
      .work_fn = work_fn,
      .work_env = work_env,
      .membership = membership,
      .next_frontier = next_frontier,
      .initial_next_size = initial_next_size,
      .dest_seen = dest_seen,
      .appended = 0,
      .n = meta->csr_n,
  };

  int64_t start_ns = now_monotonic_ns();
  parallel_for_runtime(0, lane_count, 1, autograph_owner_partition_body, &env,
                       0, 0);
  autograph_profile_record_kernel_ns(0, now_monotonic_ns() - start_ns);

  return initial_next_size + (int32_t)atomic_load_explicit(&env.appended,
                                                           memory_order_relaxed);
}

/* ── Graptor CleanCut partitions (owner-computes rule) ─────────────
 *
 * Home partition of destination d: p = d * P / n (contiguous ranges).
 * Every CSR column entry (u, v) is assigned to the partition of v; per
 * partition we store a source-grouped CSR so a worker can scan exactly the
 * edges whose destination it owns.  This guarantees:
 *     - pull:   worker p scans dests [start[p], start[p+1]) sequentially
 *     - push:   worker p scans its source-grouped edge list sequentially
 * and in both cases exactly one worker ever writes to a given destination.
 */

int32_t autograph_home_partition_of(void *graph_ptr, int32_t destination) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || meta->partition_count <= 0 || meta->csr_n <= 0 ||
      destination < 0 || destination >= meta->csr_n)
    return -1;
  return (int32_t)((int64_t)destination * meta->partition_count / meta->csr_n);
}

void autograph_debug_dump_clean_cut(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || meta->partition_count <= 0) {
    fprintf(stderr, "[clean-cut] not built\n");
    return;
  }
  for (int32_t p = 0; p < meta->partition_count; ++p) {
    fprintf(stderr, "[clean-cut] P%d rows=%lld rp=[",
            p, (long long)meta->push_row_count[p]);
    int64_t rows = meta->push_row_count[p];
    for (int64_t r = 0; r <= rows && r < 16; ++r)
      fprintf(stderr, "%lld ", (long long)meta->push_rp[p][r]);
    fprintf(stderr, "] indir=[");
    for (int64_t r = 0; r < rows && r < 12; ++r)
      fprintf(stderr, "%d ", meta->push_indir[p][r]);
    fprintf(stderr, "] ci=[");
    int64_t total = meta->push_rp[p][rows];
    for (int64_t e = 0; e < total && e < 16; ++e)
      fprintf(stderr, "%d ", meta->push_ci[p][e]);
    fprintf(stderr, "]\n");
  }
}

static void autograph_clean_cut_free(AutoGraphMeta *meta) {
  if (!meta)
    return;
  for (int32_t p = 0; p < meta->partition_count; ++p) {
    free(meta->push_rp ? meta->push_rp[p] : NULL);
    free(meta->push_ci ? meta->push_ci[p] : NULL);
    free(meta->push_indir ? meta->push_indir[p] : NULL);
  }
  free(meta->push_rp);
  free(meta->push_ci);
  free(meta->push_indir);
  free(meta->push_row_count);
  free(meta->partition_start);
  meta->push_rp = NULL;
  meta->push_ci = NULL;
  meta->push_indir = NULL;
  meta->push_row_count = NULL;
  meta->partition_start = NULL;
  meta->partition_count = 0;
}

int32_t autograph_build_clean_cut(void *graph_ptr, int32_t partitions) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta)
    return 0;
  if (!meta->csr_row_ptr || !meta->csr_col_idx || meta->csr_n <= 0)
    return 0;
  if (meta->csr_n > INT32_MAX)
    return 0;

  int32_t workers = sgpl_configured_worker_count();
  if (workers < 1)
    workers = 1;
  if (partitions <= 0) {
    const char *env = getenv("SGPL_CLEANCUT_PARTITIONS");
    partitions = env ? (int32_t)strtol(env, NULL, 10) : workers * 4;
    if (partitions < workers)
      partitions = workers;
  }
  int64_t n = meta->csr_n;
  if ((int64_t)partitions > n)
    partitions = (int32_t)n;
  if (partitions < 1)
    return 0;

  autograph_clean_cut_free(meta);

  int64_t *start =
      (int64_t *)calloc((size_t)(partitions + 1), sizeof(int64_t));
  int64_t **rp = (int64_t **)calloc((size_t)partitions, sizeof(int64_t *));
  int32_t **ci = (int32_t **)calloc((size_t)partitions, sizeof(int32_t *));
  int32_t **indir = (int32_t **)calloc((size_t)partitions, sizeof(int32_t *));
  int64_t *row_counts = (int64_t *)calloc((size_t)partitions, sizeof(int64_t));
  int64_t *edge_counts = (int64_t *)calloc((size_t)partitions, sizeof(int64_t));
  if (!start || !rp || !ci || !indir || !row_counts || !edge_counts)
    goto fail;

  for (int32_t p = 0; p <= partitions; ++p)
    start[p] = (int64_t)p * n / partitions;

  /* Helper: partition of a destination. */
#define CC_PART_OF(v) (int32_t)((int64_t)(v) * partitions / n)

  /* Pass 1: count unique source rows + edges per partition.  Sources are
   * scanned ascending, so each partition's indir rows are sorted. */
  int32_t *last_row = (int32_t *)calloc((size_t)partitions, sizeof(int32_t));
  if (!last_row)
    goto fail;
  for (int32_t p = 0; p < partitions; ++p)
    last_row[p] = -1;
  for (int64_t u = 0; u < n; ++u) {
    int64_t lo = meta->csr_row_ptr[u];
    int64_t hi = meta->csr_row_ptr[u + 1];
    for (int64_t j = lo; j < hi; ++j) {
      int32_t v = meta->csr_col_idx[j];
      int32_t p = CC_PART_OF(v);
      if (p < 0 || p >= partitions)
        continue;
      if ((int64_t)last_row[p] != u) {
        last_row[p] = (int32_t)u;
        row_counts[p]++;
      }
      edge_counts[p]++;
    }
  }
  free(last_row);

  for (int32_t p = 0; p < partitions; ++p) {
    rp[p] = (int64_t *)calloc((size_t)(row_counts[p] + 1), sizeof(int64_t));
    indir[p] = (int32_t *)calloc((size_t)row_counts[p], sizeof(int32_t));
    ci[p] = (int32_t *)calloc((size_t)edge_counts[p], sizeof(int32_t));
    if (!rp[p] || !indir[p] || !ci[p])
      goto fail;
  }

  /* Pass 2: fill indir / ci / rp with one ascending source scan. */
  int32_t *seen = (int32_t *)calloc((size_t)partitions, sizeof(int32_t));
  if (!seen)
    goto fail;
  for (int32_t p = 0; p < partitions; ++p)
    seen[p] = -1;
  int64_t *row_idx = (int64_t *)calloc((size_t)partitions, sizeof(int64_t));
  int64_t *edge_idx = (int64_t *)calloc((size_t)partitions, sizeof(int64_t));
  if (!row_idx || !edge_idx)
    goto fail;

  for (int64_t u = 0; u < n; ++u) {
    int64_t lo = meta->csr_row_ptr[u];
    int64_t hi = meta->csr_row_ptr[u + 1];
    for (int64_t j = lo; j < hi; ++j) {
      int32_t v = meta->csr_col_idx[j];
      int32_t p = CC_PART_OF(v);
      if (p < 0 || p >= partitions)
        continue;
      if (seen[p] != (int32_t)u) {
        /* new source row for partition p: open it in indir and rp */
        seen[p] = (int32_t)u;
        if (row_idx[p] < row_counts[p]) {
          indir[p][row_idx[p]] = (int32_t)u;
          rp[p][row_idx[p]] = (int64_t)edge_idx[p];
          row_idx[p]++;
        }
      }
      if (edge_idx[p] < edge_counts[p]) {
        ci[p][edge_idx[p]++] = v;
      }
    }
  }
  for (int32_t p = 0; p < partitions; ++p)
    rp[p][row_counts[p]] = edge_counts[p];

  free(seen);
  free(row_idx);
  free(edge_idx);
  free(edge_counts);

  meta->partition_start = start;
  meta->push_rp = rp;
  meta->push_ci = ci;
  meta->push_indir = indir;
  meta->push_row_count = row_counts;
  meta->partition_count = partitions;
  return partitions;

fail:
  if (start) free(start);
  if (rp)
    for (int32_t p = 0; p < partitions; ++p) free(rp[p]);
  free(rp);
  if (ci)
    for (int32_t p = 0; p < partitions; ++p) free(ci[p]);
  free(ci);
  if (indir)
    for (int32_t p = 0; p < partitions; ++p) free(indir[p]);
  free(indir);
  if (row_counts) free(row_counts);
  if (edge_counts) free(edge_counts);
  return 0;
}

typedef struct {
  AutoGraphMeta *meta;
  const int32_t *frontier;
  int32_t frontier_size;
  sgpl_frontier_pair_fn work_fn;
  void *work_env;
  const uint8_t *membership;
  int32_t *next_frontier;
  int32_t initial_next_size;
  int32_t *dest_seen;
  _Atomic int32_t appended;
  int32_t partitions;
} AutoOwnerPushEnv;

static void autograph_owner_push_partition_body(int64_t index, void *opaque) {
  AutoOwnerPushEnv *env = (AutoOwnerPushEnv *)opaque;
  int32_t p = (int32_t)index;
  AutoGraphMeta *meta = env->meta;
  int64_t rows = meta->push_row_count[p];
  int64_t *rp = meta->push_rp[p];
  int32_t *ci = meta->push_ci[p];
  int32_t *indir = meta->push_indir[p];
  for (int64_t r = 0; r < rows; ++r) {
    int32_t u = indir[r];
    if (env->membership && !env->membership[u])
      continue;
    for (int64_t j = rp[r]; j < rp[r + 1]; ++j) {
      int32_t v = ci[j];
      int32_t seen_before = env->dest_seen ? env->dest_seen[v] : 1;
      env->work_fn(u, v, v, env->work_env);
      /* Append only if work_fn requested it by storing 1 into dest_seen[v]
       * (elided DSL next.add / next_frontier[next_size++] = v). */
      if (env->dest_seen && env->next_frontier && !seen_before &&
          env->dest_seen[v]) {
        int32_t head =
            atomic_fetch_add_explicit(&env->appended, 1, memory_order_relaxed);
        env->next_frontier[env->initial_next_size + head] = v;
      }
    }
  }
}

int32_t autograph_frontier_step_owner_push(void *graph_ptr,
                                           const int32_t *frontier,
                                           int32_t frontier_size,
                                           sgpl_frontier_pair_fn work_fn,
                                           void *work_env,
                                           const uint8_t *membership,
                                           int32_t *next_frontier,
                                           int32_t initial_next_size,
                                           int32_t *dest_seen) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !work_fn || meta->partition_count <= 0)
    return initial_next_size;
  if (!next_frontier && dest_seen)
    return initial_next_size;

  AutoOwnerPushEnv env = {
      .meta = meta,
      .frontier = frontier,
      .frontier_size = frontier_size,
      .work_fn = work_fn,
      .work_env = work_env,
      .membership = membership,
      .next_frontier = next_frontier,
      .initial_next_size = initial_next_size,
      .dest_seen = dest_seen,
      .appended = 0,
      .partitions = meta->partition_count,
  };

  int64_t start_ns = now_monotonic_ns();
  parallel_for_runtime(0, meta->partition_count, 1,
                       autograph_owner_push_partition_body, &env, 0, 0);
  autograph_profile_record_kernel_ns(0, now_monotonic_ns() - start_ns);

  return initial_next_size + (int32_t)atomic_load_explicit(&env.appended,
                                                           memory_order_relaxed);
}

/* Source-owned (push-on-owner) traversal: partition p owns a contiguous
 * SOURCE range [p*n/P, (p+1)*n/P) and scans each source's own CSR row, so
 * writes indexed by the SOURCE (e.g. out_degree[u]++, u-counted state) are
 * race-free without atomics.  Zero-copy (no per-partition copies/buffers);
 * sources are visited ascending per partition, and partitions are disjoint,
 * so the work function sees exactly the serial edge order.
 */
typedef struct {
  AutoGraphMeta *meta;
  sgpl_frontier_pair_fn work_fn;
  void *work_env;
  const uint8_t *membership;
  int32_t *next_frontier;
  int32_t initial_next_size;
  int32_t *dest_seen;
  _Atomic int32_t appended;
  int32_t partitions;
} AutoSourceOwnerEnv;

static void autograph_source_owner_partition_body(int64_t index, void *opaque) {
  AutoSourceOwnerEnv *env = (AutoSourceOwnerEnv *)opaque;
  AutoGraphMeta *meta = env->meta;
  int32_t p = (int32_t)index;
  int64_t n = meta->csr_n;
  int64_t lo = p * n / env->partitions;
  int64_t hi = (p + 1) * n / env->partitions;
  int64_t *rp = meta->csr_row_ptr;
  int32_t *ci = meta->csr_col_idx;
  for (int64_t u = lo; u < hi; ++u) {
    if (env->membership && !env->membership[u])
      continue;
    for (int64_t j = rp[u]; j < rp[u + 1]; ++j) {
      int32_t v = ci[j];
      int32_t seen_before = env->dest_seen ? env->dest_seen[v] : 1;
      env->work_fn((int32_t)u, v, v, env->work_env);
      if (env->dest_seen && env->next_frontier && !seen_before &&
          env->dest_seen[v]) {
        int32_t head =
            atomic_fetch_add_explicit(&env->appended, 1, memory_order_relaxed);
        env->next_frontier[env->initial_next_size + head] = v;
      }
    }
  }
}

int32_t autograph_frontier_step_owner_source(void *graph_ptr,
                                             const int32_t *frontier,
                                             int32_t frontier_size,
                                             sgpl_frontier_pair_fn work_fn,
                                             void *work_env,
                                             const uint8_t *membership,
                                             int32_t *next_frontier,
                                             int32_t initial_next_size,
                                             int32_t *dest_seen) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !work_fn || meta->partition_count <= 0)
    return initial_next_size;
  if (!next_frontier && dest_seen)
    return initial_next_size;

  AutoSourceOwnerEnv env = {
      .meta = meta,
      .work_fn = work_fn,
      .work_env = work_env,
      .membership = membership,
      .next_frontier = next_frontier,
      .initial_next_size = initial_next_size,
      .dest_seen = dest_seen,
      .appended = 0,
      .partitions = meta->partition_count,
  };

  int64_t start_ns = now_monotonic_ns();
  parallel_for_runtime(0, meta->partition_count, 1,
                       autograph_source_owner_partition_body, &env, 0, 0);
  autograph_profile_record_kernel_ns(0, now_monotonic_ns() - start_ns);

  return initial_next_size + (int32_t)atomic_load_explicit(&env.appended,
                                                           memory_order_relaxed);
}

/* Per-partition partial reduction step (owner-computes, destination-owned):
 * each partition accumulates its pair work into its own partial at
 * work_env + p * partial_bytes; once all partitions finish, the combine
 * function folds every partial into `out` in ascending partition order
 * (deterministic combine order; each partial is summed in CSR order). */
typedef void (*sgpl_frontier_combine_fn)(const void *partial, void *out);

typedef struct {
  AutoGraphMeta *meta;
  sgpl_frontier_pair_fn work_fn;
  void *work_env;            /* partials base */
  int64_t partial_bytes;
  const uint8_t *membership;
} AutoRedEnv;

static void autograph_owner_red_partition_body(int64_t index, void *opaque) {
  AutoRedEnv *env = (AutoRedEnv *)opaque;
  AutoGraphMeta *meta = env->meta;
  int32_t p = (int32_t)index;
  int64_t rows = meta->push_row_count[p];
  int64_t *rp = meta->push_rp[p];
  int32_t *ci = meta->push_ci[p];
  int32_t *indir = meta->push_indir[p];
  char *partial = (char *)env->work_env + (int64_t)p * env->partial_bytes;
  for (int64_t r = 0; r < rows; ++r) {
    int32_t u = indir[r];
    if (env->membership && !env->membership[u])
      continue;
    for (int64_t j = rp[r]; j < rp[r + 1]; ++j)
      env->work_fn(u, ci[j], ci[j], partial);
  }
}

int32_t autograph_frontier_step_owner_red(void *graph_ptr,
                                          const int32_t *frontier,
                                          int32_t frontier_size,
                                          sgpl_frontier_pair_fn work_fn,
                                          void *work_env,
                                          int64_t partial_bytes,
                                          sgpl_frontier_combine_fn combine_fn,
                                          void *out,
                                          const uint8_t *membership,
                                          int32_t *next_frontier,
                                          int32_t initial_next_size,
                                          int32_t *dest_seen) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !work_fn || !combine_fn || meta->partition_count <= 0)
    return initial_next_size;

  AutoRedEnv env = {
      .meta = meta,
      .work_fn = work_fn,
      .work_env = work_env,
      .partial_bytes = partial_bytes,
      .membership = membership,
  };

  int64_t start_ns = now_monotonic_ns();
  parallel_for_runtime(0, meta->partition_count, 1,
                       autograph_owner_red_partition_body, &env, 0, 0);
  for (int32_t p = 0; p < meta->partition_count; ++p)
    combine_fn((char *)work_env + (int64_t)p * partial_bytes, out);
  autograph_profile_record_kernel_ns(0, now_monotonic_ns() - start_ns);
  return initial_next_size;
}


typedef struct {
  AutoGraphMeta *meta;
  const int32_t *frontier;
  AutoFrontierLane *lanes;
  int32_t lane_count;
  int32_t *labels;
  int32_t *alive;
  int32_t *deg;
  int32_t k;
  int32_t weighted; /* 1 => RelaxMin uses prop[u] + csr_weights */
  const uint8_t *frontier_membership;
  uint8_t *round_member;
} AutoMotifFrontierEnv;

static int autograph_atomic_min_update(int32_t *cell, int32_t value) {
  _Atomic int32_t *atomic_cell = (_Atomic int32_t *)cell;
  int32_t cur = atomic_load_explicit(atomic_cell, memory_order_relaxed);
  while (value < cur) {
    if (atomic_compare_exchange_weak_explicit(
            atomic_cell, &cur, value, memory_order_relaxed,
            memory_order_relaxed))
      return 1;
  }
  return 0;
}

static void autograph_motif_lane_append(AutoMotifFrontierEnv *env,
                                        int32_t vertex) {
  int32_t lane_index = sgpl_current_worker_index();
  if (lane_index < 0 || lane_index >= env->lane_count)
    lane_index = 0;
  AutoFrontierLane *lane = &env->lanes[lane_index];
  if (lane->size == lane->capacity) {
    int64_t grown_capacity = lane->capacity ? (int64_t)lane->capacity * 2 : 64;
    if (grown_capacity > INT32_MAX ||
        (uint64_t)grown_capacity > SIZE_MAX / sizeof(int32_t))
      abort();
    int32_t new_capacity = (int32_t)grown_capacity;
    int32_t *new_data =
        (int32_t *)realloc(lane->data, (size_t)new_capacity * sizeof(int32_t));
    if (!new_data)
      abort();
    lane->data = new_data;
    lane->capacity = new_capacity;
  }
  lane->data[lane->size++] = vertex;
}

static void autograph_motif_activate_once(AutoMotifFrontierEnv *env,
                                          int32_t vertex) {
  if (vertex < 0 || vertex >= env->meta->csr_n)
    return;
  if (env->round_member) {
    uint8_t expected = 0;
    if (!atomic_compare_exchange_strong_explicit(
            (_Atomic uint8_t *)&env->round_member[vertex], &expected, 1,
            memory_order_relaxed, memory_order_relaxed))
      return;
  }
  autograph_motif_lane_append(env, vertex);
}

static void autograph_motif_foreach_neighbor(
    AutoGraphMeta *meta, int32_t source,
    void (*fn)(AutoMotifFrontierEnv *, int32_t, int32_t),
    AutoMotifFrontierEnv *env) {
  if (source < 0 || source >= meta->csr_n)
    return;
  switch (meta->current_layout) {
  case LAYOUT_CSR:
    if (meta->csr_row_ptr && meta->csr_col_idx)
      for (int64_t j = meta->csr_row_ptr[source];
           j < meta->csr_row_ptr[source + 1]; ++j)
        fn(env, source, meta->csr_col_idx[j]);
    break;
  case LAYOUT_PCSR:
    if (meta->pcsr_row_ptr && meta->pcsr_col_idx)
      for (int64_t j = meta->pcsr_row_ptr[source];
           j < meta->pcsr_row_ptr[source + 1]; ++j) {
        int32_t destination = meta->pcsr_col_idx[j];
        if (destination != -1)
          fn(env, source, destination);
      }
    break;
  case LAYOUT_BCSR:
    if (meta->bcsr_brow_ptr && meta->bcsr_bcol_idx &&
        meta->bcsr_block_size > 0) {
      int32_t block_size = meta->bcsr_block_size;
      int32_t block = source / block_size;
      int32_t local_row = source % block_size;
      for (int32_t k = meta->bcsr_brow_ptr[block];
           k < meta->bcsr_brow_ptr[block + 1]; k += 2) {
        int32_t row = meta->bcsr_bcol_idx[k];
        if (row == local_row)
          fn(env, source, meta->bcsr_bcol_idx[k + 1]);
        else if (row > local_row)
          break;
      }
    }
    break;
  case LAYOUT_SET:
  default: {
    RoaringBitmap *edges = (RoaringBitmap *)meta->edges_bitmap;
    EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;
    if (edges && pairs)
      for (int64_t e = 0; e < meta->static_pair_count; ++e) {
        if (!roaring_bitmap_contains(edges, (uint32_t)e))
          continue;
        if (pairs[e].u == source)
          fn(env, source, pairs[e].v);
        else if (pairs[e].v == source)
          fn(env, source, pairs[e].u);
      }
    for (int64_t e = 0; e < meta->extra_edge_count; ++e) {
      if (!meta->extra_edge_live[e])
        continue;
      int32_t u = meta->extra_edge_pairs[2 * e];
      int32_t v = meta->extra_edge_pairs[2 * e + 1];
      if (u == source)
        fn(env, source, v);
      else if (v == source)
        fn(env, source, u);
    }
    break;
  }
  }
}

static void autograph_motif_relax_min_edge(AutoMotifFrontierEnv *env,
                                           int32_t source, int32_t destination,
                                           int32_t edge_weight) {
  if (destination < 0 || destination >= env->meta->csr_n)
    return;
  int32_t cand = env->labels[source];
  if (env->weighted) {
    if (cand > INT32_MAX - edge_weight)
      return;
    cand += edge_weight;
  }
  if (autograph_atomic_min_update(&env->labels[destination], cand))
    autograph_motif_activate_once(env, destination);
}

/* CSR-aware walk so weighted RelaxMin can read csr_weights[j]. */
static void autograph_motif_relax_min_foreach_csr(AutoMotifFrontierEnv *env,
                                                 int32_t source) {
  AutoGraphMeta *meta = env->meta;
  if (source < 0 || source >= meta->csr_n || !meta->csr_row_ptr ||
      !meta->csr_col_idx)
    return;
  for (int64_t j = meta->csr_row_ptr[source]; j < meta->csr_row_ptr[source + 1];
       ++j) {
    int32_t w = 1;
    if (env->weighted && meta->csr_weights)
      w = meta->csr_weights[j];
    autograph_motif_relax_min_edge(env, source, meta->csr_col_idx[j], w);
  }
}

static void autograph_motif_write_min_edge(AutoMotifFrontierEnv *env,
                                           int32_t source,
                                           int32_t destination) {
  autograph_motif_relax_min_edge(env, source, destination, 1);
}

static void autograph_motif_write_min_push_body(int64_t index, void *opaque) {
  AutoMotifFrontierEnv *env = (AutoMotifFrontierEnv *)opaque;
  int32_t source = env->frontier[index];
  if (env->weighted && env->meta->current_layout == LAYOUT_CSR) {
    autograph_motif_relax_min_foreach_csr(env, source);
    return;
  }
  autograph_motif_foreach_neighbor(env->meta, source,
                                   autograph_motif_write_min_edge, env);
}

static int32_t autograph_motif_min_frontier_candidate(AutoMotifFrontierEnv *env,
                                                     int32_t destination) {
  AutoGraphMeta *meta = env->meta;
  int32_t best = INT32_MAX;
#define AUTOGRAPH_MOTIF_CONSIDER_W(candidate, weight)                            \
  do {                                                                            \
    int32_t autograph_source_ = (candidate);                                       \
    if (autograph_source_ >= 0 && autograph_source_ < meta->csr_n &&               \
        env->frontier_membership[autograph_source_]) {                             \
      int32_t cand = env->labels[autograph_source_];                               \
      if (env->weighted) {                                                         \
        int32_t w_ = (weight);                                                     \
        if (cand > INT32_MAX - w_)                                                 \
          break;                                                                   \
        cand += w_;                                                                \
      }                                                                           \
      if (cand < best)                                                             \
        best = cand;                                                               \
    }                                                                             \
  } while (0)

  if (env->weighted) {
    /* Weighted pull is CSR-only (weights aligned with col_idx). */
    if (meta->current_layout == LAYOUT_CSR && meta->csr_row_ptr &&
        meta->csr_col_idx) {
      for (int64_t j = meta->csr_row_ptr[destination];
           j < meta->csr_row_ptr[destination + 1]; ++j) {
        int32_t w = meta->csr_weights ? meta->csr_weights[j] : 1;
        AUTOGRAPH_MOTIF_CONSIDER_W(meta->csr_col_idx[j], w);
      }
    }
  } else {
    switch (meta->current_layout) {
    case LAYOUT_CSR:
      if (meta->csr_row_ptr && meta->csr_col_idx)
        for (int64_t j = meta->csr_row_ptr[destination];
             j < meta->csr_row_ptr[destination + 1]; ++j)
          AUTOGRAPH_MOTIF_CONSIDER_W(meta->csr_col_idx[j], 1);
      break;
    case LAYOUT_PCSR:
      if (meta->pcsr_row_ptr && meta->pcsr_col_idx)
        for (int64_t j = meta->pcsr_row_ptr[destination];
             j < meta->pcsr_row_ptr[destination + 1]; ++j)
          if (meta->pcsr_col_idx[j] != -1)
            AUTOGRAPH_MOTIF_CONSIDER_W(meta->pcsr_col_idx[j], 1);
      break;
    case LAYOUT_BCSR:
      if (meta->bcsr_brow_ptr && meta->bcsr_bcol_idx &&
          meta->bcsr_block_size > 0) {
        int32_t block_size = meta->bcsr_block_size;
        int32_t block = destination / block_size;
        int32_t local_row = destination % block_size;
        for (int32_t k = meta->bcsr_brow_ptr[block];
             k < meta->bcsr_brow_ptr[block + 1]; k += 2) {
          int32_t row = meta->bcsr_bcol_idx[k];
          if (row == local_row)
            AUTOGRAPH_MOTIF_CONSIDER_W(meta->bcsr_bcol_idx[k + 1], 1);
          else if (row > local_row)
            break;
        }
      }
      break;
    default:
      break;
    }
  }
#undef AUTOGRAPH_MOTIF_CONSIDER_W
  return best;
}

static void autograph_motif_write_min_pull_body(int64_t index, void *opaque) {
  AutoMotifFrontierEnv *env = (AutoMotifFrontierEnv *)opaque;
  int64_t vertex_count = env->meta->csr_n;
  int64_t begin = vertex_count * index / env->lane_count;
  int64_t end = vertex_count * (index + 1) / env->lane_count;
  for (int64_t destination = begin; destination < end; ++destination) {
    int32_t best =
        autograph_motif_min_frontier_candidate(env, (int32_t)destination);
    if (best == INT32_MAX)
      continue;
    if (best < env->labels[destination]) {
      env->labels[destination] = best;
      autograph_motif_activate_once(env, (int32_t)destination);
    }
  }
}

static void autograph_motif_peel_edge(AutoMotifFrontierEnv *env, int32_t source,
                                      int32_t destination) {
  (void)source;
  if (destination < 0 || destination >= env->meta->csr_n)
    return;
  if (env->alive[destination] == 0)
    return;
  int32_t old = atomic_fetch_sub_explicit((_Atomic int32_t *)&env->deg[destination],
                                          1, memory_order_relaxed);
  int32_t nd = old - 1;
  if (nd < env->k)
    autograph_motif_activate_once(env, destination);
}

static void autograph_motif_peel_push_body(int64_t index, void *opaque) {
  AutoMotifFrontierEnv *env = (AutoMotifFrontierEnv *)opaque;
  int32_t u = env->frontier[index];
  if (u < 0 || u >= env->meta->csr_n)
    return;
  int32_t expected = 1;
  if (!atomic_compare_exchange_strong_explicit(
          (_Atomic int32_t *)&env->alive[u], &expected, 0, memory_order_relaxed,
          memory_order_relaxed))
    return;
  autograph_motif_foreach_neighbor(env->meta, u, autograph_motif_peel_edge,
                                   env);
}

/* Phase 1 of peel pull: CAS-remove frontier vertices into membership bitmap. */
static void autograph_motif_peel_mark_removed_body(int64_t index,
                                                   void *opaque) {
  AutoMotifFrontierEnv *env = (AutoMotifFrontierEnv *)opaque;
  int32_t u = env->frontier[index];
  if (u < 0 || u >= env->meta->csr_n)
    return;
  int32_t expected = 1;
  if (!atomic_compare_exchange_strong_explicit(
          (_Atomic int32_t *)&env->alive[u], &expected, 0, memory_order_relaxed,
          memory_order_relaxed))
    return;
  /* Peel pull stores the removed-this-round set in scratch_membership. */
  env->meta->scratch_membership[u] = 1;
}

static int32_t autograph_motif_count_removed_neighbors(
    const AutoMotifFrontierEnv *env, int32_t destination) {
  AutoGraphMeta *meta = env->meta;
  int32_t count = 0;
#define AUTOGRAPH_MOTIF_COUNT_REMOVED(candidate)                                 \
  do {                                                                            \
    int32_t nbr_ = (candidate);                                                    \
    if (nbr_ >= 0 && nbr_ < meta->csr_n &&                                         \
        env->frontier_membership[nbr_])                                            \
      ++count;                                                                     \
  } while (0)

  switch (meta->current_layout) {
  case LAYOUT_CSR:
    if (meta->csr_row_ptr && meta->csr_col_idx)
      for (int64_t j = meta->csr_row_ptr[destination];
           j < meta->csr_row_ptr[destination + 1]; ++j)
        AUTOGRAPH_MOTIF_COUNT_REMOVED(meta->csr_col_idx[j]);
    break;
  case LAYOUT_PCSR:
    if (meta->pcsr_row_ptr && meta->pcsr_col_idx)
      for (int64_t j = meta->pcsr_row_ptr[destination];
           j < meta->pcsr_row_ptr[destination + 1]; ++j)
        if (meta->pcsr_col_idx[j] != -1)
          AUTOGRAPH_MOTIF_COUNT_REMOVED(meta->pcsr_col_idx[j]);
    break;
  case LAYOUT_BCSR:
    if (meta->bcsr_brow_ptr && meta->bcsr_bcol_idx &&
        meta->bcsr_block_size > 0) {
      int32_t block_size = meta->bcsr_block_size;
      int32_t block = destination / block_size;
      int32_t local_row = destination % block_size;
      for (int32_t k = meta->bcsr_brow_ptr[block];
           k < meta->bcsr_brow_ptr[block + 1]; k += 2) {
        int32_t row = meta->bcsr_bcol_idx[k];
        if (row == local_row)
          AUTOGRAPH_MOTIF_COUNT_REMOVED(meta->bcsr_bcol_idx[k + 1]);
        else if (row > local_row)
          break;
      }
    }
    break;
  default:
    break;
  }
#undef AUTOGRAPH_MOTIF_COUNT_REMOVED
  return count;
}

/* Phase 2: owned destinations subtract removed-neighbor counts and activate. */
static void autograph_motif_peel_pull_partition_body(int64_t index,
                                                    void *opaque) {
  AutoMotifFrontierEnv *env = (AutoMotifFrontierEnv *)opaque;
  int64_t vertex_count = env->meta->csr_n;
  int64_t begin = vertex_count * index / env->lane_count;
  int64_t end = vertex_count * (index + 1) / env->lane_count;
  for (int64_t destination = begin; destination < end; ++destination) {
    if (env->alive[destination] == 0)
      continue;
    int32_t removed =
        autograph_motif_count_removed_neighbors(env, (int32_t)destination);
    if (removed <= 0)
      continue;
    int32_t nd = env->deg[destination] - removed;
    env->deg[destination] = nd;
    if (nd < env->k)
      autograph_motif_activate_once(env, (int32_t)destination);
  }
}

static int32_t autograph_edgemap_motif(void *graph_ptr, int32_t mode,
                                       const int32_t *frontier,
                                       int32_t frontier_size,
                                       int32_t *next_frontier,
                                       int32_t initial_next_size,
                                       int32_t *prop0, int32_t *prop1,
                                       int32_t scalar) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !frontier || !next_frontier || !prop0 || frontier_size <= 0 ||
      meta->csr_n <= 0 || meta->csr_n > INT32_MAX ||
      initial_next_size < 0 || initial_next_size > meta->csr_n)
    return initial_next_size;
  if (mode != SGPL_MOTIF_WRITE_MIN && mode != SGPL_MOTIF_PEEL_K &&
      mode != SGPL_MOTIF_RELAX_MIN_WEIGHTED)
    return initial_next_size;
  if (mode == SGPL_MOTIF_PEEL_K && (!prop1 || scalar < 0))
    return initial_next_size;
  if (mode == SGPL_MOTIF_RELAX_MIN_WEIGHTED &&
      meta->current_layout != LAYOUT_CSR)
    return initial_next_size;

  int32_t lane_count = sgpl_configured_worker_count();
  if (lane_count < 1)
    lane_count = 1;
  if (!autograph_scratch_ensure(meta, lane_count))
    return initial_next_size;
  autograph_scratch_reset_lanes(meta, lane_count);
  autograph_scratch_clear_round_member(meta);

  int relax_min = mode == SGPL_MOTIF_WRITE_MIN ||
                  mode == SGPL_MOTIF_RELAX_MIN_WEIGHTED;
  AutoFrontierLane *lanes = (AutoFrontierLane *)meta->scratch_lanes;
  AutoMotifFrontierEnv env = {
      .meta = meta,
      .frontier = frontier,
      .lanes = lanes,
      .lane_count = lane_count,
      .labels = relax_min ? prop0 : NULL,
      .alive = mode == SGPL_MOTIF_PEEL_K ? prop0 : NULL,
      .deg = mode == SGPL_MOTIF_PEEL_K ? prop1 : NULL,
      .k = scalar,
      .weighted = mode == SGPL_MOTIF_RELAX_MIN_WEIGHTED ? 1 : 0,
      .frontier_membership = NULL,
      .round_member = meta->scratch_round_member,
  };

  int use_pull = autograph_should_use_pull(meta, frontier, frontier_size);
  /* Weighted pull needs CSR weights; stay on push if weights missing. */
  if (env.weighted && !meta->csr_weights)
    use_pull = 0;

  if (relax_min) {
    if (use_pull) {
      autograph_fill_frontier_membership(meta, frontier, frontier_size);
      env.frontier_membership = meta->scratch_membership;
      parallel_for_runtime(0, lane_count, 1, autograph_motif_write_min_pull_body,
                           &env, 0, 0);
    } else {
      parallel_for_runtime(0, frontier_size, 1,
                           autograph_motif_write_min_push_body, &env, 0, 0);
    }
  } else if (use_pull) {
    /* PeelK owner-computes pull: mark removals in membership, then owned
     * deg updates. round_member stays reserved for activate-once. */
    autograph_scratch_clear_membership(meta);
    env.frontier_membership = meta->scratch_membership;
    parallel_for_runtime(0, frontier_size, 1,
                         autograph_motif_peel_mark_removed_body, &env, 0, 0);
    parallel_for_runtime(0, lane_count, 1,
                         autograph_motif_peel_pull_partition_body, &env, 0, 0);
  } else {
    parallel_for_runtime(0, frontier_size, 1, autograph_motif_peel_push_body,
                         &env, 0, 0);
  }

  return autograph_scratch_merge_lanes(meta, lane_count, next_frontier,
                                       initial_next_size);
}

int32_t autograph_edgemap(void *graph_ptr,
                          int32_t combine,
                          const int32_t *frontier,
                          int32_t frontier_size,
                          int32_t *next_frontier,
                          int32_t initial_next_size,
                          int32_t *prop0,
                          int32_t *prop1,
                          int32_t scalar0,
                          int32_t scalar1) {
  uint64_t start_ns = now_monotonic_ns();
  int32_t result = initial_next_size;
  switch (combine) {
  case SGPL_COMBINE_CAS_FIRST:
    result = autograph_edgemap_cas_first(graph_ptr, frontier, frontier_size,
                                         next_frontier, initial_next_size,
                                         prop0, prop1, scalar0, scalar1);
    break;
  case SGPL_COMBINE_MIN_COPY:
    result = autograph_edgemap_motif(graph_ptr, SGPL_MOTIF_WRITE_MIN, frontier,
                                     frontier_size, next_frontier,
                                     initial_next_size, prop0, prop1, 0);
    break;
  case SGPL_COMBINE_MIN_WEIGHTED:
    result = autograph_edgemap_motif(graph_ptr, SGPL_MOTIF_RELAX_MIN_WEIGHTED,
                                     frontier, frontier_size, next_frontier,
                                     initial_next_size, prop0, prop1, 0);
    break;
  case SGPL_COMBINE_PEEL_K:
    result = autograph_edgemap_motif(graph_ptr, SGPL_MOTIF_PEEL_K, frontier,
                                     frontier_size, next_frontier,
                                     initial_next_size, prop0, prop1, scalar0);
    break;
  default:
    break;
  }
  autograph_profile_record_kernel_ns(0, now_monotonic_ns() - start_ns);
  return result;
}

int32_t autograph_motif_frontier_step(void *graph_ptr, int32_t mode,
                                      const int32_t *frontier,
                                      int32_t frontier_size,
                                      int32_t *next_frontier,
                                      int32_t initial_next_size,
                                      int32_t *prop0, int32_t *prop1,
                                      int32_t scalar) {
  int32_t combine = SGPL_COMBINE_MIN_COPY;
  if (mode == SGPL_MOTIF_PEEL_K)
    combine = SGPL_COMBINE_PEEL_K;
  else if (mode == SGPL_MOTIF_RELAX_MIN_WEIGHTED)
    combine = SGPL_COMBINE_MIN_WEIGHTED;
  return autograph_edgemap(graph_ptr, combine, frontier, frontier_size,
                           next_frontier, initial_next_size, prop0, prop1,
                           scalar, 0);
}

/* ══════════════════════════════════════════════════════════════════
 *  Initial Registration (Called once by compiled IR)
 * ══════════════════════════════════════════════════════════════════ */
void autograph_init(void *graph_ptr, int64_t n, int64_t m, 
                    void *nodes_bmp, void *edges_bmp, 
                    void *edge_pairs_table) 
{
    AutoGraphMeta *meta = find_or_create_meta(graph_ptr);
    if (!meta) return;

    int midx = meta_index(meta);
    if (midx >= 0 && g_static_edge_hash[midx]) {
      edge_hash_destroy(g_static_edge_hash[midx]);
      g_static_edge_hash[midx] = NULL;
    }
    if (midx >= 0 && g_extra_edge_hash[midx]) {
      edge_hash_destroy(g_extra_edge_hash[midx]);
      g_extra_edge_hash[midx] = NULL;
    }

    meta->nodes_bitmap = nodes_bmp;
    meta->edges_bitmap = edges_bmp;
    meta->edge_pairs_table = edge_pairs_table;
    meta->static_pair_count = (m > 1) ? (m / 2) : m;
    meta->extra_edge_count = 0;
    meta->extra_edge_capacity = 0;
    free(meta->extra_edge_pairs);
    free(meta->extra_edge_live);
    meta->extra_edge_pairs = NULL;
    meta->extra_edge_live = NULL;
    meta->csr_n = n;
    meta->csr_m = m;
    meta->live_edge_count = meta->static_pair_count;  /* all static edges initially live */

    char *base = (char *)graph_ptr;
    meta->csr_row_ptr = *((int64_t **)(base + 16));
    meta->csr_col_idx = *((int32_t **)(base + 24));
    meta->csr_weights = *((int32_t **)(base + 32));
    meta->in_row_ptr = *((int64_t **)(base + 48));
    meta->in_col_idx = *((int32_t **)(base + 56));
    meta->csr_owned = 0;
    meta->has_class_tiers = 0;
    meta->has_csr_class_tiers = 0;
    autograph_set_layout(meta, LAYOUT_CSR);

    /* fprintf(stderr, "[AutoTuner] Initialized Graph %p (n=%ld, m=%ld) in CSR baseline layout\n",
            graph_ptr, (long)n, (long)m); */
}

/* Build (once, O(E)) the reverse adjacency (transpose) for directed graphs so
 * pull-style owner-computes traversal can scan in-edges.  The transpose lives
 * in the Graph struct's cells at byte offsets 40 (in_row_ptr) and 48
 * (in_col_idx); this mirrors (or builds) them into the meta.  Undirected
 * graphs already have symmetric CSR — returns 1 with in_row_ptr left NULL. */
int autograph_ensure_transpose(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta)
    return 0;
  if (meta->in_row_ptr)
    return 1; /* already built/aliased */

  char *base = (char *)graph_ptr;
  int64_t *g_in_rp = *((int64_t **)(base + 48));
  int32_t *g_in_ci = *((int32_t **)(base + 56));
  if (g_in_rp && g_in_ci) {
    meta->in_row_ptr = g_in_rp;
    meta->in_col_idx = g_in_ci;
    return 1;
  }

  /* Build from the forward CSR: for each edge (u -> v), append u to in-list of
   * v.  Direct use of the forward CSR assumes no self-referential transpose is
   * present; this is the standard one-pass CSR transpose (O(E), two passes). */
  int64_t n = meta->csr_n;
  if (n <= 0 || !meta->csr_row_ptr || !meta->csr_col_idx)
    return 0;

  int64_t *irp = (int64_t *)calloc((size_t)(n + 1), sizeof(int64_t));
  if (!irp)
    return 0;
  for (int64_t u = 0; u < n; ++u) {
    for (int64_t j = meta->csr_row_ptr[u]; j < meta->csr_row_ptr[u + 1]; ++j) {
      int32_t v = meta->csr_col_idx[j];
      if (v >= 0 && v < n)
        irp[v + 1]++;
    }
  }
  for (int64_t i = 1; i <= n; ++i)
    irp[i] += irp[i - 1];
  int64_t total = irp[n];
  int32_t *ici = total > 0 ? (int32_t *)malloc((size_t)total * sizeof(int32_t))
                           : NULL;
  if (total > 0 && !ici) {
    free(irp);
    return 0;
  }
  int64_t *cursor = (int64_t *)malloc((size_t)(n + 1) * sizeof(int64_t));
  if (!cursor) {
    free(irp);
    free(ici);
    return 0;
  }
  memcpy(cursor, irp, (size_t)(n + 1) * sizeof(int64_t));
  for (int64_t u = 0; u < n; ++u) {
    for (int64_t j = meta->csr_row_ptr[u]; j < meta->csr_row_ptr[u + 1]; ++j) {
      int32_t v = meta->csr_col_idx[j];
      if (v >= 0 && v < n)
        ici[cursor[v]++] = (int32_t)u;
    }
  }
  free(cursor);

  meta->in_row_ptr = irp;
  meta->in_col_idx = ici;
  return 1;
}

void autograph_set_class_tiers(void *graph_ptr, const double *tiers) {
    AutoGraphMeta *meta = find_or_create_meta(graph_ptr);
    if (!meta)
        return;
    if (!tiers) {
        meta->has_class_tiers = 0;
        return;
    }
    for (int i = 0; i < 12; ++i)
        meta->class_tiers[i] = tiers[i];
    meta->has_class_tiers = 1;
}

void autograph_set_class_tiers_csr(void *graph_ptr, const double *tiers) {
    AutoGraphMeta *meta = find_or_create_meta(graph_ptr);
    if (!meta)
        return;
    if (!tiers) {
        meta->has_csr_class_tiers = 0;
        return;
    }
    for (int i = 0; i < 12; ++i)
        meta->csr_class_tiers[i] = tiers[i];
    meta->has_csr_class_tiers = 1;
}
