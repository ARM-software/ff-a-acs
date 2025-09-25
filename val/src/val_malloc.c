/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val.h"
#include "val_malloc.h"
#include "val_misc.h"

#define BLOCK_VALID_MAGIC       0xABCD
#define BLOCK_INVALID_MAGIC     0xFEFE


#define ALIGNMENT 8
#define ALIGN(size) (((uintptr_t)(size) + (ALIGNMENT - 1)) & ~((uintptr_t)(ALIGNMENT - 1)))
#define ALIGN_TO(size, alignment) \
    (((uintptr_t)(size) + ((uintptr_t)(alignment) - 1)) & ~((uintptr_t)(alignment) - 1))

typedef struct Block {
    uint16_t magic;
    uint16_t free;
    size_t size;
    struct Block *next;
    struct Block *prev;
} Block;

static Block *free_list;
static int initialized;

#define BLOCK_SIZE ALIGN(sizeof(Block))

/**
 * @brief Initializes the heap for the custom memory allocator.
 *
 * This function sets up the initial heap region used by the `val_malloc` allocator.
 * It retrieves the heap buffer from the platform abstraction layer (`pal_get_heap_buffer()`),
 * ensures it is 32-byte aligned, and initializes a single free block representing
 * the entire heap. This block is marked as free and becomes the head of the free list.
 *
 * Preconditions:
 * - `pal_get_heap_buffer()` must return a valid, aligned memory buffer of size
 *   at least `PLATFORM_HEAP_BUF_SIZE`.
 *
 * Postconditions:
 * - The `free_list` points to one large free block.
 * - The heap is marked as initialized.
 */
static void val_init_heap(void)
{
    free_list = (Block *)((uintptr_t)pal_get_heap_buffer());
    assert(((uintptr_t)free_list % 32) == 0);
    free_list->magic = BLOCK_VALID_MAGIC;
    free_list->free = 1;
    free_list->size = PLATFORM_HEAP_BUF_SIZE - BLOCK_SIZE;
    free_list->next = NULL;
    free_list->prev = NULL;
    initialized = 1;
}


/**
 * @brief Coalesces adjacent free blocks in the heap to reduce fragmentation.
 *
 * This function merges a given free block with any adjacent free blocks in
 * both forward and backward directions. Coalescing reduces fragmentation by
 * combining neighboring free blocks into a single larger free block.
 *
 * - Forward coalescing merges the block with subsequent free blocks.
 * - Backward coalescing merges the block with preceding free blocks.
 * - Invalidated (merged) blocks are marked with `BLOCK_INVALID_MAGIC`.
 *
 * @param block_ptr A pointer to the pointer of the block to coalesce. The
 *        pointer may be updated to point to the resulting merged block after
 *        coalescing.
 */

/*
 * Coalesce adjacent free blocks
 *
 * Initial state:
 *
 *   +--------+     +--------+     +--------+
 *   | BlockA | <-> | BlockB | <-> | BlockC |
 *   |  free  |     |  free  |     |  free  |
 *   +--------+     +--------+     +--------+
 *      ^              ^              ^
 *      |              |              |
 *    block->prev    block         block->next
 *
 * Step 1: Merge block with next if next is free
 *
 *   block->size += BLOCK_SIZE + block->next->size
 *   block->next = block->next->next
 *   if (block->next)
 *       block->next->prev = block
 *
 * After Step 1:
 *
 *   +--------+     +------------------------+
 *   | BlockA | <-> |   BlockB (merged C)    |
 *   |  free  |     |        free            |
 *   +--------+     +------------------------+
 *      ^                    ^
 *      |                    |
 *  block->prev           block
 *
 * Step 2: Merge with prev if prev is free
 *
 *   block->prev->size += BLOCK_SIZE + block->size
 *   block->prev->next = block->next
 *   if (block->next)
 *       block->next->prev = block->prev
 *
 * Final state:
 *
 *   +----------------------------------------+
 *   |     BlockA (merged B and C)            |
 *   |                free                    |
 *   +----------------------------------------+
 *      ^
 *      |
 *  block->prev
 */

