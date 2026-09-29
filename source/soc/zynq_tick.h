#ifndef ZYNQ_TICK_H
#define ZYNQ_TICK_H

#include "zynq_soc_ip.h"

/*
 * 1 ms tick: TTC0 counter 1 wraps every millisecond and raises its
 * interval interrupt; the handler clears the flag and increments a
 * volatile counter, and tick_mdelay() spins on it, so the tick does
 * not depend on compiler optimization or flash wait states. The
 * interval comes from the live CPU_1x clock; delivery goes through
 * the shared IRQ dispatcher, so call irq_init() before tick_init().
 */

void tick_init(void);
void tick_mdelay(uint32_t ms);
uint32_t tick_get(void);

#endif
