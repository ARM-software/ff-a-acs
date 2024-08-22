/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#pragma once
#if !defined(__cplusplus)

/* Verbosity enums, Lower the value, higher the verbosity */
typedef enum {
    INFO    = 1,
    DBG     = 2,
    TEST    = 3,
    WARN    = 4,
    ERROR   = 5,
    ALWAYS  = 9
} print_verbosity_t;

#define static_assert _Static_assert
#define LOG_BUFFER_SIZE 8192

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0) && defined(VM1_COMPILE)
#define PRINT_VM_ID()                                 \
    do {                                              \
           pal_printf(19, "[0000 %d]", pal_get_cpuid(read_mpidr_el1() & MPID_MASK)); \
    } while (0);
#else
#define PRINT_VM_ID()                                 \
    do {                                              \
    } while (0);
#endif

/* Macro to print the message and control the verbosity */
#define PAL_LOG(verbosity, fmt, ...)                   \
    do {                                               \
    if (verbosity >= VERBOSITY) {                      \
            PRINT_VM_ID()                              \
    }                                                  \
        pal_printf(verbosity, fmt, ##__VA_ARGS__);     \
    } while (0);

#define LOG(verbosity, fmt, ...)                       \
    do {                                               \
    if (verbosity >= VERBOSITY) {                      \
            PRINT_VM_ID()                              \
    }                                                  \
        pal_printf(verbosity, fmt, ##__VA_ARGS__);     \
    } while (0);

#endif
