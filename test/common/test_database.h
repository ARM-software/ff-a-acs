/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
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

#define DECLARE_TEST_FN(testname) \
    extern void testname##_testentry(uint32_t test_num);\
    extern uint32_t testname##_client(uint32_t test_run_data);\
    extern uint32_t testname##_server(ffa_args_t args);\
    extern void testname##_sec_cpu(void);

#define CLIENT_TEST_FN_ONLY(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, NULL, NULL}

#define CLIENT_SERVER_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, testname##_server, NULL}

#define CLIENT_SEC_CPU_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, NULL, testname##_sec_cpu}

#define CLIENT_SERVER_SEC_CPU_TEST_FN(suite_num, testname) \
    {suite_num, " "#testname, testname##_testentry, \
        testname##_client, testname##_server, testname##_sec_cpu}

#if (SUITE == all || SUITE == setup_discovery)
DECLARE_TEST_FN(ffa_version);
DECLARE_TEST_FN(ffa_features);
DECLARE_TEST_FN(ffa_id_get);
DECLARE_TEST_FN(ffa_partition_info_get);
DECLARE_TEST_FN(ffa_rx_release);
DECLARE_TEST_FN(ffa_rxtx_map_and_unmap);
DECLARE_TEST_FN(rxtx_exclusive_access);
DECLARE_TEST_FN(mp_execution_contexts);
DECLARE_TEST_FN(up_migrate_capable);
#endif

#if (SUITE == all || SUITE == direct_messaging)
DECLARE_TEST_FN(ffa_direct_message_32);
DECLARE_TEST_FN(ffa_direct_message_64);
DECLARE_TEST_FN(ffa_direct_message_error);
DECLARE_TEST_FN(ffa_direct_message_error1);
#endif

#if (SUITE == all || SUITE == indirect_messaging)
DECLARE_TEST_FN(ffa_msg_send);
DECLARE_TEST_FN(ffa_msg_send_error);
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
DECLARE_TEST_FN(ffa_mem_lend);
DECLARE_TEST_FN(lend_invalid_handle_tag);
DECLARE_TEST_FN(lend_input_error_checks);
DECLARE_TEST_FN(lend_input_error_checks1);
DECLARE_TEST_FN(lend_retrieve_with_address_range);
DECLARE_TEST_FN(lend_retrieve_align_hint_check);
DECLARE_TEST_FN(lend_retrieve_input_checks);
DECLARE_TEST_FN(lend_retrieve_input_checks1);
DECLARE_TEST_FN(lend_retrieve_input_checks2);
DECLARE_TEST_FN(lend_retrieve_input_checks3);
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
DECLARE_TEST_FN(donate_input_error_checks);
DECLARE_TEST_FN(donate_invalid_handle_tag);
DECLARE_TEST_FN(donate_lower_upper_boundary_32_spsp);
DECLARE_TEST_FN(donate_lower_upper_boundary_32_vmvm);
DECLARE_TEST_FN(donate_lower_upper_boundary_64_spsp);
DECLARE_TEST_FN(donate_lower_upper_boundary_64_vmvm);
DECLARE_TEST_FN(donate_retrieve_input_checks1);
DECLARE_TEST_FN(donate_retrieve_input_checks2);
DECLARE_TEST_FN(relinquish_state_machine_2);
DECLARE_TEST_FN(relinquish_state_machine_3);
DECLARE_TEST_FN(relinquish_state_machine_4);
DECLARE_TEST_FN(relinquish_state_machine_5);
DECLARE_TEST_FN(relinquish_mem_unmap_check_vmsp);
DECLARE_TEST_FN(relinquish_mem_unmap_check_vmvm);
DECLARE_TEST_FN(relinquish_mem_unmap_check_spsp);
DECLARE_TEST_FN(reclaim_input_error_checks);
DECLARE_TEST_FN(reclaim_zero_flag);
#endif
#endif /* _TEST_DATABASE_H_ */
