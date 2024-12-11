/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "test_database.h"
#define AP_REF_CLK_TIMEOUT 10000U
#define AP_REF_CLK_WAIT    50U
static volatile bool interrupt_triggered;
static int wd_irq_handler(void)
{
    interrupt_triggered = true;
    LOG(DBG, "AP REF CLK Handler Processed");
    return 0;
}
uint32_t sp_el1_running_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    interrupt_triggered = false;
    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(1);
        goto exit;
    }
   if (val_irq_register_handler(PALTFORM_AP_REFCLK_CNTPSIRQ1, wd_irq_handler))
    {
        LOG(ERROR, "WD interrupt register failed");
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
    val_sys_phy_timer_en(AP_REF_CLK_TIMEOUT);
    LOG(DBG, "System Timer IRQ Enabled");

    /* Wait for WD interrupt */
    val_sp_sleep(AP_REF_CLK_WAIT);

    val_sys_phy_timer_dis(true);
    LOG(DBG, "SP Sleep Complete, IRQ Status %x", interrupt_triggered);

    if (interrupt_triggered != true)
    {
        LOG(ERROR, "WD interrupt should be triggered");
        status =  VAL_ERROR_POINT(3);
    }
    if (val_irq_unregister_handler(PALTFORM_AP_REFCLK_CNTPSIRQ1))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = VAL_ERROR_POINT(4);
    }
exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(5);
    }
    return status;
}
