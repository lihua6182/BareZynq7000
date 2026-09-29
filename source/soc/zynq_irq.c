#include "zynq_irq.h"

#include <stddef.h>

/* Default priority handed out by irq_enable: mid-range, so sources
   above (lower value) and below remain available for preemption control */
#define IRQ_DEF_PRIO (0xA0U)

/*
 * Handler registry: one slot per GIC interrupt ID, bound at run time by
 * irq_attach() and read by the IRQ entry; NULL = unbound. Plain
 * .bss in OCM.
 */
static irq_handler_t handlers[NR_IRQS];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
void irq_handler(void) __attribute__((interrupt("IRQ"), target("general-regs-only")));
#pragma GCC diagnostic pop

/*
 * IRQ entry: overrides the weak default in zynq_startup.s. IRQ mode banks
 * only r13/r14, so the interrupt attribute saves the full r0-r12 plus lr
 * on the IRQ stack and returns with subs pc, lr, #4. The IAR/EOIR pair
 * is mandatory: without the acknowledge the IRQ line stays asserted and
 * the exception re-fires right after the SPSR restore. Spurious ID 1023
 * fails the bounds check and takes no EOIR (IHI0048); an enabled but
 * unbound ID is dismissed with EOI only and re-pends while its
 * peripheral stays asserted.
 * target("general-regs-only") keeps the handler off the VFP/NEON bank;
 * GCC's warning below only looks at command-line options, hence the pragma.
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

/*
 * Bind handler to a GIC interrupt ID at run time. Attach before
 * irq_enable(), or the source fires unbound and is merely dismissed
 * with EOI.
 */
int irq_attach(uint32_t id, irq_handler_t handler) {
    if ((id >= NR_IRQS) || (handler == NULL)) {
        return -1;
    }

    handlers[id] = handler;
    return 0;
}

/* Unbind a slot: the source reverts to dismissed-with-EOI-only. */
void irq_detach(uint32_t id) {
    if (id >= NR_IRQS) {
        return;
    }
    handlers[id] = NULL;
}

/*
 * Bring up the GIC for plain IRQ delivery: distributor and CPU interface
 * via the CMSIS GIC_* API (vendored core_ca.h). FIQEn (GICC_CTLR bit 3)
 * is never set, and Group 0 is left untouched for a future real FIQ
 * source; everything armed through irq_enable() sits in Group 1 and
 * arrives on the IRQ input. Run once at boot, before arming any source.
 */
void irq_init(void) {
    GIC_DisableDistributor(); /* distributor off while configuring */
    GIC_DisableInterface();

    GIC_SetInterfacePriorityMask(0xFFU); /* unmask all priorities */

    GIC_EnableDistributor();
    GIC_EnableInterface();

    __enable_irq(); /* unmask IRQ in the CPSR */
}

/*
 * Route an interrupt source to IRQ: Group 1 (normal interrupts) +
 * mid-range priority + enabled in the distributor. PPI/SGI registers
 * (id < 32) are banked per CPU; this programs the bank of the executing
 * core.
 */
void irq_enable(uint32_t id) {
    if (id >= NR_IRQS) {
        return;
    }

    /* Group 1 -> IRQ path: set the source's IGROUPR bit. */
    GIC_SetGroup((IRQn_Type)id, 1U);

    GIC_SetPriority((IRQn_Type)id, IRQ_DEF_PRIO);
    GIC_EnableIRQ((IRQn_Type)id);
}

/* Stop delivering a source to IRQ (distributor enable cleared). */
void irq_disable(uint32_t id) {
    if (id >= NR_IRQS) {
        return;
    }
    GIC_DisableIRQ((IRQn_Type)id);
}
