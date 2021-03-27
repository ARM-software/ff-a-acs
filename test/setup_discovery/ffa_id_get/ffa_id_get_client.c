/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_id_get_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t output_reserve_count = 5;
    ffa_endpoint_id_t expected_id = val_get_endpoint_id(GET_CLIENT_LOGIC_ID(test_run_data));

    val_memset(&payload, 0, sizeof(ffa_args_t));

    /* FFA_SUCCESS case: Returns 16-bit ID of calling FF-A component. */
    val_ffa_id_get(&payload);
    if (payload.fid != FFA_SUCCESS_32)
    {
        LOG(ERROR, "\tCheck failed for ffa_id_get success case\n", 0, 0);
        return VAL_ERROR_POINT(1);
    }

    /* W2: ID of the caller. Bit[31:16]: Reserved (MBZ). Bit[15:0]: ID */
    if (((payload.arg2 & 0xffff) != expected_id) ||
            (((payload.arg2 >> 16) & 0xffff) != 0x0))
    {
        LOG(ERROR, "\tID mismatch, expected=0x%x but actual=0x%x\n", expected_id, payload.arg2);
        return VAL_ERROR_POINT(2);
    }

    /* Return value for reserved registers - MBZ */
    if (val_reserve_param_check(payload, output_reserve_count))
    {
        LOG(ERROR, "\tReceived non-zero value for reserved registers\n",
            0, 0);
        return VAL_ERROR_POINT(3);
    }

    return VAL_SUCCESS;
}
