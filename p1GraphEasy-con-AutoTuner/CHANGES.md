# Changes

## 1. Hash-based CSR-to-SET rebuild

**File:** `autotuner_runtime.c`  
**Functions:** `rebuild_sets_from_csr`, `rebuild_sets_from_csr_meta`

**Problem:** The parallel variant (`Graph-language-/parallel/autotuner_runtime.c:158-165`) reverses edge lookup by scanning all `MAX_EDGE_ID` pairs for each CSR edge — O(E²) worst-case:
```c
/* Reverse lookup edge ID. O(E^2) naive, optimize with hash table in prod */
for (uint32_t e = 0; e < MAX_EDGE_ID; e++) {
    if (pairs[e].u == u && pairs[e].v == v) {
        roaring_bitmap_add(eb, e);
        break;
    }
}
```

**Fix:** Introduced `EdgeHashMap` — an open-addressing hash table with linear probing using a SplitMix64 hash of canonical `(lo, hi)` 64-bit keys. Before rebuilding, all `static_pair_count` edges are inserted into the hash (O(E)). Each CSR edge then does an O(1) expected lookup to find its edge ID.

- `EdgeHashMap` struct defined at line 296 with `edge_hash_create`, `edge_hash_insert`, `edge_hash_find`
- `edge_key(u, v)` produces a canonical undirected key: `(min(u,v) << 32) | max(u,v)`
- `edge_hash_destroy` and per-graph cached hashes via `get_static_edge_hash` / `get_or_create_extra_edge_hash`

---

## 2. BCSR insertion — maintain ascending local_row order

**File:** `autotuner_runtime.c`  
**Function:** `autograph_bcsr_add_edge`

**Problem:** New `(local_row, col)` pairs were always appended at `brow[blk+1]` (end of the block's slice). After insertions, pairs for a given row could be split between the original sorted segment and the unordered tail. This meant traversal could never safely break early — it had to scan the entire block.

**Fix:** Before inserting, scan the block's `bcol` slice to find the first position where `bcol[k] > local_row`. Insert the new pair there, shifting subsequent entries right by 2 via `memmove`. This guarantees pairs within each block remain in ascending local_row order:
```
[r0, c, r0, c, ..., r1, c, r1, c, ..., r2, c, ...]
```

The `end = brow[blk+1]`-based append is replaced with a position-finding loop (lines 1371-1379) and `insert_pos`-based `memmove` (line 1389).

---

## 3. BCSR traversal — early break after all neighbors found

**File:** `autotuner_runtime.c`  
**Function:** `autograph_get_neighbors` (LAYOUT_BCSR case)

**Problem:** Traversal scanned every pair in the block from `brow[blk]` to `brow[blk+1]` even after all neighbors for the target `local_row` had been found. Since pairs were potentially unordered (before fix #2), early termination was incorrect.

**Fix:** After fix #2 guarantees ascending local_row order, traversal now breaks as soon as it encounters `local_row > target_row`:
```c
for (int32_t k = start; k < end; k += 2) {
    int32_t r = meta->bcsr_bcol_idx[k];
    if (r == local_row)
        out_buf[(*out_count)++] = meta->bcsr_bcol_idx[k + 1];
    else if (r > local_row)
        break;
}
```

For a vertex with `local_row = 0` at the start of the block, only its own pairs are scanned — all later rows' pairs are skipped. For a vertex in the middle, pairs for earlier rows are skipped and the loop breaks immediately after its row's segment ends.
