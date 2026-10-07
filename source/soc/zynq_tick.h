#ifndef ZYNQ_TICK_H
#define ZYNQ_TICK_H

#include "zynq_soc.h"

/*
 * 1 ms tick on TTC0 counter 0 (the _1 register group at +0x000):
 * interval interrupt -> volatile counter; tick_mdelay() spins on it.
 * Interval from the live CPU_1x clock. Delivery via the shared IRQ
 * dispatcher: call irq_init() before tick_init().
 */

void tick_init(void);
void tick_mdelay(uint32_t ms);
uint32_t tick_get(void);

#endif /* ZYNQ_TICK_H */
