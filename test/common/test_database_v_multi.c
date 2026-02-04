/*
 * Copyright (c) 2024-2026, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

/*
 * Multi-Version Test Database Summary
 *
 * This test database includes only those test cases that are common and
 * compatible between FF-A v1.1 and v1.2. Tests that are exclusive to v1.0
 * are intentionally omitted.
 *
 * The purpose of this database is to verify backward compatibility between
 * v1.1 and v1.2. Therefore, tests marked as ACS UNVERIFIED are excluded to
 * maintain reliability and consistency.
 */

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

#if (PLATFORM_FFA_V_MULTI == 1)
#if (SUITE == all || SUITE == setup_discovery)
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_version, FFA_VERSION_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_id_get, FFA_ID_GET_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_SETUP_DISCOVERY, ffa_partition_info_get, FFA_PARTITION_INFO_GET_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rx_release, FFA_RX_RELEASE_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_rxtx_map_and_unmap, FFA_RXTX_MAP_64),
    CLIENT_SERVER_TEST_FN(TESTSUITE_SETUP_DISCOVERY, rxtx_exclusive_access, FFA_RXTX_MAP_64),
#endif

#if (SUITE == all || SUITE == direct_messaging)i
     CLIENT_SERVER_TEST_FN(
        TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_32, FFA_MSG_SEND_DIRECT_REQ_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_64, FFA_MSG_SEND_DIRECT_REQ_64),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_error, FFA_MSG_SEND_DIRECT_REQ_ANY),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_DIRECT_MESSAGING, ffa_direct_message_error1, FFA_MSG_SEND_DIRECT_REQ_ANY),
#endif

#if (SUITE == all || SUITE == memory_manage)
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_share, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_invalid_handle_tag, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks, FFA_MEM_SHARE_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_state_machine_2, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_3, FFA_MEM_SHARE_ANY),

    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_5, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_rw_retrieve_ro, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_retrieve_align_hint_check, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_relinquish_input_checks, FFA_MEM_RELINQUISH_32),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_lend, FFA_MEM_LEND_ANY),

    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_invalid_handle_tag, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks, FFA_MEM_LEND_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks2, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_align_hint_check, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks1, FFA_MEM_RETRIEVE_REQ_ANY),

    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks3, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_ro_retrieve_rw, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_rw_retrieve_ro, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, ffa_mem_donate, FFA_MEM_DONATE_ANY),

    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks3, FFA_MEM_DONATE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_invalid_handle_tag, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks2, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, relinquish_state_machine_2, FFA_MEM_RELINQUISH_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, relinquish_state_machine_5, FFA_MEM_RELINQUISH_32),

    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, reclaim_input_error_checks, FFA_MEM_RECLAIM_32),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, reclaim_zero_flag, FFA_MEM_RECLAIM_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks4, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks6, FFA_MEM_DONATE_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks3, FFA_MEM_LEND_ANY),

    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks6, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_2, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_3, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_5, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_1, FFA_MEM_DONATE_ANY),

    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_2, FFA_MEM_DONATE_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_4, FFA_MEM_DONATE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks3, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_5, FFA_MEM_DONATE_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, donate_state_machine_3, FFA_MEM_DONATE_ANY),

    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_6, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_4, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_state_machine_1, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, relinquish_state_machine_3, FFA_MEM_RELINQUISH_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, relinquish_state_machine_4, FFA_MEM_RELINQUISH_32),

    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks9, FFA_MEM_LEND_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks2, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_6, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_4, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, share_state_machine_1, FFA_MEM_SHARE_ANY),

    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks1, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_multiple_retrievals, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks5, FFA_MEM_LEND_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks, FFA_MEM_DONATE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks4, FFA_MEM_RETRIEVE_REQ_ANY),

    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks2, FFA_MEM_DONATE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_multiple_retrievals, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_retrieve_input_checks1, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks1, FFA_MEM_LEND_ANY),

    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, share_input_error_checks2, FFA_MEM_SHARE_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, donate_input_error_checks4, FFA_MEM_DONATE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_input_checks8, FFA_MEM_RETRIEVE_REQ_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, lend_input_error_checks4, FFA_MEM_LEND_ANY),

