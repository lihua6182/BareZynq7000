
#include "zynq_dma.h"

#include "zynq_soc.h"

#define DMAC_CHANNEL_COUNT (8u)

#define DMA_OP_END (0x00u)
#define DMA_OP_LD  (0x04u)
#define DMA_OP_ST  (0x08u)
#define DMA_OP_WMB (0x13u)
#define DMA_OP_GO  (0xA0u)
#define DMA_OP_MOV (0xBCu)
#define DMA_OP_SEV (0x34u)

#define DMA_MOV_SAR (0x0u)
#define DMA_MOV_CCR (0x1u)
#define DMA_MOV_DAR (0x2u)

#define DMA_CCR(beat_count)                                                                                            \
    ((1UL << 0) | (2UL << 1) | ((((uint32_t)(beat_count)) - 1UL) << 4) | (0UL << 8) | (1UL << 14) | (2UL << 15) |      \
     ((((uint32_t)(beat_count)) - 1UL) << 18) | (0UL << 22))

uint32_t dma_last_fault_type;
uint32_t dma_last_fault_source;

#define DMA_EVENT_POLL_LIMIT (60000000u)

#define DMA_PROG_BYTES (1024u)
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
    slcr->aper_clk_ctrl |= (1u << 0);
    slcr->dmac_rst_ctrl = 1u;
    slcr->dmac_rst_ctrl = 0u;
    slcr->lock = SLCR_LOCK_KEY;

    dmac->wd = 0u;
    dmac->intclr = 0xFFFFFFFFu;
}

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
    *p++ = 0u << 3;
    p = emit_op(p, DMA_OP_END);

    count = 0;
    while ((dmac->dbgstatus & 1u) != 0u)
    {
        if (++count > 100000u)
        {
            return -1;
        }
    }

    dmac->dbginst0 = ((0x0u << 24) | ((uint32_t)DMA_OP_GO << 16) | (0u << 8) | 0u);
    dmac->dbginst1 = (uint32_t)(uintptr_t)dma_prog;
    dmac->dbgcmd = 0u;

    count = 0;
    while ((dmac->es & 1u) == 0u)
    {
        if (++count > DMA_EVENT_POLL_LIMIT)
        {
            dma_last_fault_type = dmac->ftc0;
            dma_last_fault_source = dmac->fsrc;
            return -1;
        }
    }
    dmac->intclr = 1u;
    return 0;
}
