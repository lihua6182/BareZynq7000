/**
 * @file zynq_irq.c
 * @brief Shared GIC IRQ dispatcher (UG585 ch.7, IHI0048); all sources
 *        on the IRQ input, FIQ never enabled.
 */

#include "zynq_irq.h"

#include <stddef.h>

/* Default priority from irq_enable(): mid-range, leaving room above
   and below for preemption control. */
#define IRQ_DEFAULT_PRIORITY (0xA0U)

/* One slot per GIC interrupt ID; NULL = unbound. Plain .bss in OCM. */
static irq_handler_t irq_handlers[IRQ_ID_COUNT];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
void irq_handler(void) __attribute__((interrupt("IRQ"), target("general-regs-only")));
#pragma GCC diagnostic pop

/**
 * @brief  IRQ entry point, overriding the weak default in zynq_startup.s.
 *
 * IAR/EOIR pair is mandatory or the exception re-fires; spurious ID
 * 1023 takes no EOIR (IHI0048); unbound IDs are dismissed with EOI
 * only. target("general-regs-only") keeps the handler off the VFP/NEON
 * bank; GCC only warns about that from command-line options, hence the
 * pragma above.
 */
void irq_handler(void) {
    uint32_t id = (uint32_t)GIC_AcknowledgePending();
    irq_handler_t handler;

    if (id < IRQ_ID_COUNT) {
        handler = irq_handlers[id];
        if (handler != NULL) {
            handler();
            __DSB(); /* source flag clear completes before EOI */
        }
        GIC_EndInterrupt((IRQn_Type)id); /* priority drop + deactivate */
    }
}

/**
 * @brief  Bind a handler to a GIC interrupt ID; attach before
 *         irq_enable(), or a level source re-fires forever.
 * @param  id      Interrupt ID, 0..IRQ_ID_COUNT-1.
 * @param  handler Function to call; must not be NULL.
 * @return 0 on success, -1 on bad id or NULL handler.
 */
int irq_attach(uint32_t id, irq_handler_t handler) {
    if ((id >= IRQ_ID_COUNT) || (handler == NULL)) {
        return -1;
    }

    irq_handlers[id] = handler;
    return 0;
}

/**
 * @brief  Unbind a handler.
 * @param  id Interrupt ID, 0..IRQ_ID_COUNT-1; out-of-range is ignored.
 *
 * The source stays enabled and reverts to dismissed-with-EOI-only.
 */
void irq_detach(uint32_t id) {
    if (id >= IRQ_ID_COUNT) {
        return;
    }
    irq_handlers[id] = NULL;
}

/**
 * @brief  Bring up the GIC for IRQ delivery; run once before arming any
 *         source.
 *
 * The PERI_RST pulse also clears GIC state a JTAG reload leaves behind
 * (slcr A9_CPU_RST_CTRL bit 8). FIQEn stays clear (Group 0 forwards on
 * the IRQ input), CPSR.I unmasked, FIQ stays masked.
 */
void irq_init(void) {
    uint32_t i;

    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->A9_CPU_RST_CTRL |= (1UL << 8); /* PERI_RST */
    for (i = 0U; i < 100U; i++) {
        __asm volatile("nop");
    }
    SLCR->A9_CPU_RST_CTRL &= ~(1UL << 8);
    SLCR->LOCK = SLCR_LOCK_KEY;

    GIC_DisableDistributor();
    GIC_DisableInterface();

    GIC_SetInterfacePriorityMask(0xFFU); /* unmask all priorities */

    GIC_EnableDistributor();
    /* EnableS + EnableNS + AckCtl, FIQEn = 0 (XScuGic's combination). */
    GICInterface->CTLR = (1UL << 0) | (1UL << 1) | (1UL << 2);

    __enable_irq(); /* unmask IRQ in the CPSR */
}

/**
 * @brief  Route a source to IRQ delivery and unmask it.
 * @param  id Interrupt ID, 0..IRQ_ID_COUNT-1; out-of-range is ignored.
 *
 * SPIs need a target: ITARGETSR resets to 0 and an untargeted SPI pends
 * but is never forwarded (cf. XScuGic_DistInit stamping CPU0 into every
 * SPI). PPI/SGI registers (id < 32) are banked per CPU, no target.
 */
void irq_enable(uint32_t id) {
    if (id >= IRQ_ID_COUNT) {
        return;
    }

    GIC_SetGroup((IRQn_Type)id, 0U);
    GIC_SetPriority((IRQn_Type)id, IRQ_DEFAULT_PRIORITY);
    if (id >= 32U) {
        GIC_SetTarget((IRQn_Type)id, 1U); /* route the SPI to CPU0 */
    }
    GIC_EnableIRQ((IRQn_Type)id);
}

/**
 * @brief  Stop delivering a source.
 * @param  id Interrupt ID, 0..IRQ_ID_COUNT-1; out-of-range is ignored.
 *
 * Clears the distributor enable only; the binding survives.
 */
void irq_disable(uint32_t id) {
    if (id >= IRQ_ID_COUNT) {
        return;
    }
    GIC_DisableIRQ((IRQn_Type)id);
}
