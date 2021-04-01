/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

void print_help(void)
{
    printf ("\nUsage: ./ffa_acs\n");
}

int parse_cmdline(int argc, char **argv)
{
    int c = 0;
    struct option long_opt[] =
    {
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    /* Process Command Line arguments */
    while ((c = getopt_long(argc, argv, "h:", long_opt, NULL)) != -1)
    {
        switch (c)
        {
            case 'h':
                print_help();
                return 1;
                break;
            default:
                printf("unknown commandline option\n");
                print_help();
                return 1;
        }
    }

    return 0;
}

int main (int argc, char **argv)
{
    int status;
    int flag = 1;
    FILE *fd = NULL;

    status = parse_cmdline(argc, argv);
    if (status)
        return 1;

    fd = fopen("/proc/ffa_acs", "w");
    if (fd == NULL) {
        printf("Unable to open /proc/ffa_acs\n");
        return 1;
    }

    fwrite(&flag, 1, sizeof(flag), fd);
    fclose(fd);

    return 0;
}
