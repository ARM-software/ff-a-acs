/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void ns_intr_queued_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=VM1, server=SP3 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, SP3)))
        return;
}
