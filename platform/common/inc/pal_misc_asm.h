/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_MISC_ASM_H_
#define _PAL_MISC_ASM_H_

extern void pal_uart_putc_hypcall(char c);
extern void pal_secondary_cpu_boot_entry(void);
extern uint32_t pal_syscall_for_psci(uint64_t fid, uint64_t x1, uint64_t x2, uint64_t x3);

#endif /* _PAL_MISC_ASM_H_ */
