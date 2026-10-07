project("app" C ASM)

add_executable("app")

set(SDK_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/linker/flash_xip.ld")

target_sources("app"
    PRIVATE
    "main.c"
    "syscalls.c"
    "sysmem.c"
)

target_link_libraries("app"
    PRIVATE
    "board"
    "driver"
    "cherryusb"
)

function_gcc_arm_gen_binary("app")
function_validate_boot_header("app")
