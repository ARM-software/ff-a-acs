/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void ffa_msg_send_error_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=VM1, server=VM2 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, VM2)))
        return;
}
