/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_direct_message_error2_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);
    ffa_endpoint_id_t recipient_1 = val_get_endpoint_id(VM1);

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    /* S-Endpoint cannot send a direct message to an NS-Endpoint */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient_1;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "Relayer must return error S-EP sends direct request to NS-EP %x\n",
                            payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

exit:

    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}
