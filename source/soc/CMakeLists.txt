project("soc" C ASM)

add_library("soc" INTERFACE)

target_sources("soc" 
    INTERFACE
    "zynq_boot_header.c"
    "zynq_clock.c"
    "zynq_delay.c"
    "zynq_fiq.c"
    "zynq_startup.s"
    "zynq_system.c"
)

target_include_directories("soc"
    INTERFACE
    "."
)
