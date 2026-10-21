#include "zynq_clock.h"

static uint32_t pll_freq_from_ctrl(uint32_t ctrl)
{
    uint32_t fdiv = (ctrl >> 12) & 0x7FU;

    if (fdiv == 0U || (ctrl & (1UL << 4)) != 0U)
    {
        return PS_CLK_HZ;
    }
    return (uint32_t)((uint64_t)PS_CLK_HZ * (uint64_t)fdiv);
}

static uint32_t periph_src_freq(uint32_t srcsel, const struct clock_freqs *freqs)
{
    switch (srcsel)
    {
    case 2U:
        return freqs->arm_pll;
    case 3U:
        return freqs->ddr_pll;
    default:
        return freqs->io_pll;
    }
}

static uint32_t cpu_src_freq(uint32_t srcsel, const struct clock_freqs *freqs)
{
    switch (srcsel)
    {
    case 2U:
        return freqs->ddr_pll;
    case 3U:
        return freqs->io_pll;
    default:
        return freqs->arm_pll;
    }
}

static uint32_t divisor_from_field(uint32_t ctrl, uint32_t shift)
{
    uint32_t divisor = (ctrl >> shift) & 0x3FU;

    return (divisor != 0U) ? divisor : 1U;
}

static uint32_t periph_ref_freq(uint32_t ctrl, const struct clock_freqs *freqs)
{
    uint32_t src = periph_src_freq((ctrl >> 4) & 0x3U, freqs);

    return src / divisor_from_field(ctrl, 8U);
}

static uint32_t periph_ref_freq_cascaded(uint32_t ctrl, const struct clock_freqs *freqs)
{
    uint32_t src = periph_src_freq((ctrl >> 4) & 0x3U, freqs);

    return src / divisor_from_field(ctrl, 8U) / divisor_from_field(ctrl, 20U);
}

void clock_freqs_get(struct clock_freqs *freqs)
{
    uint32_t ctrl;
    uint32_t ratio_621;

    freqs->ps_clk = PS_CLK_HZ;
    freqs->arm_pll = pll_freq_from_ctrl(slcr->arm_pll_ctrl);
    freqs->ddr_pll = pll_freq_from_ctrl(slcr->ddr_pll_ctrl);
    freqs->io_pll = pll_freq_from_ctrl(slcr->io_pll_ctrl);

    ctrl = slcr->arm_clk_ctrl;
    freqs->cpu = cpu_src_freq((ctrl >> 4) & 0x3U, freqs) / divisor_from_field(ctrl, 8U);

    ratio_621 = slcr->clk_621_true & 1UL;
    freqs->cpu_6x4x = freqs->cpu;
    freqs->cpu_3x2x = freqs->cpu / 2UL;
    freqs->cpu_2x = freqs->cpu / (2UL + ratio_621);
    freqs->cpu_1x = freqs->cpu / (4UL + 2UL * ratio_621);

    ctrl = slcr->ddr_clk_ctrl;
    freqs->ddr_2x = freqs->ddr_pll / divisor_from_field(ctrl, 26U);
    freqs->ddr_3x = freqs->ddr_pll / divisor_from_field(ctrl, 20U);

    freqs->uart_ref = periph_ref_freq(slcr->uart_clk_ctrl, freqs);
    freqs->qspi_ref = periph_ref_freq(slcr->lqspi_clk_ctrl, freqs);
    freqs->sdio_ref = periph_ref_freq(slcr->sdio_clk_ctrl, freqs);
    freqs->spi_ref = periph_ref_freq(slcr->spi_clk_ctrl, freqs);
    freqs->dbg_ref = periph_ref_freq(slcr->dbg_clk_ctrl, freqs);
    freqs->pcap_ref = periph_ref_freq(slcr->pcap_clk_ctrl, freqs);

    freqs->gem0_ref = periph_ref_freq_cascaded(slcr->gem0_clk_ctrl, freqs);
    freqs->gem1_ref = periph_ref_freq_cascaded(slcr->gem1_clk_ctrl, freqs);
    freqs->can_ref = periph_ref_freq_cascaded(slcr->can_clk_ctrl, freqs);
    freqs->fpga0_ref = periph_ref_freq_cascaded(slcr->fpga0_clk_ctrl, freqs);
    freqs->fpga1_ref = periph_ref_freq_cascaded(slcr->fpga1_clk_ctrl, freqs);
    freqs->fpga2_ref = periph_ref_freq_cascaded(slcr->fpga2_clk_ctrl, freqs);
    freqs->fpga3_ref = periph_ref_freq_cascaded(slcr->fpga3_clk_ctrl, freqs);

    freqs->smc_ref = freqs->cpu_1x;
}

static volatile uint32_t *fclk_reg(uint32_t fclk)
{
    switch (fclk)
    {
    case 0U:
        return &slcr->fpga0_clk_ctrl;
    case 1U:
        return &slcr->fpga1_clk_ctrl;
    case 2U:
        return &slcr->fpga2_clk_ctrl;
    default:
        return &slcr->fpga3_clk_ctrl;
    }
}

bool clock_fclk_config(enum fclk fclk, enum fclk_src src, uint32_t div0, uint32_t div1, bool enable)
{
    volatile uint32_t *clk_ctrl;
    uint32_t value;

    if (((uint32_t)fclk > 3U) || ((src != FCLK_SRC_IO) && (src != FCLK_SRC_ARM) && (src != FCLK_SRC_DDR)) ||
        (div0 == 0U) || (div0 > 63U) || (div1 == 0U) || (div1 > 63U))
    {
        return false;
    }

    clk_ctrl = fclk_reg((uint32_t)fclk);

    slcr->unlock = SLCR_UNLOCK_KEY;
    value = *clk_ctrl & ~((0x3FUL << 8) | (0x3FUL << 20) | (0x3UL << 4) | 0x1UL);
    value |= (div0 << 8) | (div1 << 20) | ((uint32_t)src << 4) | (enable ? 0x1UL : 0x0UL);
    *clk_ctrl = value;
    slcr->lock = SLCR_LOCK_KEY;

    return true;
}

void clock_fclk_enable(enum fclk fclk, bool enable)
{
    volatile uint32_t *clk_ctrl;
    uint32_t value;

    if ((uint32_t)fclk > 3U)
    {
        return;
    }
    clk_ctrl = fclk_reg((uint32_t)fclk);

    slcr->unlock = SLCR_UNLOCK_KEY;
    value = *clk_ctrl & ~0x1UL;
    if (enable)
    {
        value |= 0x1UL;
    }
    *clk_ctrl = value;
    slcr->lock = SLCR_LOCK_KEY;
}

uint32_t clock_fclk_freq_get(enum fclk fclk)
{
    volatile uint32_t *clk_ctrl;
    struct clock_freqs freqs;

    if ((uint32_t)fclk > 3U)
    {
        return 0U;
    }
    clk_ctrl = fclk_reg((uint32_t)fclk);
    if ((*clk_ctrl & 0x1UL) == 0U)
    {
        return 0U;
    }

    freqs.arm_pll = pll_freq_from_ctrl(slcr->arm_pll_ctrl);
    freqs.ddr_pll = pll_freq_from_ctrl(slcr->ddr_pll_ctrl);
    freqs.io_pll = pll_freq_from_ctrl(slcr->io_pll_ctrl);
    return periph_ref_freq_cascaded(*clk_ctrl, &freqs);
}
