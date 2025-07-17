/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define WD_TIME_OUT 100U
static volatile uint32_t managed_exit_received;
static uint32_t mei_id;

static int mei_irq_handler(void)
{
    managed_exit_received = true;
    val_secure_intr_disable(mei_id, INTERRUPT_TYPE_FIQ);
    LOG(DBG, "MEI Handler Processed\n");
    return 0;
}

uint32_t vm_to_sp_managed_exit_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    managed_exit_received = false;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_MEI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed to retrieve MEI err %x\n", payload.arg2);
        status =  VAL_ERROR_POINT(1);
        goto exit;
    }

    mei_id = (uint32_t)payload.arg2;
    if (val_irq_register_handler(mei_id, mei_irq_handler))
    {
        LOG(ERROR, "MEI interrupt register failed\n");
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
    val_secure_intr_enable(mei_id, INTERRUPT_TYPE_FIQ);
   LOG(DBG, "MEI Handler Registered, MEI ID %x\n", mei_id);

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(3);
        goto free_interrupt;
    }

    /* Wait for WD interrupt */
    val_sp_sleep(WD_TIME_OUT);
    LOG(DBG, "SP Sleep Complete, managed_exit_received %x\n", managed_exit_received);

    if (managed_exit_received != true)
    {
        LOG(ERROR, "WD interrupt not triggered\n");
        status =  VAL_ERROR_POINT(4);
    }

free_interrupt:
    if (val_irq_unregister_handler(mei_id))
    {
        LOG(ERROR, "IRQ handler unregister failed\n");
        status = VAL_ERROR_POINT(5);
    }

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

