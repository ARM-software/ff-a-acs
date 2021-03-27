/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_memory.h"
#include "val_framework.h"
#include "val_interfaces.h"

#define min(a, b) (a < b)?a:b

//#define PGT_DEBUG 1

static uint32_t bits_per_level;
static uint64_t pgt_addr_mask;
static uint8_t tt_l2_base_1_used = 0;
static uint8_t tt_l2_base_2_used = 0;
static uint8_t tt_l2_base_3_used = 0;
static uint8_t tt_l2_base_4_used = 0;
static uint8_t tt_l3_base_1_used = 0;
static uint8_t tt_l3_base_2_used = 0;
static uint8_t tt_l3_base_3_used = 0;
static uint8_t tt_l3_base_4_used = 0;

/* Linker symbols used to figure out the memory layout of secure partition. */
extern uintptr_t __TEXT_START__, __TEXT_END__;
#define EP_TEXT_START    ((uintptr_t)&__TEXT_START__)
#define EP_TEXT_END      ((uintptr_t)&__TEXT_END__)

extern uintptr_t __RODATA_START__, __RODATA_END__;
#define EP_RODATA_START  ((uintptr_t)&__RODATA_START__)
#define EP_RODATA_END    ((uintptr_t)&__RODATA_END__)

extern uintptr_t __DATA_START__, __DATA_END__;
#define EP_DATA_START    ((uintptr_t)&__DATA_START__)
#define EP_DATA_END      ((uintptr_t)&__DATA_END__)

extern uintptr_t __BSS_START__, __BSS_END__;
#define EP_BSS_START  ((uintptr_t)&__BSS_START__)
#define EP_BSS_END    ((uintptr_t)&__BSS_END__)

