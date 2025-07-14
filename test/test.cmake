#-------------------------------------------------------------------------------
# Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
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
    "${ROOT_DIR}/test/v1.0/${SUITE}/*/*.h"
    "${ROOT_DIR}/test/v1.0/${SUITE}/*/*.c"
    "${ROOT_DIR}/test/v1.0/${SUITE}/*/*/*.c"
    "${ROOT_DIR}/test/v1.0/${SUITE}/*/*/*.S"
    "${ROOT_DIR}/test/v1.1/${SUITE}/*/*.h"
    "${ROOT_DIR}/test/v1.1/${SUITE}/*/*.c"
    "${ROOT_DIR}/test/v1.1/${SUITE}/*/*/*.c"
    "${ROOT_DIR}/test/v1.1/${SUITE}/*/*/*.S"
    "${ROOT_DIR}/test/v1.2/${SUITE}/*/*.h"
    "${ROOT_DIR}/test/v1.2/${SUITE}/*/*.c"
    "${ROOT_DIR}/test/v1.2/${SUITE}/*/*/*.c"
    "${ROOT_DIR}/test/v1.2/${SUITE}/*/*/*.S"
)
endif()

if((${PLATFORM_FFA_V_ALL} EQUAL 1) OR (${PLATFORM_FFA_V_1_2} EQUAL 1))
    list(APPEND TEST_SRC ${ROOT_DIR}/test/common/test_database_v_1_2.c)
elseif(${PLATFORM_FFA_V_MULTI} EQUAL 1)
    list(APPEND TEST_SRC ${ROOT_DIR}/test/common/test_database_v_multi.c)
elseif(${PLATFORM_FFA_V_1_1} EQUAL 1)
    list(APPEND TEST_SRC ${ROOT_DIR}/test/common/test_database_v_1_1.c)
elseif(${PLATFORM_FFA_V_1_0} EQUAL 1)
    list(APPEND TEST_SRC ${ROOT_DIR}/test/common/test_database_v_1_0.c)
endif()


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
    ${COMMON_VAL_PATH}/inc/
)

unset(TEST_SRC)
