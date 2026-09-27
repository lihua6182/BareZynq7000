set(MCPU_FLAGS "-mcpu=cortex-a9 -marm")
set(VFP_FLAGS "-mfloat-abi=hard -mfpu=neon")

include(${CMAKE_CURRENT_LIST_DIR}/gcc-arm-toolchain.cmake)