/**
 * @brief Update the descriptors for given VA-PA mapping
 * @param tt_desc - Data like ttbr, number of tt levels..
 * @param mem_desc - memory addrs and attributes needed for page table creation.
 * @return status
**/
static uint32_t fill_translation_table(tt_descriptor_t tt_desc,
                           memory_region_descriptor_t *mem_desc)
{
    uint64_t block_size = 0x1ull << tt_desc.size_log2;
    uint64_t input_address, output_address, table_index, *table_desc;
    tt_descriptor_t tt_desc_next_level;
    uint64_t *tt_base_next_level = NULL;
    uint64_t page_size = val_curr_endpoint_page_size();

#ifdef PGT_DEBUG
    LOG(DBG, "\ttt_desc.level: %d\n", tt_desc.level, 0);
    LOG(DBG, "\ttt_desc.input_base: 0x%x\n", tt_desc.input_base, 0);
    LOG(DBG, "\ttt_desc.input_top: 0x%x\n", tt_desc.input_top, 0);
    LOG(DBG, "\ttt_desc.output_base: 0x%x\n", tt_desc.output_base, 0);
    LOG(DBG, "\ttt_desc.size_log2: %d\n", tt_desc.size_log2, 0);
    LOG(DBG, "\ttt_desc.nbits: %d\n", tt_desc.nbits, 0);
#endif

    for (input_address = tt_desc.input_base,
         output_address = tt_desc.output_base;
         input_address < tt_desc.input_top;
         input_address += block_size, output_address += block_size)
    {
        table_index = input_address >> tt_desc.size_log2 &
                      ((0x1ull << tt_desc.nbits) - 1);
        table_desc = &tt_desc.tt_base[table_index];

#ifdef PGT_DEBUG
        LOG(DBG, "\ttable_index = 0x%x\n", table_index, 0);
        LOG(DBG, "\ttable_desc_addr = 0x%lx\n", (uint64_t)table_desc, 0);
#endif

        if (tt_desc.level == 3)
        {
            /* Create level3 page descriptor entry */
            *table_desc = PGT_ENTRY_PAGE_MASK | PGT_ENTRY_VALID_MASK;
            *table_desc |= (output_address & ~(page_size - 1));
            *table_desc |= mem_desc->attributes;
#ifdef PGT_DEBUG
            LOG(DBG, "\tpage_descriptor = 0x%lx\n", *table_desc, 0);
#endif
            continue;
        }

        /*
         * Are input and output addresses eligible for being
         * described via block descriptor?
         */
        if ((input_address & (block_size - 1)) == 0 &&
             (output_address & (block_size - 1)) == 0 &&
             tt_desc.input_top >= (input_address + block_size - 1))
         {
            /* Create a block descriptor entry */
            *table_desc = PGT_ENTRY_BLOCK_MASK | PGT_ENTRY_VALID_MASK;
            *table_desc |= (output_address & ~(block_size - 1));
            *table_desc |= mem_desc->attributes;
#ifdef PGT_DEBUG
            LOG(DBG, "\tblock_descriptor = 0x%lx\n", *table_desc, 0);
#endif
            continue;
        }

        /* If there's no descriptor populated at current index of this
         * page_table, or If there's a block descriptor, allocate new page,
         * else use the already populated address. Block descriptor info will
         * be overwritten in case its there.
        */
        if (*table_desc == 0 || IS_PGT_ENTRY_BLOCK(*table_desc))
        {
           /* select table base from statically allocated memory */
           if (tt_desc.level == 1)
           {
               if (!tt_l2_base_1_used)
               {
                     tt_base_next_level = tt_l2_base_1;
                     tt_l2_base_1_used = 1;
               }
               else if (!tt_l2_base_2_used)
               {
                     tt_base_next_level = tt_l2_base_2;
                     tt_l2_base_2_used = 1;
               }
               else if (!tt_l2_base_3_used)
               {
                     tt_base_next_level = tt_l2_base_3;
                     tt_l2_base_3_used = 1;
               }
               else if (!tt_l2_base_4_used)
               {
                     tt_base_next_level = tt_l2_base_4;
                     tt_l2_base_4_used = 1;
               }
               else
               {
                    LOG(ERROR, "\tOut of memory, allocate more tt_l2 space", 0, 0);
                    return VAL_ERROR;
               }
           }
           else if (tt_desc.level == 2)
           {
               if (!tt_l3_base_1_used)
               {
                     tt_base_next_level = tt_l3_base_1;
                     tt_l3_base_1_used = 1;
               }
               else if (!tt_l3_base_2_used)
               {
                     tt_base_next_level = tt_l3_base_2;
                     tt_l3_base_2_used = 1;
               }
               else if (!tt_l3_base_3_used)
               {
                     tt_base_next_level = tt_l3_base_3;
                     tt_l3_base_3_used = 1;
               }
               else if (!tt_l3_base_4_used)
               {
                     tt_base_next_level = tt_l3_base_4;
                     tt_l3_base_4_used = 1;
               }
               else
               {
                    LOG(ERROR, "\tOut of memory, allocate more tt_l3 space", 0, 0);
                    return VAL_ERROR;
               }

           }
           else
           {
               tt_base_next_level = tt_l1_base;
           }
        }
        else
        {
            tt_base_next_level = (uint64_t *)(*table_desc & pgt_addr_mask);
        }

        *table_desc = PGT_ENTRY_TABLE_MASK | PGT_ENTRY_VALID_MASK;
        *table_desc |= (uint64_t)(tt_base_next_level) & ~(page_size - 1);
#ifdef PGT_DEBUG
        LOG(DBG, "\ttable_descriptor = 0x%lx\n", *table_desc, 0);
#endif

        tt_desc_next_level.tt_base = tt_base_next_level;
        tt_desc_next_level.input_base = input_address;
        tt_desc_next_level.input_top =
                      min(tt_desc.input_top, (input_address + block_size - 1));
        tt_desc_next_level.output_base = output_address;
        tt_desc_next_level.level = tt_desc.level + 1;
        tt_desc_next_level.size_log2 = tt_desc.size_log2 - bits_per_level;
        tt_desc_next_level.nbits = bits_per_level;

        if (fill_translation_table(tt_desc_next_level, mem_desc))
        {
            return VAL_ERROR;
        }

    }
    return VAL_SUCCESS;
}

