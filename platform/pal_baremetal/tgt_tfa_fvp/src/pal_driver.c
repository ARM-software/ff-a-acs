/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"
#include <pal_arch_helpers.h>
#include "pal_pl011_uart.h"
#include "pal_nvm.h"
#include "pal_sp805_watchdog.h"
#include "pal_ap_refclk_timer.h"
#include "pal_smmuv3_testengine.h"
#include "pal_spm_helpers.h"

#if (defined(SP1_COMPILE) || defined(SP2_COMPILE) || defined(VM2_COMPILE) ||\
     defined(SP3_COMPILE) || defined(SP4_COMPILE) || defined(VM3_COMPILE))
/* Use hyp log system call for sp2, sp3, sp4, vm2 and vm3 */
#define pal_uart_putc(x) pal_uart_putc_hypcall((char)x)
#else
/* Use platform uart for vm1 & sp1 */
#define pal_uart_putc(x) driver_uart_pl011_putc(x)
#endif

uint32_t pal_printf(const char *msg, uint64_t data1, uint64_t data2)
{
    uint8_t buffer[16];
    uint64_t j, i = 0;
    uint64_t data = data1;

    for (; *msg != '\0'; ++msg)
    {
        if (*msg == '%')
        {
            ++msg;
            if (*msg == 'l' || *msg == 'L')
                ++msg;

            if (*msg == 'd')
            {
                while (data != 0)
                {
                    j         = data % 10;
                    data      = data / 10;
                    buffer[i] = (uint8_t)(j + 48);
                    i        += 1;
                }
                data = data2;
            }
            else if (*msg == 'x' || *msg == 'X')
            {
                while (data != 0)
                {
                    j         = data & 0xf;
                    data      = data >> 4;
                    buffer[i] = (uint8_t)(j + ((j > 9) ? 55 : 48));
                    i        += 1;
                }
                data = data2;
            }
            if (i > 0)
            {
                while (i > 0)
                {
                    pal_uart_putc(buffer[--i]);
                }
            }
            else
            {
                pal_uart_putc(48);
            }
        }
        else
        {
            pal_uart_putc(*msg);

            if (*msg == '\n')
            {
                pal_uart_putc('\r');
            }
        }
    }
    return PAL_SUCCESS;
}

uint32_t pal_nvm_write(uint32_t offset, void *buffer, size_t size)
{
    return driver_nvm_write(offset, buffer, size);
}

uint32_t pal_nvm_read(uint32_t offset, void *buffer, size_t size)
{
    return driver_nvm_read(offset, buffer, size);
}

uint32_t pal_watchdog_enable(void)
{
    driver_sp805_wdog_start(PLATFORM_WDOG_BASE);
    return PAL_SUCCESS;
}

uint32_t pal_watchdog_disable(void)
{
    driver_sp805_wdog_stop(PLATFORM_WDOG_BASE);
    return PAL_SUCCESS;
}

uint32_t pal_ap_phy_refclk_en(uint32_t us)
{
    spm_interrupt_enable(PALTFORM_AP_REFCLK_CNTPSIRQ1, true, INTERRUPT_TYPE_IRQ);
    driver_ap_refclk_p_set(us, true);
    return PAL_SUCCESS;
}

uint32_t pal_ap_phy_refclk_dis(bool int_mask)
{
    spm_interrupt_deactivate(PALTFORM_AP_REFCLK_CNTPSIRQ1);
    driver_ap_refclk_p_disable(int_mask);
    return PAL_SUCCESS;
}

uint32_t pal_ap_virt_refclk_en(uint32_t us)
{
    spm_interrupt_enable(PALTFORM_AP_REFCLK_CNTPSIRQ1, true, INTERRUPT_TYPE_IRQ);
    driver_ap_refclk_p_set(us, true);
    return PAL_SUCCESS;
}

uint32_t pal_ap_virt_refclk_dis(bool int_mask)
{
    spm_interrupt_deactivate(PALTFORM_AP_REFCLK_CNTPSIRQ1);
    driver_ap_refclk_p_disable(int_mask);
    return PAL_SUCCESS;
}

uint32_t pal_twdog_enable(uint32_t ms)
{
    driver_sp805_wdog_refresh(PLATFORM_SP805_TWDOG_BASE);
    driver_sp805_twdog_start(PLATFORM_SP805_TWDOG_BASE, ms);
    spm_interrupt_enable(PLATFORM_TWDOG_INTID, true, INTERRUPT_TYPE_IRQ);
    return PAL_SUCCESS;
}

uint32_t pal_twdog_disable(void)
{
    driver_sp805_wdog_stop(PLATFORM_SP805_TWDOG_BASE);
    return PAL_SUCCESS;
}

void pal_twdog_intr_enable(void)
{
    spm_interrupt_enable(PLATFORM_TWDOG_INTID, true, INTERRUPT_TYPE_IRQ);
}

void pal_twdog_intr_disable(void)
{
    spm_interrupt_deactivate(PLATFORM_TWDOG_INTID);
}

void pal_ns_wdog_enable(uint32_t ms)
{
    driver_ns_wdog_start(ms);
}

void pal_ns_wdog_disable(void)
{
    driver_ns_wdog_stop();
}

void pal_ns_wdog_intr_enable(void)
{
    spm_interrupt_enable(PLATFORM_NS_WD_INTR, true, INTERRUPT_TYPE_IRQ);
}

void pal_ns_wdog_intr_disable(void)
{
    spm_interrupt_deactivate(PLATFORM_NS_WD_INTR);
}

void pal_secure_intr_enable(uint32_t int_id, enum interrupt_pin pin)
{
    spm_interrupt_enable(int_id, true, pin);
}

void pal_secure_intr_disable(uint32_t int_id, enum interrupt_pin pin)
{
    spm_interrupt_enable(int_id, false, pin);
}

uint64_t pal_sleep(uint32_t ms)
{
    return sp_sleep_elapsed_time(ms);
}

uint32_t pal_smmu_device_configure(uint32_t stream_id, uint64_t source, uint64_t dest,
                                     uint64_t size, bool secure)
{
    return smmuv3_configure_testengine(stream_id, source, dest, size, secure);
}
