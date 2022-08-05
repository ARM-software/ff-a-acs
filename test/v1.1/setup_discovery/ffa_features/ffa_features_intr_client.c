/*
 * Copyright (c) 2022, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_features_intr_client(uint32_t test_run_data)
{
    ffa_args_t payload;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_SRI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed to retrieve SRI err %x\n", payload.arg2, 0);
        return VAL_ERROR_POINT(1);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_NPI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed to retrieve NPI err %x\n", payload.arg2, 0);
        return VAL_ERROR_POINT(2);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_MEI;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Failed to retrieve MEI err %x\n", payload.arg2, 0);
        return VAL_ERROR_POINT(3);
    }

    /* Unused argument */
    (void)test_run_data;
    return VAL_SUCCESS;
}
