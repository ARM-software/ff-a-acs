/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#pragma once
#if !defined(__cplusplus)

#include "pal_log.h"
#include "val_common_log.h"

#define LOG(verbosity, fmt, ...)                                                              \
    do {                                                                                      \
    if (verbosity >= VERBOSITY) {                                                             \
            PRINT_VM_ID()                                                                     \
    }                                                                                         \
    val_printf(verbosity, fmt, ##__VA_ARGS__);                                                \
    if (verbosity >= VERBOSITY) {                                                             \
        if (verbosity == ERROR) {                                                             \
                PRINT_VM_ID()                                                                 \
                val_printf(verbosity, "\tCheck failed at %s ,line:%d\n", __FILE__, __LINE__); \
        }                                                                                     \
    }                                                                                         \
    } while (0);
#endif
