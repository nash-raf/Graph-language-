#include "roaring_bitmap.h"
#include <immintrin.h>
#include <cstdio>
#include <omp.h>
#include <iostream>

// Heap node containing a container pointer and metadata
struct ContainerHeapNode
{
    Container *container; // Pointer to the container
    size_t bitmap_idx;    // Which bitmap this container came from
    size_t container_idx; // Position in that bitmap's container array

    ContainerHeapNode() : container(nullptr), bitmap_idx(0), container_idx(0) {}

    ContainerHeapNode(Container *c, size_t bmap_idx, size_t cont_idx)
        : container(c), bitmap_idx(bmap_idx), container_idx(cont_idx) {}

    // Comparison: smaller key = higher priority (min-heap)
    bool operator>(const ContainerHeapNode &other) const
    {
        if (!container)
            return true; // null is lowest priority
        if (!other.container)
            return false;
        return container->key > other.container->key;
    }
};

// Min-heap for container merging
struct ContainerMinHeap
{
    ContainerHeapNode *nodes; // Array of heap nodes
    size_t size;              // Current number of elements
    size_t capacity;          // Maximum capacity

    ContainerMinHeap(size_t cap)
    {
        capacity = cap;
        size = 0;
        nodes = new ContainerHeapNode[capacity];
    }

    ~ContainerMinHeap()
    {
        delete[] nodes;
    }

    // Swap two nodes
    inline void swap(size_t i, size_t j)
    {
        ContainerHeapNode temp = nodes[i];
        nodes[i] = nodes[j];
        nodes[j] = temp;
    }

    // Get parent index
    inline size_t parent(size_t i) const { return (i - 1) >> 1; }

    // Get left child index
    inline size_t left_child(size_t i) const { return (i << 1) + 1; }

    // Get right child index
    inline size_t right_child(size_t i) const { return (i << 1) + 2; }

    // Bubble up element at index i
    void bubble_up(size_t i)
    {
        while (i > 0)
        {
            size_t p = parent(i);
            if (!(nodes[i] > nodes[p])) // nodes[i] <= nodes[p] (smaller key or equal)
            {
                swap(i, p);
                i = p;
            }
            else
            {
                break;
            }
        }
    }

    // Bubble down element at index i
    void bubble_down(size_t i)
    {
        while (true)
        {
            size_t smallest = i;
            size_t left = left_child(i);
            size_t right = right_child(i);

            // Find smallest among node, left child, and right child
            if (left < size && nodes[left] > nodes[smallest])
            {
                smallest = left;
            }

            if (right < size && nodes[right] > nodes[smallest])
            {
                smallest = right;
            }

            if (smallest != i)
            {
                swap(i, smallest);
                i = smallest;
            }
            else
            {
                break;
            }
        }
    }

    // Insert a new node
    void push(const ContainerHeapNode &node)
    {
        if (size >= capacity)
        {
            // Should not happen if capacity is set correctly
            return;
        }

        nodes[size] = node;
        bubble_up(size);
        size++;
    }

    // Get the minimum element (top of heap)
    const ContainerHeapNode &top() const
    {
        return nodes[0];
    }

    // Remove and return the minimum element
    ContainerHeapNode pop()
    {
        if (size == 0)
        {
            return ContainerHeapNode(); // Empty node
        }

        ContainerHeapNode result = nodes[0];

        size--;
        if (size > 0)
        {
            nodes[0] = nodes[size];
            bubble_down(0);
        }

        return result;
    }

    // Check if heap is empty
    bool empty() const { return size == 0; }
};

// Build initial min-heap from all bitmaps
// Each bitmap contributes its first container (if it has any)
ContainerMinHeap *build_container_heap(RoaringBitmap **bitmaps, size_t count)
{
    ContainerMinHeap *heap = new ContainerMinHeap(count);

    // Insert the first container from each bitmap
    for (size_t i = 0; i < count; ++i)
    {
        if (bitmaps[i] && bitmaps[i]->num_containers > 0)
        {
            ContainerHeapNode node(
                &bitmaps[i]->containers[0], // First container
                i,                          // Bitmap index
                0                           // Container index (first one)
            );
            heap->push(node);
        }
    }

    return heap;
}

