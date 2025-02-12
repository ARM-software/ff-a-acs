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
#include "pal_kernel_helpers.h"

#define DEV_ENTRY   "/dev/ffa-acs"
#define POOL_SIZE   0x1000 * 5
#define RXTX_SIZE   0x1000

static void *mem_pool_virt;
static uint64_t mem_pool_curr;
static uint64_t mem_pool_phys;

int fd;

/**
 *   @brief    - Initialize memory pool in kernel space and map it to user, release kernel access
 *   @return   - SUCCESS(0)/FAILURE(Negative Integer)
**/
void *mem_pool_get_virt(int fd)
{
    (void)fd;
    return mem_pool_virt;
}

uint64_t mem_pool_get_phys(int fd)
{
    (void)fd;
    return mem_pool_phys;
}

/**
 *   @brief    - Initialize memory pool in kernel space and map it to user, release kernel access
 *   @return   - SUCCESS(0)/FAILURE(Negative Integer)
**/
int mem_pool_init(int fd)
{
    int rc;
    uint64_t alloc_size = POOL_SIZE;

    rc = ioctl(fd, FFA_ACS_IOC_MEM_ALLOC, &alloc_size);
    if (rc < 0) {
        perror("Error during FFA_ACS_IOC_MEM_ALLOC");
        return -1;
    }
    mem_pool_phys = alloc_size;

    mem_pool_virt = mmap(NULL, POOL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                            fd, 0);
    if (mem_pool_virt == NULL)
    {
        perror("Error during mmap of the pool");
        return -1;
    }
    mem_pool_curr = 0;

    fprintf(stdout, "Mem pool at virtual address 0x%lx size 0x%x\n",
            (uint64_t) mem_pool_virt, POOL_SIZE);


    fprintf(stdout, "Mem pool at physical address 0x%lx size 0x%x\n",
            (uint64_t) mem_pool_phys, POOL_SIZE);

    rc = ioctl(fd, FFA_ACS_IOC_MEM_FREE, NULL);
    if (rc < 0) {
        perror("Free ioctl did not work");
    }

    return 0;
}

/**
 *   @brief    - Open FFA ACS Kernel Driver Instance and File Descriptor
 *   @return   - SUCCESS(0)/FAILURE(Negative Integer)
**/
int pal_open_ffa_acs_drv(void)
{

    fd = open(DEV_ENTRY, O_RDWR);
    if (fd < 0) {
        perror("Error opening " DEV_ENTRY);
        return -1;
    }
    return fd;
}

/**
 *   @brief    - Close FFA ACS Kernel Driver Instance
 *   @return   - SUCCESS(0)/FAILURE(Negative Integer)
**/
int pal_close_ffa_acs_drv(void)
{
    if (close(fd) < 0) {
        perror("Error closing " DEV_ENTRY);
        return -1;
    }
    return 0;
}


/**
 *   @brief    - Get FFA ACS Kernel Driver Descriptor
 *   @return   - SUCCESS((Any positive number)/FAILURE(Negative Integer)
**/
int pal_get_acs_drv_fd(void)
{
    if (fd > 0)
        return fd;
    else
        perror("File Descriptor Unitialized, FFA DRV Open Required\n");
    return -1;
}