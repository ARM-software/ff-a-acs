/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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

#if (SUITE == all || SUITE == setup_discovery)
    /* FFA_V_1_0 */
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_version),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_features),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_id_get),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_partition_info_get),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rx_release),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rxtx_map_and_unmap),
    CLIENT_SERVER_TEST_FN(TESTSUITE_SETUP_DISCOVERY, rxtx_exclusive_access),
#endif

#if (SUITE == all || SUITE == direct_messaging)
    /* FFA_V_1_0 */
    CLIENT_SERVER_TEST_FN(TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_32),
    CLIENT_SERVER_TEST_FN(TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_64),

    /* FFA_V_1_2 */
    CLIENT_SERVER_TEST_FN(TESTSUITE_DIRECT_MESSAGING, ffa_direct_message2),
#endif

#if (SUITE == all || SUITE == memory_manage)
    /* FFA_V_1_0 */
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_invalid_handle_tag),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_5),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_rw_retrieve_ro),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_retrieve_align_hint_check),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_relinquish_input_checks),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_state_machine_2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_multiple_retrievals),

    /* FFA_V_1_1 */
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_perm_set),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_perm_get),

    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share),
#endif
#ifdef ACS_FFA_UNVERIFIED
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit),
#endif
    {0, "", NULL, NULL, NULL, NULL, NULL},
};

const uint32_t total_tests = sizeof(test_list)/sizeof(test_list[0]);