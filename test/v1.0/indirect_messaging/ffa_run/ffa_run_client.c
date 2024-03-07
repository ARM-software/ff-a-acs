/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_run_client(uint32_t test_run_data)
{

    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM2);
#else
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
#endif

    /* Pass invalid endpoint id or vCPU id */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = recipient;
    val_ffa_run(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tInvalid endpoint id or vCPU id check failed, %x %x\n",
           payload.arg2, payload.fid);
        status = VAL_ERROR_POINT(1);
    }

    /* Input parameter reserved (MBZ) */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)recipient << 16;
    payload.arg2 = 0xFFFF;
    val_ffa_run(&payload);
    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "\tReserved register mbz check failed, err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(2);
    }

    (void)test_run_data;
    return status;
}
