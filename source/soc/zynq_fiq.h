#ifndef ZYNQ_FIQ_H
#define ZYNQ_FIQ_H

#include "zynq_soc_ip.h"

/*
 * Shared FIQ dispatch (STM32 vector-table style, adapted to the GIC).
 *
 * The FIQ entry in zynq_fiq.c acknowledges the GIC, calls the handler
 * selected by the interrupt ID read from GICC_IAR, then signals
 * end-of-interrupt. Every GIC interrupt ID (0..95) has a weak empty
 * handler zynq_fiq_isr_<id>; claim a source by defining that function,
 * e.g. the private-timer tick in zynq_delay.c:
 *
 *   void zynq_fiq_isr_29(void) { ... }
 *
 * With GICC CTLR.FIQEn = 1 every Group 0 interrupt is signalled on the FIQ
 * input: call zynq_fiq_init() once at boot, then arm sources with
 * zynq_fiq_enable(id). Notable IDs (UG585 Table 7-3): 27 global timer,
 * 29 CPU private timer, 30 CPU private watchdog, 51 QSPI, 52 GPIO, 54 GEM0,
 * 82 UART0.
 *
 * The dispatcher is XIP code: FIQ is unmasked only after the boot-time
 * QSPI reconfiguration, so flash is always readable on an FIQ. Code that
 * leaves QSPI linear mode at run time must first mask FIQ
 * (__disable_fiq) or its instruction fetches will stall. An enabled but
 * unhandled source is dismissed with EOI only and re-pends while its
 * peripheral stays asserted.
 */

/* Interrupt IDs implemented by the Zynq GIC: 16 SGI + 16 PPI + 64 SPI */
#define ZYNQ_GIC_NUM_IDS (96U)

void zynq_fiq_init(void);
void zynq_fiq_enable(uint32_t id);
void zynq_fiq_disable(uint32_t id);

#endif