#if (PLATFORM_SP_EL == 1)
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_lower_upper_boundary_32_spsp, FFA_MEM_DONATE_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_lower_upper_boundary_32_vmvm, FFA_MEM_DONATE_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_lower_upper_boundary_64_spsp, FFA_MEM_DONATE_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, donate_lower_upper_boundary_64_vmvm, FFA_MEM_DONATE_64),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, donate_mem_access_after_donate_32_vm, FFA_MEM_DONATE_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, donate_mem_access_after_donate_32_sp, FFA_MEM_DONATE_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, donate_mem_access_after_donate_64_vm, FFA_MEM_DONATE_64),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, donate_mem_access_after_donate_64_sp, FFA_MEM_DONATE_64),

    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_32_vmsp, FFA_MEM_LEND_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_32_spsp, FFA_MEM_LEND_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_32_vmvm, FFA_MEM_LEND_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_64_vmsp, FFA_MEM_LEND_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_64_spsp, FFA_MEM_LEND_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_lower_upper_boundary_64_vmvm, FFA_MEM_LEND_64),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, lend_mem_access_after_lend_32_vm, FFA_MEM_LEND_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, lend_mem_access_after_lend_32_sp, FFA_MEM_LEND_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, lend_mem_access_after_lend_64_vm, FFA_MEM_LEND_64),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, lend_mem_access_after_lend_64_sp, FFA_MEM_LEND_64),

    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_32_vmvm, FFA_MEM_LEND_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_32_vmsp, FFA_MEM_LEND_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_32_spsp, FFA_MEM_LEND_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_64_vmvm, FFA_MEM_LEND_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_64_vmsp, FFA_MEM_LEND_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, lend_retrieve_mem_access_64_spsp, FFA_MEM_LEND_64),

    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, relinquish_mem_unmap_check_vmsp, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, relinquish_mem_unmap_check_vmvm, FFA_MEM_SHARE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, relinquish_mem_unmap_check_spsp, FFA_MEM_SHARE_ANY),

    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_32_vmsp, FFA_MEM_SHARE_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_32_spsp, FFA_MEM_SHARE_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_32_vmvm, FFA_MEM_SHARE_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_64_vmsp, FFA_MEM_SHARE_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_64_spsp, FFA_MEM_SHARE_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_lower_upper_boundary_64_vmvm, FFA_MEM_SHARE_64),

    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_64_vmsp, FFA_MEM_RETRIEVE_REQ_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_64_spsp, FFA_MEM_RETRIEVE_REQ_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_64_vmvm, FFA_MEM_RETRIEVE_REQ_64),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_32_vmsp, FFA_MEM_RETRIEVE_REQ_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_32_spsp, FFA_MEM_RETRIEVE_REQ_32),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_ro_retrieve_rw_32_vmvm, FFA_MEM_RETRIEVE_REQ_32),

    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, mem_lend_mmio, FFA_MEM_LEND_ANY),
#endif
#endif

#if (SUITE == all || SUITE == setup_discovery)
#ifndef TARGET_LINUX
    CLIENT_SEC_CPU_CLIENT_SERVER_TEST_FN(
        TESTSUITE_SETUP_DISCOVERY, mp_execution_contexts, FFA_MSG_SEND_DIRECT_REQ_64),
    CLIENT_SERVER_SEC_CPU_CLIENT_TEST_FN(
        TESTSUITE_SETUP_DISCOVERY, up_migrate_capable, FFA_MSG_SEND_DIRECT_REQ_64),

#endif
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_features_intr, FFA_FEATURES_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_features_nsbit, FFA_FEATURES_32),
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_spm_id_get, FFA_SPM_ID_GET_32),
#endif

#if (SUITE == all || SUITE == direct_messaging)
    CLIENT_SERVER_TEST_FN(TESTSUITE_DIRECT_MESSAGING, direct_msg_sp_to_vm, FFA_MSG_SEND2_32),
#endif

#if (SUITE == all || SUITE == indirect_messaging)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INDIRECT_MESSAGING, ffa_msg_send2, FFA_MSG_SEND2_32),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INDIRECT_MESSAGING, ffa_msg_send2_sp, FFA_MSG_SEND2_32),
#endif

#if (SUITE == all || SUITE == memory_manage)
    CLIENT_SERVER_TEST_FN(
       TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit_retrieve_check, FFA_MEM_RETRIEVE_REQ_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit_share_invalid, FFA_MEM_SHARE_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit_lend_invalid, FFA_MEM_LEND_32),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_MEMORY_MANAGE, mem_security_state_ns_bit_donate_invalid, FFA_MEM_DONATE_32),
#if (PLATFORM_SP_EL == 0)
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_perm_set, FFA_FID_SKIP_CHECK),
    CLIENT_TEST_FN_ONLY(TESTSUITE_MEMORY_MANAGE, ffa_mem_perm_get, FFA_FID_SKIP_CHECK),
