#-------------------------------------------------------------------------------
# Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

project(vm1 LANGUAGES C ASM)
if(${TARGET_LINUX} STREQUAL 1)
set(EXE_NAME "acs-linux-app1")
else()
set(EXE_NAME "${PROJECT_NAME}")
endif()

add_definitions(-DVM1_COMPILE)

if(${TARGET_LINUX} STREQUAL 1)
add_definitions(-DTARGET_LINUX=${TARGET_LINUX})
add_definitions(-DTARGET_LINUX_ID="1")
set(VM_TARGET_LINUX_APP 1)
set(SOURCES
    ${ROOT_DIR}/linux-user/main.c
)
endif()

set(PAL_LIB ${EXE_NAME}_pal_lib)
set(VAL_LIB ${EXE_NAME}_val_lib)
set(TEST_LIB ${EXE_NAME}_test_lib)

if(DEFINED VM1_FFA_V)
    set(PLATFORM_FFA_V ${VM1_FFA_V})
    add_definitions(-DPLATFORM_FFA_V=${VM1_FFA_V})
    message(STATUS "[ACS] : VM1 PLATFORM_FFA_V is set to ${VM1_FFA_V}")
endif()

if(${TARGET_LINUX} STREQUAL 1)
include(${ROOT_DIR}/linux-user/pal.cmake)
else()
include(${ROOT_DIR}/platform/pal_baremetal/${TARGET}/pal.cmake)
endif()

include(${ROOT_DIR}/val/val.cmake)
include(${ROOT_DIR}/test/test.cmake)

if(${TARGET_LINUX} STREQUAL 1)
    set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--no-dynamic-linker")

    # Add executable
    add_executable(${EXE_NAME} ${SOURCES})

    target_include_directories(${EXE_NAME} PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${ROOT_DIR}/val/inc/
        ${ROOT_DIR}/linux-user/inc/
        ${ROOT_DIR}/platform/common/inc/
    )

    # Link dependency
    target_link_libraries(${TEST_LIB} PRIVATE ${VAL_LIB} ${PAL_LIB})

    # Link libraries
    target_link_libraries(${EXE_NAME} PRIVATE ${VAL_LIB} ${TEST_LIB})

    # Create the dump info
    add_custom_command(OUTPUT ${EXE_NAME}.dump
        COMMAND ${GNUARM_OBJDUMP} ${GNUARM_OBJDUMP_FLAGS} ${EXE_NAME}.elf > ${EXE_NAME}.dump
        DEPENDS ${EXE_NAME})

    # Copy the binary to common output dir
    add_custom_command(OUTPUT ${EXE_NAME}_copy_elf COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.elf ${BUILD}/output/ DEPENDS ${EXE_NAME})

    # Group into a single custom target
    add_custom_target(process_elf ALL DEPENDS ${EXE_NAME}.dump
        ${EXE_NAME}_copy_elf ${EXE_NAME}.elf)
else()
create_executable(${EXE_NAME})
endif()