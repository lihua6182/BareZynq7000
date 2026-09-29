/**
 * @file zynq_system.c
 * @brief Boot-time MMU, cache, clock and QSPI setup for XIP operation.
 *
 * No FSBL: the BootROM starts this image directly out of QSPI, so the
 * MMU, the caches, the PLLs and the flash controller are all brought up
 * here. The order in soc_init() is a hard constraint - the CPU is
 * fetching from flash the whole time.
 */

#include "zynq_soc_ip.h"

#define NUM_TT_ENTRIES (0x1000U)  // 4096 x 1 MB sections cover the full 4 GB

/*
 * Page table in non-cacheable high OCM (0xFFFF0000): the CA9 walker is
 * not coherent with D-cache, so descriptor writes must be visible
 * directly. DSB + TLB invalidate after editing.
 */
static uint32_t mmu_table[NUM_TT_ENTRIES] __attribute__((aligned(16384), section(".mmu_table")));

#define TT_SECTION          (0x2U)                                // bits[1:0] == 10: 1 MB section
#define TT_DOMAIN_0         (0x0U << 5)                           // domain 0
#define TT_XN               (1U << 4)                             // execute never
#define TT_AP_RW            (0x2U << 10)                          // AP[2:0] == 010: RW (PL1 and PL0)
#define TT_AP_RO            ((0x2U << 10) | (1U << 15))           // AP[2:0] == 110: RO (PL1 and PL0)
#define TT_STRONGLY_ORDERED (0U)                                  // TEX=000, C=0, B=0
#define TT_WRITE_BACK_ALLOC ((1U << 12) | (1U << 3) | (1U << 2))  // TEX=001, C=1, B=1: normal, write-back, write-allocate
#define TT_NON_CACHEABLE    (1U << 12)                            // TEX=001, C=0, B=0: normal, non-cacheable

#define TT_ENTRY(PAddrMB, Attr) ((((uint32_t)(PAddrMB)) << 20) | (Attr) | TT_DOMAIN_0 | TT_SECTION)

/* Must run from OCM during the QSPI/clock reconfiguration window. */
static void qspi_clock_init(void) __attribute__((section(".fast"), noinline));
static void sys_clock_init(void) __attribute__((section(".fast"), noinline));
static void xip_warm_cache(void) __attribute__((section(".fast"), noinline));

/**
 * @brief  Fill the translation table with 1 MB sections.
 * @param  ttb Base of the table; must be 16 KB aligned.
 *
 * Map (VA -> attr):
 *   0x00000000-0x0002FFFF  low OCM (OCM0-2)            WB-alloc RW
 *   0x00100000-0x3FFFFFFF  DDR                         UNMAPPED (no DDR ctrl)
 *   0x40000000-0xBFFFFFFF  PL AXI GP                   SO RW, XN
 *   0xE0000000-0xF9FFFFFF  IOP + PS sys regs           SO RW, XN
 *   0xFA000000-0xFAFFFFFF  unmapped
 *   0xFB000000-0xFB0FFFFF  CPU-only OCM alias          SO RW
 *   0xFB100000-0xFB1FFFFF  CPU-only OCM alias          non-cache RW
 *   0xFC000000-0xFCFFFFFF  16 MB QSPI flash            WB-alloc RO (XIP)
 *   0xFD000000-0xFFEFFFFF  unmapped
 *   0xFFFF0000-0xFFFFFFFF  high OCM (OCM3)             non-cache RW (MMU tbl)
 *
 * 0xFB0/0xFB1 alias PA 0x0; other bus masters skip the MMU and see DDR,
 * so DMA must use high OCM. The PL310 (L2) is enabled by mmu_enable().
 */
