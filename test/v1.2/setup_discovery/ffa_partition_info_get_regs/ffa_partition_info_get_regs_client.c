/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
#define EP_ID1 VM1
#define EP_ID2 VM2
#define EP_ID3 VM3
#define EP_ID4 VM4
#else
#define EP_ID1 SP1
#define EP_ID2 SP2
#define EP_ID3 SP3
#define EP_ID4 SP4
#endif

/**
 * FFA_INFO_GET_REGS Helper
 *
 * Returns info get reg Get arg fescriptor
 */
static ffa_args_t ffa_partition_info_get_regs(const uint32_t uuid[4])
{
    LOG(DBG, "uuid[0] %x, uuid[1] %x uuid[2] %x uuid[3] %x\n", uuid[0], uuid[1], uuid[2], uuid[3]);
    ffa_args_t payload;
    val_memset(&payload, 0, sizeof(ffa_args_t));

    payload.arg1 = ((uint64_t)uuid[1]<<32) | uuid[0];
    payload.arg2 = ((uint64_t)uuid[3]<<32) | uuid[2];
    LOG(DBG, "arg1 %lx, arg2 %lx\n", payload.arg1, payload.arg2);
    val_ffa_partition_info_get_regs(&payload);

    return payload;
}

/**
 * Checks Error Code for Invalid UUID
 *
 * Returns VAL_SUCCESS in case of valid return code
 */
static uint32_t ffa_partition_info_get_regs_invalid_uuid_test(void)
{
    uint32_t uuid[4] = {1};
    const uint32_t null_uuid[4] = {0};
    uint32_t status = VAL_SUCCESS;

    /* FFA_INFO_GET_REG call with invalid UUID */
    ffa_args_t payload = ffa_partition_info_get_regs(uuid);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Invalid parameter check failed, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    LOG(DBG, "UUID %x %x %x %x\n", uuid[0], uuid[1], uuid[2], uuid[3]);

    /* FFA_INFO_GET_REG call with NULL UUID */
    payload = ffa_partition_info_get_regs(null_uuid);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Invalid fid received, fid=0x%x\n", payload.fid);
        status = VAL_ERROR_POINT(2);
    }

    return status;
}

/**
 * Checks returned EP properties with database
 *
 * Returns VAL_SUCCESS in case of valid EP
 */
static uint32_t is_matching_endpoint_found(const val_endpoint_info_t *expected_ep,
                ffa_partition_info_t *info_get, uint64_t info_get_count)
{
    uint32_t i;

    for (i = 0; i < info_get_count; i++)
    {

        if (expected_ep[0].id != info_get[i].id)
            continue;

        LOG(DBG, "id %x exec_context %x properties %x\n", info_get[i].id, info_get[i].exec_context,
            info_get[i].properties);

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

static uint32_t ffa_partition_info_helper(const uint32_t uuid[4],
                               const val_endpoint_info_t *expected,
                               const uint16_t count)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_partition_info_t *info;
    uint32_t i;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = ffa_partition_info_get_regs(uuid);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "partition info get failed fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
        return VAL_ERROR_POINT(5);
    }

    info = (ffa_partition_info_t *)&payload.arg3;
    for (i = 0; i < count; i++)
    {
        if (!is_matching_endpoint_found(&expected[i], &info[0], count))
        {
            status = VAL_ERROR_POINT(9);
        }
    }
    return status;
}

uint32_t ffa_partition_info_get_regs_client(uint32_t test_run_data)
{
    val_endpoint_info_t *ep_info;
    uint32_t status = VAL_SUCCESS;
    const uint32_t null_uuid[4] = {0};
    uint16_t count;

     /* Unused argument */
     (void)test_run_data;

    ep_info = val_get_endpoint_info();
    if (!ep_info)
    {
        status = VAL_ERROR_POINT(14);
        LOG(ERROR, "Endpoint info failed\n");
        goto exit;
    }

    /* Endpoint can request information for a subset of partitions in the
     * system by specifying the non-Nil UUID.
     */
    if (ffa_partition_info_helper(ep_info[EP_ID1].uuid, &ep_info[EP_ID1], 1))
    {
        status = VAL_ERROR_POINT(15);
        goto exit;
    }

    if (ffa_partition_info_helper(ep_info[EP_ID2].uuid, &ep_info[EP_ID2], 1))
    {
        status = VAL_ERROR_POINT(16);
        goto exit;
    }

    if (ffa_partition_info_helper(ep_info[EP_ID3].uuid, &ep_info[EP_ID3], 1))
    {
        status = VAL_ERROR_POINT(17);
        goto exit;
    }

    if (ffa_partition_info_helper(ep_info[EP_ID4].uuid, &ep_info[EP_ID4], 1))
    {
        status = VAL_ERROR_POINT(17);
        goto exit;
    }

    LOG(DBG, "Requesting Partition Info with Nil UUID\n");
    /* Endpoint can request information for all partitions in the system
     * including the caller by specifying the Nil UUID.
     */
    if (VAL_IS_ENDPOINT_SECURE(val_get_curr_endpoint_logical_id()))
    {
        count = VAL_S_EP_COUNT;
        if (ffa_partition_info_helper(null_uuid, &ep_info[SP1], count))
        {
            status = VAL_ERROR_POINT(18);
            goto exit;
        }
    }
    else
    {
        /* Expect VM info only when NS-hyp is present */
        if (VAL_NS_EP_COUNT > 0x1)
            count = VAL_TOTAL_EP_COUNT;
        else
            count = VAL_S_EP_COUNT;

        if (ffa_partition_info_helper(null_uuid, &ep_info[EP_ID1], count))
        {
            status = VAL_ERROR_POINT(20);
            goto exit;
        }
    }

    LOG(DBG, "Partition Info Wrong Test\n");
    if (ffa_partition_info_get_regs_invalid_uuid_test())
    {
        status = VAL_ERROR_POINT(22);
    }

exit:
    return status;
}
