/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#ifndef TARGET_LINUX
static volatile uint32_t sri_flag;
static int sri_irq_handler(void)
{
    sri_flag = 1;\
    LOG(DBG, "SRI IRQ Handler Processed\n");
    return 0;
}
#endif

uint32_t vm_to_sp_notification_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);
    uint32_t id_list_count = 0;
    uint32_t expected_id_list_count = 0x1;
#ifndef TARGET_LINUX
    uint32_t sri_id;
#endif
    uint64_t notifications_bitmap = FFA_NOTIFICATION(12);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_SRI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed to retrieve SRI err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

#ifndef TARGET_LINUX
    sri_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(sri_id, sri_irq_handler))
    {
        LOG(ERROR, "SRI interrupt register failed\n");
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
    LOG(DBG, "Interrupt Registeration Done SRI ID %x\n", sri_id);
#endif

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0)
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    payload.arg2 = 1;
    val_ffa_notification_bitmap_create(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Notification bitmap create failed %x\n", payload.arg2);
        status = VAL_ERROR_POINT(3);
        goto bitmap_destroy;
    }
#endif

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    /* Notification id */
    payload.arg3 =  notifications_bitmap;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d\n", payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto bitmap_destroy;
    }

#ifndef TARGET_LINUX
    val_irq_enable(sri_id, 0xA);
#endif
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    payload.arg3 = (uint32_t)(notifications_bitmap & 0xFFFFFFFFU);
    payload.arg4 = (uint32_t)(notifications_bitmap >> 32);
    val_ffa_notification_set(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification set err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto bitmap_destroy;
    }

#ifndef TARGET_LINUX
    if (sri_flag == 1) {
        LOG(DBG, "SRI interrupt handled\n");
    } else {
        LOG(ERROR, "SRI interrupt not received\n");
        status = VAL_ERROR_POINT(6);
        goto bitmap_destroy;
    }
    val_irq_disable(sri_id);
#endif

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_notification_info_get_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification info get err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto bitmap_destroy;
    }

    LOG(DBG, "id_list_count %x expected_id_list_count %x recipient %x\n",
        id_list_count, expected_id_list_count, recipient);

    id_list_count = ffa_notifications_info_get_lists_count(payload);
    if ((id_list_count != expected_id_list_count) || (payload.arg3 != recipient))
    {
        LOG(ERROR, "Notification info get not as expected.\n"
                        "list_count %x id %x", id_list_count, payload.arg3);
        status = VAL_ERROR_POINT(8);
        goto bitmap_destroy;
    }

    /* Schedule the receiver to handle pending notification */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d\n", payload.arg2);
        status = VAL_ERROR_POINT(9);
    }

bitmap_destroy:
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0)
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    val_ffa_notification_bitmap_destroy(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Bitmap destroy failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(10);
    }
#endif

#ifndef TARGET_LINUX
    if (val_irq_unregister_handler(sri_id))
    {
        LOG(ERROR, "IRQ handler unregister failed\n");
        status = status ? status : VAL_ERROR_POINT(11);
    }
#endif

exit:
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    return status ? status : (uint32_t)payload.arg3;
}

