/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t ffa_mem_share_helper(uint32_t test_run_data, uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);
    ffa_endpoint_id_t recipient_1;
    uint32_t test_run_data_1;
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    ffa_memory_region_flags_t flags = 0;
    ffa_memory_handle_t handle;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);
#if (PLATFORM_FFA_V >= FFA_V_1_1)
    uint32_t borrower_id_list = 0;
#endif


    if (!VAL_IS_ENDPOINT_SECURE(client_logical_id) &&
        !VAL_IS_ENDPOINT_SECURE(server_logical_id))
    {
        recipient_1 = val_get_endpoint_id(VM3);
        test_run_data_1 = SET_SERVER_LOGIC_ID(test_run_data, VM3);
    }
    else
    {
        recipient_1 = val_get_endpoint_id(SP3);
        test_run_data_1 = SET_SERVER_LOGIC_ID(test_run_data, SP3);
    }

    mb.send = val_aligned_alloc(PAGE_SIZE_4K, size);
    mb.recv = val_aligned_alloc(PAGE_SIZE_4K, size);
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

    pages = (uint8_t *)val_aligned_alloc(PAGE_SIZE_4K, size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        status = VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

    val_memset(pages, 0x0, size);
    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = flags;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
#if (PLATFORM_OUTER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#elif (PLATFORM_INNER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_INNER_SHAREABLE;
#elif (PLATFORM_INNER_OUTER_SHAREABLE_SUPPORT == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#endif
    mem_region_init.multi_share = true;
    mem_region_init.receiver_count = 2;
    mem_region_init.receivers[0].receiver_permissions.receiver = recipient;
    mem_region_init.receivers[0].receiver_permissions.permissions = FFA_DATA_ACCESS_RW;
    mem_region_init.receivers[0].receiver_permissions.flags = 0;
    mem_region_init.receivers[1].receiver_permissions.receiver = recipient_1;
    mem_region_init.receivers[1].receiver_permissions.permissions = FFA_DATA_ACCESS_RW;
    mem_region_init.receivers[1].receiver_permissions.flags = 0;

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
        LOG(ERROR, "Multi-endpoint Mem_share request failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }
    LOG(DBG, "Mem Share Complete");

#if (PLATFORM_FFA_V == FFA_V_1_0)
    val_select_server_fn_direct(test_run_data, fid, 0, 0, 0);
    val_select_server_fn_direct(test_run_data_1, fid, 0, 0, 0);
#else
    /*Encode Borrower ID's for Retrieval*/
    borrower_id_list = (uint32_t)(recipient_1 << 16) | recipient;

    val_select_server_fn_direct(test_run_data, fid, borrower_id_list, 0, 0);
    val_select_server_fn_direct(test_run_data_1, fid, borrower_id_list, 0, 0);
#endif

    /* Check that lender has Owner-SA state after FFA_MEM_SHARE call */
    val_memset(pages, 0xab, size);

    handle = ffa_mem_success_handle(payload);

    /* Pass memory handle to the server using direct message */
    status = val_ffa_mem_handle_share(sender, recipient, handle);
    if (status)
    {
        status = VAL_ERROR_POINT(5);
        goto exit;
    }

    /* Pass memory handle to the server-1 using direct message */
    status = val_ffa_mem_handle_share(sender, recipient_1, handle);
    if (status)
    {
        status = VAL_ERROR_POINT(6);
        goto exit;
    }

    /* Sender has already shared memory region to recipient and recipient_1.
     * Try to share the same memory region to recipient again and check for error status code.
     */
    mem_region_init.multi_share = true;
    mem_region_init.receiver_count = 2;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;
    if (fid == FFA_MEM_SHARE_64)
        val_ffa_mem_share_64(&payload);
    else
        val_ffa_mem_share_32(&payload);

    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_share must return error for invalid memory state");
        status = VAL_ERROR_POINT(8);
        goto exit;
    }

#if (PLATFORM_FFA_V == FFA_V_1_0)
    val_select_server_fn_direct(test_run_data, fid, 0, 0, 0);
    val_select_server_fn_direct(test_run_data_1, fid, 0, 0, 0);
#else
    /*Encode Borrower ID's for Retrieval*/
    borrower_id_list = (uint32_t)(recipient_1 << 16) | recipient;

    val_select_server_fn_direct(test_run_data, fid, borrower_id_list, 0, 0);
    val_select_server_fn_direct(test_run_data_1, fid, borrower_id_list, 0, 0);
#endif

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)handle;
    payload.arg2 = (uint32_t)(handle >> 32);
    payload.arg3 = 0;
    val_ffa_mem_reclaim(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem Reclaim failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(9);
    }
    LOG(DBG, "Mem Reclaim Complete");

exit:
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    status = status ? status : (uint32_t)payload.arg3;
    payload = val_select_server_fn_direct(test_run_data_1, 0, 0, 0, 0);
    status = status ? status : (uint32_t)payload.arg3;

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(10);
    }

free_memory:
    if (val_free(mb.recv) || val_free(mb.send))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(11);
    }

    if (val_free(pages))
    {
        LOG(ERROR, "val_free failed");
        status = status ? status : VAL_ERROR_POINT(12);
    }

    return status;
}

uint32_t share_state_machine_6_client(uint32_t test_run_data)
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
        status = ffa_mem_share_helper(test_run_data, FFA_MEM_SHARE_32);
    }
    else if (!status_64 && status_32)
    {
        status = ffa_mem_share_helper(test_run_data, FFA_MEM_SHARE_64);
    }
    else
    {
        status = ffa_mem_share_helper(test_run_data, FFA_MEM_SHARE_64);
        if (status)
            return status;

        status = ffa_mem_share_helper(test_run_data, FFA_MEM_SHARE_32);
        if (status)
            return status;
    }

    return status;
}
