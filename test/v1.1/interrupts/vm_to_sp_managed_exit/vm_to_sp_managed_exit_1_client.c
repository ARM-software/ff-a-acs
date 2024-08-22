/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define NS_WD_TIMEOUT 500000U

static volatile uint32_t irq_received;
static int wd_irq_handler(void)
{
    irq_received = true;
    val_ns_wdog_disable();
    LOG(DBG, "NS-WD IRQ Handler Processed");
    return 0;
}

uint32_t vm_to_sp_managed_exit_1_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    /* NS-Interrupt Registration */
    if (val_irq_register_handler(PLATFORM_NS_WD_INTR, wd_irq_handler))
    {
        LOG(ERROR, "WD interrupt register failed");
        status = VAL_ERROR_POINT(1);
        goto free_interrupt;
    }

    val_irq_enable(PLATFORM_NS_WD_INTR, 0);
    val_ns_wdog_enable(NS_WD_TIMEOUT);
    LOG(DBG, "NS-WD IRQ Handler Registered");

    /* Send Direct Request to MEI Enabled SP */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_64)
    {
        LOG(ERROR, "DIRECT_RESP_64 not received fid %x err %x", payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto free_interrupt;
    }

    if (!irq_received)
    {
        LOG(ERROR, "WD interrupt not received");
        status = VAL_ERROR_POINT(3);
        goto free_interrupt;
    }

    LOG(DBG, "NS-WD IRQ Triggered");

    /* Send Direct Request to MEI Enabled SP For Clean up */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_64)
    {
        LOG(ERROR, "DIRECT_RESP_64 not received fid %x err %x", payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(4);
    }

free_interrupt:
    val_irq_disable(PLATFORM_NS_WD_INTR);
    if (val_irq_unregister_handler(PLATFORM_NS_WD_INTR))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = VAL_ERROR_POINT(5);
    }
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}

