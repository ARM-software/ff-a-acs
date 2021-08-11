/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_MEMORY_H_
#define _VAL_MEMORY_H_

#include "val.h"
#include "val_sysreg.h"

#define PGT_STAGE1 1
#define PGT_STAGE2 2

#define PGT_IAS     40
#define PAGT_OAS    40
#define PGT_IPS     0x2ull
#define PGT_T0SZ    (64 - PGT_IAS)

#define PGT_ENTRY_TABLE_MASK (0x1 << 1)
#define PGT_ENTRY_VALID_MASK  0x1
#define PGT_ENTRY_PAGE_MASK  (0x1 << 1)
#define PGT_ENTRY_BLOCK_MASK (0x0 << 1)

#define IS_PGT_ENTRY_PAGE(val) (val & 0x2)
#define IS_PGT_ENTRY_BLOCK(val) !(val & 0x2)

#define PGT_DESC_SIZE 8
#define PGT_DESC_ATTR_UPPER_MASK ((0x1ull << 12) - 1) << 52
#define PGT_DESC_ATTR_LOWER_MASK ((0x1ull << 10) - 1) << 2
#define PGT_DESC_ATTRIBUTES_MASK \
    (PGT_DESC_ATTR_UPPER_MASK | PGT_DESC_ATTR_LOWER_MASK)
#define PGT_DESC_ATTRIBUTES(val) (val & PGT_DESC_ATTRIBUTES_MASK)

typedef struct {
    uint32_t ias;
    uint32_t oas;
    uint32_t stage;
    uint64_t *ttbr;
} pgt_descriptor_t;

typedef struct {
    uint64_t *tt_base;
    uint64_t input_base;
    uint64_t input_top;
    uint64_t output_base;
    uint32_t level;
    uint32_t size_log2;
    uint32_t nbits;
} tt_descriptor_t;

uint32_t val_setup_mmu(void);
uint32_t val_mem_map_pgt(memory_region_descriptor_t *mem_desc);

extern uint64_t tt_l0_base[];
extern uint64_t tt_l1_base[];
extern uint64_t tt_l2_base_1[];
extern uint64_t tt_l2_base_2[];
extern uint64_t tt_l2_base_3[];
extern uint64_t tt_l2_base_4[];
extern uint64_t tt_l3_base_1[];
extern uint64_t tt_l3_base_2[];
extern uint64_t tt_l3_base_3[];
extern uint64_t tt_l3_base_4[];
#endif /* _VAL_MEMORY_H_ */
