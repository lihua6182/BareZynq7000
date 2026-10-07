#ifndef ZYNQ_COMMON_H
#define ZYNQ_COMMON_H

#include <stdbool.h>
#include <stdint.h>

#include "zynq_cmsis.h"

/* The only hard-coded frequency in the system: the crystal feeding PS_CLK. */
#define PS_CLK_HZ (33333000UL) /* 33.333 MHz */

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
