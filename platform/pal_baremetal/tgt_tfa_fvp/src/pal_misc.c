/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"

#define BUFFER_COUNT 5
uint32_t is_buffer_in_use[BUFFER_COUNT] = {0};

/* Memory granularity and alignment:
 * To map the memory region correctly in both translation
 * regimes,the following constraints must be met:
 * - If X is the larger translation granule size used
 *   by the two translation regimes, then the size of
 *   the memoryregion must be a multiple of X.
 * - The base address of the memory region must be aligned to X.
 *   At the moment, acs uses 4K size and 4k alignment
 *   for memory addresses
 */
__attribute__ ((aligned (PAGE_SIZE_4K))) uint8_t pal_buffer_4k[BUFFER_COUNT][PAGE_SIZE_4K];

static memory_region_descriptor_t endpoint_device_regions[] = {
#if defined(SP1_COMPILE)
    {PLATFORM_S_UART_BASE, PLATFORM_S_UART_BASE, PLATFORM_S_UART_SIZE, ATTR_DEVICE_RW},
    {PLATFORM_NVM_BASE, PLATFORM_NVM_BASE, PLATFORM_NVM_SIZE, ATTR_DEVICE_RW},
    {PLATFORM_WDOG_BASE, PLATFORM_WDOG_BASE, PLATFORM_WDOG_SIZE, ATTR_DEVICE_RW},
    {PLATFORM_SP805_TWDOG_BASE, PLATFORM_SP805_TWDOG_BASE, PLATFORM_TWDOG_SIZE, ATTR_DEVICE_RW_S},
#endif

#if defined(SP2_COMPILE)
    {PLATFORM_AP_REFCLK_CNTBASE1, PLATFORM_AP_REFCLK_CNTBASE1, PLATFORM_AP_REFCLK_SIZE, ATTR_DEVICE_RW_S},
#endif

#if defined(VM1_COMPILE)
    {PLATFORM_NS_UART_BASE, PLATFORM_NS_UART_BASE, PLATFORM_NS_UART_SIZE, ATTR_DEVICE_RW},
    {PLATFORM_NS_WD_BASE, PLATFORM_NS_WD_BASE, PLATFORM_NS_WD_SIZE, ATTR_DEVICE_RW},
    {GICD_BASE, GICD_BASE, GICD_SIZE, ATTR_DEVICE_RW},
    {GICR_BASE, GICR_BASE, GICR_SIZE, ATTR_DEVICE_RW},
    {GICC_BASE, GICC_BASE, GICC_SIZE, ATTR_DEVICE_RW},
#endif
    };

uint32_t pal_get_endpoint_device_map(void **region_list, size_t *no_of_mem_regions)
{
    *region_list = (void *)endpoint_device_regions;
    *no_of_mem_regions = sizeof(endpoint_device_regions)/sizeof(endpoint_device_regions[0]);

    return PAL_SUCCESS;
}

uint32_t pal_terminate_simulation(void)
{
    while (1);
    return PAL_SUCCESS;
}

void *pal_memory_alloc(uint64_t size)
{
    int i;

    if (size == PAGE_SIZE_4K)
    {
        for (i = 0; i < BUFFER_COUNT ; i++)
        {
            if (!is_buffer_in_use[i])
            {
                is_buffer_in_use[i] = 1;
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
                return &pal_buffer_4k[i][0];
            }
        }
    }
    else
    {
        /* TBD: Need to add logic for 16K and 64K pages */
        pal_printf("\tval_memory_alloc failed\n", 0, 0);
    }

    return NULL;
}

uint32_t pal_memory_free(void *address, uint64_t size)
{
    int i;

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
  /* Flat mapping va=pa */
  return va;
}
