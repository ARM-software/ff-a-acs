/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SPMC_H
#define SPMC_H

#include <pal_interfaces.h>
#include <pal_arch_helpers.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef TARGET_LINUX
/*
 * SMC calls take a function identifier and up to 7 arguments.
 * Additionally, few SMC calls that originate from EL2 leverage the seventh
 * argument explicitly. Given that TFTF runs in EL2, we need to be able to
 * specify it.
 */
typedef struct {
    /* Function identifier. Identifies which function is being invoked. */
    uint32_t    fid;

    u_register_t    arg1;
    u_register_t    arg2;
    u_register_t    arg3;
    u_register_t    arg4;
    u_register_t    arg5;
    u_register_t    arg6;
    u_register_t    arg7;
} smc_args;

/* SMC calls can return up to 8 register values */
typedef struct {
    u_register_t    ret0;
    u_register_t    ret1;
    u_register_t    ret2;
    u_register_t    ret3;
    u_register_t    ret4;
    u_register_t    ret5;
    u_register_t    ret6;
    u_register_t    ret7;
} smc_ret_values;

/*
 * Trigger an SMC call.
 */
smc_ret_values pal_smc(const smc_args *args);

/*
 * Trigger an HVC call.
 */
typedef smc_args hvc_args;

typedef smc_ret_values hvc_ret_values;

hvc_ret_values pal_hvc(const hvc_args *args);

#endif

/* Should match with IDs defined in SPM/Hafnium */
#define SPM_INTERRUPT_ENABLE            (0xFF03)
#define SPM_INTERRUPT_GET               (0xFF04)
#define SPM_INTERRUPT_DEACTIVATE    (0xFF08)
#define SPM_DEBUG_LOG                   (0xBD000000)


/*
 * Hypervisor Calls Wrappers
 */

uint32_t spm_interrupt_get(void);
int64_t spm_interrupt_enable(uint32_t int_id, bool enable, enum interrupt_pin pin);
int64_t spm_interrupt_deactivate(uint32_t vint_id);

/* Sleep for at least 'ms' milliseconds and return the elapsed time(ms). */
uint64_t sp_sleep_elapsed_time(uint64_t ms);

/* Sleep for at least 'ms' milliseconds. */
void sp_sleep(uint64_t ms);

/*
 * Check that expr == expected.
 * If not, loop forever.
 */
void expect(int expr, int expected);

#endif /* SPMC_H */