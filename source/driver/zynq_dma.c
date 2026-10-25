#include "zynq_dma.h"

#include "zynq_soc.h"

#define DMAC_CHANNEL_COUNT (8U)

#define DMA_OP_END (0x00U)
#define DMA_OP_LD  (0x04U)
#define DMA_OP_ST  (0x08U)
#define DMA_OP_WMB (0x13U)
#define DMA_OP_GO  (0xA0U)
#define DMA_OP_MOV (0xBCU)
#define DMA_OP_SEV (0x34U)

#define DMA_MOV_SAR (0x0U)
#define DMA_MOV_CCR (0x1U)
#define DMA_MOV_DAR (0x2U)

#define DMA_CCR(beat_count)                                                                                            \
    ((1U << 0) | (2U << 1) | ((((uint32_t)(beat_count)) - 1U) << 4) | (0U << 8) | (1U << 14) | (2U << 15) |            \
     ((((uint32_t)(beat_count)) - 1U) << 18) | (0U << 22))

uint32_t dma_last_fault_type;
uint32_t dma_last_fault_source;

#define DMA_EVENT_POLL_LIMIT (60000000U)

#define DMA_PROG_BYTES (1024U)
static uint8_t dma_prog[DMA_PROG_BYTES] __attribute__((section(".no_cache_ram")));

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

void dma_init(void)
{
    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->aper_clk_ctrl |= (1U << 0);
    slcr->dmac_rst_ctrl = 1U;
    slcr->dmac_rst_ctrl = 0U;
    slcr->lock = SLCR_LOCK_KEY;

    dmac->wd = 0U;
    dmac->intclr = 0xFFFFFFFFU;
}

int dma_memcpy(void *dst, const void *src, uint32_t byte_count)
{
    uint8_t *p = dma_prog;
    uint32_t block_count;
    uint32_t tail_beats;
    uint32_t i;
    volatile uint32_t count;

    if (byte_count == 0U)
    {
        return 0;
    }
    if ((byte_count > DMA_MEMCPY_MAX_BYTES) || ((byte_count & 3U) != 0U) || (((uint32_t)(uintptr_t)dst & 3U) != 0U) ||
        (((uint32_t)(uintptr_t)src & 3U) != 0U))
    {
        return -1;
    }

    block_count = byte_count / 64U;
    tail_beats = (byte_count % 64U) / 4U;

    p = emit_mov(p, DMA_MOV_SAR, (uint32_t)(uintptr_t)src);
    p = emit_mov(p, DMA_MOV_DAR, (uint32_t)(uintptr_t)dst);
    p = emit_mov(p, DMA_MOV_CCR, DMA_CCR(16));

    for (i = 0; i < block_count; i++)
    {
        p = emit_op(p, DMA_OP_LD);
        p = emit_op(p, DMA_OP_ST);
    }
    if (tail_beats != 0U)
    {
        p = emit_mov(p, DMA_MOV_CCR, DMA_CCR(tail_beats));
        p = emit_op(p, DMA_OP_LD);
        p = emit_op(p, DMA_OP_ST);
    }
    p = emit_op(p, DMA_OP_WMB);
    *p++ = DMA_OP_SEV;
    *p++ = 0U << 3;
    p = emit_op(p, DMA_OP_END);

    count = 0;
    while ((dmac->dbgstatus & 1U) != 0U)
    {
        if (++count > 100000U)
        {
            return -1;
        }
    }

    dmac->dbginst0 = ((0x0U << 24) | ((uint32_t)DMA_OP_GO << 16) | (0U << 8) | 0U);
    dmac->dbginst1 = (uint32_t)(uintptr_t)dma_prog;
    dmac->dbgcmd = 0U;

    count = 0;
    while ((dmac->es & 1U) == 0U)
    {
        if (++count > DMA_EVENT_POLL_LIMIT)
        {
            dma_last_fault_type = dmac->ftc0;
            dma_last_fault_source = dmac->fsrc;
            return -1;
        }
    }
    dmac->intclr = 1U;
    return 0;
}
