/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_features_intr_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_FEATURE_SRI;
    val_ffa_features(&payload);

    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(sender)))
    {
        if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_NOT_SUPPORTED)
        {
            LOG(ERROR, "Non NWd EP SRI request must return no support %x \n", payload.arg2);
            return VAL_ERROR_POINT(1);
        }
    }
    else
    {
        if (payload.fid == FFA_ERROR_32)
        {
        LOG(ERROR, "Failed to retrieve SRI err\n");
        return VAL_ERROR_POINT(1);
        }
    }

    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(sender)))
    {
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = FFA_FEATURE_NPI;
        val_ffa_features(&payload);

#if (PLATFORM_SP_EL == 0)
        if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_NOT_SUPPORTED)
        {
            LOG(ERROR, "FFA_Features Must Fail to retrieve NPI err %x\n", payload.arg2);
            return VAL_ERROR_POINT(2);
        }
#else
        if (payload.fid == FFA_ERROR_32)
        {
            LOG(ERROR, "Failed to retrieve NPI err %x\n", payload.arg2);
            return VAL_ERROR_POINT(2);
        }
#endif

#if (PLATFORM_SP_EL == 0)
        if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_NOT_SUPPORTED)
        {
            LOG(ERROR, "FFA_Features Must Fail to retrieve MEI err %x\n", payload.arg2);
            return VAL_ERROR_POINT(2);
        }
#else
        if (payload.fid == FFA_ERROR_32)
        {
            LOG(ERROR, "Failed to retrieve MEI err %x\n", payload.arg2);
            return VAL_ERROR_POINT(3);
        }
#endif

    }

    /* Unused argument */
    (void)test_run_data;
    return VAL_SUCCESS;
}
