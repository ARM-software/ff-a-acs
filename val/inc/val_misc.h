/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_MISC_H_
#define _VAL_MISC_H_

#include "val.h"
int val_memcmp(void *src, void *dest, size_t len);
void *val_memcpy(void *dst, const void *src, size_t len);
void *val_memset(void *dst, int val, size_t count);
char *val_strcat(char *str1, const char *str2, size_t output_buff_size);
int val_strcmp(char *str1, char *str2);
void *val_mem_virt_to_phys(void *va);
char *val_strcpy(char *dest, const char *src);
void *val_memmove(void *dest, const void *src, size_t n);
#endif /* _VAL_MISC_H_ */
