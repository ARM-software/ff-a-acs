/*
 * Copyright (c) 2026, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void mem_security_state_ns_bit_share_invalid_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=VM1, server=VM2 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, VM2)))
        return;

    /* Execute test for EP combination: client=VM1, server=NONE */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, NO_SERVER_EP)))
        return;

}
