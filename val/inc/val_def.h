/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_DEF_H_
#define _VAL_DEF_H_

#include "pal_config_def.h"

/* ACS Version Info */
#define ACS_MAJOR_VERSION   0
#define ACS_MINOR_VERSION   9

#define IMAGE_SIZE        0x100000

#define STACK_SIZE          0x1000
#define SCTLR_I_BIT         (1 << 12)
#define SCTLR_M_BIT         (1 << 0)

#if (PLATFORM_PAGE_SIZE == PAGE_SIZE_4K)
#define PAGE_ALIGNMENT      PAGE_SIZE_4K
#elif (PLATFORM_PAGE_SIZE == PAGE_SIZE_16K)
#define PAGE_ALIGNMENT      PAGE_SIZE_16K
#elif (PLATFORM_PAGE_SIZE == PAGE_SIZE_64K)
#define PAGE_ALIGNMENT      PAGE_SIZE_64K
#else
#error "Undefined value for PLATFORM_PAGE_SIZE"
#endif

/* Logical ids for test endpoint */
#define NO_SERVER_EP  0
#define SP1           1
#define SP2           2
#define SP3           3
#define SP4           4
#define VM1           5
#define VM2           6
#define VM3           7

#define HYPERVISOR_ID 0

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1)
#define VAL_NS_EP_COUNT 0x3
#else
#define VAL_NS_EP_COUNT 0x1
#endif

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
#define VAL_S_EP_COUNT  0x0
#else
#define VAL_S_EP_COUNT  0x4
#endif
#define VAL_TOTAL_EP_COUNT VAL_NS_EP_COUNT + VAL_S_EP_COUNT

#define VAL_TG0_4K  0x0
#define VAL_TG0_64K 0x1
#define VAL_TG0_16K 0x2

#define EL2 2
#define EL1 1
#define EL0 0
#define EL64 1
#define EL32 0

#define EL_NUM_MASK     0xf
#define EL_WIDTH_MASK   0xf0
#define EL_WIDTH_SHIFT  4

#define GET_EL_NUM(x)   (x & 0xf)
#define GET_EL_WIDTH(x) ((x & 0xf0) >> EL_WIDTH_SHIFT)

#define EL2_64 (1 << EL_WIDTH_SHIFT) | EL2
#define EL2_32 EL2

#define EL1_64 (1 << EL_WIDTH_SHIFT) | EL1
#define EL1_32 EL1

#define EL0_64 (1 << EL_WIDTH_SHIFT) | EL0
#define EL0_32 EL0

#define VAL_IS_ENDPOINT_SECURE(x) (x < VM1 ? 1 : 0)

/* ASM Macro */
#ifdef CMAKE_GNUARM_COMPILE
#define ASM_FUNC_START(x) .func x
#define ASM_FUNC_END(x)   .endfunc
#else
#define ASM_FUNC_START(x)
#define ASM_FUNC_END(x)
#endif

/* Macros reserved for future use */
#define PLATFORM_SP_SEND_DIRECT_REQ     1
#define PLATFORM_VM_SEND_DIRECT_RESP    1

#endif /* _VAL_DEF_H_ */
