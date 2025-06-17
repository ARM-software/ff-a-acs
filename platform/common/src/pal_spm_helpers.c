/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pal_arch_helpers.h>
#include <pal_spm_helpers.h>

/*******************************************************************************
 * Secure Monitor Calls Wrappers
 ******************************************************************************/
 smc_ret_values asm_smc64(uint32_t fid,
    u_register_t arg1,
    u_register_t arg2,
    u_register_t arg3,
    u_register_t arg4,
    u_register_t arg5,
    u_register_t arg6,
    u_register_t arg7);

smc_ret_values pal_smc(const smc_args *args)
{
return asm_smc64(args->fid,
    args->arg1,
    args->arg2,
    args->arg3,
    args->arg4,
    args->arg5,
    args->arg6,
    args->arg7);
}

/*******************************************************************************
 * Hypervisor Calls Wrappers
 ******************************************************************************/
hvc_ret_values asm_hvc64(uint32_t fid,
                  u_register_t arg1,
                  u_register_t arg2,
                  u_register_t arg3,
                  u_register_t arg4,
                  u_register_t arg5,
                  u_register_t arg6,
                  u_register_t arg7);

hvc_ret_values pal_hvc(const hvc_args *args)
{
    return asm_hvc64(args->fid,
                  args->arg1,
                  args->arg2,
                  args->arg3,
                  args->arg4,
                  args->arg5,
                  args->arg6,
                  args->arg7);
}

uint32_t spm_interrupt_get(void)
{
    hvc_args args = {
        .fid = SPM_INTERRUPT_GET
    };

    hvc_ret_values ret = pal_hvc(&args);

    return (uint32_t)ret.ret0;
}

/**
 * Hypervisor call to enable/disable SP delivery of a virtual interrupt of
 * int_id value through the IRQ or FIQ vector (pin).
 * Returns 0 on success, or -1 if passing an invalid interrupt id.
 */
int64_t spm_interrupt_enable(uint32_t int_id, bool enable, enum interrupt_pin pin)
{
    hvc_args args = {
        .fid = SPM_INTERRUPT_ENABLE,
        .arg1 = int_id,
        .arg2 = enable,
        .arg3 = pin
    };

    hvc_ret_values ret = pal_hvc(&args);

#if ((PLATFORM_SP_EL == 1) || (defined(VM1_COMPILE)))
    if (enable)
    {
        enable_irq();
        enable_fiq();
    } else {
        disable_irq();
        disable_fiq();
    }
#endif

    return (int64_t)ret.ret0;
}

/**
 * Hypervisor call to drop the priority and de-activate a secure interrupt.
 * Returns 0 on success, or -1 if passing an invalid interrupt id.
 */
int64_t spm_interrupt_deactivate(uint32_t vint_id)
{
    hvc_args args = {
        .fid = SPM_INTERRUPT_DEACTIVATE,
        .arg1 = vint_id, /* pint_id */
        .arg2 = vint_id
    };

    hvc_ret_values ret = pal_hvc(&args);
#if ((PLATFORM_SP_EL == 1) || (defined(VM1_COMPILE)))
    disable_irq();
#endif
    return (int64_t)ret.ret0;
}
