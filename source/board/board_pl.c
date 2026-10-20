/**
 * @file board_pl.c
 * @brief PL configuration over the DevC/PCAP bridge (UG585 ch.6, ch.2).
 *        What needs a loop or a sequence; the single-register primitives
 *        are static inline in board_pl.h. The image itself lives in
 *        board_pl_bitstream.c, board.c decides when to load it.
 */

#include "board_pl.h"

#include <string.h>

#include "lzma_stream.h"
#include "zynq_cache.h"
#include "zynq_common.h"
#include "zynq_soc.h"

/* slcr.PCAP_CLK_CTRL bit 0 (CLKACT); the reset value already sets it. */
#define PCAP_CLK_CTRL_CLKACT (0x1UL)

/* Busy-poll limits in bus cycles (no tick/GIC yet): ~1 s PL power-up /
   PROG_B handshake, ~10 s per DMA command. */
#define PL_INIT_POLL_LIMIT  (8000000UL)
#define DMA_DONE_POLL_LIMIT (60000000UL)

/* Reset pulse width in loop iterations, far longer than a fabric clock
   period at any PLL setting. */
#define RESET_PULSE_LOOPS (1024UL)

/* Transport-level errors that abort a load (UG585 step 10). */
#define DMA_ERROR_MASK                                                                                                 \
    (DEVCFG_IXR_DMA_CMD_ERR_Msk | DEVCFG_IXR_DMA_Q_OV_Msk | DEVCFG_IXR_P2D_LEN_ERR_Msk | DEVCFG_IXR_RX_FIFO_OV_Msk |   \
     DEVCFG_IXR_AXI_WERR_Msk | DEVCFG_IXR_AXI_WTO_Msk | DEVCFG_IXR_AXI_RERR_Msk | DEVCFG_IXR_AXI_RTO_Msk |             \
     DEVCFG_IXR_PCFG_HMAC_ERR_Msk | DEVCFG_IXR_PCFG_SEU_ERR_Msk)

/* Packed images (tools/convert_bitstream.py); the magic doubles as the
 * load-path selector - bitgen images start with 0xFFFFFFFF padding.
 * ZBLZ uses the 16-byte LE header layout:
 *
 *   0x00 magic      "ZBLZ" (raw LZMA1 payload)
 *   0x04 raw_len    uncompressed image length, % 4 == 0
 *   0x08 packed_len payload length
 *   0x0C crc32      CRC-32 (EDB88320) over the raw image
 */
#define ZBLZ_MAGIC   (0x5A4C425AUL)
#define ZB_HDR_BYTES (16UL)
#define CRC32_POLY   (0xEDB88320UL)

/* The sync word sits within the first 128 bytes of every 7-series
   layout. */
#define SYNC_SCAN_BYTES (128UL)

/* Staging buffer between the decoder and the DMA: 64-byte aligned
   (64-byte bursts never straddle a 4 KB page), cacheable low OCM.
   DevC is a central-interconnect master, not an ACP master: clean
   L1+L2 with cache_clean() before each DMA. The L2 clean is a no-op
   for OCM, but keeps the path correct if this buffer ever moves. */
#define DMA_STAGE_BYTES (4096UL)
static uint32_t dma_stage_buf[DMA_STAGE_BYTES / 4UL] __attribute__((aligned(64)));

struct zb_header
{
    uint32_t magic;
    uint32_t raw_len;
    uint32_t packed_len;
    uint32_t crc32;
};

/**
 * @brief  Busy-wait 'loops' iterations (volatile counter, survives -Og).
 */
static void busy_wait_loops(uint32_t loops)
{
    volatile uint32_t i;

    for (i = 0U; i < loops; i++)
    {
        /* spin */
    }
}

/**
 * @brief  Wait for the PL to reach a given PCFG_INIT state.
 * @param  ready true: wait until the PL reports ready (PCFG_INIT = 1);
 *               false: wait until it reports reset (PCFG_INIT = 0).
 * @return PL_OK, or PL_ERR_NOT_RESET / PL_ERR_NOT_READY on timeout.
 */
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

/**
 * @brief  Pulse one PS->PL reset line; the other three keep running. No
 *         pending AXI transactions when called.
 * @param  line Reset line, 0..3 (FCLKRESETN[line]).
 * @return 0 on success, -1 on a bad line.
 */
