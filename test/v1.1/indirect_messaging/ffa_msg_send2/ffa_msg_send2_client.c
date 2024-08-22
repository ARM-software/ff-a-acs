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

#define INVALID_ID 0xFFFF

uint32_t ffa_msg_send2_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t receiver = val_get_endpoint_id(server_logical_id);
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    ffa_partition_rxtx_header_t *partition_message_header;
    uint32_t id_list_count;
    uint32_t expected_id_list_count = 0x1;
#ifndef TARGET_LINUX
    uint32_t sri_id;
#endif


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

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(3);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(4);
        goto free_memory;
    }

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

#ifndef TARGET_LINUX
    val_irq_enable(sri_id, 0xA);
#endif

    partition_message_header = (ffa_partition_rxtx_header_t *)mb.send;
    partition_message_header->flags = 0;
    partition_message_header->reserved = 0;
    partition_message_header->offset = sizeof(ffa_partition_rxtx_header_t);
    partition_message_header->size = 32;

    pages = (uint8_t *)mb.send + sizeof(ffa_partition_rxtx_header_t);
    val_memset(pages, 0xab, 32);

    /* Sender and reciver same */
    partition_message_header->sender_receiver = (uint32_t)(sender | (sender << 16));

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16);
    val_ffa_msg_send2(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Msg 2 request must return error for same sender and receiver id %x",
        payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto rxtx_unmap;
    }

    /* Invalid receiver ID */
    partition_message_header->sender_receiver = (uint32_t)(INVALID_ID | (sender << 16));

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16);
    val_ffa_msg_send2(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Msg 2 request must return error for invalid ID%x",
        payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto rxtx_unmap;
    }

    /* SP with no support */
    partition_message_header->sender_receiver = (uint32_t)(val_get_endpoint_id(SP3)
                                                           | (sender << 16));

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16);
    val_ffa_msg_send2(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "Msg 2 request must return error for SP no support %x",
        payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto rxtx_unmap;
    }

    /* Send with valid ID */
    partition_message_header->sender_receiver = (uint32_t)(receiver | (sender << 16));

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16);

    val_ffa_msg_send2(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "FFA Message Send 2 request failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

#ifndef TARGET_LINUX
    if (sri_flag == 1) {
        LOG(DBG, "SRI inerrupt handled");
    } else {
        LOG(ERROR, "SRI inerrupt not received");
        status = VAL_ERROR_POINT(6);
        goto rxtx_unmap;
    }
    val_irq_disable(sri_id);
#endif

    /* Check Receiver RX Full error */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16);

    val_ffa_msg_send2(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_BUSY))
    {
        LOG(ERROR, "Msg 2 request must return error for rx buffer busy %x",
        payload.arg2, 0);
        status = VAL_ERROR_POINT(2);
        goto rxtx_unmap;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_notification_info_get_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification info get err %x", payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto rxtx_unmap;
    }

    id_list_count = ffa_notifications_info_get_lists_count(payload);
    LOG(DBG, "id_list_count %x expected_id_list_count %x receiver %x", id_list_count,
        expected_id_list_count, payload.arg3);

    if ((id_list_count != expected_id_list_count) || (payload.arg3 != receiver))
    {
        LOG(ERROR, "Notification info get not as expected."
                        "list_count %x id %x", id_list_count, payload.arg3);
        status = VAL_ERROR_POINT(8);
        goto rxtx_unmap;
    }

    LOG(DBG, "Schedule SP using FFA_RUN");

    /* Schedule the SP using FFA_RUN */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)val_get_endpoint_id(SP1) << 16;
    val_ffa_run(&payload);
    if (payload.fid != FFA_MSG_WAIT_32)
    {
        status = VAL_ERROR_POINT(9);
        goto rxtx_unmap;
    }

    /* Schedule the receiver to handle pending notification */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(10);
    }

#ifndef TARGET_LINUX
    if (val_irq_unregister_handler(sri_id))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = status ? status : VAL_ERROR_POINT(11);
    }
#endif

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(12);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(13);
    }

exit:
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    status = status ? status : (uint32_t)payload.arg3;
    return status;
}
