#ifndef ZYNQ_CACHE_H
#define ZYNQ_CACHE_H

/*
 * driver zynq_cache: Cortex-A9 L1 + PL310 L2 缓存维护（by-MVA / by-PA），
 * 给 DMA 缓冲一致性用。Zynq-7000 上有两类 DMA 主设备，维护深度不同：
 *
 *   中央互连（USB/ENET/SD/DMA 控制器）：不嗅探 PL310，数据必须真正落到
 *   OCM/DDR —— cache_clean / cache_invalidate（L1+L2 双层）
 *
 *   ACP 端口（DevC/PCAP）：硬件在 SCU/ACP 处与 L2 一致，只需把 CPU 侧
 *   L1 行推下去 —— cache_clean_acp（L1 单层）
 *
 * 顺序约束（ARMv7-A）：L1 维护指令完成 ≠ 数据已落到 L2；L2 的 by-PA
 * 维护必须在 L1 循环全部结束 + DSB 之后才开始，否则 L2 清到旧行、DMA
 * 读到旧数据（Linux __dma_map_area -> outer_*_range 同款序列）。
 *
 * 行宽：L1 与 PL310 均为 32 字节（与 CONFIG_USB_ALIGN_SIZE=32 同源）。
 * invalidate 是"丢弃"语义：只给 DMA 专用缓冲用，CPU 不得指望未保存的
 * 写还在。addr/size 任意对齐（内部按 32B 行取整），size == 0 为空操作。
 */

#include <stddef.h>

/* CPU -> DMA（中央互连主设备）：L1+L2 双层清写，数据落内存。 */
void cache_clean(const void* addr, size_t size);

/* DMA -> CPU（中央互连主设备）：丢弃 CPU 侧 L1/L2 行，后续读命中 DMA
 * 写入的内存。丢弃语义，DMA 专用缓冲。 */
void cache_invalidate(const void* addr, size_t size);

/* CPU -> DMA（ACP 主设备，如 DevC/PCAP）：只清 L1，L2 一致性由硬件保证。 */
void cache_clean_acp(const void* addr, size_t size);

#endif /* ZYNQ_CACHE_H */
