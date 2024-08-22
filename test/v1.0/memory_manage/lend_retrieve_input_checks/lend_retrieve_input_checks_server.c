/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
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

static uint32_t retrieve_zero_flag_check_for_ro_mem(ffa_memory_handle_t handle, uint32_t fid,
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
    /* Zero memory before retrieval flag: If the Sender has Read-only access to the memory
     * region and the Receiver sets Bit[0], the Relayer must return DENIED.
     *
     * Zero memory after relinquish flag: MBZ if the Receiver has Read-only access to the
     * memory region, else the Relayermust return DENIED. The Receiver could be a PE endpoint
     * or a dependentperipheral device.
     */
    mem_region_init.flags = flags;
    mem_region_init.data_access = FFA_DATA_ACCESS_RO;
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
#if (PLATFORM_FFA_V_1_0 == 1)
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_NON_CACHEABLE;
#else
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
#endif
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;
#if (PLATFORM_OUTER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#elif (PLATFORM_INNER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_INNER_SHAREABLE;
#elif (PLATFORM_INNER_OUTER_SHAREABLE_SUPPORT == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#endif
    msg_size = val_ffa_memory_retrieve_request_init(&mem_region_init, handle);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = msg_size;
    payload.arg2 = msg_size;
    if (fid == FFA_MEM_SHARE_64)
        val_ffa_mem_retrieve_64(&payload);
    else
        val_ffa_mem_retrieve_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        if (flags == FFA_MEMORY_REGION_FLAG_CLEAR)
        {
            LOG(ERROR,
                "Relayer must fail if bit-0 is set for RO memory, fid=%x, err=%x",
                payload.fid, payload.arg2);
        }
        else if (flags == FFA_MEMORY_REGION_FLAG_CLEAR_RELINQUISH)
        {
            LOG(ERROR,
                "Relayer must fail if bit-2 is set for RO memory, fid=%x, err=%x",
                payload.fid, payload.arg2);
        }
        status =  VAL_ERROR_POINT(1);
        if (payload.fid == FFA_MEM_RETRIEVE_RESP_32)
        {
            relinquish_memory(handle, tx_buf, receiver);
        }
   }
    LOG(DBG, "Mem Retrieve Check for Relinquish Flag Complete");

    return status;
}

uint32_t lend_retrieve_input_checks_server(ffa_args_t args)
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
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(3);
        goto free_memory;
    }

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        status = VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

    handle = payload.arg3;

    status = retrieve_zero_flag_check_for_ro_mem(handle, fid, mb.send, receiver,
                        sender, FFA_MEMORY_REGION_FLAG_CLEAR);
    if (status)
        goto rxtx_unmap;

    status = retrieve_zero_flag_check_for_ro_mem(handle, fid, mb.send, receiver,
                        sender, FFA_MEMORY_REGION_FLAG_CLEAR_RELINQUISH);
    if (status)
        goto rxtx_unmap;

    /* Now retrieve using correct inputs */
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = receiver;
    mem_region_init.receiver = sender;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RO;
#if (PLATFORM_FFA_V_1_0 == 1)
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
#else
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
#endif
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;
#if (PLATFORM_OUTER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#elif (PLATFORM_INNER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_INNER_SHAREABLE;
#elif (PLATFORM_INNER_OUTER_SHAREABLE_SUPPORT == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#endif
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
        status =  VAL_ERROR_POINT(6);
        goto rxtx_unmap;
    }
    LOG(DBG, "Mem Retrieve Complete");

    val_memset(pages, 0, size);
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
        status =  VAL_ERROR_POINT(7);
        goto rx_release;
    }

    if (val_memcmp(pages, ptr, size))
    {
        LOG(ERROR, "Data mismatch");
        status =  VAL_ERROR_POINT(8);
        goto rx_release;
    }

    /* relinquish the memory with zero flag set. */
    ffa_mem_relinquish_init((struct ffa_mem_relinquish *)mb.send, handle, 0x1, sender, 0x1);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_mem_relinquish(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "Mem relinquish must fail err %x", payload.arg2);
        status = VAL_ERROR_POINT(9);
        goto rx_release;
    }

    /* relinquish the memory with valid inputs and notify the sender. */
    ffa_mem_relinquish_init((struct ffa_mem_relinquish *)mb.send, handle, 0, sender, 0x1);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_mem_relinquish(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem relinquish failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(10);
        goto rx_release;
    }

rx_release:
    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed");
        status = status ? status : VAL_ERROR_POINT(11);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(12);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(13);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(14);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(15);
    }

    return status;
}
