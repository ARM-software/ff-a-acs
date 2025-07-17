/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define WD_TIME_OUT 100U

static uint32_t mei_disabled_sp_setup(ffa_args_t args)
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

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(1);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver_1;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid != FFA_INTERRUPT_32)
    {
        LOG(ERROR, "FFA_INTERRUPT_32 not received\n");
        status = VAL_ERROR_POINT(2);
    }

    LOG(DBG, "Call FFA Run\n");
    /* Schedule the preempted SP using FFA_RUN */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)receiver_1 << 16;
    val_ffa_run(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_64)
    {
        LOG(ERROR, "DIRECT_RESP_64 not received\n");
        status = VAL_ERROR_POINT(3);
        goto exit;
    }

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(4);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}

static uint32_t mei_enabled_sp_setup(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;


    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, " Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(5);
        goto exit;
    }

    /* Wait for WD interrupt */
    sp_sleep(WD_TIME_OUT);
    LOG(DBG, "SP Sleep Complete\n");

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(6);
    }
    return status;
}

uint32_t vm_to_sp_managed_exit_4_server(ffa_args_t args)
{
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t curr_ep_logical_id;

    curr_ep_logical_id = val_get_curr_endpoint_logical_id();

    if (curr_ep_logical_id == SP1)
    {
        status = mei_disabled_sp_setup(args);
    }
    else if (curr_ep_logical_id == SP2)
    {
        status = mei_enabled_sp_setup(args);
    }
    return status;
}

