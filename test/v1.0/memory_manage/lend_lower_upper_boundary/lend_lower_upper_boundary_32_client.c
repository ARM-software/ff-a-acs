/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t lend_lower_upper_boundary_32_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    ffa_memory_handle_t handle;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[2];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    if (val_is_ffa_feature_supported(FFA_MEM_LEND_32))
    {
        LOG(TEST, "FFA_MEM_LEND_32 not supported, skipping the check");
        return VAL_SKIP_CHECK;
    }

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }
    val_memset(mb.send, 0, size);

    /* Allocate 8KB page */
    pages = (uint8_t *)val_memory_alloc(size * 2);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        status = VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;
    constituents[1].address = val_mem_virt_to_phys((void *)pages + PAGE_SIZE_4K * 1);
    constituents[1].page_count = 1;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NOT_SPECIFIED_MEM;
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;
    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;
    val_ffa_mem_lend_32(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_lend request failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }
    LOG(DBG, "Mem Lend Complete");

    handle = ffa_mem_success_handle(payload);

    /* Pass memory handle to the server using direct message */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    payload.arg3 =  handle;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)handle;
    payload.arg2 = (uint32_t)(handle >> 32);
    payload.arg3 = 0;
    val_ffa_mem_reclaim(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem Reclaim failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(6);
    }
    LOG(DBG, "Mem Reclaim Complete");

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(7);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(8);
    }

    if (val_memory_free(pages, size * 2))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(9);
    }

    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    return status ? status : (uint32_t)payload.arg3;
}

uint32_t lend_lower_upper_boundary_32_vmsp_client(uint32_t test_run_data)
{
    return lend_lower_upper_boundary_32_client(test_run_data);
}

uint32_t lend_lower_upper_boundary_32_spsp_client(uint32_t test_run_data)
{
    return lend_lower_upper_boundary_32_client(test_run_data);
}

uint32_t lend_lower_upper_boundary_32_vmvm_client(uint32_t test_run_data)
{
    return lend_lower_upper_boundary_32_client(test_run_data);
}
