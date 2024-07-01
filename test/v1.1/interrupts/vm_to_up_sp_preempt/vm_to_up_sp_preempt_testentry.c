/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void vm_to_up_sp_preempt_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=VM1, server=SP4 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, SP4)))
        return;
}
