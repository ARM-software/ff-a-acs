/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _TEST_DATABASE_H_
#define _TEST_DATABASE_H_

#include "val_interfaces.h"

#define all                0
#define setup_discovery    1
#define direct_messaging   2
#define indirect_messaging 3
#define memory_manage      4
#define notifications      5
#define interrupts         6

#define DECLARE_TEST_FN(testname) \
    extern void testname##_testentry(uint32_t test_num);\
    extern uint32_t testname##_client(uint32_t test_run_data);\
    extern uint32_t testname##_server(ffa_args_t args);\
    extern uint32_t testname##_sec_cpu_client(uint32_t test_num);\
    extern uint32_t testname##_sec_cpu_server(ffa_args_t args);

#define CLIENT_TEST_FN_ONLY(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, NULL, NULL, NULL}

#define CLIENT_SERVER_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, testname##_server, NULL, NULL}

#define CLIENT_SEC_CPU_CLIENT_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, NULL, testname##_sec_cpu_client, \
        NULL}

#define CLIENT_SEC_CPU_SERVER_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, NULL, NULL, \
        testname##_sec_cpu_server}

#define CLIENT_SEC_CPU_CLIENT_SERVER_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, NULL, testname##_sec_cpu_client, \
        testname##_sec_cpu_server}

#define CLIENT_SERVER_SEC_CPU_CLIENT_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, testname##_server, testname##_sec_cpu_client, \
        NULL}

#define CLIENT_SERVER_SEC_CPU_SERVER_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, testname##_server, NULL, \
        testname##_sec_cpu_server}

#define CLIENT_SERVER_SEC_CPU_CLIENT_SERVER_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, testname##_server, testname##_sec_cpu_client, \
        testname##_sec_cpu_server}

#if (SUITE == all || SUITE == setup_discovery)
DECLARE_TEST_FN(ffa_version);
DECLARE_TEST_FN(ffa_features);
DECLARE_TEST_FN(ffa_features_intr);
DECLARE_TEST_FN(ffa_features_nsbit);
DECLARE_TEST_FN(ffa_id_get);
DECLARE_TEST_FN(ffa_spm_id_get);
DECLARE_TEST_FN(ffa_partition_info_get);
DECLARE_TEST_FN(ffa_rx_release);
DECLARE_TEST_FN(ffa_rxtx_map_and_unmap);
DECLARE_TEST_FN(rxtx_exclusive_access);
DECLARE_TEST_FN(mp_execution_contexts);
DECLARE_TEST_FN(up_migrate_capable);
DECLARE_TEST_FN(ffa_version_negotiation);
DECLARE_TEST_FN(ffa_msg_wait_rx_buff_rel);
DECLARE_TEST_FN(ffa_console_log);
DECLARE_TEST_FN(ffa_partition_info_get_regs);
DECLARE_TEST_FN(ffa_partition_info_get_lsp);
#endif

#if (SUITE == all || SUITE == direct_messaging)
DECLARE_TEST_FN(ffa_direct_message_32);
DECLARE_TEST_FN(ffa_direct_message_64);
DECLARE_TEST_FN(ffa_direct_message_error);
DECLARE_TEST_FN(ffa_direct_message_error1);
DECLARE_TEST_FN(ffa_direct_message_error2);
DECLARE_TEST_FN(direct_msg_sp_to_vm);
DECLARE_TEST_FN(ffa_run_direct_req);
DECLARE_TEST_FN(ffa_direct_message2);
#endif

#if (SUITE == all || SUITE == indirect_messaging)
DECLARE_TEST_FN(ffa_msg_send);
DECLARE_TEST_FN(ffa_msg_send_error);
DECLARE_TEST_FN(ffa_run);
DECLARE_TEST_FN(ffa_msg_send2);
DECLARE_TEST_FN(ffa_msg_send2_sp);
DECLARE_TEST_FN(ffa_msg_send2_uuid_check);
#endif

