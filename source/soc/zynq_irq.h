#ifndef ZYNQ_IRQ_H
#define ZYNQ_IRQ_H

#include "zynq_soc.h"

typedef void (*irq_handler_t)(void);

#define IRQ_ID_COUNT (96U)

enum irqn
{
    sgi0_irq = 0,
    sgi1_irq = 1,
    sgi2_irq = 2,
    sgi3_irq = 3,
    sgi4_irq = 4,
    sgi5_irq = 5,
    sgi6_irq = 6,
    sgi7_irq = 7,
    sgi8_irq = 8,
    sgi9_irq = 9,
    sgi10_irq = 10,
    sgi11_irq = 11,
    sgi12_irq = 12,
    sgi13_irq = 13,
    sgi14_irq = 14,
    sgi15_irq = 15,

    global_timer_irq = 27,
    fiq_fabric_irq = 28,
    priv_timer_irq = 29,
    priv_wdt_irq = 30,

    core_parity0_irq = 32,
    core_parity1_irq = 33,
    l2_cache_irq = 34,
    ocm_irq = 35,
    ecc_irq = 36,
    pmu0_irq = 37,
    pmu1_irq = 38,
    sysmon_irq = 39,
    dvc_irq = 40,
    sys_wdt_irq = 41,
    ttc0_timer0_irq = 42,
    ttc0_timer1_irq = 43,
    ttc0_timer2_irq = 44,
    dma0_abort_irq = 45,
    dma0_ch0_irq = 46,
    dma0_ch1_irq = 47,
    dma0_ch2_irq = 48,
    dma0_ch3_irq = 49,
    smc_irq = 50,
    qspi_irq = 51,
    gpio_irq = 52,
    usb0_irq = 53,
    gem0_irq = 54,
    gem0_wake_irq = 55,
    sdio0_irq = 56,
    i2c0_irq = 57,
    spi0_irq = 58,
    uart0_irq = 59,
    can0_irq = 60,
    pl_irq0 = 61,
    pl_irq1 = 62,
    pl_irq2 = 63,
    pl_irq3 = 64,
    pl_irq4 = 65,
    pl_irq5 = 66,
    pl_irq6 = 67,
    pl_irq7 = 68,
    ttc1_timer0_irq = 69,
    ttc1_timer1_irq = 70,
    ttc1_timer2_irq = 71,
    dma0_ch4_irq = 72,
    dma0_ch5_irq = 73,
    dma0_ch6_irq = 74,
    dma0_ch7_irq = 75,
    usb1_irq = 76,
    gem1_irq = 77,
    gem1_wake_irq = 78,
    sdio1_irq = 79,
    i2c1_irq = 80,
    spi1_irq = 81,
    uart1_irq = 82,
    can1_irq = 83,
    pl_irq8 = 84,
    pl_irq9 = 85,
    pl_irq10 = 86,
    pl_irq11 = 87,
    pl_irq12 = 88,
    pl_irq13 = 89,
    pl_irq14 = 90,
    pl_irq15 = 91,
    scu_parity_irq = 92
};

int irq_attach(uint32_t id, irq_handler_t handler);

void irq_detach(uint32_t id);

void irq_init(void);

void irq_enable(uint32_t id);

void irq_disable(uint32_t id);

#endif
