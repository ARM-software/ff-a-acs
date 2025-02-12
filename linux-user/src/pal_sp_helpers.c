/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


#include <stdint.h>
#include <stdlib.h>

#include "pal_arch_helpers.h"
#include "pal_spm_helpers.h"

/*******************************************************************************
 * Test framework helpers
 ******************************************************************************/

void expect(int expr, int expected)
{
    if (expr != expected) {
        PAL_LOG(ERROR, "Expected value %i, got %i", (uint64_t)expected, (uint64_t)expr);
        while (1)
            continue;
    }
}

uint64_t sp_sleep_elapsed_time(uint64_t ms)
{
    (void)ms;
    return ms;
}

#if ((PLATFORM_SP_EL == 0) && !defined(VM1_COMPILE))
/* Need to be adjusted based on platform */
#define ITERATIONS_PER_MS 10000

static inline void while_wait_loop(uint64_t ms)
{
    uint64_t timeout = ms * ITERATIONS_PER_MS;
    uint64_t loop;
    volatile uint64_t count = 0; /* to prevent optimization*/

    PAL_LOG(INFO, "Executing software loop based wait for ms %d", ms, 0);
    for (loop = 0; loop < timeout; loop++) {
        /* Wait */
        count++;
    }
}
#endif

void sp_sleep(uint64_t ms)
{
#if ((PLATFORM_SP_EL == 0) && !defined(VM1_COMPILE))
    while_wait_loop(ms);
#else
    (void)sp_sleep_elapsed_time(ms);
#endif
}