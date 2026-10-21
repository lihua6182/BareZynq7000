#include "zynq_soc.h"

#include "zynq_irq.h"
#include "zynq_tick.h"

#define MMU_TABLE_ENTRIES (0x1000U)

static uint32_t mmu_table[MMU_TABLE_ENTRIES] __attribute__((aligned(16384), section(".mmu_table")));

#define TT_SECTION  (0x2U)
#define TT_DOMAIN_0 (0x0U << 5)
#define TT_XN       (1U << 4)

#define TT_AP_RW            (0x2U << 10)
#define TT_AP_RO            ((0x2U << 10) | (1U << 15))
#define TT_STRONGLY_ORDERED (0U)
#define TT_WRITE_BACK_ALLOC ((1U << 12) | (1U << 3) | (1U << 2))
#define TT_NON_CACHEABLE    (1U << 12)

#define TT_ENTRY(phys_mb, attr) ((((uint32_t)(phys_mb)) << 20) | (attr) | TT_DOMAIN_0 | TT_SECTION)

#define SCTLR_M_Msk   (1UL << 0)
#define SCTLR_C_Msk   (1UL << 2)
#define SCTLR_Z_Msk   (1UL << 11)
#define SCTLR_I_Msk   (1UL << 12)
#define ACTLR_SMP_Msk (1UL << 6)

static inline uint32_t sctlr_read(void)
{
    uint32_t v;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(v));
    return v;
}

static inline void sctlr_write(uint32_t v)
{
    asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(v) : "memory");
}

static inline uint32_t actlr_read(void)
{
    uint32_t v;
    asm volatile("mrc p15, 0, %0, c1, c0, 1" : "=r"(v));
    return v;
}

static inline void actlr_write(uint32_t v)
{
    asm volatile("mcr p15, 0, %0, c1, c0, 1" : : "r"(v) : "memory");
}

static inline void ttbr0_write(uint32_t v)
{
    asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(v) : "memory");
}

static inline void dacr_write(uint32_t v)
{
    asm volatile("mcr p15, 0, %0, c3, c0, 0" : : "r"(v) : "memory");
}

static void qspi_reconfig(void) __attribute__((section(".fast"), noinline));
static void sys_clock_restore(uint32_t arm_ctrl, uint32_t arm_cfg, uint32_t arm_clk, uint32_t io_ctrl, uint32_t io_cfg,
                              bool io_changed) __attribute__((section(".fast"), noinline));
static bool sys_clock_init(void) __attribute__((section(".fast"), noinline));
static void sys_clock_and_qspi_reconfig(void) __attribute__((section(".fast"), noinline));

#define PLL_LOCK_POLL_LIMIT (100000U)

static void mmu_init(uint32_t *table)
{
    uint32_t i;

    for (i = 0; i < MMU_TABLE_ENTRIES; i++)
    {
        table[i] = 0;
    }

    table[0x000] = TT_ENTRY(0x000, TT_WRITE_BACK_ALLOC | TT_AP_RW);

    for (i = 0x400; i < 0xFA0u; i++)
    {
        table[i] = TT_ENTRY(i, TT_STRONGLY_ORDERED | TT_AP_RW | TT_XN);
    }

    table[0xFB0] = TT_ENTRY(0x000, TT_STRONGLY_ORDERED | TT_AP_RW);
    table[0xFB1] = TT_ENTRY(0x000, TT_NON_CACHEABLE | TT_AP_RW);

    for (i = 0xFC0; i < 0xFD0u; i++)
    {
        table[i] = TT_ENTRY(i, TT_WRITE_BACK_ALLOC | TT_AP_RO);
    }

    table[0xFFF] = TT_ENTRY(0xFFF, TT_NON_CACHEABLE | TT_AP_RW);
}

static void qspi_reconfig(void)
{
    uint32_t lqspi_cr;
    uint32_t i;

    lqspi_cr = qspi->lqspi_cr;
    qspi->lqspi_cr = lqspi_cr & ~QSPI_LQSPI_CR_LINEAR_Msk;

    for (i = 0U; i < 64U; i++)
    {
        if ((qspi->sr & QSPI_IXR_RXNEMPTY_Msk) == 0U)
        {
            break;
        }
        (void)qspi->rxd;
    }
    for (i = 0U; i < 64U; i++)
    {
        if ((qspi->sr & QSPI_IXR_TXOW_Msk) != 0U)
        {
            break;
        }
    }

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->lqspi_clk_ctrl = (5U << 8) | (1U << 0);

    slcr->lqspi_rst_ctrl = (1U << 1) | (1U << 0);
    slcr->lqspi_rst_ctrl = 0U;
    for (i = 0U; i < 100U; i++)
    {
        asm volatile("nop");
    }
    slcr->lock = SLCR_LOCK_KEY;

    qspi->cr = 0x800238D1U;
    qspi->lpbk_dly_adj = 0U;
    qspi->lqspi_cr = lqspi_cr;
    qspi->er = 1U;

    dsb();
    isb();
}

