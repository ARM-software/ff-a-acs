#-------------------------------------------------------------------------------
# Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

project(ff_a_acs LANGUAGES)

### Tool dependency check - start ###

# Set the minimum required version of CMake for the project
cmake_minimum_required(VERSION 3.10)

# cmake_policy
cmake_policy(SET CMP0057 NEW)

### Tool dependency check - end ###

get_filename_component(ROOT_DIR . ABSOLUTE)

# Set internal build directory variable
set(BUILD ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "Setting build directory to ${BUILD}" FORCE)
file(MAKE_DIRECTORY ${BUILD}/output/)

#### Include cmake support module ###
include(${ROOT_DIR}/tools/cmake/toolchain/utils.cmake)
include(${ROOT_DIR}/tools/cmake/toolchain/default.cmake)
####


### Valid value range for command line argument ###

list(APPEND VERBOSE_LIST 1 2 3 4 5)
list(APPEND PLATFORM_SPMC_EL_LIST 1 2 3)
list(APPEND PLATFORM_SP_EL_LIST -1 0 1)
list(APPEND PLATFORM_NS_HYPERVISOR_PRESENT_LIST 0 1)
list(APPEND PLATFORM_NS_HYP_MULTI_VM_LIST 0 1)
list(APPEND PLATFORM_FFA_V_1_0_LIST 0 1)
list(APPEND PLATFORM_FFA_V_1_1_LIST 0 1)
list(APPEND PLATFORM_FFA_V_1_2_LIST 0 1)
list(APPEND PLATFORM_FFA_V_ALL_LIST 0 1)
list(APPEND ENABLE_PIE_LIST ON OFF)
list(APPEND ENABLE_BTI_LIST ON OFF)
list(APPEND TARGET_LINUX_LIST 0 1)
list(APPEND CMAKE_BUILD_TYPE_LIST Release Debug)
list(APPEND ARM_ARCH_MAJOR_LIST 8 9)
list(APPEND PLATFORM_FFA_V_MULTI_LIST 0 1)
list(APPEND XEN_SUPPORT_LIST 0 1)

###

### Command line argument check ###

if(CMAKE_BUILD_TYPE)
    if(NOT ${CMAKE_BUILD_TYPE} IN_LIST CMAKE_BUILD_TYPE_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DCMAKE_BUILD_TYPE=, supported values are : ${CMAKE_BUILD_TYPE_LIST}")
    endif()
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE INTERNAL "CMAKE_BUILD_TYPE is set to ${CMAKE_BUILD_TYPE}" FORCE)
else()
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_DFLT} CACHE INTERNAL "Defaulting CMAKE_BUILD_TYPE to ${CMAKE_BUILD_TYPE_DFLT}" FORCE)
endif()

# Check for valid targets
_get_sub_dir_list(TARGET_LIST ${ROOT_DIR}/platform/pal_baremetal/)
if(NOT DEFINED TARGET)
    set(TARGET ${TARGET_DFLT} CACHE INTERNAL "Defaulting target to ${TARGET}" FORCE)
else()
    set(TARGET ${TARGET} CACHE INTERNAL "TARGET is set to ${TARGET}" FORCE)
endif()

if(NOT ${TARGET} IN_LIST TARGET_LIST)
    message(FATAL_ERROR "[ACS] : Error: Unspported value for -DTARGET=, supported targets are : ${TARGET_LIST}")
else()
    message(STATUS "[ACS] : TARGET is set to ${TARGET}")
endif()

# Include Common VAL cmake support
include(${ROOT_DIR}/tools/cmake/common/CommonVAL.cmake)

# Check for valid suites
_get_sub_dir_list(SUITE_LIST_V_1_0  ${ROOT_DIR}/test/v1.0)
_get_sub_dir_list(SUITE_LIST_V_1_1 ${ROOT_DIR}/test/v1.1)

# Append all suites without duplicates across different FFA version
set(SUITE_LIST ${SUITE_LIST_V_1_0})
list(APPEND SUITE_LIST ${SUITE_LIST_V_1_1})
list(REMOVE_DUPLICATES SUITE_LIST)

