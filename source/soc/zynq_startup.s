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
        // Disable caches/MMU/BP in case an earlier run left them on;
        // makes a debugger warm reset behave like a cold one.
        mrc     p15, 0, r0, c1, c0, 0       /* CP15 System Control Register */
        bic     r0, r0, #(0x1 << 12)        /* I: disable I cache            */
        bic     r0, r0, #(0x1 <<  2)        /* C: disable D cache            */
        bic     r0, r0, #(0x1 <<  0)        /* M: disable MMU                */
        bic     r0, r0, #(0x1 << 11)        /* Z: disable branch prediction  */
        bic     r0, r0, #(0x1 << 13)        /* V: low vector table           */
        bic     r0, r0, #(0x1 << 28)        /* TRE: disable TEX remap        */
        mcr     p15, 0, r0, c1, c0, 0

        // L2 off while the MMU is off; re-enabled (prefetch off) by
        // mmu_and_caches_enable(). reg1_control = L2C + 0x100, bit 0.
        ldr     r0, =0xF8F02100
        mov     r1, #0
        str     r1, [r0]

        // Set the vector base address register to our vector table(VBAR)
        ldr     r0, =_vectors
        mcr     p15, 0, r0, c12, c0, 0


        // Keep the BootROM OCM split (RAM_HI=1000): OCM0-2 at 0x0,
        // OCM3 at 0xFFFF0000. RMW: bit 4 (SWAP) is write-as-read.
        ldr     r3, =0xF8000000
        movw    r2, #0xDF0D
        str     r2, [r3, #0x008]            /* SLCR_UNLOCK                   */
        ldr     r2, [r3, #0x910]            /* SLCR_OCM_CFG                  */
        bic     r2, r2, #0xF                /* RAM_HI = 0, keep SWAP as read */
        orr     r2, r2, #0x8                /* RAM_HI = 4'b1000: OCM3 high   */
        str     r2, [r3, #0x910]
        movw    r2, #0x767B
        str     r2, [r3, #0x004]            /* SLCR_LOCK                     */

        // Invalidate TLBs and the branch predictor array

        mov     r0, #0
        mcr     p15, 0, r0, c8, c7, 0       /* I-TLB and D-TLB invalidation  */
        mcr     p15, 0, r0, c7, c5, 6       /* BPIALL                        */

        // Invalidate the L1 caches (Cortex-A9, cache level walk)

        // Invalidate L1 instruction cache
        mrc     p15, 1, r0, c0, c0, 1       /* CLIDR                         */
        tst     r0, #0x3                    /* Harvard cache?                */
        mov     r0, #0                      /* SBZ                           */
        mcrne   p15, 0, r0, c7, c5, 0       /* ICIALLU                       */

        // Invalidate data / unified caches 
        mrc     p15, 1, r0, c0, c0, 1       /* CLIDR                         */
        ands    r3, r0, #0x07000000         /* Extract coherency level       */
        mov     r3, r3, lsr #23             /* Total cache levels << 1       */
        beq     Finished                    /* If 0, nothing to clean        */

        mov     r10, #0                     /* Current cache level << 1      */
Loop1:  add     r2, r10, r10, lsr #1        /* R2 = cache "Set" position     */
        mov     r1, r0, lsr r2              /* Bottom 3 bits: cache type     */
        and     r1, r1, #7
        cmp     r1, #2
        blt     Skip                        /* No cache, or I cache only     */

        mcr     p15, 2, r10, c0, c0, 0      /* Select cache level            */
        isb
        mrc     p15, 1, r1, c0, c0, 0       /* Cache Size ID                 */
        and     r2, r1, #7                  /* Line length field             */
        add     r2, r2, #4                  /* +4 for the log2(16) offset    */
        ldr     r4, =0x3FF
        ands    r4, r4, r1, lsr #3          /* Max way size (right aligned)  */
        clz     r5, r4                      /* Bit position of way increment */
        ldr     r7, =0x7FFF
        ands    r7, r7, r1, lsr #13         /* Max index size (right aligned)*/

Loop2:  mov     r9, r4                      /* Working copy of max way size  */
Loop3:  orr     r11, r10, r9, lsl r5        /* Way number and cache number   */
        orr     r11, r11, r7, lsl r2        /* Set number                    */
        mcr     p15, 0, r11, c7, c6, 2      /* Invalidate by set/way         */
        subs    r9, r9, #1
        bge     Loop3
        subs    r7, r7, #1
        bge     Loop2
Skip:   add     r10, r10, #2                /* Next cache level              */
        cmp     r3, r10
        bgt     Loop1
Finished:

        // MMU: use translation table base register 0. The table itself is
        // built later in soc_init(), once we are past the point
        // where variables may be used.

        mov     r0, #0
        mcr     p15, 0, r0, c2, c0, 2

#if !defined(__SOFTFP__)

        // Enable the VFP/NEON unit (hard-float builds only)

        mrc     p15, 0, r0, c1, c0, 2
        orr     r0, r0, #(0xF << 20)        /* Kernel+user access to cp10/11 */
        mcr     p15, 0, r0, c1, c0, 2
        mov     r0, #(1 << 30)              /* EN: enable VFP and NEON       */
        vmsr    FPEXC, r0
#endif

        // Jump to the runtime initialisation, which calls main().

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

        orr     r1, r0, #0x1B                   /* Undefined mode              */
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_und_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x17                   /* Abort mode                  */
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_abt_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x12                   /* IRQ mode                    */
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_irq_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x11                   /* FIQ mode                    */
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_fiq_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x13                   /* Supervisor mode             */
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_svc_end__
        bic     sp, sp, #0x7

        orr     r1, r0, #0x1F                   /* System mode                 */
        msr     cpsr_cxsf, r1
        ldr     sp, =__stack_end__
        bic     sp, sp, #0x7

        ldr     r0, =__fast_start__             /* Copy fast section            */
        ldr     r1, =__fast_end__
        ldr     r2, =__fast_load__
        b       .Lfast_test
.Lfast_copy:
        ldr     r3, [r2], #4
        str     r3, [r0], #4
.Lfast_test:
        cmp     r0, r1
        blo     .Lfast_copy

        ldr     r0, =_sdata                     /* Copy data section            */
        ldr     r1, =_edata
        ldr     r2, =_sidata
        b       .Ldata_test
.Ldata_copy:
        ldr     r3, [r2], #4
        str     r3, [r0], #4
.Ldata_test:
        cmp     r0, r1
        blo     .Ldata_copy

        ldr     r0, =_sbss                      /* Fill bss section as zero     */
        ldr     r1, =_ebss
        mov     r2, #0
        b       .Lbss_test
.Lbss_zero:
        str     r2, [r0], #4
.Lbss_test:
        cmp     r0, r1
        blo     .Lbss_zero

        /* .fast must be resident before soc_init runs from it. */
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

        // Default exception handlers
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
irq_handler:                     /* overridden by zynq_irq.c (dispatcher) */
        b       irq_handler

        .weak   fiq_handler
        .type   fiq_handler, function
fiq_handler:                     /* nothing routes to FIQ; hang if it fires */
        b       fiq_handler
