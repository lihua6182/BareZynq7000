ENTRY(_start)

MEMORY
{
    RAM          (rwx) : ORIGIN = 0x00000000, LENGTH = 192K
    NO_CACHE_RAM (rwx) : ORIGIN = 0xFFFF0000, LENGTH = 64K
    FLASH        (rx)  : ORIGIN = 0xFC000000, LENGTH = 16M
}

__ram_start__                   = ORIGIN(RAM);
__ram_end__                     = ORIGIN(RAM) + LENGTH(RAM);
__flash_start__                 = ORIGIN(FLASH);
__flash_end__                   = ORIGIN(FLASH) + LENGTH(FLASH);

SECTIONS
{
    /* Zynq boot header */
    .boot_header ORIGIN(FLASH) :
    {
        __boot_header_start__ = .;
        KEEP(*(.boot_header))
        __boot_header_end__ = .;
    } > FLASH

    /* ARM exception vector table (8 entries + literal pool). The IRQ slot
       is overridden at link time by the dispatcher in zynq_irq.c. */
    .vectors ORIGIN(FLASH) + 0x8C0 :
    {
        . = ALIGN(32);
        __vectors_start__ = .;
        KEEP(*(.vectors))
        __vectors_end__ = .;
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        KEEP(*(.init .init.*))
        *(.text .text.*)
        *(.glue_7 .glue_7t)
        *(.v4_bx)
        . = ALIGN(4);
    } > FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
        . = ALIGN(4);
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    /* C/C++ preinit/init/fini arrays remain in XIP flash. */
    .init_array :
    {
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP(*(.preinit_array))
        __preinit_array_end = .;
        
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
        . = ALIGN(4);
    } > FLASH

    .fini_array :
    {
        . = ALIGN(4);
        __fini_array_start = .;
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        __fini_array_end = .;
        . = ALIGN(4);
    } > FLASH

    /* RAM retained across warm reset. Never cleared by zynq_startup.s.
       NOTE: with the L2 cache off, dirty L1 lines are lost on reset, so
       retention only holds for data that was cleaned out of the D-cache
       before the reset. */
    .non_init (NOLOAD) :
    {
        . = ALIGN(16);
        __non_init_start__ = .;
        KEEP(*(.non_init))
        *(.non_init.*)
        . = ALIGN(4);
        __non_init_end__ = .;
    } > RAM

    /* Non-cacheable application RAM, DMA-reachable. */
    .no_cache_ram (NOLOAD) :
    {
        . = ALIGN(16384);
        KEEP(*(.mmu_table))
        . = ALIGN(16);
        __no_cache_ram_start__ = .;
        KEEP(*(.no_cache_ram))
        *(.no_cache_ram.*)
        . = ALIGN(4);
        __no_cache_ram_end__ = .;
    } > NO_CACHE_RAM

    /* Fast RAM code: VMA in OCM, load image in QSPI. */
    .fast :
    {
        . = ALIGN(4);
        __fast_start__ = .;
        *(.fast .fast.*)
        . = ALIGN(4);
        __fast_end__ = .;
    } > RAM AT > FLASH
    __fast_load__ = LOADADDR(.fast);

    /* Initialized data */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data.*)
        *(.sdata .sdata.*)
        *(.got .got.* .got.plt)
        *(.data.rel.ro .data.rel.ro.*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH
    _sidata = LOADADDR(.data);

    /* Zero-initialized data */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Exception stacks anchored at the top of RAM, growing down. */
    __stack_svc_end__   = __ram_end__;
    __stack_svc_start__ = __stack_svc_end__ - 0x400;   /* 1 KiB */

    __stack_irq_end__   = __stack_svc_start__;
    __stack_irq_start__ = __stack_irq_end__ - 0x400;   /* 1 KiB */

    __stack_und_end__   = __stack_irq_start__;
    __stack_und_start__ = __stack_und_end__ - 0x100;   /* 256 B */

    __stack_fiq_end__   = __stack_und_start__;
    __stack_fiq_start__ = __stack_fiq_end__ - 0x100;   /* 256 B */

    __stack_abt_end__   = __stack_fiq_start__;
    __stack_abt_start__ = __stack_abt_end__ - 0x100;   /* 256 B */

    /* Heap fills everything between .bss and the stacks: the LZMA working
       memory (dict window + probs, ~38 KiB) is malloc'd from it while
       board_init() loads the PL and freed right after. */
    __heap_start__      = ALIGN(_ebss, 8);
    __heap_end__        = __stack_abt_start__;

    __stack_end__ = __ram_end__;
    _estack       = __stack_end__;
    _end          = __heap_start__;

    /* Link-time sanity checks. */
    ASSERT(SIZEOF(.boot_header) <= 0x8C0,                   ".boot_header overlaps the 0x8C0 XIP payload boundary")
    ASSERT(ADDR(.vectors) == ORIGIN(FLASH) + 0x8C0,         ".vectors must start at 0xFC0008C0")
    ASSERT((ADDR(.vectors) & 0x1F) == 0,                    ".vectors must be 32-byte aligned")
    ASSERT(__heap_end__ - __heap_start__ >= 0xC000,         "heap < 48 KiB: no room for the boot LZMA allocation")
    ASSERT(__stack_svc_end__ <= __ram_end__,                "OCM overflow: heap/exception stacks exceed RAM")

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
    }
}