int pl_reset_pulse(uint32_t line)
{
    if (line >= PL_RESET_LINE_COUNT)
    {
        return -1;
    }
    if (pl_reset_assert(line) != 0)
    {
        return -1;
    }
    busy_wait_loops(RESET_PULSE_LOOPS);
    return pl_reset_release(line);
}

/**
 * @brief  Erase the current PL design: PROG_B 1 -> 0 -> 1, wait for
 *         PCFG_INIT to fall and rise. Drops the stale DONE flag so a
 *         later load is not fooled by the previous design.
 * @return PL_OK, or the reason the PL never came back.
 */
enum pl_status pl_config_clear(void)
{
    uint32_t ctrl;
    enum pl_status st;

    if (pl_is_powered() == false)
    {
        return PL_ERR_NO_POWER;
    }

    /* RMW: CTRL holds PCAP mode/PR/POR count too; devcfg is strongly
       ordered, no barriers needed. */
    ctrl = DEVCFG->CTRL;
    DEVCFG->CTRL = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    DEVCFG->CTRL = ctrl & ~DEVCFG_CTRL_PCFG_PROG_B_Msk;

    st = wait_pl_init(false);
    if (st != PL_OK)
    {
        return st;
    }

    DEVCFG->CTRL = ctrl | DEVCFG_CTRL_PCFG_PROG_B_Msk;
    DEVCFG->INT_STS = DEVCFG_IXR_PCFG_DONE_Msk; /* W1C: stale DONE */
    return wait_pl_init(true);
}

/**
 * @brief  Acknowledge the last DMA completion (INT_STS[DMA_DONE] and
 *         STATUS[DMA_DONE_CNT], both W1C), so a stale completion is not
 *         mistaken for the one being waited on.
 */
static void pcap_ack_dma_done(void)
{
    DEVCFG->INT_STS = DEVCFG_IXR_DMA_DONE_Msk;
    DEVCFG->STATUS |= DEVCFG_STATUS_DMA_DONE_CNT_Msk;
}

/**
 * @brief  Check the first bytes for the PCAP sync word 0xAA995566 in
 *         little-endian order (66 55 99 AA) - the "byte swapped .bin"
 *         form. A native big-endian bitgen .bin only fails later with a
 *         generic NO_DONE.
 * @param  img Start of the image.
 * @param  len Length of the image in bytes.
 * @return True when present.
 */
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

/* Table-driven CRC-32 (reflected, 0xEDB88320). Runs without the final
   XOR so it carries across staging chunks; finish with ~crc. */
static uint32_t crc32_table[256];

/**
 * @brief  Build the CRC-32 table; once per load.
 */
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

/**
 * @brief  Fold bytes into a running CRC-32 (no final inversion; ~crc at
 *         the end gives the final value).
 */
static uint32_t crc32_update(uint32_t crc, const uint8_t *bytes, uint32_t len)
{
    uint32_t i;

    for (i = 0U; i < len; i++)
    {
        crc = (crc >> 8U) ^ crc32_table[(crc ^ bytes[i]) & 0xFFU];
    }
    return crc;
}

/**
 * @brief  Prepare the DevC/PCAP bridge: PS->PL shifters on (ch.2 step 2),
 *         unlock word, clear pending interrupts, PCAP mode + full POR.
 */
static void pcap_open(void)
{
    uint32_t ctrl;

    /* UG585 ch.2 step 2: PS->PL level shifters on. */
    pl_lvl_ps_to_pl();

    /* DevC APB registers reject writes until the unlock word is written
       once (UG585 devcfg.UNLOCK). */
    DEVCFG->UNLOCK = DEVCFG_UNLOCK_DATA;

    /* Clear every pending DevC interrupt. */
    DEVCFG->INT_STS = 0xFFFFFFFFUL;

    /* PCAP on, PCAP (not ICAP) path, full rate. CTRL[14:13] must be
       written 1, hence the RMW. */
    ctrl = DEVCFG->CTRL;
    ctrl |= DEVCFG_CTRL_PCAP_MODE_Msk | DEVCFG_CTRL_PCAP_PR_Msk;
    ctrl &= ~(DEVCFG_CTRL_PCAP_RATE_EN_Msk | DEVCFG_CTRL_PCFG_POR_CNT_4K_Msk);
    DEVCFG->CTRL = ctrl;
}

/**
 * @brief  Bring PL and bridge to "ready for data" (ch.6 steps 1-6 minus
 *         the DMA): power check, PCAP clock, PROG_B handshake, command
 *         queue check, drop stale state. Shared by both load paths.
 * @return PL_OK, or the reason it failed.
 */
