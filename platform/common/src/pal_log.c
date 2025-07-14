/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"
#include "pal_common_val_intf.h"

/**
 *   @brief    This function prints the given string and data onto the uart
 *   @param    msg      - Input String
 *   @param    data1    - Value for first format specifier
 *   @param    data2    - Value for second format specifier
 *   @return   SUCCESS/FAILURE
**/

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
                if (data == 0) {
                    pal_uart_putc('0');
                } else {
                    while (data != 0)
                    {
                        j         = data % 10;
                        data      = data / 10;
                        buffer[i] = (uint8_t)(j + '0');
                        i        += 1;
                    }
                    while (i > 0)
                    {
                        pal_uart_putc(buffer[--i]);
                    }
                }
                data = data2;
            }
            else if (*msg == 'x' || *msg == 'X')
            {
                if (data == 0) {
                    pal_uart_putc('0');
                } else {
                    while (data != 0)
                    {
                        j         = data & 0xf;
                        data      = data >> 4;
                        buffer[i] = (uint8_t)(j + ((j > 9) ? 'A' - 10 : '0'));
                        i        += 1;
                    }
                    while (i > 0)
                    {
                        pal_uart_putc(buffer[--i]);
                    }
                }
                data = data2;
            }
        }
        else
        {
            if (*msg == '\n') {
                pal_uart_putc('\r');
            }
            pal_uart_putc(*msg);
        }
    }

    return PAL_SUCCESS;
}
