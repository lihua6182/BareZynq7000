#ifndef ZYNQ_CLOCK_H
#define ZYNQ_CLOCK_H

#include "zynq_soc_ip.h"

/*
 * Run-time readout of the PS clock tree (UG585 ch. 25): everything is
 * recomputed from the live SLCR registers, the only hard-coded value
 * is the crystal PS_CLK_HZ (zynq_common.h). Divisors are one-based,
 * 0 bypasses (÷1). Frequencies in Hz.
 *
 *   PLL x = PS_CLK x FDIV (PLL_CTRL[18:12]), or PS_CLK when bypassed
 *   CPU   = mux / DIVISOR (ARM_CLK_CTRL);       mux: 0x ARM, 10 DDR, 11 IO
 *   periph ref = mux / DIVISOR[13:8] (/ DIV1[25:20] if cascaded),
 *                                             mux: 0x IO, 10 ARM, 11 DDR
 *   CPU_6x4x = CPU, CPU_3x2x = CPU/2; CPU_2x/1x per CLK_621_TRUE[0]
 *
 * Not decoded (external/undeterminable): CAN MIO clock, debug trace
 * EMIO clock (DBG_CLK_CTRL[6]), watchdog source (SWDT_CLK_SEL).
 */
struct clk_freqs {
    uint32_t ps_clk;    /* PS_CLK crystal input                        */
    uint32_t arm_pll;   /* ARM PLL output                              */
    uint32_t ddr_pll;   /* DDR PLL output                              */
    uint32_t io_pll;    /* IO PLL output                               */
    uint32_t cpu;       /* CPU clock (Cortex-A9)                       */
    uint32_t cpu_6x4x;  /* CPU_6x4x clock (L2/SCU domain)              */
    uint32_t cpu_3x2x;  /* CPU_3x2x clock (private timers, watchdog)   */
    uint32_t cpu_2x;    /* CPU_2x clock (OCM switch)                   */
    uint32_t cpu_1x;    /* CPU_1x clock (APB peripherals)              */
    uint32_t ddr_2x;    /* DDR_2x clock                                */
    uint32_t ddr_3x;    /* DDR_3x clock (DDR controller)               */
    uint32_t uart_ref;  /* UART ref clock, shared by UART0/1           */
    uint32_t qspi_ref;  /* QSPI ref clock (SCLK = ref/2 in dual/quad)  */
    uint32_t gem0_ref;  /* GEM0 ref clock                              */
    uint32_t gem1_ref;  /* GEM1 ref clock                              */
    uint32_t sdio_ref;  /* SDIO ref clock, shared by SDIO0/1           */
    uint32_t can_ref;   /* CAN ref clock, PLL path only                */
    uint32_t spi_ref;   /* SPI ref clock, shared by SPI0/1             */
    uint32_t dbg_ref;   /* debug APB ref clock, PLL path only          */
    uint32_t pcap_ref;  /* PCAP (devcfg) ref clock                     */
    uint32_t fpga0_ref; /* PL clock 0 (FCLK0)                          */
    uint32_t fpga1_ref; /* PL clock 1 (FCLK1)                          */
    uint32_t fpga2_ref; /* PL clock 2 (FCLK2)                          */
    uint32_t fpga3_ref; /* PL clock 3 (FCLK3)                          */
    uint32_t smc;       /* SMC clock (gated CPU_1x)                    */
};

void clk_freqs_get(struct clk_freqs* freqs);

/*
 * PL output clocks FCLK0-3 (SLCR FPGA0-3_CLK_CTRL): FCLK = PLL_src /
 * (div0 * div1), divisors 1..63, pass 1 in div1 for no cascade. The
 * source mux is not glitch-free (UG585 ch. 25), so fclk_config gates
 * the clock around the update; the divisors are glitch-free and may be
 * changed live.
 */
enum fclk {
    FCLK0 = 0,
    FCLK1 = 1,
    FCLK2 = 2,
    FCLK3 = 3
};

enum fclk_src {
    FCLK_SRC_IO = 0x0,
    FCLK_SRC_ARM = 0x2,
    FCLK_SRC_DDR = 0x3
};

/* Program source, divisors and enable of one FCLK route; returns false
   without touching the hardware on an out-of-range argument. */
bool fclk_config(enum fclk fclk, enum fclk_src src,
                 uint32_t div0, uint32_t div1, bool enable);

/* Gate one FCLK route on/off; divisors and source are preserved. */
void fclk_enable(enum fclk fclk, bool enable);

/* Frequency the PL sees on one FCLK route: 0 when gated off, else
   PLL_src / (div0 * div1) read back from the SLCR. */
uint32_t fclk_freq_get(enum fclk fclk);

#endif
