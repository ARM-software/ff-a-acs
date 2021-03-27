/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_VCPU_SETUP_H_
#define _VAL_VCPU_SETUP_H_

#include "val_memory.h"

uint32_t val_get_no_of_cpus(void);
uint32_t val_get_cpuid(uint64_t mpid);
uint64_t val_get_mpid(uint32_t cpuid);
uint32_t val_power_on_cpu(uint32_t target_cpuid);
uint32_t val_power_off_cpu(void);

#endif /* _VAL_VCPU_SETUP_H_ */
