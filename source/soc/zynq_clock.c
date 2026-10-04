/**
 * @file zynq_clock.c
 * @brief SLCR clock tree readback and PL fabric clock setup (UG585 ch.25).
 */

#include "zynq_clock.h"

/**
 * @brief  PLL output frequency from its control register.
 * @param  ctrl Value of ARM/DDR/IO_PLL_CTRL (FDIV [18:12], bypass [4]).
 * @return PS_CLK x FDIV, or PS_CLK while bypass is forced.
 */
static uint32_t pll_freq_from_ctrl(uint32_t ctrl) {
    uint32_t fdiv = (ctrl >> 12) & 0x7FU;

    if (fdiv == 0U || (ctrl & (1UL << 4)) != 0U) {
        return PS_CLK_HZ;
    }
    return (uint32_t)((uint64_t)PS_CLK_HZ * (uint64_t)fdiv);
}

/**
 * @brief  Peripheral clock mux: SRCSEL[5:4] 0x = IO, 2 = ARM, 3 = DDR PLL.
 */
static uint32_t periph_src_freq(uint32_t srcsel, const struct clock_freqs* freqs) {
    switch (srcsel) {
        case 2U:
            return freqs->arm_pll;
        case 3U:
            return freqs->ddr_pll;
        default:
            return freqs->io_pll;
    }
}

/**
 * @brief  CPU clock mux: SRCSEL[5:4] 0x = ARM, 2 = DDR, 3 = IO PLL.
 */
static uint32_t cpu_src_freq(uint32_t srcsel, const struct clock_freqs* freqs) {
    switch (srcsel) {
        case 2U:
            return freqs->ddr_pll;
        case 3U:
            return freqs->io_pll;
        default:
            return freqs->arm_pll;
    }
}

/**
 * @brief  Decode a one-based 6-bit divisor field; 0 bypasses (returns 1).
 */
static uint32_t divisor_from_field(uint32_t ctrl, uint32_t shift) {
    uint32_t divisor = (ctrl >> shift) & 0x3FU;

    return (divisor != 0U) ? divisor : 1U;
}

/**
 * @brief  Peripheral clock with one divider: mux / DIVISOR[13:8].
 */
static uint32_t periph_ref_freq(uint32_t ctrl, const struct clock_freqs* freqs) {
    uint32_t src = periph_src_freq((ctrl >> 4) & 0x3U, freqs);

    return src / divisor_from_field(ctrl, 8U);
}

/**
 * @brief  Peripheral clock with cascaded dividers DIV0[13:8] / DIV1[25:20]
 *         (GEM, CAN, FCLKs).
 */
static uint32_t periph_ref_freq_cascaded(uint32_t ctrl, const struct clock_freqs* freqs) {
    uint32_t src = periph_src_freq((ctrl >> 4) & 0x3U, freqs);

    return src / divisor_from_field(ctrl, 8U) / divisor_from_field(ctrl, 20U);
}

/**
 * @brief  Read the whole clock tree from the live SLCR values; stays
 *         correct after sys_clock_init() reprogrammed the PLLs.
 * @param  freqs Out: every frequency in struct clock_freqs.
 */
void clock_freqs_get(struct clock_freqs* freqs) {
    uint32_t ctrl;
    uint32_t ratio_621;

    freqs->ps_clk = PS_CLK_HZ;
    freqs->arm_pll = pll_freq_from_ctrl(SLCR->ARM_PLL_CTRL);
    freqs->ddr_pll = pll_freq_from_ctrl(SLCR->DDR_PLL_CTRL);
    freqs->io_pll = pll_freq_from_ctrl(SLCR->IO_PLL_CTRL);

    /* CPU = ARM_CLK_CTRL mux / DIVISOR[13:8]. */
    ctrl = SLCR->ARM_CLK_CTRL;
    freqs->cpu = cpu_src_freq((ctrl >> 4) & 0x3U, freqs) / divisor_from_field(ctrl, 8U);

    /* CPU_2x/1x depend on the 6:2:1 ratio mode (CLK_621_TRUE). */
    ratio_621 = SLCR->CLK_621_TRUE & 1UL;
    freqs->cpu_6x4x = freqs->cpu;
    freqs->cpu_3x2x = freqs->cpu / 2UL;
    freqs->cpu_2x = freqs->cpu / (2UL + ratio_621);
    freqs->cpu_1x = freqs->cpu / (4UL + 2UL * ratio_621);

    /* DDR domain: divisors DDR_CLK_CTRL[31:26] (2x) and [25:20] (3x). */
    ctrl = SLCR->DDR_CLK_CTRL;
    freqs->ddr_2x = freqs->ddr_pll / divisor_from_field(ctrl, 26U);
    freqs->ddr_3x = freqs->ddr_pll / divisor_from_field(ctrl, 20U);

    freqs->uart_ref = periph_ref_freq(SLCR->UART_CLK_CTRL, freqs);
    freqs->qspi_ref = periph_ref_freq(SLCR->LQSPI_CLK_CTRL, freqs);
    freqs->sdio_ref = periph_ref_freq(SLCR->SDIO_CLK_CTRL, freqs);
    freqs->spi_ref = periph_ref_freq(SLCR->SPI_CLK_CTRL, freqs);
    freqs->dbg_ref = periph_ref_freq(SLCR->DBG_CLK_CTRL, freqs);
    freqs->pcap_ref = periph_ref_freq(SLCR->PCAP_CLK_CTRL, freqs);

    freqs->gem0_ref = periph_ref_freq_cascaded(SLCR->GEM0_CLK_CTRL, freqs);
    freqs->gem1_ref = periph_ref_freq_cascaded(SLCR->GEM1_CLK_CTRL, freqs);
    freqs->can_ref = periph_ref_freq_cascaded(SLCR->CAN_CLK_CTRL, freqs);
    freqs->fpga0_ref = periph_ref_freq_cascaded(SLCR->FPGA0_CLK_CTRL, freqs);
    freqs->fpga1_ref = periph_ref_freq_cascaded(SLCR->FPGA1_CLK_CTRL, freqs);
    freqs->fpga2_ref = periph_ref_freq_cascaded(SLCR->FPGA2_CLK_CTRL, freqs);
    freqs->fpga3_ref = periph_ref_freq_cascaded(SLCR->FPGA3_CLK_CTRL, freqs);

    /* The SMC runs on a gated CPU_1x (SLCR SMC_CLK_CTRL holds only CLKACT). */
    freqs->smc = freqs->cpu_1x;
}

