/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t mem_lend_device_attr_check(ffa_memory_handle_t handle, uint32_t fid,
                void *tx_buf, ffa_endpoint_id_t sender, ffa_endpoint_id_t receiver,
                enum ffa_memory_cacheability device_attr)
{
    mem_region_init_t mem_region_init;
    uint32_t msg_size;
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;

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
     * The Relayer must return the DENIED error code if the validation fails.
     */
    mem_region_init.cacheability = device_attr;
    mem_region_init.shareability = FFA_MEMORY_SHARE_NON_SHAREABLE;
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
        LOG(ERROR, "\tMem retrieve request failed err %x\n", payload.arg2, 0);
        return VAL_ERROR_POINT(1);
    }

    /* relinquish the memory and notify the sender. */
    ffa_mem_relinquish_init((struct ffa_mem_relinquish *)tx_buf, handle, 0, receiver, 0x1);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_mem_relinquish(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tMem relinquish failed err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(2);
    }

    if (val_rx_release())
    {
        LOG(ERROR, "\tval_rx_release failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(3);
    }

    return status;
}

uint32_t lend_device_attr_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_memory_handle_t handle;
    uint32_t fid = (uint32_t)args.arg4;
    mb_buf_t mb;
    uint64_t size = PAGE_SIZE_4K;

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        status = VAL_ERROR_POINT(4);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "\tRxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(5);
        goto free_memory;
    }

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(6);
        goto rxtx_unmap;
    }

    if (payload.arg4 != VAL_SUCCESS)
    {
        status = VAL_ERROR_POINT(7);
        goto rxtx_unmap;
    }

    handle = payload.arg3;
    /* Test-1: sender->FFA_MEMORY_DEV_GRE receiver->FFA_MEMORY_DEV_NGRE */
    status = mem_lend_device_attr_check(handle,
             fid, mb.send, receiver, sender, FFA_MEMORY_DEV_NGRE);
    if (status)
        goto rxtx_unmap;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(8);
        goto rxtx_unmap;
    }

    if (payload.arg4 != VAL_SUCCESS)
    {
        status = VAL_ERROR_POINT(9);
        goto rxtx_unmap;
    }

    handle = payload.arg3;
    /* Test-2: sender->FFA_MEMORY_DEV_GRE receiver->FFA_MEMORY_DEV_NGNRE */
    status = mem_lend_device_attr_check(handle, fid,
             mb.send, receiver, sender, FFA_MEMORY_DEV_NGNRE);
    if (status)
        goto rxtx_unmap;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(10);
        goto rxtx_unmap;
    }

    if (payload.arg4 != VAL_SUCCESS)
    {
        status = VAL_ERROR_POINT(11);
        goto rxtx_unmap;
    }

    handle = payload.arg3;
    /* Test-3: sender->FFA_MEMORY_DEV_GRE receiver->FFA_MEMORY_DEV_NGNRNE */
    status = mem_lend_device_attr_check(handle, fid,
             mb.send, receiver, sender, FFA_MEMORY_DEV_NGNRNE);
    if (status)
        goto rxtx_unmap;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(12);
        goto rxtx_unmap;
    }

    if (payload.arg4 != VAL_SUCCESS)
    {
        status = VAL_ERROR_POINT(13);
        goto rxtx_unmap;
    }

    handle = payload.arg3;
    /* Test-4: sender->FFA_MEMORY_DEV_NGRE receiver->FFA_MEMORY_DEV_NGNRE */
    status = mem_lend_device_attr_check(handle, fid,
             mb.send, receiver, sender, FFA_MEMORY_DEV_NGNRE);
    if (status)
        goto rxtx_unmap;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(14);
        goto rxtx_unmap;
    }

    if (payload.arg4 != VAL_SUCCESS)
    {
        status = VAL_ERROR_POINT(15);
        goto rxtx_unmap;
    }

    handle = payload.arg3;
    /* Test-5: sender->FFA_MEMORY_DEV_NGRE receiver->FFA_MEMORY_DEV_NGNRNE */
    status = mem_lend_device_attr_check(handle, fid,
             mb.send, receiver, sender, FFA_MEMORY_DEV_NGNRNE);
    if (status)
        goto rxtx_unmap;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(16);
        goto rxtx_unmap;
    }

    if (payload.arg4 != VAL_SUCCESS)
    {
        status = VAL_ERROR_POINT(17);
        goto rxtx_unmap;
    }

    handle = payload.arg3;
    /* Test-6: sender->FFA_MEMORY_DEV_NGNRE receiver->FFA_MEMORY_DEV_NGNRNE */
    status = mem_lend_device_attr_check(handle, fid,
             mb.send, receiver, sender, FFA_MEMORY_DEV_NGNRNE);

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(18);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "\tfree_rxtx_buffers failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(19);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    payload.arg3 = status;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tDirect response failed err %x\n", payload.arg2, 0);
        status = status ? status : VAL_ERROR_POINT(20);
    }

    return status;
}
