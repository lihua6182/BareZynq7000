/**
 * @file zynq_soc.c
 * @brief Boot-time MMU, cache, clock and QSPI setup for XIP (no FSBL;
 *        the soc_init() order is a hard constraint - the CPU fetches
 *        from flash the whole time).
 */

#include "zynq_soc.h"

#include "zynq_irq.h"
#include "zynq_tick.h"

#define MMU_TABLE_ENTRIES (0x1000U) /* 4096 x 1 MB sections cover the full 4 GB */

/* Page table in non-cacheable high OCM (0xFFFF0000): the CA9 walker is
   not D-cache coherent. */
static uint32_t mmu_table[MMU_TABLE_ENTRIES] __attribute__((aligned(16384), section(".mmu_table")));

#define TT_SECTION          (0x2U)                      /* bits[1:0] == 10: 1 MB section */
#define TT_DOMAIN_0         (0x0U << 5)                 /* domain 0 */
#define TT_XN               (1U << 4)                   /* execute never */
#define TT_AP_RW            (0x2U << 10)                /* AP[2:0] == 010: RW (PL1 and PL0) */
#define TT_AP_RO            ((0x2U << 10) | (1U << 15)) /* AP[2:0] == 110: RO (PL1 and PL0) */
#define TT_STRONGLY_ORDERED (0U)                        /* TEX=000, C=0, B=0 */
#define TT_WRITE_BACK_ALLOC                                                                                            \
    ((1U << 12) | (1U << 3) | (1U << 2)) /* TEX=001, C=1, B=1: normal, write-back, write-allocate */
#define TT_NON_CACHEABLE (1U << 12)      /* TEX=001, C=0, B=0: normal, non-cacheable */

#define TT_ENTRY(phys_mb, attr) ((((uint32_t)(phys_mb)) << 20) | (attr) | TT_DOMAIN_0 | TT_SECTION)

/* Must run from OCM during the QSPI/clock reconfiguration window.
   sys_clock_init speeds CPU_1x to 111 MHz while QSPI is still the
   BootROM ~33 MHz ref (ch.12 ratio violated until qspi_reconfig
   finishes). Call both only via sys_clock_and_qspi_reconfig so the
   CPU never returns to flash in that window. */
static void qspi_reconfig(void) __attribute__((section(".fast"), noinline));
static void sys_clock_restore(uint32_t arm_ctrl, uint32_t arm_cfg, uint32_t arm_clk, uint32_t io_ctrl, uint32_t io_cfg,
                              bool io_changed) __attribute__((section(".fast"), noinline));
static bool sys_clock_init(void) __attribute__((section(".fast"), noinline));
static void sys_clock_and_qspi_reconfig(void) __attribute__((section(".fast"), noinline));

/* PLL lock wait: the loop runs from .fast in OCM, ~10 us at 667 MHz. */
#define PLL_LOCK_POLL_LIMIT (100000U)

/**
 * @brief  Fill the translation table with 1 MB sections (16 KB aligned).
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
 * 0xFB0/0xFB1 alias PA 0x0; DMA must use high OCM (other bus masters
 * skip the MMU and see DDR). L2 comes up in mmu_and_caches_enable().
 */
