#ifndef ZYNQ_COMMON_H
#define ZYNQ_COMMON_H

#include <stdbool.h>
#include <stdint.h>

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

#define PS_CLK_HZ (33333000UL)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif
