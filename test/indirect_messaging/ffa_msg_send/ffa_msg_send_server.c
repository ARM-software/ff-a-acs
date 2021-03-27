/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_msg_send_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t output_reserve_count = 2, size = PAGE_SIZE_4K;
    ffa_endpoint_id_t sender = (args.arg1 >> 16) & 0xFFFF;
    char message[] = "FFA ACS suite";
    mb_buf_t mb;

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, PAGE_SIZE_4K))
    {
        LOG(ERROR, "\tRxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg1, 0, 0, 0, 0, 0);

    /* ffa_msg_send_direct_resp(): Blocks the caller until message is
     * available in the caller's RX buffer */

    if (payload.fid != FFA_MSG_SEND_32)
    {
        LOG(ERROR, "\tWrong fid recieved %x\n", payload.fid, 0);
        status = VAL_ERROR_POINT(3);
    }

    /* Check the sender and reciever id */
    if (payload.arg1 != args.arg1)
    {
        LOG(ERROR, "\tSender-reciever epid check failed, actual=%x but expected\n",
            payload.arg1, args.arg1);
        status = VAL_ERROR_POINT(4);
    }

    /* Check the message size */
    if (payload.arg3 != sizeof(message))
    {
        LOG(ERROR, "\tmsg size mismatch actual=%x but expected\n",
            payload.arg3, sizeof(message));
        status = VAL_ERROR_POINT(5);
    }

    /* Check the message content */
    if (val_memcmp(mb.recv, message, sizeof(message)))
    {
        LOG(ERROR, "\tmsg content mismatched\n", 0, 0);
        status = VAL_ERROR_POINT(6);
    }

    /* Return value for reserved registers - MBZ. w6 and w7 for ffa_msg_send */
    if (val_reserve_param_check(payload, output_reserve_count))
    {
        LOG(ERROR, "\tReceived non-zero value for reserved registers\n",
            0, 0);
        status = VAL_ERROR_POINT(7);
    }

    if (val_rx_release())
    {
        LOG(ERROR, "\tval_rx_release failed\n", 0, 0);
        status = VAL_ERROR_POINT(8);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));

    /* ffa_msg_wait(): Blocks the caller until message is
     * available in the caller's RX buffer */
    val_ffa_msg_wait(&payload);

    if (payload.fid != FFA_MSG_SEND_32)
    {
        LOG(ERROR, "\tFFA_MSG_SEND failed, fid=0x%x, err %x\n",
                  payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(9);
    }

    /* Check the sender and reciever id */
    if (payload.arg1 != args.arg1)
    {
        LOG(ERROR, "\tSender-reciever epid check failed, actual=%x but expected\n",
            payload.arg1, args.arg1);
        status = VAL_ERROR_POINT(10);
    }

    /* Check the message size */
    if (payload.arg3 != sizeof(message))
    {
        LOG(ERROR, "\tmsg size mismatch actual=%x but expected\n",
            payload.arg3, sizeof(message));
        status = VAL_ERROR_POINT(11);
    }

    /* Check the message content */
    if (val_memcmp(mb.recv, message, sizeof(message)))
    {
        LOG(ERROR, "\tmsg content mismatched\n", 0, 0);
        status = VAL_ERROR_POINT(12);
    }

    /* Return value for reserved registers - MBZ. w6 and w7 for ffa_msg_send */
    if (val_reserve_param_check(payload, output_reserve_count))
    {
        LOG(ERROR, "\tReceived non-zero value for reserved registers\n",
            0, 0);
        status = VAL_ERROR_POINT(13);
    }

    if (val_rx_release())
    {
        LOG(ERROR, "\tval_rx_release failed\n", 0, 0);
        status = VAL_ERROR_POINT(14);
    }

    /* ffa_msg_wait: INVALID_PARAMETERS check
     * arg1: Endpoint and vCPU IDs are only valid
     * with the ERET conduit at the Non-secure virtual
     * FF-A instance else MBZ.
     * */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = 0xff;
    /* Executing ffa_msg_wait using non-eret conduit */
    val_ffa_msg_wait(&payload);
    if ((payload.fid != FFA_ERROR_32) && (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\t  ffa_msg_wait didn't fail\n", 0, 0);
        status = VAL_ERROR_POINT(15);
    }

    /* arg2 & arg3 are only valid at eret conduit at
     * the Non-secure virtual FF-A instance else MBZ. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg2 = 0xff;
    payload.arg3 = 0xff;
    val_ffa_msg_wait(&payload);
    if ((payload.fid != FFA_ERROR_32) && (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tffa_msg_wait didn't fail\n", 0, 0);
        status = VAL_ERROR_POINT(16);
    }

    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "\tRXTX_UNMAP failed\n", 0, 0);
        status = VAL_ERROR_POINT(17);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "\tfree_rxtx_buffers failed\n", 0, 0);
        status = VAL_ERROR_POINT(18);
    }

    return status;
}
