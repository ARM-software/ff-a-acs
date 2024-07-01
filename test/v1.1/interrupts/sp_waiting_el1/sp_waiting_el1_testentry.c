/*
 * Copyright (c) 2022-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void sp_waiting_el1_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=VM1, server=SP1 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, SP1)))
        return;

    /* Execute test for EP combination: client=SP2, server=SP1 */
    if (IS_TEST_FAIL(val_execute_test(test_num, SP2, SP1)))
        return;
}
