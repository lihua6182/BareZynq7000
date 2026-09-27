function(function_gcc_arm_gen_binary TARGET)
    if(NOT DEFINED SDK_LINKER_SCRIPT OR "${SDK_LINKER_SCRIPT}" STREQUAL "")
        message(FATAL_ERROR "SDK_LINKER_SCRIPT is not set. The selected chip must provide a linker script.")
    endif()
    if(NOT EXISTS "${SDK_LINKER_SCRIPT}")
        message(FATAL_ERROR "Linker script not found: ${SDK_LINKER_SCRIPT}")
    endif()

    set_target_properties("${TARGET}" PROPERTIES
        LINK_DEPENDS "${SDK_LINKER_SCRIPT}"
        SUFFIX ".elf"
    )
    target_link_options("${TARGET}" PRIVATE
        "-T${SDK_LINKER_SCRIPT}"
        "-Wl,-Map=$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.map"
    )
    add_custom_command(TARGET "${TARGET}" POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} "-O" "binary"
            "$<TARGET_FILE:${TARGET}>"
            "$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.bin"
        COMMAND ${CMAKE_OBJCOPY} "-O" "ihex"
            "$<TARGET_FILE:${TARGET}>"
            "$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.hex"
        COMMENT "Generating ${TARGET}.bin/.hex"
    )
endfunction()
