/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t rxtx_exclusive_access_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint64_t size = PAGE_SIZE_4K;
    uint64_t tx_buff = (args.arg5 << 32) | args.arg4;
    uint64_t rx_buff = (args.arg7 << 32) | args.arg6;

    /* Try to register the client buffers from servers */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = tx_buff;
    payload.arg2 = rx_buff;
    payload.arg3 = size/PAGE_SIZE_4K;
    val_ffa_rxtx_map_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tCheck failed for rxtx_map: fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(1);
    }

    val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    return status;
}
