/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define S_WD_TIMEOUT 10
#define WD_TIME_OUT 0x20000

static volatile bool interrupt_triggered;
static int wd_irq_handler(void)
{
    val_interrupt_get();
    interrupt_triggered = true;
    val_twdog_disable();
    return 0;
}

uint32_t sp_el0_running_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint64_t timeout = WD_TIME_OUT;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    interrupt_triggered = false;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(1);
        goto exit;
    }

    val_twdog_enable(S_WD_TIMEOUT);

    /* Wait for WD interrupt */
    while (--timeout && (interrupt_triggered != true));

    if (interrupt_triggered == true)
    {
        LOG(ERROR, "\t WD interrupt should not be triggered\n", 0, 0);
        status =  VAL_ERROR_POINT(2);
        goto free_interrupt;
    }

    /* Enter Wait state by responding to client to Get Interrupt */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid != FFA_INTERRUPT_32)
    {
        LOG(ERROR, "\t FFA_INTERRUPT_32 not received fid %x\n", payload.fid, 0);
        status = VAL_ERROR_POINT(3);
        goto free_interrupt;
    }

    if (payload.fid == FFA_INTERRUPT_32)
    {
        wd_irq_handler();
    }

    if (interrupt_triggered != true)
    {
        LOG(ERROR, "\t WD interrupt should be triggered\n", 0, 0);
        status =  VAL_ERROR_POINT(4);
    }

free_interrupt:
    val_twdog_intr_disable();
exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_msg_wait(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
    {
        LOG(ERROR, "\t DIRECT_REQ_32 not received fid %x\n", payload.fid, 0);
        status = status ? status : VAL_ERROR_POINT(5);
    }
    return status;
}

