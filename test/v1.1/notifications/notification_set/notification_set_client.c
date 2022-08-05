/*
 * Copyright (c) 2022, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define INVALID_ID 0xFFFF

uint32_t notification_set_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);
    uint64_t notifications_bm_global = FFA_NOTIFICATION(12);
    uint64_t notifications_bm_pcpu = FFA_NOTIFICATION(13);
    uint64_t notifications_bm_invalid = FFA_NOTIFICATION(16);
    ffa_endpoint_id_t receiver_vcpuid = 0x2;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    payload.arg2 = 1;
    val_ffa_notification_bitmap_create(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Notification bitmap create failed %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    /* Pass notification id information to receiver */
    payload.arg3 =  notifications_bm_global;
    payload.arg4 =  notifications_bm_pcpu;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Direct request failed err %d\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(2);
        goto bitmap_destroy;
    }

    /* Invalid receiver id */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | INVALID_ID;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    payload.arg3 = (uint32_t)(notifications_bm_global & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_global >> 32);
    val_ffa_notification_set(&payload);
    if (payload.fid != FFA_ERROR_32 && payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "\t  Notification_set must return err for invalid id %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(3);
        goto bitmap_destroy;
    }

    /* Invalid sender id */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)INVALID_ID << 16) | recipient;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    payload.arg3 = (uint32_t)(notifications_bm_global & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_global >> 32);
    val_ffa_notification_set(&payload);
    if (payload.fid != FFA_ERROR_32 && payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "\t  Notification_set must return err for invalid id %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(4);
        goto bitmap_destroy;
    }

    /* Per-vCPU notification flag = b'0 and Receiver vCPU ID != 0 */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16) | recipient;
    payload.arg2 = ((uint32_t)receiver_vcpuid << 16);
    payload.arg3 = (uint32_t)(notifications_bm_pcpu & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_pcpu >> 32);
    val_ffa_notification_set(&payload);
    if (payload.fid != FFA_ERROR_32 && payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "\t  Notification_set must return err for invalid flag %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(5);
        goto bitmap_destroy;
    }

    /* Per-vCPU notification flag = b'0 and a per-vCPU notification is specified in the
     * Notification bitmap.
     */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    payload.arg3 = (uint32_t)(notifications_bm_pcpu & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_pcpu >> 32);
    val_ffa_notification_set(&payload);
    if (payload.fid != FFA_ERROR_32 && payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "\t  Notification_set must return err for invalid flag %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(6);
        goto bitmap_destroy;
    }

    /* Per-vCPU notification flag = b'1 and a global notification is specified
     * in the Notification bitmap.
     */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_PER_VCPU | FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    payload.arg3 = (uint32_t)(notifications_bm_global & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_global >> 32);
    val_ffa_notification_set(&payload);
    if (payload.fid != FFA_ERROR_32 && payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "\t  Notification_set must return err for invalid flag %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(7);
        goto bitmap_destroy;
    }

    /* DENIED: Sender not permitted to signal the notification to the receiver */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    payload.arg3 = (uint32_t)(notifications_bm_invalid & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bm_invalid >> 32);
    val_ffa_notification_set(&payload);
    if (payload.fid != FFA_ERROR_32 && payload.arg2 != FFA_ERROR_DENIED)
    {
        LOG(ERROR, "\t  Notification_set must return err for invalid bitmap %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(8);
    }

bitmap_destroy:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    val_ffa_notification_bitmap_destroy(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Bitmap destroy failed err %x\n", payload.arg2, 0);
        status = status ? status : VAL_ERROR_POINT(9);
    }

exit:
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    return status ? status : (uint32_t)payload.arg3;
}

