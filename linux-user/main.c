/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "val_framework.h"
#include "pal_kernel_helpers.h"

int main(void)
{
    int rc;

    fprintf(stdout, "VM1 User Space Application Start\n");
    uint64_t delay = 0xFFFFFFFFFU;
    while (delay--);

    rc = pal_open_ffa_acs_drv();
    if (rc < 0)
    {
        perror("Error during FFA ACS DRV OPEN");
        abort();
    }

    rc = mem_pool_init(pal_get_acs_drv_fd());
    if (rc < 0)
    {
        perror("Error during memory pool init");
        abort();
    }

    val_main();

    rc = pal_close_ffa_acs_drv();
    if (rc < 0)
    {
        perror("Error during FFA ACS DRV CLOSE");
    }

    return 0;
}