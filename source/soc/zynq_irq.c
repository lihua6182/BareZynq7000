/**
 * @file zynq_irq.c
 * @brief Shared GIC IRQ dispatcher (UG585 ch.7, IHI0048).
 *
 * Every source is delivered on the IRQ input: sources are Group 0 and
 * ICCICR.FIQEn is clear, so the GIC drives nIRQ while the FIQ input
 * stays masked at its reset value.
 */

#include "zynq_irq.h"

#include <stddef.h>

/* Default priority from irq_enable(): mid-range, leaving room above
   and below for preemption control. */
#define IRQ_DEF_PRIO (0xA0U)

/* One slot per GIC interrupt ID; NULL = unbound. Plain .bss in OCM. */
static irq_handler_t handlers[NR_IRQS];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
void irq_handler(void) __attribute__((interrupt("IRQ"), target("general-regs-only")));
#pragma GCC diagnostic pop

/**
 * @brief  IRQ entry point, overriding the weak default in zynq_startup.s.
 *
 * IRQ mode banks only r13/r14, so the interrupt attribute has to
 * preserve r0-r12 on the IRQ stack and return with subs pc, lr, #4.
 * The IAR/EOIR pair is mandatory, or the exception re-fires right after
 * the SPSR restore. Spurious ID 1023 fails the bounds check and takes
 * no EOIR (IHI0048); an enabled but unbound ID is dismissed with EOI
 * only.
 *
 * target("general-regs-only") keeps the handler off the VFP/NEON bank;
 * GCC warns about that only from command-line options, hence the
 * pragma above the declaration.
 */
void irq_handler(void) {
    uint32_t id = (uint32_t)GIC_AcknowledgePending();
    irq_handler_t handler;

    if (id < NR_IRQS) {
        handler = handlers[id];
        if (handler != NULL) {
            handler();
            __DSB(); /* source flag clear completes before EOI */
        }
        GIC_EndInterrupt((IRQn_Type)id); /* priority drop + deactivate */
    }
}

/**
 * @brief  Bind a handler to a GIC interrupt ID.
 * @param  id      Interrupt ID, 0..NR_IRQS-1.
 * @param  handler Function to call; must not be NULL.
 * @return 0 on success, -1 when id is out of range or handler is NULL.
 *
 * Attach before irq_enable(): an enabled but unbound level source
 * re-fires forever without ever being cleared.
 */
int irq_attach(uint32_t id, irq_handler_t handler) {
    if ((id >= NR_IRQS) || (handler == NULL)) {
        return -1;
    }

    handlers[id] = handler;
    return 0;
}

/**
 * @brief  Unbind a handler.
 * @param  id Interrupt ID, 0..NR_IRQS-1; out-of-range is ignored.
 *
 * The source stays enabled and reverts to dismissed-with-EOI-only.
 */
void irq_detach(uint32_t id) {
    if (id >= NR_IRQS) {
        return;
    }
    handlers[id] = NULL;
}

/**
 * @brief  Bring up the GIC for IRQ delivery.
 *
 * The PERI_RST pulse first resets the GIC and the CPU private timers
 * (UG585 7.1.3, slcr A9_CPU_RST_CTRL bit 8): a JTAG reload resets the
 * CPU only, so GIC state from a previous run survives. FIQEn stays
 * clear, so every Group 0 source is forwarded on the IRQ input (UG585
 * Table 7-1); CPSR.I is unmasked while the FIQ input (CPSR.F) stays at
 * its reset mask. Run once at boot, before arming any source.
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
    /* EnableS + EnableNS + AckCtl, FIQEn = 0: the combination the
       Xilinx standalone driver programs. With FIQEn clear the GIC
       drives the IRQ input (UG585 Table 7-1). */
    GICInterface->CTLR = (1UL << 0) | (1UL << 1) | (1UL << 2);

    __enable_irq(); /* unmask IRQ in the CPSR */
}

/**
 * @brief  Route a source to IRQ delivery and unmask it.
 * @param  id Interrupt ID, 0..NR_IRQS-1; out-of-range is ignored.
 *
 * Group 0 (the reset default, left in place by the Xilinx driver as
 * well; with FIQEn clear - see irq_init() - Group 0 sources are
 * forwarded on the IRQ input, UG585 Table 7-1) + default priority +
 * distributor enable. SPIs additionally need a target: ITARGETSR resets
 * to 0 (no CPU), and an untargeted SPI pends but is never forwarded -
 * cf. XScuGic_DistInit, which stamps CPU0 into every SPI. PPI/SGI
 * registers (id < 32) are banked per CPU and need no target; this
 * programs the executing core's bank.
 */
void irq_enable(uint32_t id) {
    if (id >= NR_IRQS) {
        return;
    }

    GIC_SetGroup((IRQn_Type)id, 0U);
    GIC_SetPriority((IRQn_Type)id, IRQ_DEF_PRIO);
    if (id >= 32U) {
        GIC_SetTarget((IRQn_Type)id, 1U); /* route the SPI to CPU0 */
    }
    GIC_EnableIRQ((IRQn_Type)id);
}

/**
 * @brief  Stop delivering a source.
 * @param  id Interrupt ID, 0..NR_IRQS-1; out-of-range is ignored.
 *
 * Clears the distributor enable only; the binding survives.
 */
void irq_disable(uint32_t id) {
    if (id >= NR_IRQS) {
        return;
    }
    GIC_DisableIRQ((IRQn_Type)id);
}