// Advance to next container in the same bitmap and re-insert into heap
void advance_and_reinsert(ContainerMinHeap *heap, RoaringBitmap **bitmaps,
                          const ContainerHeapNode &node)
{
    size_t next_idx = node.container_idx + 1;

    // Check if there's another container in this bitmap
    if (next_idx < bitmaps[node.bitmap_idx]->num_containers)
    {
        ContainerHeapNode next_node(
            &bitmaps[node.bitmap_idx]->containers[next_idx],
            node.bitmap_idx,
            next_idx);
        heap->push(next_node);
    }
    // If no more containers in this bitmap, don't re-insert
}
// -------------------------------
// Creation / Destruction
// -------------------------------

RoaringBitmap *roaring_bitmap_create(size_t arena_size, size_t initial_capacity)
{
    RoaringBitmap *bm = new RoaringBitmap();

    // Initialize arena with chaining support
    ArenaBlock *first_block = new ArenaBlock();
    first_block->buffer = new uint8_t[arena_size];
    first_block->capacity = arena_size;
    first_block->offset = 0;
    first_block->next = nullptr;

    bm->arena.head = first_block;
    bm->arena.current = first_block;
    bm->arena.default_block_size = arena_size;

    // Initialize container array
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
        // *** CRITICAL: Initialize bitmap to zero ***
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

void add_container(RoaringBitmap *bm, Container *cont)
{
    // Resize if needed
    if (bm->num_containers >= bm->max_containers)
    {
        size_t new_capacity = bm->max_containers * 2;
        Container *new_array = reinterpret_cast<Container *>(bm->arena.alloc(sizeof(Container) * new_capacity));
        if (!new_array)
            return; // out of memory
        std::memcpy(new_array, bm->containers, sizeof(Container) * bm->num_containers);
        bm->containers = new_array;
        bm->max_containers = new_capacity;
    }

    // Find where to insert to maintain sorted order
    size_t insert_pos = find_insert_position(bm, cont->key);

    // Shift containers to the right to make space
    if (insert_pos < bm->num_containers)
    {
        std::memmove(
            &bm->containers[insert_pos + 1],
            &bm->containers[insert_pos],
            sizeof(Container) * (bm->num_containers - insert_pos));
    }

    // *** FIX: Actually copy the container data ***
    bm->containers[insert_pos] = *cont;
    bm->num_containers++;
}

void roaring_bitmap_free(RoaringBitmap *bm)
{
    if (!bm)
        return;

    bm->arena.free_all();
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
    // Save the array values pointer before we change anything
    uint16_t *old_values = c->array.values;
    size_t old_cardinality = c->array.cardinality;

    // Calculate total arena usage across all blocks
    size_t total_used = 0;
    size_t total_capacity = 0;
    int block_count = 0;
    ArenaBlock *block = bm->arena.head;
    while (block)
    {
        total_used += block->offset;
        total_capacity += block->capacity;
        block_count++;
        block = block->next;
    }

    fprintf(stderr, "  promote_to_bitmap: Allocating 8192 bytes (total used=%zu, total capacity=%zu, blocks=%d)\n",
            total_used, total_capacity, block_count);

    uint8_t *bits = reinterpret_cast<uint8_t *>(bm->arena.alloc(8192));
    if (!bits)
    {
        fprintf(stderr, "  ERROR: Arena allocation FAILED!\n");
        return;
    }

    fprintf(stderr, "  promote_to_bitmap: Allocated bits at %p (now %d blocks)\n",
            (void *)bits, block_count);
    std::memset(bits, 0, 8192);

    // Copy values from old array to new bitmap
    for (size_t i = 0; i < old_cardinality; ++i)
    {
        uint16_t v = old_values[i];
        bits[v >> 3] |= (1u << (v & 7));
    }

    // Now change the container type and set bitmap fields
    c->type = BITMAP_CONTAINER;
    c->bitmap.bits = bits;

    fprintf(stderr, "  promote_to_bitmap: Complete. Container type=%d, bits=%p\n",
            (int)c->type, (void *)c->bitmap.bits);
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
        {
            fprintf(stderr, "ERROR: create_container failed for value %u\n", value);
            return;
        }
    }

    if (container->type == ARRAY_CONTAINER)
    {
        add_to_array(bm, container, low);

        // Check if we need to promote
        if (container->array.cardinality > THRESHOLD)
        {
            fprintf(stderr, "Promoting container with key=%u, cardinality=%zu to bitmap\n",
                    container->key, container->array.cardinality);

            ContainerType oldType = container->type;
            promote_to_bitmap(bm, container);

            // Verify promotion succeeded
            if (container->type == oldType)
            {
                // Promotion failed - type wasn't changed
                fprintf(stderr, "ERROR: Promotion failed! Still ARRAY_CONTAINER. Stopping.\n");
                exit(1);
            }
            if (!container->bitmap.bits)
            {
                fprintf(stderr, "ERROR: Promotion succeeded but bitmap.bits is NULL!\n");
                exit(1);
            }
            fprintf(stderr, "Promotion successful, bitmap.bits = %p\n",
                    (void *)container->bitmap.bits);
        }
    }
    else if (container->type == BITMAP_CONTAINER)
    {
        if (!container->bitmap.bits)
        {
            fprintf(stderr, "ERROR: About to set bit in NULL bitmap! value=%u, high=%u, low=%u\n",
                    value, high, low);
            fprintf(stderr, "Container key=%u, type=%d\n",
                    container->key, (int)container->type);
            fprintf(stderr, "This should never happen - bitmap container must have valid bits!\n");
            exit(1);
        }
        set_bit(container->bitmap, low);
    }
}