static enum pl_status pcap_start(void)
{
    enum pl_status st;

    /* MCTRL[PCFG_POR_B]: reported here, not as a PCFG_INIT timeout. */
    if (pl_is_powered() == false)
    {
        return PL_ERR_NO_POWER;
    }

    /* PCAP clock per load, so a second load cannot start gated. */
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->PCAP_CLK_CTRL |= PCAP_CLK_CTRL_CLKACT;
    SLCR->LOCK = SLCR_LOCK_KEY;

    /* Erase the old design and wait for the PL to come back ready. */
    st = pl_config_clear();
    if (st != PL_OK)
    {
        return st;
    }

    /* The DMA command queue must have room; PCAP loopback off. */
    if ((DEVCFG->STATUS & DEVCFG_STATUS_DMA_CMD_Q_F_Msk) != 0U)
    {
        return PL_ERR_CMD_Q_FULL;
    }
    DEVCFG->MCTRL &= ~DEVCFG_MCTRL_PCAP_LPBK_Msk;

    /* Drop any completion left over from an earlier transfer. */
    pcap_ack_dma_done();

    return PL_OK;
}

/**
 * @brief  One DMA command to the PCAP (ch.6 steps 8-10: queue, wait,
 *         ack). Writing DMA_DEST_LEN last is what starts the transfer;
 *         the magic destination address routes to the PCAP.
 * @param  src   64-byte aligned, must not straddle a 4 KB boundary.
 * @param  words Length in 32-bit words.
 * @return PL_OK, or the transport error.
 */
static enum pl_status pcap_send_block(const void *src, uint32_t words)
{
    uint32_t i;
    uint32_t int_sts;

    if ((DEVCFG->STATUS & DEVCFG_STATUS_DMA_CMD_Q_F_Msk) != 0U)
    {
        return PL_ERR_CMD_Q_FULL;
    }

    DEVCFG->DMA_SRC_ADDR = (uint32_t)(uintptr_t)src;
    DEVCFG->DMA_DEST_ADDR = DEVCFG_DMA_INVALID_ADDRESS; /* route to PCAP */
    DEVCFG->DMA_SRC_LEN = words;
    __DSB();
    DEVCFG->DMA_DEST_LEN = words; /* triggers the transfer */