static uint32_t ilog2(uint64_t size)
{
    uint32_t bit = 0;
    while (size != 0)
    {
        if (size & 1)
            return bit;
        size >>= 1;
        ++bit;
    }
    return 0;
}

/**
 * @brief Create page table for given memory addresses and attributes
 * @param pgt_desc - Data like input and output address size, translation stage.
 * @param mem_desc - memory addrs and attributes needed for page table creation.
 * @return status
**/
static uint32_t val_pgt_create(pgt_descriptor_t pgt_desc,
                        memory_region_descriptor_t *mem_desc)
{
    tt_descriptor_t tt_desc;
    uint32_t num_pgt_levels;
    uint64_t page_size = val_curr_endpoint_page_size();

#ifdef PGT_DEBUG
    LOG(DBG, "\tval_pgt_create: input addr = 0x%lx\n",
        mem_desc->virtual_address, 0);
    LOG(DBG, "\tval_pgt_create: output addr = 0x%lx\n",
        mem_desc->physical_address, 0);
    LOG(DBG, "\tval_pgt_create: length = 0x%x\n",
        mem_desc->length, 0);
    LOG(DBG, "\tval_pgt_create: attributes = 0x%lx\n",
        mem_desc->attributes, 0);
#endif
    if ((mem_desc->virtual_address & (uint64_t)(page_size - 1)) != 0 ||
        (mem_desc->physical_address & (uint64_t)(page_size - 1)) != 0)
    {
            LOG(ERROR, "\tval_pgt_create: address alignment error\n", 0, 0);
            return VAL_ERROR;
    }

    if (mem_desc->physical_address >= (0x1ull << pgt_desc.oas))
    {
        LOG(ERROR, "\tval_pgt_create: output address size error\n", 0, 0);
        return VAL_ERROR;
    }

    if (mem_desc->virtual_address >= (0x1ull << pgt_desc.ias))
    {
        LOG(WARN, "\tval_pgt_create: input address size error,\
                    truncating to %d-bits\n", pgt_desc.ias, 0);
        mem_desc->virtual_address &= ((0x1ull << pgt_desc.ias) - 1);
    }

    uint32_t page_size_log2 = ilog2(page_size);

    bits_per_level = page_size_log2 - 3;
    num_pgt_levels = (pgt_desc.ias - page_size_log2 + bits_per_level - 1)
                      /bits_per_level;

#ifdef PGT_DEBUG
    LOG(DBG, "\ttval_pgt_create: page_size = 0x%x\n", page_size, 0);
    LOG(DBG, "\tval_pgt_create: page_size_log2 = %d\n", page_size_log2, 0);
    LOG(DBG, "\tval_pgt_create: nbits_per_level = %d\n", bits_per_level, 0);
#endif

    if (pgt_desc.stage == PGT_STAGE1)
        mem_desc->attributes |= ATTR_STAGE1_AP_RW;
    else if (pgt_desc.stage == PGT_STAGE2)
        mem_desc->attributes |= ATTR_STAGE2_AP_RW;
    else
        return VAL_ERROR;

    tt_desc.tt_base = pgt_desc.ttbr;
    tt_desc.input_base = mem_desc->virtual_address &
                        ((0x1ull << pgt_desc.ias) - 1);
    tt_desc.input_top = tt_desc.input_base + mem_desc->length - 1;
    tt_desc.output_base = mem_desc->physical_address &
                          ((0x1ull << pgt_desc.oas) - 1);
    tt_desc.level = 4 - num_pgt_levels;
    tt_desc.size_log2 = (num_pgt_levels - 1) * bits_per_level + page_size_log2;
    tt_desc.nbits = pgt_desc.ias - tt_desc.size_log2;

    pgt_addr_mask = ((0x1ull << (48 - page_size_log2)) - 1) << page_size_log2;
    if (fill_translation_table(tt_desc, mem_desc))
    {
        LOG(ERROR, "\tval_pgt_create: page table creation failed\n", 0, 0);
        return VAL_ERROR;
    }

    return VAL_SUCCESS;
}

