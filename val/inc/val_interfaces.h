/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_INTERFACES_H_
#define _VAL_INTERFACES_H_

#include "val_misc.h"
#include "val_framework.h"
#include "val_test_dispatch.h"
#include "val_ffa.h"
#include "val_memory.h"
#include "val_endpoint_info.h"
#include "val_ffa_helpers.h"
#include "val_vcpu_setup.h"
#include "val_shemaphore.h"
#include "val_exceptions.h"
#include "val_sysreg.h"
#include "val_irq.h"
#include "val_wd.h"

/* Test entry prototype */
typedef void (*test_entry_fptr_t)(uint32_t test_num);

/* client-server test func prototype */
typedef uint32_t (*client_test_t)(uint32_t test_run_data);
typedef uint32_t (*server_test_t)(ffa_args_t args);

/* Secondary cpu test func prototype */
typedef void (*sec_cpu_test_t)(void);

/* Test suite enum - offset num in structure */
typedef enum {
    TESTSUITE_SETUP_DISCOVERY    = 1,
    TESTSUITE_DIRECT_MESSAGING   = 2,
    TESTSUITE_INDIRECT_MESSAGING = 3,
    TESTSUITE_MEMORY_MANAGE      = 4,
    TESTSUITE_NOTIFICATIONS      = 5,
    TESTSUITE_INTERRUPTS         = 6,
} test_suite_num_t;

/* Structure to hold list of test suite */
typedef struct {
    test_suite_num_t suite_num;
    char             suite_desc[PRINT_LIMIT];
} test_suite_info_t;

/* Structure to hold all test info */
typedef struct {
    test_suite_num_t    suite_num;
    char                test_name[PRINT_LIMIT];
    test_entry_fptr_t   testentry_fn;
    client_test_t       client_fn;
    server_test_t       server_fn;
    sec_cpu_test_t      sec_cpu_fn;
} test_db_t;

/* Test fn types */
typedef enum {
 CLIENT_TEST    = 0,
 SERVER_TEST    = 1,
 SERVER1_TEST   = 2,
 SERVER2_TEST   = 3,
} test_fn_type_t;

extern const test_db_t test_list[];
extern const test_suite_info_t test_suite_list[];
#endif /* _VAL_INTERFACES_H_ */
