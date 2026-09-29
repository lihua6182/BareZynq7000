#ifndef ZYNQ_IRQ_H
#define ZYNQ_IRQ_H

#include "zynq_soc_ip.h"

/* Handler slot type: void f(void). */
typedef void (*irq_handler_t)(void);

/*
 * Shared interrupt dispatch, delivered on the IRQ input: the entry in
 * zynq_startup.s acknowledges the GIC, calls the handler bound to that
 * ID, then signals EOI. All sources are Group 0 and ICCICR.FIQEn is
 * clear, so they arrive as ordinary IRQs (UG585 Table 7-1); the FIQ
 * input stays masked at its reset value. Bind a source before enabling
 * it:
 *
 *   irq_attach(Ttc0Timer0_IRQn, my_tick);
 *   irq_enable(Ttc0Timer0_IRQn);
 *
 * Call irq_init() once at boot. Notable IDs (UG585 Tables 7-3/7-4):
 * 27 global timer, 29 private timer, 30 private watchdog, 42 TTC0
 * counter 1, 51 QSPI, 52 GPIO, 54 GEM0, 59 UART0.
 *
 * The dispatcher is XIP code, so IRQ stays masked until the boot-time
 * QSPI reconfiguration is done; code that leaves linear mode at run
 * time must mask IRQ first (__disable_irq) or its fetches stall. An
 * enabled but unattached source is dismissed with EOI only and
 * re-pends while its peripheral stays asserted.
 */

/* Interrupt IDs implemented by the Zynq GIC: 16 SGI + 16 PPI + 64 SPI */
#define NR_IRQS (96U)

int irq_attach(uint32_t id, irq_handler_t handler);
void irq_detach(uint32_t id);
void irq_init(void);
void irq_enable(uint32_t id);
void irq_disable(uint32_t id);

#endif