static void mmu_init(uint32_t *table)
{
    uint32_t i;

    /* unmapped: any access triggers a translation fault */
    for (i = 0; i < MMU_TABLE_ENTRIES; i++)
    {
        table[i] = 0;
    }

    /* Low 192 KB OCM, write-back allocate */
    table[0x000] = TT_ENTRY(0x000, TT_WRITE_BACK_ALLOC | TT_AP_RW);

    /* PL + IOP peripherals + PS system registers. DDR (0x001-0x3FF) left unmapped. */
    for (i = 0x400; i < 0xFA0u; i++)
    {
        table[i] = TT_ENTRY(i, TT_STRONGLY_ORDERED | TT_AP_RW | TT_XN);
    }

    /* CPU-only aliases of the low OCM (VA -> PA 0x0) */
    table[0xFB0] = TT_ENTRY(0x000, TT_STRONGLY_ORDERED | TT_AP_RW);
    table[0xFB1] = TT_ENTRY(0x000, TT_NON_CACHEABLE | TT_AP_RW);

    /* 16 MB QSPI flash, XIP read caching */
    for (i = 0xFC0; i < 0xFD0u; i++)
    {
        table[i] = TT_ENTRY(i, TT_WRITE_BACK_ALLOC | TT_AP_RO);
    }

    /* High 64 KB OCM (OCM3): MMU table + .no_cache_ram */
    table[0xFFF] = TT_ENTRY(0xFFF, TT_NON_CACHEABLE | TT_AP_RW);
}

/**
 * @brief  Reconfigure the QSPI clock and read capture for XIP: ref =
 *         IO PLL/5 = 200 MHz (> CPU_1x = 111 MHz, ch.12 ratio rule),
 *         SCLK = ref/8 = 25 MHz, direct pad sampling (LPBK_DLY_ADJ = 0).
 *
 * @note Leaves linear mode with both FIFOs empty (ch.12), or the
 *       autonomous prefetch corrupts in-flight data. Runs from OCM
 *       (.fast) - the CPU fetches from the flash being reconfigured.
 *       Direct sampling is mandatory on this board: SCLK above
 *       FQSPICLK2 (~40 MHz) would need the MIO8 feedback clock, which
 *       is neither routed nor pinmuxed, and the internal loopback
 *       sampling path is dead without it (measured 2026-10-07: reads
 *       correct at every legal config; loopback configs return zeros).
 */
static void qspi_reconfig(void)
{
    uint32_t lqspi_cr;
    uint32_t i;

    /* 1. Leave linear mode; CPU keeps running from OCM. Keep the saved
       word (BootROM opcode, LINEAR=1) to restore after the reset. */
    lqspi_cr = QSPI->LQSPI_CR;
    QSPI->LQSPI_CR = lqspi_cr & ~QSPI_LQSPI_CR_LINEAR_Msk;

    /* 2. Drain both FIFOs (ch.12). Rx: pop while RXNEMPTY. Tx: wait for
       TXOW (fill at or below watermark; empty after leaving linear). */
    for (i = 0U; i < 64U; i++)
    {
        if ((QSPI->SR & QSPI_IXR_RXNEMPTY_Msk) == 0U)
        {
            break;
        }
        (void)QSPI->RXD;
    }
    for (i = 0U; i < 64U; i++)
    {
        if ((QSPI->SR & QSPI_IXR_TXOW_Msk) != 0U)
        {
            break;
        }
    }

    /* 3. Ref clk to IO PLL/5, then pulse both resets (clears regs, Table 12-6). */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->LQSPI_CLK_CTRL = (5U << 8) | /* DIVISOR = 5                     */
                           (1U << 0);  /* CLKACT = 1, SRCSEL = 0x: IO PLL */

    SLCR->LQSPI_RST_CTRL = (1U << 1) | /* QSPI_REF_RST                    */
                           (1U << 0);  /* LQSPI_CPU1X_RST                 */
    SLCR->LQSPI_RST_CTRL = 0U;
    for (i = 0U; i < 100U; i++)
    {
        __asm volatile("nop");
    }
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* 4. Reprogram controller and re-enter linear with the saved BootROM
       LQSPI_CR (not the reset value 0x8000016B). CR = BootROM 0x800238C1
       with BAUD_RATE_DIV = 010 -> SCLK = ref/8 = 25 MHz. LPBK_DLY_ADJ = 0:
       direct pad sampling - MIO8 feedback is not routed, so loopback
       capture is dead and must not be enabled. */
    QSPI->CR = 0x800238D1U;
    QSPI->LPBK_DLY_ADJ = 0U;
    QSPI->LQSPI_CR = lqspi_cr; /* LINEAR=1, BootROM read opcode   */
    QSPI->ER = 1U;             /* SPI_EN = 1                     */

    /* Controller is SO; XIP window is WB. Drain the enable before any
       subsequent instruction fetch from 0xFC00_0000. */
    __DSB();
    __ISB();
}