/**
 * @brief  SLCR.FPGAn_CLK_CTRL of one FCLK route (the four CLK_CTRLs are
 *         not contiguous: each route is {CLK_CTRL, THR_CTRL, THR_CNT,
 *         THR_STA}).
 */
static volatile uint32_t* fclk_reg(uint32_t fclk) {
    switch (fclk) {
        case 0U:
            return &SLCR->FPGA0_CLK_CTRL;
        case 1U:
            return &SLCR->FPGA1_CLK_CTRL;
        case 2U:
            return &SLCR->FPGA2_CLK_CTRL;
        default:
            return &SLCR->FPGA3_CLK_CTRL;
    }
}

/**
 * @brief  Configure one FCLK route: source, divisors, gate.
 * @param  fclk   FCLK0..FCLK3.
 * @param  src    FCLK_SRC_IO, _ARM or _DDR.
 * @param  div0   First divisor, 1..63.
 * @param  div1   Second divisor, 1..63.
 * @param  enable true = clock on.
 * @return true on success, false on any out-of-range argument.
 */
bool clock_fclk_config(enum fclk fclk, enum fclk_src src,
                       uint32_t div0, uint32_t div1, bool enable) {
    volatile uint32_t* clk_ctrl;
    uint32_t value;

    if (((uint32_t)fclk > 3U) ||
        ((src != FCLK_SRC_IO) && (src != FCLK_SRC_ARM) && (src != FCLK_SRC_DDR)) ||
        (div0 == 0U) || (div0 > 63U) || (div1 == 0U) || (div1 > 63U)) {
        return false;
    }

    clk_ctrl = fclk_reg((uint32_t)fclk);

    /* Gate off around the update: the mux is not glitch-free, the
       dividers are (ch. 25). One unlock/lock window for it all. */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    value = *clk_ctrl & ~((0x3FUL << 8) | (0x3FUL << 20) | (0x3UL << 4) | 0x1UL);
    value |= ((div0 & 0x3FUL) << 8) |
             ((div1 & 0x3FUL) << 20) |
             ((uint32_t)src << 4) |
             ((enable != false) ? 0x1UL : 0x0UL);
    *clk_ctrl = value;
    SLCR->LOCK = SLCR_LOCK_KEY;

    return true;
}

/**
 * @brief  Gate a PL fabric clock on or off, keeping its configuration.
 * @param  fclk   FCLK0..FCLK3.
 * @param  enable true to gate the clock on, false to gate it off.
 */
void clock_fclk_enable(enum fclk fclk, bool enable) {
    volatile uint32_t* clk_ctrl;
    uint32_t value;

    if ((uint32_t)fclk > 3U) {
        return;
    }
    clk_ctrl = fclk_reg((uint32_t)fclk);

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    value = *clk_ctrl & ~0x1UL;
    if (enable != false) {
        value |= 0x1UL;
    }
    *clk_ctrl = value;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Read back the frequency of a PL fabric clock.
 * @param  fclk FCLK0..FCLK3.
 * @return The configured frequency in Hz, or 0 when the route is gated
 *         off (the PL then sees no clock at all).
 */
uint32_t clock_fclk_freq_get(enum fclk fclk) {
    volatile uint32_t* clk_ctrl;
    struct clock_freqs freqs;

    if ((uint32_t)fclk > 3U) {
        return 0U;
    }
    clk_ctrl = fclk_reg((uint32_t)fclk);
    if ((*clk_ctrl & 0x1UL) == 0U) {
        return 0U; /* gated off */
    }

    freqs.arm_pll = pll_freq_from_ctrl(SLCR->ARM_PLL_CTRL);
    freqs.ddr_pll = pll_freq_from_ctrl(SLCR->DDR_PLL_CTRL);
    freqs.io_pll = pll_freq_from_ctrl(SLCR->IO_PLL_CTRL);
    return periph_ref_freq_cascaded(*clk_ctrl, &freqs);
}
