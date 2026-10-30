add_executable(app)

target_sources(app
    PRIVATE
    main.c
    syscalls.c
    sysmem.c
)

target_compile_features(app PRIVATE c_std_23)

target_link_libraries(app
    PRIVATE
    board
)

set_source_files_properties(
    ../middleware/lzma/sdk/LzmaDec.c
    PROPERTIES
    COMPILE_OPTIONS -Wno-pedantic
)

function_gcc_arm_gen_binary(
    app
    "${PROJECT_SOURCE_DIR}/scripts/linker/flash_xip.ld"
)
function_validate_boot_header(app)