static void mmu_init(uint32_t* ttb) {
    uint32_t i;

    // unmapped: any access triggers a translation fault
    for (i = 0; i < NUM_TT_ENTRIES; i++) {
        ttb[i] = 0;
    }

    // Low 192 KB OCM, write-back allocate
    ttb[0x000] = TT_ENTRY(0x000, TT_WRITE_BACK_ALLOC | TT_AP_RW);

    // PL + IOP peripherals + PS system registers. DDR (0x001-0x3FF) left unmapped.
    for (i = 0x400; i < 0xFA0u; i++) {
        ttb[i] = TT_ENTRY(i, TT_STRONGLY_ORDERED | TT_AP_RW | TT_XN);
    }

    // CPU-only aliases of the low OCM (VA -> PA 0x0)
    ttb[0xFB0] = TT_ENTRY(0x000, TT_STRONGLY_ORDERED | TT_AP_RW);
    ttb[0xFB1] = TT_ENTRY(0x000, TT_NON_CACHEABLE | TT_AP_RW);

    // 16 MB QSPI flash, XIP read caching
    for (i = 0xFC0; i < 0xFD0u; i++) {
        ttb[i] = TT_ENTRY(i, TT_WRITE_BACK_ALLOC | TT_AP_RO);
    }

    // High 64 KB OCM (OCM3): MMU table + .no_cache_ram
    ttb[0xFFF] = TT_ENTRY(0xFFF, TT_NON_CACHEABLE | TT_AP_RW);
}

/**
 * @brief  Pull the whole flash-resident image into L2/L1D.
 *
 * Reads the image as data while the QSPI is still in its BootROM
 * configuration: the first fetch after the clock reconfiguration is
 * always a miss (startup code was never cached), and reads through the
 * freshly reconfigured controller are not reliable on this board - so
 * the warmed L2 serves all early fetches. The span is _sidata + .data
 * size, the end of everything fetchable from QSPI; extend it if another
 * AT > FLASH section is ever added after .data.
 *
 * @note Runs from OCM (.fast) during the reconfiguration window.
 */
static void xip_warm_cache(void) {
    extern uint8_t _sidata[], _sdata[], _edata[];
    volatile const uint32_t* p = (volatile const uint32_t*)LQSPI_BASE;
    uint32_t words = ((uint32_t)(uintptr_t)(_sidata + (_edata - _sdata)) - LQSPI_BASE + 3U) / 4U;
    uint32_t i;

    for (i = 0U; i < words; i++) {
        (void)p[i];
    }
}

/**
 * @brief  Reconfigure the QSPI clock and read capture for XIP.
 *
 * UG585 ch.12: leave linear mode with both FIFOs empty or the autonomous
 * prefetch corrupts in-flight data. Result: QSPI ref = IO PLL/5 =
 * 200 MHz, SCLK = 100 MHz, loopback capture; config_reg = the BootROM
 * handoff 0x800238C1 (BAUD_RATE_DIV = 0).
 *
 * @note Runs from OCM (.fast): the CPU is fetching from the very flash
 *       whose controller is being reconfigured.
 */
static void qspi_clock_init(void) {
    uint32_t lqspi_cr;
    uint32_t t;

    /* 1. Leave linear mode; CPU keeps running from OCM. */
    lqspi_cr = QSPI->LQSPI_CR;
    QSPI->LQSPI_CR = lqspi_cr & ~(1U << 31); /* LQ_MODE = 0 */

    /* 2. Drain RxFIFO (63 deep): stop once SR[4] = 0. */
    for (t = 0U; t < 64U; t++) {
        if ((QSPI->SR & (1U << 4)) == 0U) {
            break;
        }
        (void)QSPI->RXD;
    }

    /* 3. Ref clk to IO PLL/5, then pulse both resets (clears regs, Table 12-6). */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->LQSPI_CLK_CTRL = (5U << 8) | /* DIVISOR = 5                     */
                           (1U << 0);  /* CLKACT = 1, SRCSEL = 0x: IO PLL */

    /* MIO8/9 = QSPI_SCLK_FB (UG585 Table 6-10): loopback needs the ring closed. */
    SLCR->MIO_PIN_08 = 0x00000602U;
    SLCR->MIO_PIN_09 = 0x00000602U;

    SLCR->LQSPI_RST_CTRL = (1U << 1) | /* QSPI_REF_RST                    */
                           (1U << 0);  /* LQSPI_CPU1X_RST                 */
    SLCR->LQSPI_RST_CTRL = 0U;
    for (t = 0U; t < 100U; t++) {
        __asm volatile("nop");
    }
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* 4. Reprogram controller, re-enter linear mode (LQSPI_CR = 0x8000016B). */
    QSPI->CR = 0x800238C1U;
    QSPI->LPBK_DLY_ADJ = 0x2DU; /* USE_LPBK = 1; reserved [4:0] at reset value */
    QSPI->LQSPI_CR = lqspi_cr;  /* LQ_MODE = 1                    */
    QSPI->ER = 1U;              /* SPI_EN = 1                     */
}

