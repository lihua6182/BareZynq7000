#ifndef ZYNQ_TICK_H
#define ZYNQ_TICK_H

#include "zynq_soc_ip.h"

/*
 * Millisecond tick + delay in the STM32 HAL SysTick style: the CPU private
 * timer raises a 1 ms IRQ; tick_init() binds the handler into the
 * zynq_irq dispatcher at run time, the handler increments a volatile tick
 * counter, and tick_mdelay() spins on it, so the accuracy does not depend on
 * compiler optimization or flash wait states. The tick period comes from
 * the live CPU_3x2x frequency (zynq_clock driver); the handler runs from
 * .fast in OCM.
 *
 * Private timer = GIC ID 29; PTIM_CONTROL: [0] enable, [1] auto-reload,
 * [2] interrupt enable, [15:8] prescaler; PTIM_ISR is W1C (UG585 ch. 8).
 * Run irq_init() before tick_init().
 */

void tick_init(void);
void tick_mdelay(uint32_t ms);
uint32_t tick_get(void);

#endif
