/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#if (PLATFORM_SP_EL == 1)
static volatile uint32_t npi_flag;

static int npi_irq_handler(void)
{
    npi_flag = 1;
    LOG(DBG, "NPI IRQ Handler Processed");
    return 0;
}
#endif

#define INVALID_ID 0xFFFF

static uint32_t ffa_msg_send2_sp_tx(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    ffa_endpoint_id_t receiver_rx = val_get_endpoint_id(SP1);
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    ffa_partition_rxtx_header_t *partition_message_header;

    if (val_is_ffa_feature_supported(FFA_MSG_SEND2_32))
    {
        LOG(ERROR, "FFA_MSG_SEND2_32 not supported, skipping the test");
        return VAL_SKIP_CHECK;
    }

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

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
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    val_ffa_msg_send2(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Msg 2 request must return error for same sender and receiver id %x",
        payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    /* Invalid receiver ID */
    partition_message_header->sender_receiver = (uint32_t)(INVALID_ID | (sender << 16));

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    val_ffa_msg_send2(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Msg 2 request must return error for invalid ID%x",
        payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

    /* SP with no support */
    partition_message_header->sender_receiver = (uint32_t)(val_get_endpoint_id(SP3)
                                                           | (sender << 16));

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;
    val_ffa_msg_send2(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "Msg 2 request must return error for SP no support %x",
        payload.arg2);
        status = VAL_ERROR_POINT(6);
        goto rxtx_unmap;
    }

   /* Send with valid ID */
    partition_message_header->sender_receiver = (uint32_t)(receiver_rx | (sender << 16));

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;

    val_ffa_msg_send2(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "FFA Message Send 2 request failed err %x", payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto rxtx_unmap;
    }

    /* Check Receiver RX Full error */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_DELAY_SRI;

    val_ffa_msg_send2(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_BUSY))
    {
        LOG(ERROR, "Msg 2 request must return error for rx buffer busy %x",
        payload.arg2);
        status = VAL_ERROR_POINT(8);
        goto rxtx_unmap;
    }

    /* Respond back to Scheduler VM  */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(9);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(10);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(11);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(12);
    }

    return status;
}


static uint32_t ffa_msg_send2_sp_rx(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    mb_buf_t mb;
    uint32_t i;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    ffa_partition_rxtx_header_t *partition_message_header;
    uint32_t msg_size;
    ffa_notification_bitmap_t notifications_bitmap = 0;
#if (PLATFORM_SP_EL == 1)
    uint32_t npi_id;
#endif

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(13);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(14);
        goto free_memory;
    }

#if (PLATFORM_SP_EL == 1)
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_NPI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed to retrieve NPI err %x", payload.arg2);
        status = VAL_ERROR_POINT(15);
        goto free_memory;
    }

    npi_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(npi_id, npi_irq_handler))
    {
        LOG(ERROR, "NPI interrupt register failed");
        status = VAL_ERROR_POINT(16);
        goto free_memory;
    }
    LOG(DBG, "NPI interrupt registered ID %x", npi_id);
    val_secure_intr_enable(npi_id, INTERRUPT_TYPE_FIQ);
#endif

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_RUN_32)
    {
        LOG(ERROR, "FFA RUN not received?, fid=0x%x, err 0x%x",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(17);
        goto rxtx_unmap;
    }

#if (PLATFORM_SP_EL == 1)
    if (npi_flag == 1) {
        LOG(DBG, "NPI inerrupt handled");
    } else {
        LOG(DBG, "NPI inerrupt not received");
        status = VAL_ERROR_POINT(18);
        goto rxtx_unmap;
    }

    val_secure_intr_disable(npi_id, INTERRUPT_TYPE_FIQ);
#endif

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = sender;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_BITMAP_SPM;
    val_ffa_notification_get(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Failed notification get err %x", payload.arg2);
        status = VAL_ERROR_POINT(19);
        goto rxtx_unmap;
    }

    /* Notification is signaled by setting Bit[0]
    in the framework notifications bitmap of an endpoint */
    notifications_bitmap = 0x1;

    if (notifications_bitmap != (uint32_t)payload.arg6)
    {
        LOG(ERROR, "Not received expected notification err w6 %x w7 %x ",
           payload.arg6, payload.arg7);
        status = VAL_ERROR_POINT(20);
    }

    partition_message_header = (ffa_partition_rxtx_header_t *)mb.recv;
    msg_size = partition_message_header->size;
    pages = (uint8_t *)mb.recv + sizeof(ffa_partition_rxtx_header_t);

    /* Check the content of memory equal to the data set by receiver. */
    for (i = 0; i < msg_size; ++i)
    {
        if (pages[i] != 0xab)
        {
            LOG(ERROR, "Region data mismatch after retrieve %x", pages[i]);
            status = VAL_ERROR_POINT(21);
            goto rxtx_unmap;
        }
    }

    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed");
        status = status ? status : VAL_ERROR_POINT(22);
    }

    /* Call FFA Wait */
      val_memset(&payload, 0, sizeof(ffa_args_t));
      val_ffa_msg_wait(&payload);
      if (payload.fid == FFA_ERROR_32)
      {
          LOG(ERROR, "Call to FFA_YIELD must not fail %x ", payload.fid);
          status = VAL_ERROR_POINT(23);
          goto rxtx_unmap;
      }

rxtx_unmap:
#if (PLATFORM_SP_EL == 1)
    if (val_irq_unregister_handler(npi_id))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = status ? status : VAL_ERROR_POINT(24);
    }
#endif

    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(25);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(26);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(27);
    }

    return status;
}


uint32_t ffa_msg_send2_sp_server(ffa_args_t args)
{

    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t curr_ep_logical_id;

    curr_ep_logical_id = val_get_curr_endpoint_logical_id();

    if (curr_ep_logical_id == SP2)
    {
        status = ffa_msg_send2_sp_tx(args);
    }
    else if (curr_ep_logical_id == SP1)
    {
        status = ffa_msg_send2_sp_rx(args);
    }

    return status;
}
