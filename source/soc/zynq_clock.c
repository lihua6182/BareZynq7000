#include "zynq_clock.h"

/*
 * PLL output = PS_CLK x FDIV, or PS_CLK straight through while the bypass
 * multiplexer is forced (UG585 ch. 25; FDIV = PLL_CTRL[18:12],
 * PLL_BYPASS_FORCE = PLL_CTRL[4]). 64-bit intermediate: a large crystal x
 * the maximum FDIV can exceed 2^31.
 */
static uint32_t zynq_clock_pll_freq(uint32_t pll_ctrl) {
    uint32_t fdiv = (pll_ctrl >> 12) & 0x7FU;

    if (fdiv == 0U || (pll_ctrl & (1UL << 4)) != 0U) {
        return ZYNQ_PS_CLK_VALUE;
    }
    return (uint32_t)((uint64_t)ZYNQ_PS_CLK_VALUE * (uint64_t)fdiv);
}

/* Peripheral clock mux: SRCSEL[5:4] = 0x:IO, 10:ARM, 11:DDR. */
static uint32_t zynq_clock_periph_src(uint32_t srcsel, const zynq_clock_freqs_type* f) {
    switch (srcsel) {
        case 2U:
            return f->arm_pll;
        case 3U:
            return f->ddr_pll;
        default:
            return f->io_pll;
    }
}

/* CPU clock mux: SRCSEL[5:4] = 0x:ARM, 10:DDR, 11:IO. */
static uint32_t zynq_clock_cpu_src(uint32_t srcsel, const zynq_clock_freqs_type* f) {
    switch (srcsel) {
        case 2U:
            return f->ddr_pll;
        case 3U:
            return f->io_pll;
        default:
            return f->arm_pll;
    }
}

/* One-based 6-bit divisor field; 0 bypasses the division. */
static uint32_t zynq_clock_div_field(uint32_t ctrl, uint32_t shift) {
    uint32_t div = (ctrl >> shift) & 0x3FU;

    return (div != 0U) ? div : 1U;
}

/* Single-divider peripheral: freq = mux / DIVISOR[13:8]. */
static uint32_t zynq_clock_periph_freq(uint32_t ctrl, const zynq_clock_freqs_type* f) {
    uint32_t src = zynq_clock_periph_src((ctrl >> 4) & 0x3U, f);

    return src / zynq_clock_div_field(ctrl, 8U);
}

/* Cascaded two-divider peripheral (GEM, CAN, FCLK): mux / DIV0[13:8] /
   DIV1[25:20]. */
static uint32_t zynq_clock_periph2_freq(uint32_t ctrl, const zynq_clock_freqs_type* f) {
    uint32_t src = zynq_clock_periph_src((ctrl >> 4) & 0x3U, f);

    return src / zynq_clock_div_field(ctrl, 8U) / zynq_clock_div_field(ctrl, 20U);
}

