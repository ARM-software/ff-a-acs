/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t vm_to_sp_notification_el0_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_notification_bitmap_t notifications_bitmap;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_NPI;
    val_ffa_features(&payload);
    if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_NOT_SUPPORTED)
    {
        LOG(ERROR, "FFA_Features Must Fail to retrieve NPI err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(3);
        goto exit;
    }

    notifications_bitmap = payload.arg3;
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bitmap >> 32);
    val_ffa_notification_bind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification bind err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed, err %d\n", payload.arg2);
        status =  VAL_ERROR_POINT(5);
        goto unbind;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_BITMAP_VM;
    val_ffa_notification_get(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification get err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto unbind;
    }

    LOG(DBG, "Notifications_bitmap %x payload.arg4 %x\n", notifications_bitmap,
        (uint32_t)payload.arg4);

    if (notifications_bitmap != (uint32_t)payload.arg4)
    {
        LOG(ERROR, "Not received expected notification err %x\n", payload.arg4);
        status = VAL_ERROR_POINT(8);
    }

unbind:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bitmap >> 32);
    val_ffa_notification_unbind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification unbind err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(9);
    }

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(11);
    }

    return status;
}

