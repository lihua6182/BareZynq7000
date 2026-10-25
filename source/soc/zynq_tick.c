#include "zynq_tick.h"

#include "zynq_clock.h"
#include "zynq_irq.h"

static volatile uint32_t tick_ms;

static void tick_irq_handler(void)
{
    uint32_t flags = ttc0->interrupt_register_1;

    if ((flags & 0x1U) != 0U)
    {
        tick_ms++;
    }
}

void tick_init(void)
{
    struct clock_freqs clocks;

    clock_freqs_get(&clocks);

    ttc0->counter_control_1 = (1U << 0) | (1U << 1) | (1U << 2);
    ttc0->interrupt_enable_1 = 0U;
    (void)ttc0->interrupt_register_1;
    ttc0->clock_control_1 = 0x1U;
    ttc0->interval_counter_1 = (clocks.cpu_1x / 2U / 1000U) - 1U;

    if (irq_attach(ttc0_timer0_irq, tick_irq_handler) == 0)
    {
        irq_enable(ttc0_timer0_irq);
        ttc0->interrupt_enable_1 = 1U;
    }

    ttc0->counter_control_1 = (1U << 1) | (1U << 2);
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
