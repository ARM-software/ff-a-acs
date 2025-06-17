/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static void relinquish_memory(ffa_memory_handle_t handle, void *tx_buf, ffa_endpoint_id_t receiver)
{
    ffa_args_t payload;

    /* relinquish the memory and notify the sender. */
    ffa_mem_relinquish_init((struct ffa_mem_relinquish *)tx_buf, handle, 0, receiver, 0x1);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_mem_relinquish(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem relinquish failed err %x", payload.arg2);
    }
    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed");
    }
}

static uint32_t retrieve_zero_flag_check(ffa_memory_handle_t handle, uint32_t fid,
                void *tx_buf, ffa_endpoint_id_t sender, ffa_endpoint_id_t receiver,
                ffa_memory_region_flags_t flags)
{
    mem_region_init_t mem_region_init;
    uint32_t msg_size;
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = receiver;
    mem_region_init.tag = 0;
    mem_region_init.flags = flags;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
#if (PLATFORM_FFA_V == FFA_V_1_0)
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
#else
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
#endif
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_NON_CACHEABLE;
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
    if (fid == FFA_MEM_SHARE_64)
        val_ffa_mem_retrieve_64(&payload);
    else
        val_ffa_mem_retrieve_32(&payload);

    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR,
                "Relayer must return error if zero memory before retrieval flag is set",
                0, 0);
        status =  VAL_ERROR_POINT(1);
        if (payload.fid == FFA_MEM_RETRIEVE_RESP_32)
        {
            relinquish_memory(handle, tx_buf, receiver);
        }
   }
    LOG(DBG, "Mem Retrieve Check zero memory before retrieval Complete");

    return status;
}

static uint32_t retrieve_with_invalid_mem_transaction_type_check(ffa_memory_handle_t handle,
        uint32_t fid, void *tx_buf, ffa_endpoint_id_t sender, ffa_endpoint_id_t receiver)
{
    mem_region_init_t mem_region_init;
    uint32_t msg_size;
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = receiver;
    mem_region_init.tag = 0;
    /* Relayer must return INVALID_PARAMETERS if the transaction type specified by the
     * Receiver is not the same as that specified by the Sender for the memory region
     * identified by the Handle value specified in the transaction descriptor
     */
    mem_region_init.flags = (FFA_MEMORY_REGION_TRANSACTION_TYPE_DONATE <<
                                FFA_MEMORY_REGION_TRANSACTION_TYPE_MASK);
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
#if (PLATFORM_FFA_V == FFA_V_1_0)
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
#else
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
#endif
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_NON_CACHEABLE;
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
    if (fid == FFA_MEM_SHARE_64)
        val_ffa_mem_retrieve_64(&payload);
    else
        val_ffa_mem_retrieve_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Relayer must return %x instead of %x for invalid transaction type flag",
                  FFA_ERROR_INVALID_PARAMETERS, payload.arg2);
        status =  VAL_ERROR_POINT(2);
        if (payload.fid == FFA_MEM_RETRIEVE_RESP_32)
        {
            relinquish_memory(handle, tx_buf, receiver);
        }
    }

    return status;
}


uint32_t lend_retrieve_input_checks2_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    uint32_t fid = (uint32_t)args.arg4;
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint8_t *ptr;
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

    /* Invalid memory management transaction type flag check */
    status = retrieve_with_invalid_mem_transaction_type_check(handle, fid, mb.send, receiver,
                        sender);
    if (status)
        goto rxtx_unmap;

    /* Zero memory before retrieval flag - b'1: Retrieve
     * the memory region only if the Sender requested
     * the Relayer to zero its contents prior to retrieval
     * by setting the Bit[0].
     *
     * Look for error as lender hasn't set the zero memory flag.
     */
    status = retrieve_zero_flag_check(handle, fid, mb.send, receiver,
                        sender, FFA_MEMORY_REGION_FLAG_CLEAR);
    if (status)
        goto rxtx_unmap;

    /* Now retrieve using correct inputs */
    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = receiver;
    mem_region_init.receiver = sender;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
#if (PLATFORM_FFA_V == FFA_V_1_0)
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
#else
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
#endif
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

    val_memset(pages, 0xab, size);
    memory_region = (struct ffa_memory_region *)mb.recv;
    /* In an invocation of FFA_MEM_RETRIEVE_RESP during a transaction to lend or
     * donate memory, this flag is used by the Relayer to specify whether the memory
     * region was retrieved with or without zeroing its contents first.
     */
    flags = memory_region->flags;
    flags = VAL_EXTRACT_BITS(flags, 0, 0);
    if (flags)
    {
        LOG(ERROR, "Relayer must set Zero memory before retrieval flag bit[0]=0");
        status =  VAL_ERROR_POINT(8);
        goto rx_release;
    }

    composite = ffa_memory_region_get_composite(memory_region, 0);
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
        status = status ? status : VAL_ERROR_POINT(9);
    }

    if (val_memcmp(pages, ptr, size))
    {
        LOG(ERROR, "Data mismatch");
        status = status ? status : VAL_ERROR_POINT(10);
    }

    /* relinquish the memory with zero flag set. */
    ffa_mem_relinquish_init((struct ffa_mem_relinquish *)mb.send, handle, 0x1, sender, 0x1);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_mem_relinquish(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem relinquish failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(11);
        goto rx_release;
    }

rx_release:
    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed");
        status = status ? status : VAL_ERROR_POINT(12);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(13);
    }

free_memory:
    if (val_free(mb.recv) || val_free(mb.send))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(14);
    }

    if (val_free(pages))
    {
        LOG(ERROR, "val_free failed");
        status = status ? status : VAL_ERROR_POINT(15);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(16);
    }

    return status;
}
