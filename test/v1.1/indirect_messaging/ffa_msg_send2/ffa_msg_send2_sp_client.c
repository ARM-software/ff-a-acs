/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#ifndef TARGET_LINUX
static volatile uint32_t sri_flag;
static int sri_irq_handler(void)
{
    sri_flag = 1;
    LOG(DBG, "SRI IRQ Handler Processed");
    return 0;
}
#endif

uint32_t ffa_msg_send2_sp_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t receiver_tx = val_get_endpoint_id(server_logical_id);
    ffa_endpoint_id_t receiver_rx = val_get_endpoint_id(SP1);
    uint32_t test_run_data_rx = 0;
    uint32_t id_list_count;
    uint32_t expected_id_list_count = 0x1;
#ifndef TARGET_LINUX
    uint32_t sri_id = 0;
#endif

    test_run_data_rx = TEST_RUN_DATA(GET_TEST_NUM((uint32_t)test_run_data),
      (uint32_t)sender, (uint32_t)receiver_rx, GET_TEST_TYPE((uint32_t)test_run_data));

    if (val_is_ffa_feature_supported(FFA_MSG_SEND2_32))
    {
        LOG(ERROR, "FFA_MSG_SEND2_32 not supported, skipping the test");
        return VAL_SKIP_CHECK;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_SRI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed to retrieve SRI err %x", payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

#ifndef TARGET_LINUX
    sri_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(sri_id, sri_irq_handler))
    {
        LOG(ERROR, "SRI interrupt register failed");
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
    LOG(DBG, "Interrupt Registeration Done SRI ID %x", sri_id);
#endif

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    val_select_server_fn_direct(test_run_data_rx, 0, 0, 0, 0);

#ifndef TARGET_LINUX
    val_irq_enable(sri_id, 0xA);
#endif

    /* Send Direct Request to SP for Message send 2 */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver_tx;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_64)
    {
        LOG(ERROR, "DIRECT_RESP_64 not received fid %x err %x", payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

#ifndef TARGET_LINUX
    if (sri_flag == 1) {
        LOG(DBG, "SRI interrupt handled");
    } else {
        LOG(ERROR, "SRI interrupt not received");
        status = VAL_ERROR_POINT(3);
        goto exit;
    }
    val_irq_disable(sri_id);
#endif

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_notification_info_get_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification info get err %x", payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto exit;
    }

    id_list_count = ffa_notifications_info_get_lists_count(payload);
    LOG(DBG, "id_list_count %x expected_id_list_count %x receiver_rx %x", id_list_count,
        expected_id_list_count, payload.arg3);

    if ((id_list_count != expected_id_list_count) || (payload.arg3 != receiver_rx))
    {
        LOG(ERROR, "Notification info get not as expected."
                        "list_count %x id %x", id_list_count, payload.arg3);
        status = VAL_ERROR_POINT(5);
        goto exit;
    }

    LOG(DBG, "Schedule SP using FFA_RUN");

    /* Schedule the SP using FFA_RUN */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)val_get_endpoint_id(SP1) << 16;
    val_ffa_run(&payload);
    if (payload.fid != FFA_MSG_WAIT_32)
    {
        status = VAL_ERROR_POINT(6);
        goto exit;
    }


    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver_tx;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver_rx;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(8);
    }

exit:
#ifndef TARGET_LINUX
    if (val_irq_unregister_handler(sri_id))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = status ? status : VAL_ERROR_POINT(9);
    }
#endif

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    status = status ? status : (uint32_t)payload.arg3;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_select_server_fn_direct(test_run_data_rx, 0, 0, 0, 0);
    status = status ? status : (uint32_t)payload.arg3;

    return status;
}
