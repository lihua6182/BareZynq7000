project("driver" C ASM)

add_library("driver" INTERFACE)

target_sources("driver"
    INTERFACE
    "zynq_gpio.c"
    "zynq_uart.c"
)

target_include_directories("driver"
    INTERFACE
    "."
)

target_link_libraries("driver"
    INTERFACE
    "soc"
)
