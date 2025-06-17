/*
 * Copyright (c) 2022-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

    .section .text.pal_misc_asm, "ax"

    .extern pal_get_cpuid
    .extern val_secondary_cpu_test_entry

.equ STACK_SIZE, 0x1000

.globl pal_uart_putc_hypcall
pal_uart_putc_hypcall:
    mov x2, x0 /* Char c */
    mov x1, #1
    mov x0, #0x8A /* FID */
    movk x0, #0xC400, lsl #16
#if (PLATFORM_SP_EL == 0)
    svc #0
#else
    hvc #0
#endif
    ret

/* int32_t pal_syscall_for_psci(uint64_t fid, uint64_t x1, uint64_t x2, uint64_t x3); */
.globl pal_syscall_for_psci
pal_syscall_for_psci:
    smc #0
    ret

.globl pal_secondary_cpu_boot_entry
pal_secondary_cpu_boot_entry:
    /* Temporary stack setup to call C function */
    mov x0, xzr
    adrp x1, stack_base
    add  x1, x1, :lo12:stack_base
    add  x1, x1, #STACK_SIZE
    mov  x2, #STACK_SIZE
    mul  x2, x0, x2
    add  sp, x1, x2

    /* Get logical cpuid for sec cpu */
    mrs x0, mpidr_el1
    bl pal_get_cpuid

    /* Use separate stack for each of the sec cpus */
    adrp x1, stack_base
    add  x1, x1, :lo12:stack_base
    add  x1, x1, #STACK_SIZE
    mov  x2, #STACK_SIZE
    mul  x2, x0, x2
    add  sp, x1, x2

    bl val_secondary_cpu_test_entry

    ret

.section .bss.stack_space, "aw"
    .balign 128
    .global stack_base
stack_base:
    .fill STACK_SIZE * 16