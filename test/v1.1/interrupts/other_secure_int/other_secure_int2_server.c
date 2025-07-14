/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define S_WD_TIMEOUT 50U
#define WD_TIME_OUT  75U

static volatile bool interrupt_triggered;
static int wd_irq_handler(void)
{
    interrupt_triggered = true;
    val_twdog_disable();
    LOG(DBG, "T-WD IRQ Handler Processed\n");
    return 0;
}

uint32_t other_secure_int2_server(ffa_args_t args)
{
    ffa_args_t payload = args;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    uint32_t status = VAL_SUCCESS;

   if (val_irq_register_handler(PLATFORM_TWDOG_INTID, wd_irq_handler))
    {
        LOG(ERROR, "WD interrupt register failed\n");
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    /* Enable Trusted WD Interrupt */
    val_twdog_enable(S_WD_TIMEOUT);

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(2);
        goto exit;
    }

    /* SP is uni-processor capable with a single execution context.
     * It must run only on a single PE in the system at any point of
     * time. Since client has send direct request on different PE other
     * than boot cpu, spm must migrate sp on the PE on which client
     * has sent second direct request. If there is an interrupt targetting
     * other PE, it must be queued until EC runs on specified PE.
     * */

    /* Wait for WD interrupt */
    sp_sleep(WD_TIME_OUT);
    LOG(DBG, "SP Sleep Complete\n");

    if (interrupt_triggered == true)
    {
        LOG(ERROR, "WD interrupt should be queued\n");
        status =  VAL_ERROR_POINT(3);
        goto free_interrupt;
    }
    LOG(DBG, "IRQ Status %x\n", interrupt_triggered);

    /* Enter Wait state by responding to client to Get Interrupt */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid != FFA_INTERRUPT_32)
    {
        LOG(ERROR, "FFA_INTERRUPT_32 not received fid %x\n", payload.fid);
        status = VAL_ERROR_POINT(4);
        goto exit;
    }

    val_twdog_intr_disable();

    if (interrupt_triggered != true)
    {
        LOG(ERROR, "WD interrupt should be triggered\n");
        status =  VAL_ERROR_POINT(5);
    }
    LOG(DBG, "IRQ Status %x\n", interrupt_triggered);

free_interrupt:
    if (val_irq_unregister_handler(PLATFORM_TWDOG_INTID))
    {
        LOG(ERROR, "IRQ handler unregister failed\n");
        status = VAL_ERROR_POINT(6);
    }

exit:
    return status;
}
