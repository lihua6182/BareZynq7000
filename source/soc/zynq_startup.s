        .syntax unified

        .section .vectors, "ax"
        .code 32
        .balign 4
        .global _vectors
        .type   _vectors, function
_vectors:
        ldr     pc, =reset_handler
        ldr     pc, =undef_handler
        ldr     pc, =svc_handler
        ldr     pc, =pabort_handler
        ldr     pc, =dabort_handler
        nop
        ldr     pc, =irq_handler
        ldr     pc, =fiq_handler

        .section .init, "ax"
        .code 32
        .balign 4
        .global reset_handler
        .type   reset_handler, function
reset_handler:

        cpsid   if

        mrc     p15, 0, r0, c1, c0, 0
        bic     r0, r0, #(0x1 << 12)
        bic     r0, r0, #(0x1 <<  2)
        bic     r0, r0, #(0x1 <<  0)
        bic     r0, r0, #(0x1 << 11)
        bic     r0, r0, #(0x1 << 13)
        bic     r0, r0, #(0x1 << 28)
        mcr     p15, 0, r0, c1, c0, 0
        dsb     sy
        isb     sy

        ldr     r0, =0xF8F02100
        mov     r1, #0
        str     r1, [r0]
        dsb     sy

        ldr     r0, =_vectors
        mcr     p15, 0, r0, c12, c0, 0

        ldr     r3, =0xF8000000
        movw    r2, #0xDF0D
        str     r2, [r3, #0x008]
        ldr     r2, [r3, #0x910]
        bic     r2, r2, #0xF
        orr     r2, r2, #0x8
        str     r2, [r3, #0x910]
        movw    r2, #0x767B
        str     r2, [r3, #0x004]

        mov     r0, #0
        mcr     p15, 0, r0, c8, c7, 0
        mcr     p15, 0, r0, c7, c5, 6

        mrc     p15, 1, r0, c0, c0, 1
        tst     r0, #0x3
        mov     r0, #0
        mcrne   p15, 0, r0, c7, c5, 0

        mrc     p15, 1, r0, c0, c0, 1
        ands    r3, r0, #0x07000000
        mov     r3, r3, lsr #23
        beq     Finished

        mov     r10, #0
Loop1:  add     r2, r10, r10, lsr #1
        mov     r1, r0, lsr r2
        and     r1, r1, #7
        cmp     r1, #2
        blt     Skip

        mcr     p15, 2, r10, c0, c0, 0
        isb
        mrc     p15, 1, r1, c0, c0, 0
        and     r2, r1, #7
        add     r2, r2, #4
        ldr     r4, =0x3FF
        ands    r4, r4, r1, lsr #3
        clz     r5, r4
        ldr     r7, =0x7FFF
        ands    r7, r7, r1, lsr #13

Loop2:  mov     r9, r4
Loop3:  orr     r11, r10, r9, lsl r5
        orr     r11, r11, r7, lsl r2
        mcr     p15, 0, r11, c7, c6, 2
        subs    r9, r9, #1
        bge     Loop3
        subs    r7, r7, #1
        bge     Loop2
Skip:   add     r10, r10, #2
        cmp     r3, r10
        bgt     Loop1
Finished:

        mov     r0, #0
        mcr     p15, 0, r0, c2, c0, 2

#if !defined(__SOFTFP__)

        mrc     p15, 0, r0, c1, c0, 2
        orr     r0, r0, #(0xF << 20)
        mcr     p15, 0, r0, c1, c0, 2
        mov     r0, #(1 << 30)
        vmsr    FPEXC, r0
#endif

        b       _start

        .pool

        .section .init, "ax"
        .code 32
        .balign 4
        .weak   _start
        .global _start
        .type   _start, function
_start:
        mrs     r0, cpsr
        bic     r0, r0, #0x1F

        orr     r1, r0, #0x1B
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_und_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x17
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_abt_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x12
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_irq_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x11
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_fiq_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x13
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_svc_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x1F
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_end__
        bic     sp, sp, #0x7

        ldr     r0, =__fast_start__
        ldr     r1, =__fast_end__
        ldr     r2, =__fast_load__
        b       .Lfast_test
.Lfast_copy:
        ldr     r3, [r2], #4
        str     r3, [r0], #4
.Lfast_test:
        cmp     r0, r1
        blo     .Lfast_copy

        ldr     r0, =_sdata
        ldr     r1, =_edata
        ldr     r2, =_sidata
        b       .Ldata_test
.Ldata_copy:
        ldr     r3, [r2], #4
        str     r3, [r0], #4
.Ldata_test:
        cmp     r0, r1
        blo     .Ldata_copy

        ldr     r0, =_sbss
        ldr     r1, =_ebss
        mov     r2, #0
        b       .Lbss_test
.Lbss_zero:
        str     r2, [r0], #4
.Lbss_test:
        cmp     r0, r1
        blo     .Lbss_zero

        ldr     r0, =__no_cache_ram_start__
        ldr     r1, =__no_cache_ram_end__
        mov     r2, #0
        b       .Lnc_test
.Lnc_zero:
        str     r2, [r0], #4
.Lnc_test:
        cmp     r0, r1
        blo     .Lnc_zero

        bl      soc_init

        ldr     r4, =__preinit_array_start
        ldr     r5, =__preinit_array_end
        b       .Lpreinit_test
.Lpreinit_call:
        ldr     r3, [r4], #4
        blx     r3
.Lpreinit_test:
        cmp     r4, r5
        blo     .Lpreinit_call

        ldr     r4, =__init_array_start
        ldr     r5, =__init_array_end
        b       .Linit_test
.Linit_call:
        ldr     r3, [r4], #4
        blx     r3
.Linit_test:
        cmp     r4, r5
        blo     .Linit_call

        bl      main
.Lmain_hang:
        b       .Lmain_hang

        .pool

        .section .init, "ax"
        .code 32
        .balign 4

        .weak   undef_handler
        .type   undef_handler, function
undef_handler:
        b       undef_handler

        .weak   svc_handler
        .type   svc_handler, function
svc_handler:
        b       svc_handler

        .weak   pabort_handler
        .type   pabort_handler, function
pabort_handler:
        b       pabort_handler

        .weak   dabort_handler
        .type   dabort_handler, function
dabort_handler:
        b       dabort_handler

        .weak   irq_handler
        .type   irq_handler, function
irq_handler:
        b       irq_handler

        .weak   fiq_handler
        .type   fiq_handler, function
fiq_handler:
        b       fiq_handler
