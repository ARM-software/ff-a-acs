/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
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
    /* Index-0: Valid non-zero input version check */
    {
        .major = FFA_VERSION_MAJOR,
        .minor = FFA_VERSION_MINOR,
        .expected_status = ((FFA_VERSION_MAJOR << 16) | FFA_VERSION_MINOR),
    },
};

#endif /* _FFA_VERSION_DATA_H_ */
