/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"
#include "ffa_message_data2.h"

uint32_t ffa_direct_message2_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    val_endpoint_info_t *info_ptr;


    info_ptr = val_get_endpoint_info();
    if (info_ptr == NULL)
    {
        VAL_PANIC("Server Info Null");
    }
    LOG(ALWAYS, "Select Server UUID 0x%x 0x%x 0x%x 0x%x\n",
    info_ptr[server_logical_id].uuid[0],
    info_ptr[server_logical_id].uuid[1],
    info_ptr[server_logical_id].uuid[2],
    info_ptr[server_logical_id].uuid[3]);

    /* Run server test fn by calling through direct_req */
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n", payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    req_data_64.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                            val_get_endpoint_id(server_logical_id);
    req_data_64.arg2 = ((uint64_t)info_ptr[server_logical_id].uuid[1] << 32) |
                       ((uint64_t)info_ptr[server_logical_id].uuid[0] & 0xFFFFFFFF);
    req_data_64.arg3 = ((uint64_t)info_ptr[server_logical_id].uuid[3] << 32) |
                       ((uint64_t)info_ptr[server_logical_id].uuid[2] & 0xFFFFFFFF);

    val_ffa_msg_send_direct_req2_64(&req_data_64);
    if (req_data_64.fid != FFA_MSG_SEND_DIRECT_RESP2_64)
    {
        if  (req_data_64.fid == FFA_YIELD_32)
        {
            LOG(DBG, "FFA_YIELD Received\n");
            req_data_64.arg1 = ((uint32_t)val_get_endpoint_id(server_logical_id) << 16);
            val_ffa_run((ffa_args_t *)&req_data_64);
            if (req_data_64.fid != FFA_MSG_SEND_DIRECT_RESP2_64)
            {
                LOG(ERROR, "FFA_RUN Failed err %x\n", req_data_64.fid);
                status = VAL_ERROR_POINT(2);
                goto exit;
            }
        }
        else
        {
            LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
            req_data_64.fid, req_data_64.arg2);
            status = VAL_ERROR_POINT(3);
            goto exit;
        }

    }

    LOG(DBG, "req data arg3:0x%lx arg4:0x%lx arg5:0x%lx arg6:0x%lx arg7:0x%lx\n",
        req_data_64.arg3, req_data_64.arg4, req_data_64.arg5, req_data_64.arg6, req_data_64.arg7);

    LOG(DBG, "req data arg8:0x%lx arg9:0x%lx arg10:0x%lx arg11:0x%lx arg12:0x%lx\n",
        req_data_64.ext_args.arg8, req_data_64.ext_args.arg9, req_data_64.ext_args.arg10,
        req_data_64.ext_args.arg11, req_data_64.ext_args.arg12);

    LOG(DBG, "req data arg13:0x%lx arg14:0x%lx arg15:0x%lx arg16:0x%lx arg17:0x%lx\n",
        req_data_64.ext_args.arg13, req_data_64.ext_args.arg14, req_data_64.ext_args.arg15,
        req_data_64.ext_args.arg16, req_data_64.ext_args.arg17);

    LOG(DBG, "expected resp arg3:0x%lx arg4:0x%lx arg5:0x%lx arg6:0x%lx arg7:0x%lx\n",
        expected_resp_data_64.arg3, expected_resp_data_64.arg4, expected_resp_data_64.arg5,
        expected_resp_data_64.arg6, expected_resp_data_64.arg7);

    LOG(DBG, "expected resp arg8:0x%lx arg9:0x%lx arg10:0x%lx arg11:0x%lx arg12:0x%lx\n",
        expected_resp_data_64.ext_args.arg8, expected_resp_data_64.ext_args.arg9,
        expected_resp_data_64.ext_args.arg10, expected_resp_data_64.ext_args.arg11,
        expected_resp_data_64.ext_args.arg12);

    LOG(DBG, "expected resp arg13:0x%lx arg14:0x%lx arg15:0x%lx arg16:0x%lx arg17:0x%lx\n",
        expected_resp_data_64.ext_args.arg13, expected_resp_data_64.ext_args.arg14,
        expected_resp_data_64.ext_args.arg15, expected_resp_data_64.ext_args.arg16,
        expected_resp_data_64.ext_args.arg17);

    /* Direct respond received, Compare the respond req_data_64 */
    if (req_data_64.arg3 != expected_resp_data_64.arg3   ||
        req_data_64.arg4 != expected_resp_data_64.arg4   ||
        req_data_64.arg5 != expected_resp_data_64.arg5   ||
        req_data_64.arg6 != expected_resp_data_64.arg6   ||
        req_data_64.arg7 != expected_resp_data_64.arg7   ||
        req_data_64.ext_args.arg8 != expected_resp_data_64.ext_args.arg8   ||
        req_data_64.ext_args.arg9 != expected_resp_data_64.ext_args.arg9   ||
        req_data_64.ext_args.arg10 != expected_resp_data_64.ext_args.arg10 ||
        req_data_64.ext_args.arg11 != expected_resp_data_64.ext_args.arg11 ||
        req_data_64.ext_args.arg12 != expected_resp_data_64.ext_args.arg12 ||
        req_data_64.ext_args.arg13 != expected_resp_data_64.ext_args.arg13 ||
        req_data_64.ext_args.arg14 != expected_resp_data_64.ext_args.arg14 ||
        req_data_64.ext_args.arg15 != expected_resp_data_64.ext_args.arg15 ||
        req_data_64.ext_args.arg16 != expected_resp_data_64.ext_args.arg16 ||
        req_data_64.ext_args.arg17 != expected_resp_data_64.ext_args.arg17)
    {
        LOG(ERROR, "Direct response data mismatched\n");
        status = VAL_ERROR_POINT(4);
        goto exit;
    }

    /* Wrong source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(server_logical_id) << 16) |
                            val_get_endpoint_id(client_logical_id);
    req_data_64.arg2 = ((uint64_t)info_ptr[server_logical_id].uuid[1] << 32) |
                       ((uint64_t)info_ptr[server_logical_id].uuid[0] & 0xFFFFFFFF);
    req_data_64.arg3 = ((uint64_t)info_ptr[server_logical_id].uuid[3] << 32) |
                       ((uint64_t)info_ptr[server_logical_id].uuid[2] & 0xFFFFFFFF);
    val_ffa_msg_send_direct_req2_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Wrong source and dest id check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto exit;
    }
    LOG(DBG, "Invalid Source and Destination ID Check Passed\n");

    /* Wrong UUID check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                            val_get_endpoint_id(server_logical_id);
    payload.arg2 = 0xFFFFF;
    payload.arg3 = 0xEEEEE;
    val_ffa_msg_send_direct_req2_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Wrong UUID check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(6);
        goto exit;
    }
    LOG(DBG, "Invalid UUID Check  Check Passed\n");

    /* Same source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                            val_get_endpoint_id(client_logical_id);
    req_data_64.arg2 = ((uint64_t)info_ptr[server_logical_id].uuid[1] << 32) |
                       ((uint64_t)info_ptr[server_logical_id].uuid[0] & 0xFFFFFFFF);
    req_data_64.arg3 = ((uint64_t)info_ptr[server_logical_id].uuid[3] << 32) |
                       ((uint64_t)info_ptr[server_logical_id].uuid[2] & 0xFFFFFFFF);
    val_ffa_msg_send_direct_req2_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Same source and dest id check failed, fid=0x%x, err=0x%x\n",
        payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto exit;
    }
    LOG(DBG, "Same Source and Destination Check Passed\n");

exit:
    (void)resp_data_64;
    /* Collect the server status in payload.arg3 */
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}
