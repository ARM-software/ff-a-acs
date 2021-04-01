/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_rx_release_client(uint32_t test_run_data)
{
    ffa_args_t payload;

    /* DENIED: Try to release Rx buffer ownership without register rx buffer */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_rx_release(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "\tCheck failed for rx_release denied case\n", 0, 0);
        return VAL_ERROR_POINT(1);
    }

    /* Unused argument */
    (void)test_run_data;
    return VAL_SUCCESS;
}
