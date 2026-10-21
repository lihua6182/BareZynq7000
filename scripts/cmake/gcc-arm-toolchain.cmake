set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX "arm-none-eabi-")

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(TOOLCHAIN_EXT ".exe")
else()
    set(TOOLCHAIN_EXT "")
endif()

if(DEFINED ENV{SDK_TOOLCHAIN_PATH} AND NOT "$ENV{SDK_TOOLCHAIN_PATH}" STREQUAL "")
    set(SDK_TOOLCHAIN_PATH "$ENV{SDK_TOOLCHAIN_PATH}")
else()
    set(SDK_TOOLCHAIN_PATH "${CMAKE_CURRENT_LIST_DIR}/../../toolchain/arm-none-eabi-gcc/bin")
endif()

get_filename_component(SDK_TOOLCHAIN_PATH "${SDK_TOOLCHAIN_PATH}" ABSOLUTE)

if(NOT EXISTS "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_EXT}")
    message(FATAL_ERROR "ARM GCC toolchain not found at: ${SDK_TOOLCHAIN_PATH} (set SDK_TOOLCHAIN_PATH env var)")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_ASM_COMPILER  "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_EXT}")
set(CMAKE_C_COMPILER    "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_EXT}")
set(CMAKE_SIZE          "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}size${TOOLCHAIN_EXT}")
set(CMAKE_OBJCOPY       "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}objcopy${TOOLCHAIN_EXT}")
set(CMAKE_AR            "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}ar${TOOLCHAIN_EXT}")
set(CMAKE_RANLIB        "${SDK_TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}ranlib${TOOLCHAIN_EXT}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_COMMON_FLAGS "-Wall -Wextra -Wshadow -Wpedantic -Wdouble-promotion")
set(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -ffunction-sections -fdata-sections -fstack-usage")
set(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -Wno-unused-parameter -Wno-unused-function")
set(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -mno-unaligned-access -fno-builtin")

set(CMAKE_C_FLAGS_INIT   "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${MCPU_FLAGS} ${VFP_FLAGS} ${CMAKE_COMMON_FLAGS} -x assembler-with-cpp")

set(CMAKE_EXE_LINKER_FLAGS_INIT "${MCPU_FLAGS} ${VFP_FLAGS} --specs=nano.specs -nostartfiles")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,--print-memory-usage")

set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,--no-warn-rwx-segments")

set(CMAKE_C_FLAGS_DEBUG     "-Og -g -ggdb3"           CACHE STRING "C compiler flags, debug")
set(CMAKE_C_FLAGS_RELEASE   "-O2 -g -ggdb3 -DNDEBUG"  CACHE STRING "C compiler flags, release")
set(CMAKE_ASM_FLAGS_DEBUG   "-g -ggdb3"               CACHE STRING "Assembler flags, debug")
set(CMAKE_ASM_FLAGS_RELEASE "-g -ggdb3 -DNDEBUG"      CACHE STRING "Assembler flags, release")
