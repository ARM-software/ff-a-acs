/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_spm_id_get_client(uint32_t test_run_data)
{
    ffa_args_t payload;

    val_memset(&payload, 0, sizeof(ffa_args_t));

    /* FFA_SUCCESS case: Returns 16-bit ID of the SPMC or SPMD. */
    val_ffa_spm_id_get(&payload);
    if (payload.fid != FFA_SUCCESS_32)
    {
        LOG(ERROR, "Check failed for ffa_spm_id_get success case")
        return VAL_ERROR_POINT(1);
    }

    /* Unused argument */
    (void)test_run_data;
    return VAL_SUCCESS;
}
