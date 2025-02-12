/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pal_log.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

/**
 *   @brief    - This function prints the given string and data onto the uart
 *   @param    - str      : Input String
 *             - ...      : ellipses for variadic args
 *   @return   - SUCCESS((Any positive number for character written)/FAILURE(0)
**/
uint32_t pal_printf(print_verbosity_t verbosity, const char *msg, ...)
{
    size_t chars_written = 0;
    va_list args;

    va_start(args, msg);
    if (verbosity >= VERBOSITY)
    {

        switch (verbosity)
        {
            case INFO:
                fprintf(stdout, "\t\tINFO: ");
                break;

            case DBG:
                fprintf(stdout, "\t\tDBG: ");
                break;

            case TEST:
                fprintf(stdout, "\t");
                break;

            case WARN:
                fprintf(stdout, "\tWARN: ");
                break;

            case ERROR:
                fprintf(stdout, "\tERROR: ");
                break;

            case ALWAYS:
                fprintf(stdout, "\t");
                break;

            default:
                chars_written = (size_t)vprintf(msg, args);
                return (uint32_t)chars_written;
                break;
        }
        chars_written = (size_t)vprintf(msg, args);
        fprintf(stdout, "\r");
        fprintf(stdout, "\n");
        va_end(args);
    }

    return (uint32_t)chars_written;
}