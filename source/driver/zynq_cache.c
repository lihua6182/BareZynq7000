/**
 * @file zynq_cache.c
 * @brief Cortex-A9 L1 / PL310 L2 缓存维护；DMA 主设备拓扑与顺序约束的
 *        说明在 zynq_cache.h，这里只有实现。
 */

#include "zynq_cache.h"

#include "zynq_common.h"

/* Cortex-A9 L1 行宽 = PL310 行宽 = 32 字节。 */
#define CACHE_LINE_BYTES (32U)

static uint32_t cache_line_first(uintptr_t addr) {
    return (uint32_t)addr & ~(CACHE_LINE_BYTES - 1U);
}

static uint32_t cache_line_end(uintptr_t addr, size_t size) {
    return (uint32_t)((addr + size + CACHE_LINE_BYTES - 1U) &
                      ~(CACHE_LINE_BYTES - 1U));
}

/* L1 (by MVA) 循环 + DSB：推完（或丢完）CPU 侧行，并保证 DSB 之后 L2
   状态已就绪 —— L2 的 by-PA 维护从这里开始才安全。 */
static void cache_l1_lines(uintptr_t addr, uint32_t end, void (*op)(void*)) {
    uint32_t p = cache_line_first(addr);

    for (; p < end; p += CACHE_LINE_BYTES) {
        op((void*)(uintptr_t)p);
    }
    __DSB();
}

/* PL310 (by PA) 循环 + CACHE_SYNC + DSB：逐行维护并等它真正完成。 */
static void cache_l2_lines(uintptr_t addr, uint32_t end,
                           volatile uint32_t* reg) {
    uint32_t p = cache_line_first(addr);

    for (; p < end; p += CACHE_LINE_BYTES) {
        *reg = p;
    }
    L2C_310->CACHE_SYNC = 0U;
    __DSB();
}

void cache_clean(const void* addr, size_t size) {
    if (size != 0U) {
        uint32_t end = cache_line_end((uintptr_t)addr, size);

        cache_l1_lines((uintptr_t)addr, end, L1C_CleanDCacheMVA);
        cache_l2_lines((uintptr_t)addr, end, &L2C_310->CLEAN_LINE_PA);
    }
}

void cache_invalidate(const void* addr, size_t size) {
    if (size != 0U) {
        uint32_t end = cache_line_end((uintptr_t)addr, size);

        cache_l1_lines((uintptr_t)addr, end, L1C_InvalidateDCacheMVA);
        cache_l2_lines((uintptr_t)addr, end, &L2C_310->INV_LINE_PA);
    }
}

void cache_clean_acp(const void* addr, size_t size) {
    if (size != 0U) {
        uint32_t end = cache_line_end((uintptr_t)addr, size);

        /* ACP 主设备与 L2 硬件一致：L1 行推入 L2 即对 DMA 可见。 */
        cache_l1_lines((uintptr_t)addr, end, L1C_CleanDCacheMVA);
    }
}
