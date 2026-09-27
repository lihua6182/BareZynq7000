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

set(SDK_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/linker/flash_xip.ld")

function_gcc_arm_gen_binary("app")

add_custom_command(TARGET "app" POST_BUILD
    COMMAND ${Python3_EXECUTABLE}
        "${CMAKE_SOURCE_DIR}/scripts/validate_boot_header.py"
        "$<TARGET_FILE_DIR:app>/app.bin"
    COMMENT "Validating BootROM header of app.bin"
    VERBATIM
)
