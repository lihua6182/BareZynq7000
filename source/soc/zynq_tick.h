#ifndef ZYNQ_TICK_H
#define ZYNQ_TICK_H

#include "zynq_soc.h"

void tick_init(void);

void tick_mdelay(uint32_t ms);

uint32_t tick_get(void);

#endif
