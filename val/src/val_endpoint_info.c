/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val.h"
#include "val_endpoint_info.h"
#include "pal_config_def.h"

static val_endpoint_info_t endpoint_info_table[] = {
    {"", 0, 0, 0, 0, 0, {0} },
    {
        "SP1",
        PLATFORM_SP1_ID,
        VAL_TG0_4K,
#if (PLATFORM_SP_EL == EL1)
        EL1_64,
#else
        EL0_64,
#endif
        PLATFORM_SP1_EC_COUNT,
        PLATFORM_SP1_EP_PROPERTIES,
        PLATFORM_SP1_UUID,
    },
    {
        "SP2",
        PLATFORM_SP2_ID,
        VAL_TG0_4K,
#if (PLATFORM_SP_EL == EL1)
        EL1_64,
#else
        EL0_64,
#endif
        PLATFORM_SP2_EC_COUNT,
        PLATFORM_SP2_EP_PROPERTIES,
        PLATFORM_SP2_UUID,
    },
    {
        "SP3",
        PLATFORM_SP3_ID,
        VAL_TG0_4K,
#if (PLATFORM_SP_EL == EL1)
        EL1_64,
#else
        EL0_64,
#endif
        PLATFORM_SP3_EC_COUNT,
        PLATFORM_SP3_EP_PROPERTIES,
        PLATFORM_SP3_UUID,
    },
    {
        "SP4",
        PLATFORM_SP4_ID,
        VAL_TG0_4K,
#if (PLATFORM_SP_EL == EL1)
        EL1_64,
#else
        EL0_64,
#endif
        PLATFORM_SP4_EC_COUNT,
        PLATFORM_SP4_EP_PROPERTIES,
        PLATFORM_SP4_UUID,
    },
	{
        "VM1",
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1)
        PLATFORM_VM1_ID,
#else
#ifdef TARGET_LINUX_ID
        0x1,
#else
        0x0,
#endif
#endif
        VAL_TG0_4K,
        EL1_64,
        PLATFORM_VM1_EC_COUNT,
        PLATFORM_VM1_EP_PROPERTIES,
        PLATFORM_VM1_UUID,
    },
    {
        "VM2",
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1)
        PLATFORM_VM2_ID,
        VAL_TG0_4K,
        EL1_64,
        PLATFORM_VM2_EC_COUNT,
        PLATFORM_VM2_EP_PROPERTIES,
        PLATFORM_VM2_UUID,
#else
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        {0x0}
#endif
    },
    {
        "VM3",
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1)
        PLATFORM_VM3_ID,
        VAL_TG0_4K,
        EL1_64,
        PLATFORM_VM3_EC_COUNT,
        PLATFORM_VM3_EP_PROPERTIES,
        PLATFORM_VM3_UUID,
#else
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        {0x0}
#endif
    },
};

/**
 *   @brief    - Convert the logical endpoint id into actual id
 *   @param    - Endpoint logical id
 *   @return   - Actual endpoint id assigned by the system
**/
ffa_endpoint_id_t val_get_endpoint_id(uint32_t logical_id)
{
    return endpoint_info_table[logical_id].id;
}

/**
 *   @brief    - Convert the actual id into logical endpoint id
 *   @param    - Endpoint id
 *   @return   - Logical endpoint id
**/
ffa_endpoint_id_t val_get_endpoint_logical_id(ffa_endpoint_id_t endpoint_id)
{
    ffa_endpoint_id_t logical_id = 1;

    while (logical_id <
            sizeof(endpoint_info_table)/sizeof(endpoint_info_table[0]))
    {
        if (endpoint_info_table[logical_id].id == endpoint_id)
            return logical_id;

        logical_id++;
    }

    VAL_PANIC("\tError: Couldn't find correct logical_id.\n");
    return logical_id;
}

/**
 *   @brief    - Returns the current endpoint logical id
 *   @param    - void
 *   @return   - Current endpoint logical id
**/
ffa_endpoint_id_t val_get_curr_endpoint_logical_id(void)
{
    ffa_endpoint_id_t logical_id = 1;

    while (logical_id <
           sizeof(endpoint_info_table)/sizeof(endpoint_info_table[0]))
    {
        if (endpoint_info_table[logical_id].id == val_get_curr_endpoint_id())
            return logical_id;

        logical_id++;
    }

    LOG(ERROR, "\tError: Couldn't find correct logical_id. %x\n", val_get_curr_endpoint_id());
    return logical_id;
}

/**
 *   @brief    - Returns the current endpoint name
 *   @param    - void
 *   @return   - Current endpoint name
**/
char *val_get_curr_endpoint_name(void)
{
    return endpoint_info_table[val_get_curr_endpoint_logical_id()].name;
}

