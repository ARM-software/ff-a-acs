/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"

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

__attribute__ ((aligned (PAGE_SIZE_4K))) uint8_t pal_heap_buffer[PLATFORM_HEAP_BUF_SIZE];

static memory_region_descriptor_t endpoint_device_regions[] = {
#if defined(SP1_COMPILE)
    {PLATFORM_S_UART_BASE, PLATFORM_S_UART_BASE, PLATFORM_S_UART_SIZE, ATTR_DEVICE_RW},
    {PLATFORM_NVM_BASE, PLATFORM_NVM_BASE, PLATFORM_NVM_SIZE, ATTR_DEVICE_RW},
    {PLATFORM_WDOG_BASE, PLATFORM_WDOG_BASE, PLATFORM_WDOG_SIZE, ATTR_DEVICE_RW},
    {PLATFORM_SP805_TWDOG_BASE, PLATFORM_SP805_TWDOG_BASE, PLATFORM_TWDOG_SIZE, ATTR_DEVICE_RW_S},
#endif

#if defined(SP2_COMPILE)
    {PLATFORM_AP_REFCLK_CNTBASE1, PLATFORM_AP_REFCLK_CNTBASE1, PLATFORM_AP_REFCLK_SIZE, \
        ATTR_DEVICE_RW_S},
    {PLAT_SMMUV3_TEST_ENGINE_MEM_REGION, PLAT_SMMUV3_TEST_ENGINE_MEM_REGION, \
        PLAT_SMMUV3_TEST_ENGINE_MEM_SIZE, ATTR_DEVICE_RW_S},
#endif

#if defined(VM1_COMPILE)
    {PLATFORM_NS_UART_BASE, PLATFORM_NS_UART_BASE, PLATFORM_NS_UART_SIZE, ATTR_DEVICE_RW},
    {PLATFORM_NS_WD_BASE, PLATFORM_NS_WD_BASE, PLATFORM_NS_WD_SIZE, ATTR_DEVICE_RW},
    {GICD_BASE, GICD_BASE, GICD_SIZE, ATTR_DEVICE_RW},
    {GICR_BASE, GICR_BASE, GICR_SIZE, ATTR_DEVICE_RW},
    {GICC_BASE, GICC_BASE, GICC_SIZE, ATTR_DEVICE_RW},
#endif
    };

void *pal_get_heap_buffer(void)
{
    return (void *)pal_heap_buffer;
}

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

void *pal_mem_virt_to_phys(void *va)
{
  /* Flat mapping va=pa */
  return va;
}
