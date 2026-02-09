/*
 * Copyright (c) 2022-2026, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#define INVALID_VMID 0xFFFF

#include "test_database.h"

uint32_t notification_get_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;

    /* Invalid receiver id check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = INVALID_VMID;
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_BITMAP_VM;
    val_ffa_notification_get(&payload);
    if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "Relayer must return error for invalid endpoint id err %x\n",
                            payload.arg2);
        status = VAL_ERROR_POINT(1);
        goto exit;
    }
    (void)(test_run_data);
exit:
    return status;
}
