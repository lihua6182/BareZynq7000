/**
 * @file zynq_tick.c
 * @brief 1 ms system tick on TTC0 counter 0.
 */

#include "zynq_tick.h"

#include "zynq_clock.h"
#include "zynq_irq.h"

/*
 * TTC0 counter 0 counts down at pclk (= CPU_1x) / 2, reloads and raises
 * its interval interrupt (SPI 42) each wrap; the handler increments
 * tick_ms. Same CPU, same L1 cache: volatile alone keeps the handler
 * and thread views of tick_ms consistent.
 */
#define TICK_IRQ_ID ((uint32_t)Ttc0Timer0_IRQn) /* SPI 42 = TTC0 counter 0 */

static volatile uint32_t tick_ms;

/**
 * @brief  Interval interrupt handler: count one tick. The TTC interrupt
 *         register clears on read, dropping the level source before EOI.
 *
 * @note Must not live in .fast: the section starts at address 0, and
 *       irq_attach() rejects a NULL-looking pointer.
 */
static void tick_irq_handler(void) {
    uint32_t flags = TTC0->INTERRUPT_REGISTER_1;

    if ((flags & 0x1UL) != 0U) { /* interval event */
        tick_ms++;
    }
}

/**
 * @brief  Start the 1 ms tick counter: program while stopped, drop stale
 *         state first (a JTAG reload resets the CPU but not the TTC).
 */
void tick_init(void) {
    struct clock_freqs clocks;

    clock_freqs_get(&clocks);

    TTC0->COUNTER_CONTROL_1 = (1UL << 0) | (1UL << 1) | (1UL << 2); /* DIS|INT|DEC */
    TTC0->INTERRUPT_ENABLE_1 = 0UL;   /* no interrupt while reprogramming */
    (void)TTC0->INTERRUPT_REGISTER_1; /* drop stale flags (cleared on read) */
    TTC0->CLOCK_CONTROL_1 = 0x1UL;    /* PS_EN, PS_VAL = 0 (/2), SRC = pclk */
    TTC0->INTERVAL_COUNTER_1 = (clocks.cpu_1x / 2UL / 1000UL) - 1UL;

    /* Bind before arming, or the level source re-fires forever. */
    if (irq_attach(TICK_IRQ_ID, tick_irq_handler) == 0) {
        irq_enable(TICK_IRQ_ID);
        TTC0->INTERRUPT_ENABLE_1 = 1UL; /* interval interrupt */
    }

    TTC0->COUNTER_CONTROL_1 = (1UL << 1) | (1UL << 2); /* INT|DEC: run */
}

/**
 * @brief  Busy-wait ms milliseconds; the unsigned difference is
 *         rollover-safe.
 * @param  ms Milliseconds to wait; 0 returns immediately.
 */
void tick_mdelay(uint32_t ms) {
    uint32_t start = tick_ms;

    while ((tick_ms - start) < ms) {
    }
}

/**
 * @brief  Read the tick counter.
 * @return Milliseconds since tick_init().
 */
uint32_t tick_get(void) {
    return tick_ms;
}
