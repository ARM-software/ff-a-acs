/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t borrower_to_share_memory(ffa_endpoint_id_t recipient, mb_buf_t mb, void *pages)
{
    ffa_args_t payload;
    uint32_t status = VAL_ERROR_POINT(1);
    ffa_endpoint_id_t sender = val_get_curr_endpoint_id();
    ffa_memory_handle_t handle;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);
    uint32_t status_32, status_64;

    status_64 = val_is_ffa_feature_supported(FFA_MEM_SHARE_64);
    status_32 = val_is_ffa_feature_supported(FFA_MEM_SHARE_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "FFA_MEM_SHARE not supported, skipping the check");
        return VAL_SKIP_CHECK;
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
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
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    /* Must validate that the memory region is in the Owner-EA state
     * for the Lender. It must return DENIED in case of an error. */
    if (!status_64)
        val_ffa_mem_share_64(&payload);
    else
        val_ffa_mem_share_32(&payload);

    if (payload.fid == FFA_ERROR_32 && payload.arg2 == FFA_ERROR_DENIED)
        return VAL_SUCCESS;

    LOG(ERROR, "MEM_SHARE request must fail with DENIED err %x", payload.arg2);

    if (payload.fid == FFA_SUCCESS_32 || payload.fid == FFA_SUCCESS_64)
    {
        handle = ffa_mem_success_handle(payload);
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = (uint32_t)handle;
        payload.arg2 = (uint32_t)(handle >> 32);
        payload.arg3 = 0;
        val_ffa_mem_reclaim(&payload);
        if (payload.fid == FFA_ERROR_32)
        {
            LOG(ERROR, "Mem Reclaim failed err %x", payload.arg2);
        }
    }

    return status;
}

static uint32_t borrower_to_donate_memory(ffa_endpoint_id_t recipient, mb_buf_t mb, void *pages)
{
    ffa_args_t payload;
    uint32_t status = VAL_ERROR_POINT(2);
    ffa_endpoint_id_t sender = val_get_curr_endpoint_id();
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);
    uint32_t status_32, status_64;

    status_64 = val_is_ffa_feature_supported(FFA_MEM_DONATE_64);
    status_32 = val_is_ffa_feature_supported(FFA_MEM_DONATE_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "FFA_MEM_DONATE not supported, skipping the check");
        return VAL_SKIP_CHECK;
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_NOT_SPECIFIED;
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

    /* Must validate that the memory region is in the Owner-EA state
     * for the Lender. It must return DENIED in case of an error. */
    if (!status_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);

    if (payload.fid == FFA_ERROR_32 && payload.arg2 == FFA_ERROR_DENIED)
        return VAL_SUCCESS;

    LOG(ERROR, "MEM_DONATE request must fail with DENIED%x", payload.arg2);

    return status;
}

uint32_t ffa_mem_lend_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_endpoint_id_t recipient_1;
    uint32_t fid = (uint32_t)args.arg4;
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint8_t *ptr;
    uint32_t i;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    ffa_memory_region_flags_t flags;
    struct ffa_memory_region *memory_region;
    struct ffa_composite_memory_region *composite;
    ffa_memory_handle_t handle;
    uint32_t msg_size;
    memory_region_descriptor_t mem_desc;

    mb.send = val_aligned_alloc(PAGE_SIZE_4K, size);
    mb.recv = val_aligned_alloc(PAGE_SIZE_4K, size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(3);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(4);
        goto free_memory;
    }

    pages = (uint8_t *)val_aligned_alloc(PAGE_SIZE_4K, size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        status = VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(6);
        goto rxtx_unmap;
    }

    handle = payload.arg3;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = receiver;
    mem_region_init.receiver = sender;
    mem_region_init.tag = 0;
    /* Set FFA_MEMORY_REGION_FLAG_CLEAR_RELINQUISH to check this flag
     * could be overridden by the Receiver in an invocation of
     * FFA_MEM_RELINQUISH ABI.
     * Relay must not clear the memory after relinquish as this flag is set
     * to zero as part of FFA_MEM_RELINQUISH ABI
     * */
    mem_region_init.flags = FFA_MEMORY_REGION_FLAG_CLEAR_RELINQUISH;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
