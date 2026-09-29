/**
 * @file zynq_tick.c
 * @brief 1 ms system tick on TTC0 counter 0.
 */

#include "zynq_tick.h"

#include "zynq_clock.h"
#include "zynq_irq.h"

/*
 * 1 ms tick: TTC0 channel 1 (registers at +0x000, i.e. TTC0 counter 0)
 * counts down from the interval at pclk (= CPU_1x) / 2, reloads
 * automatically and raises its interval interrupt (SPI 42, UG585
 * Table 7-4) on each wrap; the handler reads the flag and increments
 * jiffies, and tick_mdelay() spins on it. Delivery goes through the
 * shared IRQ dispatcher (the interrupt is an ordinary IRQ), so
 * accuracy does not depend on compiler optimization or flash wait
 * states. Same CPU, same L1 cache: volatile alone keeps the handler
 * and the thread views of jiffies consistent.
 */
#define TICK_IRQ_ID ((uint32_t)Ttc0Timer0_IRQn) /* SPI 42 = TTC0 counter 0 */

static volatile uint32_t jiffies;

/**
 * @brief  Interval interrupt handler: count one tick.
 *
 * The TTC interrupt register is cleared on read (UG585 ch.8, "Interrupt
 * Clear and Acknowledge Sequence"), so the read below drops the level
 * source before the dispatcher EOIs it.
 *
 * @note Must not live in .fast: that section starts at address 0, so
 *       irq_attach() would reject the handler as a NULL pointer.
 */
static void tick_irq(void) {
    uint32_t flags = TTC0->INTERRUPT_REGISTER_1;

    if ((flags & 0x1UL) != 0U) { /* interval event */
        jiffies++;
    }
}

/**
 * @brief  Start the 1 ms tick counter.
 *
 * TTC0 counter 0 counts down from the interval at pclk (= CPU_1x) / 2,
 * reloads automatically and raises its interval interrupt (SPI 42); the
 * handler increments jiffies, which tick_mdelay() spins on. Programmed
 * while stopped (UG585 ch.8 enable sequence), with stale state dropped
 * first - a JTAG reload resets the CPU but not the TTC.
 */
void tick_init(void) {
    struct clk_freqs clks;

    clk_freqs_get(&clks);

    TTC0->COUNTER_CONTROL_1 = (1UL << 0) | (1UL << 1) | (1UL << 2); /* DIS|INT|DEC */
    TTC0->INTERRUPT_ENABLE_1 = 0UL;   /* no interrupt while reprogramming */
    (void)TTC0->INTERRUPT_REGISTER_1; /* drop stale flags (cleared on read) */
    TTC0->CLOCK_CONTROL_1 = 0x1UL;    /* PS_EN, PS_VAL = 0 (/2), SRC = pclk */
    TTC0->INTERVAL_COUNTER_1 = (clks.cpu_1x / 2UL / 1000UL) - 1UL;

    /* Bind before arming: an enabled but unbound level source would
       re-fire forever without ever being cleared. */
    if (irq_attach(TICK_IRQ_ID, tick_irq) == 0) {
        irq_enable(TICK_IRQ_ID);
        TTC0->INTERRUPT_ENABLE_1 = 1UL; /* interval interrupt */
    }

    TTC0->COUNTER_CONTROL_1 = (1UL << 1) | (1UL << 2); /* INT|DEC: run */
}

/**
 * @brief  Busy-wait for a number of milliseconds.
 * @param  ms Milliseconds to wait; 0 returns immediately.
 *
 * Spins on the tick counter. The unsigned difference is rollover-safe.
 */
void tick_mdelay(uint32_t ms) {
    uint32_t start = jiffies;

    while ((jiffies - start) < ms) {
    }
}

/**
 * @brief  Read the tick counter.
 * @return Milliseconds since tick_init().
 */
uint32_t tick_get(void) {
    return jiffies;
}
