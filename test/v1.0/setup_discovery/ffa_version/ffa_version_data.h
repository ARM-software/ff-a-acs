/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _FFA_VERSION_DATA_H_
#define _FFA_VERSION_DATA_H_

#include "test_database.h"

typedef struct {
    uint16_t major;
    uint16_t minor;
    uint64_t expected_status;
} test_data;

static const test_data check[] = {
    /* Index-0: Zero input version check */
    {
        .major = 0,
        .minor = 0,
#if (PLATFORM_FFA_V_1_0 == 1)
        .expected_status = ((FFA_VERSION_MAJOR << 16) | FFA_VERSION_MINOR),
#elif (PLATFORM_FFA_V_1_1 == 1 || PLATFORM_FFA_V_ALL == 1)
        .expected_status = FFA_ERROR_NOT_SUPPORTED,
#endif
    },
    /* Index-1: Valid non-zero input version check */
    {
        .major = FFA_VERSION_MAJOR,
        .minor = FFA_VERSION_MINOR,
        .expected_status = ((FFA_VERSION_MAJOR << 16) | FFA_VERSION_MINOR),
    },
    /* Index-2: Incompatible minor version check */
    {
        .major = FFA_VERSION_MAJOR,
        .minor = FFA_VERSION_MINOR + 2,
#if (PLATFORM_FFA_V_1_0 == 1)
        .expected_status = ((FFA_VERSION_MAJOR << 16) | FFA_VERSION_MINOR),
#elif (PLATFORM_FFA_V_1_1 == 1 || PLATFORM_FFA_V_ALL == 1)
        .expected_status = FFA_ERROR_NOT_SUPPORTED,
#endif
    },
    /* Index-3: Incompatible major version check */
    {
        .major = FFA_VERSION_MAJOR + 1,
        .minor = FFA_VERSION_MINOR,
#if (PLATFORM_FFA_V_1_0 == 1)
        .expected_status = ((FFA_VERSION_MAJOR << 16) | FFA_VERSION_MINOR),
#elif (PLATFORM_FFA_V_1_1 == 1 || PLATFORM_FFA_V_ALL == 1)
        .expected_status = FFA_ERROR_NOT_SUPPORTED,
#endif
    },
    /* Index-4: bit-31 must be zero check */
    {
        .major = (FFA_VERSION_MAJOR | (1 << 15)),
        .minor = FFA_VERSION_MINOR,
        .expected_status = FFA_ERROR_NOT_SUPPORTED,
    },
};

#endif /* _FFA_VERSION_DATA_H_ */
