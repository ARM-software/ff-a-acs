/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"
#include "ffa_message_data.h"

uint32_t ffa_direct_message_64_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);

    /* Run server test fn by calling through direct_req */
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    req_data_64.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                            val_get_endpoint_id(server_logical_id);
    val_ffa_msg_send_direct_req_64(&req_data_64);
    if (req_data_64.fid != FFA_MSG_SEND_DIRECT_RESP_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  req_data_64.fid, req_data_64.arg2);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

    /* Direct respond received, Compare the respond req_data_64 */
    if (req_data_64.arg3 != expected_resp_data_64.arg3 ||
        req_data_64.arg4 != expected_resp_data_64.arg4 ||
        req_data_64.arg5 != expected_resp_data_64.arg5 ||
        req_data_64.arg6 != expected_resp_data_64.arg6 ||
        req_data_64.arg7 != expected_resp_data_64.arg7)
    {
        LOG(ERROR, "\tDirect response data mismatched\n", 0, 0);
        status = VAL_ERROR_POINT(3);
        goto exit;
    }

    /* Wrong source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(server_logical_id) << 16) |
                            val_get_endpoint_id(client_logical_id);
    val_ffa_msg_send_direct_req_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tWrong source and dest id check failed, fid=0x%x\n, err=0x%x",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto exit;
    }

    /* Same source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                            val_get_endpoint_id(client_logical_id);
    val_ffa_msg_send_direct_req_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tSame source and dest id check failed, fid=0x%x\n, err=0x%x",
        payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto exit;
    }

    /* Input parameter w2 reserved (MBZ) */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                            val_get_endpoint_id(server_logical_id);
    payload.arg2 = 0xFFFF;
    val_ffa_msg_send_direct_req_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tW2 reserved register mbz check failed, fid=0x%x\n, err=0x%x",
                 payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(6);
        goto exit;
    }

exit:
    (void)resp_data_64;
    /* Collect the server status in payload.arg3 */
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}
