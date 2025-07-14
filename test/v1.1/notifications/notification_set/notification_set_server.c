/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t notification_set_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_notification_bitmap_t notifications_bm_global;
    ffa_notification_bitmap_t notifications_bm_pcpu;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(1);
        goto exit;
    }

    notifications_bm_global = payload.arg3;
    notifications_bm_pcpu = payload.arg4;
    /* Bind the global notification */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bm_global & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_global >> 32);
    val_ffa_notification_bind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification bind err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

    /* Bind the per-cpu notification */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_PER_VCPU;
    payload.arg3 = (uint32_t)(notifications_bm_pcpu & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_pcpu >> 32);
    val_ffa_notification_bind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification bind err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(3);
        goto unbind_global;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed, err %d\n", payload.arg2);
        status =  VAL_ERROR_POINT(4);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bm_pcpu & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_pcpu >> 32);
    val_ffa_notification_unbind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification unbind err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(5);
    }

unbind_global:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bm_global & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_global >> 32);
    val_ffa_notification_unbind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification unbind err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(6);
    }

exit:
    return status;
}

