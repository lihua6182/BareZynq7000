/**
 * @file zynq_clock.c
 * @brief SLCR clock tree readback and PL fabric clock setup (UG585 ch.25).
 */

#include "zynq_clock.h"

/**
 * @brief  Compute a PLL output frequency from its control register.
 * @param  pll_ctrl Value of ARM/DDR/IO_PLL_CTRL.
 * @return PS_CLK x FDIV, or PS_CLK while bypass is forced.
 *
 * UG585 ch. 25; FDIV = PLL_CTRL[18:12], bypass = PLL_CTRL[4]. The
 * 64-bit intermediate keeps a large crystal x max FDIV within range.
 */
static uint32_t pll_freq(uint32_t pll_ctrl) {
    uint32_t fdiv = (pll_ctrl >> 12) & 0x7FU;

    if (fdiv == 0U || (pll_ctrl & (1UL << 4)) != 0U) {
        return PS_CLK_HZ;
    }
    return (uint32_t)((uint64_t)PS_CLK_HZ * (uint64_t)fdiv);
}

/**
 * @brief  Resolve a peripheral clock mux setting.
 * @param  srcsel SRCSEL[5:4]: 0x = IO PLL, 2 = ARM PLL, 3 = DDR PLL.
 * @param  f      Frequencies already resolved for the three PLLs.
 * @return The selected source frequency.
 */
static uint32_t periph_src(uint32_t srcsel, const struct clk_freqs* f) {
    switch (srcsel) {
        case 2U:
            return f->arm_pll;
        case 3U:
            return f->ddr_pll;
        default:
            return f->io_pll;
    }
}

/**
 * @brief  Resolve the CPU clock mux setting.
 * @param  srcsel SRCSEL[5:4]: 0x = ARM PLL, 2 = DDR PLL, 3 = IO PLL.
 * @param  f      Frequencies already resolved for the three PLLs.
 * @return The selected source frequency.
 */
static uint32_t cpu_src(uint32_t srcsel, const struct clk_freqs* f) {
    switch (srcsel) {
        case 2U:
            return f->ddr_pll;
        case 3U:
            return f->io_pll;
        default:
            return f->arm_pll;
    }
}

/**
 * @brief  Decode a one-based 6-bit divisor field.
 * @param  ctrl  Clock control register value.
 * @param  shift Bit position of the divisor field.
 * @return The divisor, or 1 when the field is 0 (division bypassed).
 */
static uint32_t div_field(uint32_t ctrl, uint32_t shift) {
    uint32_t div = (ctrl >> shift) & 0x3FU;

    return (div != 0U) ? div : 1U;
}

/**
 * @brief  Frequency of a single-divider peripheral clock.
 * @param  ctrl Clock control register value: mux / DIVISOR[13:8].
 * @param  f    Frequencies already resolved for the three PLLs.
 * @return The resulting frequency.
 */
static uint32_t periph_freq(uint32_t ctrl, const struct clk_freqs* f) {
    uint32_t src = periph_src((ctrl >> 4) & 0x3U, f);

    return src / div_field(ctrl, 8U);
}

/**
 * @brief  Frequency of a cascaded two-divider peripheral clock.
 * @param  ctrl Clock control register value: mux / DIV0[13:8] / DIV1[25:20].
 * @param  f    Frequencies already resolved for the three PLLs.
 * @return The resulting frequency.
 *
 * GEM, CAN and the FCLKs use this shape.
 */
static uint32_t periph2_freq(uint32_t ctrl, const struct clk_freqs* f) {
    uint32_t src = periph_src((ctrl >> 4) & 0x3U, f);

    return src / div_field(ctrl, 8U) / div_field(ctrl, 20U);
}

/**
 * @brief  Read the whole clock tree out of the SLCR.
 * @param  freqs Out: every frequency in struct clk_freqs.
 *
 * Derived from the live register values, so it stays correct after
 * sys_clock_init() reprogrammed the PLLs. DDR is reported even though
 * this board has no DDR controller.
 */