/**
 * @brief  Put the BootROM clock tree back after a PLL lock failure.
 *
 * Runs from OCM (.fast) while the QSPI may still be on the pre-change
 * source. A PLL that was not in bypass originally is re-locked behind
 * its bypass bit, so the CPU never runs on an unlocked PLL. Best
 * effort: a PLL that still refuses to lock is left in bypass.
 * @param arm_ctrl   Original ARM_PLL_CTRL.
 * @param arm_cfg    Original ARM_PLL_CFG.
 * @param arm_clk    Original ARM_CLK_CTRL.
 * @param io_ctrl    Original IO_PLL_CTRL.
 * @param io_cfg     Original IO_PLL_CFG.
 * @param io_changed True when the IO PLL was reprogrammed before the failure.
 */
static void sys_clock_restore(uint32_t arm_ctrl, uint32_t arm_cfg, uint32_t arm_clk, uint32_t io_ctrl, uint32_t io_cfg,
                              bool io_changed)
{
    uint32_t retry;

    /* CPU back on the original mux/divider. The ARM PLL may still be
       locked here (IO-failure path); the branch below forces bypass
       before reprogramming it. */
    SLCR->ARM_CLK_CTRL = arm_clk;

    if ((arm_ctrl & (1U << 4)) == 0U)
    {
        SLCR->ARM_PLL_CTRL = arm_ctrl | (1U << 4);
        SLCR->ARM_PLL_CFG = arm_cfg;
        for (retry = 0U; retry < PLL_LOCK_POLL_LIMIT; retry++)
        {
            if ((SLCR->PLL_STATUS & 1U) != 0U)
            {
                break;
            }
        }
        SLCR->ARM_PLL_CTRL = (retry == PLL_LOCK_POLL_LIMIT) ? (arm_ctrl | (1U << 4)) : arm_ctrl;
    }
    else
    {
        SLCR->ARM_PLL_CTRL = arm_ctrl;
        SLCR->ARM_PLL_CFG = arm_cfg;
    }

    if (io_changed)
    {
        if ((io_ctrl & (1U << 4)) == 0U)
        {
            SLCR->IO_PLL_CTRL = io_ctrl | (1U << 4);
            SLCR->IO_PLL_CFG = io_cfg;
            for (retry = 0U; retry < PLL_LOCK_POLL_LIMIT; retry++)
            {
                if ((SLCR->PLL_STATUS & (1U << 2)) != 0U)
                {
                    break;
                }
            }
            SLCR->IO_PLL_CTRL = (retry == PLL_LOCK_POLL_LIMIT) ? (io_ctrl | (1U << 4)) : io_ctrl;
        }
        else
        {
            SLCR->IO_PLL_CTRL = io_ctrl;
            SLCR->IO_PLL_CFG = io_cfg;
        }
    }
}

/**
 * @brief  Bring up the ARM and IO PLLs and the CPU divider: ARM PLL FDIV
 *         40 -> CPU /2 = 667 MHz, IO PLL FDIV 30 -> QSPI ref /5 = 200 MHz
 *         (Table 25-3, 6:2:1). Per PLL: program, force bypass, pulse
 *         reset, wait lock, leave bypass. DDR PLL untouched.
 *
 * @note Runs from OCM (.fast) during the reconfiguration window. On a
 *       lock failure the pre-change clock tree is restored (both PLLs
 *       and the CPU divider), so the BootROM QSPI source and ratio stay
 *       valid.
 * @return true if both PLLs locked (QSPI may switch to IO PLL/5); false
 *         after a failure and restore (skip qspi_reconfig).
 */
