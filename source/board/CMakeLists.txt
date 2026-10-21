
add_library(board INTERFACE)

target_sources(board
    INTERFACE
    board.c
    board_pl.c
)

target_include_directories(board
    INTERFACE
    .
)

target_link_libraries(board
    INTERFACE
    soc
    driver
    lzma
)

function(board_add_pl_image target)
    find_program(PYTHON_EXECUTABLE NAMES py python3 python)

    set(bitstream "${PROJECT_SOURCE_DIR}/source/board/bitstream.bin")
    if(ARGC GREATER 1)
        set(bitstream "${ARGV1}")
    endif()

    set(image "${CMAKE_CURRENT_BINARY_DIR}/board_pl_bitstream.c")
    set(header "${CMAKE_CURRENT_BINARY_DIR}/board_pl_bitstream.h")

    add_custom_command(
        OUTPUT "${image}" "${header}"
        COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/convert_bitstream.py" "${bitstream}" "${image}"
                "${header}"
        DEPENDS "${bitstream}" "${PROJECT_SOURCE_DIR}/tools/convert_bitstream.py"
        COMMENT "Packing PL bitstream"
        VERBATIM
    )

    target_sources(${target} PRIVATE "${image}")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()
