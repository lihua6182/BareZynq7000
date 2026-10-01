project("driver" C ASM)

add_library("driver" INTERFACE)

target_sources("driver"
    INTERFACE
    "zynq_mio.c"
    "zynq_uart.c"
    "zynq_ppc.c"
)

target_include_directories("driver"
    INTERFACE
    "."
)

target_link_libraries("driver"
    INTERFACE
    "soc"
)
