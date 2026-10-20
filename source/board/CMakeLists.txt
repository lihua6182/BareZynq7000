project("board" C)

# Board stage: sits between the soc stage (soc_init) and the
# application.
#
#   board.c                 board_init(): PL, fabric clock, PL resets;
#                           console/LED pin mux and the newlib console binding
#   board_usb.c             USB0 board wiring: ULPI pin mux, USB3300 PHY
#                           reset, clock routing, GIC - implements the
#                           CherryUSB usb_dc_low_level_init()/deinit() hooks
#   board_pl.c/h          the PL itself: DevC/PCAP configuration plus
#                           the control plane (resets, level shifters,
#                           status) - code, hand-written
#   board_pl_bitstream.*  the PL image - generated data, produced by
#                           the VSCode task "Convert PL bitstream"
#                           (tools/convert_bitstream.py) from bitstream.bin
#                           in this folder
#
# The image and the code that loads it are deliberately separate files:
# a bitstream swap regenerates only the data pair. The app target
# compiles every source; this subdirectory only exports the include path.
add_library("board" INTERFACE)

target_sources("board"
    INTERFACE
    "board.c"
    "board_usb.c"
    "board_pl.c"
    "board_pl_bitstream.c"
)

target_include_directories("board"
    INTERFACE
    "."
)

target_link_libraries("board" 
    INTERFACE
    "soc"
    "driver"
    "lzma"
)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/board_pl_bitstream.c")
    message(FATAL_ERROR
        "board_pl_bitstream.c is missing - run the VSCode task "
        "\"Convert PL bitstream\" (or: python tools/convert_bitstream.py "
        "source/board/bitstream.bin source/board/board_pl_bitstream.c "
        "source/board/board_pl_bitstream.h)")
endif()
