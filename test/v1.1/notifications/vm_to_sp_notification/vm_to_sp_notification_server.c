/*
 * Copyright (c) 2022, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static volatile uint32_t npi_flag = 0;

static int npi_irq_handler(void)
{
    npi_flag = 1;
    return 0;
}

uint32_t vm_to_sp_notification_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    uint32_t npi_id;
    ffa_notification_bitmap_t notifications_bitmap;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_NPI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed to retrieve NPI err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    npi_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(npi_id, npi_irq_handler))
    {
        LOG(ERROR, "\t  NPI interrupt register failed\n", 0, 0);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

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

    val_secure_intr_enable(npi_id, INTERRUPT_TYPE_FIQ);
    notifications_bitmap = payload.arg3;
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bitmap >> 32);
    val_ffa_notification_bind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed notification bind err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(4);
        goto free_interrupt;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Direct response failed, err %d\n", payload.arg2, 0);
        status =  VAL_ERROR_POINT(5);
        goto unbind;
    }

    if (npi_flag == 1) {
        LOG(DBG, "\t  NPI inerrupt handled\n", 0, 0);
    } else {
        LOG(DBG, "\t  NPI inerrupt not received\n", 0, 0);
        status = VAL_ERROR_POINT(6);
        goto unbind;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_BITMAP_VM;
    val_ffa_notification_get(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed notification get err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(7);
        goto unbind;
    }

    if (notifications_bitmap != (uint32_t)payload.arg4)
    {
        LOG(ERROR, "\t  Not received expected notification err %x\n", payload.arg4, 0);
        status = VAL_ERROR_POINT(8);
    }

unbind:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = (uint32_t)(notifications_bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bitmap >> 32);
    val_ffa_notification_unbind(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed notification unbind err %x\n", payload.arg2, 0);
        status = status ? status : VAL_ERROR_POINT(9);
    }

free_interrupt:
    val_secure_intr_disable(npi_id, INTERRUPT_TYPE_FIQ);
    if (val_irq_unregister_handler(npi_id))
    {
        LOG(ERROR, "\t  IRQ handler unregister failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(10);
    }

exit:
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tDirect response failed err %x\n", payload.arg2, 0);
        status = status ? status : VAL_ERROR_POINT(11);
    }

    return status;
}

