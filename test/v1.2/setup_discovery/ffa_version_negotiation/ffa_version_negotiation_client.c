/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_version_negotiation_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t input_version_number = 0;

    LOG(DBG, "FFA_VERSION Negotiation Error Check");
    input_version_number = (uint32_t)((FFA_VERSION_MAJOR << 16) | (FFA_VERSION_MINOR-1));
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = input_version_number;
    val_ffa_version(&payload);
    if (payload.fid != FFA_ERROR_NOT_SUPPORTED)
    {
        LOG(ERROR, "Relayer Must Return Error For Version Re-Negotiation fid %x arg2 %x",
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    LOG(DBG, "FFA_VERSION Check With Current Version");
    input_version_number = (uint32_t)((FFA_VERSION_MAJOR << 16) | (FFA_VERSION_MINOR));
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = input_version_number;
    val_ffa_version(&payload);
    if (payload.fid == FFA_ERROR_32 || payload.arg2 == FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "Relayer Must Not Return Error For Negotiated Version fid %x arg2 %x",
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    /* Unused argument */
    (void)test_run_data;
    return VAL_SUCCESS;
}
