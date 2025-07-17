/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t mp_execution_contexts_sec_cpu_server(ffa_args_t args)
{
    uint32_t data_pattern;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_args_t payload = args;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    /* Receive direct request using secondary cpu */
    data_pattern = (uint32_t)payload.arg3;

    /* Send direct respond back */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16) | receiver;
    payload.arg3 = data_pattern;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err %x\n",
                 payload.fid, payload.arg2);
        return VAL_ERROR_POINT(2);
    }

    return VAL_SUCCESS;
}