static void val_coalesce(Block **block_ptr)
{
    Block *block = *block_ptr;

    // Forward coalescing
    while (block->next && block->next->free) {
        Block *to_remove = block->next;

        block->size += BLOCK_SIZE + to_remove->size;
        block->next = to_remove->next;
        to_remove->magic = BLOCK_INVALID_MAGIC;

        if (block->next)
            block->next->prev = block;
    }

    // Backward coalescing
    while (block->prev && block->prev->free) {
        Block *to_remove = block;

        block->prev->size += BLOCK_SIZE + to_remove->size;
        block->prev->next = to_remove->next;
        to_remove->magic = BLOCK_INVALID_MAGIC;

        if (to_remove->next)
            to_remove->next->prev = block->prev;

        block = block->prev;
    }

    *block_ptr = block;
}

/**
 * @brief Splits a larger free block into an allocated block and a new free block.
 *
 * This function checks whether a given free block is large enough to be split into:
 * - An allocated block of the requested `size`, and
 * - A new free block using the remaining space.
 *
 * If splitting is possible, it:
 * - Creates a new `Block` structure immediately after the allocated block.
 * - Initializes the new block as free with updated size and links.
 * - Updates the original block's size and links accordingly.
 *
 * @param block Pointer to the free block to be split.
 * @param size The requested size for the allocation (excluding metadata).
 */

/*
 * Split a large free block into two parts:
 *
 * Condition:
 *   Only split if block is large enough to hold
 *   the requested size, metadata, and ALIGNMENT padding.
 *
 * Initial state:
 *
 *   +---------------------------+
 *   |        block (free)       |
 *   |        large size         |
 *   +---------------------------+
 *        ^
 *        |
 *      block
 *
 * After split:
 *
 *   +---------------+ +---------------------+
 *   |  block (used) | |   new_block (free)  |
 *   |   size = sz   | | size = old - sz - OH|
 *   +---------------+ +---------------------+
 *        ^                    ^
 *        |                    |
 *      block              new_block
 *
 */

static void val_split(Block *block, size_t size)
{
    if (block->size >= size + BLOCK_SIZE + ALIGNMENT) {
        Block *new_block = (Block *)((uintptr_t)block + BLOCK_SIZE + size);

        new_block->magic = BLOCK_VALID_MAGIC;
        new_block->free = 1;
        new_block->size = block->size - size - BLOCK_SIZE;
        new_block->next = block->next;
        new_block->prev = block;
        if (new_block->next)
            new_block->next->prev = new_block;

        block->next = new_block;
        block->size = size;
    }
}

/**
 * @brief Allocates a block of memory from the custom heap.
 *
 * This function searches the free list for a suitable free block that can
 * accommodate the requested size. If a suitable block is found:
 * - The block is split if there's enough excess space.
 * - The block is marked as allocated.
 * - A pointer to the usable memory region (just after the block header) is returned.
 *
 * If the heap is not yet initialized, it will be initialized before allocation.
 * If no suitable block is found, the function returns NULL.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */

void *val_malloc(size_t size)
{

    if (!size)
        return NULL;

    if (!initialized)
        val_init_heap();

    size = ALIGN(size);
    for (Block *block = free_list; block; block = block->next) {
        if (block->free && block->size >= size) {
            val_split(block, size);
            block->magic = BLOCK_VALID_MAGIC;
            block->free  = 0;
            return (uint8_t *)block + BLOCK_SIZE;
        }
    }
    return NULL;
}