/**
 * @brief  Bring up the ARM and IO PLLs and the CPU clock divider.
 *
 * Clocks per UG585 Table 25-3 (6:2:1, PS_CLK = 33.333 MHz):
 *   ARM PLL FDIV 40 -> 1333 MHz; CPU = /2 = 667 MHz
 *   IO  PLL FDIV 30 -> 1000 MHz; QSPI ref = /5 = 200 MHz
 *
 * Each PLL: program FDIV/CFG, force bypass (glitch-free, system runs on
 * PS_CLK), pulse reset, wait for lock, leave bypass. DDR PLL untouched
 * (no DDR controller). MMU/L1 on, so the lock waits stay in I-cache. A
 * PLL that never locks leaves that clock in bypass and returns early.
 *
 * @note Runs from OCM (.fast) during the reconfiguration window.
 */
static void sys_clock_init(void) {
    uint32_t ctrl;
    uint32_t t;

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;

    /* ARM PLL FDIV 40, CFG 250/2/2 (Table 25-6). */
    ctrl = SLCR->ARM_PLL_CTRL;
    ctrl = (ctrl & ~(0x7FU << 12)) | (40UL << 12);
    SLCR->ARM_PLL_CTRL = ctrl;

    SLCR->ARM_PLL_CFG = ((uint32_t)250UL << 12) |
                        ((uint32_t)2UL << 8) |
                        ((uint32_t)2UL << 4);

    SLCR->ARM_PLL_CTRL = ctrl | (1U << 4);             /* force bypass     */
    SLCR->ARM_PLL_CTRL = ctrl | (1U << 4) | (1U << 0); /* assert PLL_RESET */
    SLCR->ARM_PLL_CTRL = ctrl | (1U << 4);             /* de-assert reset  */

    for (t = 0U; t < 100000U; t++) {
        if ((SLCR->PLL_STATUS & 1U) != 0U) { /* ARM_PLL_LOCK */
            break;
        }
    }
    if (t == 100000U) {
        /* ARM PLL failed to lock: keep bypass clocks, return. */
        SLCR->LOCK = SLCR_LOCK_KEY;
        return;
    }

    SLCR->ARM_PLL_CTRL = ctrl; /* leave bypass */

    /* CPU = ARM PLL / 2. */
    SLCR->ARM_CLK_CTRL = 0x1F000200U;

    /* IO PLL FDIV 30 -> 1000 MHz, CFG 325/2/12 (Table 25-6). */
    ctrl = SLCR->IO_PLL_CTRL;
    ctrl = (ctrl & ~(0x7FU << 12)) | (30UL << 12);
    SLCR->IO_PLL_CTRL = ctrl;

    SLCR->IO_PLL_CFG = (325UL << 12) | /* LOCK_CNT = 325 */
                       (2UL << 8) |    /* CP       = 2  */
                       (12UL << 4);    /* RES      = 12 */

    SLCR->IO_PLL_CTRL = ctrl | (1U << 4);             /* force bypass     */
    SLCR->IO_PLL_CTRL = ctrl | (1U << 4) | (1U << 0); /* assert PLL_RESET */
    SLCR->IO_PLL_CTRL = ctrl | (1U << 4);             /* de-assert reset  */

    for (t = 0U; t < 100000U; t++) {
        if ((SLCR->PLL_STATUS & (1U << 2)) != 0U) { /* IO_PLL_LOCK */
            break;
        }
    }
    if (t == 100000U) {
        /* IO PLL failed to lock: keep bypassed, leave QSPI on ARM PLL. */
        SLCR->LOCK = SLCR_LOCK_KEY;
        return;
    }

    SLCR->IO_PLL_CTRL = ctrl; /* leave bypass */

    SLCR->LOCK = SLCR_LOCK_KEY;
}

