/*
 * Copyright (c) 2025-2026, Arm Limited or its affiliates. All rights reserved.
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

    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_features, FFA_FID_SKIP_CHECK),

#if (SUITE == all || SUITE == setup_discovery)
    /* FFA_V_1_0 */
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_version, FFA_VERSION_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_id_get, FFA_ID_GET_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_SETUP_DISCOVERY, ffa_partition_info_get, FFA_PARTITION_INFO_GET_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rx_release, FFA_RX_RELEASE_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rxtx_map_and_unmap, FFA_RXTX_MAP_64),
    CLIENT_SERVER_TEST_FN(TESTSUITE_SETUP_DISCOVERY, rxtx_exclusive_access, FFA_RXTX_MAP_64),
#endif

#if (SUITE == all || SUITE == direct_messaging)
    /* FFA_V_1_0 */
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_32, FFA_MSG_SEND_DIRECT_REQ_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_64, FFA_MSG_SEND_DIRECT_REQ_64),

    /* FFA_V_1_2 */
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_DIRECT_MESSAGING, ffa_direct_message2, FFA_MSG_SEND_DIRECT_REQ2_64),
#endif

#if (SUITE == all || SUITE == memory_manage)
    /* FFA_V_1_0 */
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_invalid_handle_tag, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_5, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_rw_retrieve_ro, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_retrieve_align_hint_check, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_relinquish_input_checks, FFA_MEM_RELINQUISH_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_state_machine_2, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_3, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_multiple_retrievals, FFA_MEM_RETRIEVE_REQ_ANY),

    /* FFA_V_1_1 */
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_perm_set, FFA_FID_SKIP_CHECK),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_perm_get, FFA_FID_SKIP_CHECK),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_bind, FFA_NOTIFICATION_BIND),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_unbind, FFA_NOTIFICATION_UNBIND),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_get, FFA_NOTIFICATION_GET),
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, notification_set, FFA_NOTIFICATION_SET),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_NOTIFICATIONS, notification_info_get, FFA_NOTIFICATION_INFO_GET_ANY),

    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, vm_to_sp_notification, FFA_NOTIFICATION_SET),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_NOTIFICATIONS, vm_to_sp_notification_pcpu, FFA_NOTIFICATION_SET),

    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, sp_signals_vm, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share, FFA_MEM_SHARE_ANY),
#endif
#ifdef ACS_FFA_UNVERIFIED
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks, FFA_MEM_SHARE_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks1, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(
       TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit_retrieve_check, FFA_MEM_RETRIEVE_REQ_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit_share_invalid, FFA_MEM_SHARE_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit_lend_invalid, FFA_MEM_LEND_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit_donate_invalid, FFA_MEM_DONATE_32),
#endif

    {0, "", NULL, NULL, NULL, NULL, NULL},
};

const uint32_t total_tests = sizeof(test_list)/sizeof(test_list[0]);
