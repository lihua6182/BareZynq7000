#ifndef ZYNQ_DMA_H
#define ZYNQ_DMA_H

#include "zynq_common.h"

/*
 * PS DMAC 驱动 (ARM PL330, UG585 ch.9)。通道线程执行系统内存里的 DMA
 * 微码: dma_memcpy() 现场生成一段 "DMAMOV SAR/DAR/CCR + 展开 N 对
 * DMALD/DMAST + WMB/SEV/END" 的程序, 经 debug 接口 (DBGSTATUS ->
 * DBGINST0/1 -> DBGCMD) 让 DMA manager 执行 DMAGO 启动通道 0, 然后轮询
 * 事件状态 (ES) 等完成。轮询风格, 不占 GIC (中断 ID 45..49/72..75 留给
 * 将来)。指令编码出自 ARM DDI0424 (PL330 TRM)。
 *
 *   dma_init();
 *   if (dma_memcpy(dst, src, 4096) == 0) { ... }
 *
 * 约束:
 *   - src/dst/byte_count 必须按 4 字节对齐 (CCR 用 4B beat, 双端自增)。
 *   - DMAC 是中央互连主设备, 不过 CPU 的 L1/L2: 缓冲若在可缓存区
 *     (低 OCM .bss/堆), 调用方负责 L1+L2 维护; 放 .no_cache_ram 则免。
 *   - 单笔最大 DMA_MEMCPY_MAX_BYTES (微码缓冲限制), 更大请分段。
 *   - 固定用通道 0 与事件 0; 阻塞直到完成或超时。
 *   - debug 指令走安全 APB 口 (DMAC, 0xF8003000): A9 复位后为安全态,
 *     非安全口 (0xF8004000) 在本系统被过滤、读写皆空。
 */

/* 单笔搬运上限: 微码 = 3xDMAMOV + 2 字节/64B 块 + 收尾, 程序缓冲 1 KiB。 */
#define DMA_MEMCPY_MAX_BYTES (496u * 64u)

int dma_init(void);
int dma_memcpy(void* dst, const void* src, uint32_t byte_count);

#endif /* ZYNQ_DMA_H */
