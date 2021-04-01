/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t ffa_mem_donate_mmio_helper(uint32_t test_run_data, uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
    mb_buf_t mb;
    ffa_memory_region_flags_t flags = 0;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "\tRxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    /* Framework does not permit: Ownership of a device MMIO region to be
     * transferred to another partition during run-time.
     */
    if (VAL_IS_ENDPOINT_SECURE(val_get_curr_endpoint_logical_id()))
        constituents[0].address = (void *)PLATFORM_S_UART_BASE;
    else
        constituents[0].address = (void *)PLATFORM_NS_UART_BASE;

    constituents[0].page_count = 1;

    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = flags;
    mem_region_init.data_access = FFA_DATA_ACCESS_NOT_SPECIFIED;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_DEVICE_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_NON_CACHEABLE;
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;
    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "\tFramework must not allow to donate mmio region during run time\n", 0, 0);
        status = VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "\tRXTX_UNMAP failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(4);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "\tval_mem_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(5);
    }

    return status;
}
uint32_t ffa_mem_donate_mmio_client(uint32_t test_run_data)
{
    uint32_t status_32, status_64, status;

    status_64 = val_is_ffa_feature_supported(FFA_MEM_DONATE_64);
    status_32 = val_is_ffa_feature_supported(FFA_MEM_DONATE_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "\tFFA_MEM_DONATE not supported, skipping the check\n", 0, 0);
        return VAL_SKIP_CHECK;
    }
    else if (status_64 && !status_32)
    {
        status = ffa_mem_donate_mmio_helper(test_run_data, FFA_MEM_DONATE_32);
    }
    else if (!status_64 && status_32)
    {
        status = ffa_mem_donate_mmio_helper(test_run_data, FFA_MEM_DONATE_64);
    }
    else
    {
        status = ffa_mem_donate_mmio_helper(test_run_data, FFA_MEM_DONATE_64);
        if (status)
            return status;

        status = ffa_mem_donate_mmio_helper(test_run_data, FFA_MEM_DONATE_32);
        if (status)
            return status;
    }
    return status;
}
