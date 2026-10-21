#include "board_pl.h"

#include <string.h>

#include "lzma_stream.h"
#include "zynq_cache.h"
#include "zynq_common.h"
#include "zynq_soc.h"

#define PCAP_CLK_CTRL_CLKACT (0x1UL)

#define PL_INIT_POLL_LIMIT  (8000000UL)
#define DMA_DONE_POLL_LIMIT (60000000UL)

#define RESET_PULSE_LOOPS (1024UL)

#define DMA_ERROR_MASK                                                                                                 \
    (DEVCFG_IXR_DMA_CMD_ERR_Msk | DEVCFG_IXR_DMA_Q_OV_Msk | DEVCFG_IXR_P2D_LEN_ERR_Msk | DEVCFG_IXR_RX_FIFO_OV_Msk |   \
     DEVCFG_IXR_AXI_WERR_Msk | DEVCFG_IXR_AXI_WTO_Msk | DEVCFG_IXR_AXI_RERR_Msk | DEVCFG_IXR_AXI_RTO_Msk |             \
     DEVCFG_IXR_PCFG_HMAC_ERR_Msk | DEVCFG_IXR_PCFG_SEU_ERR_Msk)

#define ZBLZ_MAGIC   (0x5A4C425AUL)
#define ZB_HDR_BYTES (16UL)
#define CRC32_POLY   (0xEDB88320UL)

#define SYNC_SCAN_BYTES (128UL)

#define DMA_STAGE_BYTES (4096UL)
static uint32_t dma_stage_buf[DMA_STAGE_BYTES / 4UL] __attribute__((aligned(64)));

struct zb_header
{
    uint32_t magic;
    uint32_t raw_len;
    uint32_t packed_len;
    uint32_t crc32;
};

static void busy_wait_loops(uint32_t loops)
{
    volatile uint32_t i;

    for (i = 0U; i < loops; i++)
    {
    }
}

static enum pl_status wait_pl_init(bool ready)
{
    uint32_t i;

    for (i = 0U; i < PL_INIT_POLL_LIMIT; i++)
    {
        if (pl_is_ready() == ready)
        {
            return PL_OK;
        }
    }
    return ready ? PL_ERR_NOT_READY : PL_ERR_NOT_RESET;
}

int pl_reset_pulse(uint32_t line)
{
    if (pl_reset_assert(line) != 0)
    {
        return -1;
    }
    busy_wait_loops(RESET_PULSE_LOOPS);
    return pl_reset_release(line);
}

enum pl_status pl_config_clear(void)
{
    uint32_t ctrl;
    enum pl_status st;

    if (pl_is_powered() == false)
    {
        return PL_ERR_NO_POWER;
    }

    ctrl = devcfg->ctrl;
    devcfg->ctrl = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    devcfg->ctrl = ctrl & ~DEVCFG_CTRL_PCFG_PROG_B_Msk;

    st = wait_pl_init(false);
    if (st != PL_OK)
    {
        return st;
    }

    devcfg->ctrl = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    devcfg->int_sts = DEVCFG_IXR_PCFG_DONE_Msk;
    return wait_pl_init(true);
}

static void pcap_ack_dma_done(void)
{
    devcfg->int_sts = DEVCFG_IXR_DMA_DONE_Msk;
    devcfg->status |= DEVCFG_STATUS_DMA_DONE_CNT_Msk;
}

static bool has_pcap_sync_word(const uint8_t *img, uint32_t len)
{
    uint32_t limit;
    uint32_t i;

    limit = (len < SYNC_SCAN_BYTES) ? len : SYNC_SCAN_BYTES;
    for (i = 0U; (i + 4UL) <= limit; i++)
    {
        if ((img[i] == 0x66U) && (img[i + 1U] == 0x55U) && (img[i + 2U] == 0x99U) && (img[i + 3U] == 0xAAU))
        {
            return true;
        }
    }
    return false;
}

static uint32_t crc32_table[256];

