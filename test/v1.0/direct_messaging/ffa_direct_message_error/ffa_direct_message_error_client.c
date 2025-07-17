/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static ffa_args_t ffa_partition_info_get(const uint32_t uuid[4])
{
    ffa_args_t args = {
                .arg1 = uuid[0],
                .arg2 = uuid[1],
                .arg3 = uuid[2],
                .arg4 = uuid[3],
    };

    val_ffa_partition_info_get(&args);

    return args;
}

uint32_t ffa_direct_message_error_client(uint32_t test_run_data)
{
    ffa_partition_info_t *info;
    void *rx_buff, *tx_buff;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    const uint32_t null_uuid[4] = {0};
    uint64_t size = PAGE_SIZE_4K;
    uint32_t count;
    ffa_args_t payload;
    uint32_t i;
    uint32_t status = VAL_SUCCESS;

    tx_buff = val_memory_alloc(size);
    rx_buff = val_memory_alloc(size);
    if (rx_buff == NULL || tx_buff == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer\n");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)tx_buff, (uint64_t)rx_buff, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed\n");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    payload = ffa_partition_info_get(null_uuid);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Invalid fid received, fid=0x%x\n",
            payload.fid);
        status = VAL_ERROR_POINT(3);
        goto rx_release;
    }

    info = (ffa_partition_info_t *)rx_buff;
    count = (uint32_t)payload.arg2;

    LOG(DBG, "Partition info count %x\n", (uint32_t)payload.arg2);

    /* Relayer must ensure that target endpoint supports receipt of direct messages.
     * Invoke FFA_ERROR with DENIED as status if this is not the case.
     */
    for (i = 0; i < count; i++)
    {
        LOG(DBG, "info.id %x\n", info[i].id);
        if (info[i].id == val_get_curr_endpoint_id())
            continue;

        if ((info[i].properties & FFA_RECEIPT_DIRECT_REQUEST_SUPPORT) == 0)
            break;
    }

    if (i == count)
    {
        LOG(TEST, "Skipping the check, required endpoint not found\n");
        status = VAL_SKIP_CHECK;
        goto rx_release;
    }

    if (val_is_ffa_feature_supported(FFA_MSG_SEND_DIRECT_REQ_32) == VAL_SUCCESS)
    {
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = val_get_endpoint_id(client_logical_id | (uint32_t)info[i].id << 16);
        LOG(DBG, "Sending direct msg to epid=0x%x\n", info[i].id);
        val_ffa_msg_send_direct_req_32(&payload);
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
        {
            LOG(ERROR, "Unexpected return status, fid=0x%x, err=0x%x\n",
                payload.fid, payload.arg2);
            status = VAL_ERROR_POINT(4);
        }
    }
    else if (val_is_ffa_feature_supported(FFA_MSG_SEND_DIRECT_REQ_64) == VAL_SUCCESS)
    {
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = val_get_endpoint_id(client_logical_id | (uint32_t)info[i].id << 16);
        LOG(DBG, "Sending direct msg to epid=0x%x\n", info[i].id);
        val_ffa_msg_send_direct_req_64(&payload);
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
        {
            LOG(ERROR, "Unexpected return status, fid=0x%x, err=0x%x\n",
                payload.fid, payload.arg2);
            status = VAL_ERROR_POINT(5);
        }
    }
    else
    {
        LOG(TEST, "Skipping the check, direct_msg_req is not supported\n");
        status = VAL_SKIP_CHECK;
    }

rx_release:
    /* Release the RX buffer */
    if (val_rx_release())
    {
        LOG(ERROR, "Rx release failed\n");
        status = status ? status : VAL_ERROR_POINT(6);
    }

    if (val_rxtx_unmap(val_get_endpoint_id(client_logical_id)))
    {
        LOG(ERROR, "val_rxtx_unmap failed\n");
        status = status ? status : VAL_ERROR_POINT(7);
    }

free_memory:
    if (val_memory_free(rx_buff, size) || val_memory_free(tx_buff, size))
    {
        LOG(ERROR, "val_memory_free failed\n");
        status = status ? status : VAL_ERROR_POINT(8);
    }
    return status;
}
