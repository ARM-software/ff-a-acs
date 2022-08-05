#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Compile all .c/.S files from test directory
if(${SUITE} STREQUAL "all")
file(GLOB TEST_SRC
    "${ROOT_DIR}/test/*/*/*.h"
    "${ROOT_DIR}/test/*/*/*/*.c"
    "${ROOT_DIR}/test/*/*/*.c"
    "${ROOT_DIR}/test/*/*/*/*.S"
)
else()
file(GLOB TEST_SRC
    "${ROOT_DIR}/test/${SUITE}/*/*.h"
    "${ROOT_DIR}/test/${SUITE}/*/*.c"
    "${ROOT_DIR}/test/${SUITE}/*/*/*.c"
    "${ROOT_DIR}/test/${SUITE}/*/*/*.S"
)
endif()

list(APPEND TEST_SRC
    ${ROOT_DIR}/test/common/test_database.c
)

# Create TEST library
add_library(${TEST_LIB} STATIC ${TEST_SRC})

target_include_directories(${TEST_LIB} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ROOT_DIR}/val/inc/
    ${ROOT_DIR}/platform/common/inc/
    ${ROOT_DIR}/platform/common/inc/aarch64/
    ${ROOT_DIR}/platform/pal_baremetal/${TARGET}/inc/
    ${ROOT_DIR}/platform/driver/inc/
    ${ROOT_DIR}/test/common/
)

unset(TEST_SRC)
