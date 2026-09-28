project("app" C ASM)

add_executable("app"
    "main.c"
    "syscalls.c"
    "sysmem.c"
)

target_link_libraries("app"
    PRIVATE
    "drivers"
)

target_link_options("app"
    PRIVATE
)

# Chip assets, specified the same way the rest of the SDK is: SDK_* variables
# consumed by the functions in scripts/cmake/.
set(SDK_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/linker/flash_xip.ld")
set(SDK_BITSTREAM     "${CMAKE_SOURCE_DIR}/scripts/bitstream/bitstream.bin")

function_gcc_arm_bitstream("app")
function_gcc_arm_gen_binary("app")
function_validate_boot_header("app")
