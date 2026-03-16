/*
 * autotuner_runtime.c — Phase-Aware Graph Layout AutoTuner Runtime
 *
 * Implements the Set-Based Architecture constraints.
 * LAYOUT_SET (Roaring bitamps) is the absolute base.
 * Transient adjacency layouts (CSR, PCSR, BCSR) are built on-demand
 * for specific execution phases and destroyed after use.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autotuner_runtime.h"

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

/* Forward-declared: per-graph cached hash map for static edge lookup. */
typedef struct EdgeHashEntry_s EdgeHashEntry;
typedef struct EdgeHashMap_s EdgeHashMap;
static EdgeHashMap *g_static_edge_hash[MAX_GRAPHS];  /* indexed same as g_meta */

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
  return meta;
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

static int canonical_pair_find_static(AutoGraphMeta *meta, int32_t u, int32_t v) {
  if (!meta || !meta->edge_pairs_table)
    return -1;
  EdgeHashMap *m = get_static_edge_hash(meta);
  return (int)edge_hash_find(m, u, v);
}

static int64_t canonical_pair_find_extra(AutoGraphMeta *meta, int32_t u, int32_t v) {
  if (!meta || meta->extra_edge_count <= 0)
    return -1;
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    int32_t eu = meta->extra_edge_pairs[2 * i];
    int32_t ev = meta->extra_edge_pairs[2 * i + 1];
    if ((eu == u && ev == v) || (eu == v && ev == u))
      return i;
  }
  return -1;
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
  int32_t *labels = NULL;
  if (node_count > 0) {
    labels = (int32_t *)malloc((size_t)node_count * sizeof(int32_t));
    for (int64_t i = 0; i < node_count; i++) {
      labels[i] = (int32_t)roaring_bitmap_get_at_index(nb, (uint64_t)i);
    }
    qsort(labels, (size_t)node_count, sizeof(int32_t), cmp_i32);
  }

  int map_cap = next_pow2_at_least((int)(node_count * 2 + 1));
  IdMapEntry *map = (IdMapEntry *)calloc((size_t)map_cap, sizeof(IdMapEntry));
  for (int64_t i = 0; i < node_count; i++) {
    uint32_t h = hash_u32((uint32_t)labels[i]) & (uint32_t)(map_cap - 1);
    while (map[h].used)
      h = (h + 1U) & (uint32_t)(map_cap - 1);
    map[h].used = 1;
    map[h].key = labels[i];
    map[h].value = (int32_t)i;
  }
  #define LOOKUP_DENSE(label, out_idx)                                              \
    do {                                                                             \
      out_idx = -1;                                                                  \
      if (map_cap > 0) {                                                             \
        uint32_t _h = hash_u32((uint32_t)(label)) & (uint32_t)(map_cap - 1);        \
        while (map[_h].used) {                                                       \
          if (map[_h].key == (label)) {                                              \
            out_idx = map[_h].value;                                                 \
            break;                                                                    \
          }                                                                          \
          _h = (_h + 1U) & (uint32_t)(map_cap - 1);                                  \
        }                                                                            \
      }                                                                              \
    } while (0)

  int64_t dense_n = node_count;
  int64_t *rp = (int64_t *)calloc((size_t)(dense_n + 1), sizeof(int64_t));
  int64_t edge_count = 0;

  for (int64_t e = 0; e < meta->static_pair_count; e++) {
    if (!roaring_bitmap_contains(eb, (uint32_t)e))
      continue;
    int32_t du, dv;
    LOOKUP_DENSE(pairs[e].u, du);
    LOOKUP_DENSE(pairs[e].v, dv);
    if (du >= 0 && dv >= 0) {
      rp[du + 1]++;
      edge_count++;
    }
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    if (!meta->extra_edge_live[i])
      continue;
    int32_t du, dv;
    LOOKUP_DENSE(meta->extra_edge_pairs[2 * i], du);
    LOOKUP_DENSE(meta->extra_edge_pairs[2 * i + 1], dv);
    if (du >= 0 && dv >= 0) {
      rp[du + 1]++;
      edge_count++;
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
    int32_t du, dv;
    LOOKUP_DENSE(pairs[e].u, du);
    LOOKUP_DENSE(pairs[e].v, dv);
    if (du >= 0 && dv >= 0) {
      ci[next[du]++] = dv;
    }
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    if (!meta->extra_edge_live[i])
      continue;
    int32_t du, dv;
    LOOKUP_DENSE(meta->extra_edge_pairs[2 * i], du);
    LOOKUP_DENSE(meta->extra_edge_pairs[2 * i + 1], dv);
    if (du >= 0 && dv >= 0) {
      ci[next[du]++] = dv;
    }
  }

  free(next);
  free(map);
  free(labels);
  #undef LOOKUP_DENSE

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
  RoaringBitmap *nb = (RoaringBitmap *)meta->nodes_bitmap;
  RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
  EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;

  roaring_bitmap_clear(nb);
  roaring_bitmap_clear(eb);
  for (int64_t i = 0; i < meta->extra_edge_capacity; i++)
    meta->extra_edge_live[i] = 0;
  meta->extra_edge_count = 0;

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
        }
      }
    }
  }
  edge_hash_destroy(ht);
  /* fprintf(stderr, "[AutoTuner] CSR->SET rebuilt (meta): n=%ld m=%ld extra=%ld\n",
          (long)n, (long)m, (long)meta->extra_edge_count); */
}