/**
 *   @brief    - Returns the current endpoint name
 *   @param    - logical_id
 *   @return   - Endpoint name associated with given logical_id
**/
char *val_get_endpoint_name(uint32_t logical_id)
{
    return endpoint_info_table[logical_id].name;
}

/**
 *   @brief    - Returns the current endpoint tg0
 *   @param    - void
 *   @return   - Current endpoint tt tg0
**/
uint64_t val_get_curr_endpoint_tt_tg0(void)
{
    return endpoint_info_table[val_get_curr_endpoint_logical_id()].tg0;
}

/**
 *   @brief    - set tg0 for given endpoint
 *   @param    - logical enpoint id
 *   @param    - tg0 value
 *   @return   - void
**/
void val_set_endpoint_tt_tg0(uint32_t logical_id, uint8_t tg0)
{
    endpoint_info_table[logical_id].tg0 = tg0;
}

/**
 *   @brief    - Returns the current endpoint el_info
 *   @param    - void
 *   @return   - Current endpoint el_info
**/
uint8_t val_get_curr_endpoint_el_info(void)
{
    return endpoint_info_table[val_get_curr_endpoint_logical_id()].el_info;
}

/**
 *   @brief    - Returns the endpoint el_info for given logical ep number
 *   @param    - logical_id
 *   @return   - Endpoint el_info
**/
uint8_t val_get_endpoint_el_info(uint32_t logical_id)
{
    return endpoint_info_table[logical_id].el_info;
}
/**
 *   @brief    - Returns the endpoint info structure pointer.
 *   @param    - void.
 *   @return   - Returns the endpoint info structure pointer.
**/
val_endpoint_info_t *val_get_endpoint_info(void)
{
    return endpoint_info_table;
}

/**
 * @brief Select tg0 for endpoint and update endpoint info
 *        structure with updated tg0 value
 * @param none
 * @return page size
**/
uint32_t val_assign_tg0_to_endpoint(void)
{
    uint64_t mmfr0;
    uint8_t tg0_4k = 0, tg0_64k = 0, tg0_16k = 0;

    mmfr0 = val_id_aa64mmfr0_el1_read();

    /* Check that 4KB granule is supported. */
    if (((mmfr0 >> 28) & 0xf) == 0x0)
        tg0_4k = 0x1;

    /* Check that 64KB granule is supported. */
    if (((mmfr0 >> 24) & 0xf) == 0x0)
        tg0_64k = 0x1;

    /* Check that 16KB granule is supported. */
    if (((mmfr0 >> 20) & 0xf) == 0x1)
        tg0_16k = 0x1;

    if (tg0_4k)
    {
    /* Bydefault all endpoint data structure set with 4K tg0 */
        return VAL_SUCCESS;
    }
    else if (tg0_64k)
    {
        val_set_endpoint_tt_tg0(SP1, VAL_TG0_64K);
        val_set_endpoint_tt_tg0(SP2, VAL_TG0_64K);
        val_set_endpoint_tt_tg0(SP3, VAL_TG0_64K);
        val_set_endpoint_tt_tg0(SP4, VAL_TG0_64K);
        val_set_endpoint_tt_tg0(VM1, VAL_TG0_64K);
        val_set_endpoint_tt_tg0(VM2, VAL_TG0_64K);
        val_set_endpoint_tt_tg0(VM3, VAL_TG0_64K);
    }
    else if (tg0_16k)
    {
        val_set_endpoint_tt_tg0(SP1, VAL_TG0_16K);
        val_set_endpoint_tt_tg0(SP2, VAL_TG0_16K);
        val_set_endpoint_tt_tg0(SP3, VAL_TG0_16K);
        val_set_endpoint_tt_tg0(SP4, VAL_TG0_16K);
        val_set_endpoint_tt_tg0(VM1, VAL_TG0_16K);
        val_set_endpoint_tt_tg0(VM2, VAL_TG0_16K);
        val_set_endpoint_tt_tg0(VM3, VAL_TG0_16K);
    }
    else
    {
        return VAL_ERROR;
    }

    return VAL_SUCCESS;
}

/**
 * @brief return tt page size for current endpoint
 * @param none
 * @return page size
**/
uint32_t val_curr_endpoint_page_size(void)
{
    uint8_t  tg0 = (uint8_t)val_get_curr_endpoint_tt_tg0();

    if (tg0 == 0)
        return (4 * 1024);
    else if (tg0 == 1)
        return (64 * 1024);
    else
        return (16 * 1024);
}
