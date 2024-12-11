/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t lend_multiple_retrievals_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t outstanding_retrieve_count = 0;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    uint32_t fid = (uint32_t)args.arg4;
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint8_t *ptr;
    uint32_t i, j;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region *memory_region;
    struct ffa_composite_memory_region *composite;
    ffa_memory_handle_t handle;
    uint32_t msg_size;
    memory_region_descriptor_t mem_desc;

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

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        status = VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    if (fid == FFA_MEM_LEND_64)
        payload.arg1 = FFA_MEM_RETRIEVE_REQ_64;
    else
        payload.arg1 = FFA_MEM_RETRIEVE_REQ_32;

#if (PLATFORM_FFA_V >= FFA_V_1_1)
    payload.arg2 = 0x2;
#endif

    val_ffa_features(&payload);

    if (payload.fid == FFA_ERROR_32 || (payload.arg2 == FFA_ERROR_NOT_SUPPORTED))
    {
        LOG(ERROR, "RETRIEVE_REQ not supported %x .", payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

    if (payload.fid == FFA_SUCCESS_32 || payload.fid == FFA_SUCCESS_64)
    {
       /* Check for Outstanding retrievals field [7:0] */
#if (PLATFORM_FFA_V >= FFA_V_1_1)
       outstanding_retrieve_count = VAL_EXTRACT_BITS(payload.arg3, 0, 7);
#else
       outstanding_retrieve_count = VAL_EXTRACT_BITS(payload.arg1, 0, 7);
#endif
       outstanding_retrieve_count = (1U << (outstanding_retrieve_count + 1)) - 1;
       LOG(TEST, "Outstanding retrievals count %d", outstanding_retrieve_count);
    }

    handle = payload.arg3;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = receiver;
    mem_region_init.receiver = sender;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
#if (PLATFORM_FFA_V >= FFA_V_1_1)
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
#else
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
#endif
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
#if (PLATFORM_OUTER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#elif (PLATFORM_INNER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_INNER_SHAREABLE;
#elif (PLATFORM_INNER_OUTER_SHAREABLE_SUPPORT == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#endif
    msg_size = val_ffa_memory_retrieve_request_init(&mem_region_init, handle);

    for (j = 0; j < outstanding_retrieve_count ; j++)
    {
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
            status =  VAL_ERROR_POINT(6);
            goto rxtx_unmap;
        }

        if (val_rx_release())
        {
            LOG(ERROR, "val_rx_release failed");
            status = status ? status : VAL_ERROR_POINT(7);
        }
    }

    val_memset(pages, 0xab, size);
    memory_region = (struct ffa_memory_region *)mb.recv;
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
        status =  VAL_ERROR_POINT(8);
    }

    if (val_memcmp(pages, ptr, size))
    {
        LOG(ERROR, "Data mismatch");
        status =  VAL_ERROR_POINT(9);
    }

    /* Check memory write access. */
    for (i = 0; i < size; ++i)
    {
        ptr[i] = 1;
    }

    /* Zero memory before retrieval flag:
     * MBZ if the Receiver has previously retrieved this memory region,
     * else the Relayer must return INVALID_PARAMETERS
     */
    if (outstanding_retrieve_count > 1)
    {
        mem_region_init.flags = FFA_MEMORY_REGION_FLAG_CLEAR;
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = msg_size;
        payload.arg2 = msg_size;
        if (fid == FFA_MEM_LEND_64)
            val_ffa_mem_retrieve_64(&payload);
        else
            val_ffa_mem_retrieve_32(&payload);

        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
        {
            LOG(ERROR, "FFA_MEM_RETRIEVE_REQ  err %x %x", payload.arg2, payload.fid);
            status =  VAL_ERROR_POINT(10);
            goto rxtx_unmap;
        }
    }

    /* Try to retreive the memory region more than supported count
     * and check for DENIED status code
     */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = msg_size;
    payload.arg2 = msg_size;
    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_retrieve_64(&payload);
    else
        val_ffa_mem_retrieve_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "Mem retrieve request should fail for more than supported count  err %x",
                                                                               payload.arg2);
        status =  VAL_ERROR_POINT(11);
        goto rxtx_unmap;
    }
    LOG(DBG, "Mem Retrieve Check for Unsupported Count Complete");

   for (j = 0; j < outstanding_retrieve_count ; j++)
   {
        /* relinquish the memory and notify the sender. */
        ffa_mem_relinquish_init((struct ffa_mem_relinquish *)mb.send, handle, 0, sender, 0x1);
        val_memset(&payload, 0, sizeof(ffa_args_t));
        val_ffa_mem_relinquish(&payload);
        if (payload.fid == FFA_ERROR_32)
        {
            LOG(ERROR, "Mem relinquish failed err %x", payload.arg2);
            status = status ? status : VAL_ERROR_POINT(12);
        }
   }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(13);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(14);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
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