/**
 * @brief Frees a previously allocated block of memory.
 *
 * This function marks the block corresponding to the given pointer as free and
 * attempts to coalesce it with adjacent free blocks to reduce fragmentation.
 *
 * It performs several safety checks:
 * - Ignores NULL pointers.
 * - Validates the block's magic number to detect corruption or invalid frees.
 * - Detects and logs double free attempts.
 *
 * @param ptr Pointer to the memory block to be freed. Must have been returned
 *            by a previous call to `val_malloc`.
 * @return VAL_SUCCESS on successful free, VAL_ERROR if the pointer is invalid
 *         or the block is already freed.
 */

size_t val_free(void *ptr)
{
    if (!ptr)
        return VAL_ERROR;

    Block *block = (Block *)((uintptr_t)ptr - BLOCK_SIZE);

    if (block->magic != BLOCK_VALID_MAGIC) {
        LOG(ERROR, "Invalid Block at %p", block);
        return VAL_ERROR;
    }

    if (block->free) {
        LOG(ERROR, "Double free detected for block %p", block);
        return VAL_ERROR;
    }

    block->free = 1;
    val_coalesce(&block);
    return VAL_SUCCESS;
}

/**
 * @brief Allocates zero-initialized memory for an array.
 *
 * This function allocates memory for an array of `nmemb` elements, each of
 * `size` bytes, and initializes the entire block to zero. It behaves similarly
 * to the standard `calloc` function.
 *
 * Internally, it calls `val_malloc` to allocate the memory and `val_memset`
 * to zero out the allocated region.
 *
 * @param nmemb Number of elements to allocate.
 * @param size  Size of each element in bytes.
 * @return Pointer to the allocated and zero-initialized memory, or NULL on failure.
 */

void *val_calloc(size_t nmemb, size_t size)
{
    if (!size || !nmemb)
        return NULL;

    size_t total = nmemb * size;
    void *ptr = val_malloc(total);

    if (ptr)
        val_memset(ptr, 0, total);

    return ptr;
}

/**
 * @brief Reallocates a previously allocated memory block to a new size.
 *
 * This function resizes the memory block pointed to by `ptr` to `size` bytes.
 * It mimics the behavior of the standard `realloc` function, with the following behavior:
 *
 * - If `ptr` is NULL, it behaves like `val_malloc(size)`.
 * - If `size` is 0, it behaves like `val_free(ptr)` and returns NULL.
 * - If the current block is large enough, it may be split in place.
 * - If adjacent free blocks are available, it attempts to coalesce and expand in place.
 * - Otherwise, a new block is allocated, the data is copied, and the old block is freed.
 *
 * Safety checks include:
 * - Validation of the block's magic number.
 * - Proper alignment and handling of memory movement.
 *
 * @param ptr  Pointer to the previously allocated memory block.
 * @param size New size in bytes.
 * @return Pointer to the reallocated memory, or NULL on failure.
 */

void *val_realloc(void *ptr, size_t size)
{
    if (!ptr)
        return val_malloc(size);

    if (size == 0) {
        val_free(ptr);
        return NULL;
    }

    Block *block = (Block *)((uintptr_t)ptr - BLOCK_SIZE);

    if (block->magic != BLOCK_VALID_MAGIC) {
        LOG(ERROR, "Invalid Block at %p", block);
        return NULL;
    }

    size = ALIGN(size);
    size_t original_size = block->size;

    if (block->size >= size) {
        val_split(block, size);
        val_coalesce(&block->next);
        return ptr;
    }

    if ((block->next && block->next->free &&
       ((block->size + BLOCK_SIZE + block->next->size) >= size)) ||
       (block->prev && block->prev->free &&
       ((block->size + BLOCK_SIZE + block->prev->size) >= size)))
    {
        val_coalesce(&block);
        void *new_ptr = (uint8_t *)block + BLOCK_SIZE;

        if (new_ptr != ptr)
            val_memmove(new_ptr, ptr, original_size);

        val_split(block, size);
        block->free = 0;
        return new_ptr;
    }

    void *new_ptr = val_malloc(size);

    if (!new_ptr)
        return NULL;

    val_memcpy(new_ptr, ptr, block->size);
    val_free(ptr);
    return new_ptr;
}

