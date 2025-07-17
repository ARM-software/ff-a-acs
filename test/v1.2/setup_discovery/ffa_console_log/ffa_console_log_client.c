/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"
#define CONSOLE_LOG_SMC_32_MAX_CHAR 24
#define CONSOLE_LOG_SMC_64_MAX_CHAR 128

static uint32_t ffa_console_log_helper(uint32_t test_run_data, uint32_t fid)
{
    char char_buff_64[] = "CONSOLE_LOG_64 TEST OUTPUT FOR 24 < CHAR CNT < 128\n";
    char char_buff_32[] = "CONSOLE_LOG_32 OUT\n";
    ffa_args_t payload;
    size_t str_len = 0;

    if (fid == FFA_CONSOLE_LOG_32)
    {
        str_len = 0;
        while (char_buff_32[str_len] != '\0')
        {
            str_len++;
        }
        LOG(DBG, "char_buff 32 str_len %d\n", str_len)
        val_memset(&payload, 0, sizeof(ffa_args_t));
        val_memcpy(&payload.arg2, char_buff_32, str_len);

        /* Invalid Char count Check SMC32 */
        payload.arg1 = 0;
        val_ffa_console_log_32(&payload);
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
        {
            LOG(ERROR, "ffa console log 32 must fail fid %x arg2 %x\n", payload.fid, payload.arg2);
            return VAL_ERROR_POINT(1);
        }

        payload.arg1 = 30;
        val_ffa_console_log_32(&payload);
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
        {
            LOG(ERROR, "ffa console log 32 must fail fid %x arg2 %x\n", payload.fid, payload.arg2);
            return VAL_ERROR_POINT(1);
        }

        /* Valid Char count i; 1 <= i <= 24 if SMC32 */
        payload.arg1 = str_len;
        val_ffa_console_log_32(&payload);
        if (payload.fid == FFA_ERROR_32)
        {
            LOG(ERROR, "ffa console log 32 failed fid %x arg2 %x\n", payload.fid, payload.arg2);
            return VAL_ERROR_POINT(1);
        }
    }


    if (fid == FFA_CONSOLE_LOG_64)
    {
        str_len = 0;
        while (char_buff_64[str_len] != '\0')
        {
            str_len++;
        }
        LOG(DBG, "char_buff 64 str_len %d\n", str_len)
        val_memset(&payload, 0, sizeof(ffa_args_t));
        val_memcpy(&payload.arg2, char_buff_64, str_len);


        /* Invalid Char count Check SMC64 */
        payload.arg1 = 0;
        val_ffa_console_log_64(&payload);
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
        {
            LOG(ERROR, "ffa console log 32 must fail fid %x arg2 %x\n", payload.fid, payload.arg2);
            return VAL_ERROR_POINT(1);
        }

        payload.arg1 = 130;
        val_ffa_console_log_64(&payload);
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
        {
            LOG(ERROR, "ffa console log 32 must fail fid %x arg2 %x\n", payload.fid, payload.arg2);
            return VAL_ERROR_POINT(1);
        }

        /* Char count i; 1 <= i <= 128 if SMC64 */
        payload.arg1 = str_len;
        val_ffa_console_log_64(&payload);
        if (payload.fid == FFA_ERROR_32)
        {
            LOG(ERROR, "ffa console log 64 failed fid %x arg 2 %x\n", payload.fid, payload.arg2);
            return VAL_ERROR_POINT(1);
        }
    }

    (void)test_run_data;
    return VAL_SUCCESS;
}

uint32_t ffa_console_log_client(uint32_t test_run_data)
{
    uint32_t status_32, status_64, status;

    status_64 = val_is_ffa_feature_supported(FFA_CONSOLE_LOG_64);
    status_32 = val_is_ffa_feature_supported(FFA_CONSOLE_LOG_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "FFA_CONSOLE_LOG not supported, skipping the check\n");
        return VAL_SKIP_CHECK;
    }
    else if (status_64 && !status_32)
    {
        status = ffa_console_log_helper(test_run_data, FFA_CONSOLE_LOG_32);
    }
    else if (!status_64 && status_32)
    {
        status = ffa_console_log_helper(test_run_data, FFA_CONSOLE_LOG_64);
    }
    else
    {
        status = ffa_console_log_helper(test_run_data, FFA_CONSOLE_LOG_64);
        if (status)
            return status;

        status = ffa_console_log_helper(test_run_data, FFA_CONSOLE_LOG_32);
        if (status)
            return status;
    }
    return status;
}