static void crc32_table_init(void)
{
    uint32_t i;
    uint32_t v;
    uint32_t b;

    for (i = 0U; i < 256U; i++)
    {
        v = i;
        for (b = 0U; b < 8U; b++)
        {
            v = ((v & 1UL) != 0UL) ? (CRC32_POLY ^ (v >> 1U)) : (v >> 1U);
        }
        crc32_table[i] = v;
    }
}

static uint32_t crc32_update(uint32_t crc, const uint8_t *bytes, uint32_t len)
{
    uint32_t i;

    for (i = 0U; i < len; i++)
    {
        crc = (crc >> 8U) ^ crc32_table[(crc ^ bytes[i]) & 0xFFU];
    }
    return crc;
}

static void pcap_open(void)
{
    uint32_t ctrl;

    pl_lvl_ps_to_pl();

    devcfg->unlock = DEVCFG_UNLOCK_DATA;

    devcfg->int_sts = 0xFFFFFFFFUL;

    ctrl = devcfg->ctrl;
    ctrl |= DEVCFG_CTRL_PCAP_MODE_Msk | DEVCFG_CTRL_PCAP_PR_Msk;
    ctrl &= ~(DEVCFG_CTRL_PCAP_RATE_EN_Msk | DEVCFG_CTRL_PCFG_POR_CNT_4K_Msk);
    devcfg->ctrl = ctrl;
}

static enum pl_status pcap_start(void)
{
    enum pl_status st;

    slcr->unlock = SLCR_UNLOCK_KEY;
    slcr->pcap_clk_ctrl |= PCAP_CLK_CTRL_CLKACT;
    slcr->lock = SLCR_LOCK_KEY;

    st = pl_config_clear();
    if (st != PL_OK)
    {
        return st;
    }

    if ((devcfg->status & DEVCFG_STATUS_DMA_CMD_Q_F_Msk) != 0U)
    {
        return PL_ERR_CMD_Q_FULL;
    }
    devcfg->mctrl &= ~DEVCFG_MCTRL_PCAP_LPBK_Msk;

    pcap_ack_dma_done();

    return PL_OK;
}

static enum pl_status pcap_send_block(const void *src, uint32_t words)
{
    uint32_t i;
    uint32_t int_sts;

    devcfg->dma_src_addr = (uint32_t)(uintptr_t)src;
    devcfg->dma_dest_addr = DEVCFG_DMA_INVALID_ADDRESS;
    devcfg->dma_src_len = words;
    dsb();
    devcfg->dma_dest_len = words;

    for (i = 0U; i < DMA_DONE_POLL_LIMIT; i++)
    {
        int_sts = devcfg->int_sts;
        if ((int_sts & DMA_ERROR_MASK) != 0U)
        {
            devcfg->int_sts = int_sts;
            return PL_ERR_DMA;
        }
        if ((int_sts & DEVCFG_IXR_DMA_DONE_Msk) != 0U)
        {
            break;
        }
    }
    if (i == DMA_DONE_POLL_LIMIT)
    {
        return PL_ERR_DMA_TIMEOUT;
    }

    pcap_ack_dma_done();
    return PL_OK;
}

static enum pl_status pcap_wait_pl_done(void)
{
    uint32_t i;

    for (i = 0U; i < DMA_DONE_POLL_LIMIT; i++)
    {
        if ((devcfg->int_sts & DEVCFG_IXR_PCFG_DONE_Msk) != 0U)
        {
            break;
        }
    }
    devcfg->int_sts = DEVCFG_IXR_PCFG_DONE_Msk;
    if (i == DMA_DONE_POLL_LIMIT)
    {
        return PL_ERR_NO_DONE;
    }

    pl_lvl_all();

    return PL_OK;
}

static enum pl_status load_raw_bitstream(const uint8_t *img, uint32_t len)
{
    if (!has_pcap_sync_word(img, len))
    {
        return PL_ERR_NO_SYNC;
    }
    return pcap_send_block(img, len / 4UL);
}

static enum pl_status parse_zb_header(const uint8_t *img, uint32_t len, struct zb_header *hdr)
{
    if (len < ZB_HDR_BYTES)
    {
        return PL_ERR_HDR;
    }
    memcpy(hdr, img, sizeof(*hdr));
    if (hdr->magic != ZBLZ_MAGIC)
    {
        return PL_ERR_HDR;
    }

