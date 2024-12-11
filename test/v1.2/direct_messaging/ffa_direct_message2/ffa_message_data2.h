/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _FFA_MESSAGE_DATA_H_
#define _FFA_MESSAGE_DATA_H_

#include "test_database.h"

static ffa_args_t req_data_64 = {
        .arg4 = 0x1122334411223344,
        .arg5 = 0x8877665588776655,
        .arg6 = 0xFFEEDDCCFFEEDDCC,
        .arg7 = 0xBBAA9988BBAA9988,
        .ext_args = {
            .arg8 = 0xCCBBAA99CCBBAA99,
            .arg9 = 0x1122334411223344,
            .arg10 = 0x8877665588776655,
            .arg11 = 0xFFEEDDCCFFEEDDCC,
            .arg12 = 0xCCBBAA99CCBBAA99,
            .arg13 = 0x1122334411223344,
            .arg14 = 0x8877665588776655,
            .arg15 = 0xFFEEDDCCFFEEDDCC,
            .arg16 = 0xCCBBAA99CCBBAA99,
            .arg17 = 0x1122334411223344
        }
};

static const ffa_args_t expected_req_data_64 = {
        .arg4 = 0x1122334411223344,
        .arg5 = 0x8877665588776655,
        .arg6 = 0xFFEEDDCCFFEEDDCC,
        .arg7 = 0xBBAA9988BBAA9988,
        .ext_args = {
            .arg8 = 0xCCBBAA99CCBBAA99,
            .arg9 = 0x1122334411223344,
            .arg10 = 0x8877665588776655,
            .arg11 = 0xFFEEDDCCFFEEDDCC,
            .arg12 = 0xCCBBAA99CCBBAA99,
            .arg13 = 0x1122334411223344,
            .arg14 = 0x8877665588776655,
            .arg15 = 0xFFEEDDCCFFEEDDCC,
            .arg16 = 0xCCBBAA99CCBBAA99,
            .arg17 = 0x1122334411223344
        }
};

static ffa_args_t resp_data_64 = {
        .arg4 = 0x1125534411255344,
        .arg5 = 0x8899665588996655,
        .arg6 = 0xFF44DDCCFF44DDCC,
        .arg7 = 0xBB669988BB669988,
        .ext_args = {
            .arg8 = 0xCC00AA99CC00AA99,
            .arg9 = 0x1125534411255344,
            .arg10 = 0x8899665588996655,
            .arg11 = 0xFF44DDCCFF44DDCC,
            .arg12 = 0xBB669988BB669988,
            .arg13 = 0xCC00AA99CC00AA99,
            .arg14 = 0x1125534411255344,
            .arg15 = 0x8899665588996655,
            .arg16 = 0xFF44DDCCFF44DDCC,
            .arg17 = 0xBB669988BB669988
        }
};

static const ffa_args_t expected_resp_data_64 = {
        .arg4 = 0x1125534411255344,
        .arg5 = 0x8899665588996655,
        .arg6 = 0xFF44DDCCFF44DDCC,
        .arg7 = 0xBB669988BB669988,
        .ext_args = {
            .arg8 = 0xCC00AA99CC00AA99,
            .arg9 = 0x1125534411255344,
            .arg10 = 0x8899665588996655,
            .arg11 = 0xFF44DDCCFF44DDCC,
            .arg12 = 0xBB669988BB669988,
            .arg13 = 0xCC00AA99CC00AA99,
            .arg14 = 0x1125534411255344,
            .arg15 = 0x8899665588996655,
            .arg16 = 0xFF44DDCCFF44DDCC,
            .arg17 = 0xBB669988BB669988
        }
};
#endif /* _FFA_MESSAGE_DATA_H_ */
