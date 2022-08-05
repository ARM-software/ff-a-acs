/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"
#include "ffa_message_data.h"

uint32_t ffa_direct_message_32_server(ffa_args_t args)
{
    ffa_args_t payload = args;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = (args.arg1 >> 16) & 0xFFFF;
    ffa_endpoint_id_t receiver = (args.arg1) & 0xFFFF;

    /* Handshake client test fn */
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    /* Process the msg and compare data with the expected payload message */
    if (payload.arg3 != expected_req_data_32.arg3 ||
        payload.arg4 != expected_req_data_32.arg4 ||
        payload.arg5 != expected_req_data_32.arg5 ||
        payload.arg6 != expected_req_data_32.arg6 ||
        payload.arg7 != expected_req_data_32.arg7)
    {
        LOG(ERROR, "\tDirect request data mismatched\n", 0, 0);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

    /* Same source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16) | sender;
    val_ffa_msg_send_direct_resp_32(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tSame source and dest id check failed, fid=0x%x\n, err=0x%x",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(3);
        goto exit;
    }

    /* Wrong source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_32(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tWrong source and dest id check failed, fid=0x%x\n, err=0x%x",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto exit;
    }

    /* Input parameter w2 reserved (MBZ) */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)receiver << 16) | sender;
    payload.arg2 = 0xFFFF;
    val_ffa_msg_send_direct_resp_32(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tW2 reserved register mbz check failed, fid=0x%x\n, err=0x%x",
                 payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto exit;
    }

    /* It is invaild to call non-framwork messages
     * while processing the direct request. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_yield(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "\tCall to FFA_YIELD must fail while processing direct msg\n", 0, 0);
        status = VAL_ERROR_POINT(6);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_msg_poll(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "\tCall to FFA_MSG_POLL must fail while processing direct msg\n", 0, 0);
        status = VAL_ERROR_POINT(7);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)receiver << 16) | sender;
    val_ffa_msg_send(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "\tCall to FFA_MSG_SEND must fail while processing direct msg\n", 0, 0);
        status = VAL_ERROR_POINT(8);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_msg_wait(&payload);
    if (payload.fid != FFA_ERROR_32 || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "\tCall to FFA_MSG_WAIT must fail while processing direct msg\n", 0, 0);
        status = VAL_ERROR_POINT(9);
        goto exit;
    }
exit:
    resp_data_32.arg1 = ((uint32_t)receiver << 16) | sender;
    val_ffa_msg_send_direct_resp_32(&resp_data_32);

    (void)req_data_32;
    return status;
}