#if (SUITE == all || SUITE == memory_manage)
DECLARE_TEST_FN(ffa_mem_share);
DECLARE_TEST_FN(share_invalid_handle_tag);
DECLARE_TEST_FN(share_cache_attr);
DECLARE_TEST_FN(share_shareability_attr);
DECLARE_TEST_FN(share_shareability_attr1);
DECLARE_TEST_FN(share_device_attr);
DECLARE_TEST_FN(share_device_attr1);
DECLARE_TEST_FN(share_state_machine_1);
DECLARE_TEST_FN(share_state_machine_2);
DECLARE_TEST_FN(share_state_machine_3);
DECLARE_TEST_FN(share_state_machine_4);
DECLARE_TEST_FN(share_state_machine_5);
DECLARE_TEST_FN(share_state_machine_6);
DECLARE_TEST_FN(share_input_error_checks);
DECLARE_TEST_FN(share_input_error_checks1);
DECLARE_TEST_FN(share_input_error_checks2);
DECLARE_TEST_FN(share_rw_retrieve_ro);
DECLARE_TEST_FN(share_ro_retrieve_rw_64_vmsp);
DECLARE_TEST_FN(share_ro_retrieve_rw_64_spsp);
DECLARE_TEST_FN(share_ro_retrieve_rw_64_vmvm);
DECLARE_TEST_FN(share_ro_retrieve_rw_32_vmsp);
DECLARE_TEST_FN(share_ro_retrieve_rw_32_spsp);
DECLARE_TEST_FN(share_ro_retrieve_rw_32_vmvm);
DECLARE_TEST_FN(share_lower_upper_boundary_32_vmsp);
DECLARE_TEST_FN(share_lower_upper_boundary_32_spsp);
DECLARE_TEST_FN(share_lower_upper_boundary_32_vmvm);
DECLARE_TEST_FN(share_lower_upper_boundary_64_vmsp);
DECLARE_TEST_FN(share_lower_upper_boundary_64_spsp);
DECLARE_TEST_FN(share_lower_upper_boundary_64_vmvm);
DECLARE_TEST_FN(share_retrieve_with_address_range);
DECLARE_TEST_FN(share_retrieve_input_checks);
DECLARE_TEST_FN(share_retrieve_align_hint_check);
DECLARE_TEST_FN(share_relinquish_input_checks);
DECLARE_TEST_FN(share_multiple_retrievals);
DECLARE_TEST_FN(share_sepid);
DECLARE_TEST_FN(share_sepid_multiple_borrowers);
DECLARE_TEST_FN(ffa_mem_lend);
DECLARE_TEST_FN(lend_invalid_handle_tag);
DECLARE_TEST_FN(lend_input_error_checks);
DECLARE_TEST_FN(lend_input_error_checks1);
DECLARE_TEST_FN(lend_input_error_checks2);
DECLARE_TEST_FN(lend_input_error_checks3);
DECLARE_TEST_FN(lend_input_error_checks4);
DECLARE_TEST_FN(lend_retrieve_with_address_range);
DECLARE_TEST_FN(lend_retrieve_align_hint_check);
DECLARE_TEST_FN(lend_retrieve_input_checks);
DECLARE_TEST_FN(lend_retrieve_input_checks1);
DECLARE_TEST_FN(lend_retrieve_input_checks2);
DECLARE_TEST_FN(lend_retrieve_input_checks3);
DECLARE_TEST_FN(lend_retrieve_input_checks4);
DECLARE_TEST_FN(lend_retrieve_input_checks5);
DECLARE_TEST_FN(lend_retrieve_input_checks6);
DECLARE_TEST_FN(lend_retrieve_input_checks7);
DECLARE_TEST_FN(lend_retrieve_input_checks8);
DECLARE_TEST_FN(lend_retrieve_input_checks9);
DECLARE_TEST_FN(lend_retrieve_input_checks10);
DECLARE_TEST_FN(lend_mem_access_after_lend_32_vm);
DECLARE_TEST_FN(lend_mem_access_after_lend_32_sp);
DECLARE_TEST_FN(lend_mem_access_after_lend_64_vm);
DECLARE_TEST_FN(lend_mem_access_after_lend_64_sp);
DECLARE_TEST_FN(lend_retrieve_mem_access_32_vmvm);
DECLARE_TEST_FN(lend_retrieve_mem_access_32_vmsp);
DECLARE_TEST_FN(lend_retrieve_mem_access_32_spsp);
DECLARE_TEST_FN(lend_retrieve_mem_access_64_vmvm);
DECLARE_TEST_FN(lend_retrieve_mem_access_64_vmsp);
DECLARE_TEST_FN(lend_retrieve_mem_access_64_spsp);
DECLARE_TEST_FN(lend_cache_attr);
DECLARE_TEST_FN(lend_cache_attr1);
DECLARE_TEST_FN(lend_shareability_attr);
DECLARE_TEST_FN(lend_shareability_attr1);
DECLARE_TEST_FN(lend_shareability_attr2);
DECLARE_TEST_FN(lend_shareability_attr3);
DECLARE_TEST_FN(lend_lower_upper_boundary_32_vmsp);
DECLARE_TEST_FN(lend_lower_upper_boundary_32_spsp);
DECLARE_TEST_FN(lend_lower_upper_boundary_32_vmvm);
DECLARE_TEST_FN(lend_lower_upper_boundary_64_vmsp);
DECLARE_TEST_FN(lend_lower_upper_boundary_64_spsp);
DECLARE_TEST_FN(lend_lower_upper_boundary_64_vmvm);
DECLARE_TEST_FN(lend_device_attr);
DECLARE_TEST_FN(lend_device_attr1);
DECLARE_TEST_FN(lend_ro_retrieve_rw);
DECLARE_TEST_FN(lend_rw_retrieve_ro);
DECLARE_TEST_FN(lend_state_machine_1);
DECLARE_TEST_FN(lend_state_machine_2);
DECLARE_TEST_FN(lend_state_machine_3);
DECLARE_TEST_FN(lend_state_machine_4);
DECLARE_TEST_FN(lend_state_machine_5);
DECLARE_TEST_FN(lend_state_machine_6);
DECLARE_TEST_FN(lend_multiple_retrievals);
DECLARE_TEST_FN(lend_sepid);
DECLARE_TEST_FN(ffa_mem_donate);
DECLARE_TEST_FN(donate_input_error_checks);
DECLARE_TEST_FN(donate_input_error_checks1);
DECLARE_TEST_FN(donate_input_error_checks2);
DECLARE_TEST_FN(donate_input_error_checks3);
DECLARE_TEST_FN(donate_input_error_checks4);
DECLARE_TEST_FN(donate_invalid_handle_tag);
DECLARE_TEST_FN(donate_lower_upper_boundary_32_spsp);
DECLARE_TEST_FN(donate_lower_upper_boundary_32_vmvm);
DECLARE_TEST_FN(donate_lower_upper_boundary_64_spsp);
DECLARE_TEST_FN(donate_lower_upper_boundary_64_vmvm);
DECLARE_TEST_FN(donate_retrieve_input_checks1);
DECLARE_TEST_FN(donate_retrieve_input_checks2);
DECLARE_TEST_FN(donate_retrieve_input_checks3);
DECLARE_TEST_FN(donate_retrieve_input_checks4);
DECLARE_TEST_FN(donate_retrieve_input_checks5);
DECLARE_TEST_FN(donate_retrieve_input_checks6);
DECLARE_TEST_FN(donate_state_machine_1);
DECLARE_TEST_FN(donate_state_machine_2);
DECLARE_TEST_FN(donate_state_machine_3);
DECLARE_TEST_FN(donate_state_machine_4);
DECLARE_TEST_FN(donate_state_machine_5);
DECLARE_TEST_FN(donate_mem_access_after_donate_32_vm);
DECLARE_TEST_FN(donate_mem_access_after_donate_32_sp);
DECLARE_TEST_FN(donate_mem_access_after_donate_64_vm);
DECLARE_TEST_FN(donate_mem_access_after_donate_64_sp);
DECLARE_TEST_FN(donate_retrieve_align_hint_check);
DECLARE_TEST_FN(donate_retrieve_with_address_range);
DECLARE_TEST_FN(donate_sepid);
DECLARE_TEST_FN(relinquish_state_machine_2);
DECLARE_TEST_FN(relinquish_state_machine_3);
DECLARE_TEST_FN(relinquish_state_machine_4);
DECLARE_TEST_FN(relinquish_state_machine_5);
DECLARE_TEST_FN(relinquish_mem_unmap_check_vmsp);
DECLARE_TEST_FN(relinquish_mem_unmap_check_vmvm);
DECLARE_TEST_FN(relinquish_mem_unmap_check_spsp);
DECLARE_TEST_FN(reclaim_input_error_checks);
DECLARE_TEST_FN(reclaim_zero_flag);
DECLARE_TEST_FN(mem_security_state_ns_bit);
DECLARE_TEST_FN(ffa_mem_perm_set);
DECLARE_TEST_FN(ffa_mem_perm_get);
DECLARE_TEST_FN(static_mapping_dma);
DECLARE_TEST_FN(mem_share_mmio);
DECLARE_TEST_FN(mem_lend_mmio);
DECLARE_TEST_FN(mem_donate_mmio);
DECLARE_TEST_FN(mem_share_impdef);
DECLARE_TEST_FN(mem_lend_impdef);
DECLARE_TEST_FN(mem_donate_impdef);
DECLARE_TEST_FN(share_multi_borrower_flag_check);
DECLARE_TEST_FN(lend_multi_borrower_flag_check);
#endif

