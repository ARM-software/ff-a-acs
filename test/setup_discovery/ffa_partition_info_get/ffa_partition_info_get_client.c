/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
#define EP_ID1 VM1
#define EP_ID2 VM2
#define EP_ID3 VM3
#else
#define EP_ID1 SP1
#define EP_ID2 SP2
#define EP_ID3 SP3
#endif

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

static uint32_t ffa_partition_info_wrong_test(void)
{
    uint32_t uuid[4] = {1};
    const uint32_t null_uuid[4] = {0};
    uint32_t status = VAL_SUCCESS;
    ffa_args_t payload = ffa_partition_info_get(uuid);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tInvalid parameter check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    /* First sanity call */
    payload = ffa_partition_info_get(null_uuid);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tInvalid fid received, fid=0x%x\n",
            payload.fid, 0);
        status = VAL_ERROR_POINT(2);
        goto rx_release1;
    }

    /* Second sanity call without releasing rx buffer - BUSY error code check */
    payload = ffa_partition_info_get(null_uuid);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_BUSY))
    {
        LOG(ERROR, "\tBusy error check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(3);
        goto rx_release1;
    }

rx_release1:
    /* Release the RX buffer */
    if (val_rx_release())
    {
        LOG(ERROR, "Rx release failed\n", 0, 0);
        status = VAL_ERROR_POINT(4);
    }

    return status;
}

static uint32_t is_matching_endpoint_found(const val_endpoint_info_t *expected_ep,
                ffa_partition_info_t *info_get, uint64_t info_get_count)
{
    uint32_t i;

    for (i = 0; i < info_get_count; i++)
    {
        if (expected_ep[0].id != info_get[i].id)
            continue;

        if (expected_ep[0].ec_count != info_get[i].exec_context)
        {
            LOG(ERROR, "\tData mismatch for endpoint id: info.id=%x\n",
                expected_ep[0].id, 0);
            LOG(ERROR, "\texpected_ep[0].ec_count=%x, info.exec_context=%x\n",
                expected_ep[0].ec_count,  info_get[i].exec_context);
            return 0;
        }

        if (VAL_IS_ENDPOINT_SECURE(val_get_curr_endpoint_logical_id()))
        {
            if (expected_ep[0].ep_properties != info_get[i].properties)
            {
                LOG(ERROR, "\tData mismatch for endpoint id: info.id=%x\n",
                    expected_ep[0].id, 0);
                LOG(ERROR, "\texpected_ep[0].ep_properties=%x, info.properties=%x\n",
                    expected_ep[0].ep_properties,  info_get[i].properties);
                return 0;
            }
        }
        else
        {
            if (expected_ep[0].ep_properties < info_get[i].properties)
            {
                LOG(ERROR, "\tData mismatch for endpoint id: info.id=%x\n",
                    expected_ep[0].id, 0);
                LOG(ERROR, "\texpected_ep[0].ep_properties=%x, info.properties=%x\n",
                    expected_ep[0].ep_properties,  info_get[i].properties);
                return 0;
            }
        }
        return 1;
    }
    LOG(ERROR, "\tEndpoint-id=%x info not found\n", expected_ep[0].id, 0);
    return 0;
}

static uint32_t ffa_partition_info_helper(void *rx_buff, const uint32_t uuid[4],
                               const val_endpoint_info_t *expected,
                               const uint16_t expected_count)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t reserve_param_count;
    ffa_partition_info_t *info;
    uint32_t i;

    payload = ffa_partition_info_get(uuid);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tffa_partition_info_get failed\n", 0, 0);
        return VAL_ERROR_POINT(5);
    }

    /* Check output w3-w7 reserved(MBZ) */
    reserve_param_count = 5;
    if (val_reserve_param_check(payload, reserve_param_count))
    {
        LOG(ERROR, "\treserved registers must be zero\n", 0, 0);
        status = VAL_ERROR_POINT(6);
        goto rx_release;
    }

    if (uuid[0] || uuid[1] || uuid[2] || uuid[3])
    {
        if (payload.arg2 != expected_count)
        {
            LOG(ERROR, "\tCount mismatched, expected=%x, actual=%x\n",
                     expected_count, payload.arg2);
            status = VAL_ERROR_POINT(7);
            goto rx_release;
        }
    }
    else
    {
        if (payload.arg2 < expected_count)
        {
            LOG(ERROR, "\tCount mismatched, expected=%x < actual=%x\n",
                     expected_count, payload.arg2);
            status = VAL_ERROR_POINT(8);
            goto rx_release;
        }
    }

    info = (ffa_partition_info_t *)rx_buff;
    for (i = 0; i < expected_count; i++)
    {
        if (!is_matching_endpoint_found(&expected[i], &info[0], payload.arg2))
        {
            status = VAL_ERROR_POINT(9);
            goto rx_release;
        }
    }

