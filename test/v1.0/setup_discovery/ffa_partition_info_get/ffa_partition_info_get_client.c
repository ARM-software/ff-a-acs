/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
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

static ffa_args_t ffa_partition_info_get(const uint32_t uuid[4], uint32_t flags)
{
    ffa_args_t args = {
                .arg1 = uuid[0],
                .arg2 = uuid[1],
                .arg3 = uuid[2],
                .arg4 = uuid[3],
                .arg5 = flags,
    };

    val_ffa_partition_info_get(&args);

    return args;
}

static uint32_t ffa_partition_info_wrong_test(void)
{
    uint32_t uuid[4] = {1};
    const uint32_t null_uuid[4] = {0};
    uint32_t status = VAL_SUCCESS;
    uint32_t flags = FFA_PARTITION_INFO_FLAG_RETDESC;

    ffa_args_t payload = ffa_partition_info_get(uuid, flags);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Invalid parameter check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    LOG(DBG, "UUID %x %x %x %x\n", uuid[0], uuid[1], uuid[2], uuid[3]);

    /* First sanity call */
    payload = ffa_partition_info_get(null_uuid, flags);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Invalid fid received, fid=0x%x\n", payload.fid);
        status = VAL_ERROR_POINT(2);
        goto rx_release1;
    }

    /* Second sanity call without releasing rx buffer - BUSY error code check */
    payload = ffa_partition_info_get(null_uuid, flags);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_BUSY))
    {
        LOG(ERROR, "Busy error check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(3);
        goto rx_release1;
    }

rx_release1:
    /* Release the RX buffer */
    if (val_rx_release())
    {
        LOG(ERROR, "Rx release failed\n");
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
        LOG(DBG, "id %x exec_context %x properties %x exp properties %x\n", info_get[i].id,
        info_get[i].exec_context, info_get[i].properties, expected_ep[0].ep_properties);

        if (expected_ep[0].ec_count != info_get[i].exec_context)
        {
            LOG(ERROR, "Data mismatch for endpoint id: info.id=%x\n", expected_ep[0].id);
            LOG(ERROR, "expected_ep[0].ec_count=%x, info.exec_context=%x\n",
                expected_ep[0].ec_count,  info_get[i].exec_context);
            return 0;
        }

#if (PLATFORM_SP_EL == 1)
        if (VAL_IS_ENDPOINT_SECURE(val_get_curr_endpoint_logical_id()))
        {
            if (expected_ep[0].ep_properties != info_get[i].properties)
            {
                LOG(ERROR, "Data mismatch for endpoint id: info.id=%x\n",
                    expected_ep[0].id, 0);
                LOG(ERROR, "expected_ep[0].ep_properties=%x, info.properties=%x\n",
                    expected_ep[0].ep_properties,  info_get[i].properties);
                return 0;
            }
        }
        else
#endif
        {
            if (expected_ep[0].ep_properties < info_get[i].properties)
            {
                LOG(ERROR, "Data mismatch for endpoint id: info.id=%x\n",
                    expected_ep[0].id);
                LOG(ERROR, "expected_ep[0].ep_properties=%x, info.properties=%x\n",
                    expected_ep[0].ep_properties,  info_get[i].properties);
                return 0;
            }
        }
        return 1;
    }
    LOG(ERROR, "Endpoint-id=%x info not found\n", expected_ep[0].id);
    return 0;
}

