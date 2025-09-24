/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once
#if !defined(__cplusplus)

#include "pal_interfaces.h"

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0) && defined(VM1_COMPILE)
#define PRINT_VM_ID()                                                                         \
    do {                                                                                      \
           pal_printf("[0000 %d]", pal_get_cpuid(read_mpidr_el1() & MPID_MASK), 0);         \
    } while (0);
#else
#define PRINT_VM_ID()                                                                         \
    do {                                                                                      \
    } while (0);
#endif

/* Macro to print the message and control the verbosity */
#if (TARGET_LINUX == 1) && defined(VM1_COMPILE)
#define PAL_LOG(fmt, ...)                                                      \
    do {                                                                                  \
        PRINT_VM_ID();                                                                \
        pal_printf(fmt, ##__VA_ARGS__);                                    \
    } while (0)

#else
#define PAL_LOG(fmt, x, y)                                                                \
    do {                                                                                  \
        PRINT_VM_ID();                                                                    \
        pal_printf(fmt, x, y);                                                            \
    } while (0)

#endif
#endif/* _PAL_LOG_H_ */