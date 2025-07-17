/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define SPMD_LP_PARTITION_ID 0xFFC0
#define SPMD_LP_UUID {0xe98e43ad, 0xb7db524f, 0x47a3bf57, 0x1588f4e3}
#define LSP_EP_COUNT 0x1

/* SPMD Logical SP currently only supports sending direct message. */
#define SPMD_PARTITION_PROPERTIES FFA_DIRECT_REQUEST_SEND | FFA_PARTITION_EXEC_STATE_ARCH64

static val_endpoint_info_t lsp_info = {
        "SP1",
        SPMD_LP_PARTITION_ID,
        VAL_TG0_4K,
        EL1_64,
        1,
        SPMD_PARTITION_PROPERTIES,
        SPMD_LP_UUID,
};

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

        if (expected_ep[0].ep_properties != info_get[i].properties)
        {
            LOG(ERROR, "Data mismatch for endpoint id: info.id=%x\n",
                expected_ep[0].id, 0);
            LOG(ERROR, "expected_ep[0].ep_properties=%x, info.properties=%x\n",
                expected_ep[0].ep_properties,  info_get[i].properties);
            return 0;
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

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = ffa_partition_info_get_regs(uuid);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "partition info get failed fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
        return VAL_ERROR_POINT(5);
    }

    info = (ffa_partition_info_t *)&payload.arg3;

    if (!is_matching_endpoint_found(&expected[0], &info[0], count))
    {
        status = VAL_ERROR_POINT(9);
    }

    return status;
}

uint32_t ffa_partition_info_get_lsp_client(uint32_t test_run_data)
{
    uint32_t status = VAL_SUCCESS;
    const uint32_t null_uuid[4] = {0};
    uint16_t count;

    /* Unused argument */
    (void)test_run_data;

    /* Endpoint can request information for a subset of logical partitions in the
     * system by specifying the non-Nil UUID.
     */
    if (ffa_partition_info_helper(lsp_info.uuid, &lsp_info, 1))
    {
        status = VAL_ERROR_POINT(17);
        goto exit;
    }

    LOG(DBG, "Requesting Partition Info with Nil UUID\n");

    /* Endpoint can request information for all partitions in the system
     * including the caller by specifying the Nil UUID.
     */

    count = VAL_S_EP_COUNT + LSP_EP_COUNT;
    if (ffa_partition_info_helper(null_uuid, &lsp_info, count))
    {
        status = VAL_ERROR_POINT(18);
        goto exit;
    }

exit:
    return status;
}