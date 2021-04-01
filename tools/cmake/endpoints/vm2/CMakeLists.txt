#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

project(vm2 LANGUAGES C ASM)

set(EXE_NAME "${PROJECT_NAME}")

add_definitions(-DVM2_COMPILE)

set(PAL_LIB ${EXE_NAME}_pal_lib)
set(VAL_LIB ${EXE_NAME}_val_lib)
set(TEST_LIB ${EXE_NAME}_test_lib)

include(${ROOT_DIR}/platform/pal_baremetal/${TARGET}/pal.cmake)
include(${ROOT_DIR}/val/val.cmake)
include(${ROOT_DIR}/test/test.cmake)

create_executable(${EXE_NAME})
