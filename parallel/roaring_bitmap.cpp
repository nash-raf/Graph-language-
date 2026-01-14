#include "roaring_bitmap.h"
#include <immintrin.h>
#include <cstdio>

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
    bm->containers = reinterpret_cast<Container *>(bm->arena.alloc(sizeof(Container) * initial_capacity));
    std::memset(bm->containers, 0, sizeof(Container) * initial_capacity);
    return bm;
}

size_t find_insert_position(RoaringBitmap *bm, uint16_t key)
{
    size_t left = 0;
    size_t right = bm->num_containers;

    while (left < right)
    {
        size_t mid = left + (right - left) / 2;
        if (bm->containers[mid].key < key)
            left = mid + 1;
        else
            right = mid;
    }

    return left;
}

Container *create_container(RoaringBitmap *bm, uint16_t key, ContainerType type)
{
    // Resize if needed
    if (bm->num_containers >= bm->max_containers)
    {
        size_t new_capacity = bm->max_containers * 2;
        Container *new_array = reinterpret_cast<Container *>(bm->arena.alloc(sizeof(Container) * new_capacity));
        if (!new_array)
            return nullptr; // out of memory
        std::memcpy(new_array, bm->containers, sizeof(Container) * bm->num_containers);
        bm->containers = new_array;
        bm->max_containers = new_capacity;
    }

    // Find where to insert to maintain sorted order
    size_t insert_pos = find_insert_position(bm, key);

    // Shift containers to the right to make space
    if (insert_pos < bm->num_containers)
    {
        std::memmove(
            &bm->containers[insert_pos + 1],
            &bm->containers[insert_pos],
            sizeof(Container) * (bm->num_containers - insert_pos));
    }

    // Initialize the new container at insert_pos
    Container *container = &bm->containers[insert_pos];
    bm->num_containers++;

    container->key = key;
    container->type = type;

    if (type == BITMAP_CONTAINER)
    {
        container->bitmap.bits = bm->arena.alloc(8192); // 2^16 / 8
        if (!container->bitmap.bits)
            return nullptr; // out of memory
        std::memset(container->bitmap.bits, 0, 8192);
    }
    else if (type == ARRAY_CONTAINER)
    {
        container->array.cardinality = 0;
        container->array.capacity = 16;

        container->array.values = reinterpret_cast<uint16_t *>(
            bm->arena.alloc(sizeof(uint16_t) * container->array.capacity));

        if (!container->array.values)
            return nullptr; // out of memory
    }

    return container;
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

Container *find_container(RoaringBitmap *bm, uint16_t key)
{
    if (bm->num_containers == 0)
        return nullptr;

    size_t left = 0;
    size_t right = bm->num_containers;

    while (left < right)
    {
        size_t mid = left + (right - left) / 2;
        if (bm->containers[mid].key < key)
            left = mid + 1;
        else if (bm->containers[mid].key > key)
            right = mid;
        else
            return &bm->containers[mid]; // found
    }

    return nullptr; // not found
}

// Container *create_container(RoaringBitmap *bm, uint16_t key, ContainerType type)
// {
//     if (bm->num_containers >= bm->max_containers)
//     {
//         size_t new_capacity = bm->max_containers * 2;
//         Container *new_array = reinterpret_cast<Container *>(bm->arena.alloc(sizeof(Container) * new_capacity));
//         if (!new_array)
//             return nullptr; // out of memory
//         std::memcpy(new_array, bm->containers, sizeof(Container) * bm->num_containers);
//         bm->containers = new_array;
//         bm->max_containers = new_capacity;
//     }

//     Container *container = &bm->containers[bm->num_containers++];
//     container->key = key;
//     container->type = type;

//     if (type == BITMAP_CONTAINER)
//     {
//         container->bitmap.bits = bm->arena.alloc(8192); // 2^16 / 8
//         if (!container->bitmap.bits)
//             return nullptr; // out of memory
//         std::memset(container->bitmap.bits, 0, 8192);
//     }
//     else if (type == ARRAY_CONTAINER)
//     {
//         container->array.cardinality = 0;
//         container->array.capacity = 16;

//         container->array.values = reinterpret_cast<uint16_t *>(
//             bm->arena.alloc(sizeof(uint16_t) * container->array.capacity));

//         if (!container->array.values)
//             return nullptr; // out of memory
//     }

//     return container;
// }

void add_to_array(RoaringBitmap *bm, Container *c, uint16_t value)
{
    ArrayContainer &a = c->array;

    size_t left = 0;
    size_t right = a.cardinality;
    while (left < right)
    {
        size_t mid = left + (right - left) / 2;
        if (a.values[mid] < value)
            left = mid + 1;
        else
            right = mid;
    }
    size_t pos = left;

    if (pos < a.cardinality && a.values[pos] == value)
        return;

    if (a.cardinality >= a.capacity)
    {
        size_t new_capacity = a.capacity * 2;
        uint16_t *new_values = reinterpret_cast<uint16_t *>(bm->arena.alloc(sizeof(uint16_t) * new_capacity));
        if (!new_values)
            return; // out of memory
        std::memcpy(new_values, a.values, sizeof(uint16_t) * a.cardinality);
        a.values = new_values;
        a.capacity = new_capacity;
    }

    if (pos < a.cardinality)
    {
        std::memmove(a.values + pos + 1, a.values + pos, sizeof(uint16_t) * (a.cardinality - pos));
    }
    a.values[pos] = value;
    a.cardinality++;
}

void promote_to_bitmap(RoaringBitmap *bm, Container *c)
{
    ArrayContainer &a = c->array;

    uint8_t *bits = reinterpret_cast<uint8_t *>(bm->arena.alloc(8192));
    if (!bits)
        return; // out of memory
    std::memset(bits, 0, 8192);

    for (size_t i = 0; i < a.cardinality; ++i)
    {
        uint16_t v = a.values[i];
        bits[v >> 3] |= (1u << (v & 7));
    }

    c->type = BITMAP_CONTAINER;
    c->bitmap.bits = bits;

    a.values = nullptr;
    a.capacity = 0;
    a.cardinality = 0;
}

inline void set_bit(BitmapContainer &b, uint16_t value)
{
    uint32_t byte_index = value >> 3;
    uint8_t bit_mask = 1u << (value & 7);
    b.bits[byte_index] |= bit_mask;
}

void roaring_bitmap_add(RoaringBitmap *bm, uint32_t value)
{
    if (!bm)
        return;
    uint16_t high = (uint16_t)(value >> 16);
    uint16_t low = (uint16_t)(value & 0xFFFF);

    Container *container = find_container(bm, high);

    if (!container)
    {
        container = create_container(bm, high, ARRAY_CONTAINER);
        if (!container)
            return; // out of memory
    }

    if (container->type == ARRAY_CONTAINER)
    {
        add_to_array(bm, container, low);
        if (container->array.cardinality > THRESHOLD)
        {
            promote_to_bitmap(bm, container);
        }
    }

    else if (container->type == BITMAP_CONTAINER)
    {
        set_bit(container->bitmap, low);
    }
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

// extern "C" RoaringBitmap *roaring_bitmap_intersect(RoaringBitmap *bm1, RoaringBitmap *bm2)
// {
//     if (!bm1 || !bm2)
//         return nullptr;

//     for (size_t i = 0; i < bm1->num_containers; ++i)
//     {
//         if (bm1->containers[i].key != bm2->containers[i].key)
//         {
//             break;
//         }

//         if (bm1->containers[i].type == ARRAY_CONTAINER)
//         {
//             promote_to_bitmap(bm1, &bm1->containers[i]);
//         }

//         if (bm2->containers[i].type == ARRAY_CONTAINER)
//         {
//             promote_to_bitmap(bm2, &bm2->containers[i]);
//         }
//     }

//     uint64_t c = 0;
//     RoaringBitmap *result = new RoaringBitmap();

//     for (uint8_t i = 0; i < 1024; ++i)
//     {
//         if (bm1->containers[i].key != bm2->containers[i].key)
//         {
//             break;
//         }

//         __m256i vecA = _mm256_loadu_si256((__m256i const *)(&bm1->containers[i].bitmap.bits + i * 32));
//         __m256i vecB = _mm256_loadu_si256((__m256i const *)(&bm2->containers[i].bitmap.bits + i * 32));

//         __m256i vecC = _mm256_and_si256(vecA, vecB);
//         uint64_t *cWords = (uint64_t *)&vecC;
//         _mm256_storeu_si256((__m256i *)(result->containers[i].bitmap.bits + i * 32), vecC);

//         cWords = (uint64_t *)&vecC;
//         for (int j = 0; j < 4; j++)
//         {
//             c += __builtin_popcountll(cWords[j]);
//         }
//     }

//     if (c > 4096)
//     {
//         return result;
//     }

//     else
//     {
//         // convert to array container
//         RoaringBitmap *finalResult = new RoaringBitmap();
//         finalResult->num_containers = result->num_containers;
//         finalResult->max_containers = result->max_containers;

//         for (size_t i = 0; i < result->num_containers; ++i)
//         {
//             Container *srcC = &result->containers[i];
//             Container *destC = create_container(finalResult, srcC->key, ARRAY_CONTAINER);

//             for (uint16_t i = 0; i < 1024; ++i)
//             {
//                 uint8_t *w = result->containers[i].bitmap.bits;
//                 uint8_t w_complement = ~(*w);
//                 while (w != 0)
//                 {
//                     uint8_t t = (*w) & w_complement;
//                     uint16_t r = __builtin_popcount(t - 1);
//                     add_to_array(finalResult, destC, r);
//                     *w = (*w) & ((*w) - 1);
//                 }
//             }

//             return finalResult;
//         }

//         // for (size_t i = 0; i < result->num_containers; ++i)
//         // {
//         //     Container *srcC = &result->containers[i];
//         //     Container *destC = create_container(finalResult, srcC->key, ARRAY_CONTAINER);

//         //     for (uint16_t v = 0; v < 65536; ++v)
//         //     {
//         //         uint32_t byte_index = v >> 3;
//         //         uint8_t bit_mask = 1u << (v & 7);
//         //         if (srcC->bitmap.bits[byte_index] & bit_mask)
//         //         {
//         //             add_to_array(finalResult, destC, v);
//         //         }
//         //     }
//         // }

//         // delete result;
//         // return finalResult;
//     }

//     return nullptr;
// }

extern "C" RoaringBitmap *roaring_bitmap_intersect(RoaringBitmap *bm1, RoaringBitmap *bm2)
{
    if (!bm1 || !bm2)
        return nullptr;

    // fprintf(stderr, "bm1->num_containers = %zu\n", bm1->num_containers);
    // fprintf(stderr, "bm2->num_containers = %zu\n", bm2->num_containers);

    // Create result bitmap with estimated capacity
    size_t estimated_containers = (bm1->num_containers < bm2->num_containers)
                                      ? bm1->num_containers
                                      : bm2->num_containers;
    RoaringBitmap *result = roaring_bitmap_create(100 * 1024 * 1024, estimated_containers);

    uint64_t total_cardinality = 0;

    // Two-pointer scan through sorted containers
    size_t i1 = 0, i2 = 0;

    while (i1 < bm1->num_containers && i2 < bm2->num_containers)
    {
        uint16_t key1 = bm1->containers[i1].key;
        uint16_t key2 = bm2->containers[i2].key;

        if (key1 < key2)
        {
            i1++;
        }
        else if (key1 > key2)
        {
            i2++;
        }
        else
        {
            // Keys match - intersect these containers
            Container *c1 = &bm1->containers[i1];
            Container *c2 = &bm2->containers[i2];

            // fprintf(stderr, "Intersecting container with key=%u\n", key1);

            // Create result container (bitmap type)
            Container *resultC = create_container(result, key1, BITMAP_CONTAINER);
            if (!resultC)
            {
                // fprintf(stderr, "ERROR: Failed to create result container\n");
                return nullptr;
            }

            // Get or create bitmap representations
            uint8_t *bits1 = nullptr;
            uint8_t *bits2 = nullptr;
            uint8_t temp_bits1[8192];
            uint8_t temp_bits2[8192];

            if (c1->type == BITMAP_CONTAINER)
            {
                bits1 = c1->bitmap.bits;
            }
            else
            {
                // Convert array to bitmap on stack
                std::memset(temp_bits1, 0, 8192);
                for (size_t i = 0; i < c1->array.cardinality; ++i)
                {
                    uint16_t v = c1->array.values[i];
                    temp_bits1[v >> 3] |= (1u << (v & 7));
                }
                bits1 = temp_bits1;
            }

            if (c2->type == BITMAP_CONTAINER)
            {
                bits2 = c2->bitmap.bits;
            }
            else
            {
                // Convert array to bitmap on stack
                std::memset(temp_bits2, 0, 8192);
                for (size_t i = 0; i < c2->array.cardinality; ++i)
                {
                    uint16_t v = c2->array.values[i];
                    temp_bits2[v >> 3] |= (1u << (v & 7));
                }
                bits2 = temp_bits2;
            }

            // Perform AVX2 intersection
            uint64_t cardinality = 0;
            for (size_t word_idx = 0; word_idx < 256; ++word_idx)
            {
                __m256i vecA = _mm256_loadu_si256((__m256i const *)(bits1 + word_idx * 32));
                __m256i vecB = _mm256_loadu_si256((__m256i const *)(bits2 + word_idx * 32));
                __m256i vecC = _mm256_and_si256(vecA, vecB);
                _mm256_storeu_si256((__m256i *)(resultC->bitmap.bits + word_idx * 32), vecC);

                uint64_t *cWords = (uint64_t *)&vecC;
                for (int j = 0; j < 4; j++)
                {
                    cardinality += __builtin_popcountll(cWords[j]);
                }
            }

            total_cardinality += cardinality;
            // fprintf(stderr, "Container cardinality: %lu\n", cardinality);

            i1++;
            i2++;
        }
    }

    // fprintf(stderr, "Total intersection cardinality: %lu\n", total_cardinality);

    // If small result, convert to array containers
    if (total_cardinality <= 4096)
    {
        // fprintf(stderr, "Converting to array containers\n");
        RoaringBitmap *finalResult = roaring_bitmap_create(100 * 1024 * 1024, result->num_containers);

        for (size_t i = 0; i < result->num_containers; ++i)
        {
            Container *srcC = &result->containers[i];
            Container *destC = create_container(finalResult, srcC->key, ARRAY_CONTAINER);
            if (!destC)
            {
                // fprintf(stderr, "ERROR: Failed to create array container\n");
                roaring_bitmap_free(result);
                return nullptr;
            }

            // Extract bits from bitmap to array
            uint64_t *words = reinterpret_cast<uint64_t *>(srcC->bitmap.bits);
            for (uint16_t word_idx = 0; word_idx < 1024; ++word_idx)
            {
                uint64_t w = words[word_idx];
                while (w != 0)
                {
                    uint64_t t = w & (-w);
                    int r = __builtin_ctzll(t);
                    add_to_array(finalResult, destC, static_cast<uint16_t>(word_idx * 64 + r));
                    w = w & (w - 1);
                }
            }
        }

        roaring_bitmap_free(result);
        return finalResult;
    }

    return result;
}

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
extern "C" size_t roaring_bitmap_portable_size_in_bytes(RoaringBitmap *bm)
{
    if (!bm)
        return 0;

    size_t total_size = sizeof(size_t); // for num_containers

    for (size_t i = 0; i < bm->num_containers; ++i)
    {
        Container &c = bm->containers[i];

        total_size += sizeof(uint16_t); // key
        total_size += sizeof(uint8_t);  // type

        if (c.type == BITMAP_CONTAINER)
        {
            total_size += 8192;
        }
        else if (c.type == ARRAY_CONTAINER)
        {
            total_size += sizeof(size_t);                         // cardinality
            total_size += c.array.cardinality * sizeof(uint16_t); // array elements
        }
    }

    return total_size;
}

extern "C" void roaring_bitmap_portable_serialize(RoaringBitmap *bm, uint8_t *buf)
{
    if (!bm || !buf)
        return;

    uint8_t *ptr = buf;
    *reinterpret_cast<size_t *>(ptr) = bm->num_containers;
    ptr += sizeof(size_t);

    for (size_t i = 0; i < bm->num_containers; ++i)
    {
        Container &c = bm->containers[i];
        reinterpret_cast<uint16_t *>(ptr)[0] = c.key;
        ptr += sizeof(uint16_t);

        *reinterpret_cast<uint8_t *>(ptr) = static_cast<uint8_t>(c.type);
        ptr += sizeof(uint8_t);

        if (c.type == BITMAP_CONTAINER)
        {
            std::memcpy(ptr, c.bitmap.bits, 8192);
            ptr += 8192;
        }
        else if (c.type == ARRAY_CONTAINER)
        {
            // write array cardinality first
            *reinterpret_cast<size_t *>(ptr) = c.array.cardinality;
            ptr += sizeof(size_t);

            std::memcpy(ptr, c.array.values, c.array.cardinality * sizeof(uint16_t));
            ptr += c.array.cardinality * sizeof(uint16_t);
        }
    }
}

// -------------------------------
// Runtime helpers (for LLVM IR)
// -------------------------------

extern "C" uint8_t *roaring_from_serialized(const uint8_t *data, uint64_t size)
{
    if (!data || size == 0)
        return nullptr;

    const uint8_t *ptr = data;

    // read number of containers
    size_t num_containers = *reinterpret_cast<const size_t *>(ptr);
    ptr += sizeof(size_t);

    RoaringBitmap *bm = new RoaringBitmap();
    bm->num_containers = num_containers;
    bm->max_containers = num_containers;

    // FIX: Allocate a much larger arena for the deserialized bitmap
    size_t arena_size = 100 * 1024 * 1024; // 100MB like in roaring_bitmap_create
    bm->arena.buffer = new uint8_t[arena_size];
    bm->arena.capacity = arena_size;
    bm->arena.offset = 0;

    // allocate container array
    bm->containers = reinterpret_cast<Container *>(bm->arena.alloc(sizeof(Container) * num_containers));
    if (!bm->containers)
    {
        fprintf(stderr, "ERROR: Failed to allocate containers in roaring_from_serialized\n");
        return nullptr;
    }

    std::memset(bm->containers, 0, sizeof(Container) * num_containers);

    for (size_t i = 0; i < num_containers; ++i)
    {
        Container &c = bm->containers[i];

        // read key
        c.key = *reinterpret_cast<const uint16_t *>(ptr);
        ptr += sizeof(uint16_t);

        // read type
        c.type = static_cast<ContainerType>(*reinterpret_cast<const uint8_t *>(ptr));
        ptr += sizeof(uint8_t);

        if (c.type == BITMAP_CONTAINER)
        {
            c.bitmap.bits = bm->arena.alloc(8192);
            if (!c.bitmap.bits)
            {
                fprintf(stderr, "ERROR: Failed to allocate bitmap container\n");
                return nullptr;
            }
            std::memcpy(c.bitmap.bits, ptr, 8192);
            ptr += 8192;
        }
        else if (c.type == ARRAY_CONTAINER)
        {
            // read array cardinality
            c.array.cardinality = *reinterpret_cast<const size_t *>(ptr);
            ptr += sizeof(size_t);

            c.array.capacity = c.array.cardinality;
            c.array.values = reinterpret_cast<uint16_t *>(bm->arena.alloc(sizeof(uint16_t) * c.array.capacity));
            if (!c.array.values)
            {
                fprintf(stderr, "ERROR: Failed to allocate array container\n");
                return nullptr;
            }

            std::memcpy(c.array.values, ptr, c.array.cardinality * sizeof(uint16_t));
            ptr += c.array.cardinality * sizeof(uint16_t);
        }
        else
        {
            // unknown container type
            fprintf(stderr, "ERROR: Unknown container type\n");
            return nullptr;
        }
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

extern "C" void roaring_print(uint8_t *ptr)
{
    if (!ptr)
    {
        printf("{}\n");
        return;
    }
    RoaringBitmap *bm = reinterpret_cast<RoaringBitmap *>(ptr);
    printf("{");
    bool first = true;
    for (size_t i = 0; i < bm->num_containers; ++i)
    {
        Container &c = bm->containers[i];
        uint32_t base = static_cast<uint32_t>(c.key) << 16;
        if (c.type == ARRAY_CONTAINER)
        {
            for (size_t j = 0; j < c.array.cardinality; ++j)
            {
                if (!first)
                    printf(", ");
                printf("%u", base + c.array.values[j]);
                first = false;
            }
        }
        else if (c.type == BITMAP_CONTAINER)
        {
            uint64_t *words = reinterpret_cast<uint64_t *>(c.bitmap.bits);
            for (int w = 0; w < 1024; ++w)
            {
                uint64_t word = words[w];
                if (word == 0)
                    continue;
                while (word)
                {
                    int bit = __builtin_ctzll(word);
                    if (!first)
                        printf(", ");
                    printf("%u", base + (w * 64 + bit));
                    first = false;
                    word &= word - 1;
                }
            }
        }
    }
    printf("}\n");
    fflush(stdout);
}