/*
 * Copyright (c) 2022, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define WD_TIME_OUT 0x10000000
static volatile uint32_t managed_exit_received = false;
static uint32_t mei_id;

static int mei_irq_handler(void)
{
    managed_exit_received = true;
    val_secure_intr_disable(mei_id, INTERRUPT_TYPE_FIQ);

    return 0;
}

uint32_t vm_to_sp_managed_exit_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint64_t timeout = WD_TIME_OUT;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_MEI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed to retrieve MEI err %x\n", payload.arg2, 0);
        status =  VAL_ERROR_POINT(1);
        goto exit;
    }

    mei_id = (uint32_t)payload.arg2;
    if (val_irq_register_handler(mei_id, mei_irq_handler))
    {
        LOG(ERROR, "\t  MEI interrupt register failed\n", 0, 0);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
    val_secure_intr_enable(mei_id, INTERRUPT_TYPE_FIQ);

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(3);
        goto free_interrupt;
    }

    /* Wait for WD interrupt */
    while(--timeout && !managed_exit_received);

    if (!timeout)
    {
        LOG(ERROR, "\t  WD interrupt not triggered\n", 0, 0);
        status =  VAL_ERROR_POINT(4);
    }

free_interrupt:
    if (val_irq_unregister_handler(mei_id))
    {
        LOG(ERROR, "\t  IRQ handler unregister failed\n", 0, 0);
        status = VAL_ERROR_POINT(5);
    }

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tDirect response failed err %x\n", payload.arg2, 0);
        status = status ? status : VAL_ERROR_POINT(6);
    }

    return status;
}

