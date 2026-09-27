set(CMAKE_SYSTEM_NAME Generic)  # No OS，Generic
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX "arm-none-eabi-")

# Toolchain location comes from the SDK_TOOLCHAIN_PATH environment variable.
if(DEFINED ENV{SDK_TOOLCHAIN_PATH} AND NOT "$ENV{SDK_TOOLCHAIN_PATH}" STREQUAL "")
    set(SDK_TOOLCHAIN_PATH "$ENV{SDK_TOOLCHAIN_PATH}")
else()
    # Default  path
    set(SDK_TOOLCHAIN_PATH "${CMAKE_CURRENT_LIST_DIR}/../../toolchain/arm-none-eabi-gcc/bin")
endif()

# Get the absolute path of the toolchain directory
get_filename_component(SDK_TOOLCHAIN_PATH "${SDK_TOOLCHAIN_PATH}" ABSOLUTE)

if(NOT EXISTS "${SDK_TOOLCHAIN_PATH}/arm-none-eabi-gcc.exe")
    message(FATAL_ERROR "ARM GCC toolchain not found at: ${SDK_TOOLCHAIN_PATH} (set SDK_TOOLCHAIN_PATH env var)")
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(TOOLCHAIN_EXT ".exe")
else()
    set(TOOLCHAIN_EXT "")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_ASM_COMPILER  "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_EXT}")
set(CMAKE_C_COMPILER    "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_EXT}")
set(CMAKE_CXX_COMPILER  "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}g++${TOOLCHAIN_EXT}")
set(CMAKE_SIZE          "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}size${TOOLCHAIN_EXT}")
set(CMAKE_OBJCOPY       "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}objcopy${TOOLCHAIN_EXT}")
set(CMAKE_AR            "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}ar${TOOLCHAIN_EXT}")
set(CMAKE_RANLIB        "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}ranlib${TOOLCHAIN_EXT}")
# set(CMAKE_OBJDUMP       "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}objdump${TOOLCHAIN_EXT}")
# set(CMAKE_NM_UTIL       "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc-nm${TOOLCHAIN_EXT}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_COMMON_FLAGS "-Wall -Wextra -Wshadow -Wpedantic -Wdouble-promotion")
set(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -ffunction-sections -fdata-sections -fstack-usage")
set(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -Wno-unused-parameter -Wno-unused-function")
set(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -mno-unaligned-access -fno-builtin")

set(CMAKE_C_FLAGS_INIT   "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -fno-rtti -fno-exceptions -fno-threadsafe-statics")
set(CMAKE_ASM_FLAGS_INIT "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS} -x assembler-with-cpp")

set(CMAKE_EXE_LINKER_FLAGS_INIT "${MCPU_FLAGS} ${VFP_FLAGS} --specs=nano.specs -nostartfiles")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,--print-memory-usage")
# Silence the benign RWX warning: .init_array/.fini_array (WA) share segment 00 with
# .text in FLASH, which binutils >= 2.39 flags as RWE. Flash is not writable on Cortex-M.
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,--no-warn-rwx-segments")

set(CMAKE_C_FLAGS_DEBUG_INIT "-Og -g -ggdb3")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Og -g -ggdb3")
set(CMAKE_ASM_FLAGS_DEBUG_INIT "-g -ggdb3")

set(CMAKE_C_FLAGS_RELEASE_INIT "-O2")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2")
set(CMAKE_ASM_FLAGS_RELEASE_INIT "")
