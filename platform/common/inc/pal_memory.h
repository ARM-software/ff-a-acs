/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_MEMORY_H_
#define _PAL_MEMORY_H_

#include "pal_interfaces.h"

#define MAX_ALOCATION_COUNT       10
#define MAX_ALOCATION_SIZE        0x10000

#define ATTR_NORMAL_NONCACHEABALE (0x0ull << 2)
#define ATTR_NORMAL_WB_WA_RA      (0x1ull << 2)
#define ATTR_DEVICE               (0x2ull << 2)

#define ATTR_NS   (0x1ull << 5)
#define ATTR_S    (0x0ull << 5)

#define ATTR_STAGE1_AP_RW (0x1ull << 6)
#define ATTR_STAGE2_AP_RW (0x3ull << 6)

#define ATTR_NON_SHARED     (0x0ull << 8)
#define ATTR_OUTER_SHARED   (0x2ull << 8)
#define ATTR_INNER_SHARED   (0x3ull << 8)

#define ATTR_AF   (0x1ull << 10)
#define ATTR_nG   (0x1ull << 11)

#define ATTR_UXN    (0x1ull << 54)
#define ATTR_PXN    (0x1ull << 53)

#define ATTR_PRIV_RW        (0x0ull << 6)
#define ATTR_PRIV_RO        (0x2ull << 6)
#define ATTR_USER_RW        (0x1ull << 6)
#define ATTR_USER_RO        (0x3ull << 6)

#if (defined(VM1_COMPILE) ||\
     defined(VM2_COMPILE)  ||\
     defined(VM3_COMPILE))
#define ADD_NS_BIT  ATTR_NS
#else
#define ADD_NS_BIT  ATTR_S
#endif

#define ATTR_CODE           (ATTR_NORMAL_WB_WA_RA | ATTR_USER_RO | \
                              ATTR_AF | ATTR_INNER_SHARED | ADD_NS_BIT)
#define ATTR_RO_DATA        (ATTR_NORMAL_WB_WA_RA | ATTR_USER_RO | \
                              ATTR_UXN | ATTR_PXN | ATTR_AF | \
                              ATTR_INNER_SHARED | ADD_NS_BIT)
#define ATTR_RW_DATA        (ATTR_NORMAL_WB_WA_RA | \
                              ATTR_USER_RW | ATTR_UXN | ATTR_PXN | ATTR_AF \
                              | ATTR_INNER_SHARED | ADD_NS_BIT)
#define ATTR_DEVICE_RW      (ATTR_DEVICE | ATTR_USER_RW | ATTR_UXN | \
                              ATTR_PXN | ATTR_AF | ATTR_INNER_SHARED | \
                              ATTR_NS)

typedef struct mem_info {
    uint32_t index;
    uint64_t base_address;
    uint64_t total_size;
    uint32_t hole;
} mem_info_t;

typedef struct mem_alloc_info {
    uint64_t address;
    uint64_t size;
} mem_alloc_info_t;

typedef struct {
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t length;
    uint64_t attributes;
} memory_region_descriptor_t;
#endif /* _PAL_MEMORY_H_ */
