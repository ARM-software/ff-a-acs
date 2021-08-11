/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void ffa_msg_send_error_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=VM1, server=NO_SERVER_EP */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, NO_SERVER_EP)))
        return;
}
