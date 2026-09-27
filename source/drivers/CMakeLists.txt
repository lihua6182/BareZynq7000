project("drivers" C ASM)

add_library("drivers" INTERFACE)

target_sources("drivers" 
    INTERFACE
    "zynq_uart_drv.c"
)

target_include_directories("drivers"
    INTERFACE
    "."
)

target_link_libraries("drivers"
    INTERFACE
    "soc"
)
