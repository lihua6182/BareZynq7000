project("soc" C ASM)

add_library("soc" INTERFACE)

target_sources("soc" 
    INTERFACE
    "zynq_boot_header.c"
    "zynq_clock.c"
    "zynq_tick.c"
    "zynq_irq.c"
    "zynq_startup.s"
    "zynq_soc.c"
)

target_include_directories("soc"
    INTERFACE
    "."
    "cmsis"
)
