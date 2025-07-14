/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t ffa_feature_query(uint32_t fid, char *str)
{
    ffa_args_t payload;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = fid;
    val_ffa_features(&payload);

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(DBG, "%s -> feature not supported\n", str);
        return VAL_ERROR_POINT(1);
    } else if (payload.fid == FFA_SUCCESS_32 || payload.fid == FFA_SUCCESS_64)
    {
        LOG(DBG, "%s -> feature supported\n", str);
        return VAL_SUCCESS;
    }
    else
    {
        LOG(ERROR, "%s-Invalid return code received, fid=%x, err=%x\n", str,
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(2);
    }
}

uint32_t notification_comp_client(uint32_t test_run_data)
{
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t status, status_1;

    if (!VAL_IS_ENDPOINT_SECURE(client_logical_id))
    {
        status = ffa_feature_query(FFA_NOTIFICATION_BITMAP_CREATE,
                                            "FFA_NOTIFICATION_BITMAP_CREATE");
        if (status)
            return status;

        status = ffa_feature_query(FFA_NOTIFICATION_BITMAP_DESTROY,
                                        "FFA_NOTIFICATION_BITMAP_DESTROY");
        if (status)
            return status;

        status = ffa_feature_query(FFA_NOTIFICATION_INFO_GET_32, "FFA_NOTIFICATION_INFO_GET_32");
        status_1 = ffa_feature_query(FFA_NOTIFICATION_INFO_GET_64,
                                        "FFA_NOTIFICATION_INFO_GET_64");
        if (status && status_1)
            return VAL_ERROR_POINT(3);
    }

    status = ffa_feature_query(FFA_NOTIFICATION_BIND, "FFA_NOTIFICATION_BIND");
    if (status)
        return status;

    status = ffa_feature_query(FFA_NOTIFICATION_UNBIND, "FFA_NOTIFICATION_UNBIND");
    if (status)
        return status;

    status = ffa_feature_query(FFA_NOTIFICATION_SET, "FFA_NOTIFICATION_SET");
    if (status)
        return status;

    status = ffa_feature_query(FFA_NOTIFICATION_GET, "FFA_NOTIFICATION_GET");
    if (status)
        return status;

    return VAL_SUCCESS;
}
