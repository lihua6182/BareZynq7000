set(MCPU_FLAGS "-mcpu=cortex-m3")
set(VFP_FLAGS "-mfloat-abi=soft")

include(${CMAKE_CURRENT_LIST_DIR}/gcc-arm-toolchain.cmake)