#endif
#endif

#if (SUITE == all || SUITE == notifications)
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_NOTIFICATIONS, notification_bitmap_create, FFA_NOTIFICATION_BITMAP_CREATE),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_NOTIFICATIONS, notification_bitmap_destroy, FFA_NOTIFICATION_BITMAP_DESTROY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_bind, FFA_NOTIFICATION_BIND),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_unbind, FFA_NOTIFICATION_UNBIND),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_get, FFA_NOTIFICATION_GET),
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, notification_set, FFA_NOTIFICATION_SET),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_NOTIFICATIONS, notification_info_get, FFA_NOTIFICATION_INFO_GET_ANY),
    CLIENT_TEST_FN_ONLY(TESTSUITE_NOTIFICATIONS, notification_comp, FFA_FEATURES_32),
#if (PLATFORM_SP_EL == 1)
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, vm_to_sp_notification, FFA_NOTIFICATION_SET),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_NOTIFICATIONS, vm_to_sp_notification_pcpu, FFA_NOTIFICATION_SET),
    CLIENT_SERVER_TEST_FN(TESTSUITE_NOTIFICATIONS, sp_signals_vm_sp, FFA_NOTIFICATION_SET),
#endif
#if (PLATFORM_SP_EL == 0)
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_NOTIFICATIONS, vm_to_sp_notification_el0, FFA_NOTIFICATION_SET),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_NOTIFICATIONS, vm_to_sp_notification_pcpu_el0, FFA_NOTIFICATION_SET),
#endif
#endif

#ifndef TARGET_LINUX
#if (SUITE == all || SUITE == interrupts)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_preempt, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, ns_intr_queued, FFA_FID_SKIP_CHECK),
#if (PLATFORM_SP_EL == 1)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, s_int_sp_preempt, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, other_secure_int1, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, other_secure_int6, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, ns_int_precedence, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_el1_running, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_waiting_el1, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_managed_exit, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_managed_exit_1, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_managed_exit_2, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, vm_to_sp_managed_exit_3, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_to_sp_blocked, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_preempted_el1, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, s_int_ec_blocked, FFA_FID_SKIP_CHECK),
#endif
#if (PLATFORM_SP_EL == 0)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_el0_blocked, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_el0_running, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_waiting_el0, FFA_FID_SKIP_CHECK),
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_preempted_el0, FFA_FID_SKIP_CHECK),
#endif
#endif
#endif

/* The below test is only in version 1.2 FFA-ACS */

#if (SUITE == all || SUITE == setup_discovery)
    CLIENT_TEST_FN_ONLY(TESTSUITE_SETUP_DISCOVERY, ffa_version_negotiation, FFA_VERSION_32),
    CLIENT_SERVER_TEST_FN(TESTSUITE_SETUP_DISCOVERY, ffa_msg_wait_rx_buff_rel, FFA_FID_SKIP_CHECK),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_SETUP_DISCOVERY, ffa_partition_info_get_regs, FFA_PARTITION_INFO_GET_REGS_64),
    CLIENT_TEST_FN_ONLY(
        TESTSUITE_SETUP_DISCOVERY, ffa_partition_info_get_lsp, FFA_PARTITION_INFO_GET_REGS_64),
#endif

#if (SUITE == all || SUITE == memory_manage)
//    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, mem_share_impdef, FFA_MEM_SHARE_ANY),
//    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, mem_lend_impdef, FFA_MEM_LEND_ANY),
//    CLIENT_SERVER_TEST_FN(TESTSUITE_MEMORY_MANAGE, mem_donate_impdef, FFA_MEM_DONATE_ANY),
    CLIENT_SERVER_TEST_FN(
        TESTSUITE_MEMORY_MANAGE, share_multi_borrower_flag_check, FFA_MEM_SHARE_ANY),
#endif



#if (SUITE == all || SUITE == interrupts)
#if (PLATFORM_SP_EL == 1)
    CLIENT_SERVER_TEST_FN(TESTSUITE_INTERRUPTS, sp_el1_int_mask, FFA_FID_SKIP_CHECK),
#endif
#endif

#if (SUITE == all || SUITE == direct_messaging)
//    CLIENT_SERVER_TEST_FN(
//        TESTSUITE_DIRECT_MESSAGING, ffa_direct_message2, FFA_MSG_SEND_DIRECT_REQ2_64),
#endif
#endif

    {0, "", NULL, NULL, NULL, NULL, NULL},
};

const uint32_t total_tests = sizeof(test_list)/sizeof(test_list[0]);
