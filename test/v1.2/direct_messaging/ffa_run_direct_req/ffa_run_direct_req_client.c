/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_run_direct_req_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    LOG(DBG, "Calling Server EP for invalid FFA_RUN Check");
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(8);
        goto exit;
    }


    LOG(DBG, "FFA_RUN call chain check");
    /* Call ffa run */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)recipient << 16;
    val_ffa_run(&payload);
    if (payload.fid != FFA_MSG_WAIT_32)
    {
        LOG(ERROR, "FFA_MSG_WAIT_32 not received %x", payload.fid);
        status = VAL_ERROR_POINT(6);
        goto exit;
    }

exit:
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}