/**
 * @brief Allocates memory with a specified alignment and size.
 *
 * This function allocates `size` bytes of memory aligned to `alignment` bytes,
 * mimicking the behavior of the standard `aligned_alloc` function. It ensures
 * that the returned pointer is aligned as requested and the size is a multiple
 * of the alignment.
 *
 * The allocator scans the free list for a block large enough to satisfy both the
 * alignment and size requirements. It may split the block into:
 * - A leading block to discard misaligned space (if large enough), and
 * - A trailing block if extra space remains after the aligned allocation.
 *
 * Safety checks include:
 * - Ensuring alignment is a power of two and at least the size of a pointer.
 * - Ensuring size is a multiple of the alignment.
 * - Ensuring the heap is initialized before allocation.
 *
 * @param alignment The desired memory alignment (power of two and greater/equal to sizeof(void*)).
 * @param size      The number of bytes to allocate (must be a multiple of `alignment`).
 * @return Pointer to the aligned allocated memory, or NULL on failure.
 */

void *val_aligned_alloc(size_t alignment, size_t size)
{
    if (alignment < sizeof(void *) || (alignment & (alignment - 1)) != 0)
        return NULL;

    if (size % alignment != 0)
        return NULL;

    size = ALIGN(size);

    if (!initialized)
        val_init_heap();

    for (Block *block = free_list; block; block = block->next) {

        if (!block->free)
            continue;

        uintptr_t raw = (uintptr_t)block;
        uintptr_t aligned = ALIGN_TO(raw + BLOCK_SIZE, alignment);
        uintptr_t header_addr = aligned - BLOCK_SIZE;
        size_t total_needed = aligned + size - raw;

        if (block->size + BLOCK_SIZE >= total_needed) {
            // Split leading free space if large enough
            intptr_t diff = (intptr_t)header_addr - (intptr_t)raw;
            size_t leading_space = diff > 0 ? (size_t)diff : 0;

            if (leading_space >= BLOCK_SIZE) {
                val_split(block, leading_space - BLOCK_SIZE);
                block = block->next;
            }
            // Split trailing space after aligned block if large enough
            if (block->size > size + BLOCK_SIZE + ALIGNMENT)
                val_split(block, size);

            block->size = size;
            block->free = 0;
            return (void *)(header_addr + BLOCK_SIZE);  // aligned pointer
        }
    }
    return NULL;
}

/**
 * @brief Prints a visual representation of the current heap layout.
 *
 * This diagnostic function traverses the entire heap and logs information
 * about each block, including:
 * - Block index
 * - Block address
 * - Size
 * - Allocation status (FREE or USED)
 * - Magic number for integrity verification
 *
 * It helps developers understand how memory is currently organized, detect
 * fragmentation, and verify allocator integrity.
 *
 * If the heap is not initialized, it logs a message and exits early.
 *
 * This function is intended for debugging and visualization purposes only.
 */

void val_malloclib_visualize(void)
{
    int index = 0;
    uint8_t *heap = (uint8_t *)((uintptr_t)pal_get_heap_buffer());

    LOG(INFO, "[Heap Visualization] initialized %d", initialized);
    if (!initialized) {
        LOG(INFO, "Heap not initialized.");
        return;
    }

    Block *block = (Block *)((uintptr_t)heap);

    while ((uint8_t *)block < heap + PLATFORM_HEAP_BUF_SIZE) {
        LOG(INFO, "Block %02d: %p | Size: %6zu | %s | Magic: 0x%04X",
               index,
               (void *)block,
               block->size,
               block->free ? "FREE" : "USED",
               block->magic);

        if ((uint8_t *)block + BLOCK_SIZE + block->size >= heap + PLATFORM_HEAP_BUF_SIZE)
            break;

        block = (Block *)((uintptr_t)block + BLOCK_SIZE + block->size);
        index++;
    }
    LOG(INFO, "[End of Heap]\n");
}
