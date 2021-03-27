/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

const test_suite_info_t test_suite_list[] = {
    {0, ""},
    {TESTSUITE_SETUP_DISCOVERY, "Running 'setup_discovery' test suite..\n"},
    {TESTSUITE_DIRECT_MESSAGING, "Running 'direct_messaging' test suite..\n"},
    {TESTSUITE_INDIRECT_MESSAGING, "Running 'indirect_messaging' test suite..\n"},
    {TESTSUITE_MEMORY_MANAGE, "Running 'memory_management' test suite..\n"},
};

const test_db_t test_list[] = {
    /* {suite_num, "testname", client_fn_list, server_fn_list} */
    {0, "", NULL, NULL, NULL, NULL},

#if (SUITE == all || SUITE == setup_discovery)
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_version),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_features),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_id_get),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_partition_info_get),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rx_release),
    CLIENT_SEC_CPU_TEST_FN(TESTSUITE_SETUP_DISCOVERY, mp_execution_contexts),
    CLIENT_SERVER_SEC_CPU_TEST_FN(TESTSUITE_SETUP_DISCOVERY, up_migrate_capable),
#endif

#if (SUITE == all || SUITE == direct_messaging)
    CLIENT_SERVER_TEST_FN(TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_32),
    CLIENT_SERVER_TEST_FN(TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_64),
    CLIENT_TEST_FN_ONLY(TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_error),
    CLIENT_TEST_FN_ONLY(TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_error1),
#endif

#if (SUITE == all || SUITE == indirect_messaging)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INDIRECT_MESSAGING, ffa_msg_send),
    CLIENT_TEST_FN_ONLY(TESTSUITE_INDIRECT_MESSAGING, ffa_msg_send_error),
#endif

#if (SUITE == all || SUITE == memory_manage)
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_handle),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_tag),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_mmio),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_lend_mmio),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_donate_mmio),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_retrieve),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_64_read_only_chk_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_64_read_only_chk_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_64_read_only_chk_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_32_read_only_chk_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_32_read_only_chk_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share_32_read_only_chk_vmvm),
#endif

    {0, "", NULL, NULL, NULL, NULL},
};
