#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>

// -------------------------------
// Your bitmap type
// -------------------------------
struct ArenaBlock
{
    uint8_t *buffer;
    size_t capacity;
    size_t offset;
    ArenaBlock *next;
};

struct MemoryArena
{
    ArenaBlock *head;
    ArenaBlock *current;
    size_t default_block_size;

    // Statistics
    size_t total_allocations;
    size_t total_bytes_allocated;
    size_t blocks_created;

    MemoryArena() : head(nullptr), current(nullptr), default_block_size(0),
                    total_allocations(0), total_bytes_allocated(0), blocks_created(0) {}

    uint8_t *alloc(size_t n)
    {
        total_allocations++;
        total_bytes_allocated += n;

        // Try to allocate from current block
        if (current && current->offset + n <= current->capacity)
        {
            uint8_t *ptr = current->buffer + current->offset;
            current->offset += n;

            // Log large allocations
            if (n >= 8192)
            {
                fprintf(stderr, "[Arena] Allocated %zu bytes from current block (block has %zu/%zu used)\n",
                        n, current->offset, current->capacity);
            }

            return ptr;
        }

        // Need a new block
        size_t new_block_size = (n > default_block_size) ? n : default_block_size;

        fprintf(stderr, "\n[Arena] *** CREATING NEW BLOCK ***\n");
        fprintf(stderr, "[Arena]   Reason: Request for %zu bytes, current block has %zu free\n",
                n, current ? (current->capacity - current->offset) : 0);
        fprintf(stderr, "[Arena]   New block size: %zu bytes (%.1f MB)\n",
                new_block_size, new_block_size / (1024.0 * 1024.0));

        ArenaBlock *new_block = new ArenaBlock();
        new_block->buffer = new uint8_t[new_block_size];
        new_block->capacity = new_block_size;
        new_block->offset = 0;
        new_block->next = nullptr;

        blocks_created++;

        // Link the new block
        if (current)
        {
            current->next = new_block;
            fprintf(stderr, "[Arena]   Chained to existing block (now have %zu blocks)\n", blocks_created);
        }
        else
        {
            head = new_block;
            fprintf(stderr, "[Arena]   Created first block\n");
        }
        current = new_block;

        // Allocate from the new block
        uint8_t *ptr = current->buffer + current->offset;
        current->offset += n;

        fprintf(stderr, "[Arena]   Allocated %zu bytes from new block\n\n", n);

        return ptr;
    }

    void free_all()
    {
        fprintf(stderr, "\n[Arena] Freeing all arena blocks...\n");
        fprintf(stderr, "[Arena]   Total allocations made: %zu\n", total_allocations);
        fprintf(stderr, "[Arena]   Total bytes allocated: %zu (%.1f MB)\n",
                total_bytes_allocated, total_bytes_allocated / (1024.0 * 1024.0));
        fprintf(stderr, "[Arena]   Blocks created: %zu\n", blocks_created);

        ArenaBlock *block = head;
        int count = 0;
        while (block)
        {
            count++;
            ArenaBlock *next = block->next;
            delete[] block->buffer;
            delete block;
            block = next;
        }

        fprintf(stderr, "[Arena]   Freed %d blocks\n\n", count);

        head = nullptr;
        current = nullptr;
    }
};

enum ContainerType
{
    ARRAY_CONTAINER,
    BITMAP_CONTAINER
};

struct BitmapContainer
{
    uint8_t *bits; // points into arena, size 2^16 / 8 = 8192 bytes
};

struct ArrayContainer
{
    uint16_t *values;   // sorted array of 16-bit integers
    size_t cardinality; // number of elements
    size_t capacity;    // allocated capacity
};

struct Container
{
    uint16_t key; // high 16 bits
    ContainerType type;
    union
    {
        ArrayContainer array;
        BitmapContainer bitmap;
    };
};

struct RoaringBitmap
{
    MemoryArena arena;
    Container *containers;
    size_t num_containers;
    size_t max_containers; // capacity of container array
};

const size_t THRESHOLD = 4096;

#ifdef __cplusplus
extern "C"
{
#endif

    RoaringBitmap *roaring_bitmap_create(size_t arena_size, size_t initial_containers);
    void roaring_bitmap_free(RoaringBitmap *bm);
    void roaring_bitmap_add(RoaringBitmap *bm, uint32_t value);
    size_t roaring_bitmap_portable_size_in_bytes(RoaringBitmap *bm);
    void roaring_bitmap_portable_serialize(RoaringBitmap *bm, uint8_t *buf);
    RoaringBitmap *roaring_bitmap_portable_deserialize(const uint8_t *data, size_t size);
    RoaringBitmap *roaring_bitmap_union(RoaringBitmap **bitmaps, size_t count);
    RoaringBitmap *roaring_bitmap_intersect(RoaringBitmap *bm1, RoaringBitmap *bm2);
    void roaring_print(uint8_t *ptr);

#ifdef __cplusplus
}
#endif