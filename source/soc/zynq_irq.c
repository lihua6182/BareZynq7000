#include "zynq_irq.h"

#include <stddef.h>

#define IRQ_DEFAULT_PRIORITY (0xA0U)

static irq_handler_t irq_handlers[IRQ_ID_COUNT];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
void irq_handler(void) __attribute__((interrupt("IRQ"), target("general-regs-only")));
#pragma GCC diagnostic pop

void irq_handler(void)
{
    uint32_t id = gicc->iar;
    irq_handler_t handler;

    if (id < IRQ_ID_COUNT)
    {
        handler = irq_handlers[id];
        if (handler != NULL)
        {
            handler();
            dsb();
        }
        gicc->eoir = id;
    }
}

int irq_attach(uint32_t id, irq_handler_t handler)
{
    if ((id >= IRQ_ID_COUNT) || (handler == NULL))
    {
        return -1;
    }

    irq_handlers[id] = handler;
    return 0;
}

void irq_detach(uint32_t id)
{
    if (id >= IRQ_ID_COUNT)
    {
        return;
    }
    irq_handlers[id] = NULL;
}

static void gicd_write_byte(volatile uint32_t *reg, uint32_t id, uint8_t value)
{
    uint32_t shift = (id % 4U) * 8U;

    *reg = (*reg & ~(0xFFU << shift)) | ((uint32_t)value << shift);
}

void irq_init(void)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->a9_cpu_rst_ctrl |= (1U << 8);
    delay_loops(100U);
    slcr->a9_cpu_rst_ctrl &= ~(1U << 8);
    slcr->lock = SLCR_LOCK_KEY;

    gicd->ctlr &= ~1U;
    gicc->ctlr &= ~1U;

    gicc->pmr = 0xFFU;

    gicd->ctlr |= 1U;

    gicc->ctlr = (1U << 0) | (1U << 1) | (1U << 2);

    asm volatile("cpsie i" ::: "memory");
}

void irq_enable(uint32_t id)
{
    if (id >= IRQ_ID_COUNT)
    {
        return;
    }

    gicd->igroupr[id / 32U] &= ~(1U << (id % 32U));
    gicd_write_byte(&gicd->ipriorityr[id / 4U], id, IRQ_DEFAULT_PRIORITY);
    if (id >= 32U)
    {
        gicd_write_byte(&gicd->itargetsr[id / 4U], id, 1U);
    }
    gicd->isenabler[id / 32U] = 1U << (id % 32U);
}

void irq_disable(uint32_t id)
{
    if (id >= IRQ_ID_COUNT)
    {
        return;
    }
    gicd->icenabler[id / 32U] = 1U << (id % 32U);
}