    if ((hdr->raw_len < 4UL) || ((hdr->raw_len & 3UL) != 0UL))
    {
        return PL_ERR_HDR;
    }
    if ((hdr->packed_len == 0UL) || (hdr->packed_len > (len - ZB_HDR_BYTES)))
    {
        return PL_ERR_HDR;
    }
    return PL_OK;
}

struct zblz_sink
{
    uint8_t *stage;
    uint32_t filled;
    uint32_t crc;
    bool sync_checked;
    enum pl_status st;
};

static enum pl_status zblz_flush(struct zblz_sink *s)
{
    enum pl_status st;

    if (s->filled == 0UL)
    {
        return PL_OK;
    }
    if (!s->sync_checked)
    {
        if (!has_pcap_sync_word(s->stage, s->filled))
        {
            return PL_ERR_NO_SYNC;
        }
        s->sync_checked = true;
    }
    cache_clean(s->stage, s->filled);
    s->crc = crc32_update(s->crc, s->stage, s->filled);
    st = pcap_send_block(s->stage, s->filled / 4UL);
    if (st == PL_OK)
    {
        s->filled = 0UL;
    }
    return st;
}

static void zblz_sink(void *user, const uint8_t *data, uint32_t len)
{
    struct zblz_sink *s = user;

    while ((len != 0UL) && (s->st == PL_OK))
    {
        uint32_t room = DMA_STAGE_BYTES - s->filled;
        uint32_t take = (room < len) ? room : len;

        memcpy(&s->stage[s->filled], data, take);
        s->filled += take;
        data += take;
        len -= take;
        if (s->filled == DMA_STAGE_BYTES)
        {
            s->st = zblz_flush(s);
        }
    }
}

static enum pl_status load_zblz_image(const uint8_t *img, const struct zb_header *hdr)
{
    struct zblz_sink s;

    crc32_table_init();
    s.stage = (uint8_t *)dma_stage_buf;
    s.filled = 0UL;
    s.crc = 0xFFFFFFFFUL;
    s.sync_checked = false;
    s.st = PL_OK;

    if (lzma_decode_stream(img + ZB_HDR_BYTES, hdr->packed_len, hdr->raw_len, zblz_sink, &s) != LZMA_STREAM_OK)
    {
        return PL_ERR_STREAM;
    }
    if (s.st != PL_OK)
    {
        return s.st;
    }
    s.st = zblz_flush(&s);
    if (s.st != PL_OK)
    {
        return s.st;
    }
    if ((~s.crc) != hdr->crc32)
    {
        return PL_ERR_CRC;
    }
    return PL_OK;
}

static enum pl_status pl_load_image(const void *img, uint32_t len)
{
    const uint8_t *bytes = (const uint8_t *)img;
    uint32_t magic;
    enum pl_status st;

    if ((len == 0U) || ((len & 0x3U) != 0U))
    {
        return PL_ERR_LENGTH;
    }

    memcpy(&magic, bytes, sizeof(magic));
    if (magic == ZBLZ_MAGIC)
    {

        struct zb_header hdr;

        st = parse_zb_header(bytes, len, &hdr);
        if (st != PL_OK)
        {
            return st;
        }
        st = pcap_start();
        if (st != PL_OK)
        {
            return st;
        }
        st = load_zblz_image(bytes, &hdr);
    }
    else
    {

        if ((((uintptr_t)bytes) & 0x3FU) != 0U)
        {
            return PL_ERR_SRC_ALIGN;
        }
        st = pcap_start();
        if (st != PL_OK)
        {
            return st;
        }
        st = load_raw_bitstream(bytes, len);
    }
    if (st != PL_OK)
    {
        return st;
    }
    return pcap_wait_pl_done();
}

enum pl_status pl_load(const void *img, uint32_t len)
{

    pcap_open();

    pl_reset_assert_all();

    return pl_load_image(img, len);
}
