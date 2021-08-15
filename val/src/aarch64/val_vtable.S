/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/**
 * This is a generic handler for exceptions taken at the current EL while using
 * SPx. It saves volatile registers, calls the C handler, restores volatile
 * registers, then returns.
 *
 * Saving state and jumping to C handler takes 15 instructions. We add an extra
 * branch to a common exit path. So each handler takes up one unique cache line
 * and one shared cache line (assuming 16-byte cache lines).
 */
.macro current_exception_spx elx:req handler:req
    save_volatile_to_stack \elx
    bl \handler
    b restore_from_stack_and_return
.endm

/**
 * Saves the volatile registers onto the stack. This currently takes 14
 * instructions, so it can be used in exception handlers with 18 instructions
 * left, 2 of which in the same cache line (assuming a 16-byte cache line).
 *
 * On return, x0 and x1 are initialised to elr_el2 and spsr_el2 respectively,
 * which can be used as the first and second arguments of a subsequent call.
 */
.macro save_volatile_to_stack elx:req
    /* Reserve stack space and save registers x0-x18, x29 & x30. */
    stp x0, x1, [sp, #-(8 * 24)]!
    stp x2, x3, [sp, #8 * 2]
    stp x4, x5, [sp, #8 * 4]
    stp x6, x7, [sp, #8 * 6]
    stp x8, x9, [sp, #8 * 8]
    stp x10, x11, [sp, #8 * 10]
    stp x12, x13, [sp, #8 * 12]
    stp x14, x15, [sp, #8 * 14]
    stp x16, x17, [sp, #8 * 16]
    str x18, [sp, #8 * 18]
    stp x29, x30, [sp, #8 * 20]

    /*
     * Save elr_elx & spsr_elx. This such that we can take nested exception
     * and still be able to unwind.
     */
    mrs x0, elr_\elx
    mrs x1, spsr_\elx
    stp x0, x1, [sp, #8 * 22]
.endm

  .section .text.vtable, "ax"
  .align 12

.global vector_table
vector_table:

// ------------------------------------------------------------
// Current EL with SP0
// ------------------------------------------------------------
  .balign 128
sync_current_el_sp0:
  B        .                    //        Synchronous
  //current_exception_sp0 el1 val_irq_current exception_handler_return

  .balign 128
irq_current_el_sp0:
  B        .                    //        IRQ

  .balign 128
fiq_current_el_sp0:
  B        .                    //        FIQ

  .balign 128
serror_current_el_sp0:
  B        .                    //        SError

// ------------------------------------------------------------
// Current EL with SPx
// ------------------------------------------------------------

  .balign 128
sync_current_el_spx:
  //B        .                    //        Synchronous
  current_exception_spx el1 val_sync_exception_current

  .balign 128
irq_current_el_spx:
  //B        .                    //        IRQ
  current_exception_spx el1 val_irq_current

  .balign 128
fiq_current_el_spx:
  B        .                    //        FIQ

  .balign 128
serror_current_el_spx:
  B        .                    //        SError

// ------------------------------------------------------------
// Lower EL using AArch64
// ------------------------------------------------------------

  .balign 128
sync_lower_el_aarch64:
   B        .

  .balign 128
irq_lower_el_aarch64:
  B        .                    //        IRQ

  .balign 128
fiq_lower_el_aarch64:
  B        .                    //        FIQ

  .balign 128
serror_lower_el_aarch64:
  B        .                    //        SError

// ------------------------------------------------------------
// Lower EL using AArch32
// ------------------------------------------------------------

  .balign 128
sync_lower_el_aarch32:
   B        .

  .balign 128
irq_lower_el_aarch32:
  B        .                    //        IRQ

  .balign 128
fiq_lower_el_aarch32:
  B        .                    //        FIQ

  .balign 128
serror_lower_el_aarch32:
  B        .                    //        SError

.balign 0x40
/**
 * Restores the volatile registers from the stack.

 * Register x0: if false restores elr_el1, if true retains the value of elr_el1.
 * This enables exception handlers to indicate whether they have changed the
 * value of elr_el1 (e.g., to skip the faulting instruction).
 */
restore_from_stack_and_return:
    /* Restore registers x2-x18, x29 & x30. */
    ldp x2, x3, [sp, #8 * 2]
    ldp x4, x5, [sp, #8 * 4]
    ldp x6, x7, [sp, #8 * 6]
    ldp x8, x9, [sp, #8 * 8]
    ldp x10, x11, [sp, #8 * 10]
    ldp x12, x13, [sp, #8 * 12]
    ldp x14, x15, [sp, #8 * 14]
    ldp x16, x17, [sp, #8 * 16]
    ldr x18, [sp, #8 * 18]
    ldp x29, x30, [sp, #8 * 20]

    cbnz x0, skip_elr

    /* Restore register elr_el1 using x1 as scratch. */
    ldr x1, [sp, #8 * 22]
    msr elr_el1, x1

skip_elr:
    /* Restore register spsr_el1 using x1 as scratch. */
    ldr x1, [sp, #8 * 23]
    msr spsr_el1, x1

    /* Restore x0 & x1, and release stack space. */
    ldp x0, x1, [sp], #8 * 24

    eret
// ------------------------------------------------------------
// Translation tables
// ------------------------------------------------------------

  .section .bss.tt, "aw"
  .align 16
  .global tt_l0_base
tt_l0_base:
  .fill  4096

  .align 16
  .global tt_l1_base
tt_l1_base:
  .fill 4096

  .align 16
 // Allocate space for 4 contiguous L2 tables
  .global tt_l2_base_1
tt_l2_base_1:
  .fill 16384

  .align 16
 // Allocate space for 4 contiguous L2 tables
  .global tt_l2_base_2
tt_l2_base_2:
  .fill 16384

  .align 16
 // Allocate space for 4 contiguous L2 tables
  .global tt_l2_base_3
tt_l2_base_3:
  .fill 16384

  .align 16
 // Allocate space for 4 contiguous L2 tables
  .global tt_l2_base_4
tt_l2_base_4:
  .fill 16384

   .align 16
 // Allocate space for 4 contiguous L3 tables
  .global tt_l3_base_1
tt_l3_base_1:
  .fill 16384

  .align 16
// Allocate space for 4 contiguous L3 tables
  .global tt_l3_base_2
tt_l3_base_2:
  .fill 16384

  .align 16
// Allocate space for 4 contiguous L3 tables
  .global tt_l3_base_3
tt_l3_base_3:
  .fill 16384

    .align 16
// Allocate space for 4 contiguous L3 tables
  .global tt_l3_base_4
tt_l3_base_4:
  .fill 16384
