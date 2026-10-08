project("driver" C ASM)

# PS peripherals sit at this directory root (mio / uart / usb / dma / cache).
# PL IP access (proto_coproc + SWD layer) lives in pl/ — same INTERFACE
# library, separate include dir so they are not mistaken for PS drivers.
add_library("driver" INTERFACE)

target_sources("driver"
    INTERFACE
    "zynq_cache.c"
    "zynq_mio.c"
    "zynq_uart.c"
    "zynq_usb.c"
    "zynq_dma.c"
    "pl/zynq_ppc.c"
    "pl/zynq_swd.c"
)

target_include_directories("driver"
    INTERFACE
    "."
    "pl"
)

target_link_libraries("driver"
    INTERFACE
    "soc"
)
