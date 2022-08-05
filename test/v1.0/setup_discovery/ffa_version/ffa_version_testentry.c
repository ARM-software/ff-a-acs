/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void ffa_version_testentry(uint32_t test_num)
{
#if (PLATFORM_FFA_V_1_0 == 1)
    /* Execute test for EP combination: client=VM1, server=NONE */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, NO_SERVER_EP)))
        return;
#endif

    /* Execute test for EP combination: client=SP1, server=NONE */
    if (IS_TEST_FAIL(val_execute_test(test_num, SP1, NO_SERVER_EP)))
        return;
}
