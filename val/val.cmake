#-------------------------------------------------------------------------------
# Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Listing all common sources from val
set(VAL_SRC ${ROOT_DIR}/val/src/val_main.c
            ${ROOT_DIR}/val/src/val_framework.c
            ${ROOT_DIR}/val/src/val_ffa_abi.c
            ${ROOT_DIR}/val/src/val_test_dispatch.c
            ${ROOT_DIR}/val/src/val_endpoint_info.c
            ${ROOT_DIR}/val/src/val_misc.c
            ${ROOT_DIR}/val/src/val_ffa_helpers.c
            ${ROOT_DIR}/val/src/val_vcpu_setup.c
            ${ROOT_DIR}/val/src/val_shemaphore.c
            ${ROOT_DIR}/val/src/val_exceptions.c
            ${ROOT_DIR}/val/src/aarch64/val_vtable.S
            ${ROOT_DIR}/val/src/aarch64/val_sysreg.S
            ${ROOT_DIR}/val/src/aarch64/val_syscalls.S
            ${ROOT_DIR}/val/src/val_irq.c
            ${ROOT_DIR}/val/src/val_wd.c
            ${ROOT_DIR}/val/src/val_malloc.c
            ${FFA_GEN_OUTPUT_DIR}/val_endpoints.c)

if(NOT DEFINED VM_TARGET_LINUX_APP)
    list(APPEND VAL_SRC
        ${ROOT_DIR}/val/src/val_memory.c
        ${ROOT_DIR}/val/src/val_timer.c)

    if((${PROJECT_NAME} STREQUAL "vm1") OR (${PLATFORM_SP_EL} STREQUAL 1))
        list(APPEND VAL_SRC
            ${ROOT_DIR}/val/src/aarch64/val_entry.S)
    else()
        list(APPEND VAL_SRC
            ${ROOT_DIR}/val/src/aarch64/val_entry_el0.S)
    endif()
endif()

# Create VAL library
add_library(${VAL_LIB} STATIC ${VAL_SRC})

set(MY_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ROOT_DIR}/val/inc/
    ${FFA_GEN_OUTPUT_DIR}/
    ${ROOT_DIR}/platform/common/inc/
    ${ROOT_DIR}/platform/pal_baremetal/${TARGET}/inc/)

if(DEFINED VM_TARGET_LINUX_APP)
    list(APPEND MY_INCLUDE_DIRS ${ROOT_DIR}/linux-user/inc/)
else()
    list(APPEND MY_INCLUDE_DIRS
        ${ROOT_DIR}/platform/common/inc/aarch64/
        ${ROOT_DIR}/platform/driver/inc/)
endif()
target_include_directories(${VAL_LIB} PRIVATE ${MY_INCLUDE_DIRS})

unset(MY_INCLUDE_DIRS)
unset(VAL_SRC)