static bool sys_clock_init(void)
{
    uint32_t arm_ctrl_orig;
    uint32_t arm_cfg_orig;
    uint32_t arm_clk_orig;
    uint32_t io_ctrl_orig;
    uint32_t io_cfg_orig;
    uint32_t arm_ctrl;
    uint32_t io_ctrl;
    uint32_t retry;

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;

    arm_ctrl_orig = SLCR->ARM_PLL_CTRL;
    arm_cfg_orig = SLCR->ARM_PLL_CFG;
    arm_clk_orig = SLCR->ARM_CLK_CTRL;
    io_ctrl_orig = SLCR->IO_PLL_CTRL;
    io_cfg_orig = SLCR->IO_PLL_CFG;

    /* ARM PLL FDIV 40, CFG 250/2/2 (Table 25-6). */
    arm_ctrl = (arm_ctrl_orig & ~(0x7FU << 12)) | (40UL << 12);
    SLCR->ARM_PLL_CTRL = arm_ctrl;

    SLCR->ARM_PLL_CFG = ((uint32_t)250UL << 12) | ((uint32_t)2UL << 8) | ((uint32_t)2UL << 4);

    SLCR->ARM_PLL_CTRL = arm_ctrl | (1U << 4);             /* force bypass     */
    SLCR->ARM_PLL_CTRL = arm_ctrl | (1U << 4) | (1U << 0); /* assert PLL_RESET */
    SLCR->ARM_PLL_CTRL = arm_ctrl | (1U << 4);             /* de-assert reset  */

    for (retry = 0U; retry < PLL_LOCK_POLL_LIMIT; retry++)
    {
        if ((SLCR->PLL_STATUS & 1U) != 0U)
        { /* ARM_PLL_LOCK */
            break;
        }
    }
    if (retry == PLL_LOCK_POLL_LIMIT)
    {
        /* ARM PLL failed: restore the BootROM ARM/CPU state, skip QSPI. */
        sys_clock_restore(arm_ctrl_orig, arm_cfg_orig, arm_clk_orig, io_ctrl_orig, io_cfg_orig, false);
        SLCR->LOCK = SLCR_LOCK_KEY;
        return false;
    }

    SLCR->ARM_PLL_CTRL = arm_ctrl; /* leave bypass */

    /* CPU = ARM PLL / 2. */
    SLCR->ARM_CLK_CTRL = 0x1F000200U;

    /* IO PLL FDIV 30 -> 1000 MHz, CFG 325/2/12 (Table 25-6). */
    io_ctrl = (io_ctrl_orig & ~(0x7FU << 12)) | (30UL << 12);
    SLCR->IO_PLL_CTRL = io_ctrl;

    SLCR->IO_PLL_CFG = (325UL << 12) | /* LOCK_CNT = 325 */
                       (2UL << 8) |    /* CP       = 2  */
                       (12UL << 4);    /* RES      = 12 */

    SLCR->IO_PLL_CTRL = io_ctrl | (1U << 4);             /* force bypass     */
    SLCR->IO_PLL_CTRL = io_ctrl | (1U << 4) | (1U << 0); /* assert PLL_RESET */
    SLCR->IO_PLL_CTRL = io_ctrl | (1U << 4);             /* de-assert reset  */

    for (retry = 0U; retry < PLL_LOCK_POLL_LIMIT; retry++)
    {
        if ((SLCR->PLL_STATUS & (1U << 2)) != 0U)
        { /* IO_PLL_LOCK */
            break;
        }
    }
    if (retry == PLL_LOCK_POLL_LIMIT)
    {
        /* IO PLL failed: put the ARM PLL/CPU divider back, then skip
           qspi_reconfig. The BootROM QSPI source is restored with it. */
        sys_clock_restore(arm_ctrl_orig, arm_cfg_orig, arm_clk_orig, io_ctrl_orig, io_cfg_orig, true);
        SLCR->LOCK = SLCR_LOCK_KEY;
        return false;
    }

    SLCR->IO_PLL_CTRL = io_ctrl; /* leave bypass */

    SLCR->LOCK = SLCR_LOCK_KEY;
    return true;
}

