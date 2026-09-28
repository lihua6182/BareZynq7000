# BootROM header validation for raw Zynq-7000 QSPI XIP boot images.
#
# Dual-mode CMake file:
#   - Included from CMakeLists (configure time): defines
#     function_validate_boot_header(<target>), which registers a POST_BUILD
#     step. Call it AFTER function_gcc_arm_gen_binary so the .bin exists.
#   - Run with `cmake -DBIN_FILE=<image.bin> -P <this file>` (build time,
#     from that POST_BUILD step): performs the check. No Python needed.
#
# Checks the fields of the 0x8C0-byte boot header (UG585 ch. 6, "BootROM
# Header Parameters") against the rules the BootROM enforces before it hands
# control to the image:
#
#   - width detection word / image signature / QSPI config word
#   - source offset: >= 0x8C0 and 64-byte aligned
#   - execute-in-place selection: image length and total length both 0
#   - XIP start of execution: absolute address inside the 16 MB QSPI linear
#     window, at or past the header, 64-byte aligned
#   - header checksum: one's complement of the sum of words 0x20..0x44

set(FUNCTION_VALIDATE_BOOT_HEADER_FILE "${CMAKE_CURRENT_LIST_DIR}/function_validate_boot_header.cmake")

# boot_header_validate(<image.bin>): run all checks, fail the build with the
# expected value(s) on any mismatch.
function(boot_header_validate bin_file)
    if(NOT EXISTS "${bin_file}")
        message(FATAL_ERROR "Boot header check: image not found: ${bin_file}")
    endif()

    file(READ "${bin_file}" image HEX)
    string(TOUPPER "${image}" image)
    string(LENGTH "${image}" hexlen)
    math(EXPR size "${hexlen} / 2")
    math(EXPR header_size "0x8C0")
    math(EXPR exec_lo "0xFC000000 + 0x8C0")
    math(EXPR exec_hi "0xFC000000 + 16 * 1024 * 1024")

    set(errors "")

    # word_le(<out> <uppercase hex image> <byte offset>): little-endian 32-bit
    # word of the image, as 8 uppercase hex digits (byte pairs reversed).
    function(word_le out hex off)
        math(EXPR pos "2 * ${off}") # hex chars are 2 per byte
        string(SUBSTRING "${hex}" ${pos} 8 b)
        string(SUBSTRING "${b}" 6 2 b0)
        string(SUBSTRING "${b}" 4 2 b1)
        string(SUBSTRING "${b}" 2 2 b2)
        string(SUBSTRING "${b}" 0 2 b3)
        set(${out} "${b0}${b1}${b2}${b3}" PARENT_SCOPE)
    endfunction()

    if(size LESS header_size)
        list(APPEND errors "image is ${size} bytes, smaller than the 0x8C0 boot header")
    else()
        word_le(w20 "${image}" 0x20)
        word_le(w24 "${image}" 0x24)
        word_le(w28 "${image}" 0x28)
        word_le(w30 "${image}" 0x30)
        word_le(w34 "${image}" 0x34)
        word_le(w3c "${image}" 0x3C)
        word_le(w40 "${image}" 0x40)
        word_le(w44 "${image}" 0x44)
        word_le(w48 "${image}" 0x48)

        if(NOT w20 STREQUAL "AA995566")
            list(APPEND errors "width detection word is 0x${w20}, expected 0xAA995566")
        endif()
        if(NOT w24 STREQUAL "584C4E58")
            list(APPEND errors "image signature is 0x${w24}, expected 0x584C4E58 ('XLNX')")
        endif()
        if(w28 STREQUAL "A5C3C5A3" OR w28 STREQUAL "3A5C3C5A")
            list(APPEND errors "encryption status selects an encrypted image; only plain images are supported here")
        endif()
        if(NOT w44 STREQUAL "00000001")
            list(APPEND errors "QSPI config word is 0x${w44}, expected 0x00000001")
        endif()

        math(EXPR source_offset "0x${w30}")
        math(EXPR image_length "0x${w34}")
        math(EXPR exec_address "0x${w3c}")
        math(EXPR total_length "0x${w40}")

        if(source_offset LESS header_size)
            list(APPEND errors "source offset 0x${w30} points inside the boot header, must be >= 0x8C0")
        endif()
        math(EXPR source_offset_mod64 "0x${w30} & 0x3F")
        if(source_offset_mod64 GREATER 0)
            list(APPEND errors "source offset 0x${w30} is not 64-byte aligned")
        endif()

        if(image_length EQUAL 0)
            # Execute-in-place (UG585 Table 6-5, note 6)
            if(NOT total_length EQUAL 0)
                list(APPEND errors "XIP image: total image length must be 0 when the image length is 0")
            endif()
            if(exec_address LESS exec_lo OR exec_address GREATER_EQUAL exec_hi)
                list(APPEND errors "XIP start of execution 0x${w3c} is outside the QSPI linear window [0xFC0008C0, 0xFD000000)")
            endif()
            math(EXPR exec_address_mod64 "0x${w3c} & 0x3F")
            if(exec_address_mod64 GREATER 0)
                list(APPEND errors "start of execution 0x${w3c} is not 64-byte aligned")
            endif()
        elseif(NOT total_length EQUAL image_length)
            list(APPEND errors "non-secure boot: total image length must equal the image length")
        endif()

        set(sum 0)
        foreach(off RANGE 32 68 4) # words 0x20..0x44
            word_le(w "${image}" ${off})
            math(EXPR sum "${sum} + 0x${w}")
        endforeach()
        math(EXPR expected "(~${sum}) & 0xFFFFFFFF" OUTPUT_FORMAT HEXADECIMAL)
        string(TOUPPER "${expected}" expected)
        string(REGEX REPLACE "^0X" "" expected "${expected}")
        string(LENGTH "${expected}" expected_len)
        while(expected_len LESS 8)
            string(PREPEND expected "0")
            math(EXPR expected_len "${expected_len} + 1")
        endwhile()
        if(NOT w48 STREQUAL expected)
            list(APPEND errors "header checksum is 0x${w48}, expected 0x${expected}")
        endif()
    endif()

    if(errors)
        foreach(e IN LISTS errors)
            message(SEND_ERROR "error: ${e}")
        endforeach()
        message(FATAL_ERROR "BootROM header check failed")
    endif()

    message(STATUS "BootROM header OK (${bin_file}, ${size} bytes)")
endfunction()

# Register the POST_BUILD validation of <target>'s raw binary. Must be
# called after function_gcc_arm_gen_binary(<target>).
function(function_validate_boot_header target)
    add_custom_command(TARGET "${target}" POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DBIN_FILE=$<TARGET_FILE_DIR:${target}>/${target}.bin
            -P "${FUNCTION_VALIDATE_BOOT_HEADER_FILE}"
        COMMENT "Validating BootROM header of ${target}.bin"
        VERBATIM
    )
endfunction()

# Script-mode entry point: cmake -DBIN_FILE=<image.bin> -P <this file>
if(CMAKE_SCRIPT_MODE_FILE)
    if(NOT DEFINED BIN_FILE)
        message(FATAL_ERROR "Usage: cmake -DBIN_FILE=<image.bin> -P ${CMAKE_SCRIPT_MODE_FILE}")
    endif()
    boot_header_validate("${BIN_FILE}")
endif()