if(NOT DEFINED SUITE)
    set(SUITE ${SUITE_DFLT} CACHE INTERNAL "Defaulting suite to ${SUITE}" FORCE)
else()
    set(SUITE ${SUITE} CACHE INTERNAL "SUITE is set to ${SUITE}" FORCE)
endif()

list(APPEND SUITE_LIST all)
if(NOT ${SUITE} IN_LIST SUITE_LIST)
    message(FATAL_ERROR "[ACS] : Error: Unspported value for -DSUITE=, supported values are : ${SUITE_LIST}")
else()
    message(STATUS "[ACS] : SUITE is set to ${SUITE}")
endif()

# Check for VERBOSE
if(NOT DEFINED VERBOSE)
    set(VERBOSE ${VERBOSE_DFLT} CACHE INTERNAL "Default VERBOSE value" FORCE)
        message(STATUS "[ACS] : Defaulting VERBOSE to ${VERBOSE}")
else()
    if(NOT ${VERBOSE} IN_LIST VERBOSE_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DVERBOSE=, supported values are : ${VERBOSE_LIST}")
    endif()
    message(STATUS "[ACS] : VERBOSE is set to ${VERBOSE}")
endif()

# Check for PLATFORM_SPMC_EL
if(NOT DEFINED PLATFORM_SPMC_EL)
    set(PLATFORM_SPMC_EL ${PLATFORM_SPMC_EL_DFLT} CACHE INTERNAL "Default PLATFORM_SPMC_EL value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_SPMC_EL to ${PLATFORM_SPMC_EL}")
else()
    if(NOT ${PLATFORM_SPMC_EL} IN_LIST PLATFORM_SPMC_EL_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_SPMC_EL=, supported values are : ${PLATFORM_SPMC_EL_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_SPMC_EL is set to ${PLATFORM_SPMC_EL}")
endif()

# Check for PLATFORM_SP_EL
if(NOT DEFINED PLATFORM_SP_EL)
    set(PLATFORM_SP_EL ${PLATFORM_SP_EL_DFLT} CACHE INTERNAL "Default PLATFORM_SP_EL value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_SP_EL to ${PLATFORM_SP_EL}")
else()
    if(NOT ${PLATFORM_SP_EL} IN_LIST PLATFORM_SP_EL_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_SP_EL=, supported values are : ${PLATFORM_SP_EL_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_SP_EL is set to ${PLATFORM_SP_EL}")
endif()

# Check for PLATFORM_NS_HYPERVISOR_PRESENT
if(NOT DEFINED PLATFORM_NS_HYPERVISOR_PRESENT)
    set(PLATFORM_NS_HYPERVISOR_PRESENT ${PLATFORM_NS_HYPERVISOR_PRESENT_DFLT} CACHE INTERNAL "Default PLATFORM_NS_HYPERVISOR_PRESENT value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_NS_HYPERVISOR_PRESENT to ${PLATFORM_NS_HYPERVISOR_PRESENT}")
else()
    if(NOT ${PLATFORM_NS_HYPERVISOR_PRESENT} IN_LIST PLATFORM_NS_HYPERVISOR_PRESENT_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_NS_HYPERVISOR_PRESENT=, supported values are : ${PLATFORM_NS_HYPERVISOR_PRESENT_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_NS_HYPERVISOR_PRESENT is set to ${PLATFORM_NS_HYPERVISOR_PRESENT}")
endif()

# Check for PLATFORM_NS_HYP_MULTI_VM
if(NOT DEFINED PLATFORM_NS_HYP_MULTI_VM)
    set(PLATFORM_NS_HYP_MULTI_VM ${PLATFORM_NS_HYP_MULTI_VM_DFLT} CACHE INTERNAL "Default PLATFORM_NS_HYP_MULTI_VM value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_NS_HYP_MULTI_VM to ${PLATFORM_NS_HYP_MULTI_VM}")
else()
    if(NOT ${PLATFORM_NS_HYP_MULTI_VM} IN_LIST PLATFORM_NS_HYP_MULTI_VM_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_NS_HYP_MULTI_VM=, supported values are : ${PLATFORM_NS_HYP_MULTI_VM_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_NS_HYP_MULTI_VM is set to ${PLATFORM_NS_HYP_MULTI_VM}")
endif()

# Check for PLATFORM_FFA_V_1_0
if(NOT DEFINED PLATFORM_FFA_V_1_0)
    set(PLATFORM_FFA_V_1_0 ${PLATFORM_FFA_V_1_0_DFLT} CACHE INTERNAL "Default PLATFORM_FFA_V_1_0 value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_FFA_V_1_0 to ${PLATFORM_FFA_V_1_0}")
else()
    if(NOT ${PLATFORM_FFA_V_1_0} IN_LIST PLATFORM_FFA_V_1_0_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_FFA_V_1_0=, supported values are : ${PLATFORM_FFA_V_1_0_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_FFA_V_1_0 is set to ${PLATFORM_FFA_V_1_0}")
endif()

# Check for PLATFORM_FFA_V_1_1
if(NOT DEFINED PLATFORM_FFA_V_1_1)
    set(PLATFORM_FFA_V_1_1 ${PLATFORM_FFA_V_1_1_DFLT} CACHE INTERNAL "Default PLATFORM_FFA_V_1_1 value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_FFA_V_1_1 to ${PLATFORM_FFA_V_1_1}")
else()
    if(NOT ${PLATFORM_FFA_V_1_1} IN_LIST PLATFORM_FFA_V_1_1_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_FFA_V_1_1=, supported values are : ${PLATFORM_FFA_V_1_1_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_FFA_V_1_1 is set to ${PLATFORM_FFA_V_1_1}")
endif()

# Check for PLATFORM_FFA_V_1_2
if(NOT DEFINED PLATFORM_FFA_V_1_2)
    set(PLATFORM_FFA_V_1_2 ${PLATFORM_FFA_V_1_2_DFLT} CACHE INTERNAL "Default PLATFORM_FFA_V_1_2 value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_FFA_V_1_2 to ${PLATFORM_FFA_V_1_2}")
else()
    if(NOT ${PLATFORM_FFA_V_1_2} IN_LIST PLATFORM_FFA_V_1_2_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_FFA_V_1_2=, supported values are : ${PLATFORM_FFA_V_1_2_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_FFA_V_1_2 is set to ${PLATFORM_FFA_V_1_2}")
endif()

# Check for PLATFORM_FFA_V_ALL
if(NOT DEFINED PLATFORM_FFA_V_ALL)
    set(PLATFORM_FFA_V_ALL ${PLATFORM_FFA_V_ALL_DFLT} CACHE INTERNAL "Default PLATFORM_FFA_V_ALL value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_FFA_V_ALL to ${PLATFORM_FFA_V_ALL}")
else()
    if(NOT ${PLATFORM_FFA_V_ALL} IN_LIST PLATFORM_FFA_V_ALL_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_FFA_V_ALL=, supported values are : ${PLATFORM_FFA_V_ALL_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_FFA_V_ALL is set to ${PLATFORM_FFA_V_ALL}")
endif()

# Check for PLATFORM_FFA_V_MULTI
if(NOT DEFINED PLATFORM_FFA_V_MULTI)
    set(PLATFORM_FFA_V_MULTI ${PLATFORM_FFA_V_MULTI_DFLT} CACHE INTERNAL "Default PLATFORM_FFA_V_MULTI value" FORCE)
        message(STATUS "[ACS] : Defaulting PLATFORM_FFA_V_MULTI to ${PLATFORM_FFA_V_MULTI}")
else()
    if(NOT ${PLATFORM_FFA_V_MULTI} IN_LIST PLATFORM_FFA_V_MULTI)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DPLATFORM_FFA_V_MULTI=, supported values are : ${PLATFORM_FFA_V_MULTI_LIST}")
    endif()
    message(STATUS "[ACS] : PLATFORM_FFA_V_MULTI is set to ${PLATFORM_FFA_V_MULTI}")
endif()

# Check for ARM_ARCH_MAJOR
if(NOT DEFINED ARM_ARCH_MAJOR)
    set(ARM_ARCH_MAJOR "${ARM_ARCH_MAJOR_DFLT}" CACHE INTERNAL "Default ARM_ARCH_MAJOR value" FORCE)
        message(STATUS "[ACS] : Defaulting ARM_ARCH_MAJOR to ${ARM_ARCH_MAJOR}")
else()
    if(NOT ${ARM_ARCH_MAJOR} IN_LIST ARM_ARCH_MAJOR_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DARM_ARCH_MAJOR=, supported values are : ${ARM_ARCH_MAJOR_LIST}")
    endif()
    message(STATUS "[ACS] : ARM_ARCH_MAJOR is set to ${ARM_ARCH_MAJOR}")
endif()

# Check for ARM_ARCH_MINOR
if(NOT DEFINED ARM_ARCH_MINOR)
    set(ARM_ARCH_MINOR "${ARM_ARCH_MINOR_DFLT}" CACHE INTERNAL "Default ARM_ARCH_MINOR value" FORCE)
        message(STATUS "[ACS] : Defaulting ARM_ARCH_MINOR to ${ARM_ARCH_MINOR}")
else()
    message(STATUS "[ACS] : ARM_ARCH_MINOR is set to ${ARM_ARCH_MINOR}")
endif()

# Check for ENABLE_PIE
if(NOT DEFINED ENABLE_PIE)
    set(ENABLE_PIE ${ENABLE_PIE_DFLT} CACHE INTERNAL "Default ENABLE_PIE value" FORCE)
        message(STATUS "[ACS] : Defaulting ENABLE_PIE to ${ENABLE_PIE}")
else()
    if(NOT ${ENABLE_PIE} IN_LIST ENABLE_PIE_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DENABLE_PIE=, supported values are : ${ENABLE_PIE_LIST}")
    endif()
    message(STATUS "[ACS] : ENABLE_PIE is set to ${ENABLE_PIE}")
endif()

# Check for ENABLE_BTI
if(NOT DEFINED ENABLE_BTI)
    set(ENABLE_BTI ${ENABLE_BTI_DFLT} CACHE INTERNAL "Default ENABLE_BTI value" FORCE)
        message(STATUS "[ACS] : Defaulting ENABLE_BTI to ${ENABLE_BTI}")
else()
    if(NOT ${ENABLE_BTI} IN_LIST ENABLE_BTI_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DENABLE_BTI=, supported values are : ${ENABLE_BTI_LIST}")
    endif()
    message(STATUS "[ACS] : ENABLE_BTI is set to ${ENABLE_BTI}")
endif()

# Check for TARGET_LINUX
if(NOT DEFINED TARGET_LINUX)
    set(TARGET_LINUX ${TARGET_LINUX_DFLT} CACHE INTERNAL "Default TARGET_LINUX value" FORCE)
        message(STATUS "[ACS] : Defaulting TARGET_LINUX to ${TARGET_LINUX}")
else()
    if(NOT ${TARGET_LINUX} IN_LIST TARGET_LINUX_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DTARGET_LINUX=, supported values are : ${TARGET_LINUX_LIST}")
    endif()
    message(STATUS "[ACS] : TARGET_LINUX is set to ${TARGET_LINUX}")
endif()

# Check for XEN_SUPPORT
if(NOT DEFINED XEN_SUPPORT)
    set(XEN_SUPPORT ${XEN_SUPPORT_DFLT} CACHE INTERNAL "Default XEN_SUPPORT value" FORCE)
        message(STATUS "[ACS] : Defaulting XEN_SUPPORT to ${XEN_SUPPORT}")
else()
    if(NOT ${XEN_SUPPORT} IN_LIST XEN_SUPPORT_LIST)
        message(FATAL_ERROR "[ACS] : Error: Unspported value for -DXEN_SUPPORT=, supported values are : ${XEN_SUPPORT_LIST}")
    endif()
    message(STATUS "[ACS] : XEN_SUPPORT is set to ${XEN_SUPPORT}")
    if(${XEN_SUPPORT} EQUAL "1")
        add_definitions(-DXEN_SUPPORT)
    endif()
endif()

if(DEFINED SUITE_TEST_RANGE)
    if(DEFINED SUITE)
        #set(SUITE ${SUITE_DFLT} CACHE INTERNAL "Defaulting suite to ${SUITE}" FORCE)
        if(NOT ${SUITE} STREQUAL "all")
            message(WARNING "[ACS] : SUITE is set to ${SUITE} when SUITE_TEST_RANGE is defined ")
        endif()
        set(SUITE ${SUITE_DFLT} CACHE INTERNAL "Defaulting suite to ${SUITE}" FORCE)
    endif()

    add_definitions(-DSUITE_TEST_RANGE)
    list(LENGTH SUITE_TEST_RANGE SUITE_TEST_RANGE_LENGTH)
    if(${SUITE_TEST_RANGE_LENGTH} GREATER "2")
        message(FATAL_ERROR "[ACS] : -DSUITE_TEST_RANGE=<...> value error! accepts two "
                                             " numbers in quotes separated with ';'")
    endif()
    if(${SUITE_TEST_RANGE_LENGTH} EQUAL "2")
        list(GET SUITE_TEST_RANGE 0 SUITE_TEST_RANGE_MIN)
        list(GET SUITE_TEST_RANGE 1 SUITE_TEST_RANGE_MAX)
        message(STATUS "[ACS] : Testing (${SUITE_TEST_RANGE_MIN}, ${SUITE_TEST_RANGE_MAX}) of ${SUITE} suite")
    endif()
    if(${SUITE_TEST_RANGE_LENGTH} EQUAL "1")
        set(SUITE_TEST_RANGE_MIN ${SUITE_TEST_RANGE})
        set(SUITE_TEST_RANGE_MAX ${SUITE_TEST_RANGE})
        message(STATUS "[ACS] : Testing ${SUITE_TEST_RANGE_MIN} of ${SUITE} suite")
    endif()
    message(STATUS "[ACS] : Test range is (${SUITE_TEST_RANGE_MIN}, ${SUITE_TEST_RANGE_MAX}) of ${SUITE} suite")
    add_definitions(-DSUITE_TEST_RANGE_MIN="${SUITE_TEST_RANGE_MIN}")
    add_definitions(-DSUITE_TEST_RANGE_MAX="${SUITE_TEST_RANGE_MAX}")
    unset(SUITE_TEST_RANGE CACHE)
endif()

# Setup toolchain parameters for compilation and link
include(${ROOT_DIR}/tools/cmake/toolchain/common.cmake)

###

### Add compilation macros ###
set(FFA_V_1_0 10 CACHE INTERNAL "Default FFA_V_1_0 value" FORCE)
set(FFA_V_1_1 11 CACHE INTERNAL "Default FFA_V_1_1 value" FORCE)
set(FFA_V_1_2 12 CACHE INTERNAL "Default FFA_V_1_2 value" FORCE)
set(PLATFORM_FFA_V 00 CACHE INTERNAL "Default PLATFORM_FFA_V value" FORCE)

add_definitions(-DFFA_V_1_0=${FFA_V_1_0})
add_definitions(-DFFA_V_1_1=${FFA_V_1_1})
add_definitions(-DFFA_V_1_2=${FFA_V_1_2})

# Global macro to identify the ACS test suite cmake build
add_definitions(-DCMAKE_BUILD)
add_definitions(-DVERBOSITY=${VERBOSE})
add_definitions(-DPLATFORM_SPMC_EL=${PLATFORM_SPMC_EL})
add_definitions(-DPLATFORM_SP_EL=${PLATFORM_SP_EL})
add_definitions(-DPLATFORM_NS_HYPERVISOR_PRESENT=${PLATFORM_NS_HYPERVISOR_PRESENT})
add_definitions(-DPLATFORM_NS_HYP_MULTI_VM=${PLATFORM_NS_HYP_MULTI_VM})
add_definitions(-DPLATFORM_FFA_V_1_0=${PLATFORM_FFA_V_1_0})
add_definitions(-DPLATFORM_FFA_V_1_1=${PLATFORM_FFA_V_1_1})
add_definitions(-DPLATFORM_FFA_V_1_2=${PLATFORM_FFA_V_1_2})
add_definitions(-DSUITE=${SUITE})

if(${PLATFORM_FFA_V_MULTI} EQUAL 1)
    # Set EP Specific Version Flags
    set(SP1_FFA_V ${FFA_V_1_2})
    set(SP2_FFA_V ${FFA_V_1_1})
    set(SP3_FFA_V ${FFA_V_1_2})
    set(SP4_FFA_V ${FFA_V_1_1})
    set(VM1_FFA_V ${FFA_V_1_2})
    set(VM2_FFA_V ${FFA_V_1_1})
    set(VM3_FFA_V ${FFA_V_1_2})
    add_definitions(-DPLATFORM_FFA_V_MULTI=${PLATFORM_FFA_V_MULTI})
else()
    add_definitions(-DPLATFORM_FFA_V_ALL=${PLATFORM_FFA_V_ALL})
    if(${PLATFORM_FFA_V_1_0} EQUAL 1)
        add_definitions(-DPLATFORM_FFA_V=${FFA_V_1_0})
        message(STATUS "[ACS] : PLATFORM_FFA_V is set to ${FFA_V_1_0}")
    elseif(${PLATFORM_FFA_V_1_1} EQUAL 1)
        add_definitions(-DPLATFORM_FFA_V=${FFA_V_1_1})
        message(STATUS "[ACS] : PLATFORM_FFA_V is set to ${FFA_V_1_1}")
    else()
        add_definitions(-DPLATFORM_FFA_V=${FFA_V_1_2})
        message(STATUS "[ACS] : PLATFORM_FFA_V is set to ${FFA_V_1_2}")
    endif()
endif()
####

### Cmake clean target ###
list(APPEND CLEAN_LIST
        ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_HEADER}
        ${CMAKE_CURRENT_BINARY_DIR}/output
)

# Include the files for make clean
foreach(clean_item ${CLEAN_LIST})
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${clean_item})
endforeach()
###

set(MANIFEST_PATHS ${ROOT_DIR}/platform/manifest/tgt_tfa_fvp/v12/)

# Ensure Python is available
find_program(PYTHON_EXECUTABLE python3)

if(NOT PYTHON_EXECUTABLE)
    message(FATAL_ERROR "Python3 is required but was not found on your system.")
endif()

# Define and create output directory
set(FFA_GEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${FFA_GEN_OUTPUT_DIR}")

# Run Python script at configure time
execute_process(
    COMMAND ${PYTHON_EXECUTABLE}
            ${ROOT_DIR}/tools/python/ffa_endpoints.py
            ${MANIFEST_PATHS} ${PLATFORM_SP_EL} ${FFA_GEN_OUTPUT_DIR}
    RESULT_VARIABLE gen_result
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

if(NOT gen_result EQUAL 0)
    message(FATAL_ERROR "Failed to generate FFA endpoints.")
endif()

if(NOT(${PLATFORM_SP_EL} STREQUAL -1))
add_subdirectory(${ROOT_DIR}/tools/cmake/endpoints/sp1)
add_subdirectory(${ROOT_DIR}/tools/cmake/endpoints/sp2)
add_subdirectory(${ROOT_DIR}/tools/cmake/endpoints/sp3)
add_subdirectory(${ROOT_DIR}/tools/cmake/endpoints/sp4)
endif()
add_subdirectory(${ROOT_DIR}/tools/cmake/endpoints/vm1)

if(${PLATFORM_NS_HYP_MULTI_VM} STREQUAL 1)
add_subdirectory(${ROOT_DIR}/tools/cmake/endpoints/vm2)
add_subdirectory(${ROOT_DIR}/tools/cmake/endpoints/vm3)
endif()

if((${PLATFORM_NS_HYPERVISOR_PRESENT} STREQUAL 1) AND (${PLATFORM_SP_EL} STREQUAL -1))
    message(FATAL_ERROR "[ACS] : Invalid combination PLATFORM_NS_HYPERVISOR_PRESENT=0 and PLATFORM_SP_EL=-1")
endif()
