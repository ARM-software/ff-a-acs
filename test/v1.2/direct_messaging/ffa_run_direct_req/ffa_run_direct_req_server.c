/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t primary_sp_entry(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_endpoint_id_t receiver_1 = val_get_endpoint_id(SP2);
    uint32_t test_run_data = (uint32_t)args.arg3;

    test_run_data = TEST_RUN_DATA(GET_TEST_NUM((uint32_t)test_run_data),
     (uint32_t)sender, (uint32_t)receiver_1, GET_TEST_TYPE((uint32_t)test_run_data));

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    LOG(DBG, "Primary SP Test Setup Complete\n");

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(3);
        goto exit;
    }

    LOG(DBG, "FFA_RUN Error Check\n");
    /* Call ffa run to check for error */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)receiver_1 << 16;
    val_ffa_run(&payload);
    if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_DENIED)
    {
        LOG(ERROR, "FFA_RUN must fail fid=0x%lx, arg2 %lx %lx\n",
            payload.fid, payload.arg2, FFA_ERROR_DENIED);
        status =  VAL_ERROR_POINT(3);
        goto exit;
    }

    LOG(DBG, "FFA_RUN Error check complete\n");
    /* Respond back to client to switch run time model */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid != FFA_RUN_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(8);
        goto exit;
    }

    /* Call ffa run to check in ffa_run call chain */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)receiver_1 << 16;
    val_ffa_run(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "FFA_RUN must fail fid=0x%x, arg2 0x%x %x %x \n", payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(3);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver_1;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d\n", payload.arg2);
        status = VAL_ERROR_POINT(8);
        goto exit;
    }

    LOG(DBG, "FFA_MSG_WAIT to unwind FFA_RUN call chain\n");
    /* Call ffa msg wait */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_msg_wait(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "DIRECT_REQ_64 not received fid %x\n", payload.fid);
        status = VAL_ERROR_POINT(9);
        goto exit;
    }

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}

static uint32_t secondary_sp_entry(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;

    LOG(DBG, "Secondary SP Test Entry\n");

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(10);
        goto exit;
    }

    LOG(DBG, "FFA_RUN Call Call End, Respond back\n");
    /* Call ffa msg wait to respond back */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(8);
        goto exit;
    }

exit:
    return status;
}


uint32_t ffa_run_direct_req_server(ffa_args_t args)
{

    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t curr_ep_logical_id;

    curr_ep_logical_id = val_get_curr_endpoint_logical_id();

    if (curr_ep_logical_id == SP1)
    {
        status = primary_sp_entry(args);
    }
    else if (curr_ep_logical_id == SP2)
    {
        status = secondary_sp_entry(args);
    }

    return status;
}
