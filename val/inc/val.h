/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_H_
#define _VAL_H_

#include "pal_interfaces.h"
#include "pal_spm_helpers.h"
#include "val_def.h"
#include "val_log.h"
#include "val_common_framework.h"

#define  VAL_ERROR  1

/** Multi PE Test Synchronization Status Codes */
#define  VAL_MP_STATE_INPROGRESS            105
#define  VAL_MP_STATE_COMPLETE              106
#define  VAL_MP_STATE_WAIT                  107

/* SP3 SERVICE INDEX */
#define NVM_WRITE_SERVICE  0xFFAA
#define NVM_READ_SERVICE   0xFFBB
#define WD_ENABLE_SERVICE  0xFFCC
#define WD_DISABLE_SERVICE 0xFFDD
#define EP_INFO_SYNC_SERVICE 0xFFEE

#define TEST_PASS_WITH_SKIP        0x10

/* Print char limit for val_printf caller */
#define PRINT_LIMIT       100
#define TOTAL_PRINT_LIMIT 120

/* 32 Bit field to capture current test run information
 * Reserved [31:23],
 * test_type[22:19],
 * server_logic_id[19:15],
 * client_logic_id[14:11],
 * test_num[10:0]
*/
#define TEST_RUN_DATA(test_num,                         \
                     client_logic_id,                   \
                     server_logic_id,                   \
                     test_type)                         \
                                                        \
                    ((uint32_t)test_num)       |        \
                    (client_logic_id << 11)    |        \
                    (server_logic_id << 15)    |        \
                    (test_type << 19) | 0

#define GET_TEST_NUM(x)          (x & 0x3ff)
#define GET_CLIENT_LOGIC_ID(x)   ((x >> 11) & 0xf)
#define GET_SERVER_LOGIC_ID(x)   ((x >> 15) & 0xf)
#define GET_TEST_TYPE(x)         ((x >> 19) & 0xf)
#define ADD_TEST_TYPE(y, x)      (uint32_t)((y & ~(0xful << 19)) \
                                  |   ((x & 0xf) << 19))

/* Set server logic_id[19:15] */
#define SET_SERVER_LOGIC_ID(test_run_data, x)  (((uint32_t)(~0 & ~(0x1f << 15)) & test_run_data) \
                                                  | (x << 15))

/* Terminate simulation for unexpected events */
#define VAL_PANIC(x)                               \
   do {                                             \
        LOG(ERROR, x, 0, 0);                        \
        pal_terminate_simulation();                 \
   } while (0);

/* Assert macros */
#define TEST_ASSERT_EQUAL(arg1, arg2)                                       \
    do {                                                                    \
        if ((arg1) != arg2)                                                 \
        {                                                                   \
            LOG(ERROR, "\tActual: %x, Expected: %x\n", arg1, arg2);         \
            return 1;                                                       \
        }                                                                   \
    } while (0);

#endif /* _VAL_H_ */
