project("app" C ASM)

add_executable("app")

set(SDK_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/linker/flash_xip.ld")

target_sources("app"
    PRIVATE
    "main.c"
    "qspi_probe.c"
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

# Ground truth for qspi_probe: the flash is programmed from app.hex, whose
# gaps stay erased (0xFF) - print the CRC of the rendered hex image.
add_custom_command(TARGET "app" POST_BUILD
    COMMAND python "${CMAKE_SOURCE_DIR}/tools/crc32.py"
            "$<TARGET_FILE_DIR:app>/app.hex"
            "$<TARGET_FILE_DIR:app>/app.bin"
    COMMENT "crc32 of rendered app.hex (qspi_probe ground truth)"
)
