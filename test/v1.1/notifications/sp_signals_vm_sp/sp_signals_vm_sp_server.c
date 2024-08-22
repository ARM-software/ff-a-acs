/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"


#define S_WD_TIMEOUT 50U

static volatile uint32_t secure_int_received;

static int wd_irq_handler(void)
{
    secure_int_received = true;
    val_twdog_disable();
    LOG(DBG, "T-WD IRQ Handler Processed");
    return 0;
}

static volatile uint32_t npi_flag;

static int npi_irq_handler(void)
{
    npi_flag = 1;
    return 0;

}

static uint32_t notification_set_helper(
                        ffa_notification_bitmap_t bitmap,
                        uint32_t flags,
                        ffa_endpoint_id_t sender,
                        ffa_endpoint_id_t receiver)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    payload.arg2 = flags;
    payload.arg3 = (uint32_t)(bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(bitmap >> 32);

    val_ffa_notification_set(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification set err %x", payload.arg2);
        status = VAL_ERROR_POINT(1);
    }
    return status;
}

static uint32_t sp1_entry_func(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_endpoint_id_t receiver_1 = val_get_endpoint_id(SP2);
    ffa_notification_bitmap_t notifications_bitmap_1;
    ffa_notification_bitmap_t notifications_bitmap_2;
    ffa_notification_bitmap_t notifications_bitmap_3;
    uint32_t flags;

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(2);
        goto exit;
    }

    /* notfication bitmap setting
    VM1 Global Notification   - bitmap_1
    VM1 per-vCPU Notification - bitmap_2
    SP2 Global Notification   - bitmap_3 */
    notifications_bitmap_1 = payload.arg3;
    notifications_bitmap_2 = payload.arg4;
    notifications_bitmap_3 = payload.arg5;

    /* Register T-WD handler for Secure Interrupt */
    if (val_irq_register_handler(PLATFORM_TWDOG_INTID, wd_irq_handler))
    {
        LOG(ERROR, "WD interrupt register failed");
        status = VAL_ERROR_POINT(3);
        goto exit;
    }

    /* Enable T-WD with Timeout */
    val_twdog_enable(S_WD_TIMEOUT);

    LOG(DBG, "T-WD IRQ handler Registered ID %x", PLATFORM_TWDOG_INTID);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid != FFA_INTERRUPT_32)
    {
        LOG(ERROR, "FFA_INTERRUPT_32 not received fid %x", payload.fid);
        status = VAL_ERROR_POINT(4);
        goto free_interrupt;
    }

    if (!secure_int_received)
    {
        LOG(ERROR, "S-Int not triggered");
        status =  VAL_ERROR_POINT(5);
        goto free_interrupt;
    }

    LOG(DBG, "T-WD IRQ Received");

    val_twdog_intr_disable();

    /* Set Notification for VM1/Scheduler, SP2 */
    flags = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    status = notification_set_helper(notifications_bitmap_1, flags, sender, receiver);
    if (status)
    {
        goto free_interrupt;
    }

    flags = FFA_NOTIFICATIONS_FLAG_PER_VCPU | FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    status = notification_set_helper(notifications_bitmap_2, flags, sender, receiver);
    if (status)
    {
        goto free_interrupt;
    }

    flags = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    status = notification_set_helper(notifications_bitmap_3, flags, sender, receiver_1);
    if (status)
    {
        goto free_interrupt;
    }

    /* Resume Preempted VM1 context */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_msg_wait(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "DIRECT_REQ_64 not received fid %x", payload.fid);
        status = VAL_ERROR_POINT(6);
        goto free_interrupt;
    }

free_interrupt:
    if (val_irq_unregister_handler(PLATFORM_TWDOG_INTID))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = status ? status : VAL_ERROR_POINT(7);
    }

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(8);
    }

    return status;
}


static uint32_t sp2_entry_func(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t sender_1;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_notification_bitmap_t notifications_bitmap;
    uint32_t npi_id;

    npi_flag = 0;

    /* Query NPI Feature */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_NPI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed to retrieve NPI err %x", payload.arg2);
        status = VAL_ERROR_POINT(9);
        goto exit;
    }

    /* Register NPI */
    npi_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(npi_id, npi_irq_handler))
    {
        LOG(ERROR, "NPI interrupt register failed");
        status = VAL_ERROR_POINT(10);
        goto exit;
    }

    LOG(DBG, "NPI interrupt registered NPI ID %x", npi_id);

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(11);
        goto exit;
    }

    /* Enable NPI */
    val_secure_intr_enable(npi_id, INTERRUPT_TYPE_FIQ);

    /* Bind SP1 notifications */
    notifications_bitmap = payload.arg3;
    sender_1 = (ffa_endpoint_id_t)payload.arg4;
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((sender_1 << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bitmap >> 32);
    val_ffa_notification_bind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification bind err %x", payload.arg2);
        status = VAL_ERROR_POINT(12);
        goto free_interrupt;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed, err %d", payload.arg2);
        status =  VAL_ERROR_POINT(13);
        goto unbind;
    }

    /* Should have received NPI */
    if (npi_flag == 1) {
        LOG(DBG, "NPI interrupt handled");
    } else {
        LOG(ERROR, "NPI inerrupt not received");
        status = VAL_ERROR_POINT(14);
        goto unbind;
    }

    /* Get Notifications */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_BITMAP_SP;
    val_ffa_notification_get(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification get err %x", payload.arg2);
        status = VAL_ERROR_POINT(15);
        goto unbind;
    }

    LOG(DBG, "Notifications_bitmap %x payload.arg2 %x", notifications_bitmap,
        (uint32_t)payload.arg2);

    if (notifications_bitmap != (uint32_t)payload.arg2)
    {
        LOG(ERROR, "Not received expected notification err %x", payload.arg2);
        status = VAL_ERROR_POINT(16);
    }

unbind:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((sender_1 << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bitmap >> 32);
    val_ffa_notification_unbind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification unbind err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(17);
    }

free_interrupt:
    val_secure_intr_disable(npi_id, INTERRUPT_TYPE_FIQ);
    if (val_irq_unregister_handler(npi_id))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = status ? status : VAL_ERROR_POINT(18);
    }

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(19);
    }
    return status;
}

uint32_t sp_signals_vm_sp_server(ffa_args_t args)
{

    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t curr_ep_logical_id;

    curr_ep_logical_id = val_get_curr_endpoint_logical_id();

    if (curr_ep_logical_id == SP1)
    {
        status = sp1_entry_func(args);
    } else if (curr_ep_logical_id == SP2)
    {
        status = sp2_entry_func(args);
    }
    return status;
}