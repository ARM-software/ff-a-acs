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

uint32_t ffa_msg_send_error_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    const char message[] = "FFA ACS suite";
    ffa_partition_info_t *info;
    void *rx_buff, *tx_buff;
    uint32_t i, size = PAGE_SIZE_4K, count;
    const uint32_t null_uuid[4] = {0};

    if (val_is_ffa_feature_supported(FFA_MSG_SEND_32))
    {
        LOG(TEST, "FFA_MSG_SEND_32 not supported, skipping the check\n");
        return VAL_SKIP_CHECK;
    }

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
        goto rxtx_unmap;
    }

    info = (ffa_partition_info_t *)rx_buff;
    count = (uint32_t)payload.arg2;

    LOG(DBG, "Partition info count %x\n", (uint32_t)payload.arg2);

    if (val_rx_release())
    {
        LOG(ERROR, "Rx release failed\n");
        status = VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    /* Send a indirect message to an endpoint that only supports receipt of
     * direct requests must be rejected by the Hypervisor
     */
    for (i = 0; i < count; i++)
    {
        LOG(DBG, "info.id %x\n", info[i].id);
        if (info[i].id == val_get_curr_endpoint_id())
            continue;

        if ((info[i].properties & FFA_RECEIPT_DIRECT_REQUEST_SUPPORT) &&
                ((info[i].properties & FFA_INDIRECT_MESSAGE_SUPPORT) == 0))
            break;
    }

    if (i == count)
    {
        LOG(TEST, "Skipping the check, required endpoint not found\n");
        status = VAL_SKIP_CHECK;
        goto rxtx_unmap;
    }
    /* Fill the payload and send message to server */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_memcpy(tx_buff, message, sizeof(message));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                              info[i].id;
    payload.arg3 = sizeof(message);
    payload.arg4 = 0;
    LOG(DBG, "Sending indirect msg to epid=0x%x\n", info[i].id);
    val_ffa_msg_send(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "Unexpected return status, fid=0x%x, err=0x%x\n",
                payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(5);
    }

rxtx_unmap:
    if (val_rxtx_unmap(val_get_endpoint_id(client_logical_id)))
    {
        LOG(ERROR, "RXTX_UNMAP failed\n");
        status = status ? status : VAL_ERROR_POINT(6);
    }

free_memory:
    if (val_memory_free(rx_buff, size) || val_memory_free(tx_buff, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed\n");
        status = status ? status : VAL_ERROR_POINT(7);
    }
    return status;
}
