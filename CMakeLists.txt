cmake_minimum_required(VERSION 3.24)

include("${CMAKE_SOURCE_DIR}/scripts/cmake/function_gcc_arm_gen_binary.cmake")
include("${CMAKE_SOURCE_DIR}/scripts/cmake/function_validate_boot_header.cmake")

MESSAGE(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "No build type specified. Defaulting to Debug.")
endif()

# Project name
project("BareZynq7000" C ASM)

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Add source files
add_subdirectory("source")
