/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_TEST_DISPATCH_H_
#define _VAL_TEST_DISPATCH_H_

#include "val_framework.h"
#include "val_interfaces.h"
#include "val_ffa.h"

void val_run_test_suite(void);
void val_wait_for_test_fn_req(void);
void val_sec_cpu_wait_for_test_fn_req(void);
void val_test_dispatch(void);
uint32_t val_execute_test(
                uint32_t test_num,
                uint32_t client_logical_id,
                uint32_t server_logical_id);
ffa_args_t val_select_server_fn_direct(uint32_t test_run_data,
                                       uint32_t arg4,
                                       uint32_t arg5,
                                       uint32_t arg6,
                                       uint32_t arg7);
ffa_args_t val_resp_client_fn_direct(uint32_t test_run_data,
                                       uint32_t arg3,
                                       uint32_t arg4,
                                       uint32_t arg5,
                                       uint32_t arg6,
                                       uint32_t arg7);
void val_secondary_cpu_test_entry(void);

/**
 * Global datatype for Multi PE test status coherency
 */
typedef struct multi_pe_test_status {
    /** global test num variable */
    uint32_t g_current_test_num;
    /** global pe state variable */
    uint32_t g_other_pe_test_state[PLATFORM_NO_OF_CPUS];
    /** global current test result variable */
    uint32_t g_other_pe_test_result[PLATFORM_NO_OF_CPUS];
} mp_test_status_t;

uint32_t val_get_multi_pe_test_status(uint64_t mpid, uint32_t test_num);

#endif /* _VAL_TEST_DISPATCH_H */
