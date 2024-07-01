/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_timer.h"
#include "val_irq.h"
#include "pal_interfaces.h"
#include "val.h"

/**
 *   @brief   This API disables system physical timer
 *   @param   int_mask mask interrupt
 *   @return  status
**/
uint32_t val_sys_phy_timer_dis(bool int_mask)
{
    return pal_ap_phy_refclk_dis(int_mask);
}

/**
 *   @brief   This API sets physical system timer interrupt
 *   @param   timeout timeout in microseconds
 *   @return  status
**/
uint32_t val_sys_phy_timer_en(uint32_t timeout)
{
    return pal_ap_phy_refclk_en(timeout);
}

/**
 *   @brief   This API disables system virtual timer
 *   @param   int_mask mask interrupt
 *   @return  status
**/
uint32_t val_sys_virt_timer_dis(bool int_mask)
{
    return pal_ap_virt_refclk_dis(int_mask);
}

/**
 *   @brief   This API sets virtual system timer interrupt
 *   @param   timeout timeout in microseconds
 *   @return  status
**/
uint32_t val_sys_virt_timer_en(uint32_t timeout)
{
    return pal_ap_virt_refclk_en(timeout);
}