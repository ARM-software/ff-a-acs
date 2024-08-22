/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t mem_share_device_attr_check(uint32_t test_run_data, uint32_t fid,
                void *tx_buf, enum ffa_memory_cacheability device_attr)
{
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t receiver = val_get_endpoint_id(server_logical_id);
    mem_region_init_t mem_region_init;
    ffa_memory_handle_t handle;
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        return VAL_ERROR_POINT(1);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = receiver;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_DEVICE_MEM;
    /* Device memory attribute precedence rules are as follows:
     * Device-nGnRnE < Device-nGnRE < Device-nGRE < Device-GRE < Normal.
     */
    mem_region_init.cacheability = device_attr;
    mem_region_init.shareability = FFA_MEMORY_SHARE_NON_SHAREABLE;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_SHARE_64)
        val_ffa_mem_share_64(&payload);
    else
        val_ffa_mem_share_32(&payload);

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_share request failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }
    LOG(DBG, "Mem Share Complete");

    handle = ffa_mem_success_handle(payload);

    /* Pass memory handle to the server using direct message */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    payload.arg3 =  handle;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(3);
        goto free_memory;
    }

    if (payload.arg3 != VAL_SUCCESS)
    {
        status = VAL_ERROR_POINT(4);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)handle;
    payload.arg2 = (uint32_t)(handle >> 32);
    payload.arg3 = 0;
    val_ffa_mem_reclaim(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "MEM_RECLAIM failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(5);
    }
    LOG(DBG, "Mem Reclaim Complete");

free_memory:
    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(6);
    }

    return status;
}

static uint32_t ffa_mem_share_handle_helper(uint32_t test_run_data, uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    mb_buf_t mb;
    uint64_t size = 0x1000;

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(7);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(8);
        goto free_memory;
    }

    val_select_server_fn_direct(test_run_data, fid, 0, 0, 0);

    status = mem_share_device_attr_check(test_run_data, fid, mb.send, FFA_MEMORY_DEV_NGNRNE);
    if (status)
        goto rxtx_unmap;

    status = mem_share_device_attr_check(test_run_data, fid, mb.send, FFA_MEMORY_DEV_NGNRE);
    if (status)
        goto rxtx_unmap;

    status = mem_share_device_attr_check(test_run_data, fid, mb.send, FFA_MEMORY_DEV_NGRE);

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(9);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(10);
    }

    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    return status ? status : (uint32_t)payload.arg3;
}

uint32_t share_device_attr1_client(uint32_t test_run_data)
{
    uint32_t status_32, status_64, status;

    status_64 = val_is_ffa_feature_supported(FFA_MEM_SHARE_64);
    status_32 = val_is_ffa_feature_supported(FFA_MEM_SHARE_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "FFA_MEM_SHARE not supported, skipping the check");
        return VAL_SKIP_CHECK;
    }
    else if (status_64 && !status_32)
    {
        status = ffa_mem_share_handle_helper(test_run_data, FFA_MEM_SHARE_32);
    }
    else if (!status_64 && status_32)
    {
        status = ffa_mem_share_handle_helper(test_run_data, FFA_MEM_SHARE_64);
    }
    else
    {
        status = ffa_mem_share_handle_helper(test_run_data, FFA_MEM_SHARE_64);
        if (status)
            return status;

        status = ffa_mem_share_handle_helper(test_run_data, FFA_MEM_SHARE_32);
        if (status)
            return status;
    }
    return status;
}
