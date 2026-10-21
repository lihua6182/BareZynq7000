#include "zynq_tick.h"

#include "zynq_clock.h"
#include "zynq_irq.h"

static volatile uint32_t tick_ms;

static void tick_irq_handler(void)
{
    uint32_t flags = ttc0->interrupt_register_1;

    if ((flags & 0x1UL) != 0U)
    {
        tick_ms++;
    }
}

void tick_init(void)
{
    struct clock_freqs clocks;

    clock_freqs_get(&clocks);

    ttc0->counter_control_1 = (1UL << 0) | (1UL << 1) | (1UL << 2);
    ttc0->interrupt_enable_1 = 0UL;
    (void)ttc0->interrupt_register_1;
    ttc0->clock_control_1 = 0x1UL;
    ttc0->interval_counter_1 = (clocks.cpu_1x / 2UL / 1000UL) - 1UL;

    if (irq_attach(ttc0_timer0_irq, tick_irq_handler) == 0)
    {
        irq_enable(ttc0_timer0_irq);
        ttc0->interrupt_enable_1 = 1UL;
    }

    ttc0->counter_control_1 = (1UL << 1) | (1UL << 2);
}

void tick_mdelay(uint32_t ms)
{
    uint32_t start = tick_ms;

    while ((tick_ms - start) < ms)
    {
    }
}

uint32_t tick_get(void)
{
    return tick_ms;
}
