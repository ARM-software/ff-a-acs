/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _FFA_MESSAGE_DATA_H_
#define _FFA_MESSAGE_DATA_H_

#include "test_database.h"

static ffa_args_t req_data_32 = {
        .arg3 = 0x1125534411255344,
        .arg4 = 0xFFEEDDCC88776655,
        .arg5 = 0x88776655FFEEDDCC,
        .arg6 = 0xCCBBAA99BBAA9988,
        .arg7 = 0xBBAA9988CCBBAA99,
};

static const ffa_args_t expected_req_data_32 = {
        .arg3 = 0x11255344,
        .arg4 = 0x88776655,
        .arg5 = 0xFFEEDDCC,
        .arg6 = 0xBBAA9988,
        .arg7 = 0xCCBBAA99,
};

static ffa_args_t resp_data_32 = {
        .arg3 = 0x1125534411255344,
        .arg4 = 0x8899665588996655,
        .arg5 = 0xFF44DDCCFF44DDCC,
        .arg6 = 0xBB669988BB669988,
        .arg7 = 0xCC00AA99CC00AA99,
};

static const ffa_args_t expected_resp_data_32 = {
        .arg3 = 0x11255344,
        .arg4 = 0x88996655,
        .arg5 = 0xFF44DDCC,
        .arg6 = 0xBB669988,
        .arg7 = 0xCC00AA99,
};

#endif /* _FFA_MESSAGE_DATA_H_ */
