/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t mem_donate_back_to_sender(ffa_memory_handle_t handle, uint32_t fid,
                uint32_t tag, mb_buf_t mb, ffa_endpoint_id_t sender, ffa_endpoint_id_t receiver)
{
    mem_region_init_t mem_region_init;
    uint32_t msg_size;
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    struct ffa_memory_region *memory_region;
    struct ffa_composite_memory_region *composite;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = receiver;
    mem_region_init.tag = tag;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
#if (PLATFORM_FFA_V == FFA_V_1_0)
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
#else
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
#endif
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
    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_retrieve_64(&payload);
    else
        val_ffa_mem_retrieve_32(&payload);

    if (payload.fid != FFA_MEM_RETRIEVE_RESP_32)
    {
        LOG(ERROR, "Mem retrieve request failed err %x\n", payload.arg2);
        status =  VAL_ERROR_POINT(1);
        goto err;
    }

    memory_region = (struct ffa_memory_region *)mb.recv;
    composite = ffa_memory_region_get_composite(memory_region, 0);
    constituents[0].address = composite->constituents[0].address;
    constituents[0].page_count = 1;

    /* Give up ownership back to the sender */
    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = receiver;
    mem_region_init.receiver = sender;
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
    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_donate request failed err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto rx_release;
    }

    LOG(DBG, "Mem Donate Complete\n");

    handle = ffa_mem_success_handle(payload);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)receiver << 16) | sender;
    payload.arg3 =  handle;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
    {
        LOG(ERROR, "Direct request failed fid %x err %x\n", payload.fid, payload.arg2);
        status = status ? status : VAL_ERROR_POINT(3);
    }

rx_release:
    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed\n");
        status = status ? status : VAL_ERROR_POINT(4);
    }

err:
    return status;
}

static uint32_t mem_donate_invalid_handle_tag_check(ffa_memory_handle_t handle, uint32_t fid,
                uint32_t tag, void *tx_buf, ffa_endpoint_id_t sender, ffa_endpoint_id_t receiver)
{
    mem_region_init_t mem_region_init;
    uint32_t msg_size;
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;

    /* MEM_DONATE: Relayer must ensure that the handle was allocated to the owner
     * specified in the sender endpoint ID field of the transaction descriptor
     */
    /* MEM_DONATE: The Relayer must ensure the Tag value specified by the Receiver
     * is equal to the value that was specified by the Sender.
     */
    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = receiver;
    mem_region_init.tag = tag;
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
    msg_size = val_ffa_memory_retrieve_request_init(&mem_region_init, handle);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = msg_size;
    payload.arg2 = msg_size;
    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_retrieve_64(&payload);
    else
        val_ffa_mem_retrieve_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Relayer must return error for invalid handle and tag err %x\n",
                        payload.arg2);
        status =  VAL_ERROR_POINT(5);
    }
    LOG(DBG, "Mem Retrieve Check Invalid Handle Complete\n");

    return status;
}

uint32_t donate_invalid_handle_tag_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t status1 = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    uint32_t fid = (uint32_t)args.arg4;
    uint32_t tag;
    mb_buf_t mb;
    uint64_t size = 0x1000;
    ffa_memory_handle_t handle;

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer\n");
        status = VAL_ERROR_POINT(6);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed\n");
        status = VAL_ERROR_POINT(7);
        goto free_memory;
    }

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(8);
        goto rxtx_unmap;
    }

    handle = payload.arg3;
    tag = (uint32_t)payload.arg4;
    /* Pass invalid handle from the receiver and check for error status code */
    status = mem_donate_invalid_handle_tag_check((handle - 0xff), fid, tag, mb.send,
                                                receiver, sender);
    if (status)
        goto mem_donate;

    /* Pass invalid tag from the receiver and check for error status code */
    status = mem_donate_invalid_handle_tag_check(handle, fid, (tag - 0xff), mb.send,
                                                receiver, sender);

mem_donate:
    /* Give up ownership back to the sender */
    status1 = mem_donate_back_to_sender(handle, fid, tag, mb, receiver, sender);
    status = status ? status : status1;

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed\n");
        status = status ? status : VAL_ERROR_POINT(9);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(10);
    }

    return status;
}
