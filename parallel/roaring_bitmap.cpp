#include "roaring_bitmap.h"

// -------------------------------
// Creation / Destruction
// -------------------------------

RoaringBitmap *roaring_bitmap_create(size_t arena_size = 100 * 1024 * 1024, size_t initial_capacity = 8)
{
    RoaringBitmap *bm = new RoaringBitmap();

    // initialize arena
    bm->arena.buffer = new uint8_t[arena_size];
    bm->arena.capacity = arena_size;
    bm->arena.offset = 0;

    // initialize container array
    bm->num_containers = 0;
    bm->max_containers = initial_capacity;
    bm->containers = reinterpret_cast<BitmapContainer *>(bm->arena.alloc(sizeof(BitmapContainer) * initial_capacity));

    return bm;
}

void roaring_bitmap_free(RoaringBitmap *bm)
{
    if (!bm)
        return;
    delete bm;
}

// -------------------------------
// Mutation
// -------------------------------

void roaring_bitmap_add(RoaringBitmap *bm, uint32_t value)
{
    if (!bm)
        return;
    uint16_t high = (uint16_t)(value >> 16);
    uint16_t low = (uint16_t)(value & 0xFFFF);

    BitmapContainer *container = nullptr;

    for (size_t i = 0; i < bm->num_containers; ++i)
    {
        if (bm->containers[i].key == high)
        {
            container = &bm->containers[i];
            break;
        }
    }

    if (!container)
    {
        if (bm->num_containers >= bm->max_containers)
        {
            size_t new_capacity = bm->max_containers * 2;
            BitmapContainer *new_array = reinterpret_cast<BitmapContainer *>(bm->arena.alloc(sizeof(BitmapContainer) * new_capacity));
            if (!new_array)
                return; // out of memory
            std::memcpy(new_array, bm->containers, sizeof(BitmapContainer) * bm->num_containers);
            bm->containers = new_array;
            bm->max_containers = new_capacity;
        }

        container = &bm->containers[bm->num_containers++];
        container->key = high;
        container->bits = bm->arena.alloc(8192);
        if (!container->bits)
            return;
        std::memset(container->bits, 0, 8192);
    }

    uint32_t byte_index = low >> 3;
    uint8_t bit_mask = (uint8_t)(1u << (low & 7));

    container->bits[byte_index] |= bit_mask;
}

// inline void roaring_bitmap_remove(RoaringBitmap *bm, uint32_t value)
// {
//     if (!bm)
//         return;
//     // TODO: implement actual remove
//     auto &v = bm->data;
//     for (size_t i = 0; i < v.size(); ++i)
//     {
//         if (v[i] == value)
//         {
//             v[i] = v.back();
//             v.pop_back();
//             break;
//         }
//     }
// }

// -------------------------------
// Queries
// -------------------------------

// inline int roaring_bitmap_contains(roaring_bitmap_t *bm, uint32_t value)
// {
//     if (!bm)
//         return 0;
//     for (auto x : bm->data)
//     {
//         if (x == value)
//             return 1;
//     }
//     return 0;
// }

// inline uint64_t roaring_bitmap_get_cardinality(roaring_bitmap_t *bm)
// {
//     if (!bm)
//         return 0;
//     return bm->data.size();
// }

// -------------------------------
// Serialization / Deserialization
// -------------------------------

size_t roaring_bitmap_portable_size_in_bytes(RoaringBitmap *bm)
{
    if (!bm)
        return 0;
    return sizeof(size_t) + bm->num_containers * (sizeof(uint16_t) + 8192);
}

void roaring_bitmap_portable_serialize(RoaringBitmap *bm, uint8_t *buf)
{
    if (!bm || !buf)
        return;

    uint8_t *ptr = buf;
    *reinterpret_cast<size_t *>(ptr) = bm->num_containers;
    ptr += sizeof(size_t);

    for (size_t i = 0; i < bm->num_containers; ++i)
    {
        BitmapContainer &container = bm->containers[i];
        *reinterpret_cast<uint16_t *>(ptr) = container.key;
        ptr += sizeof(uint16_t);
        std::memcpy(ptr, container.bits, 8192);
        ptr += 8192;
    }
}

// -------------------------------
// Runtime helpers (for LLVM IR)
// -------------------------------

uint8_t *roaring_from_serialized(const uint8_t *data, uint64_t size)
{

    const uint8_t *ptr = data;

    size_t num_containers = *reinterpret_cast<const size_t *>(ptr);
    std::memcpy(&num_containers, ptr, sizeof(size_t));
    ptr += sizeof(size_t);

    RoaringBitmap *bm = new RoaringBitmap();
    bm->num_containers = num_containers;
    bm->max_containers = num_containers;
    bm->arena.buffer = new uint8_t[size];
    bm->arena.capacity = size;
    bm->arena.offset = 0;
    bm->containers = reinterpret_cast<BitmapContainer *>(bm->arena.alloc(sizeof(BitmapContainer) * num_containers));

    if (!bm->containers)
    {
        return nullptr;
    }

    for (size_t i = 0; i < num_containers; ++i)
    {
        BitmapContainer &container = bm->containers[i];

        std::memcpy(&container.key, ptr, sizeof(uint16_t));
        ptr += sizeof(uint16_t);
        container.bits = bm->arena.alloc(8192);
        if (!container.bits)
            return nullptr;
        std::memcpy(container.bits, ptr, 8192);
        ptr += 8192;
    }

    return reinterpret_cast<uint8_t *>(bm);
}

// inline void __init_serialized_global(void **dest_ptr, const uint8_t *data, uint64_t size)
// {
//     if (!dest_ptr)
//         return;
//     *dest_ptr = roaring_from_serialized(data, size);
// }

// inline void __destroy_global(void **dest_ptr)
// {
//     if (!dest_ptr)
//         return;
//     void *p = *dest_ptr;
//     if (p)
//     {
//         roaring_bitmap_free(reinterpret_cast<roaring_bitmap_t *>(p));
//         *dest_ptr = nullptr;
//     }
// }