// -------------------------------
// Queries
// -------------------------------

// Three-way partition for quicksort (handles duplicates efficiently)
// Returns: {left_partition_end, right_partition_start}
struct PartitionResult
{
    size_t left_end;
    size_t right_start;
};

// Helper: Get container cardinality (works with Container pointers)
inline size_t get_container_cardinality(const Container *c)
{
    if (!c)
        return 0;

    if (c->type == BITMAP_CONTAINER)
    {
        uint64_t *words = reinterpret_cast<uint64_t *>(c->bitmap.bits);
        size_t count = 0;
        for (int i = 0; i < 1024; ++i)
        {
            count += __builtin_popcountll(words[i]);
        }
        return count;
    }
    else if (c->type == ARRAY_CONTAINER)
    {
        return c->array.cardinality;
    }
    return 0;
}

// Three-way partition for quicksort on container pointer arrays
inline PartitionResult partition_three_way_ptrs(Container **containers, size_t low, size_t high)
{
    size_t mid = low + (high - low) / 2;
    size_t pivot_idx = low;

    size_t card_low = get_container_cardinality(containers[low]);
    size_t card_mid = get_container_cardinality(containers[mid]);
    size_t card_high = get_container_cardinality(containers[high]);

    if (card_mid > card_low && card_mid > card_high)
        pivot_idx = mid;
    else if (card_high > card_low && card_high > card_mid)
        pivot_idx = high;

    std::swap(containers[pivot_idx], containers[high]);
    size_t pivot_cardinality = get_container_cardinality(containers[high]);

    size_t i = low;
    size_t k = high;

    for (size_t j = low; j < k; ++j)
    {
        size_t card_j = get_container_cardinality(containers[j]);
        if (card_j > pivot_cardinality)
        {
            std::swap(containers[i], containers[j]);
            i++;
        }
        else if (card_j == pivot_cardinality)
        {
            std::swap(containers[j], containers[k - 1]);
            k--;
            j--;
        }
    }

    std::swap(containers[high], containers[k]);
    return {i, k};
}

