/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
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
DECLARE_TEST_FN(ffa_mem_share_handle);
DECLARE_TEST_FN(ffa_mem_share_tag);
DECLARE_TEST_FN(ffa_mem_share_mmio);
DECLARE_TEST_FN(ffa_mem_lend_mmio);
DECLARE_TEST_FN(ffa_mem_donate_mmio);
DECLARE_TEST_FN(ffa_mem_share_64_read_only_chk_vmsp);
DECLARE_TEST_FN(ffa_mem_share_64_read_only_chk_spsp);
DECLARE_TEST_FN(ffa_mem_share_64_read_only_chk_vmvm);
DECLARE_TEST_FN(ffa_mem_share_32_read_only_chk_vmsp);
DECLARE_TEST_FN(ffa_mem_share_32_read_only_chk_spsp);
DECLARE_TEST_FN(ffa_mem_share_32_read_only_chk_vmvm);
DECLARE_TEST_FN(ffa_mem_share_retrieve);
#endif
#endif /* _TEST_DATABASE_H_ */
