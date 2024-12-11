/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

const test_suite_info_t test_suite_list[] = {
    {0, ""},
    {TESTSUITE_SETUP_DISCOVERY, "setup_discovery"},
    {TESTSUITE_DIRECT_MESSAGING, "direct_messaging"},
    {TESTSUITE_INDIRECT_MESSAGING, "indirect_messaging"},
    {TESTSUITE_MEMORY_MANAGE, "memory_management"},
    {TESTSUITE_NOTIFICATIONS, "notifications"},
    {TESTSUITE_INTERRUPTS, "interrupts"},
};

const test_db_t test_list[] = {
    /* {suite_num, "testname", client_fn_list, server_fn_list} */
    {0, "", NULL, NULL, NULL, NULL, NULL},

#if (PLATFORM_FFA_V_MULTI == 1)

#if (SUITE == all || SUITE == setup_discovery)
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_version),
#endif

#if (SUITE == all || SUITE == memory_manage)
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_lend),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_donate),
#endif
#endif

    {0, "", NULL, NULL, NULL, NULL, NULL},
};

const uint32_t total_tests = sizeof(test_list)/sizeof(test_list[0]);
