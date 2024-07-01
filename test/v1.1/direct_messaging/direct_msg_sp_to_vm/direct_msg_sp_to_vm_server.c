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
    return 0;
}
#endif

uint32_t direct_msg_sp_to_vm_server(ffa_args_t args)
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
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        status = VAL_ERROR_POINT(3);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "\tRxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(4);
        goto free_memory;
    }

#if (PLATFORM_SP_EL == 1)
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_NPI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tFailed to retrieve NPI err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    npi_id = ffa_feature_intid(payload);
    if (val_irq_register_handler(npi_id, npi_irq_handler))
    {
        LOG(ERROR, "\tNPI interrupt register failed\n", 0, 0);
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    val_secure_intr_enable(npi_id, INTERRUPT_TYPE_FIQ);
#endif

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_RUN_32)
    {
        LOG(ERROR, "\tFFA RUN not received?, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

#if (PLATFORM_SP_EL == 1)
    if (npi_flag == 1) {
        LOG(DBG, "\t  NPI inerrupt handled\n", 0, 0);
    } else {
        LOG(DBG, "\t  NPI inerrupt not received\n", 0, 0);
        status = VAL_ERROR_POINT(6);
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
        LOG(ERROR, "\t  Failed notification get err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(7);
        goto rxtx_unmap;
    }

    /* notification is signaled by setting Bit[0]
    in the framework notifications bitmap of an endpoint */
    notifications_bitmap = 0x1;

    if (notifications_bitmap != (uint32_t)payload.arg7)
    {
        LOG(ERROR, "\t  Not received expected notification err w6 %x w7 %x \n",
           payload.arg6, payload.arg7);
        status = VAL_ERROR_POINT(8);
    }

    partition_message_header = (ffa_partition_rxtx_header_t *)mb.recv;
    msg_size = partition_message_header->size;
    pages = (uint8_t *)mb.recv + sizeof(ffa_partition_rxtx_header_t);

    /* Check that sender can still access the
     * memory after retrieve operation. Also check the
     * content of memory equal to the data
     * set by receiver. */
    for (i = 0; i < msg_size; ++i)
    {
        if (pages[i] != 0xab)
        {
            LOG(ERROR, "\tRegion data mismatch after retrieve %x\n", pages[i], 0);
            status = VAL_ERROR_POINT(9);
            goto rxtx_unmap;
        }
    }

    if (val_rx_release())
    {
        LOG(ERROR, "\tval_rx_release failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(10);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    if (val_is_ffa_feature_supported(FFA_MSG_SEND_DIRECT_REQ_32) == VAL_SUCCESS)
    {
        val_ffa_msg_send_direct_req_32(&payload);
        if (payload.fid != FFA_ERROR_32 || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
        {
            LOG(ERROR, "\tDirect request must fail err %x\n", payload.arg2, 0);
            status = VAL_ERROR_POINT(11);
            goto rxtx_unmap;
        }
    }
    else if (val_is_ffa_feature_supported(FFA_MSG_SEND_DIRECT_REQ_64) == VAL_SUCCESS)
    {
        val_ffa_msg_send_direct_req_64(&payload);
        if (payload.fid != FFA_ERROR_32 || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
        {
            LOG(ERROR, "\tDirect request must fail err %x\n", payload.arg2, 0);
            status = VAL_ERROR_POINT(12);
            goto rxtx_unmap;
        }
    }
    else
    {
        LOG(ERROR, "\tAny one of direct messaging must be supported \n", 0, 0);
        status = VAL_ERROR_POINT(13);
    }

    /* Call FFA Wait */
      val_memset(&payload, 0, sizeof(ffa_args_t));
      val_ffa_msg_wait(&payload);
      if (payload.fid == FFA_ERROR_32)
      {
          LOG(ERROR, "\tCall to FFA_YIELD must not fail %x \n", payload.fid, 0);
          status = VAL_ERROR_POINT(14);
          goto rxtx_unmap;
      }



rxtx_unmap:
#if (PLATFORM_SP_EL == 1)
    if (val_irq_unregister_handler(npi_id))
    {
        LOG(ERROR, "\tIRQ handler unregister failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(15);
    }
#endif

    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "\tRXTX_UNMAP failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(16);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "\tfree_rxtx_buffers failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(17);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tDirect response failed err %x\n", payload.arg2, 0);
        status = status ? status : VAL_ERROR_POINT(18);
    }

    return status;
}