#if (SUITE == all || SUITE == notifications)
DECLARE_TEST_FN(vm_to_sp_notification);
DECLARE_TEST_FN(vm_to_sp_notification_el0);
DECLARE_TEST_FN(vm_to_sp_notification_pcpu);
DECLARE_TEST_FN(vm_to_sp_notification_pcpu_el0);
DECLARE_TEST_FN(notification_bitmap_create);
DECLARE_TEST_FN(notification_bitmap_destroy);
DECLARE_TEST_FN(notification_bind);
DECLARE_TEST_FN(notification_unbind);
DECLARE_TEST_FN(notification_get);
DECLARE_TEST_FN(notification_set);
DECLARE_TEST_FN(notification_info_get);
DECLARE_TEST_FN(sp_signals_vm_sp);
DECLARE_TEST_FN(notification_comp);
DECLARE_TEST_FN(sp_signals_vm);
#endif

#if (SUITE == all || SUITE == interrupts)
DECLARE_TEST_FN(vm_to_sp_preempt);
DECLARE_TEST_FN(vm_to_sp_managed_exit);
DECLARE_TEST_FN(vm_to_sp_managed_exit_1);
DECLARE_TEST_FN(vm_to_sp_managed_exit_2);
DECLARE_TEST_FN(vm_to_sp_managed_exit_3);
DECLARE_TEST_FN(vm_to_sp_managed_exit_4);
DECLARE_TEST_FN(sp_to_sp_blocked);
DECLARE_TEST_FN(sp_waiting_el1);
DECLARE_TEST_FN(s_int_ec_blocked);
DECLARE_TEST_FN(sp_waiting_el0);
DECLARE_TEST_FN(ns_intr_queued);
DECLARE_TEST_FN(sp_el0_blocked);
DECLARE_TEST_FN(sp_el0_running);
DECLARE_TEST_FN(sp_preempted_el0);
DECLARE_TEST_FN(sp_preempted_el1);
DECLARE_TEST_FN(sp_el1_running);
DECLARE_TEST_FN(ns_int_precedence);
DECLARE_TEST_FN(other_secure_int1);
DECLARE_TEST_FN(s_int_sp_preempt);
DECLARE_TEST_FN(vm_to_up_sp_preempt);
DECLARE_TEST_FN(other_secure_int2);
DECLARE_TEST_FN(other_secure_int6);
DECLARE_TEST_FN(other_secure_int7);
DECLARE_TEST_FN(sp_el1_int_mask);
DECLARE_TEST_FN(sp_yield_spmc_mode);
#endif
#endif /* _TEST_DATABASE_H_ */
