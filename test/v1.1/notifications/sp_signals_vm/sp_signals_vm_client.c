/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static volatile uint32_t npi_flag;
static int npi_irq_handler(void)
{
    LOG(DBG, "NPI IRQ Serviced\n");
    npi_flag = 1;
    return 0;

}

static uint32_t notification_bind_unbind_helper(
                        ffa_notification_bitmap_t bitmap,
                        bool bind, uint32_t flag,
                        ffa_endpoint_id_t sender,
                        ffa_endpoint_id_t recipient)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((recipient << 16) | (sender));
    payload.arg3 = (uint32_t)(bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(bitmap >> 32);

    if (bind) {
        payload.arg2 = (uint32_t)flag;
        val_ffa_notification_bind(&payload);
    } else
    {
        payload.arg2 = 0;
        val_ffa_notification_unbind(&payload);
    }

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification bind/unbind err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(1);
    }

    return status;
}

uint32_t sp_signals_vm_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t ret;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    uint32_t npi_id;
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);

    /* notfication bitmap setting
    VM1 Global Notification   - 0
    VM1 per-vCPU Notification - 1 */
    uint64_t notifications_bitmap_1 = FFA_NOTIFICATION(1) | FFA_NOTIFICATION(0);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_NPI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed to retrieve NPI err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    npi_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(npi_id, npi_irq_handler))
    {
        LOG(ERROR, "NPI interrupt register failed\n");
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
    LOG(DBG, "Interrupt registration Done NPI ID %x %p\n", npi_id, npi_irq_handler);

    /* Select SP Test Entry Functions */
    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    LOG(DBG, "Enable NPI\n");
    val_irq_enable(npi_id, 0xA);

    LOG(DBG, "Bind Notification from SP\n");
    /* Bind Notifications from SP1 */
    status = notification_bind_unbind_helper(notifications_bitmap_1, true, 0x0, sender, recipient);
    if (status != VAL_SUCCESS)
        goto free_interrupt;

    LOG(DBG, "Client notifications_bitmap_1 bind complete\n");

    /* Run SP1 */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    payload.arg3 =  notifications_bitmap_1;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d\n", payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto unbind;
    }

#if defined(TARGET_LINUX)
    val_sleep(5000);
#endif

    if (npi_flag == 1) {
        LOG(DBG, "NPI interrupt handled\n");
    } else {
        LOG(ERROR, "NPI interrupt not received\n");
        status = VAL_ERROR_POINT(6);
        goto free_interrupt;
    }
    val_irq_disable(npi_id);

    /* Get Notifications from SP1 */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_BITMAP_SP;
    val_ffa_notification_get(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification get err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(8);
        goto unbind;
    }

    LOG(DBG, "notifications_bitmap_1 %x payload.arg2 %x\n",
        notifications_bitmap_1, (uint32_t)payload.arg2);

    /* Compare Notification bitmap */
    if ((notifications_bitmap_1) != (uint32_t)payload.arg2)
    {
        LOG(ERROR, "Not received expected notification err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(9);
    }

    /* Call SP1 */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed\n");
        status = VAL_ERROR_POINT(11);
        goto unbind;
    }

unbind:
    ret = notification_bind_unbind_helper(notifications_bitmap_1, false, 0x0, sender, recipient);
    status = status ? status : ret;

free_interrupt:
    if (val_irq_unregister_handler(npi_id))
    {
        LOG(ERROR, "IRQ handler unregister failed\n");
        status = status ? status : VAL_ERROR_POINT(11);
    }

exit:
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}
