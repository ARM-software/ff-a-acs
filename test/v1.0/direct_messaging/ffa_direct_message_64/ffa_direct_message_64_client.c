/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
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
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    req_data_64.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                            val_get_endpoint_id(server_logical_id);
    val_ffa_msg_send_direct_req_64(&req_data_64);
    if (req_data_64.fid != FFA_MSG_SEND_DIRECT_RESP_32)
    {
#if (PLATFORM_FFA_V_1_0 != 1)
        if  (req_data_64.fid == FFA_YIELD_32)
        {
            req_data_64.arg1 = ((uint32_t)val_get_endpoint_id(server_logical_id) << 16);
            val_ffa_run(&req_data_64);
            if (req_data_64.fid != FFA_MSG_SEND_DIRECT_RESP_64)
            {
                LOG(ERROR, "FFA_RUN Failed err %x", req_data_64.fid);
                status = VAL_ERROR_POINT(2);
                goto exit;
            }
        }
        else
#endif
        {
            LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
            req_data_64.fid, req_data_64.arg2);
            status = VAL_ERROR_POINT(3);
            goto exit;
        }

    }

    LOG(DBG, "req data arg3 %x arg4 %x arg5 %x arg 6 %x arg7 %x",
        req_data_64.arg3, req_data_64.arg4, req_data_64.arg5, req_data_64.arg6, req_data_64.arg7);

    LOG(DBG, "expected resp arg3 %x arg4 %x arg5 %x arg 6 %x arg7 %x",
        expected_resp_data_64.arg3, expected_resp_data_64.arg4, expected_resp_data_64.arg5,
        expected_resp_data_64.arg6, expected_resp_data_64.arg7);

    /* Direct respond received, Compare the respond req_data_64 */
    if (req_data_64.arg3 != expected_resp_data_64.arg3 ||
        req_data_64.arg4 != expected_resp_data_64.arg4 ||
        req_data_64.arg5 != expected_resp_data_64.arg5 ||
        req_data_64.arg6 != expected_resp_data_64.arg6 ||
        req_data_64.arg7 != expected_resp_data_64.arg7)
    {
        LOG(ERROR, "Direct response data mismatched");
        status = VAL_ERROR_POINT(4);
        goto exit;
    }

    /* Wrong source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(server_logical_id) << 16) |
                            val_get_endpoint_id(client_logical_id);
    val_ffa_msg_send_direct_req_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Wrong source and dest id check failed, fid=0x%x, err=0x%x",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto exit;
    }

    /* Same source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                            val_get_endpoint_id(client_logical_id);
    val_ffa_msg_send_direct_req_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Same source and dest id check failed, fid=0x%x, err=0x%x",
        payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(6);
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
        LOG(ERROR, "W2 reserved register mbz check failed, fid=0x%x, err=0x%x",
                 payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto exit;
    }

exit:
    (void)resp_data_64;
    /* Collect the server status in payload.arg3 */
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}