void zynq_clock_freqs_get(zynq_clock_freqs_type* freqs) {
    zynq_clock_freqs_type f;
    uint32_t ctrl;
    uint32_t ratio_621;

    f.ps_clk = ZYNQ_PS_CLK_VALUE;
    f.arm_pll = zynq_clock_pll_freq(SLCR->ARM_PLL_CTRL);
    f.ddr_pll = zynq_clock_pll_freq(SLCR->DDR_PLL_CTRL);
    f.io_pll = zynq_clock_pll_freq(SLCR->IO_PLL_CTRL);

    /* CPU = ARM_CLK_CTRL mux / DIVISOR[13:8]. */
    ctrl = SLCR->ARM_CLK_CTRL;
    f.cpu = zynq_clock_cpu_src((ctrl >> 4) & 0x3U, &f) / zynq_clock_div_field(ctrl, 8U);

    /* CPU domain ratios. CPU_6x4x and CPU_3x2x are the same in both modes;
       CPU_2x and CPU_1x depend on the 6:2:1 ratio mode (UG585 ch. 25). */
    ratio_621 = SLCR->CLK_621_TRUE & 1UL;
    f.cpu_6x4x = f.cpu;
    f.cpu_3x2x = f.cpu / 2UL;
    f.cpu_2x = f.cpu / (2UL + ratio_621);
    f.cpu_1x = f.cpu / (4UL + 2UL * ratio_621);

    /* DDR domain: divisors DDR_CLK_CTRL[31:26] (2x) and [25:20] (3x). */
    ctrl = SLCR->DDR_CLK_CTRL;
    f.ddr_2x = f.ddr_pll / zynq_clock_div_field(ctrl, 26U);
    f.ddr_3x = f.ddr_pll / zynq_clock_div_field(ctrl, 20U);

    f.uart_ref = zynq_clock_periph_freq(SLCR->UART_CLK_CTRL, &f);
    f.qspi_ref = zynq_clock_periph_freq(SLCR->LQSPI_CLK_CTRL, &f);
    f.sdio_ref = zynq_clock_periph_freq(SLCR->SDIO_CLK_CTRL, &f);
    f.spi_ref = zynq_clock_periph_freq(SLCR->SPI_CLK_CTRL, &f);
    f.dbg_ref = zynq_clock_periph_freq(SLCR->DBG_CLK_CTRL, &f);
    f.pcap_ref = zynq_clock_periph_freq(SLCR->PCAP_CLK_CTRL, &f);

    f.gem0_ref = zynq_clock_periph2_freq(SLCR->GEM0_CLK_CTRL, &f);
    f.gem1_ref = zynq_clock_periph2_freq(SLCR->GEM1_CLK_CTRL, &f);
    f.can_ref = zynq_clock_periph2_freq(SLCR->CAN_CLK_CTRL, &f);
    f.fpga0_ref = zynq_clock_periph2_freq(SLCR->FPGA0_CLK_CTRL, &f);
    f.fpga1_ref = zynq_clock_periph2_freq(SLCR->FPGA1_CLK_CTRL, &f);
    f.fpga2_ref = zynq_clock_periph2_freq(SLCR->FPGA2_CLK_CTRL, &f);
    f.fpga3_ref = zynq_clock_periph2_freq(SLCR->FPGA3_CLK_CTRL, &f);

    /* The SMC runs on a gated CPU_1x (SLCR SMC_CLK_CTRL holds only CLKACT). */
    f.smc = f.cpu_1x;

    *freqs = f;
}

/*
 * PL output clocks (FCLK0-3): SLCR FPGA0-3_CLK_CTRL at 0x170 + 0x10 * n.
 * The register group per route is {CLK_CTRL, THR_CTRL, THR_CNT, THR_STA},
 * so the four CLK_CTRLs are not contiguous in the struct.
 */
static volatile uint32_t* zynq_clock_fclk_ctrl_reg(uint32_t fclk) {
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

bool zynq_clock_fclk_config(zynq_fclk_type fclk, zynq_fclk_src_type src,
                            uint32_t div0, uint32_t div1, bool enable) {
    volatile uint32_t* clk_ctrl;
    uint32_t v;

    if (((uint32_t)fclk > 3U) ||
        ((src != ZYNQ_FCLK_SRC_IO_PLL) && (src != ZYNQ_FCLK_SRC_ARM_PLL) && (src != ZYNQ_FCLK_SRC_DDR_PLL)) ||
        (div0 == 0U) || (div0 > 63U) || (div1 == 0U) || (div1 > 63U)) {
        return false;
    }

    clk_ctrl = zynq_clock_fclk_ctrl_reg((uint32_t)fclk);

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

void zynq_clock_fclk_enable(zynq_fclk_type fclk, bool enable) {
    volatile uint32_t* clk_ctrl;
    uint32_t v;

    if ((uint32_t)fclk > 3U) {
        return;
    }
    clk_ctrl = zynq_clock_fclk_ctrl_reg((uint32_t)fclk);

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    v = *clk_ctrl & ~0x1UL;
    if (enable != false) {
        v |= 0x1UL;
    }
    *clk_ctrl = v;
    SLCR->LOCK = SLCR_LOCK_KEY;
}

uint32_t zynq_clock_fclk_freq_get(zynq_fclk_type fclk) {
    volatile uint32_t* clk_ctrl;
    zynq_clock_freqs_type freqs;

    if ((uint32_t)fclk > 3U) {
        return 0U;
    }
    clk_ctrl = zynq_clock_fclk_ctrl_reg((uint32_t)fclk);
    if ((*clk_ctrl & 0x1UL) == 0U) {
        return 0U; /* route gated off: the PL sees no clock */
    }

    freqs.arm_pll = zynq_clock_pll_freq(SLCR->ARM_PLL_CTRL);
    freqs.ddr_pll = zynq_clock_pll_freq(SLCR->DDR_PLL_CTRL);
    freqs.io_pll = zynq_clock_pll_freq(SLCR->IO_PLL_CTRL);
    return zynq_clock_periph2_freq(*clk_ctrl, &freqs);
}
