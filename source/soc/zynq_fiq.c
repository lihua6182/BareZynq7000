#include "zynq_fiq.h"

/* IAR/EOIR interrupt ID field; the bounds check below also dismisses the
   spurious ID 1023 without EOIR, as IHI0048 requires */
#define GIC_INT_ID_MSK  (0x3FFUL)

/* Default priority handed out by zynq_fiq_enable: mid-range, below the
   reset value 0 so a future IRQ group can always preempt if needed */
#define ZYNQ_FIQ_DEFAULT_PRIORITY (0xA0U)

/* Handler slot type: void f(void), STM32 IRQ-handler style. */
typedef void (*zynq_fiq_isr_t)(void);

/*
 * One weak handler per GIC interrupt ID, 0..95 (16 SGI + 16 PPI + 64 SPI,
 * UG585 Table 7-3). Unclaimed sources run the empty default body; a module
 * claims a source by defining a plain function with the same name.
 */
__attribute__((weak)) void zynq_fiq_isr_0(void) {}
__attribute__((weak)) void zynq_fiq_isr_1(void) {}
__attribute__((weak)) void zynq_fiq_isr_2(void) {}
__attribute__((weak)) void zynq_fiq_isr_3(void) {}
__attribute__((weak)) void zynq_fiq_isr_4(void) {}
__attribute__((weak)) void zynq_fiq_isr_5(void) {}
__attribute__((weak)) void zynq_fiq_isr_6(void) {}
__attribute__((weak)) void zynq_fiq_isr_7(void) {}
__attribute__((weak)) void zynq_fiq_isr_8(void) {}
__attribute__((weak)) void zynq_fiq_isr_9(void) {}
__attribute__((weak)) void zynq_fiq_isr_10(void) {}
__attribute__((weak)) void zynq_fiq_isr_11(void) {}
__attribute__((weak)) void zynq_fiq_isr_12(void) {}
__attribute__((weak)) void zynq_fiq_isr_13(void) {}
__attribute__((weak)) void zynq_fiq_isr_14(void) {}
__attribute__((weak)) void zynq_fiq_isr_15(void) {}
__attribute__((weak)) void zynq_fiq_isr_16(void) {}
__attribute__((weak)) void zynq_fiq_isr_17(void) {}
__attribute__((weak)) void zynq_fiq_isr_18(void) {}
__attribute__((weak)) void zynq_fiq_isr_19(void) {}
__attribute__((weak)) void zynq_fiq_isr_20(void) {}
__attribute__((weak)) void zynq_fiq_isr_21(void) {}
__attribute__((weak)) void zynq_fiq_isr_22(void) {}
__attribute__((weak)) void zynq_fiq_isr_23(void) {}
__attribute__((weak)) void zynq_fiq_isr_24(void) {}
__attribute__((weak)) void zynq_fiq_isr_25(void) {}
__attribute__((weak)) void zynq_fiq_isr_26(void) {}
__attribute__((weak)) void zynq_fiq_isr_27(void) {}
__attribute__((weak)) void zynq_fiq_isr_28(void) {}
__attribute__((weak)) void zynq_fiq_isr_29(void) {}
__attribute__((weak)) void zynq_fiq_isr_30(void) {}
__attribute__((weak)) void zynq_fiq_isr_31(void) {}
__attribute__((weak)) void zynq_fiq_isr_32(void) {}
__attribute__((weak)) void zynq_fiq_isr_33(void) {}
__attribute__((weak)) void zynq_fiq_isr_34(void) {}
__attribute__((weak)) void zynq_fiq_isr_35(void) {}
__attribute__((weak)) void zynq_fiq_isr_36(void) {}
__attribute__((weak)) void zynq_fiq_isr_37(void) {}
__attribute__((weak)) void zynq_fiq_isr_38(void) {}
__attribute__((weak)) void zynq_fiq_isr_39(void) {}
__attribute__((weak)) void zynq_fiq_isr_40(void) {}
__attribute__((weak)) void zynq_fiq_isr_41(void) {}
__attribute__((weak)) void zynq_fiq_isr_42(void) {}
__attribute__((weak)) void zynq_fiq_isr_43(void) {}
__attribute__((weak)) void zynq_fiq_isr_44(void) {}
__attribute__((weak)) void zynq_fiq_isr_45(void) {}
__attribute__((weak)) void zynq_fiq_isr_46(void) {}
__attribute__((weak)) void zynq_fiq_isr_47(void) {}
__attribute__((weak)) void zynq_fiq_isr_48(void) {}
__attribute__((weak)) void zynq_fiq_isr_49(void) {}
__attribute__((weak)) void zynq_fiq_isr_50(void) {}
__attribute__((weak)) void zynq_fiq_isr_51(void) {}
__attribute__((weak)) void zynq_fiq_isr_52(void) {}
__attribute__((weak)) void zynq_fiq_isr_53(void) {}
__attribute__((weak)) void zynq_fiq_isr_54(void) {}
__attribute__((weak)) void zynq_fiq_isr_55(void) {}
__attribute__((weak)) void zynq_fiq_isr_56(void) {}
__attribute__((weak)) void zynq_fiq_isr_57(void) {}
__attribute__((weak)) void zynq_fiq_isr_58(void) {}
__attribute__((weak)) void zynq_fiq_isr_59(void) {}
__attribute__((weak)) void zynq_fiq_isr_60(void) {}
__attribute__((weak)) void zynq_fiq_isr_61(void) {}
__attribute__((weak)) void zynq_fiq_isr_62(void) {}
__attribute__((weak)) void zynq_fiq_isr_63(void) {}
__attribute__((weak)) void zynq_fiq_isr_64(void) {}
__attribute__((weak)) void zynq_fiq_isr_65(void) {}
__attribute__((weak)) void zynq_fiq_isr_66(void) {}
__attribute__((weak)) void zynq_fiq_isr_67(void) {}
__attribute__((weak)) void zynq_fiq_isr_68(void) {}
__attribute__((weak)) void zynq_fiq_isr_69(void) {}
__attribute__((weak)) void zynq_fiq_isr_70(void) {}
__attribute__((weak)) void zynq_fiq_isr_71(void) {}
__attribute__((weak)) void zynq_fiq_isr_72(void) {}
__attribute__((weak)) void zynq_fiq_isr_73(void) {}
__attribute__((weak)) void zynq_fiq_isr_74(void) {}
__attribute__((weak)) void zynq_fiq_isr_75(void) {}
__attribute__((weak)) void zynq_fiq_isr_76(void) {}
__attribute__((weak)) void zynq_fiq_isr_77(void) {}
__attribute__((weak)) void zynq_fiq_isr_78(void) {}
__attribute__((weak)) void zynq_fiq_isr_79(void) {}
__attribute__((weak)) void zynq_fiq_isr_80(void) {}
__attribute__((weak)) void zynq_fiq_isr_81(void) {}
__attribute__((weak)) void zynq_fiq_isr_82(void) {}
__attribute__((weak)) void zynq_fiq_isr_83(void) {}
__attribute__((weak)) void zynq_fiq_isr_84(void) {}
__attribute__((weak)) void zynq_fiq_isr_85(void) {}
__attribute__((weak)) void zynq_fiq_isr_86(void) {}
__attribute__((weak)) void zynq_fiq_isr_87(void) {}
__attribute__((weak)) void zynq_fiq_isr_88(void) {}
__attribute__((weak)) void zynq_fiq_isr_89(void) {}
__attribute__((weak)) void zynq_fiq_isr_90(void) {}
__attribute__((weak)) void zynq_fiq_isr_91(void) {}
__attribute__((weak)) void zynq_fiq_isr_92(void) {}
__attribute__((weak)) void zynq_fiq_isr_93(void) {}
__attribute__((weak)) void zynq_fiq_isr_94(void) {}
__attribute__((weak)) void zynq_fiq_isr_95(void) {}

