#ifndef ZYNQ_DELAY_H
#define ZYNQ_DELAY_H

#include "zynq_soc_ip.h"

/*
 * Millisecond tick + delay in the STM32 HAL SysTick style: the CPU private
 * timer raises a 1 ms FIQ (a slot in the zynq_fiq dispatcher), the handler
 * increments a volatile tick counter, and zynq_delay_ms() spins on it, so
 * the accuracy does not depend on compiler optimization or flash wait
 * states. The tick period comes from the live CPU_3x2x frequency
 * (zynq_clock driver); the handler runs from .fast in OCM.
 *
 * Private timer = GIC ID 29; PTIM_CONTROL: [0] enable, [1] auto-reload,
 * [2] interrupt enable, [15:8] prescaler; PTIM_ISR is W1C (UG585 ch. 8).
 * Run zynq_fiq_init() before zynq_delay_init().
 */

void zynq_delay_init(void);
void zynq_delay_ms(uint32_t ms);
uint32_t zynq_delay_tick_get(void);

#endif
