/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

void ffa_mem_share_32_read_only_chk_vmsp_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=VM1, server=SP2 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, SP2)))
        return;

}

void ffa_mem_share_32_read_only_chk_spsp_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=SP2, server=SP3 */
    if (IS_TEST_FAIL(val_execute_test(test_num, SP2, SP3)))
        return;
}

void ffa_mem_share_32_read_only_chk_vmvm_testentry(uint32_t test_num)
{
    /* Execute test for EP combination: client=VM1, server=VM2 */
    if (IS_TEST_FAIL(val_execute_test(test_num, VM1, VM2)))
        return;
}
