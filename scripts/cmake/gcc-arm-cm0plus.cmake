set(MCPU_FLAGS "-mcpu=cortex-m0plus")
set(VFP_FLAGS "-mfloat-abi=soft")

include(${CMAKE_CURRENT_LIST_DIR}/gcc-arm-toolchain.cmake)