static void sys_clock_restore(uint32_t arm_ctrl, uint32_t arm_cfg, uint32_t arm_clk, uint32_t io_ctrl, uint32_t io_cfg,
                              bool io_changed)
{
    uint32_t retry;

    slcr->arm_clk_ctrl = arm_clk;

    if ((arm_ctrl & (1U << 4)) == 0U)
    {
        slcr->arm_pll_ctrl = arm_ctrl | (1U << 4);
        slcr->arm_pll_cfg = arm_cfg;
        for (retry = 0U; retry < PLL_LOCK_POLL_LIMIT; retry++)
        {
            if ((slcr->pll_status & 1U) != 0U)
            {
                break;
            }
        }
        slcr->arm_pll_ctrl = (retry == PLL_LOCK_POLL_LIMIT) ? (arm_ctrl | (1U << 4)) : arm_ctrl;
    }
    else
    {
        slcr->arm_pll_ctrl = arm_ctrl;
        slcr->arm_pll_cfg = arm_cfg;
    }

    if (io_changed)
    {
        if ((io_ctrl & (1U << 4)) == 0U)
        {
            slcr->io_pll_ctrl = io_ctrl | (1U << 4);
            slcr->io_pll_cfg = io_cfg;
            for (retry = 0U; retry < PLL_LOCK_POLL_LIMIT; retry++)
            {
                if ((slcr->pll_status & (1U << 2)) != 0U)
                {
                    break;
                }
            }
            slcr->io_pll_ctrl = (retry == PLL_LOCK_POLL_LIMIT) ? (io_ctrl | (1U << 4)) : io_ctrl;
        }
        else
        {
            slcr->io_pll_ctrl = io_ctrl;
            slcr->io_pll_cfg = io_cfg;
        }
    }
}

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

    slcr->unlock = SLCR_UNLOCK_KEY;

    arm_ctrl_orig = slcr->arm_pll_ctrl;
    arm_cfg_orig = slcr->arm_pll_cfg;
    arm_clk_orig = slcr->arm_clk_ctrl;
    io_ctrl_orig = slcr->io_pll_ctrl;
    io_cfg_orig = slcr->io_pll_cfg;

    arm_ctrl = (arm_ctrl_orig & ~(0x7FU << 12)) | (40UL << 12);
    slcr->arm_pll_ctrl = arm_ctrl;

    slcr->arm_pll_cfg = ((uint32_t)250UL << 12) | ((uint32_t)2UL << 8) | ((uint32_t)2UL << 4);

    slcr->arm_pll_ctrl = arm_ctrl | (1U << 4);
    slcr->arm_pll_ctrl = arm_ctrl | (1U << 4) | (1U << 0);
    slcr->arm_pll_ctrl = arm_ctrl | (1U << 4);

    for (retry = 0U; retry < PLL_LOCK_POLL_LIMIT; retry++)
    {
        if ((slcr->pll_status & 1U) != 0U)
        {
            break;
        }
    }
    if (retry == PLL_LOCK_POLL_LIMIT)
    {

        sys_clock_restore(arm_ctrl_orig, arm_cfg_orig, arm_clk_orig, io_ctrl_orig, io_cfg_orig, false);
        slcr->lock = SLCR_LOCK_KEY;
        return false;
    }

    slcr->arm_pll_ctrl = arm_ctrl;

    slcr->arm_clk_ctrl = 0x1F000200U;

    io_ctrl = (io_ctrl_orig & ~(0x7FU << 12)) | (30UL << 12);
    slcr->io_pll_ctrl = io_ctrl;

    slcr->io_pll_cfg = (325UL << 12) | (2UL << 8) | (12UL << 4);

    slcr->io_pll_ctrl = io_ctrl | (1U << 4);
    slcr->io_pll_ctrl = io_ctrl | (1U << 4) | (1U << 0);
    slcr->io_pll_ctrl = io_ctrl | (1U << 4);

    for (retry = 0U; retry < PLL_LOCK_POLL_LIMIT; retry++)
    {
        if ((slcr->pll_status & (1U << 2)) != 0U)
        {
            break;
        }
    }
    if (retry == PLL_LOCK_POLL_LIMIT)
    {

        sys_clock_restore(arm_ctrl_orig, arm_cfg_orig, arm_clk_orig, io_ctrl_orig, io_cfg_orig, true);
        slcr->lock = SLCR_LOCK_KEY;
        return false;
    }

    slcr->io_pll_ctrl = io_ctrl;

    slcr->lock = SLCR_LOCK_KEY;
    return true;
}

static void l2_cache_invalidate_all(void)
{
    uint32_t ways = ((l2c->aux_cnt & (1UL << 16)) != 0U) ? 16U : 8U;
    uint32_t mask = (1UL << ways) - 1U;

    l2c->inv_way = mask;
    while ((l2c->inv_way & mask) != 0U)
    {
    }
    l2c->cache_sync = 0U;
}

static void l2_cache_enable(void)
{
    l2c->aux_cnt &= ~((1UL << 29) | (1UL << 28));
    l2c->tag_ram_cntrl = 0x111UL;
    l2c->data_ram_cntrl = 0x121UL;

    l2_cache_invalidate_all();
    dsb();

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->l2c_ram = 0x00020202UL;
    slcr->lock = SLCR_LOCK_KEY;

    l2c->control = 1UL;
    dsb();
}

static void mmu_and_caches_enable(uint32_t *table)
{
    uint32_t sctlr;

    dsb();
    ttbr0_write((uint32_t)(uintptr_t)table);
    dacr_write(0x55555555U);

    actlr_write(actlr_read() | ACTLR_SMP_Msk | (1UL << 0));

    mpcore->scu_invalidate = 0xFFFFU;
    mpcore->scu_control |= 1UL;

    l2_cache_enable();

    actlr_write(actlr_read() | (1UL << 2));

    sctlr = sctlr_read();
    sctlr |= SCTLR_M_Msk | SCTLR_C_Msk | SCTLR_Z_Msk | SCTLR_I_Msk;
    sctlr_write(sctlr);
    isb();
}

static void sys_clock_and_qspi_reconfig(void)
{
    if (sys_clock_init())
    {
        qspi_reconfig();
    }

    dsb();
    isb();
}

void soc_init(void)
{
    mmu_init(mmu_table);
    mmu_and_caches_enable(mmu_table);

    sys_clock_and_qspi_reconfig();

    irq_init();
    tick_init();
}
