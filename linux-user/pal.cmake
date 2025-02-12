#-------------------------------------------------------------------------------
# Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Listing all the sources from val
set(PAL_SRC
    ${ROOT_DIR}/linux-user/src/pal_log.c
    ${ROOT_DIR}/linux-user/src/pal_libc.c
    ${ROOT_DIR}/linux-user/src/pal_vcpu_setup.c
    ${ROOT_DIR}/linux-user/src/pal_irq.c
    ${ROOT_DIR}/linux-user/src/pal_driver.c
    ${ROOT_DIR}/linux-user/src/pal_sp_helpers.c
    ${ROOT_DIR}/linux-user/src/pal_kernel_helpers.c)
# Create PAL library
add_library(${PAL_LIB} STATIC ${PAL_SRC})

target_include_directories(${PAL_LIB} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ROOT_DIR}/linux-user/inc/
    ${ROOT_DIR}/platform/common/inc/
    ${ROOT_DIR}/platform/common/inc/aarch64/
)

unset(PAL_SRC)
