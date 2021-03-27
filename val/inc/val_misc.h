/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_MISC_H_
#define _VAL_MISC_H_

#include "val.h"

void *val_memory_alloc(size_t size);
uint32_t val_memory_free(void *address, size_t size);
int val_memcmp(void *src, void *dest, size_t len);
void *val_memcpy(void *dst, const void *src, size_t len);
void *val_memset(void *dst, int val, size_t count);
char *val_strcat(char *str1, char *str2, size_t output_buff_size);
void *val_mem_virt_to_phys(void *va);
#endif /* _VAL_MISC_H_ */