void clk_freqs_get(struct clk_freqs* freqs) {
    struct clk_freqs f;
    uint32_t ctrl;
    uint32_t ratio_621;

    f.ps_clk = PS_CLK_HZ;
    f.arm_pll = pll_freq(SLCR->ARM_PLL_CTRL);
    f.ddr_pll = pll_freq(SLCR->DDR_PLL_CTRL);
    f.io_pll = pll_freq(SLCR->IO_PLL_CTRL);

    /* CPU = ARM_CLK_CTRL mux / DIVISOR[13:8]. */
    ctrl = SLCR->ARM_CLK_CTRL;
    f.cpu = cpu_src((ctrl >> 4) & 0x3U, &f) / div_field(ctrl, 8U);

    /* CPU domain ratios. CPU_6x4x and CPU_3x2x are the same in both modes;
       CPU_2x and CPU_1x depend on the 6:2:1 ratio mode (UG585 ch. 25). */
    ratio_621 = SLCR->CLK_621_TRUE & 1UL;
    f.cpu_6x4x = f.cpu;
    f.cpu_3x2x = f.cpu / 2UL;
    f.cpu_2x = f.cpu / (2UL + ratio_621);
    f.cpu_1x = f.cpu / (4UL + 2UL * ratio_621);

    /* DDR domain: divisors DDR_CLK_CTRL[31:26] (2x) and [25:20] (3x). */
    ctrl = SLCR->DDR_CLK_CTRL;
    f.ddr_2x = f.ddr_pll / div_field(ctrl, 26U);
    f.ddr_3x = f.ddr_pll / div_field(ctrl, 20U);

    f.uart_ref = periph_freq(SLCR->UART_CLK_CTRL, &f);
    f.qspi_ref = periph_freq(SLCR->LQSPI_CLK_CTRL, &f);
    f.sdio_ref = periph_freq(SLCR->SDIO_CLK_CTRL, &f);
    f.spi_ref = periph_freq(SLCR->SPI_CLK_CTRL, &f);
    f.dbg_ref = periph_freq(SLCR->DBG_CLK_CTRL, &f);
    f.pcap_ref = periph_freq(SLCR->PCAP_CLK_CTRL, &f);

    f.gem0_ref = periph2_freq(SLCR->GEM0_CLK_CTRL, &f);
    f.gem1_ref = periph2_freq(SLCR->GEM1_CLK_CTRL, &f);
    f.can_ref = periph2_freq(SLCR->CAN_CLK_CTRL, &f);
    f.fpga0_ref = periph2_freq(SLCR->FPGA0_CLK_CTRL, &f);
    f.fpga1_ref = periph2_freq(SLCR->FPGA1_CLK_CTRL, &f);
    f.fpga2_ref = periph2_freq(SLCR->FPGA2_CLK_CTRL, &f);
    f.fpga3_ref = periph2_freq(SLCR->FPGA3_CLK_CTRL, &f);

    /* The SMC runs on a gated CPU_1x (SLCR SMC_CLK_CTRL holds only CLKACT). */
    f.smc = f.cpu_1x;

    *freqs = f;
}

/**
 * @brief  Get the SLCR clock control register of a PL fabric clock.
 * @param  fclk FCLK index, 0..3.
 * @return Pointer to SLCR.FPGAn_CLK_CTRL.
 *
 * The register group per route is {CLK_CTRL, THR_CTRL, THR_CNT,
 * THR_STA}, so the four CLK_CTRLs are not contiguous in the struct.
 */
static volatile uint32_t* fclk_ctrl_reg(uint32_t fclk) {
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
 * @brief  Configure a PL fabric clock and gate it on or off.
 * @param  fclk   FCLK0..FCLK3.
 * @param  src    Clock source: FCLK_SRC_IO, _ARM or _DDR.
 * @param  div0   First divisor, 1..63.
 * @param  div1   Second divisor, 1..63.
 * @param  enable true to gate the clock on, false to gate it off.
 * @return true on success, false on any out-of-range argument.
 *
 * Resulting frequency = src / div0 / div1.
 */
bool fclk_config(enum fclk fclk, enum fclk_src src,
                 uint32_t div0, uint32_t div1, bool enable) {
    volatile uint32_t* clk_ctrl;
    uint32_t v;

    if (((uint32_t)fclk > 3U) ||
        ((src != FCLK_SRC_IO) && (src != FCLK_SRC_ARM) && (src != FCLK_SRC_DDR)) ||
        (div0 == 0U) || (div0 > 63U) || (div1 == 0U) || (div1 > 63U)) {
        return false;
    }

    clk_ctrl = fclk_ctrl_reg((uint32_t)fclk);

    /* Gate the route off around the update: the source multiplexer is not
       glitch-free (UG585 ch. 25); the dividers are. One unlock/lock window
       for the whole reconfiguration. */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    v = *clk_ctrl & ~((0x3FUL << 8) | (0x3FUL << 20) | (0x3UL << 4) | 0x1UL);
    v |= ((div0 & 0x3FUL) << 8) |
         ((div1 & 0x3FUL) << 20) |
         ((uint32_t)src << 4) |
         ((enable != false) ? 0x1UL : 0x0UL);
    *clk_ctrl = v;
    SLCR->LOCK = SLCR_LOCK_KEY;

    return true;
}

/**
 * @brief  Gate a PL fabric clock on or off, keeping its configuration.
 * @param  fclk   FCLK0..FCLK3.
 * @param  enable true to gate the clock on, false to gate it off.
 */
void fclk_enable(enum fclk fclk, bool enable) {
    volatile uint32_t* clk_ctrl;
    uint32_t v;

    if ((uint32_t)fclk > 3U) {
        return;
    }
    clk_ctrl = fclk_ctrl_reg((uint32_t)fclk);

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    v = *clk_ctrl & ~0x1UL;
    if (enable != false) {
        v |= 0x1UL;
    }
    *clk_ctrl = v;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

/**
 * @brief  Read back the frequency of a PL fabric clock.
 * @param  fclk FCLK0..FCLK3.
 * @return The configured frequency in Hz, or 0 when the route is gated
 *         off (the PL then sees no clock at all).
 */
uint32_t fclk_freq_get(enum fclk fclk) {
    volatile uint32_t* clk_ctrl;
    struct clk_freqs freqs;

    if ((uint32_t)fclk > 3U) {
        return 0U;
    }
    clk_ctrl = fclk_ctrl_reg((uint32_t)fclk);
    if ((*clk_ctrl & 0x1UL) == 0U) {
        return 0U; /* route gated off: the PL sees no clock */
    }

    freqs.arm_pll = pll_freq(SLCR->ARM_PLL_CTRL);
    freqs.ddr_pll = pll_freq(SLCR->DDR_PLL_CTRL);
    freqs.io_pll = pll_freq(SLCR->IO_PLL_CTRL);
    return periph2_freq(*clk_ctrl, &freqs);
}
