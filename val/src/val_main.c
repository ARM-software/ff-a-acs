/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_memory.h"
#include "val_test_dispatch.h"

/* Stack memory */
__attribute__ ((aligned (4096))) uint8_t val_stack[STACK_SIZE];

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
    if (val_get_curr_endpoint_logical_id() == VM1)
    {
#ifndef TARGET_LINUX
        write_hcr_el2((1 << 27)); //TGE=1
#endif
        val_irq_setup();
    }
    val_run_test_suite();

#ifndef TARGET_LINUX
exit:
#endif
    LOG(ALWAYS, "Entering standby.. \n", 0, 0);
    pal_terminate_simulation();
}