static uint32_t ffa_partition_info_helper(void *rx_buff, const uint32_t uuid[4],
                               const val_endpoint_info_t *expected,
                               const uint16_t expected_count, uint32_t flags)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t reserve_param_count;
    ffa_partition_info_t *info;
    uint32_t i;

    payload = ffa_partition_info_get(uuid, flags);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "ffa_partition_info_get failed %x\n", uuid[0]);
        return VAL_ERROR_POINT(5);
    }

    /* Check output w4-w7 reserved(MBZ) */
    reserve_param_count = 4;
    if (val_reserve_param_check(payload, reserve_param_count))
    {
        LOG(ERROR, "reserved registers must be zero\n");
        status = VAL_ERROR_POINT(6);
        goto rx_release;
    }

    LOG(DBG, "Count %x flags %x", payload.arg2, flags);

    if (payload.arg2 < expected_count)
    {
        LOG(ERROR, "Count mismatched, expected >=%x, actual=%x",
                 expected_count, payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto rx_release;
    }

    if (flags == FFA_PARTITION_INFO_FLAG_RETDESC)
    {
        info = (ffa_partition_info_t *)rx_buff;
        for (i = 0; i < expected_count; i++)
        {
            if (!is_matching_endpoint_found(&expected[i], &info[0], payload.arg2))
            {
                status = VAL_ERROR_POINT(9);
                goto rx_release;
            }
        }
    }
    else if (flags == FFA_PARTITION_INFO_FLAG_RETCOUNT)
    {
        if (payload.arg3)
        {
            LOG(ERROR, "If Bit[0] = b’1, size field MBZ.\n");
            status = VAL_ERROR_POINT(10);
            return status;
        }
        else
            return status;
    }

rx_release:
    /* Release the RX buffer */
    if (val_rx_release())
    {
        LOG(ERROR, "Rx release failed\n");
        status = VAL_ERROR_POINT(11);
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

    tx_buff = val_aligned_alloc(PAGE_SIZE_4K, size);
    rx_buff = val_aligned_alloc(PAGE_SIZE_4K, size);
    if (rx_buff == NULL || tx_buff == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer\n");
        status = VAL_ERROR_POINT(12);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)tx_buff, (uint64_t)rx_buff, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed\n");
        status = VAL_ERROR_POINT(13);
        goto free_memory;
    }

    ep_info = val_get_endpoint_info();
    if (!ep_info)
    {
        status = VAL_ERROR_POINT(14);
        LOG(ERROR, "Endpoint info failed\n");
        goto unmap_rxtx;
    }

    /* Endpoint can request information for a subset of partitions in the
     * system by specifying the non-Nil UUID.
     */

    if (val_is_partition_valid(EP_ID1))
    {
        if (ffa_partition_info_helper(rx_buff, ep_info[EP_ID1].uuid, &ep_info[EP_ID1],
                                       1, FFA_PARTITION_INFO_FLAG_RETDESC))
        {
            status = VAL_ERROR_POINT(15);
            goto unmap_rxtx;
        }
    }

    if (val_is_partition_valid(EP_ID2))
    {
        if (ffa_partition_info_helper(rx_buff, ep_info[EP_ID2].uuid, &ep_info[EP_ID2],
                                       1, FFA_PARTITION_INFO_FLAG_RETDESC))
        {
            status = VAL_ERROR_POINT(16);
            goto unmap_rxtx;
        }
    }

    if (val_is_partition_valid(EP_ID3))
    {
        if (ffa_partition_info_helper(rx_buff, ep_info[EP_ID3].uuid, &ep_info[EP_ID3],
                                       1, FFA_PARTITION_INFO_FLAG_RETDESC))
        {
            status = VAL_ERROR_POINT(17);
            goto unmap_rxtx;
        }
    }

    /* Endpoint can request information for all partitions in the system
     * including the caller by specifying the Nil UUID.
     */
    count = (uint16_t)val_get_secure_partition_count();

    if (ffa_partition_info_helper(rx_buff, null_uuid, &ep_info[EP_ID1],
                                    count, FFA_PARTITION_INFO_FLAG_RETDESC))
    {
        status = VAL_ERROR_POINT(20);
        goto unmap_rxtx;
    }

    #if (PLATFORM_FFA_V >= FFA_V_1_1)
    if (ffa_partition_info_helper(rx_buff, null_uuid, &ep_info[EP_ID1],
                                    count, FFA_PARTITION_INFO_FLAG_RETCOUNT))
    {
        status = VAL_ERROR_POINT(21);
        goto unmap_rxtx;
    }
    #endif

    if (ffa_partition_info_wrong_test())
    {
        status = VAL_ERROR_POINT(22);
        goto unmap_rxtx;
    }

unmap_rxtx:
    if (val_rxtx_unmap(val_get_endpoint_id(client_logical_id)))
    {
        LOG(ERROR, "val_rxtx_unmap failed\n");
        status = status ? status : VAL_ERROR_POINT(23);
    }

free_memory:
    if (val_free(rx_buff) || val_free(tx_buff))
    {
        LOG(ERROR, "val_memory_free failed\n");
        status = status ? status : VAL_ERROR_POINT(24);
    }
    return status;
}