void autograph_update_csr_pointers(void *graph_ptr, int64_t *row_ptr, int32_t *col_idx) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (meta) {
    meta->csr_row_ptr = row_ptr;
    meta->csr_col_idx = col_idx;
  }
}

/* Option B: Convert to SET from current layout. Rebuild bitmaps from adjacency. */
void autograph_ensure_layout_set(void *graph_ptr) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->nodes_bitmap || !meta->edges_bitmap || !meta->edge_pairs_table)
    return;
  if (meta->current_layout == LAYOUT_SET)
    return;

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

  if (rp && ci)
    rebuild_sets_from_csr_meta(meta, rp, ci, nn, mm);
  if (free_rp_ci && rp) free(rp);
  if (free_rp_ci && ci) free(ci);

  meta->csr_row_ptr = NULL;
  meta->csr_col_idx = NULL;
  meta->csr_owned = 0;

  char *base = (char *)graph_ptr;
  nn = (int64_t)roaring_bitmap_get_cardinality((RoaringBitmap *)meta->nodes_bitmap);
  mm = 0;
  RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
  for (int64_t e = 0; e < meta->static_pair_count; e++) {
    if (roaring_bitmap_contains(eb, (uint32_t)e))
      mm++;
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    if (meta->extra_edge_live[i])
      mm++;
  }
  *((int64_t *)(base + 0)) = nn;
  *((int64_t *)(base + 8)) = mm;
  *((int64_t **)(base + 16)) = NULL;
  *((int32_t **)(base + 24)) = NULL;
  meta->current_layout = LAYOUT_SET;
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

    prp[u] = write_pos;
    for (int64_t j = 0; j < deg; j++) {
      pcol[write_pos + j] = csr_ci[csr_rp[u] + j];
    }
    write_pos += row_cap;
    if (write_pos > cap) {
      cap = write_pos * 2;
      pcol = (int32_t *)realloc(pcol, (size_t)cap * sizeof(int32_t));
      memset(pcol + write_pos, 0xFF,
             (size_t)(cap - write_pos) * sizeof(int32_t));
    }
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

  /* fprintf(stderr, "[AutoTuner] Converting graph %p: layout %d → %d\n",
          graph_ptr, meta->current_layout, target_layout); */

  /*
   * CASE 1: Transitioning FROM a transient layout BACK TO the BASE SET
   * Action: Rebuild bitmaps from current layout (lazy sync), then DESTROY the
   * transient layout.
   */
  if (target_layout == LAYOUT_SET) {
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
                          meta->bcsr_brow_ptr, meta->bcsr_bcol_idx, &rp,
                          &ci, &mm);
      nn = meta->csr_n;
      free_rp_ci = 1;
      free(meta->bcsr_brow_ptr);
      meta->bcsr_brow_ptr = NULL;
      free(meta->bcsr_bcol_idx);
      meta->bcsr_bcol_idx = NULL;
    }

    if (rp && ci)
      rebuild_sets_from_csr_meta(meta, rp, ci, nn, mm);
    if (free_rp_ci && rp) { free(rp); }
    if (free_rp_ci && ci) { free(ci); }

    meta->csr_row_ptr = NULL;
    meta->csr_col_idx = NULL;
    meta->csr_owned = 0;

    char *base = (char *)graph_ptr;
    nn = (int64_t)roaring_bitmap_get_cardinality((RoaringBitmap *)meta->nodes_bitmap);
    mm = 0;
    RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
    for (int64_t e = 0; e < meta->static_pair_count; e++) {
      if (roaring_bitmap_contains(eb, (uint32_t)e))
        mm++;
    }
    for (int64_t i = 0; i < meta->extra_edge_count; i++) {
      if (meta->extra_edge_live[i])
        mm++;
    }
    *((int64_t *)(base + 0)) = nn;
    *((int64_t *)(base + 8)) = mm;
    *((int64_t **)(base + 16)) = NULL;
    *((int32_t **)(base + 24)) = NULL;

    meta->current_layout = LAYOUT_SET;
    return;
  }

  /*
   * CASE 2: Transitioning FROM BASE SET to a TRANSIENT LAYOUT
   * Action: Build the layout directly from the Sets.
   */
  if (meta->current_layout == LAYOUT_SET) {
    if (target_layout == LAYOUT_CSR) {
      build_csr_from_meta(meta, &meta->csr_n, &meta->csr_row_ptr, &meta->csr_col_idx, &meta->csr_m);
      meta->current_layout = LAYOUT_CSR;
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
      meta->current_layout = LAYOUT_PCSR;
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
      meta->current_layout = LAYOUT_BCSR;
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
    meta->current_layout = target_layout;
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
  return 1;
}

int autograph_canonical_remove_node(void *graph_ptr, int32_t node_label) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->nodes_bitmap || !meta->edges_bitmap)
    return 0;
  roaring_bitmap_remove((RoaringBitmap *)meta->nodes_bitmap, (uint32_t)node_label);

  EdgePair *pairs = (EdgePair *)meta->edge_pairs_table;
  RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
  for (int64_t i = 0; i < meta->static_pair_count; i++) {
    if (!roaring_bitmap_contains(eb, (uint32_t)i))
      continue;
    if (pairs[i].u == node_label || pairs[i].v == node_label) {
      roaring_bitmap_remove(eb, (uint32_t)i);
    }
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {
    if (!meta->extra_edge_live[i])
      continue;
    int32_t u = meta->extra_edge_pairs[2 * i];
    int32_t v = meta->extra_edge_pairs[2 * i + 1];
    if (u == node_label || v == node_label)
      meta->extra_edge_live[i] = 0;
  }
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
    roaring_bitmap_add((RoaringBitmap *)meta->edges_bitmap, (uint32_t)sidx);
    return 1;
  }
  int64_t eidx = canonical_pair_find_extra(meta, u, v);
  if (eidx >= 0) {
    meta->extra_edge_live[eidx] = 1;
    return 1;
  }
  if (!ensure_extra_capacity(meta, meta->extra_edge_count + 1))
    return 0;
  int64_t at = meta->extra_edge_count++;
  meta->extra_edge_pairs[2 * at] = u;
  meta->extra_edge_pairs[2 * at + 1] = v;
  meta->extra_edge_live[at] = 1;
  return 1;
}

int autograph_canonical_remove_edge(void *graph_ptr, int32_t u, int32_t v) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (!meta || !meta->edges_bitmap)
    return 0;
  int sidx = canonical_pair_find_static(meta, u, v);
  if (sidx >= 0) {
    roaring_bitmap_remove((RoaringBitmap *)meta->edges_bitmap, (uint32_t)sidx);
    return 1;
  }
  int64_t eidx = canonical_pair_find_extra(meta, u, v);
  if (eidx >= 0) {
    meta->extra_edge_live[eidx] = 0;
    return 1;
  }
  return 1;
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

    char *base = (char *)graph_ptr;
    meta->csr_row_ptr = *((int64_t **)(base + 16));
    meta->csr_col_idx = *((int32_t **)(base + 24));
    meta->csr_owned = 0;
    meta->current_layout = LAYOUT_CSR;

    /* fprintf(stderr, "[AutoTuner] Initialized Graph %p (n=%ld, m=%ld) in CSR baseline layout\n",
            graph_ptr, (long)n, (long)m); */
}
