#include "zynq_cache.h"

#include "zynq_soc.h"

#define CACHE_LINE_BYTES (32U)

static uint32_t cache_line_first(uintptr_t addr)
{
    return (uint32_t)addr & ~(CACHE_LINE_BYTES - 1U);
}

static uint32_t cache_line_end(uintptr_t addr, size_t size)
{
    return (uint32_t)((addr + size + CACHE_LINE_BYTES - 1U) & ~(CACHE_LINE_BYTES - 1U));
}

static void cache_l1_clean_mva(void *va)
{
    asm volatile("mcr p15, 0, %0, c7, c10, 1" : : "r"(va) : "memory");
    dmb();
}

static void cache_l1_invalidate_mva(void *va)
{
    asm volatile("mcr p15, 0, %0, c7, c6, 1" : : "r"(va) : "memory");
    dmb();
}

static void cache_l1_lines(uintptr_t addr, uint32_t end, void (*op)(void *))
{
    uint32_t p = cache_line_first(addr);

    for (; p < end; p += CACHE_LINE_BYTES)
    {
        op((void *)(uintptr_t)p);
    }
    dsb();
}

static void cache_l2_lines(uintptr_t addr, uint32_t end, volatile uint32_t *reg)
{
    uint32_t p = cache_line_first(addr);

    for (; p < end; p += CACHE_LINE_BYTES)
    {
        *reg = p;
    }
    l2c->cache_sync = 0U;
    dsb();
}

void cache_clean(const void *addr, size_t size)
{
    if (size != 0U)
    {
        uint32_t end = cache_line_end((uintptr_t)addr, size);

        cache_l1_lines((uintptr_t)addr, end, cache_l1_clean_mva);
        cache_l2_lines((uintptr_t)addr, end, &l2c->clean_line_pa);
    }
}

void cache_invalidate(const void *addr, size_t size)
{
    if (size != 0U)
    {
        uint32_t end = cache_line_end((uintptr_t)addr, size);

        cache_l1_lines((uintptr_t)addr, end, cache_l1_invalidate_mva);
        cache_l2_lines((uintptr_t)addr, end, &l2c->inv_line_pa);
    }
}

void cache_clean_acp(const void *addr, size_t size)
{
    if (size != 0U)
    {
        uint32_t end = cache_line_end((uintptr_t)addr, size);

        cache_l1_lines((uintptr_t)addr, end, cache_l1_clean_mva);
    }
}
