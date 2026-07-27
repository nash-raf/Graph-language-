#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// -------------------------------
// Your bitmap type
// -------------------------------
struct ArenaBlock {
  uint8_t *buffer;
  size_t capacity;
  size_t offset;
  ArenaBlock *next;
};

struct MemoryArena {
  ArenaBlock *head;
  ArenaBlock *current;
  size_t default_block_size;

  // Statistics
  size_t total_allocations;
  size_t total_bytes_allocated;
  size_t blocks_created;

  MemoryArena()
      : head(nullptr), current(nullptr), default_block_size(0),
        total_allocations(0), total_bytes_allocated(0), blocks_created(0) {}

  uint8_t *alloc(size_t n) {
    total_allocations++;
    total_bytes_allocated += n;

    // Try to allocate from current block
    if (current && current->offset + n <= current->capacity) {
      uint8_t *ptr = current->buffer + current->offset;
      current->offset += n;

      return ptr;
    }

    // Need a new block
    size_t new_block_size = (n > default_block_size) ? n : default_block_size;

    ArenaBlock *new_block = new ArenaBlock();
    new_block->buffer = new uint8_t[new_block_size];
    new_block->capacity = new_block_size;
    new_block->offset = 0;
    new_block->next = nullptr;

    blocks_created++;

    // Link the new block
    if (current) {
      current->next = new_block;
    } else {
      head = new_block;
    }
    current = new_block;

    // Allocate from the new block
    uint8_t *ptr = current->buffer + current->offset;
    current->offset += n;

    return ptr;
  }

  void free_all() {
    ArenaBlock *block = head;
    int count = 0;
    while (block) {
      count++;
      ArenaBlock *next = block->next;
      delete[] block->buffer;
      delete block;
      block = next;
    }

    (void)count;

    head = nullptr;
    current = nullptr;
  }
};

enum ContainerType { ARRAY_CONTAINER, BITMAP_CONTAINER };

struct BitmapContainer {
  uint8_t *bits; // points into arena, size 2^16 / 8 = 8192 bytes
};

struct ArrayContainer {
  uint16_t *values;   // sorted array of 16-bit integers
  size_t cardinality; // number of elements
  size_t capacity;    // allocated capacity
};

struct Container {
  uint16_t key; // high 16 bits
  ContainerType type;
  union {
    ArrayContainer array;
    BitmapContainer bitmap;
  };
};

struct RoaringBitmap {
  MemoryArena arena;
  Container *containers;
  size_t num_containers;
  size_t max_containers; // capacity of container array
};

const size_t THRESHOLD = 4096;

#ifdef __cplusplus
extern "C" {
#endif

RoaringBitmap *roaring_bitmap_create(size_t arena_size,
                                     size_t initial_containers);
RoaringBitmap *roaring_bitmap_copy(RoaringBitmap *bm);
void roaring_bitmap_free(RoaringBitmap *bm);
void roaring_bitmap_clear(RoaringBitmap *bm);
void roaring_bitmap_add(RoaringBitmap *bm, uint32_t value);
size_t roaring_bitmap_portable_size_in_bytes(RoaringBitmap *bm);
void roaring_bitmap_portable_serialize(RoaringBitmap *bm, uint8_t *buf);
RoaringBitmap *roaring_bitmap_portable_deserialize(const uint8_t *data,
                                                   size_t size);
RoaringBitmap *roaring_bitmap_union(RoaringBitmap **bitmaps, size_t count);
RoaringBitmap *roaring_bitmap_intersect(RoaringBitmap *bm1, RoaringBitmap *bm2);
void roaring_print(uint8_t *ptr);
void roaring_bitmap_remove(RoaringBitmap *bm, uint32_t value);
int roaring_bitmap_contains(RoaringBitmap *bm, uint32_t value);
uint32_t roaring_bitmap_get_at_index(RoaringBitmap *bm, uint32_t index);
uint64_t roaring_bitmap_get_cardinality(RoaringBitmap *bm);
void roaring_print_edges(uint8_t *ptr, const int32_t *pairs,
                         uint64_t pair_count);

#ifdef __cplusplus
}
#endif