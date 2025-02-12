/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"

uint32_t pal_get_no_of_cpus(void)
{
    return 0;
}

uint32_t pal_get_cpuid(uint64_t mpdir)
{
    (void)mpdir;
    return 0;
}

uint64_t pal_get_mpid(uint32_t cpuid)
{
    (void)cpuid;
    return 0;
}

uint32_t pal_power_on_cpu(uint64_t mpidr)
{
    (void)mpidr;
    return PAL_SUCCESS;
}

uint32_t pal_power_off_cpu(void)
{
    return PAL_SUCCESS;
}

void pal_secondary_cpu_boot_entry(void)
{
    return;
}