/* Dispatch table indexed by interrupt ID; unclaimed slots resolve to the
   empty weak bodies above, overridden slots to the claiming module's
   function. The array is sized ZYNQ_GIC_NUM_IDS, so a mismatched list
   fails to link. Served from flash through the I-cache; see the note in
   zynq_fiq.h before taking QSPI out of linear mode at run time. */
static const zynq_fiq_isr_t zynq_fiq_dispatch[ZYNQ_GIC_NUM_IDS] = {
    [0] = zynq_fiq_isr_0,
    [1] = zynq_fiq_isr_1,
    [2] = zynq_fiq_isr_2,
    [3] = zynq_fiq_isr_3,
    [4] = zynq_fiq_isr_4,
    [5] = zynq_fiq_isr_5,
    [6] = zynq_fiq_isr_6,
    [7] = zynq_fiq_isr_7,
    [8] = zynq_fiq_isr_8,
    [9] = zynq_fiq_isr_9,
    [10] = zynq_fiq_isr_10,
    [11] = zynq_fiq_isr_11,
    [12] = zynq_fiq_isr_12,
    [13] = zynq_fiq_isr_13,
    [14] = zynq_fiq_isr_14,
    [15] = zynq_fiq_isr_15,
    [16] = zynq_fiq_isr_16,
    [17] = zynq_fiq_isr_17,
    [18] = zynq_fiq_isr_18,
    [19] = zynq_fiq_isr_19,
    [20] = zynq_fiq_isr_20,
    [21] = zynq_fiq_isr_21,
    [22] = zynq_fiq_isr_22,
    [23] = zynq_fiq_isr_23,
    [24] = zynq_fiq_isr_24,
    [25] = zynq_fiq_isr_25,
    [26] = zynq_fiq_isr_26,
    [27] = zynq_fiq_isr_27,
    [28] = zynq_fiq_isr_28,
    [29] = zynq_fiq_isr_29,
    [30] = zynq_fiq_isr_30,
    [31] = zynq_fiq_isr_31,
    [32] = zynq_fiq_isr_32,
    [33] = zynq_fiq_isr_33,
    [34] = zynq_fiq_isr_34,
    [35] = zynq_fiq_isr_35,
    [36] = zynq_fiq_isr_36,
    [37] = zynq_fiq_isr_37,
    [38] = zynq_fiq_isr_38,
    [39] = zynq_fiq_isr_39,
    [40] = zynq_fiq_isr_40,
    [41] = zynq_fiq_isr_41,
    [42] = zynq_fiq_isr_42,
    [43] = zynq_fiq_isr_43,
    [44] = zynq_fiq_isr_44,
    [45] = zynq_fiq_isr_45,
    [46] = zynq_fiq_isr_46,
    [47] = zynq_fiq_isr_47,
    [48] = zynq_fiq_isr_48,
    [49] = zynq_fiq_isr_49,
    [50] = zynq_fiq_isr_50,
    [51] = zynq_fiq_isr_51,
    [52] = zynq_fiq_isr_52,
    [53] = zynq_fiq_isr_53,
    [54] = zynq_fiq_isr_54,
    [55] = zynq_fiq_isr_55,
    [56] = zynq_fiq_isr_56,
    [57] = zynq_fiq_isr_57,
    [58] = zynq_fiq_isr_58,
    [59] = zynq_fiq_isr_59,
    [60] = zynq_fiq_isr_60,
    [61] = zynq_fiq_isr_61,
    [62] = zynq_fiq_isr_62,
    [63] = zynq_fiq_isr_63,
    [64] = zynq_fiq_isr_64,
    [65] = zynq_fiq_isr_65,
    [66] = zynq_fiq_isr_66,
    [67] = zynq_fiq_isr_67,
    [68] = zynq_fiq_isr_68,
    [69] = zynq_fiq_isr_69,
    [70] = zynq_fiq_isr_70,
    [71] = zynq_fiq_isr_71,
    [72] = zynq_fiq_isr_72,
    [73] = zynq_fiq_isr_73,
    [74] = zynq_fiq_isr_74,
    [75] = zynq_fiq_isr_75,
    [76] = zynq_fiq_isr_76,
    [77] = zynq_fiq_isr_77,
    [78] = zynq_fiq_isr_78,
    [79] = zynq_fiq_isr_79,
    [80] = zynq_fiq_isr_80,
    [81] = zynq_fiq_isr_81,
    [82] = zynq_fiq_isr_82,
    [83] = zynq_fiq_isr_83,
    [84] = zynq_fiq_isr_84,
    [85] = zynq_fiq_isr_85,
    [86] = zynq_fiq_isr_86,
    [87] = zynq_fiq_isr_87,
    [88] = zynq_fiq_isr_88,
    [89] = zynq_fiq_isr_89,
    [90] = zynq_fiq_isr_90,
    [91] = zynq_fiq_isr_91,
    [92] = zynq_fiq_isr_92,
    [93] = zynq_fiq_isr_93,
    [94] = zynq_fiq_isr_94,
    [95] = zynq_fiq_isr_95,
};

