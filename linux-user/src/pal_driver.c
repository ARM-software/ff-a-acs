/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "arm_ffa_acs.h"
#include "pal_interfaces.h"
#include "pal_kernel_helpers.h"

void pal_linux_call_conduit(void *args)
{
    int rc = 0;
    int fd = 0;
    struct ffa_acs_smc *smc_args = (struct ffa_acs_smc *)args;

    fd = pal_get_acs_drv_fd();
    rc = ioctl(fd, FFA_ACS_IOC_CALL, smc_args);
    if (rc < 0)
    {
        perror("Error during FFA_ID_GET ioctl");
        abort();
    }
}

uint32_t pal_nvm_write(uint32_t offset, void *buffer, size_t size)
{
    /* Empty api - not in use */
    (void)offset;
    (void)buffer;
    (void)size;
    return PAL_SUCCESS;
}

uint32_t pal_nvm_read(uint32_t offset, void *buffer, size_t size)
{
    /* Empty api - not in use */
    (void)offset;
    (void)buffer;
    (void)size;
    return PAL_SUCCESS;
}

uint32_t pal_watchdog_enable(void)
{
    /* Empty api - not in use */
    return PAL_SUCCESS;
}

uint32_t pal_watchdog_disable(void)
{
    /* Empty api - not in use */
    return PAL_SUCCESS;
}

uint32_t pal_twdog_enable(uint32_t ms)
{
    /* Empty api - not in use */
    (void)ms;
    return PAL_SUCCESS;
}

uint32_t pal_twdog_disable(void)
{
    /* Empty api - not in use */
    return PAL_SUCCESS;
}

void pal_twdog_intr_enable(void)
{
    /* Empty api - not in use */
    return;
}

void pal_twdog_intr_disable(void)
{
    /* Empty api - not in use */
    return;
}

void pal_ns_wdog_enable(uint32_t ms)
{
    (void)ms;
}

void pal_ns_wdog_disable(void)
{
return;
}

void pal_ns_wdog_intr_enable(void)
{
    /* Empty api - not in use */
    return;
}

void pal_ns_wdog_intr_disable(void)
{
    /* Empty api - not in use */
    return;
}

uint64_t pal_sleep(uint32_t ms)
{
    /* Empty api - not in use */
    (void)ms;
    return PAL_SUCCESS;
}

void pal_secure_intr_enable(uint32_t int_id, enum interrupt_pin pin)
{
    /* Empty api - not in use */
    (void)int_id;
    (void)pin;
    return;
}

void pal_secure_intr_disable(uint32_t int_id, enum interrupt_pin pin)
{
    /* Empty api - not in use */
    (void)int_id;
    (void)pin;
    return;
}

uint32_t pal_smmu_device_configure(uint32_t stream_id, uint64_t source, uint64_t dest,
                                     uint64_t size, bool secure)
{
    /* Empty api - not in use */
    (void)stream_id;
    (void)source;
    (void)dest;
    (void)size;
    (void)secure;
    return PAL_SUCCESS;
}