rx_release:
    /* Release the RX buffer */
    if (val_rx_release())
    {
        LOG(ERROR, "\tRx release failed\n", 0, 0);
        status = VAL_ERROR_POINT(10);
    }

    return status;
}

uint32_t ffa_partition_info_get_client(uint32_t test_run_data)
{
    val_endpoint_info_t *ep_info;
    void *rx_buff, *tx_buff;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    const uint32_t null_uuid[4] = {0};
    uint64_t size = PAGE_SIZE_4K;
    uint16_t count;

    tx_buff = val_memory_alloc(size);
    rx_buff = val_memory_alloc(size);
    if (rx_buff == NULL || tx_buff == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        status = VAL_ERROR_POINT(11);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)tx_buff, (uint64_t)rx_buff, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "\t  RxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(12);
        goto free_memory;
    }

    ep_info = val_get_endpoint_info();
    if (!ep_info)
    {
        status = VAL_ERROR_POINT(13);
        LOG(ERROR, "\t Endpoint info failed\n", 0, 0);
        goto unmap_rxtx;
    }

    /* Endpoint can request information for a subset of partitions in the
     * system by specifying the non-Nil UUID.
     */
    if (ffa_partition_info_helper(rx_buff, ep_info[EP_ID1].uuid, &ep_info[EP_ID1], 1))
    {
        status = VAL_ERROR_POINT(14);
        goto unmap_rxtx;
    }

    if (ffa_partition_info_helper(rx_buff, ep_info[EP_ID2].uuid, &ep_info[EP_ID2], 1))
    {
        status = VAL_ERROR_POINT(15);
        goto unmap_rxtx;
    }

    if (ffa_partition_info_helper(rx_buff, ep_info[EP_ID3].uuid, &ep_info[EP_ID3], 1))
    {
        status = VAL_ERROR_POINT(16);
        goto unmap_rxtx;
    }

    /* Endpoint can request information for all partitions in the system
     * including the caller by specifying the Nil UUID.
     */
    if (VAL_IS_ENDPOINT_SECURE(val_get_curr_endpoint_logical_id()))
    {
        count = VAL_S_EP_COUNT;
        if (ffa_partition_info_helper(rx_buff, null_uuid, &ep_info[SP1], count))
        {
            status = VAL_ERROR_POINT(17);
            goto unmap_rxtx;
        }
    }
    else
    {
        /* Expect VM info only when NS-hyp is present */
        if (VAL_NS_EP_COUNT > 0x1)
            count = VAL_TOTAL_EP_COUNT;
        else
            count = VAL_S_EP_COUNT;

        if (ffa_partition_info_helper(rx_buff, null_uuid, &ep_info[EP_ID1], count))
        {
            status = VAL_ERROR_POINT(18);
            goto unmap_rxtx;
        }
    }

    if (ffa_partition_info_wrong_test())
    {
        status = VAL_ERROR_POINT(19);
        goto unmap_rxtx;
    }

unmap_rxtx:
    if (val_rxtx_unmap(val_get_endpoint_id(client_logical_id)))
    {
        LOG(ERROR, "\tval_rxtx_unmap failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(20);
    }

free_memory:
    if (val_memory_free(rx_buff, size) || val_memory_free(tx_buff, size))
    {
        LOG(ERROR, "\tval_memory_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(21);
    }
    return status;
}