/* TAG/DATA RAM latency registers (0x108/0x10C) are not members of the
   CMSIS L2C_310_TypeDef, so they are written by offset. */
#define L2C_TAG_RAM_CNTRL  ((*((__IO uint32_t*)(uintptr_t)L2C_310_BASE + (0x108U / 4U))))
#define L2C_DATA_RAM_CNTRL ((*((__IO uint32_t*)(uintptr_t)L2C_310_BASE + (0x10CU / 4U))))

/**
 * @brief  Bring up the PL310 L2 cache controller.
 *
 * Per the official boot.S: configure while disabled, invalidate by way,
 * set SLCR L2C_RAM = 0x202 (UG585, AR#54190), then enable. Speculative
 * prefetch (AUX_CNT[29:28]) must stay off: with it on, the prefetcher
 * runs into the QSPI linear window during the clock reconfiguration and
 * causes a prefetch-abort storm (hardware verified). Demand linefills
 * are what makes the marginal 100 MHz QSPI XIP path reliable.
 */
static void l2_enable(void) {
    L2C_310->AUX_CNT &= ~((1UL << 29) | (1UL << 28)); /* prefetch off (board) */
    L2C_TAG_RAM_CNTRL = 0x111UL;                      /* official boot.S value */
    L2C_DATA_RAM_CNTRL = 0x121UL;                     /* official boot.S value */

    L2C_InvAllByWay(); /* invalidate all ways (8-way here: AUX_CNT[16] = 0) */
    __DSB();

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->L2C_RAM = 0x00020202UL;
    SLCR->LOCK = SLCR_LOCK_KEY;

    L2C_310->CONTROL = 1UL;
    __DSB();
}

/**
 * @brief  Activate the translation table, L2, MMU, L1 caches and branch
 *         prediction.
 * @param  ttb Base of the table filled by mmu_init(); 16 KB aligned.
 *
 * Caches and the branch predictor were invalidated by reset_handler
 * while still disabled.
 */
static void mmu_enable(uint32_t* ttb) {
    uint32_t v;

    // TTBR0 = table, DACR: all domains client. ACTLR: SMP bit + cache/TLB
    // maintenance broadcast (required before MMU/caches), as the official
    // boot.S does. All CP15 access via CMSIS.
    __DSB();
    __set_TTBR0((uint32_t)(uintptr_t)ttb);
    __set_DACR(0x55555555u);

    __set_ACTLR(__get_ACTLR() | ACTLR_SMP_Msk | (1UL << 0));

    // SCU: flush the duplicate tag store, then enable.
    MPCORE->SCU_INVALIDATE = 0xFFFFU;
    MPCORE->SCU_CONTROL |= 1UL;

    // L2 on before MMU/L1: the QSPI reconfiguration window later runs
    // fully covered by L1+L2.
    l2_enable();

    // ACTLR[2]: L1 instruction prefetch.
    __set_ACTLR(__get_ACTLR() | (1UL << 2));

    v = __get_SCTLR();
    v |= SCTLR_M_Msk     // M: enable MMU
         | SCTLR_C_Msk   // C: enable D cache
         | SCTLR_Z_Msk   // Z: enable branch prediction
         | SCTLR_I_Msk;  // I: enable I cache
    __set_SCTLR(v);
    __ISB();
}

/**
 * @brief  Bring up everything the XIP image needs to run at speed.
 *
 * The order is a hard constraint: page table and MMU/L2 first so the
 * caches cover the flash window, then warm the image into L2, and only
 * then reconfigure the PLLs and the QSPI controller from OCM. Doing the
 * clock work with the MMU or the caches off, or straight from flash,
 * hangs the bus.
 */
void soc_init(void) {
    mmu_init(mmu_table);
    mmu_enable(mmu_table);
    xip_warm_cache();

    sys_clock_init();
    qspi_clock_init();
}
