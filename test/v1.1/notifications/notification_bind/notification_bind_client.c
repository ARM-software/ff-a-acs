/*
 * Copyright (c) 2022, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#define INVALID_VMID 0xFFFF

#include "test_database.h"

uint32_t notification_bind_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t receiver = val_get_endpoint_id(SP3);

    /* Invalid receiver id check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((INVALID_VMID << 16) | (sender));
    payload.arg2 = 0;
    payload.arg3 = 0;
    payload.arg4 = 0;
    val_ffa_notification_bind(&payload);
    if (payload.fid != FFA_ERROR_32 && payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "\t  Relayer must return error for invalid endpoint id err %x\n",
                            payload.arg2, 0);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }

    /* Invalid sender id check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (INVALID_VMID));
    payload.arg2 = 0;
    payload.arg3 = 0;
    payload.arg4 = 0;
    val_ffa_notification_bind(&payload);
    if (payload.fid != FFA_ERROR_32 && payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "\t  Relayer must return error for invalid endpoint id err %x\n",
                            payload.arg2, 0);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }

    /* Invalid flags check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((receiver << 16) | (sender));
    payload.arg2 = 0x10;
    payload.arg3 = 0;
    payload.arg4 = 0;
    val_ffa_notification_bind(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "\t  Relayer must return error for invalid flags err %x\n",
                            payload.arg2, 0);
        status = VAL_ERROR_POINT(3);
    }

exit:
    return status;
}
