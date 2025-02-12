/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/**
 *   @brief    - Open FFA ACS Kernel Driver Instance and File Descriptor
 *   @return   - SUCCESS(0)/FAILURE(Negative Integer)
**/
int pal_open_ffa_acs_drv(void);


/**
 *   @brief    - Close FFA ACS Kernel Driver Instance
 *   @return   - SUCCESS(0)/FAILURE(Negative Integer)
**/
int pal_close_ffa_acs_drv(void);


/**
 *   @brief    - Get FFA ACS Kernel Driver Descriptor
 *   @return   - SUCCESS((Any positive number)/FAILURE(Negative Integer)
**/
int pal_get_acs_drv_fd(void);

/**
 *   @brief    - Initialize memory pool in kernel space and map it to user, release kernel access
 *   @return   - SUCCESS(0)/FAILURE(Negative Integer)
**/
int mem_pool_init(int fd);

void *mem_pool_get_virt(int fd);
uint64_t mem_pool_get_phys(int fd);