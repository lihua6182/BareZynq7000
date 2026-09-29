#ifndef ZYNQ_IRQ_H
#define ZYNQ_IRQ_H

#include "zynq_soc_ip.h"

/* Handler slot type: void f(void), STM32 IRQ-handler style. */
typedef void (*irq_handler_t)(void);

/*
 * Shared IRQ dispatch (STM32 vector-table style, adapted to the GIC).
 *
 * The exception vectors stay in XIP flash; the dispatch table the IRQ
 * entry uses - one handler pointer per GIC interrupt ID - is a plain RAM
 * array in zynq_irq.c. The IRQ entry acknowledges the GIC, calls the
 * handler bound to the acknowledged interrupt ID, then signals
 * end-of-interrupt. Handlers are bound at run time, before the source
 * is enabled:
 *
 *   static void my_tick(void) { ... }
 *   irq_attach(PrivTimer_IRQn, my_tick);   // CPU private timer
 *   irq_enable(PrivTimer_IRQn);
 *
 * All sources are delivered as ordinary IRQs: the CPU interface is left
 * with FIQEn clear and irq_enable() routes each source to Group 1.
 * Call irq_init() once at boot, then attach+enable sources.
 * Notable IDs (UG585 Table 7-3): 27 global timer, 29 CPU private timer,
 * 30 CPU private watchdog, 51 QSPI, 52 GPIO, 54 GEM0, 82 UART0.
 *
 * The dispatcher is XIP code: IRQ is unmasked only after the boot-time
 * QSPI reconfiguration, so flash is always readable on an IRQ. Code that
 * leaves QSPI linear mode at run time must first mask IRQ
 * (__disable_irq) or its instruction fetches will stall. An enabled but
 * unattached source is dismissed with EOI only and re-pends while its
 * peripheral stays asserted.
 */

/* Interrupt IDs implemented by the Zynq GIC: 16 SGI + 16 PPI + 64 SPI */
#define NR_IRQS (96U)

int irq_attach(uint32_t id, irq_handler_t handler);
void irq_detach(uint32_t id);
void irq_init(void);
void irq_enable(uint32_t id);
void irq_disable(uint32_t id);

#endif
