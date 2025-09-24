/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
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
#if ((!defined(XEN_SUPPORT) && !(TARGET_LINUX_ENV == 1)) || (PLATFORM_FFA_V < FFA_V_1_2))
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
#endif

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

#if !defined(XEN_SUPPORT) && !(TARGET_LINUX_ENV == 1)
    /* Invalid receiver vcpu id check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)((INVALID_VMID << 16) | (sender));
    payload.arg2 = FFA_NOTIFICATIONS_FLAG_BITMAP_VM;
    val_ffa_notification_get(&payload);
    if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "Relayer must return error for invalid vcpu id err %x\n",
                            payload.arg2);
        status = VAL_ERROR_POINT(2);
        goto exit;
    }
#endif

#if PLATFORM_FFA_V < FFA_V_1_2
    /* Invalid flags check */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)(sender);
    payload.arg2 = 0x10000;
    val_ffa_notification_get(&payload);
    if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
    {
        LOG(ERROR, "Relayer must return error for invalid flags err %x\n",
                            payload.arg2);
        status = VAL_ERROR_POINT(3);
    }
#endif

#if !((!defined(XEN_SUPPORT) && !(TARGET_LINUX_ENV == 1)) || (PLATFORM_FFA_V < FFA_V_1_2))
    (void)(test_run_data);
#endif
exit:
    return status;
}
