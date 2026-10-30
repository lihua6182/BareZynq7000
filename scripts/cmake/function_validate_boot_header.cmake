find_program(BOOT_HEADER_PYTHON NAMES py python3 python)
if(NOT BOOT_HEADER_PYTHON)
    message(FATAL_ERROR "Python (py/python3) not found: required to validate the boot header")
endif()

set(BOOT_HEADER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/../../tools/validate_boot_header.py")
get_filename_component(BOOT_HEADER_SCRIPT "${BOOT_HEADER_SCRIPT}" ABSOLUTE)

function(function_validate_boot_header target)
    add_custom_command(TARGET "${target}" POST_BUILD
        COMMAND "${BOOT_HEADER_PYTHON}" "${BOOT_HEADER_SCRIPT}"
            "$<TARGET_FILE_DIR:${target}>/${target}.bin"
        COMMENT "Validating BootROM header of ${target}.bin"
        VERBATIM
    )
endfunction()
