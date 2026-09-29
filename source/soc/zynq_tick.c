#include "zynq_tick.h"

#include "zynq_clock.h"
#include "zynq_irq.h"

/* IRQ-incremented tick counter, read by tick_mdelay()/tick_get(). Same CPU,
   same L1 cache: volatile alone keeps the two views consistent. */
static volatile uint32_t jiffies;

/* Private timer IRQ handler, bound at run time by tick_init() via
   irq_attach(); runs from .fast in OCM. IAR/EOIR is handled by
   irq_handler. */
static void tick_irq(void) __attribute__((section(".fast"), noinline));

static void tick_irq(void) {
    MPCORE->PTIM_ISR = 1; /* clear the timer event flag (W1C) */
    jiffies++;
}

/* Start the 1 ms tick (irq_init must have run). */
void tick_init(void) {
    struct clk_freqs clks;

    /* Private timer, 1 ms periodic (UG585 ch. 8): stop, clear any stale
       event, reload for the full period. */
    MPCORE->PTIM_CONTROL = 0;
    MPCORE->PTIM_ISR = 1;
    clk_freqs_get(&clks);
    MPCORE->PTIM_LOAD = (clks.cpu_3x2x / 1000U) - 1U;

    /* Bind before arming, so the source never fires unbound. ID is the
       CMSIS IRQn enum value (PrivTimer_IRQn = PPI 29, UG585 Table 7-3). */
    (void)irq_attach(PrivTimer_IRQn, tick_irq);
    irq_enable(PrivTimer_IRQn); /* PPI 29 -> Group 1 -> IRQ */

    /* Enable + auto-reload + interrupt enable. */
    MPCORE->PTIM_CONTROL = (1UL << 0) | (1UL << 1) | (1UL << 2);
}

void tick_mdelay(uint32_t ms) {
    uint32_t start = jiffies;

    while ((jiffies - start) < ms) {
    }
}

uint32_t tick_get(void) {
    return jiffies;
}
