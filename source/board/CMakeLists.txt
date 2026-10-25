add_library(board INTERFACE)

# board_usb.c is intentionally left out until the USB device stack is linked.

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

# board_pl_bitstream.c is pre-generated from bitstream.bin by running the VSCode
# task "Convert Bitstream (ZBLZ pack)" (tools/convert_bitstream.py); the build
# itself never packs the bitstream. CMake only picks the file up at configure
# time, so re-run the configure step after (re)generating it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/board_pl_bitstream.c")
    target_sources(board INTERFACE board_pl_bitstream.c)
endif()
