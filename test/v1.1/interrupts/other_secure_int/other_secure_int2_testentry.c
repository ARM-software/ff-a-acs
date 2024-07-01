/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void other_secure_int2_testentry(uint32_t test_num)
{
#ifdef TARGET_LINUX
    LOG(TEST, "\tSkipping this test: UP test not supported for Linux env\n", 0, 0);
    val_set_status(RESULT_SKIP(VAL_SKIP_CHECK));
    return;
#endif

    /* Execute test for EP combination: client=VM1, server=SP1 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, SP1)))
        return;
}
