#ifndef ZYNQ_COMMON_H
#define ZYNQ_COMMON_H

#include <stdbool.h>
#include <stdint.h>

#define PS_CLK_HZ (33333000U)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static inline void dsb(void)
{
    asm volatile("dsb" ::: "memory");
}

static inline void isb(void)
{
    asm volatile("isb" ::: "memory");
}

static inline void dmb(void)
{
    asm volatile("dmb" ::: "memory");
}

/* Must stay inline: qspi_reconfig() runs from OCM and cannot fetch from flash. */
static inline void __attribute__((always_inline)) delay_loops(uint32_t loops)
{
    uint32_t i;

    for (i = 0U; i < loops; i++)
    {
        asm volatile("nop");
    }
}

#endif
