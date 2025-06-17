/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pal.h>
#include <pal_interfaces.h>
#include <pal_arch_helpers.h>
#include <pal_ffa_console_log.h>
#include <pal_spm_helpers.h>
#include <stdbool.h>
#include <stdint.h>

#define PAL_FFA_CONSOLE_LOG_32          0x8400008AU
#define PAL_FFA_CONSOLE_LOG_SUCCESS_32  0x84000061
#define PAL_FFA_CONSOLE_LOG_CHAR_COUNT  0x1U

/**
 * Uses FFA_CONSOLE_LOG Interface to pipe character to SPMC.
 */

void driver_ffa_console_log_putc(char x)
{
    smc_args args;
    smc_ret_values retargs;
    args.fid =  (uint32_t) PAL_FFA_CONSOLE_LOG_32;
    args.arg1 = (u_register_t) PAL_FFA_CONSOLE_LOG_CHAR_COUNT;
    args.arg2 = (u_register_t) x;
    args.arg3 = (u_register_t) 0;
    args.arg4 = (u_register_t) 0;
    args.arg5 = (u_register_t) 0;
    args.arg6 = (u_register_t) 0;
    args.arg7 = (u_register_t) 0;

    retargs = pal_smc(&args);
    assert(retargs.ret0 == (u_register_t)PAL_FFA_CONSOLE_LOG_SUCCESS_32);
    return;
}