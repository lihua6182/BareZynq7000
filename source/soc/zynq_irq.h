#ifndef ZYNQ_IRQ_H
#define ZYNQ_IRQ_H

#include "zynq_soc_ip.h"

/* Handler slot type: void f(void). */
typedef void (*irq_handler_t)(void);

/*
 * Shared IRQ dispatch (GIC, all sources Group 0, FIQEn clear - UG585
 * Table 7-1): irq_handler acknowledges, calls the bound handler, EOIs.
 * Bind before enabling:
 *
 *   irq_attach(Ttc0Timer0_IRQn, my_tick);
 *   irq_enable(Ttc0Timer0_IRQn);
 *
 * Notable IDs (Tables 7-3/7-4): 27 global timer, 29 private timer,
 * 30 private watchdog, 42 TTC0 counter 1, 51 QSPI, 52 GPIO, 54 GEM0,
 * 59 UART0. Run-time QSPI linear-mode exits must mask IRQ first
 * (__disable_irq), or XIP fetches stall.
 */

/* Interrupt IDs implemented by the Zynq GIC: 16 SGI + 16 PPI + 64 SPI */
#define IRQ_ID_COUNT (96U)

int irq_attach(uint32_t id, irq_handler_t handler);
void irq_detach(uint32_t id);
void irq_init(void);
void irq_enable(uint32_t id);
void irq_disable(uint32_t id);

#endif