/*
 * FIQ entry: overrides the weak default in zynq_startup.s. FIQ mode banks
 * r8-r14 and the stack, so the interrupt attribute only has to preserve
 * r0-r7 (it adds the exception prologue and the subs pc, lr, #4 return).
 * The IAR/EOIR pair is mandatory: without the acknowledge the FIQ line
 * stays asserted and the exception re-fires right after the SPSR restore.
 * Spurious ID 1023 takes no EOIR (IHI0048). target("general-regs-only")
 * keeps the handler off the VFP/NEON bank; GCC's warning below only looks
 * at command-line options, hence the pragma.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((interrupt("FIQ"), target("general-regs-only"))) void fiq_handler(void) {
    uint32_t id = GICC->IAR & GIC_INT_ID_MSK;

    if (id < ZYNQ_GIC_NUM_IDS) {
        zynq_fiq_dispatch[id]();
        __asm volatile("dsb" ::: "memory"); /* source flag clear completes before EOI */
        GICC->EOIR = id;                    /* priority drop + deactivate */
    }
}
#pragma GCC diagnostic pop

/*
 * Bring up the GIC for FIQ delivery: enable the distributor and the CPU
 * interface with FIQEn (Group 0 -> FIQ), then unmask FIQ in the CPSR. Run
 * once at boot, before arming any source. IRQ (Group 1) delivery is left
 * untouched.
 */
