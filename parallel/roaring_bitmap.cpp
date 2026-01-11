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
    bm->containers = reinterpret_cast<Container *>(bm->arena.alloc(sizeof(Container) * initial_capacity));
    std::memset(bm->containers, 0, sizeof(Container) * initial_capacity);
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

Container *find_container(RoaringBitmap *bm, uint16_t key)
{
    for (size_t i = 0; i < bm->num_containers; ++i)
    {
        if (bm->containers[i].key == key)
        {
            return &bm->containers[i];
        }
    }
    return nullptr;
}

Container *create_container(RoaringBitmap *bm, uint16_t key, ContainerType type)
{
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

    Container *container = &bm->containers[bm->num_containers++];
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

void roaring_bitmap_portable_serialize(RoaringBitmap *bm, uint8_t *buf)
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

uint8_t *roaring_from_serialized(const uint8_t *data, uint64_t size)
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
    bm->arena.buffer = new uint8_t[size];
    bm->arena.capacity = size;
    bm->arena.offset = 0;

    // allocate container array
    bm->containers = reinterpret_cast<Container *>(bm->arena.alloc(sizeof(Container) * num_containers));
    if (!bm->containers)
        return nullptr;

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
                return nullptr;
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
                return nullptr;

            std::memcpy(c.array.values, ptr, c.array.cardinality * sizeof(uint16_t));
            ptr += c.array.cardinality * sizeof(uint16_t);
        }
        else
        {
            // unknown container type
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
