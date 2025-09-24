/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t notification_set_helper(
                        ffa_notification_bitmap_t bitmap,
                        uint32_t flags,
                        ffa_endpoint_id_t sender,
                        ffa_endpoint_id_t receiver)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    payload.arg2 = flags;
    payload.arg3 = (uint32_t)(bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(bitmap >> 32);

    val_ffa_notification_set(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification set err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(1);
    }
    return status;
}

uint32_t sp_signals_vm_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_notification_bitmap_t notifications_bitmap_1;
    uint32_t flags;

    LOG(DBG, "Server Setup Compelte, resp to client\n");
    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(2);
        goto exit;
    }

    /* notfication bitmap setting
    VM1 Global Notification   - bitmap_1 */
    notifications_bitmap_1 = payload.arg3;

    LOG(DBG, "Server set notifications_bitmap_1\n");
    /* Set Notification for VM1 */
    flags = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    status = notification_set_helper(notifications_bitmap_1, flags, sender, receiver);
    if (status)
    {
        goto exit;
    }

    LOG(DBG, "Resume Client to process notification\n");
    /* Resume Preempted VM1 context */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "DIRECT_REQ_64 not received fid %x\n", payload.fid);
        status = VAL_ERROR_POINT(6);
    }

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(8);
    }

    return status;
}