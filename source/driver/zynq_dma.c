/**
 * @file zynq_dma.c
 * @brief PS DMAC (PL330) 驱动: 微码式内存搬运, 轮询完成。
 */

#include "zynq_dma.h"

#include "zynq_soc.h"

/* PL330 通道数 (UG585 ch.9)。 */
#define DMAC_CHANNEL_COUNT (8u)

/* PL330 指令操作码 (ARM DDI0424 指令集)。 */
#define DMA_OP_END (0x00u) /* DMAEND:  结束线程                        */
#define DMA_OP_LD  (0x04u) /* DMALD:  按 CCR 装载一个突发              */
#define DMA_OP_ST  (0x08u) /* DMAST:  按 CCR 存储一个突发              */
#define DMA_OP_WMB (0x13u) /* DMAWMB: 写屏障, 保证 ST 对总线可见       */
#define DMA_OP_GO  (0xA0u) /* DMAGO:  (manager 经 debug 接口) 启动通道 */
#define DMA_OP_MOV (0xBCu) /* DMAMOV: 写 SAR/CCR/DAR 之一, imm32 小端  */
#define DMA_OP_SEV (0x34u) /* DMASEV: 发事件, 事件号 = 通道号          */

/* DMAMOV 的目的寄存器操作数。 */
#define DMA_MOV_SAR (0x0u)
#define DMA_MOV_CCR (0x1u)
#define DMA_MOV_DAR (0x2u)

/* CCR 位图按 UG585 附录 (Zynq 版 PL330, 与通用 DDI0424 不同: 插入了
 * cache_ctrl 字段): src_inc[0], src_size[3:1], src_len[7:4], src_prot[10:8],
 * src_cache[13:11], dst_inc[14], dst_size[17:15], dst_len[21:18],
 * dst_prot[24:22], dst_cache[27:25], endian_swap[30:28]。
 * 4 字节 beat、双端自增、突发长度 beat_count (1..16); prot=0 (安全
 * 属性数据访问, 上板定案: 通道由安全 manager 以 ns=0 启动, 互连把
 * AxPROT[1]=1 的非安全数据访问回 SLVERR - data_read_err), cache=0。 */
#define DMA_CCR(beat_count)                                                                                            \
    ((1UL << 0) | (2UL << 1) | ((((uint32_t)(beat_count)) - 1UL) << 4) | (0UL << 8) | (1UL << 14) | (2UL << 15) |      \
     ((((uint32_t)(beat_count)) - 1UL) << 18) | (0UL << 22))

/* 最近一次超时的故障现场 (PL330 FTC0/FSRC), 供排障; 0 = 无故障。 */
uint32_t dma_last_fault_type;
uint32_t dma_last_fault_source;

/* 完成轮询上限 (与 board_pl 的 PCAP 轮询同量级, 667MHz 下若干秒)。 */
#define DMA_EVENT_POLL_LIMIT (60000000u)

/* 微码缓冲: DMAC 读它、CPU 写它, 放非缓存 OCM 免一致性维护。
 * 容量与 DMA_MEMCPY_MAX_BYTES 对应 (头文件)。 */
#define DMA_PROG_BYTES (1024u)
static uint8_t dma_prog[DMA_PROG_BYTES] __attribute__((section(".no_cache_ram")));

/* 微码发射器: p 指向下一个空闲字节。 */
static uint8_t *emit_op(uint8_t *p, uint8_t op)
{
    *p++ = op;
    return p;
}

static uint8_t *emit_mov(uint8_t *p, uint8_t rd, uint32_t imm)
{
    *p++ = DMA_OP_MOV;
    *p++ = rd;
    *p++ = (uint8_t)(imm >> 0);
    *p++ = (uint8_t)(imm >> 8);
    *p++ = (uint8_t)(imm >> 16);
    *p++ = (uint8_t)(imm >> 24);
    return p;
}

/**
 * @brief  DMAC 复位与静态配置: 复位脉冲 + 关看门狗 + 清残留事件。
 * @return 0。
 * @note   时钟 APER_CLK_CTRL[0] 复位即开, 仍显式再开一次。本函数不
 *         下发 debug 指令; 安全 APB 口约束见 dma_memcpy()。
 */
