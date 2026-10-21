#ifndef ZYNQ_CACHE_H
#define ZYNQ_CACHE_H

#include <stddef.h>

void cache_clean(const void *addr, size_t size);

void cache_invalidate(const void *addr, size_t size);

void cache_clean_acp(const void *addr, size_t size);

#endif
