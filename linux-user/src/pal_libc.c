/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"
#include "pal_kernel_helpers.h"
#include <string.h>

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

void *pal_get_heap_buffer(void)
{
    uint8_t *ptr = mem_pool_get_virt(pal_get_acs_drv_fd());

    assert(((uintptr_t)ptr % PAGE_SIZE_4K) == 0);
    assert(ptr);
    return (void *)ptr;
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