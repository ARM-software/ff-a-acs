/*
 * Copyright (c) 2021, 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void mp_execution_contexts_testentry(uint32_t test_num)
{
#ifdef TARGET_LINUX
    LOG(TEST, "Skipping this test: MP test not supported for Linux env\n");
    val_set_status(RESULT_SKIP(VAL_SKIP_CHECK));
    return;
#endif
    /* Execute test for EP combination: client=VM1, server=NONE */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, NO_SERVER_EP)))
        return;

}

