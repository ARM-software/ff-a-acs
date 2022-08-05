/*
 * Copyright (c) 2022, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <pal_arch_helpers.h>

#include <pal_spm_helpers.h>

/*******************************************************************************
 * Test framework helpers
 ******************************************************************************/

void expect(int expr, int expected)
{
    if (expr != expected) {
        pal_printf("Expected value %i, got %i\n", (uint64_t)expected, (uint64_t)expr);
        while (1)
            continue;
    }
}

uint64_t sp_sleep_elapsed_time(uint32_t ms)
{
    uint64_t timer_freq = read_cntfrq_el0();

    pal_printf("Timer frequency = %llu Sleeping for %u milliseconds\n", timer_freq, (uint64_t)ms);

    uint64_t time1 = virtualcounter_read();
    volatile uint64_t time2 = time1;

    while ((time2 - time1) < ((ms * timer_freq) / 1000U)) {
        time2 = virtualcounter_read();
    }

    return ((time2 - time1) * 1000) / timer_freq;
}

void sp_sleep(uint32_t ms)
{
    (void)sp_sleep_elapsed_time(ms);
}