void zynq_fiq_init(void) {
    GICD->CTLR = 0; /* distributor off while configuring */
    GICC->CTLR = 0; /* CPU interface off */

    GICC->PMR = 0xFFUL; /* unmask all priorities */

    GICD->CTLR = 1UL;                     /* enable the distributor */
    GICC->CTLR = (1UL << 0) | (1UL << 3); /* enable + FIQEn          */

    __enable_fiq(); /* unmask FIQ in the CPSR */
}

/*
 * Route an interrupt source to FIQ: Group 0 (secure) + mid-range priority
 * + enabled in the distributor. PPI/SGI registers (id < 32) are banked per
 * CPU; this programs the bank of the executing core.
 */
void zynq_fiq_enable(uint32_t id) {
    uint32_t shift;

    if (id >= ZYNQ_GIC_NUM_IDS) {
        return;
    }
    shift = id & 0x1FU;

    GICD->IGROUPR[id / 32U] &= ~(1UL << shift); /* Group 0 -> FIQ path */
    GICD->IPRIORITYR[id / 4U] =
        (GICD->IPRIORITYR[id / 4U] & ~(0xFFUL << ((id & 3U) * 8U))) |
        ((uint32_t)ZYNQ_FIQ_DEFAULT_PRIORITY << ((id & 3U) * 8U));
    GICD->ISENABLER[id / 32U] = (1UL << shift);
}

/* Stop delivering a source to FIQ (distributor enable cleared). */
void zynq_fiq_disable(uint32_t id) {
    if (id >= ZYNQ_GIC_NUM_IDS) {
        return;
    }
    GICD->ICENABLER[id / 32U] = (1UL << (id & 0x1FU));
}
