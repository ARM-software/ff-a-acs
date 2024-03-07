/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
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
    {TESTSUITE_NOTIFICATIONS, "Running 'notifications' test suite..\n"},
    {TESTSUITE_INTERRUPTS, "Running 'interrupts' test suite..\n"},
};

const test_db_t test_list[] = {
    /* {suite_num, "testname", client_fn_list, server_fn_list} */
    {0, "", NULL, NULL, NULL, NULL},

#if (PLATFORM_FFA_V_ALL == 1)
#if (SUITE == all || SUITE == setup_discovery)
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_version),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_features),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_id_get),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_partition_info_get),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rx_release),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rxtx_map_and_unmap),
    CLIENT_SERVER_TEST_FN(TESTSUITE_SETUP_DISCOVERY, rxtx_exclusive_access),
#ifdef ACS_FFA_UNVERIFIED
    CLIENT_SEC_CPU_TEST_FN(TESTSUITE_SETUP_DISCOVERY, mp_execution_contexts),
    CLIENT_SERVER_SEC_CPU_TEST_FN(TESTSUITE_SETUP_DISCOVERY, up_migrate_capable),
#endif
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
#ifdef ACS_FFA_UNVERIFIED
    CLIENT_TEST_FN_ONLY(TESTSUITE_INDIRECT_MESSAGING, ffa_run),
#endif
#endif

#if (SUITE == all || SUITE == memory_manage)
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_invalid_handle_tag),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_state_machine_2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_5),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_rw_retrieve_ro),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_retrieve_align_hint_check),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_relinquish_input_checks),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_lend),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_invalid_handle_tag),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_align_hint_check),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_ro_retrieve_rw),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_rw_retrieve_ro),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_donate),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_invalid_handle_tag),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, relinquish_state_machine_2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, relinquish_state_machine_5),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, reclaim_input_error_checks),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, reclaim_zero_flag),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks4),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks6),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks6),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_5),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_4),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_5),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_6),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_4),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, relinquish_state_machine_3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, relinquish_state_machine_4),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks9),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_6),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_4),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_1),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_multiple_retrievals),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks5),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks4),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_multiple_retrievals),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks1),

#if (PLATFORM_SP_EL == 1)
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_lower_upper_boundary_32_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_lower_upper_boundary_32_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_lower_upper_boundary_64_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_lower_upper_boundary_64_vmvm),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_mem_access_after_donate_32_vm),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_mem_access_after_donate_32_sp),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_mem_access_after_donate_64_vm),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_mem_access_after_donate_64_sp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_32_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_32_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_32_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_64_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_64_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_64_vmvm),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_mem_access_after_lend_32_vm),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_mem_access_after_lend_32_sp),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_mem_access_after_lend_64_vm),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_mem_access_after_lend_64_sp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_32_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_32_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_32_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_64_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_64_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_64_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, relinquish_mem_unmap_check_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, relinquish_mem_unmap_check_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, relinquish_mem_unmap_check_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_32_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_32_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_32_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_64_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_64_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_64_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_64_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_64_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_64_vmvm),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_32_vmsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_32_spsp),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_32_vmvm),
#endif

#ifdef ACS_FFA_UNVERIFIED
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_with_address_range),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_retrieve_with_address_range),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_cache_attr),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_cache_attr1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_shareability_attr),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_shareability_attr1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_shareability_attr2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_shareability_attr3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_device_attr),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_device_attr1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_cache_attr),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_cache_attr1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_shareability_attr),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_shareability_attr1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_shareability_attr2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_shareability_attr3),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks10),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks7),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_sepid_multiple_borrowers),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_sepid_multiple_borrowers),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_cache_attr),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_shareability_attr),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_shareability_attr1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_device_attr),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_device_attr1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_sepid),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_sepid),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_sepid),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_retrieve_input_checks),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks5),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_retrieve_align_hint_check),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_retrieve_with_address_range),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks1),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks4),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks8),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks4),
#endif
#endif
#endif

#if (PLATFORM_FFA_V_1_1 == 1 || PLATFORM_FFA_V_ALL == 1)
#if (SUITE == all || SUITE == setup_discovery)
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_features_intr),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_spm_id_get),
#endif

#if (SUITE == all || SUITE == direct_messaging)
#ifdef ACS_FFA_UNVERIFIED
    CLIENT_SERVER_TEST_FN(TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_error2),
#endif
#endif

#if (SUITE == all || SUITE == memory_manage)
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit),
#if (PLATFORM_SP_EL == 0)
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_perm_set),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_perm_get),
#endif
#endif

#if (SUITE == all || SUITE == notifications)
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_bitmap_create),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_bitmap_destroy),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_bind),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_unbind),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_get),
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, notification_set),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_info_get),
#if (PLATFORM_SP_EL == 1)
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, vm_to_sp_notification),
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, vm_to_sp_notification_pcpu),
#endif
#if (PLATFORM_SP_EL == 0)
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, vm_to_sp_notification_el0),
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, vm_to_sp_notification_pcpu_el0),
#endif
#endif

#if (SUITE == all || SUITE == interrupts)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_preempt),
#if (PLATFORM_SP_EL == 1)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_managed_exit),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_managed_exit_1),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_to_sp_blocked),
#endif
#if (PLATFORM_SP_EL == 0)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_el0_blocked),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_el0_running),
#endif
#ifdef ACS_FFA_UNVERIFIED
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_waiting_el0),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_preempted_el0),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_to_sp_waiting),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_waiting),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_managed_exit_2),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_managed_exit_3),
#endif
#endif
#endif

    {0, "", NULL, NULL, NULL, NULL},
};

const uint32_t total_tests = sizeof(test_list)/sizeof(test_list[0]);
