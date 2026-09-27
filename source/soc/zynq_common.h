#ifndef ZYNQ_COMMON_H
#define ZYNQ_COMMON_H

#include <stdbool.h>
#include <stdint.h>

#define __I  volatile const /* "read only" permissions    */
#define __O  volatile       /* "write only" permissions   */
#define __IO volatile       /* "read / write" permissions */

/*
 * The only hard-coded frequency in the system: the crystal feeding PS_CLK.
 * Every other clock is derived from the SLCR registers at run time by the
 * zynq_clock driver (zynq_clock_freqs_get).
 */
#define ZYNQ_PS_CLK_VALUE (33333000UL) /* 33.333 MHz */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#if defined(__GNUC__)

/*
 * CPSR interrupt masks, CMSIS core_ca style: the irq pair controls the IRQ
 * mask (CPSR.I), the fiq pair the FIQ mask (CPSR.F). The disables are
 * unconditional, so nested critical sections need their own save/restore
 * of the CPSR around the re-enable.
 */
static inline void __enable_irq(void) {
    __asm volatile("cpsie i" ::: "memory");
}

static inline void __disable_irq(void) {
    __asm volatile("cpsid i" ::: "memory");
}

static inline void __enable_fiq(void) {
    __asm volatile("cpsie f" ::: "memory");
}

static inline void __disable_fiq(void) {
    __asm volatile("cpsid f" ::: "memory");
}

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
