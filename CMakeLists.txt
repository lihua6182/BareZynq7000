cmake_minimum_required(VERSION 4.4)

project(BareZynq7000 C ASM)

include("${CMAKE_SOURCE_DIR}/scripts/cmake/function_gcc_arm_gen_binary.cmake")
include("${CMAKE_SOURCE_DIR}/scripts/cmake/function_validate_boot_header.cmake")

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "No build type specified; using Debug")
endif()

add_subdirectory(source)
