#include "zynq_delay.h"

#include "zynq_clock.h"
#include "zynq_fiq.h"

/* CPU private timer interrupt ID (PPI 29, UG585 Table 7-3) */
#define PTIM_INT_ID (29U)

/* FIQ-incremented tick counter, read by zynq_delay_ms()/zynq_delay_tick_get().
   Same CPU, same L1 cache: volatile alone keeps the two views consistent. */
static volatile uint32_t zynq_delay_tick;

void zynq_fiq_isr_29(void) __attribute__((section(".fast"), noinline));

/* Private timer FIQ slot: weak override; IAR/EOIR is handled by fiq_handler. */
void zynq_fiq_isr_29(void) {
    MPCORE->PTIM_ISR = 1; /* clear the timer event flag (W1C) */
    zynq_delay_tick++;
}

/* Start the 1 ms tick (zynq_fiq_init must have run). */
void zynq_delay_init(void) {
    zynq_clock_freqs_type clocks;

    /* Private timer, 1 ms periodic (UG585 ch. 8): stop, clear any stale
       event, reload for the full period. */
    MPCORE->PTIM_CONTROL = 0;
    MPCORE->PTIM_ISR = 1;
    zynq_clock_freqs_get(&clocks);
    MPCORE->PTIM_LOAD = (clocks.cpu_3x2x / 1000U) - 1U;

    zynq_fiq_enable(PTIM_INT_ID); /* PPI 29 -> Group 0 -> FIQ */

    /* Enable + auto-reload + interrupt enable. */
    MPCORE->PTIM_CONTROL = (1UL << 0) | (1UL << 1) | (1UL << 2);
}

/*
 * STM32 HAL_Delay style: spin until the tick advanced by ms. The unsigned
 * difference keeps the compare correct across rollover.
 */
void zynq_delay_ms(uint32_t ms) {
    uint32_t start = zynq_delay_tick;

    while ((zynq_delay_tick - start) < ms) {
    }
}

uint32_t zynq_delay_tick_get(void) {
    return zynq_delay_tick;
}
