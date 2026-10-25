#ifndef ZYNQ_CLOCK_H
#define ZYNQ_CLOCK_H

#include "zynq_soc.h"

struct clock_freqs
{
    uint32_t ps_clk;
    uint32_t arm_pll;
    uint32_t ddr_pll;
    uint32_t io_pll;
    uint32_t cpu;
    uint32_t cpu_6x4x;
    uint32_t cpu_3x2x;
    uint32_t cpu_2x;
    uint32_t cpu_1x;
    uint32_t ddr_2x;
    uint32_t ddr_3x;
    uint32_t uart_ref;
    uint32_t qspi_ref;
    uint32_t gem0_ref;
    uint32_t gem1_ref;
    uint32_t sdio_ref;
    uint32_t can_ref;
    uint32_t spi_ref;
    uint32_t dbg_ref;
    uint32_t pcap_ref;
    uint32_t fpga0_ref;
    uint32_t fpga1_ref;
    uint32_t fpga2_ref;
    uint32_t fpga3_ref;
    uint32_t smc_ref;
};

void clock_freqs_get(struct clock_freqs *freqs);

enum fclk
{
    fclk0 = 0,
    fclk1 = 1,
    fclk2 = 2,
    fclk3 = 3
};

enum fclk_src
{
    fclk_src_io = 0x0,
    fclk_src_arm = 0x2,
    fclk_src_ddr = 0x3
};

bool clock_fclk_config(enum fclk fclk, enum fclk_src src, uint32_t div0, uint32_t div1, bool enable);

void clock_fclk_enable(enum fclk fclk, bool enable);

uint32_t clock_fclk_freq_get(enum fclk fclk);

#endif
