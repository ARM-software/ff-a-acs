/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_LIBC_H_
#define _PAL_LIBC_H_

#include "pal.h"
#include "pal_config_def.h"
#include "pal_memory.h"
#include "pal_endpoint_info.h"

#define __assert(e) ({ \
                  pal_printf("ASSERT: ", 0, 0); \
                  pal_printf(__FILE__, 0, 0); \
                  pal_printf(" ,line:%d) ", __LINE__, 0); \
                  pal_printf(#e, 0, 0); \
                  pal_printf("\n", 0, 0); \
                  pal_terminate_simulation(); \
                })

#define assert(e)   ((e) ? (void)0 : __assert(e))

/**
 * @brief        - Compare the two input buffer content
 * @param        - src: Source buffer to be compared
 * @dest         - dest: Destination buffer to be compared

 * @return       - Zero if buffer content are equal, else non-zero
**/
int pal_memcmp(void *src, void *dest, size_t len);

/**
 * @brief        - Fill a buffer with a known specified input value
 * @param        - dst: Pointer to the buffer to fill
 * @param        - value: Value to fill buffer with
 * @param        - count: Number of bytes in buffer to fill
 * @return       - dst
**/
void *pal_memset(void *dst, int val, size_t count);

/**
 * @brief        - Copy src buffer content into dst
 * @param        - dst: Pointer to the destination buffer
 * @param        - src: Pointer to the source buffer
 * @param        - len: Number of bytes in buffer to copy
 * @return       - dst
**/
void *pal_memcpy(void *dst, const void *src, size_t len);

/**
 * @brief - Allocates requested buffer size in bytes in a contiguous memory.
 *
 * @param  size - allocation size in bytes
 * @return - Returns pointer to allocated memory/NULL if it fails.
**/
void *pal_memory_alloc(uint64_t size);

/**
 * @brief - Free the allocated memory.
 *
 * @param address - Buffer the base address of the memory range to be freed.
 * @return - Returns success/error status code.
**/
uint32_t pal_memory_free(void *address, uint64_t size);
#endif /* _PAL_LIBC_H_ */
