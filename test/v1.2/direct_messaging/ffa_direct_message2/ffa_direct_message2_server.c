/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"
#include "ffa_message_data2.h"

uint32_t ffa_direct_message2_server(ffa_args_t args)
{
    ffa_args_t payload = (ffa_args_t)args;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = (args.arg1 >> 16) & 0xFFFF;
    ffa_endpoint_id_t receiver = (args.arg1) & 0xFFFF;
    val_endpoint_info_t *info_ptr;
    uint32_t server_logical_id = val_get_endpoint_logical_id(sender);

    info_ptr = val_get_endpoint_info();
    if (info_ptr == NULL)
    {
        VAL_PANIC("Server Info Null");
    }
    LOG(ALWAYS, "Select Client UUID 0x%x 0x%x 0x%x 0x%x\n",
    info_ptr[server_logical_id].uuid[0],
    info_ptr[server_logical_id].uuid[1],
    info_ptr[server_logical_id].uuid[2],
    info_ptr[server_logical_id].uuid[3]);

    /* Handshake client test fn */
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);

    /* Is direct request received? */
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ2_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    LOG(DBG, "Payload arg4:0x%lx arg5:0x%lx arg6:0x%lx arg7:0x%lx\n",
        payload.arg4, payload.arg5, payload.arg6, payload.arg7);

    LOG(DBG, "Payload arg8:0x%lx arg9:0x%lx arg10:0x%lx arg11:0x%lx arg12:0x%lx\n",
        payload.ext_args.arg8, payload.ext_args.arg9, payload.ext_args.arg10,
        payload.ext_args.arg11, payload.ext_args.arg12);

    LOG(DBG, "Payload arg13:0x%lx arg14:0x%lx arg15:0x%lx arg16:0x%lx arg17:0x%lx\n",
        payload.ext_args.arg13, payload.ext_args.arg14, payload.ext_args.arg15,
        payload.ext_args.arg16, payload.ext_args.arg17);

    LOG(DBG, "Expected req arg4:0x%lx arg5:0x%lx arg6:0x%lx arg7:0x%lx\n",
        expected_req_data_64.arg4, expected_req_data_64.arg5,
        expected_req_data_64.arg6, expected_req_data_64.arg7);

    LOG(DBG, "Expected req arg8:0x%lx arg9:0x%lx arg10:0x%lx arg11:0x%lx arg12:0x%lx\n",
        expected_req_data_64.ext_args.arg8, expected_req_data_64.ext_args.arg9,
        expected_req_data_64.ext_args.arg10, expected_req_data_64.ext_args.arg11,
        expected_req_data_64.ext_args.arg12);

    LOG(DBG, "Expected req arg13:0x%lx arg14:0x%lx arg15:0x%lx arg16:0x%lx arg17:0x%lx\n",
        expected_req_data_64.ext_args.arg13, expected_req_data_64.ext_args.arg14,
        expected_req_data_64.ext_args.arg15, expected_req_data_64.ext_args.arg16,
        expected_req_data_64.ext_args.arg17);

    /* Process the msg and compare data with the expected payload message */
    if (payload.arg4 != expected_req_data_64.arg4  ||
        payload.arg5 != expected_req_data_64.arg5  ||
        payload.arg6 != expected_req_data_64.arg6  ||
        payload.arg7 != expected_req_data_64.arg7  ||
        payload.ext_args.arg8 != expected_req_data_64.ext_args.arg8  ||
        payload.ext_args.arg9 != expected_req_data_64.ext_args.arg9  ||
        payload.ext_args.arg10 != expected_req_data_64.ext_args.arg10 ||
        payload.ext_args.arg11 != expected_req_data_64.ext_args.arg11 ||
        payload.ext_args.arg12 != expected_req_data_64.ext_args.arg12 ||
        payload.ext_args.arg13 != expected_req_data_64.ext_args.arg13 ||
        payload.ext_args.arg14 != expected_req_data_64.ext_args.arg14 ||
        payload.ext_args.arg15 != expected_req_data_64.ext_args.arg15 ||
        payload.ext_args.arg16 != expected_req_data_64.ext_args.arg16 ||
        payload.ext_args.arg17 != expected_req_data_64.ext_args.arg17)
    {
        LOG(ERROR, "Direct request data mismatched\n");
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

    /* Same source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16) | sender;
    val_ffa_msg_send_direct_resp2_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Same source and dest id check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(3);
        goto exit;
    }
    LOG(DBG, "Same Source and Destination Check Passed\n");

    /* Wrong source and destination ids check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp2_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Wrong source and dest id check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto exit;
    }
    LOG(DBG, "Invalid Source and Destination ID Check Passed\n");

    /* It is invaild to call non-framwork messages
     * while processing the direct request. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_yield(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Call to FFA_YIELD must not fail %x\n", payload.fid);
        status = VAL_ERROR_POINT(6);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)receiver << 16) | sender;
    val_ffa_msg_send(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "Call to FFA_MSG_SEND must fail while processing direct msg2\n");
        status = VAL_ERROR_POINT(8);
        goto exit;
    }
    LOG(DBG, "FFA MSG_SEND Check Complete\n");

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_msg_wait(&payload);
    if (payload.fid != FFA_ERROR_32 || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "Call to FFA_MSG_WAIT must fail while processing direct msg2\n");
        status = VAL_ERROR_POINT(9);
        goto exit;
    }
    LOG(DBG, "FFA MSG_WAIT Check Complete\n");

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_memcpy(&payload, &resp_data_64, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)receiver << 16) | sender;
    val_ffa_msg_send_direct_resp2_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(10);
    }
    (void)req_data_64;
    return status;
}
