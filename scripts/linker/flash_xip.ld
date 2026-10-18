ENTRY(reset_handler)

MEMORY
{
    RAM          (rwx) : ORIGIN = 0x00000000, LENGTH = 192K
    NO_CACHE_RAM (rwx) : ORIGIN = 0xFFFF0000, LENGTH = 64K
    FLASH        (rx)  : ORIGIN = 0xFC000000, LENGTH = 16M
}

__ram_end__ = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Zynq boot header (UG585 ch.6), must occupy [0, 0x8C0). */
    .boot_header ORIGIN(FLASH) :
    {
        __boot_header_start__ = .;
        KEEP(*(.boot_header))
        __boot_header_end__ = .;
    } > FLASH

    /* VBAR: 8 ARM vectors + literal pool. 0x8C0 is already 32-byte aligned.
       The IRQ slot still points at irq_handler; zynq_irq.c supplies the
       strong definition that overrides the weak hang in zynq_startup.s. */
    .vectors ORIGIN(FLASH) + 0x8C0 :
    {
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

    /* preinit/init are called from zynq_startup.s; fini is never run
       (main does not return) but GCC may still emit the section. */
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

    /* 32 B at VA 0: NULL writes and irq_attach(NULL) cannot land in .fast. */
    .null_guard (NOLOAD) :
    {
        . = . + 32;
    } > RAM

    /* MMU table first (16 KB aligned; 0xFFFF0000 already is), then DMA RAM.
       NOLOAD: startup zeros .no_cache_ram; mmu_init fills the table. */
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

    /* Fast RAM code: VMA in OCM (after the NULL guard), LMA in QSPI. */
    .fast :
    {
        . = ALIGN(4);
        __fast_start__ = .;
        *(.fast .fast.*)
        . = ALIGN(4);
        __fast_end__ = .;
    } > RAM AT > FLASH
    __fast_load__ = LOADADDR(.fast);

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

    /* Stacks grow down from RAM top. SYS (main) and the SVC slot share the
       same 1 KiB — _start sets SVC SP then switches to SYS with the same
       top; SVC is unused after that. IRQ/UND/FIQ/ABT are separate. */
    __stack_end__     = __ram_end__;           /* SYS / main */
    __stack_svc_end__ = __ram_end__;           /* same 1 KiB, abandoned after mode switch */
    __stack_irq_end__ = __ram_end__ - 0x400;
    __stack_und_end__ = __stack_irq_end__ - 0x400;
    __stack_fiq_end__ = __stack_und_end__ - 0x100;
    __stack_abt_end__ = __stack_fiq_end__ - 0x100;
    __stack_abt_start__ = __stack_abt_end__ - 0x100;

    /* Heap: .bss up to the abort stack. LZMA boot decode ~38 KiB, then free. */
    __heap_start__ = ALIGN(_ebss, 8);
    __heap_end__   = __stack_abt_start__;

    ASSERT(SIZEOF(.boot_header) <= 0x8C0,                   ".boot_header overlaps the 0x8C0 XIP payload boundary")
    ASSERT(ADDR(.vectors) == ORIGIN(FLASH) + 0x8C0,         ".vectors must start at 0xFC0008C0")
    ASSERT((ADDR(.vectors) & 0x1F) == 0,                    ".vectors must be 32-byte aligned")
    ASSERT(ADDR(.fast) >= 32,                               ".fast must not start at NULL")
    ASSERT(_ebss <= __stack_abt_start__,                    "OCM overflow: .bss collided with exception stacks")
    ASSERT(__heap_start__ <= __heap_end__,                  "OCM overflow: heap start past heap end")
    ASSERT(__heap_end__ - __heap_start__ >= 0xC000,         "heap < 48 KiB: no room for the boot LZMA allocation")

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
    }
}
