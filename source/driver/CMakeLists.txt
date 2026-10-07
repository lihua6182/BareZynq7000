project("driver" C ASM)

add_library("driver" INTERFACE)

target_sources("driver"
    INTERFACE
    "zynq_cache.c"
    "zynq_mio.c"
    "zynq_uart.c"
    "zynq_ppc.c"
    "zynq_usb.c"
    "zynq_dma.c"
    "zynq_swd.c"
)

target_include_directories("driver"
    INTERFACE
    "."
)

target_link_libraries("driver"
    INTERFACE
    "soc"
)
