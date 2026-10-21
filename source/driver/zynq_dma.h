#ifndef ZYNQ_DMA_H
#define ZYNQ_DMA_H

#include "zynq_common.h"

#define DMA_MEMCPY_MAX_BYTES (496u * 64u)

void dma_init(void);
int dma_memcpy(void *dst, const void *src, uint32_t byte_count);

extern uint32_t dma_last_fault_type;
extern uint32_t dma_last_fault_source;

#endif