int dma_init(void)
{
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->APER_CLK_CTRL |= (1u << 0); /* DMA_CPU_2XCLKACT */
    SLCR->DMAC_RST_CTRL = 1u;
    SLCR->DMAC_RST_CTRL = 0u;
    SLCR->LOCK = SLCR_LOCK_KEY;

    DMAC->WD = 0u;              /* 看门狗关 (默认也是关) */
    DMAC->INTCLR = 0xFFFFFFFFu; /* 清全部残留事件 */
    return 0;
}

/**
 * @brief  阻塞式内存搬运: 通道 0 执行展开的 DMALD/DMAST 微码。
 * @param  dst        目的地址, 4 字节对齐。
 * @param  src        源地址, 4 字节对齐。
 * @param  byte_count 字节数, 4 的倍数, 上限 DMA_MEMCPY_MAX_BYTES。
 * @return 0 完成; -1 参数非法/超时/通道启动失败。
 * @note   缓冲若在可缓存区, 调用方负责 DMA 前后的一致性维护; 微码
 *         本身在非缓存 OCM。完成信号 = 事件 0 (ES bit0), INTCLR 收尾。
 */
int dma_memcpy(void *dst, const void *src, uint32_t byte_count)
{
    uint8_t *p = dma_prog;
    uint32_t block_count;
    uint32_t tail_beats;
    uint32_t i;
    volatile uint32_t count;

    if (byte_count == 0u)
    {
        return 0;
    }
    if ((byte_count > DMA_MEMCPY_MAX_BYTES) || ((byte_count & 3u) != 0u) || (((uint32_t)(uintptr_t)dst & 3u) != 0u) ||
        (((uint32_t)(uintptr_t)src & 3u) != 0u))
    {
        return -1;
    }

    /* 微码: 定址 + CCR(满突发 16x4B), 整块展开, 尾巴换小 CCR 补一笔,
     * WMB 保证存储可见后发事件收尾。 */
    block_count = byte_count / 64u;
    tail_beats = (byte_count % 64u) / 4u;

    p = emit_mov(p, DMA_MOV_SAR, (uint32_t)(uintptr_t)src);
    p = emit_mov(p, DMA_MOV_DAR, (uint32_t)(uintptr_t)dst);
    p = emit_mov(p, DMA_MOV_CCR, DMA_CCR(16));

    for (i = 0; i < block_count; i++)
    {
        p = emit_op(p, DMA_OP_LD);
        p = emit_op(p, DMA_OP_ST);
    }
    if (tail_beats != 0u)
    {
        p = emit_mov(p, DMA_MOV_CCR, DMA_CCR(tail_beats));
        p = emit_op(p, DMA_OP_LD);
        p = emit_op(p, DMA_OP_ST);
    }
    p = emit_op(p, DMA_OP_WMB);
    *p++ = DMA_OP_SEV;
    *p++ = 0u << 3; /* 事件号 0 (= 通道号) */
    p = emit_op(p, DMA_OP_END);

    /* 经 debug 接口让 DMA manager 执行 DMAGO: 等 debug 空闲 ->
     * DBGINST0/1 装 "DMAGO 通道0, 微码地址" -> DBGCMD 触发。 */
    count = 0;
    while ((DMAC->DBGSTATUS & 1u) != 0u)
    {
        if (++count > 100000u)
        {
            return -1; /* debug 忙: 前一笔没收敛 */
        }
    }
    /* A9 复位后处于安全态, 必须走安全 APB 口 (DMAC, 0xF8003000) 下发;
     * 非安全口 (DMAC_NS) 在本系统被过滤, 写入全部无效 (上板实测)。ns=0:
     * 安全 manager 启动安全通道, 匹配。 */
    DMAC->DBGINST0 = ((0x0u << 24) | ((uint32_t)DMA_OP_GO << 16) | (0u << 8) | 0u);
    DMAC->DBGINST1 = (uint32_t)(uintptr_t)dma_prog;
    DMAC->DBGCMD = 0u;

    /* 轮询事件 0 (ES bit0), 完成后清掉。 */
    count = 0;
    while ((DMAC->ES & 1u) == 0u)
    {
        if (++count > DMA_EVENT_POLL_LIMIT)
        {
            dma_last_fault_type = DMAC->FTC0;
            dma_last_fault_source = DMAC->FSRC;
            return -1; /* 超时: 微码 fault 或地址不可达 */
        }
    }
    DMAC->INTCLR = 1u;
    return 0;
}
