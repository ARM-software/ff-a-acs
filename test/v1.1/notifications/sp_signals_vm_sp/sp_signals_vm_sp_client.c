/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define WD_TIME_OUT 100U

#ifndef TARGET_LINUX
static volatile uint32_t sri_flag;
static int sri_irq_handler(void)
{
    sri_flag = 1;
    LOG(DBG, "SRI IRQ Handler Processed\n");
    return 0;
}
#endif

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
        LOG(ERROR, "Failed notification unbind err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(1);
    }

    return status;
}

uint32_t sp_signals_vm_sp_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t ret;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);
    ffa_endpoint_id_t recipient_1 = val_get_endpoint_id(SP2);

    uint32_t test_run_data_1 = TEST_RUN_DATA(GET_TEST_NUM((uint32_t)test_run_data),
            (uint32_t)sender, (uint32_t)recipient_1, GET_TEST_TYPE((uint32_t)test_run_data));

#ifndef TARGET_LINUX
    uint32_t sri_id = 0;
#endif

    /* notfication bitmap setting
    VM1 Global Notification   - 0
    VM1 per-vCPU Notification - 1
    SP2 Global Notification   - 5 */
    uint64_t notifications_bitmap_1 = FFA_NOTIFICATION(0);
    uint64_t notifications_bitmap_2 = FFA_NOTIFICATION(1);
    uint64_t notifications_bitmap_3 = FFA_NOTIFICATION(5);

    /* Query SRI Feature */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_SRI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed to retrieve SRI err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }


#ifndef TARGET_LINUX
    /* Register SRI Int */
    sri_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(sri_id, sri_irq_handler))
    {
        LOG(ERROR, "SRI interrupt register failed\n");
        status = VAL_ERROR_POINT(3);
        goto exit;
    }
    LOG(DBG, "Interrupt registration Done SRI ID %x\n", sri_id);
#endif

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0)
    /* Create Notification Bitmap for VM in absence of NS-Hyp */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    payload.arg2 = 1;
    val_ffa_notification_bitmap_create(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "  Notification bitmap create failed %x\n", payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto exit;
    }
#endif

    /* Select SP Test Entry Functions */
    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    val_select_server_fn_direct(test_run_data_1, 0, 0, 0, 0);

    /* Bind Notifications from SP1 */
    status = notification_bind_unbind_helper(notifications_bitmap_1, true, 0x0, sender, recipient);
    if (status != VAL_SUCCESS)
        goto bitmap_destroy;

    status = notification_bind_unbind_helper(notifications_bitmap_2, true, 0x1, sender, recipient);
    if (status != VAL_SUCCESS)
        goto bitmap_destroy;

#ifndef TARGET_LINUX
    /* Enable Schedule Receiver Interrupt */
    val_irq_enable(sri_id, 0xA);
#endif

    /* Run SP1 to start T-WD */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    payload.arg3 =  notifications_bitmap_1;
    payload.arg4 =  notifications_bitmap_2;
    payload.arg5 =  notifications_bitmap_3;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d\n", payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto unbind;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient_1;
    payload.arg3 =  notifications_bitmap_3;
    payload.arg4 =  recipient;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d\n", payload.arg2);
        status = VAL_ERROR_POINT(6);
        goto unbind;
    }

    /* Wait for T-WD interrupt */
    val_sp_sleep(WD_TIME_OUT);

#ifndef TARGET_LINUX
    if (sri_flag == 1) {
        LOG(DBG, "SRI inerrupt handled\n");
    } else {
        LOG(ERROR, "SRI inerrupt not received\n");
        status = VAL_ERROR_POINT(7);
        goto unbind;
    }
    val_irq_disable(sri_id);
#endif

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

    LOG(DBG, "notifications_bitmap_1 %x notifications_bitmap_2 %x payload.arg2 %x\n",
        notifications_bitmap_1, notifications_bitmap_2, (uint32_t)payload.arg2);

    /* Compare Notification bitmap */
    if ((notifications_bitmap_1|notifications_bitmap_2) != (uint32_t)payload.arg2)
    {
        LOG(ERROR, "Not received expected notification err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(9);
    }

    /* Call SP2 to process notification from SP1 */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient_1;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed\n");
        status = VAL_ERROR_POINT(10);
        goto unbind;
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

    ret = notification_bind_unbind_helper(notifications_bitmap_2, false, 0x1, sender, recipient);
    status = status ? status : ret;

bitmap_destroy:
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0)
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    val_ffa_notification_bitmap_destroy(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Bitmap destroy failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(12);
    }
#endif

exit:
#ifndef TARGET_LINUX
    if (val_irq_unregister_handler(sri_id))
    {
        LOG(ERROR, "IRQ handler unregister failed\n");
        status = status ? status : VAL_ERROR_POINT(13);
    }
#endif

    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    status = status ? status : (uint32_t)payload.arg3;

    payload = val_select_server_fn_direct(test_run_data_1, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}
