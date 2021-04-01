/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_H_
#define _PAL_H_

#ifndef TARGET_LINUX
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#else
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/page-def.h>
#endif

#ifndef __UNUSED
#define __UNUSED __attribute__((unused))
#endif

typedef uintptr_t addr_t;

/* Status macro */
#define PAL_SUCCESS  0
#define PAL_ERROR    1

#define PAL_INVALID_CPU_INFO  0xffffffff


#define SMC_PSCI_CPU_ON_AARCH64     0xc4000003
#define SMC_PSCI_CPU_OFF            0x84000002

#define PSCI_E_SUCCESS              0x0
#define PSCI_E_NOT_SUPPORTED        0xffffffff //-1
#define PSCI_E_INVALID_PARAMS       0xfffffffe //-2
#define PSCI_E_DENIED               0xfffffffd //-3
#define PSCI_E_ALREADY_ON           0xfffffffc //-4
#define PSCI_E_ON_PENDING           0xfffffffb //-5
#define PSCI_E_INTERN_FAIL          0xfffffffa //-6
#define PSCI_E_NOT_PRESENT          0xfffffff9 //-7
#define PSCI_E_DISABLED             0xfffffff8 //-8
#define PSCI_E_INVALID_ADDRESS      0xfffffff7 //-9

#define CONTEXT_ID_VALUE 0x55555555

#endif /* _PAL_H_ */
