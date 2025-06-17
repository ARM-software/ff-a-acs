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

/* Various test status codes, Max value = 0xff */
#define  VAL_SUCCESS            0
#define  VAL_ERROR              1
#define  VAL_ERROR_POINT(n)     n
#define  VAL_TEST_INIT_FAILED   101
#define  VAL_STATUS_INVALID     102
#define  VAL_SKIP_CHECK         103
#define  VAL_SIM_ERROR          104

/** Multi PE Test Synchronization Status Codes */
#define  VAL_MP_STATE_INPROGRESS            105
#define  VAL_MP_STATE_COMPLETE              106
#define  VAL_MP_STATE_WAIT                  107

#define  VAL_STATUS_ERROR_MAX   255

/* NVM Indext size */
#define VAL_NVM_BLOCK_SIZE         4
#define VAL_NVM_OFFSET(nvm_idx)    (nvm_idx * VAL_NVM_BLOCK_SIZE)

#define VAL_INVALID_TEST_NUM 0xFFFFFFFF

/* Struture to capture test state */
typedef struct {
    uint16_t reserved;
    uint8_t  state;
    uint8_t  status_code;
} test_status_buffer_t;

typedef struct {
    uint32_t total_pass;
    uint32_t total_fail;
    uint32_t total_skip;
    uint32_t total_error;
} regre_report_t;

typedef struct {
    uint32_t suite_num;
    uint32_t test_num;
    uint32_t end_test_num;
    uint32_t test_progress;
} test_info_t;

typedef enum {
    NVM_PLATFORM_RESERVE_INDEX         = 0x0,
    NVM_CUR_SUITE_NUM_INDEX            = 0x1,
    NVM_CUR_TEST_NUM_INDEX             = 0x2,
    NVM_END_TEST_NUM_INDEX             = 0x3,
    NVM_TEST_PROGRESS_INDEX            = 0x4,
    NVM_TOTAL_PASS_INDEX               = 0x5,
    NVM_TOTAL_FAIL_INDEX               = 0x6,
    NVM_TOTAL_SKIP_INDEX               = 0x7,
    NVM_TOTAL_ERROR_INDEX              = 0x8,
} nvm_map_index_t;

/* SP3 SERVICE INDEX */
#define NVM_WRITE_SERVICE  0xFFAA
#define NVM_READ_SERVICE   0xFFBB
#define WD_ENABLE_SERVICE  0xFFCC
#define WD_DISABLE_SERVICE 0xFFDD
#define EP_INFO_SYNC_SERVICE 0xFFEE

/* Test state macros */
#define TEST_START                 0x01
#define TEST_PASS                  0x02
#define TEST_FAIL                  0x03
#define TEST_SKIP                  0x04
#define TEST_ERROR                 0x05
#define TEST_END                   0x06
#define TEST_REBOOTING             0x07
#define TEST_PASS_WITH_SKIP        0x10

#define TEST_STATE_SHIFT           8
#define TEST_STATUS_CODE_SHIFT     0

#define TEST_STATE_MASK            0xFF
#define TEST_STATUS_CODE_MASK      0xFF

#define RESULT_PASS(status)     (((TEST_PASS) << TEST_STATE_SHIFT) |\
                                    ((status) << TEST_STATUS_CODE_SHIFT))
#define RESULT_FAIL(status)     (((TEST_FAIL) << TEST_STATE_SHIFT) |\
                                    ((status) << TEST_STATUS_CODE_SHIFT))
#define RESULT_SKIP(status)     (((TEST_SKIP) << TEST_STATE_SHIFT) |\
                                    ((status) << TEST_STATUS_CODE_SHIFT))
#define RESULT_ERROR(status)     (((TEST_ERROR) << TEST_STATE_SHIFT) |\
                                    ((status) << TEST_STATUS_CODE_SHIFT))

#define IS_TEST_FAIL(status)    (((status >> TEST_STATE_SHIFT) &\
                                    TEST_STATE_MASK) == TEST_FAIL)
#define IS_STATUS_FAIL(status)  ((status & TEST_STATUS_CODE_MASK) ? 1 : 0)

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

#define VAL_BIT_MASK(len) ((1 << len) - 1)
/* Set the value in given position */
#define VAL_SET_BITS(data, pos, len, val) (((uint32_t)(~(uint32_t)0 & ~(uint32_t) \
                    (VAL_BIT_MASK(len) << pos)) & data) | (val << pos))

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
