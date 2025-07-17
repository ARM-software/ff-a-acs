/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#define INVALID_VMID 0xFFFF

#include "test_database.h"

uint32_t notification_bitmap_destroy_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = INVALID_VMID;
    val_ffa_notification_bitmap_destroy(&payload);
    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(sender)))
    {
        if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_NOT_SUPPORTED)
        {
            LOG(ERROR, "Relayer must return no support for invocation from secure endpoint %x\n",
                                payload.arg2);
            status = VAL_ERROR_POINT(1);
            goto exit;
        }
    }
    else
    {
        if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_INVALID_PARAMETERS)
        {
            LOG(ERROR, "Relayer must return error for invalid endpoint id %x\n",
                                payload.arg2);
            status = VAL_ERROR_POINT(2);
            goto exit;
        }
    }

    if (!VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(sender)))
    {
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0)
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = sender;
        val_ffa_notification_bitmap_destroy(&payload);
        if (payload.fid != FFA_ERROR_32 || payload.arg2 != FFA_ERROR_DENIED)
        {
            LOG(ERROR, "Relayer must return denied Notification bitmap not registered %x\n",
                                payload.arg2);
            status = VAL_ERROR_POINT(2);
            goto exit;
        }
#endif
    }

exit:
    return status;
}
