# Packs the PL bitstream and embeds it into the target's XIP flash image.
#
# The file is designated by SDK_BITSTREAM, mirroring how SDK_LINKER_SCRIPT
# designates the linker script (set it next to SDK_LINKER_SCRIPT, typically
# to a file under scripts/bitstream/):
#
#   unset / empty  -> the target is built without a bitstream;
#                     zynq_devcfg_boot() reports ZYNQ_DEVCFG_NO_IMAGE
#   set, missing   -> hard error, so a typo cannot silently drop the image
#   set, existing  -> packed by tools/rle_bitstream.py (byte-order fixup +
#                     SEGGER-style RLE + CRC header) and embedded as the
#                     .bitstream section; the linker script places it via
#                     __bitstream_start__/__bitstream_end__
#
# Must be called after the target exists, before function_gcc_arm_gen_binary
# (the embedded image is part of the payload the boot header describes).
function(function_gcc_arm_bitstream TARGET)
    if(NOT DEFINED SDK_BITSTREAM OR "${SDK_BITSTREAM}" STREQUAL "")
        message(STATUS "SDK_BITSTREAM is not set: building ${TARGET} without a PL bitstream")
        return()
    endif()
    if(NOT EXISTS "${SDK_BITSTREAM}")
        message(FATAL_ERROR "SDK_BITSTREAM is set but the file is missing: ${SDK_BITSTREAM}")
    endif()

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    set(BITSTREAM_BLOB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_bitstream.blob")
    set(BITSTREAM_OBJ  "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_bitstream.blob.o")

    add_custom_command(
        OUTPUT  "${BITSTREAM_BLOB}"
        COMMAND ${Python3_EXECUTABLE}
                "${CMAKE_SOURCE_DIR}/tools/rle_bitstream.py"
                "${SDK_BITSTREAM}"
                "${BITSTREAM_BLOB}"
        DEPENDS "${SDK_BITSTREAM}" "${CMAKE_SOURCE_DIR}/tools/rle_bitstream.py"
        COMMENT "Packing PL bitstream ${SDK_BITSTREAM}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT  "${BITSTREAM_OBJ}"
        COMMAND ${CMAKE_OBJCOPY}
                -I binary
                -O elf32-littlearm
                --rename-section .data=.bitstream,alloc,load,readonly,data,contents
                "${BITSTREAM_BLOB}"
                "${BITSTREAM_OBJ}"
        DEPENDS "${BITSTREAM_BLOB}"
        COMMENT "Embedding packed PL bitstream into ${TARGET}"
        VERBATIM
    )

    target_sources("${TARGET}" PRIVATE "${BITSTREAM_OBJ}")
    set_source_files_properties("${BITSTREAM_OBJ}" PROPERTIES
        EXTERNAL_OBJECT TRUE
        GENERATED TRUE
    )
endfunction()
