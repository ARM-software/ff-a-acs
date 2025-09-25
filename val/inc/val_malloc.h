/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MALLOCLIB_H
#define MALLOCLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocates a block of memory of the specified size.
 *
 * @param size  Number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 *         Memory is uninitialized.
 */
void *val_malloc(size_t size);

/**
 * @brief Allocates memory for an array of elements and initializes it to zero.
 *
 * @param nmemb  Number of elements.
 * @param size   Size of each element in bytes.
 * @return Pointer to the allocated and zero-initialized memory,
 *         or NULL if allocation fails.
 */
void *val_calloc(size_t nmemb, size_t size);

/**
 * @brief Resizes the memory block pointed to by ptr to the new size.
 *
 * @param ptr   Pointer to previously allocated memory. If NULL, behaves like val_malloc().
 * @param size  New size in bytes. If 0, behaves like val_free().
 * @return Pointer to the reallocated memory, or NULL if reallocation fails.
 *         Old memory contents are preserved up to the minimum of old and new size.
 */
void *val_realloc(void *ptr, size_t size);

/**
 * @brief Frees the memory block pointed to by ptr.
 *
 * @param ptr  Pointer to memory previously allocated by val_malloc, val_calloc, val_realloc,
 *             or val_aligned_alloc. If NULL, no operation is performed.
 * @return VAL_SUCCESS/VAL_ERROR.
 */
size_t val_free(void *ptr);

/**
 * @brief Allocates memory with the specified alignment.
 *
 * @param alignment  Alignment in bytes (must be a power of two and at least sizeof(void*)).
 * @param size       Number of bytes to allocate (must be a multiple of alignment).
 * @return Pointer to aligned memory block, or NULL if allocation fails.
 */
void *val_aligned_alloc(size_t alignment, size_t size);

/**
 * @brief Prints the current state of the heap and all memory blocks for debugging purposes.
 *        Shows block addresses, sizes, free/used status, and magic numbers.
 */
void val_malloclib_visualize(void);

int malloc_test_main(void);

#ifdef __cplusplus
}
#endif

#endif // MALLOCLIB_H