/**
 * @brief Map endpoint image sections and assigned device into pgt
 * @param pgt_desc - Data like input and output address size, ttbr
 * @return status
**/
static uint32_t val_map_endpoint_region(pgt_descriptor_t pgt_desc)
{
    void *region_list = NULL;
    memory_region_descriptor_t mem_desc;
    size_t no_of_regions = 0, i = 0;
    memory_region_descriptor_t endpoint_image_regions[] = {
    {EP_TEXT_START, EP_TEXT_START, (EP_TEXT_END - EP_TEXT_START), ATTR_CODE},
    {EP_RODATA_START, EP_RODATA_START, (EP_RODATA_END - EP_RODATA_START), ATTR_RO_DATA},
    {EP_DATA_START, EP_DATA_START, (EP_DATA_END - EP_DATA_START), ATTR_RW_DATA},
    {EP_BSS_START, EP_BSS_START, (EP_BSS_END - EP_BSS_START), ATTR_RW_DATA},
    };

    /* Map Image regions */
    no_of_regions = sizeof(endpoint_image_regions)/sizeof(endpoint_image_regions[0]);
    while (i < no_of_regions)
    {
        mem_desc.virtual_address = endpoint_image_regions[i].virtual_address;
        mem_desc.physical_address = endpoint_image_regions[i].physical_address;
        mem_desc.length = endpoint_image_regions[i].length;
        mem_desc.attributes = endpoint_image_regions[i].attributes;

        LOG(DBG, "\tCreating page table for image region  : 0x%lx - 0x%lx\n",
            mem_desc.virtual_address, (mem_desc.virtual_address + mem_desc.length) - 1);

        if (val_pgt_create(pgt_desc, &mem_desc))
        {
            return VAL_ERROR;
        }
        i++;
    }

    /* Map Device regions */
    if (pal_get_endpoint_device_map(&region_list, &no_of_regions))
        return VAL_ERROR;

    while (no_of_regions--)
    {
        mem_desc.virtual_address = (uint64_t)((memory_region_descriptor_t *)region_list)->virtual_address;
        mem_desc.physical_address = (uint64_t)((memory_region_descriptor_t *)region_list)->physical_address;
        mem_desc.length = (uint64_t)((memory_region_descriptor_t *)region_list)->length;
        mem_desc.attributes = (uint64_t)((memory_region_descriptor_t *)region_list)->attributes;

        LOG(DBG, "\tCreating page table for device region  : 0x%lx - 0x%lx\n",
            mem_desc.virtual_address, (mem_desc.virtual_address + mem_desc.length) - 1);

        if (val_pgt_create(pgt_desc, &mem_desc))
        {
            return VAL_ERROR;
        }

        region_list = region_list + sizeof(memory_region_descriptor_t);
    }

    return VAL_SUCCESS;
}

