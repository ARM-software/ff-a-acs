/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_vcpu_setup.h"

/**
 *   @brief    - Returns number of cpus in the system.
 *   @param    - void
 *   @return   - Number of cpus
**/
uint32_t val_get_no_of_cpus(void)
{
    return pal_get_no_of_cpus();
}

/**
 *   @brief    - Convert mpid to logical cpu number
 *   @param    - mpid value
 *   @return   - Logical cpu number
**/
uint32_t val_get_cpuid(uint64_t mpid)
{
    return pal_get_cpuid(mpid);
}

/**
 *   @brief    - Return mpid value of given logical cpu index
 *   @param    - Logical cpu index
 *   @return   - mpid value
**/
uint64_t val_get_mpid(uint32_t cpuid)
{
    return pal_get_mpid(cpuid);
}

/**
 *   @brief    - Power up the given core
 *   @param    - Logical cpuid value of the core
 *   @return   - SUCCESS/FAILURE
**/
uint32_t val_power_on_cpu(uint32_t target_cpuid)
{
    return pal_power_on_cpu(val_get_mpid(target_cpuid));
}

/**
 *   @brief    - Power down the calling core.
 *   @param    - Void
 *   @return   - The call does not return when successful. Otherwise, it returns VAL_ERROR.
**/
uint32_t val_power_off_cpu(void)
{
    return pal_power_off_cpu();
}
