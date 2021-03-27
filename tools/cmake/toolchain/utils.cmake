#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Function to get all the folders inside given parent directory
function(_get_sub_dir_list result parent_dir)
        file(GLOB parent_dir_items RELATIVE ${parent_dir} ${parent_dir}/*)
        set(dir_list "")
        foreach(item ${parent_dir_items})
                if(IS_DIRECTORY ${parent_dir}/${item})
                        list(APPEND dir_list ${item})
                endif()
        endforeach()
        set(${result} ${dir_list} PARENT_SCOPE)
endfunction(_get_sub_dir_list)
