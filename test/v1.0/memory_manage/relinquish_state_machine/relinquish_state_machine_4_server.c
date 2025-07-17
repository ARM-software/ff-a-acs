/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t relinquish_state_machine_4_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff, curr_ep_logical_id;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    uint32_t fid = (uint32_t)args.arg4;
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint8_t *ptr;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region *memory_region;
    struct ffa_composite_memory_region *composite;
    memory_region_descriptor_t mem_desc;
    ffa_memory_handle_t handle;
    uint32_t msg_size, relinquish_flag = 0;
#if (PLATFORM_FFA_V >= FFA_V_1_1)
    uint32_t borrower_list = 0;
    uint16_t borrower_1 = 0;
    uint16_t borrower_2 = 0;
#endif

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer\n");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed\n");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }
    val_memset(mb.send, 0, size);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed\n");
        status = VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

    val_memset(pages, 0xab, size);
    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    handle = payload.arg3;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = receiver;
    mem_region_init.receiver = sender;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
    mem_region_init.multi_share = true;
    mem_region_init.receiver_count = 2;

#if (PLATFORM_FFA_V >= FFA_V_1_1)
    borrower_list = (uint32_t)args.arg5;
    borrower_1 = (uint16_t)(borrower_list & 0xFFFF);
    borrower_2 = (uint16_t)((borrower_list >> 16) & 0xFFFF);

    mem_region_init.receivers[0].receiver_permissions.receiver = borrower_1;
    mem_region_init.receivers[0].receiver_permissions.permissions = FFA_DATA_ACCESS_RW;
    mem_region_init.receivers[0].receiver_permissions.flags = 0;
    mem_region_init.receivers[1].receiver_permissions.receiver  = borrower_2;
    mem_region_init.receivers[1].receiver_permissions.permissions = FFA_DATA_ACCESS_RW;
    mem_region_init.receivers[1].receiver_permissions.flags = 0;
#endif

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

    if (payload.fid != FFA_MEM_RETRIEVE_RESP_32)
    {
        LOG(ERROR, "Mem retrieve request failed err %x\n", payload.arg2);
        status =  VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }
    LOG(DBG, "Mem Retrieve Complete\n");

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(6);
        goto rx_release;
    }

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
        LOG(ERROR, "Va to pa mapping failed\n");
        status =  VAL_ERROR_POINT(7);
        goto rx_release;
    }

    if (val_memcmp(pages, ptr, size))
    {
        LOG(ERROR, "Data mismatch\n");
        status =  VAL_ERROR_POINT(8);
        goto rx_release;
    }

    /* Zero memory after relinquish flag- If the memory region is
     * lent to multiple Borrowers, the Relayer must clear memory
     * region contents after unmapping it from each Borrower's
     * translation regime, if any Borrower including the caller
     * sets this flag.
     * SP2(or VM2) sets this flag to check relayer clears the
     * memory region after relinquish.
     * */
    curr_ep_logical_id = val_get_curr_endpoint_logical_id();
    if (curr_ep_logical_id == VM3 || curr_ep_logical_id == SP3)
        relinquish_flag = FFA_MEMORY_REGION_FLAG_CLEAR;

    /* relinquish the memory and notify the sender. */
    ffa_mem_relinquish_init((struct ffa_mem_relinquish *)mb.send,
        handle, relinquish_flag, sender, 0x1);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_mem_relinquish(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem relinquish failed err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(9);
    }
    LOG(DBG, "Mem Relinquish Complete\n");

rx_release:
    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed\n");
        status = status ? status : VAL_ERROR_POINT(10);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed\n");
        status = status ? status : VAL_ERROR_POINT(11);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed\n");
        status = status ? status : VAL_ERROR_POINT(12);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(13);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(14);
    }

    return status;
}
