#-------------------------------------------------------------------------------
# Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(_LINKER_CMAKE_LOADED)
  return()
endif()
set(_LINKER_CMAKE_LOADED TRUE)

set(GNUARM_LINKER "${CROSS_COMPILE}ld" CACHE FILEPATH "The GNUARM linker" FORCE)
set(GNUARM_OBJCOPY "${CROSS_COMPILE}objcopy" CACHE FILEPATH "The GNUARM objcopy" FORCE)
set(GNUARM_OBJDUMP "${CROSS_COMPILE}objdump" CACHE FILEPATH "The GNUARM objdump" FORCE)

if(${ENABLE_PIE})
    set(LINKER_PIE_SWITCH "-pie" "--no-dynamic-linker")
else()
    set(LINKER_PIE_SWITCH "")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(LINKER_DEBUG_OPTIONS "-g")
else()
    set(LINKER_DEBUG_OPTIONS "")
endif()

set(GNUARM_LINKER_FLAGS "--fatal-warnings"  ${LINKER_PIE_SWITCH} ${LINKER_DEBUG_OPTIONS} "-O1" "--gc-sections" "--build-id=none")
set(GNUARM_OBJDUMP_FLAGS    "-dSx")
set(GNUARM_OBJCOPY_FLAGS    "-Obinary")

function (create_executable EXE_NAME)
    set(SCATTER_INPUT_FILE "${ROOT_DIR}/tools/cmake/endpoints/${EXE_NAME}/image.ld.S")
    set(SCATTER_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/image.ld")

    # Preprocess the scatter file for image layout symbols
    add_custom_command(OUTPUT Process-linker-script--${EXE_NAME}
                    COMMAND ${CROSS_COMPILE}gcc -E -P -I${ROOT_DIR}/val/inc -I${ROOT_DIR}/platform/pal_baremetal/${TARGET}/inc ${SCATTER_INPUT_FILE} -o ${SCATTER_OUTPUT_FILE} -DPLATFORM_NS_HYPERVISOR_PRESENT=${PLATFORM_NS_HYPERVISOR_PRESENT} -DCMAKE_BUILD={CMAKE_BUILD}
                    DEPENDS ${VAL_LIB} ${PAL_LIB} ${TEST_LIB})
    add_custom_target(Process-linker-script-${EXE_NAME} ALL DEPENDS Process-linker-script--${EXE_NAME})

    # Link the objects
    add_custom_command(
    OUTPUT ${EXE_NAME}.elf
    COMMAND ${GNUARM_LINKER} ${CMAKE_LINKER_FLAGS} ${GNUARM_LINKER_FLAGS}
            -T ${SCATTER_OUTPUT_FILE}
            -o ${EXE_NAME}.elf
            --start-group
                ${PAL_LIB}.a ${VAL_LIB}.a ${TEST_LIB}.a
            --end-group
            ${PAL_OBJ_LIST}
    DEPENDS Process-linker-script-${EXE_NAME})
    add_custom_target(${EXE_NAME}_elf ALL DEPENDS ${EXE_NAME}.elf)

    # Create the dump info
    add_custom_command(OUTPUT ${EXE_NAME}.dump
                    COMMAND ${GNUARM_OBJDUMP} ${GNUARM_OBJDUMP_FLAGS} ${EXE_NAME}.elf > ${EXE_NAME}.dump
                    DEPENDS ${EXE_NAME}_elf)
    add_custom_target(${EXE_NAME}_dump ALL DEPENDS ${EXE_NAME}.dump)

    # Create the binary
    add_custom_command(OUTPUT ${EXE_NAME}.bin
                    COMMAND ${GNUARM_OBJCOPY} ${GNUARM_OBJCOPY_FLAGS} ${EXE_NAME}.elf ${EXE_NAME}.bin
                    DEPENDS ${EXE_NAME}_dump)
    add_custom_target(${EXE_NAME}_bin ALL DEPENDS ${EXE_NAME}.bin)

    # Copy the binary to common output dir
    add_custom_target(${EXE_NAME}_copy_bin_file ALL DEPENDS ${EXE_NAME}_copy_bin)
    add_custom_command(OUTPUT ${EXE_NAME}_copy_bin
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.bin
                    ${BUILD}/output/
            DEPENDS ${EXE_NAME}_bin)
endfunction()