/**
 * @brief Setup and enable page table mmu for endpoint
 * @param void
 * @return status
**/
uint32_t val_setup_mmu(void)
{
    pgt_descriptor_t pgt_desc;
    uint64_t tcr;
    uint8_t  tg0;
    uint8_t  el_info = val_get_curr_endpoint_el_info();
    uint8_t  currentEL;

    /* Skip mmu setup for EL0 SP */
    if (GET_EL_NUM(el_info) == EL0)
        return VAL_SUCCESS;

    currentEL = (val_read_current_el() & 0xc) >> 2;
    /*
     * Setup mair
     * Attr0 = b01000100 = Normal, Inner/Outer Non-Cacheable
     * Attr1 = b11111111 = Normal, Inner/Outer WB/WA/RA
     * Attr2 = b00000000 = Device-nGnRnE
     */
    val_mair_write(0xFF44, currentEL);

    if (val_assign_tg0_to_endpoint())
    {
        LOG(ERROR, "\tEndpoint tg0 update failed\n", 0, 0);
        return VAL_ERROR;
    }

    tg0 = (uint8_t)val_get_curr_endpoint_tt_tg0();

    /* Setup ttbr0 */
    pgt_desc.ttbr = tt_l0_base;
    val_ttbr0_write((uint64_t)tt_l0_base, currentEL);

    /* setup tcr */
    if (currentEL == EL2)
    {
        tcr = ((1ull << 20) |          /* TBI, top byte ignored. */
              ((uint64_t)tg0 << 14) |  /* TG0, granule size */
              (3ull << 12) |           /* SH0, inner shareable. */
              (1ull << 10) |           /* ORGN0, normal mem, WB RA WA Cacheable */
              (1ull << 8) |            /* IRGN0, normal mem, WB RA WA Cacheable */
              PGT_T0SZ);               /* T0SZ, input address is 2^40 bytes. */
    }
    else
    {
        tcr = ((1ull << 37) |          /* TBI, top byte ignored. */
              ((uint64_t)tg0 << 14) |  /* TG0, granule size */
              (3ull << 12) |           /* SH0, inner shareable. */
              (1ull << 10) |           /* ORGN0, normal mem, WB RA WA Cacheable */
              (1ull << 8) |            /* IRGN0, normal mem, WB RA WA Cacheable */
              (1ull << 23) |           /* EPD1=0b1 Disable table walk from TTBR1 */
              (PGT_IPS << 32) |        /* IPS=2, 40 bit IPA space */
              PGT_T0SZ);               /* T0SZ, input address is 2^40 bytes. */
        /*
         * NOTE: We don't need to set up T1SZ/TBI1/ORGN1/IRGN1/SH1,
         * as we've set EPD==1 (disabling walks from TTBR1)
         */
    }
    val_tcr_write(tcr, currentEL);
    pgt_desc.stage = PGT_STAGE1;
    pgt_desc.ias = PGT_IAS;
    pgt_desc.oas = PAGT_OAS;
#ifdef PGT_DEBUG
    LOG(DBG, "\tval_setup_mmu: TG0=0x%x\n", tg0, 0);
    LOG(DBG, "\tval_setup_mmu: ias=%d\n", pgt_desc.ias, 0);
    LOG(DBG, "\tval_setup_mmu: oas=%d\n", pgt_desc.oas, 0);
    LOG(DBG, "\tval_setup_mmu: tcr=0x%lx\n", tcr, 0);
#endif

    /* Create page tables for image sections and assigned device */
    if (val_map_endpoint_region(pgt_desc))
        return VAL_ERROR;

    /* Enable MMU */
    val_sctlr_write((1 << 0) | // M=1 Enable the stage 1 MMU
                    (1 << 2) | // C=1 Enable data and unified caches
                    (1 << 12)| // I=1 Enable instruction caches
                    val_sctlr_read(currentEL),
                    currentEL);
#ifdef PGT_DEBUG
    LOG(DBG, "\tval_setup_mmu: successful\n", 0, 0);
#endif
    return VAL_SUCCESS;
}

/**
 * @brief Map the given region into page table
 * @param mem_desc - memory addrs and attributes needed for page table mapping.
 * @return status
**/
uint32_t val_mem_map_pgt(memory_region_descriptor_t *mem_desc)
{
    pgt_descriptor_t pgt_desc;
    uint8_t el_info = val_get_curr_endpoint_el_info();
    uint8_t  currentEL;

    /* Skip mmu setup for EL0 SP */
    if (GET_EL_NUM(el_info) == EL0)
        return VAL_SUCCESS;

    currentEL = (val_read_current_el() & 0xc) >> 2;
    pgt_desc.ttbr = (uint64_t *)val_ttbr0_read(currentEL);
    pgt_desc.stage = PGT_STAGE1;
    pgt_desc.ias = PGT_IAS;
    pgt_desc.oas = PAGT_OAS;

    return val_pgt_create(pgt_desc, mem_desc);
}
