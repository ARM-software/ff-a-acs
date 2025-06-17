/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
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
#if ((PLATFORM_SP_EL == 1) || (defined(VM1_COMPILE)))
void val_main(void)
{

#if ((PLATFORM_NS_HYPERVISOR_PRESENT == 1) && (defined(VM1_COMPILE)))
    ffa_args_t payload;
    payload.arg1 = (FFA_VERSION_MAJOR << 16) | FFA_VERSION_MINOR;
    val_ffa_version(&payload);
    if (payload.fid == FFA_ERROR_NOT_SUPPORTED)
    {
        LOG(ERROR, "FFA Version Negotiation Failed fid %x arg2 %x", payload.fid, payload.arg2);
        VAL_PANIC("Terminating ACS Run");
    }

    LOG(ALWAYS, "Negotiate FF-A Version %x", (FFA_VERSION_MAJOR << 16) | FFA_VERSION_MINOR);
#endif

#if ((defined(TARGET_LINUX) || (PLATFORM_NS_HYPERVISOR_PRESENT == 1)) && (defined(VM1_COMPILE)))
    val_endpoint_info_t *info_ptr = (val_endpoint_info_t *)val_get_endpoint_info();
    ffa_endpoint_id_t id;

    /* Update Primary VM1 ID as per hypervisor info */
    id = val_get_curr_endpoint_id();
    info_ptr[VM1].id = id;
#endif

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
#if !defined(TARGET_LINUX) && (PLATFORM_NS_HYPERVISOR_PRESENT == 0)
        write_hcr_el2((1 << 27)); //TGE=1
#endif
        val_irq_setup();
    }

    val_run_test_suite();

#ifndef TARGET_LINUX
exit:
#endif
    LOG(ALWAYS, "Entering standby.. ");
    pal_terminate_simulation();
}
#else
void val_main(void)
{
    ffa_args_t payload;

    val_memset(&payload, 0, sizeof(ffa_args_t));

    /* FFA_SUCCESS case: Returns 16-bit ID of calling FF-A component. */
    val_ffa_id_get(&payload);
    if (payload.fid != FFA_SUCCESS_32)
    {
        LOG(ERROR, "EL0: Check failed for ffa_id_get success case");
        goto exit;
    }

    LOG(ALWAYS, "EL0 entry.. id %lx", payload.arg2);
    val_run_test_suite();

exit:
    LOG(ALWAYS, "Entering standby.. ");
    pal_terminate_simulation();
}
#endif
