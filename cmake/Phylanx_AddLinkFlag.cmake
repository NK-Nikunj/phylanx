# Copyright (c) 2011 Bryce Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

include(CMakeParseArguments)

macro(phylanx_add_link_flag FLAG)
  set(options)
  set(one_value_args)
  set(multi_value_args TARGETS CONFIGURATIONS)
  cmake_parse_arguments(PHYLANX_ADD_LINK_FLAG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  set(_targets EXE SHARED STATIC)
  if(PHYLANX_ADD_LINK_FLAG_TARGETS)
    set(_targets ${PHYLANX_ADD_LINK_FLAG_TARGETS})
  endif()

  set(_configurations "none")
  if(PHYLANX_REMOVE_LINK_FLAG_CONFIGURATIONS)
    set(_configurations "${PHYLANX_REMOVE_LINK_FLAG_CONFIGURATIONS}")
  endif()

  foreach(_config ${_configurations})
    set(_conf)
    if(NOT _config STREQUAL "none")
      string(TOUPPER "${_config}" _conf)
      set(_conf "_${_conf}")
    endif()

    foreach(_target ${_targets})
      set(CMAKE_${_target}_LINKER_FLAGS${_conf}
        "${CMAKE_${_target}_LINKER_FLAGS${_conf}} ${FLAG}")
    endforeach()
  endforeach()
endmacro()

macro(phylanx_remove_link_flag FLAG)
  set(options)
  set(one_value_args)
  set(multi_value_args TARGETS CONFIGURATIONS)
  cmake_parse_arguments(PHYLANX_REMOVE_LINK_FLAG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  set(_targets EXE SHARED STATIC)
  if(PHYLANX_ADD_LINK_FLAG_TARGETS)
    set(_targets ${PHYLANX_ADD_LINK_FLAG_TARGETS})
  endif()

  set(_configurations "none")
  if(PHYLANX_REMOVE_LINK_FLAG_CONFIGURATIONS)
    set(_configurations "${PHYLANX_REMOVE_LINK_FLAG_CONFIGURATIONS}")
  endif()

  foreach(_config ${_configurations})
    set(_conf)
    if(NOT _config STREQUAL "none")
      string(TOUPPER "${_config}" _conf)
      set(_conf "_${_conf}")
    endif()

    foreach(_target ${_targets})
      STRING (REGEX REPLACE "${FLAG}" ""
          CMAKE_${_target}_LINKER_FLAGS${_conf}
          "${CMAKE_${_target}_LINKER_FLAGS${_conf}}")
    endforeach()
  endforeach()
endmacro()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

macro(phylanx_add_link_flag_if_available FLAG)
  set(options)
  set(one_value_args NAME)
  set(multi_value_args TARGETS)
  cmake_parse_arguments(PHYLANX_ADD_LINK_FLAG_IA "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  if(PHYLANX_ADD_LINK_FLAG_IA_NAME)
    string(TOUPPER ${PHYLANX_ADD_LINK_FLAG_IA_NAME} _name)
  else()
    string(TOUPPER ${FLAG} _name)
  endif()

  string(REGEX REPLACE " " "" _name ${_name})
  string(REGEX REPLACE "^-+" "" _name ${_name})
  string(REGEX REPLACE "[=\\-]" "_" _name ${_name})
  string(REGEX REPLACE "," "_" _name ${_name})
  string(REGEX REPLACE "\\+" "X" _name ${_name})

  check_cxx_compiler_flag("${FLAG}" WITH_LINKER_FLAG_${_name})
  if(WITH_LINKER_FLAG_${_name})
    phylanx_add_link_flag(${FLAG} TARGETS ${PHYLANX_ADD_LINK_FLAG_IA_TARGETS})
  else()
    phylanx_info("Linker \"${FLAG}\" not available.")
  endif()

endmacro()
