#ifndef ZYNQ_CLOCK_H
#define ZYNQ_CLOCK_H

#include "zynq_soc_ip.h"

/*
 * Run-time frequency readout of the PS clock tree (UG585 ch. 25). The only
 * hard-coded value is the crystal (ZYNQ_PS_CLK_VALUE in zynq_common.h);
 * every frequency below is recomputed from the live SLCR registers:
 *
 *   PLL x = PS_CLK x FDIV (PLL_CTRL[18:12]), or PS_CLK when bypassed
 *   CPU   = cpu mux / DIVISOR (ARM_CLK_CTRL), cpu mux = 0x:ARM 10:DDR 11:IO
 *   CPU_6x4x = CPU              CPU_3x2x = CPU/2
 *   CPU_2x   = CPU/3 or CPU/2 } CLK_621_TRUE[0]: 1 = 6:2:1, 0 = 4:2:1
 *   CPU_1x   = CPU/6 or CPU/4 }
 *   DDR_2x/3x = DDR PLL / divisor (DDR_CLK_CTRL[31:26] / [25:20])
 *   periph ref = mux / DIVISOR[13:8] (/ DIV1[25:20] if cascaded),
 *                mux = 0x:IO, 10:ARM, 11:DDR
 *
 * All divisors are one-based; a 0 field bypasses the division (÷1).
 *
 * Not decoded (external/undeterminable sources): the CAN MIO clock
 * (CAN_MIOCLK_CTRL), the debug trace EMIO clock (DBG_CLK_CTRL[6]) and the
 * watchdog source (SWDT_CLK_SEL). Frequencies are in Hz.
 */
typedef struct {
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
} zynq_clock_freqs_type;

void zynq_clock_freqs_get(zynq_clock_freqs_type* freqs);

/*
 * PL output clocks (FCLK0-3, SLCR FPGA0-3_CLK_CTRL). FCLK = PLL_src /
 * (div0 * div1); both divisors are one-based 6-bit fields (1..63), pass 1
 * in div1 when no cascade is needed. The source multiplexer is not
 * glitch-free (UG585 ch. 25), so zynq_clock_fclk_config gates the clock
 * (CLKACT = 0) around the register update - expect the FCLK line to stop
 * briefly - while the divisors themselves are glitch-free and may be
 * changed live. The throttle registers (FPGAx_THR_*) stay at reset
 * (continuous clock).
 */
typedef enum {
    ZYNQ_FCLK_0 = 0,
    ZYNQ_FCLK_1 = 1,
    ZYNQ_FCLK_2 = 2,
    ZYNQ_FCLK_3 = 3
} zynq_fclk_type;

typedef enum {
    ZYNQ_FCLK_SRC_IO_PLL = 0x0,
    ZYNQ_FCLK_SRC_ARM_PLL = 0x2,
    ZYNQ_FCLK_SRC_DDR_PLL = 0x3
} zynq_fclk_src_type;

/* Program source, both divisors and the enable of one FCLK route. Returns
   false (without touching the hardware) for an out-of-range argument. */
bool zynq_clock_fclk_config(zynq_fclk_type fclk, zynq_fclk_src_type src,
                            uint32_t div0, uint32_t div1, bool enable);

/* Gate one FCLK route on/off; the divisors and source are preserved. */
void zynq_clock_fclk_enable(zynq_fclk_type fclk, bool enable);

/* Frequency the PL actually sees on one FCLK route: 0 when the route is
   gated off, otherwise PLL_src / (div0 * div1) read back from the SLCR. */
uint32_t zynq_clock_fclk_freq_get(zynq_fclk_type fclk);

#endif
