function(function_gcc_arm_gen_binary TARGET LINKER_SCRIPT)
    if(NOT DEFINED LINKER_SCRIPT OR "${LINKER_SCRIPT}" STREQUAL "")
        message(FATAL_ERROR "Linker script is not set for target ${TARGET}")
    endif()
    if(NOT EXISTS "${LINKER_SCRIPT}")
        message(FATAL_ERROR "Linker script not found: ${LINKER_SCRIPT}")
    endif()

    set_target_properties("${TARGET}" PROPERTIES
        LINK_DEPENDS "${LINKER_SCRIPT}"
        SUFFIX ".elf"
    )
    target_link_options("${TARGET}" PRIVATE
        "-T${LINKER_SCRIPT}"
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
