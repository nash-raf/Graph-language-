#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

// -------------------------------
// Your bitmap type
// -------------------------------
struct MemoryArena
{
    uint8_t *buffer;
    size_t capacity;
    size_t offset;

    uint8_t *alloc(size_t n)
    {
        if (offset + n > capacity)
            return nullptr; // out of memory
        uint8_t *ptr = buffer + offset;
        offset += n;
        return ptr;
    }
};

struct BitmapContainer
{
    uint16_t key;  // high 16 bits
    uint8_t *bits; // points into arena, size 2^16 / 8 = 8192 bytes
};

struct RoaringBitmap
{
    MemoryArena arena;
    BitmapContainer *containers;
    size_t num_containers;
    size_t max_containers; // capacity of container array
};

#ifdef __cplusplus
extern "C"
{
#endif

    RoaringBitmap *roaring_bitmap_create(size_t arena_size, size_t initial_capacity);
    void roaring_bitmap_free(RoaringBitmap *bm);
    void roaring_bitmap_add(RoaringBitmap *bm, uint32_t value);
    size_t roaring_bitmap_portable_size_in_bytes(RoaringBitmap *bm);
    void roaring_bitmap_portable_serialize(RoaringBitmap *bm, uint8_t *buf);
    uint8_t *roaring_from_serialized(const uint8_t *data, uint64_t size);

#ifdef __cplusplus
}
#endif