#if (PLATFORM_OUTER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#elif (PLATFORM_INNER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_INNER_SHAREABLE;
#elif (PLATFORM_INNER_OUTER_SHAREABLE_SUPPORT == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#endif
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

    msg_size = val_ffa_memory_retrieve_request_init(&mem_region_init, handle);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = msg_size;
    payload.arg2 = msg_size;
    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_retrieve_64(&payload);
    else
        val_ffa_mem_retrieve_32(&payload);

    if (payload.fid != FFA_MEM_RETRIEVE_RESP_32)
    {
        LOG(ERROR, "Mem retrieve request failed err %x", payload.arg2);
        status =  VAL_ERROR_POINT(7);
        goto rxtx_unmap;
    }
    LOG(DBG, "Mem Retrieve Complete");

    val_memset(pages, 0xab, size);
    memory_region = (struct ffa_memory_region *)mb.recv;
    composite = ffa_memory_region_get_composite(memory_region, 0);
    /* Memory management transaction type flag Bit[4:3] check */
    flags = memory_region->flags;
    flags = VAL_EXTRACT_BITS(flags, 3, 4);
    if (flags != FFA_MEMORY_REGION_TRANSACTION_TYPE_LEND)
    {
        LOG(ERROR, "Invalid memory management transaction type flag %x", flags);
        status = VAL_ERROR_POINT(8);
        goto rx_release;
    }

    ptr = (uint8_t *)composite->constituents[0].address;

    /* Map the region into endpoint translation */
    mem_desc.virtual_address = (uint64_t)composite->constituents[0].address;
    mem_desc.physical_address = (uint64_t)composite->constituents[0].address;
    mem_desc.length = size;
    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(receiver)))
        mem_desc.attributes = ATTR_RW_DATA;
    else
        mem_desc.attributes = ATTR_RW_DATA | ATTR_NS;

    if (val_mem_map_pgt(&mem_desc))
    {
        LOG(ERROR, "Va to pa mapping failed");
        status =  VAL_ERROR_POINT(9);
        goto rx_release;
    }

    if (val_memcmp(pages, ptr, size))
    {
        LOG(ERROR, "Data mismatch");
        status =  VAL_ERROR_POINT(10);
        goto rx_release;
    }

    /* Check memory write access. */
    for (i = 0; i < size; ++i)
    {
        ptr[i] = 1;
    }

    if (!VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(sender)) &&
        !VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(receiver)))
    {
        recipient_1 = val_get_endpoint_id(VM3);
    }
    else
    {
        recipient_1 = val_get_endpoint_id(SP3);
    }

    /* Check that borrower can't share memory to others */
    status = borrower_to_share_memory(recipient_1, mb, ptr);
    if (status)
    {
        status = VAL_ERROR_POINT(11);
        goto relinquish_mem;
    }
    LOG(DBG, "Borrower Share Memory Check Complete");

    /* Check that borrower can't donate memory to others */
    status = borrower_to_donate_memory(recipient_1, mb, ptr);
    if (status)
        status = VAL_ERROR_POINT(12);
    LOG(DBG, "Borrower Donate Memory Check Complete");

relinquish_mem:
    /* relinquish the memory and notify the sender. */
    ffa_mem_relinquish_init((struct ffa_mem_relinquish *)mb.send, handle, 0, sender, 0x1);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_mem_relinquish(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem relinquish failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(13);
        goto rx_release;
    }

rx_release:
    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed");
        status = status ? status : VAL_ERROR_POINT(14);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(15);
    }

free_memory:
    if (val_free(mb.recv) || val_free(mb.send))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(16);
    }

    if (val_free(pages))
    {
        LOG(ERROR, "val_free failed");
        status = status ? status : VAL_ERROR_POINT(17);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(18);
    }

    return status;
}
