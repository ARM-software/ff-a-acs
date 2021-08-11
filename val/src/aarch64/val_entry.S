/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_def.h"

    .extern val_main
    .extern val_stack
    .extern vector_table
    .extern val_fixup_symbol_table

    .cfi_sections .debug_frame
    .globl    val_entry
    .section .text.val_entry, "ax"
val_entry:

#if (PLATFORM_SP_EL != 0)
   /* Install vector table */
    adr     x0, vector_table
    msr     vbar_el1, x0

    /* MMU must be disabled at partition boot entry for VM and EL1 SP */
    mrs    x0, sctlr_el1
    and    x1, x0, #SCTLR_M_BIT
    cmp    xzr, x1
    b.eq   1f
    b .
1:
    /* Enable I-Cache */
    orr    x0, x0, #SCTLR_I_BIT
    msr    sctlr_el1, x0
    isb
#endif /* PLATFORM_SP_EL != 0 */

    /* Relocate symbols */
pie_fixup:
    ldr    x0, =pie_fixup
    and    x0, x0, #~(PAGE_ALIGNMENT - 1)
    mov    x1, #IMAGE_SIZE
    add    x1, x1, x0
    bl    fix_symbol_table

    /* Setup the stack pointer to call C entry */
    adr    x0, val_stack + STACK_SIZE
    mov    sp, x0

   /* Clear BSS */
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
   stp xzr, xzr, [x0]

    /* And jump to the C entrypoint. */
    b      val_main

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
