add_executable(usb_cdc_speed_test)

target_sources(usb_cdc_speed_test
    PRIVATE
    main.c
    ../../app/syscalls.c
    ../../app/sysmem.c
)

target_compile_features(usb_cdc_speed_test PRIVATE c_std_23)

target_link_libraries(usb_cdc_speed_test
    PRIVATE
    board
    cherryusb
)

function_gcc_arm_gen_binary(
    usb_cdc_speed_test
    "${PROJECT_SOURCE_DIR}/scripts/linker/flash_xip.ld"
)
function_validate_boot_header(usb_cdc_speed_test)
