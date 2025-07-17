/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_direct_message_error2_server(ffa_args_t args)
{
    ffa_args_t payload = args;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = (args.arg1 >> 16) & 0xFFFF;
    ffa_endpoint_id_t receiver = (args.arg1) & 0xFFFF;

    /* Handshake client test fn */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    /* Try to send direct req to SP1,
     * which should return BUSY error code.
     */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)receiver << 16) | sender;
    val_ffa_msg_send_direct_req_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_BUSY))
    {
        LOG(ERROR, "Direct request must failed err %x %x\n", payload.arg2, payload.fid);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
    LOG(DBG, "SP Returned Busy for Direct Req arg2 %x\n", payload.arg2);

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)receiver << 16) | sender;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(3);
    }

    return status;
}
