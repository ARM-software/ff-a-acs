/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void vm_to_sp_managed_exit_testentry(uint32_t test_num)
{
#ifdef TARGET_LINUX
    LOG(TEST, "Skipping this test: Preempt test not supported for Linux env\n")
    val_set_status(RESULT_SKIP(VAL_SKIP_CHECK));
    return;
#endif

    /* Execute test for EP combination: client=VM1, server=SP2 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, SP2)))
        return;

    /* Execute test for EP combination: client=VM1, server=SP4 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, SP4)))
        return;

}
