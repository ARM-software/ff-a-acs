#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(PAL_SRC
    ${ROOT_DIR}/platform/common/src/pal_libc.c
    ${ROOT_DIR}/platform/common/src/pal_misc_asm.S
    ${ROOT_DIR}/platform/common/src/pal_spinlock.S
    ${ROOT_DIR}/platform/common/src/pal_sp_helpers.c
    ${ROOT_DIR}/platform/common/src/pal_spm_helpers.c
    ${ROOT_DIR}/platform/common/src/pal_asm_smc.S
    ${ROOT_DIR}/platform/pal_baremetal/${TARGET}/src/pal_driver.c
    ${ROOT_DIR}/platform/pal_baremetal/${TARGET}/src/pal_misc.c
    ${ROOT_DIR}/platform/pal_baremetal/${TARGET}/src/pal_mmio.c
    ${ROOT_DIR}/platform/pal_baremetal/${TARGET}/src/pal_vcpu_setup.c
    ${ROOT_DIR}/platform/pal_baremetal/${TARGET}/src/pal_irq.c
    ${ROOT_DIR}/platform/driver/src/pal_pl011_uart.c
    ${ROOT_DIR}/platform/driver/src/pal_sp805_watchdog.c
    ${ROOT_DIR}/platform/driver/src/pal_ap_refclk_timer.c
    ${ROOT_DIR}/platform/driver/src/pal_nvm.c
    ${ROOT_DIR}/platform/driver/src/gic/pal_arm_gic_v2v3.c
    ${ROOT_DIR}/platform/driver/src/gic/pal_gic_common.c
    ${ROOT_DIR}/platform/driver/src/gic/pal_arm_gic_v2.c
    ${ROOT_DIR}/platform/driver/src/gic/pal_gic_v3.c
    ${ROOT_DIR}/platform/driver/src/gic/pal_gic_v2.c
    ${ROOT_DIR}/platform/driver/src/gic/platform.S
    ${ROOT_DIR}/platform/driver/src/pal_smmuv3_testengine.c
)

# Create PAL library
add_library(${PAL_LIB} STATIC ${PAL_SRC})

target_include_directories(${PAL_LIB} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ROOT_DIR}/platform/common/inc/
    ${ROOT_DIR}/platform/common/inc/aarch64/
    ${ROOT_DIR}/platform/pal_baremetal/${TARGET}/inc
    ${ROOT_DIR}/platform/driver/inc/
)

unset(PAL_SRC)
