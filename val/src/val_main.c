/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_memory.h"
#include "val_test_dispatch.h"

/* Stack memory */
__attribute__ ((aligned (128))) uint8_t val_stack[STACK_SIZE];

/**
 *   @brief    - C entry function for endpoint
 *   @param    - void
 *   @return   - void (Never returns)
**/
void val_main(void)
{
#ifndef TARGET_LINUX
    /* Configure and enable Stage-1 MMU */
     if (val_setup_mmu())
    {
        goto exit;
    }
#endif

    /* Ready to run test regression */
    val_run_test_suite();

#ifndef TARGET_LINUX
exit:
#endif
    LOG(ALWAYS, "Entering standby.. \n", 0, 0);
    pal_terminate_simulation();
}
