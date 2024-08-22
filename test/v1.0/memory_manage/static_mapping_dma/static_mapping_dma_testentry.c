/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void static_mapping_dma_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=SP2 */
    if (IS_TEST_FAIL(val_execute_test(test_num, SP2, NO_SERVER_EP)))
        return;
}
