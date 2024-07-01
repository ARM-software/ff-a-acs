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
        LOG(ERROR, "\tFFA_MSG_SEND2_32 not supported, skipping the test\n", 0, 0);
        return VAL_SKIP_CHECK;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_SRI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed to retrieve SRI err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

#ifndef TARGET_LINUX
    sri_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(sri_id, sri_irq_handler))
    {
        LOG(ERROR, "\t  SRI interrupt register failed\n", 0, 0);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
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
        LOG(ERROR, "\tDIRECT_RESP_64 not received fid %x err %x\n", payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

#ifndef TARGET_LINUX
    if (sri_flag == 1) {
        LOG(DBG, "\tSRI interrupt handled\n", 0, 0);
    } else {
        LOG(ERROR, "\tSRI interrupt not received\n", 0, 0);
        status = VAL_ERROR_POINT(3);
        goto exit;
    }
    val_irq_disable(sri_id);
#endif

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_notification_info_get_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tFailed notification info get err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(4);
        goto exit;
    }

    id_list_count = ffa_notifications_info_get_lists_count(payload);

    if ((id_list_count != expected_id_list_count) || (payload.arg3 != receiver_rx))
    {
        LOG(ERROR, "\tNotification info get not as expected."
                        "list_count %x id %x\n", id_list_count, payload.arg3);
        status = VAL_ERROR_POINT(5);
        goto exit;
    }

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
        LOG(ERROR, "\tDirect request failed err %d\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(7);
        goto exit;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver_rx;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tDirect request failed err %d\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(8);
    }

exit:
#ifndef TARGET_LINUX
    if (val_irq_unregister_handler(sri_id))
    {
        LOG(ERROR, "\tIRQ handler unregister failed\n", 0, 0);
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
