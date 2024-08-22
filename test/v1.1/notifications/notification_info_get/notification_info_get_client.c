/*
 * Copyright (c) 2022-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


#include "test_database.h"

static uint32_t notification_info_get(uint32_t fid)
{
    ffa_args_t payload;

    /* No pending notifications check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    if (fid == FFA_NOTIFICATION_INFO_GET_64)
        val_ffa_notification_info_get_64(&payload);
    else
        val_ffa_notification_info_get_32(&payload);

    if (VAL_IS_ENDPOINT_SECURE(val_get_curr_endpoint_logical_id()))
    {
        if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_NOT_SUPPORTED)
        {
            LOG(ERROR, "Relayer must return no support for secure ep err %x",
                                payload.arg2);
            return VAL_ERROR_POINT(1);
        }
    }
    else if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_NODATA)
    {
        LOG(ERROR, "Relayer must return error for no pending notifications err %x",
                            payload.arg2);
        return VAL_ERROR_POINT(2);
    }

    return VAL_SUCCESS;
}

uint32_t notification_info_get_client(uint32_t test_run_data)
{
    uint32_t status_32, status_64, status;

    status_64 = val_is_ffa_feature_supported(FFA_NOTIFICATION_INFO_GET_64);
    status_32 = val_is_ffa_feature_supported(FFA_NOTIFICATION_INFO_GET_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "FFA_NOTIFICATION_INFO_GET not supported, skipping the check");
        return VAL_SKIP_CHECK;
    }
    else if (status_64 && !status_32)
    {
        status = notification_info_get(FFA_NOTIFICATION_INFO_GET_32);
    }
    else if (!status_64 && status_32)
    {
        status = notification_info_get(FFA_NOTIFICATION_INFO_GET_64);
    }
    else
    {
        status = notification_info_get(FFA_NOTIFICATION_INFO_GET_64);
        if (status)
            return status;

        status = notification_info_get(FFA_NOTIFICATION_INFO_GET_32);
        if (status)
            return status;
    }

    /* Unused argument */
    (void)test_run_data;
    return status;
}
