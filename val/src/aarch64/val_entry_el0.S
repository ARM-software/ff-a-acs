/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "val_def.h"

    .extern val_main
    .globl    val_entry

/* Provision one stack per Execution Context (or vCPU) */
.section .bss.stacks
    .balign 64
    .fill   4096
stacks_end:

    .section .text.val_entry, "ax"

/* Call FFA_MEM_PERM_SET_32 to set the permissions of a given memory region. */
.macro ffa_mem_perm_set start:req end:req perm:req
    adrp x29, \start
    add x29, x29, :lo12:\start

    adrp x30, \end
    add x30, x30, :lo12:\end

    /* x30 = end - start */
    sub x30, x30, x29
    /* x28 = x30 >> 12 (number of pages) */
    mov x28, #12
    lsrv x28, x30, x28

    /* 0x84000089 is function identifier for FFA_MEM_PERM_SET_32 */
    mov w0, #0x89
    movk w0, 0x8400, lsl #16
    mov x1, x29 /* Base VA */
    mov x2, x28 /* Page count */
    mov w3, #\perm /* Memory permissions */
    svc #0

    /* 0x84000061 is function identifier for FFA_SUCCESS_32 */
    mov w1, #0x61
    movk w1, #0x8400, lsl #16
    cmp w1, w0
    b.ne .
.endm

val_entry:

    /* Setup the stack pointer to call C entry */
    adrp    x0, stacks_end
    add     x0, x0, :lo12:stacks_end
    mov     sp, x0

    /* RODATA+DATA+BSS marked RW so relocations can succeed. */
    ffa_mem_perm_set __RODATA_START__ __BSS_END__ 5

    /* Relocate symbols */
pie_fixup:
    ldr    x0, =pie_fixup
    and    x0, x0, #~(PAGE_ALIGNMENT - 1)
    mov    x1, #IMAGE_SIZE
    add    x1, x1, x0
    bl    fix_symbol_table

    /* Clear S-EL0 partition BSS */
    adr x2, val_image_load_offset
    ldr x2, [x2]
    adr x0, bss_start_addr
    ldr x0, [x0]
    add x0, x0, x2
    adr x1, bss_end_addr
    ldr x1, [x1]
    add x1, x1, x2
    sub x1, x1, x0
2:
    stp xzr, xzr, [x0]
    add x0, x0, #16
    sub x1, x1, #16
    cmp xzr, x1
    b.ne 2b

    /* Then mark RODATA as RO */
    ffa_mem_perm_set __RODATA_START__ __RODATA_END__ 7

    /* And jump to the C entrypoint. */
    b    val_main

/* ---------------------------------------------------------------------------
 * Helper to fixup Global Offset table (GOT) at runtime.
 *
 * This function is used as the partition is compiled with -fpie
 * and linked with -pie options. We rely on the linker script exporting
 * appropriate markers for start and end of the section. For GOT, we
 * expect __GOT_START__ and __GOT_END__.
 *
 * The function takes the limits of the memory to apply fixups to as
 * arguments (which is usually the limits of the relocable BL image).
 *   x0 -  the start of the fixup region
 *   x1 -  the limit of the fixup region
 * These addresses have to be max page aligned(64k).
 * ---------------------------------------------------------------------------
 */

 fix_symbol_table:

    mov    x6, x0
    mov    x7, x1

    /* Test if the limits are page aligned */
    orr    x0, x0, x1
    tst    x0, #(PAGE_ALIGNMENT - 1)
    b.eq   1f
    b .
1:

    /*
     * Calculate the offset based on return address in x30.
     * Assume that this function is called within a page at the start of
     * fixup region.
     */
    and    x2, x30, #~(PAGE_ALIGNMENT - 1)
    sub    x0, x2, x6    /* Diff(S) = Current Address - Compiled Address */
    adrp   x1, __GOT_START__
    add    x1, x1, :lo12:__GOT_START__
    adrp   x2, __GOT_END__
    add    x2, x2, :lo12:__GOT_END__

    /*
     * GOT is an array of 64_bit addresses which must be fixed up as
     * new_addr = old_addr + Diff(S).
     * The new_addr is the address currently the binary is executing from
     * and old_addr is the address at compile time.
     */
2:
    ldr    x3, [x1]
    /* Skip adding offset if address is < lower limit */
    cmp    x3, x6
    b.lo   3f
    /* Skip adding offset if address is >= upper limit */
    cmp    x3, x7
    b.ge   3f
    add    x3, x3, x0
    str    x3, [x1]
3:
    add    x1, x1, #8
    cmp    x1, x2
    b.lo   2b

    /* set Image offset variable */
    adr   x1, val_image_load_offset
    str   x0, [x1]

    ret


 .section .data.far_addr, "aw"
  .align 12
  .global val_image_load_offset
val_image_load_offset:
  .fill  8
  .global bss_start_addr
bss_start_addr:
   .quad __BSS_START__
  .global bss_end_addr
bss_end_addr:
   .quad __BSS_END__

  .global text_start_addr
text_start_addr:
   .quad __TEXT_START__

