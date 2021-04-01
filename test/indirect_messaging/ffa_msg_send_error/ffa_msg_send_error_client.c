/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_msg_send_error_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    const char message[] = "FFA ACS suite";
    mb_buf_t mb;
    val_endpoint_info_t *ep_info;
    uint32_t i, size = PAGE_SIZE_4K;

    if (val_is_ffa_feature_supported(FFA_MSG_SEND_32))
    {
        LOG(TEST, "\tFFA_MSG_SEND_32 not supported, skipping the check\n", 0, 0);
        return VAL_SKIP_CHECK;
    }

    ep_info = val_get_endpoint_info();
    if (!ep_info)
    {
        LOG(ERROR, "\tget_endpoint_info error!\n", 0, 0);
        return VAL_ERROR_POINT(1);
    }

    /* Send a indirect message to an endpoint that only supports receipt of
     * direct requests must be rejected by the Hypervisor
     */
    for (i = 1; i < (VAL_TOTAL_EP_COUNT + 1); i++)
    {
        if ((ep_info[i].ep_properties & FFA_RECEIPT_DIRECT_REQUEST_SUPPORT) &&
            !(ep_info[i].ep_properties & FFA_INDIRECT_MESSAGE_SUPPORT))
            break;
    }

    if (i == (VAL_TOTAL_EP_COUNT + 1))
    {
        LOG(INFO, "\tSkipping the check, not found expected endpoint\n", 0, 0);
        return VAL_SKIP_CHECK;
    }

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "\tRxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(3);
        goto free_memory;
    }

    /* Fill the payload and send message to server */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_memcpy(mb.send, message, sizeof(message));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                              ep_info[i].id;
    payload.arg3 = sizeof(message);
    payload.arg4 = 0;
    val_ffa_msg_send(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        status = VAL_ERROR_POINT(4);
    }

    if (val_rxtx_unmap(val_get_endpoint_id(client_logical_id)))
    {
        LOG(ERROR, "\tRXTX_UNMAP failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(5);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "\tfree_rxtx_buffers failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(6);
    }
    return status;
}