/* TAG/DATA RAM latency regs (0x108/0x10C) are not in the CMSIS struct. */
#define L2C_TAG_RAM_CNTRL  ((*((__IO uint32_t *)(uintptr_t)L2C_310_BASE + (0x108U / 4U))))
#define L2C_DATA_RAM_CNTRL ((*((__IO uint32_t *)(uintptr_t)L2C_310_BASE + (0x10CU / 4U))))

/**
 * @brief  Bring up the PL310: official boot.S sequence - configure while
 *         disabled, invalidate by way, SLCR L2C_RAM = 0x202 (AR#54190),
 *         enable. Speculative prefetch (AUX_CNT[29:28]) stays off or the
 *         prefetcher kills the flash during QSPI reconfig (verified on
 *         this board); demand linefills make 25 MHz XIP reliable.
 */
static void l2_cache_enable(void)
{
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
 *         prediction (L2 before MMU/L1). PLL/QSPI reconfig then runs from
 *         OCM; after it returns, XIP is covered by L1+L2 at 25 MHz.
 *         Caches were invalidated by reset_handler while still disabled.
 * @param  table Base of the table filled by mmu_init(); 16 KB aligned.
 */
static void mmu_and_caches_enable(uint32_t *table)
{
    uint32_t sctlr;

    /* TTBR0 = table; DACR all-client; ACTLR SMP + maintenance broadcast. */
    __DSB();
    __set_TTBR0((uint32_t)(uintptr_t)table);
    __set_DACR(0x55555555u);

    __set_ACTLR(__get_ACTLR() | ACTLR_SMP_Msk | (1UL << 0));

    /* SCU: flush the duplicate tag store, then enable. */
    MPCORE->SCU_INVALIDATE = 0xFFFFU;
    MPCORE->SCU_CONTROL |= 1UL;

    /* L2 on before MMU/L1: demand fills cover XIP after the OCM window. */
    l2_cache_enable();

    /* ACTLR[2]: L1 instruction prefetch. */
    __set_ACTLR(__get_ACTLR() | (1UL << 2));

    sctlr = __get_SCTLR();
    sctlr |= SCTLR_M_Msk    /* M: enable MMU */
             | SCTLR_C_Msk  /* C: enable D cache */
             | SCTLR_Z_Msk  /* Z: enable branch prediction */
             | SCTLR_I_Msk; /* I: enable I cache */
    __set_SCTLR(sctlr);
    __ISB();
}

/**
 * @brief  Raise CPU clocks then reconfigure QSPI, without returning to
 *         flash in between. After sys_clock_init, CPU_1x is 111 MHz while
 *         QSPI is still the BootROM ref (ratio violated); qspi_reconfig
 *         must run from this same OCM call. A PLL failure restores the
 *         pre-change clock tree, so the skip path returns to the BootROM
 *         QSPI source instead of a violated ratio.
 */
static void sys_clock_and_qspi_reconfig(void)
{
    if (sys_clock_init())
    {
        qspi_reconfig();
    }

    /* The skip path restored the PLLs; drain before Flash. */
    __DSB();
    __ISB();
}

/**
 * @brief  Bring up everything the XIP image needs: MMU/L2 first (BootROM
 *         QSPI still valid), then PLL+QSPI from one OCM call, then GIC
 *         and tick. Any other order hangs the bus.
 */
void soc_init(void)
{
    mmu_init(mmu_table);
    mmu_and_caches_enable(mmu_table);

    sys_clock_and_qspi_reconfig();

    irq_init();
    tick_init();
}
