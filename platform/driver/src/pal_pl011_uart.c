/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_pl011_uart.h"

static volatile uint64_t g_uart = PLATFORM_UART_BASE;
static uint8_t is_uart_init_done = 0;

/**
 *   @brief    - This function initializes the UART
 *   @param    - uart_base_addr: Base address of UART
 *   @return   - none
**/
static void driver_uart_pl011_init(void)
{
    uint32_t bauddiv = (UART_PL011_CLK_IN_HZ * 4) / UART_PL011_BAUDRATE;

    /* Disable uart before programming */
    ((uart_t *)g_uart)->uartcr &= ~UART_PL011_UARTCR_EN_MASK ;

    /* Write the IBRD */
    ((uart_t *)g_uart)->uartibrd = bauddiv >> 6;

    /* Write the FBRD */
    ((uart_t *)g_uart)->uartfbrd = bauddiv & 0x3F;

    /* Set line of control */
    ((uart_t *)g_uart)->uartlcr_h = UART_PL011_LINE_CONTROL;

    /* Clear any pending errors */
    ((uart_t *)g_uart)->uartecr = 0;

    /* Enable tx, rx, and uart overall */
    ((uart_t *)g_uart)->uartcr = UART_PL011_UARTCR_EN_MASK
                            | UART_PL011_UARTCR_TX_EN_MASK;
}

/**
 *   @brief    - This function checks for empty TX FIFO
 *   @param    - none
 *   @return   - status
**/
static int driver_uart_pl011_is_tx_empty(void)
{
    if ((((uart_t *)g_uart)->uartcr & UART_PL011_UARTCR_EN_MASK) &&
        /* UART is enabled */
        (((uart_t *)g_uart)->uartcr & UART_PL011_UARTCR_TX_EN_MASK) &&
        /* Transmit is enabled */
        ((((uart_t *)g_uart)->uartfr & UART_PL011_UARTFR_TX_FIFO_FULL) == 0))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 *   @brief    - This function checks for empty TX FIFO and writes to FIFO register
 *   @param    - char to be written
 *   @return   - none
**/
void driver_uart_pl011_putc(uint8_t c)
{
    const uint8_t pdata = (uint8_t)c;

    if (is_uart_init_done == 0)
    {
        driver_uart_pl011_init();
        is_uart_init_done = 1;
    }

    /* ensure TX buffer to be empty */
    while (!driver_uart_pl011_is_tx_empty())
      ;

    /* write the data (upper 24 bits are reserved) */
    ((uart_t *)g_uart)->uartdr = pdata;
}
