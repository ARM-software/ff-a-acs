/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"
#include "pal_kernel_helpers.h"
#include <string.h>

#define BUFFER_COUNT 5
uint32_t is_buffer_in_use[BUFFER_COUNT] = {0};

int pal_memcmp(void *src, void *dest, size_t len)
{
    return memcmp(src, dest, len);
}

void *pal_memset(void *dst, int val, size_t count)
{
    memset(dst, val, count);
    return dst;
}

void *pal_memcpy(void *dst, const void *src, size_t len)
{
    return memcpy(dst, src, len);
}

uint32_t pal_terminate_simulation(void)
{
    return PAL_SUCCESS;
}

void *pal_memory_alloc(uint64_t size)
{
    int i;
    uint8_t (*pal_buffer_4k)[PAGE_SIZE_4K] = mem_pool_get_virt(pal_get_acs_drv_fd());

    if (size == PAGE_SIZE_4K)
    {
        for (i = 0; i < BUFFER_COUNT ; i++)
        {
            if (!is_buffer_in_use[i])
            {
                is_buffer_in_use[i] = 1;
                PAL_LOG(DBG, "pal_memory_alloc %lx", (uint64_t)&pal_buffer_4k[i][0]);
                return &pal_buffer_4k[i][0];
            }
        }
    }
    else if (size == PAGE_SIZE_4K * 2)
    {
        for (i = 0; i < BUFFER_COUNT ; i++)
        {
            if ((!is_buffer_in_use[i]) && (i != BUFFER_COUNT - 1))
            {
                is_buffer_in_use[i] = 1;
                is_buffer_in_use[i+1] = 1;
                PAL_LOG(DBG, "pal_memory_alloc %lx", (uint64_t)&pal_buffer_4k[i][0]);
                return &pal_buffer_4k[i][0];
            }
        }
    }
    else
    {
        /* Need to add logic for 16K and 64K pages */
        PAL_LOG(ERROR, "val_memory_alloc failed");
    }

    return NULL;
}

uint32_t pal_memory_free(void *address, uint64_t size)
{
    int i;
    uint8_t (*pal_buffer_4k)[PAGE_SIZE_4K] = mem_pool_get_virt(pal_get_acs_drv_fd());

    for (i = 0; i < BUFFER_COUNT ; i++)
    {
        if (&pal_buffer_4k[i][0] == address && size == PAGE_SIZE_4K)
        {
            is_buffer_in_use[i] = 0;
            return PAL_SUCCESS;
        }
        if (&pal_buffer_4k[i][0] == address && size == PAGE_SIZE_4K * 2)
        {
            is_buffer_in_use[i] = 0;
            is_buffer_in_use[i+1] = 0;
            return PAL_SUCCESS;
        }
    }

    (void)size;
    return PAL_ERROR;
}

void *pal_mem_virt_to_phys(void *va)
{
    uintptr_t pa = 0;
    uintptr_t va_req = (uintptr_t)va;
    uintptr_t pa_block = mem_pool_get_phys(pal_get_acs_drv_fd());
    uintptr_t va_block = (uintptr_t)mem_pool_get_virt(pal_get_acs_drv_fd());

    if ((va_block % PAGE_SIZE_4K != 0) ||
        (pa % PAGE_SIZE_4K != 0) ||
        (va_req % PAGE_SIZE_4K != 0))
    {
        PAL_LOG(ERROR, "Virtual to Physical Address Misaligned va_req \
            %lx pa_block %lx va_block %lx");
        return NULL;
    }

    // Calculate the offset from the virtual start
    uintptr_t offset = va_req - va_block;
    pa = pa_block + offset;

    PAL_LOG(DBG, "Requested VA %lx --> PA %lx", (uint64_t)va, (uint64_t)pa);
    return (void *) pa;
}