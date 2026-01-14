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
    RoaringBitmap *roaring_bitmap_union(RoaringBitmap *bm1, RoaringBitmap *bm2);
    RoaringBitmap *roaring_bitmap_intersect(RoaringBitmap *bm1, RoaringBitmap *bm2);
    void roaring_print(uint8_t *ptr);

#ifdef __cplusplus
}
#endif