    for (i = 0U; i < DMA_DONE_POLL_LIMIT; i++)
    {
        int_sts = DEVCFG->INT_STS;
        if ((int_sts & DMA_ERROR_MASK) != 0U)
        {
            DEVCFG->INT_STS = int_sts; /* W1C: drop the latched error bits */
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

/**
 * @brief  Wait for PL DONE, then open the level shifters (ch.6 step 11
 *         + ch.2 step 5).
 * @return PL_OK, or PL_ERR_NO_DONE.
 */
static enum pl_status pcap_wait_pl_done(void)
{
    uint32_t i;

    for (i = 0U; i < DMA_DONE_POLL_LIMIT; i++)
    {
        if ((DEVCFG->INT_STS & DEVCFG_IXR_PCFG_DONE_Msk) != 0U)
        {
            break;
        }
    }
    DEVCFG->INT_STS = DEVCFG_IXR_PCFG_DONE_Msk;
    if (i == DMA_DONE_POLL_LIMIT)
    {
        return PL_ERR_NO_DONE;
    }

    /* UG585 ch.2 step 5: the PL drives PS inputs now. */
    pl_lvl_all();

    return PL_OK;
}

/**
 * @brief  Raw bitstream, one DMA command - fallback and debug escape
 *         hatch; the build normally packs the image.
 * @param  img 64-byte aligned.
 * @param  len Multiple of 4.
 * @return PL_OK, or the reason it failed.
 */
static enum pl_status load_raw_bitstream(const uint8_t *img, uint32_t len)
{
    if (!has_pcap_sync_word(img, len))
    {
        return PL_ERR_NO_SYNC;
    }
    return pcap_send_block(img, len / 4UL);
}

/**
 * @brief  Parse and validate a ZBLZ header; pure memory work, rejects a
 *         malformed blob before any hardware is touched.
 * @param  img   Start of the packed image.
 * @param  len   Length of the whole image in bytes.
 * @param  hdr   Out: the decoded header.
 * @return PL_OK, or PL_ERR_HDR.
 */
static enum pl_status parse_zb_header(const uint8_t *img, uint32_t len, struct zb_header *hdr)
{
    struct zb_header parsed;

    if (len < ZB_HDR_BYTES)
    {
        return PL_ERR_HDR;
    }
    memcpy(&parsed, img, sizeof(parsed));
    if (parsed.magic != ZBLZ_MAGIC)
    {
        return PL_ERR_HDR;
    }
    /* raw_len must be a non-empty word stream; packed_len must fit in
       the image (len is normally exactly 16 + packed_len; slack is
       tolerated for robustness). */
    if ((parsed.raw_len < 4UL) || ((parsed.raw_len & 3UL) != 0UL))
    {
        return PL_ERR_HDR;
    }
    if ((parsed.packed_len == 0UL) || (parsed.packed_len > (len - ZB_HDR_BYTES)))
    {
        return PL_ERR_HDR;
    }
    *hdr = parsed;
    return PL_OK;
}

/* Sink state for load_zblz_image(): the staging buffer fills through
   zblz_sink() while lzma_decode_stream() runs. Chunk lengths are whole
   words (middleware contract), so 'filled' is always word-aligned. */
struct zblz_sink
{
    uint8_t *stage;
    uint32_t filled; /* bytes staged, awaiting DMA            */
    uint32_t crc;    /* running CRC over DMA'd words          */
    bool sync_checked;
    enum pl_status st; /* first sink-side error sticks          */
};

/**
 * @brief  DMA what is staged: sync-word check (first block only),
 *         cache clean, CRC fold, one PCAP DMA command.
 */
static enum pl_status zblz_flush(struct zblz_sink *s)
{
    uint32_t scan;
    enum pl_status st;

    if (s->filled == 0UL)
    {
        return PL_OK;
    }
    if (!s->sync_checked)
    {
        scan = (s->filled < SYNC_SCAN_BYTES) ? s->filled : SYNC_SCAN_BYTES;
        if (!has_pcap_sync_word(s->stage, scan))
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

/**
 * @brief  Sink for lzma_decode_stream(): stage decoded words, flush one
 *         DMA per full 4 KB block. pcap_send_block() only polls devcfg
 *         registers - safe to call mid-decode.
 */
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

/**
 * @brief  LZMA-decode a ZBLZ image and stream it to the PCAP through
 *         the shared staging buffer. The middleware checks the stream
 *         shape (end marker, exact lengths); this layer adds the
 *         sync-word check, the CRC and the DMA flow.
 * @param  img Start of the packed image (header included).
 * @param  hdr Header validated by parse_zb_header().
 * @return PL_OK, or the reason it failed.
 */
static enum pl_status load_zblz_image(const uint8_t *img, const struct zb_header *hdr)
{
    struct zblz_sink s;

    crc32_table_init(); /* zblz_flush() folds through the table */
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
    s.st = zblz_flush(&s); /* final partial block */
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

/**
 * @brief  Push an image over the PCAP bridge; requires pcap_open(). The
 *         first word picks the path - only packed blobs start with the
 *         "ZBLZ" magic (bitgen images start with 0xFFFFFFFF).
 * @param  img ZBLZ blob (no alignment) or raw bitstream (64-byte
 *             aligned).
 * @param  len Multiple of 4.
 * @return PL_OK, or the reason it failed.
 */
static enum pl_status pl_load_image(const void *img, uint32_t len)
{
    uintptr_t addr = (uintptr_t)img;
    const uint8_t *bytes = (const uint8_t *)img;
    uint32_t magic;
    enum pl_status st;

    if ((addr & 0x3FU) != 0U)
    {
        return PL_ERR_SRC_ALIGN;
    }
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

/**
 * @brief  Configure the PL from an image (public entry point): bridge up,
 *         all reset lines asserted (a half-configured design never runs),
 *         push, wait for DONE, open the shifters. The lines stay
 *         asserted on return - clock the fabric and release when ready.
 * @param  img ZBLZ packed blob or raw bitstream.
 * @param  len Length in bytes, a multiple of 4.
 * @return PL_OK, or the reason it failed.
 */
enum pl_status pl_load(const void *img, uint32_t len)
{
    /* Before the image is touched: a rejected image leaves the bridge
       usable. */
    pcap_open();

    /* Partial designs would run; hold the logic until DONE. */
    pl_reset_assert_all();

    return pl_load_image(img, len);
}
