/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
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
    char message1[] = "FFA ACS suite is running";
    uint32_t msg_size = sizeof(message) - 0x1;
    mb_buf_t mb;

    mb.send = val_aligned_alloc(PAGE_SIZE_4K, size);
    mb.recv = val_aligned_alloc(PAGE_SIZE_4K, size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);

    /* ffa_msg_send_direct_resp(): Blocks the caller until message is
     * available in the caller's RX buffer */

    if (payload.fid != FFA_MSG_SEND_32)
    {
        LOG(ERROR, "Wrong fid recieved %x", payload.fid);
        status = VAL_ERROR_POINT(3);
    }

    /* Check the sender and reciever id */
    if (payload.arg1 != args.arg1)
    {
        LOG(ERROR, "Sender-reciever epid check failed, actual=%x but expected",
            payload.arg1, args.arg1);
        status = VAL_ERROR_POINT(4);
    }

    LOG(DBG, "Sender Receiver ID payload.arg1 %x args.arg1 %x", payload.arg1, args.arg1);

    /* Check the message size */
    if (payload.arg3 != msg_size)
    {
        LOG(ERROR, "msg size mismatch actual=%x but expected=%x",
            payload.arg3, msg_size);
        status = VAL_ERROR_POINT(5);
    }

    LOG(DBG, "Message Size %x", msg_size);

    /* Check that only the amount of data as specified
     * by the length is sent */
    if (val_memcmp(mb.recv, message, msg_size) ||
        !val_memcmp(mb.recv, message1, sizeof(message1)))
    {
        LOG(ERROR, "msg content mismatched");
        status = VAL_ERROR_POINT(6);
    }

    /* Return value for reserved registers - MBZ. w6 and w7 for ffa_msg_send */
    if (val_reserve_param_check(payload, output_reserve_count))
    {
        LOG(ERROR, "Received non-zero value for reserved registers");
        status = VAL_ERROR_POINT(7);
    }

    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed");
        status = VAL_ERROR_POINT(8);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));

    /* ffa_msg_wait(): Blocks the caller until message is
     * available in the caller's RX buffer */
    val_ffa_msg_wait(&payload);

    if (payload.fid != FFA_MSG_SEND_32)
    {
        LOG(ERROR, "FFA_MSG_SEND failed, fid=0x%x, err %x",
                  payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(9);
    }

    /* ffa_yield: Only valid with the ERET conduit at
     * the Non-secure virtual FF-A instance else MBZ.*/
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    val_ffa_yield(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Only valid with the ERET conduit at the Non-secure virtual FF-A instance\
                  else MBZ, fid=0x%x, err=0x%x", payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(10);
    }

    /* ffa_yield(): Input parameter reserved (MBZ) */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg4 = 0xFFFF;
    val_ffa_yield(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "Reserved register mbz check failed, err %x", payload.arg2);
        status = VAL_ERROR_POINT(11);
    }

    /* ffa_yield: Block the current VM execution and
     * pass control back to the scheduler.*/
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_yield(&payload);
    if (payload.fid != FFA_SUCCESS_32)
    {
        LOG(ERROR, "FFA_YIELD failed, fid=0x%x, err %x",
                  payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(12);
    }

    /* Check the sender and reciever id */
    if (payload.arg1 != args.arg1)
    {
        LOG(ERROR, "Sender-reciever epid check failed, actual=%x but expected",
            payload.arg1, args.arg1);
        status = VAL_ERROR_POINT(13);
    }

    LOG(DBG, "Sender Receiver ID payload.arg1 %x args.arg1 %x", payload.arg1, args.arg1);

    /* Check the message size */
    if (payload.arg3 != msg_size)
    {
        LOG(ERROR, "msg size mismatch actual=%x but expected=%x",
            payload.arg3, msg_size);
        status = VAL_ERROR_POINT(14);
    }

    LOG(DBG, "Message Size %x", msg_size);

    /* Check the message content */
    if (val_memcmp(mb.recv, message, msg_size))
    {
        LOG(ERROR, "msg content mismatched");
        status = VAL_ERROR_POINT(15);
    }

    /* Return value for reserved registers - MBZ. w6 and w7 for ffa_msg_send */
    if (val_reserve_param_check(payload, output_reserve_count))
    {
        LOG(ERROR, "Received non-zero value for reserved registers",
            0, 0);
        status = VAL_ERROR_POINT(16);
    }

    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed");
        status = VAL_ERROR_POINT(17);
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
        LOG(ERROR, "ffa_msg_wait didn't fail");
        status = VAL_ERROR_POINT(18);
    }

    /* arg2 & arg3 are only valid at eret conduit at
     * the Non-secure virtual FF-A instance else MBZ. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg2 = 0xff;
    payload.arg3 = 0xff;
    val_ffa_msg_wait(&payload);
    if ((payload.fid != FFA_ERROR_32) && (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "ffa_msg_wait didn't fail");
        status = VAL_ERROR_POINT(19);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    /* ffa_msg_wait(): Blocks the caller until message is
     * available in the caller's RX buffer */
    val_ffa_msg_wait(&payload);

    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = VAL_ERROR_POINT(20);
    }

free_memory:
    if (val_free(mb.recv) || val_free(mb.send))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = VAL_ERROR_POINT(21);
    }

    return status;
}
