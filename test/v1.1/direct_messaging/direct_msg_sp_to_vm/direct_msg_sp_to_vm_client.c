/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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
    LOG(DBG, "SRI IRQ Handler Processed\n");
    return 0;
}
#endif

uint32_t direct_msg_sp_to_vm_client(uint32_t test_run_data)
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
        LOG(ERROR, "FFA_MSG_SEND2_32 not supported, skipping the test\n");
        return VAL_SKIP_CHECK;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_SRI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "  Failed to retrieve SRI err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

#ifndef TARGET_LINUX
    sri_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(sri_id, sri_irq_handler))
    {
        LOG(ERROR, "  SRI interrupt register failed\n");
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
    LOG(DBG, "SRI IRQ Registered SRI ID %x\n", sri_id);
#endif

    mb.send = val_aligned_alloc(PAGE_SIZE_4K, size);
    mb.recv = val_aligned_alloc(PAGE_SIZE_4K, size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer\n");
        status = VAL_ERROR_POINT(3);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed\n");
        status = VAL_ERROR_POINT(4);
        goto free_memory;
    }

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

#ifndef TARGET_LINUX
    val_irq_enable(sri_id, 0xA);
#endif

    partition_message_header = (ffa_partition_rxtx_header_t *)mb.send;
    partition_message_header->flags = 0;
    partition_message_header->reserved_0 = 0;
    partition_message_header->offset = sizeof(ffa_partition_rxtx_header_t);
    partition_message_header->sender_receiver = (uint32_t)(receiver | (sender << 16));
    partition_message_header->size = 32;
    pages = (uint8_t *)mb.send + sizeof(ffa_partition_rxtx_header_t);
    val_memset(pages, 0xab, 32);

    /* send message type 2 to SP*/
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)sender << 16);

    val_ffa_msg_send2(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "FFA Message Send 2 request failed err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

#ifndef TARGET_LINUX
    if (sri_flag == 1) {
        LOG(DBG, "SRI inerrupt handled\n");
    } else {
        LOG(ERROR, "SRI inerrupt not received\n");
        status = VAL_ERROR_POINT(6);
        goto rxtx_unmap;
    }
    val_irq_disable(sri_id);
#endif

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_notification_info_get_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification info get err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto rxtx_unmap;
    }

    id_list_count = ffa_notifications_info_get_lists_count(payload);
    LOG(DBG, "id_list_count %x expected coutn %x receiver %x\n", id_list_count,
        expected_id_list_count, payload.arg3);

    if ((id_list_count != expected_id_list_count) || (payload.arg3 != receiver))
    {
        LOG(ERROR, "Notification info get not as expected.\n"
                        "list_count %x id %x", id_list_count, payload.arg3);
        status = VAL_ERROR_POINT(8);
        goto rxtx_unmap;
    }

    LOG(DBG, "Schedule SP using FFA_RUN\n");

    /* Schedule the SP using FFA_RUN */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)val_get_endpoint_id(SP1) << 16;
    val_ffa_run(&payload);
    if (payload.fid != FFA_MSG_WAIT_32)
    {
        status = VAL_ERROR_POINT(9);
        goto rxtx_unmap;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d\n", payload.arg2);
        status = VAL_ERROR_POINT(10);
    }

#ifndef TARGET_LINUX
    LOG(DBG, "Unregistering SRI IRQ Handler\n");
    if (val_irq_unregister_handler(sri_id))
    {
        LOG(ERROR, "IRQ handler unregister failed\n");
        status = status ? status : VAL_ERROR_POINT(11);
    }
#endif

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed\n");
        status = status ? status : VAL_ERROR_POINT(12);
    }

free_memory:
    if (val_free(mb.recv) || val_free(mb.send))
    {
        LOG(ERROR, "free_rxtx_buffers failed\n");
        status = status ? status : VAL_ERROR_POINT(13);
    }

exit:
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    status = status ? status : (uint32_t)payload.arg3;
    return status;
}
