/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"
#include "ffa_version_data.h"

uint32_t ffa_version_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t input_version_number, i;
    uint32_t num_checks = sizeof(check)/sizeof(check[0]);
    uint32_t output_reserve_count = 7;

    for (i = 0; i < num_checks; i++)
    {
        input_version_number = (uint32_t)((check[i].major << 16)
                                          | check[i].minor);
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = input_version_number;
        val_ffa_version(&payload);
        if (payload.fid != check[i].expected_status)
        {
            /* Filter Index-1: Valid non-zero input version */
            #if (PLATFORM_FFA_V_1_1 == 1 || PLATFORM_FFA_V_ALL == 1)
            if (!(i == 1 && ((check[i].major == payload.fid >> 16)
                         &&  (check[i].minor <= (payload.fid & 0xFFFF)))))
            #endif
            {
            LOG(ERROR, "\tCheck failed for iteration = %d\n", i, 0);
            LOG(ERROR, "\tExpected=0x%x but Actual=0x%x\n",
                        check[i].expected_status,
                        payload.fid);
            return VAL_ERROR_POINT(1);
            }
        }

        /* Reserved registers (MBZ) */
        if (val_reserve_param_check(payload, output_reserve_count))
        {
            LOG(ERROR, "\tReceived non-zero value for reserved registers\n",
                0, 0);
            return VAL_ERROR_POINT(2);
        }

    }

    /* Unused argument */
    (void)test_run_data;
    return VAL_SUCCESS;
}