// Optimized quicksort for container pointer arrays
void quicksort_containers_desc_ptrs(Container **containers, size_t low, size_t high)
{
    while (low < high)
    {
        if (high - low < 16)
        {
            for (size_t i = low + 1; i <= high; ++i)
            {
                size_t card_i = get_container_cardinality(containers[i]);
                int j = i - 1;
                Container *temp = containers[i];

                while (j >= (int)low && get_container_cardinality(containers[j]) < card_i)
                {
                    containers[j + 1] = containers[j];
                    j--;
                }
                containers[j + 1] = temp;
            }
            break;
        }

        PartitionResult part = partition_three_way_ptrs(containers, low, high);

        if (part.left_end - low < high - part.right_start)
        {
            quicksort_containers_desc_ptrs(containers, low, part.left_end - 1);
            low = part.right_start;
        }
        else
        {
            quicksort_containers_desc_ptrs(containers, part.right_start, high);
            high = part.left_end - 1;
        }
    }
}

// Public interface: Sort array of container pointers in descending order by cardinality
void roaring_bitmap_sort_containers_array(Container **Q, size_t Q_size)
{
    if (!Q || Q_size <= 1)
        return;

    quicksort_containers_desc_ptrs(Q, 0, Q_size - 1);
}

extern "C" RoaringBitmap *roaring_bitmap_intersect(RoaringBitmap *bm1, RoaringBitmap *bm2)
{
    if (!bm1 || !bm2)
        return nullptr;

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

            // Create result container (bitmap type)
            Container *resultC = create_container(result, key1, BITMAP_CONTAINER);
            if (!resultC)
            {
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

            i1++;
            i2++;
        }
    }

    // If small result, convert to array containers
    if (total_cardinality <= 4096)
    {
        RoaringBitmap *finalResult = roaring_bitmap_create(100 * 1024 * 1024, result->num_containers);

        for (size_t i = 0; i < result->num_containers; ++i)
        {
            Container *srcC = &result->containers[i];
            Container *destC = create_container(finalResult, srcC->key, ARRAY_CONTAINER);
            if (!destC)
            {
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

extern "C" RoaringBitmap *roaring_bitmap_union(RoaringBitmap **bitmaps, size_t count)
{
    if (!bitmaps || count == 0)
        return nullptr;

    RoaringBitmap *T = roaring_bitmap_create(100 * 1024 * 1024, 16);
    ContainerMinHeap *P = build_container_heap(bitmaps, count);

    while (!P->empty())
    {
        ContainerHeapNode x_h = P->pop();
        Container *x = x_h.container;
        uint16_t current_key = x->key;

        std::vector<Container *> Q_vec;
        std::vector<ContainerHeapNode> popped_nodes;

        Q_vec.push_back(x);
        popped_nodes.push_back(x_h);

        while (!P->empty() && P->top().container->key == current_key)
        {
            ContainerHeapNode next_h = P->pop();
            Q_vec.push_back(next_h.container);
            popped_nodes.push_back(next_h);
        }

        for (const auto &node : popped_nodes)
        {
            advance_and_reinsert(P, bitmaps, node);
        }

        size_t Q_size = Q_vec.size();
        Container **Q = new Container *[Q_size];
        for (size_t i = 0; i < Q_size; ++i)
        {
            Q[i] = Q_vec[i];
        }
        roaring_bitmap_sort_containers_array(Q, Q_size);

        // ========================================
        // FIXED: Allocate container directly in T's array
        // ========================================
        if (T->num_containers >= T->max_containers)
        {
            size_t new_capacity = T->max_containers * 2;
            Container *new_array = reinterpret_cast<Container *>(T->arena.alloc(sizeof(Container) * new_capacity));
            if (!new_array)
            {
                delete[] Q;
                return nullptr;
            }
            std::memcpy(new_array, T->containers, sizeof(Container) * T->num_containers);
            T->containers = new_array;
            T->max_containers = new_capacity;
        }

        size_t insert_pos = find_insert_position(T, current_key);
        if (insert_pos < T->num_containers)
        {
            std::memmove(
                &T->containers[insert_pos + 1],
                &T->containers[insert_pos],
                sizeof(Container) * (T->num_containers - insert_pos));
        }

        // Work directly on the container in T's array
        Container *A = &T->containers[insert_pos];
        T->num_containers++;

        A->key = current_key;

        bool start_as_bitmap = (Q[0]->type == BITMAP_CONTAINER) ||
                               (Q[0]->type == ARRAY_CONTAINER && Q[0]->array.cardinality > THRESHOLD);

        if (start_as_bitmap)
        {
            // Start as bitmap container
            A->type = BITMAP_CONTAINER;
            A->bitmap.bits = T->arena.alloc(8192);

            // *** CRITICAL FIX: Check for allocation failure ***
            if (!A->bitmap.bits)
            {
                delete[] Q;
                delete P;
                // fprintf(stderr, "ERROR: Failed to allocate bitmap bits for union - arena out of memory\n");
                // fprintf(stderr, "Arena usage: %zu / %zu bytes\n", T->arena.offset, T->arena.capacity);
                return nullptr;
            }

            // *** CRITICAL FIX: Initialize bitmap to zero before ORing ***
            std::memset(A->bitmap.bits, 0, 8192);

            // Step 1: Convert all containers to bitmap format in parallel
            std::vector<uint8_t *> temp_bitmaps(Q_size);
            std::vector<bool> needs_cleanup(Q_size, false);

#pragma omp parallel for schedule(dynamic)
            for (size_t i = 0; i < Q_size; ++i)
            {
                if (Q[i]->type == BITMAP_CONTAINER)
                {
                    temp_bitmaps[i] = Q[i]->bitmap.bits;
                    needs_cleanup[i] = false;
                }
                else
                {
                    // Array container - convert to bitmap
                    uint8_t *temp = new uint8_t[8192];
                    std::memset(temp, 0, 8192);
                    for (size_t j = 0; j < Q[i]->array.cardinality; ++j)
                    {
                        uint16_t v = Q[i]->array.values[j];
                        temp[v >> 3] |= (1u << (v & 7));
                    }
                    temp_bitmaps[i] = temp;
                    needs_cleanup[i] = true;
                }
            }

            // Step 2: Parallel reduction - OR all bitmaps word by word
#pragma omp parallel for schedule(static)
            for (size_t word_idx = 0; word_idx < 256; ++word_idx)
            {
                __m256i result = _mm256_loadu_si256((__m256i const *)(temp_bitmaps[0] + word_idx * 32));

                for (size_t i = 1; i < Q_size; ++i)
                {
                    __m256i vec = _mm256_loadu_si256((__m256i const *)(temp_bitmaps[i] + word_idx * 32));
                    result = _mm256_or_si256(result, vec);
                }

                _mm256_storeu_si256((__m256i *)(A->bitmap.bits + word_idx * 32), result);
            }

            // Step 3: Clean up temporary bitmaps
            for (size_t i = 0; i < Q_size; ++i)
            {
                if (needs_cleanup[i])
                {
                    delete[] temp_bitmaps[i];
                }
            }
        }
        else // Array container path
        {
            A->type = ARRAY_CONTAINER;
            A->array.capacity = Q[0]->array.capacity;
            A->array.cardinality = Q[0]->array.cardinality;
            A->array.values = reinterpret_cast<uint16_t *>(T->arena.alloc(sizeof(uint16_t) * A->array.capacity));

            // *** CRITICAL FIX: Check array allocation ***
            if (!A->array.values)
            {
                delete[] Q;
                delete P;
                fprintf(stderr, "ERROR: Failed to allocate array values for union\n");
                return nullptr;
            }

            std::memcpy(A->array.values, Q[0]->array.values, sizeof(uint16_t) * A->array.cardinality);

            size_t i = 1;
            while (i < Q_size && A->type == ARRAY_CONTAINER)
            {
                if (Q[i]->type == ARRAY_CONTAINER)
                {
                    for (size_t j = 0; j < Q[i]->array.cardinality; ++j)
                    {
                        add_to_array(T, A, Q[i]->array.values[j]);
                    }

                    if (A->array.cardinality > THRESHOLD)
                    {
                        promote_to_bitmap(T, A);
                        break;
                    }
                }
                else
                {
                    promote_to_bitmap(T, A);
                    break;
                }
                i++;
            }

            if (A->type == BITMAP_CONTAINER)
            {
                for (; i < Q_size; ++i)
                {
                    if (Q[i]->type == BITMAP_CONTAINER)
                    {
                        for (size_t word_idx = 0; word_idx < 256; ++word_idx)
                        {
                            __m256i vecA = _mm256_loadu_si256((__m256i const *)(A->bitmap.bits + word_idx * 32));
                            __m256i vecQ = _mm256_loadu_si256((__m256i const *)(Q[i]->bitmap.bits + word_idx * 32));
                            __m256i vecResult = _mm256_or_si256(vecA, vecQ);
                            _mm256_storeu_si256((__m256i *)(A->bitmap.bits + word_idx * 32), vecResult);
                        }
                    }
                    else
                    {
                        for (size_t j = 0; j < Q[i]->array.cardinality; ++j)
                        {
                            set_bit(A->bitmap, Q[i]->array.values[j]);
                        }
                    }
                }
            }
        }

        // NO add_container call - A is already in T->containers[]!
        delete[] Q;
    }

    delete P;
    return T;
}

// extern "C" RoaringBitmap *roaring_bitmap_union(RoaringBitmap **bitmaps, size_t count)
// {
//     if (!bitmaps || count == 0)
//         return nullptr;

//     RoaringBitmap *T = roaring_bitmap_create(100 * 1024 * 1024, 16);
//     ContainerMinHeap *P = build_container_heap(bitmaps, count);

//     while (!P->empty())
//     {
//         // Step 6: Remove ALL elements with the same key as root
//         ContainerHeapNode x_h = P->pop();
//         Container *x = x_h.container;
//         uint16_t current_key = x->key;

//         // Collect all containers with the same key (this is Q from the algorithm)
//         std::vector<Container *> Q_vec;
//         std::vector<ContainerHeapNode> popped_nodes; // Track which nodes we popped

//         Q_vec.push_back(x);
//         popped_nodes.push_back(x_h);

//         // Remove ALL remaining containers with same key from heap
//         while (!P->empty() && P->top().container->key == current_key)
//         {
//             ContainerHeapNode next_h = P->pop();
//             Q_vec.push_back(next_h.container);
//             popped_nodes.push_back(next_h);
//         }

//         // advance all the bitmaps that contributed containers
//         for (const auto &node : popped_nodes)
//         {
//             advance_and_reinsert(P, bitmaps, node);
//         }

//         // Step 7: Sort Q by descending cardinality
//         size_t Q_size = Q_vec.size();
//         Container **Q = new Container *[Q_size];
//         for (size_t i = 0; i < Q_size; ++i)
//         {
//             Q[i] = Q_vec[i];
//         }
//         roaring_bitmap_sort_containers_array(Q, Q_size);

//         // Step 8: Clone Q_1 and call the result A
//         Container *A = reinterpret_cast<Container *>(T->arena.alloc(sizeof(Container)));
//         A->key = current_key;

//         // Determine if we should start with bitmap or array based on largest container
//         bool start_as_bitmap = (Q[0]->type == BITMAP_CONTAINER) ||
//                                (Q[0]->type == ARRAY_CONTAINER && Q[0]->array.cardinality > THRESHOLD);

//         // If A is bitmap, union all Q containers in parallel
//         if (start_as_bitmap)
//         {
//             // Start as bitmap container
//             A->type = BITMAP_CONTAINER;
//             A->bitmap.bits = T->arena.alloc(8192);

//             // Step 1: Convert all containers to bitmap format in parallel
//             std::vector<uint8_t *> temp_bitmaps(Q_size);
//             std::vector<bool> needs_cleanup(Q_size, false);

// #pragma omp parallel for schedule(dynamic)
//             for (size_t i = 0; i < Q_size; ++i)
//             {
//                 if (Q[i]->type == BITMAP_CONTAINER)
//                 {
//                     // Already a bitmap, just reference it
//                     temp_bitmaps[i] = Q[i]->bitmap.bits;
//                     needs_cleanup[i] = false;
//                 }
//                 else
//                 {
//                     // Array container - convert to bitmap
//                     uint8_t *temp = new uint8_t[8192];
//                     std::memset(temp, 0, 8192);
//                     for (size_t j = 0; j < Q[i]->array.cardinality; ++j)
//                     {
//                         uint16_t v = Q[i]->array.values[j];
//                         temp[v >> 3] |= (1u << (v & 7));
//                     }
//                     temp_bitmaps[i] = temp;
//                     needs_cleanup[i] = true;
//                 }
//             }

// // Step 2: Parallel reduction - OR all bitmaps word by word
// #pragma omp parallel for schedule(static)
//             for (size_t word_idx = 0; word_idx < 256; ++word_idx)
//             {
//                 __m256i result = _mm256_loadu_si256((__m256i const *)(temp_bitmaps[0] + word_idx * 32));

//                 for (size_t i = 1; i < Q_size; ++i)
//                 {
//                     __m256i vec = _mm256_loadu_si256((__m256i const *)(temp_bitmaps[i] + word_idx * 32));
//                     result = _mm256_or_si256(result, vec);
//                 }

//                 _mm256_storeu_si256((__m256i *)(A->bitmap.bits + word_idx * 32), result);
//             }

//             // Step 3: Clean up temporary bitmaps
//             for (size_t i = 0; i < Q_size; ++i)
//             {
//                 if (needs_cleanup[i])
//                 {
//                     delete[] temp_bitmaps[i];
//                 }
//             }
//         }
//         else
//         {
//             // Steps 12-13: A is an array container
//             A->type = ARRAY_CONTAINER;
//             A->array.capacity = Q[0]->array.capacity;
//             A->array.cardinality = Q[0]->array.cardinality;
//             A->array.values = reinterpret_cast<uint16_t *>(T->arena.alloc(sizeof(uint16_t) * A->array.capacity));
//             std::memcpy(A->array.values, Q[0]->array.values, sizeof(uint16_t) * A->array.cardinality);

//             // Step 13: Compute union with remaining array containers
//             size_t i = 1;
//             while (i < Q_size && A->type == ARRAY_CONTAINER)
//             {
//                 if (Q[i]->type == ARRAY_CONTAINER)
//                 {
//                     // Merge sorted arrays
//                     for (size_t j = 0; j < Q[i]->array.cardinality; ++j)
//                     {
//                         add_to_array(T, A, Q[i]->array.values[j]);
//                     }

//                     // If A exceeds cardinality of 4096, promote to bitmap
//                     if (A->array.cardinality > THRESHOLD)
//                     {
//                         promote_to_bitmap(T, A);
//                         break;
//                     }
//                 }
//                 else
//                 {
//                     // Q[i] is bitmap, we must promote
//                     promote_to_bitmap(T, A);
//                     break;
//                 }
//                 i++;
//             }

//             // If promoted to bitmap during the loop, handle remaining containers
//             if (A->type == BITMAP_CONTAINER)
//             {
//                 for (; i < Q_size; ++i)
//                 {
//                     if (Q[i]->type == BITMAP_CONTAINER)
//                     {
//                         for (size_t word_idx = 0; word_idx < 256; ++word_idx)
//                         {
//                             __m256i vecA = _mm256_loadu_si256((__m256i const *)(A->bitmap.bits + word_idx * 32));
//                             __m256i vecQ = _mm256_loadu_si256((__m256i const *)(Q[i]->bitmap.bits + word_idx * 32));
//                             __m256i vecResult = _mm256_or_si256(vecA, vecQ);
//                             _mm256_storeu_si256((__m256i *)(A->bitmap.bits + word_idx * 32), vecResult);
//                         }
//                     }
//                     else
//                     {
//                         for (size_t j = 0; j < Q[i]->array.cardinality; ++j)
//                         {
//                             set_bit(A->bitmap, Q[i]->array.values[j]);
//                         }
//                     }
//                 }
//             }
//         }

//         // Step 15: Add A to the output
//         add_container(T, A);
//         delete[] Q;
//     }

//     delete P;
//     return T;
// }

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

    // Read number of containers
    size_t num_containers = *reinterpret_cast<const size_t *>(ptr);
    ptr += sizeof(size_t);

    RoaringBitmap *bm = new RoaringBitmap();
    bm->num_containers = num_containers;
    bm->max_containers = num_containers;

    size_t arena_size = 100 * 1024 * 1024; // 100MB default

    // Initialize arena with chaining
    ArenaBlock *first_block = new ArenaBlock();
    first_block->buffer = new uint8_t[arena_size];
    first_block->capacity = arena_size;
    first_block->offset = 0;
    first_block->next = nullptr;

    bm->arena.head = first_block;
    bm->arena.current = first_block;
    bm->arena.default_block_size = arena_size;

    // Allocate container array
    bm->containers = reinterpret_cast<Container *>(bm->arena.alloc(sizeof(Container) * num_containers));
    if (!bm->containers)
    {
        fprintf(stderr, "ERROR: Failed to allocate containers in roaring_from_serialized\n");
        bm->arena.free_all();
        delete bm;
        return nullptr;
    }

    std::memset(bm->containers, 0, sizeof(Container) * num_containers);

    for (size_t i = 0; i < num_containers; ++i)
    {
        Container &c = bm->containers[i];

        // Read key
        c.key = *reinterpret_cast<const uint16_t *>(ptr);
        ptr += sizeof(uint16_t);

        // Read type
        c.type = static_cast<ContainerType>(*reinterpret_cast<const uint8_t *>(ptr));
        ptr += sizeof(uint8_t);

        if (c.type == BITMAP_CONTAINER)
        {
            c.bitmap.bits = bm->arena.alloc(8192);
            if (!c.bitmap.bits)
            {
                fprintf(stderr, "ERROR: Failed to allocate bitmap container\n");
                bm->arena.free_all();
                delete bm;
                return nullptr;
            }
            std::memcpy(c.bitmap.bits, ptr, 8192);
            ptr += 8192;
        }
        else if (c.type == ARRAY_CONTAINER)
        {
            // Read array cardinality
            c.array.cardinality = *reinterpret_cast<const size_t *>(ptr);
            ptr += sizeof(size_t);

            c.array.capacity = c.array.cardinality;
            c.array.values = reinterpret_cast<uint16_t *>(bm->arena.alloc(sizeof(uint16_t) * c.array.capacity));
            if (!c.array.values)
            {
                fprintf(stderr, "ERROR: Failed to allocate array container\n");
                bm->arena.free_all();
                delete bm;
                return nullptr;
            }

            std::memcpy(c.array.values, ptr, c.array.cardinality * sizeof(uint16_t));
            ptr += c.array.cardinality * sizeof(uint16_t);
        }
        else
        {
            // Unknown container type
            fprintf(stderr, "ERROR: Unknown container type\n");
            bm->arena.free_all();
            delete bm;
            return nullptr;
        }
    }

    return reinterpret_cast<uint8_t *>(bm);
}

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