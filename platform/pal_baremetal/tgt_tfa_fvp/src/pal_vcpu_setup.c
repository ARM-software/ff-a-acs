/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"

static const uint64_t mpid_array[PLATFORM_NO_OF_CPUS] = {
    0x1000000, //cpu0
    0x1000100, //cpu1
    0x1000200, //cpu2
    0x1000300, //cpu3
    0x1010000, //cpu4
    0x1010100, //cpu5
    0x1010200, //cpu6
    0x1010300, //cpu7
};

static const uint64_t vmpid_array[PLATFORM_NO_OF_CPUS] = {
    0x0, //cpu0
    0x1, //cpu1
    0x2, //cpu2
    0x3, //cpu3
    0x4, //cpu4
    0x5, //cpu5
    0x6, //cpu6
    0x7, //cpu7
};

uint32_t pal_get_no_of_cpus(void)
{
    return PLATFORM_NO_OF_CPUS;
}

uint32_t pal_get_cpuid(uint64_t mpid)
{
    uint32_t cpu_index = 0;

    mpid = mpid & MPID_MASK;

    /* In case physical mpidr returned by partition manager */
    for (cpu_index = 0; cpu_index < pal_get_no_of_cpus(); cpu_index++)
    {
        if (mpid == mpid_array[cpu_index])
            return cpu_index;
    }

    /* In case virtual mpidr returned by partition manager */
    for (cpu_index = 0; cpu_index < pal_get_no_of_cpus(); cpu_index++)
    {
        if (mpid == vmpid_array[cpu_index])
            return cpu_index;
    }

    return PAL_INVALID_CPU_INFO;
}

uint64_t pal_get_mpid(uint32_t cpuid)
{
    if (cpuid < pal_get_no_of_cpus())
        return  mpid_array[cpuid];
    else
        return PAL_INVALID_CPU_INFO;
}

static uint32_t pal_psci_cpu_on(uint64_t target_cpu,
                                void *entry_point_address,
                                 uint64_t context_id)
{
    return pal_syscall_for_psci(SMC_PSCI_CPU_ON_AARCH64,
                                target_cpu,
                                (uint64_t)entry_point_address,
                                context_id);
}

static uint32_t pal_psci_cpu_off(void)
{
    return pal_syscall_for_psci(SMC_PSCI_CPU_OFF, 0, 0, 0);
}

uint32_t pal_power_on_cpu(uint64_t mpid)
{
    uint64_t target_cpu = mpid;
    uint32_t ret;

    ret = pal_psci_cpu_on(target_cpu, &pal_secondary_cpu_boot_entry, CONTEXT_ID_VALUE);
    if (ret == PSCI_E_SUCCESS)
    {
        return PAL_SUCCESS;
    }
    else
    {
        pal_printf("\tPSCI_CPU_ON failed, ret=0x%x\n", ret, 0);
        return PAL_ERROR;
    }
}

uint32_t pal_power_off_cpu(void)
{
    uint32_t ret = pal_psci_cpu_off();

    pal_printf("\tPSCI_CPU_OFF failed, ret=0x%x\n", ret, 0);
    return PAL_ERROR;
}
