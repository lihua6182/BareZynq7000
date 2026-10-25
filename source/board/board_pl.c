#include "board_pl.h"

#include <string.h>

#include "lzma_stream.h"
#include "zynq_cache.h"
#include "zynq_pcap.h"

#define RESET_PULSE_LOOPS (1024U)

#define ZBLZ_MAGIC     (0x5A4C425AU)
#define ZBLZ_HDR_BYTES (16U)
#define CRC32_POLY     (0xEDB88320U)

#define SYNC_SCAN_BYTES (128U)

#define DMA_STAGE_BYTES (4096U)
static uint32_t dma_stage_buf[DMA_STAGE_BYTES / 4U] __attribute__((aligned(64)));

struct zblz_header
{
    uint32_t magic;
    uint32_t raw_len;
    uint32_t packed_len;
    uint32_t crc32;
};

static enum pl_status pl_from_pcap(enum pcap_status st)
{
    switch (st)
    {
    case PCAP_OK:
        return PL_OK;
    case PCAP_ERR_NO_POWER:
        return PL_ERR_NO_POWER;
    case PCAP_ERR_NOT_RESET:
        return PL_ERR_NOT_RESET;
    case PCAP_ERR_NOT_READY:
        return PL_ERR_NOT_READY;
    case PCAP_ERR_CMD_Q_FULL:
        return PL_ERR_CMD_Q_FULL;
    case PCAP_ERR_DMA:
        return PL_ERR_DMA;
    case PCAP_ERR_DMA_TIMEOUT:
        return PL_ERR_DMA_TIMEOUT;
    case PCAP_ERR_NO_DONE:
        return PL_ERR_NO_DONE;
    }
    return PL_ERR_DMA;
}

int pl_reset_pulse(uint32_t line)
{
    if (pl_reset_assert(line) != 0)
    {
        return -1;
    }
    delay_loops(RESET_PULSE_LOOPS);
    return pl_reset_release(line);
}

static bool pl_has_pcap_sync_word(const uint8_t *img, uint32_t len)
{
    uint32_t limit;
    uint32_t i;

    limit = (len < SYNC_SCAN_BYTES) ? len : SYNC_SCAN_BYTES;
    for (i = 0U; (i + 4U) <= limit; i++)
    {
        if ((img[i] == 0x66U) && (img[i + 1U] == 0x55U) && (img[i + 2U] == 0x99U) && (img[i + 3U] == 0xAAU))
        {
            return true;
        }
    }
    return false;
}

static uint32_t pl_crc32_table[256];

static void pl_crc32_table_init(void)
{
    uint32_t i;
    uint32_t v;
    uint32_t b;

    for (i = 0U; i < 256U; i++)
    {
        v = i;
        for (b = 0U; b < 8U; b++)
        {
            v = ((v & 1U) != 0U) ? (CRC32_POLY ^ (v >> 1U)) : (v >> 1U);
        }
        pl_crc32_table[i] = v;
    }
}

static uint32_t pl_crc32_update(uint32_t crc, const uint8_t *bytes, uint32_t len)
{
    uint32_t i;

    for (i = 0U; i < len; i++)
    {
        crc = (crc >> 8U) ^ pl_crc32_table[(crc ^ bytes[i]) & 0xFFU];
    }
    return crc;
}

static enum pl_status load_raw_bitstream(const uint8_t *img, uint32_t len)
{
    if (!pl_has_pcap_sync_word(img, len))
    {
        return PL_ERR_NO_SYNC;
    }
    return pl_from_pcap(pcap_send_block(img, len / 4U));
}

static enum pl_status zblz_parse_header(const uint8_t *img, uint32_t len, struct zblz_header *hdr)
{
    if (len < ZBLZ_HDR_BYTES)
    {
        return PL_ERR_HDR;
    }
    memcpy(hdr, img, sizeof(*hdr));
    if (hdr->magic != ZBLZ_MAGIC)
    {
        return PL_ERR_HDR;
    }

    if ((hdr->raw_len < 4U) || ((hdr->raw_len & 3U) != 0U))
    {
        return PL_ERR_HDR;
    }
    if ((hdr->packed_len == 0U) || (hdr->packed_len > (len - ZBLZ_HDR_BYTES)))
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

    if (s->filled == 0U)
    {
        return PL_OK;
    }
    if (!s->sync_checked)
    {
        if (!pl_has_pcap_sync_word(s->stage, s->filled))
        {
            return PL_ERR_NO_SYNC;
        }
        s->sync_checked = true;
    }
    cache_clean(s->stage, s->filled);
    s->crc = pl_crc32_update(s->crc, s->stage, s->filled);
    st = pl_from_pcap(pcap_send_block(s->stage, s->filled / 4U));
    if (st == PL_OK)
    {
        s->filled = 0U;
    }
    return st;
}

static void zblz_sink_cb(void *user, const uint8_t *data, uint32_t len)
{
    struct zblz_sink *s = user;

    while ((len != 0U) && (s->st == PL_OK))
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

static enum pl_status load_zblz_image(const uint8_t *img, const struct zblz_header *hdr)
{
    struct zblz_sink s;

    pl_crc32_table_init();
    s.stage = (uint8_t *)dma_stage_buf;
    s.filled = 0U;
    s.crc = 0xFFFFFFFFU;
    s.sync_checked = false;
    s.st = PL_OK;

    if (lzma_stream_decode(img + ZBLZ_HDR_BYTES, hdr->packed_len, hdr->raw_len, zblz_sink_cb, &s) != LZMA_STREAM_OK)
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
        struct zblz_header hdr;

        st = zblz_parse_header(bytes, len, &hdr);
        if (st != PL_OK)
        {
            return st;
        }
        st = pl_from_pcap(pcap_start());
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
        st = pl_from_pcap(pcap_start());
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
    return pl_from_pcap(pcap_wait_pl_done());
}

enum pl_status pl_load(const void *img, uint32_t len)
{
    pcap_open();

    pl_reset_assert_all();

    return pl_load_image(img, len);
}
